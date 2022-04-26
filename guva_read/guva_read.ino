/*
R2  - read sensor value as int instead of float
	- divide by 1023 instead of 1024
R3  - second sensor
R4  - smoothing
R5  - 4 sensors
R6  - 6 sensors
R7  - multiplexer (no smoothing)
R8  - multiplexer with smoothing
R9  - log / lin adjustment **** NEEDS IMPROVEMENT
R10 - op amp output along with raw output
R11 - log / lin with op amp and autoselect channel (raw or amp) for irrad output
R12 - low irrad cutoff variable added to sensor class
	- high irrad cutoff warning
R13 - digital lowpass filter for smoothing (replace averaging implemented in R4)
	- one channel with multiple tau values to test and print
	  T/tau = 16 looks good
R14 - TODO: all channels with one tau (still needs to be done)
	- TEST FILTERING AFTER IRRAD COMPUTATION - noise amplified by calculation
	  so many need to make T/tau > 16 to further damp noise so that final irrad
	  value is not noisy
##############################################################
USE T/TAU = 16 !!!
##############################################################
R15 - all channels with raw and gain filtered (tau 16)
    -

to add:
- integrator to get total dosage
- timer schedule so sensors are read at regular interval
	- needed for consistent integration of dosage
	- needed for consistent lowpass filtering (recall we have T and tau)
- mux class?
	- params
		- sig pin
		- gain pin
		- control pins
		- num channels
		- interval for reading sensors continuously
	- methods
		- read all sensors once
			- filter
		- read all sensors continuously
		- read one sensor once
		- read all sensors once
		- output to csv or thumb drive (serial input triggers for starting and stopping recording?)



added:
- a cutoff variable as part of the sensor class (raw and amplified)
  - because there is an offset voltage, the sensor output will be greater than zero
	when the input to the sensor is 0 (complete darkness)
  - the analog output due to the offset voltage is known from calibration, this is to be set as the cutoff
  - if the sensor output is close to this cutoff, need to tell user that irrad is too low to register
  - for example, sensor 5 flattens at 200 uW/cm2. any irradiance below this will
	spit out ~4 (raw) or ~290 (G=47). if the irrad drops below 200 uW/cm2, the sensor will still
	spit out 4 and 290, which will incorrectly return 200 uW/cm2 to the user. need to set cutoff of
	6 and 300 (a little higher than flatline to be safe) and when output is below this the user
	is warned that irrad is too low to measure
*/




class sensor
{
	int id; // id number of sensor

	// m, b, and cutoff values (raw and gain) come from calibration data

	// calibration data for raw stuff
	float m_r; // slope (m) and intercept (b) of y=mx+b regression
	float b_r;
	float anlg_cutoff_r; // analog values below this will print "X"

	// calibration data for amplified stuff (gain)
	float m_g0; //gain0 - may expand to have multiple op amp channels
	float b_g0;
	float anlg_cutoff_g0;

//    // lowpass filter coeffs
//    float tau_r;
//    float tau_g0;

	public:
	sensor(int ID, float M_R, float B_R, float ANLG_CUTOFF_R, float M_G0, float B_G0, float ANLG_CUTOFF_G0)
	// m and b are from reference of irr (I) on x axis, analog (A) on y axis
	// i.e. A = m*I + b
	{
		id = ID;

		m_r = M_R;
		b_r = B_R;
		anlg_cutoff_r = ANLG_CUTOFF_R;

		m_g0 = M_G0;
		b_g0 = B_G0;
		anlg_cutoff_g0 = ANLG_CUTOFF_G0;

//        tau_r = 1;
//        tau_g0 = 1;
	} // end public


	float irr_raw(float ao)
	{
		float irr = (ao - b_r)/m_r;
		return irr;
	}

	float irr_g0(float ao)
	{
		float irr = (ao - b_g0)/m_g0;
		return irr;
	}

	float get_anlg_cutoff_r()
	{
		float cut = 1.1*anlg_cutoff_r; // 10% buffer added to cutoff
		return cut;
	}

	float get_anlg_cutoff_g0()
	{
		float cut = 1.1*anlg_cutoff_g0; // 10% buffer added to cutoff
		return cut;
	}

}; // end sensor class

//Mux control pins
const int s0 = 5;
const int s1 = 4;
const int s2 = 3;
const int s3 = 2;

//Mux in "SIG" pin
const int SIG_pin = 6;
const int GAIN_pin = 3;

// number of channels
const int max_channel = 8;

// interval for reading sensors (units????)
const float T = 1;
const float tau = 16;

// for filtered raw readings
float raw_f[max_channel];

// for filtered op amp readings
// g0 indicates ch0 of gain
// there is expandability to multiple gain channelsf or higher res at even lower readings
float g0_f[max_channel];

float irr[max_channel];

sensor sens_arr[max_channel]{
    // DATA FROM 2022 APR 25
    //sensor(int ID, float M_R, float B_R, float CUTOFF_R, float M_G0, float B_G0, float CUTOFF_G0)
    //
    //             RAW                          GAIN
    // id   slope, intcpt, cutoff       slope, intcpt, low cutoff
    // ------------------------------------------------------
    {0, 0.0492, -9.5134, 5,     2.5322, -412.43,  80},
    {1, 0.0800, -8.2195, 5,     4.1207, -370.50, 157},
    {2, 0.1133, +1.0343, 5,     6.3067, +33.803, 400},
    {3, 0.0973, -5.4148, 5,     4.9533, -240.55, 125},
    {4, 0.0599, +1.4029, 5,     3.0105, +134.53, 368},
    {5, 0.0632, -13.559, 5,     3.5144, -726.62, 313},
    {6, 0.0706, -10.680, 5,     3.8513, -539.97, 207},
    {7, 0.0619, +2.0349, 5,     3.1128, +166.87, 350},
};

void setup(){
	pinMode(s0, OUTPUT);
	pinMode(s1, OUTPUT);
	pinMode(s2, OUTPUT);
	pinMode(s3, OUTPUT);

	digitalWrite(s0, LOW);
	digitalWrite(s1, LOW);
	digitalWrite(s2, LOW);
	digitalWrite(s3, LOW);

	// sensor(int CHANNEL, float M_LIN, float B_LIN, float M_LOG, float M_LIN)
	// (int , float , float , float , float )
	// ( CHANNEL,  M_LIN,  B_LIN,  M_LOG,  M_LIN)
//    sensor s0(0, 1,1,1,1);

	Serial.begin(9600);

    Serial.println("PROGRAM: GUVA_READ_R15");
    Serial.println("EDIT: 2022 APR 25");

	Serial.println("-------------------------------------------------------");
	Serial.println("--------------------- BEGIN TEST ----------------------");
	Serial.println("-------------------------------------------------------");

    Serial.println("\nUNITS IN uW/cm^2\n");

    Serial.println("ch0,, ch1,, ch2,, ch3,, ch4,, ch5,, ch6,, ch7");
    Serial.print("ch0_msg, ch0_irr, ch1_msg, ch1_irr, ch2_msg, ch2_irr, ch3_msg, ch3_irr, ");
    Serial.println("ch4_msg, ch4_irr, ch5_msg, ch5_irr, ch6_msg, ch6_irr, ch7_msg, ch7_irr");
}


void loop(){

	// for each channel...
	for(int i = 0; i < max_channel; i ++){
		// Serial.print("Value at channel ");
		// Serial.print(i);
		// Serial.print(" is : ");
		// Serial.println(readMux(i));

		int *p; // make a pointer to store readings
		p = readMux(i); // *p returns raw reading, *(p+1) returns gain0

		// this implements lowpass filter 1/(tau*s+1)
		raw_f[i] = raw_f[i] + T/tau*(-raw_f[i] + *p);

		g0_f[i] = g0_f[i] + T/tau*(-g0_f[i] + *(p+1));

		// now compute irradiance and print
		if (g0_f[i] < 1.1*sens_arr[i].get_anlg_cutoff_g0()) {
			// if the amp signal is too low, output 0 irradiance and warning
			irr[i] = 0;
			Serial.print("amp cut (low irrad), ");
		} else if (g0_f[i] < 1000) {
			// if amp signal is <1000 (not too close to 1024 saturation), use it
			irr[i] = sens_arr[i].irr_g0(g0_f[i]);
			Serial.print("amp, ");
		} else if (raw_f[i] < 5) {
			// if raw signal is too low, output 0 irrad and warning
			irr[i] = 0;
			Serial.print("raw cut (mid irrad), ");
		} else if (raw_f[i] > 1000) {
			// if raw signal is >1000 (close to saturated)
			irr[i] = 999999;
			Serial.print("!!WARNING!! IRRADIANCE TOO HIGH, SENSORS MAY INCUR DAMAGE, ");
		} else {
			// if amp signal close to saturation, use raw signal
			irr[i] = sens_arr[i].irr_raw(raw_f[i]);
			Serial.print("raw, ");
		}
        Serial.print(irr[i]);
        Serial.print(", ");

	}
	Serial.println("");
}



int * readMux(int channel){
	int controlPin[] = {s0, s1, s2, s3};

	int muxChannel[16][4]={
		{0,0,0,0}, //channel 0
		{1,0,0,0}, //channel 1
		{0,1,0,0}, //channel 2
		{1,1,0,0}, //channel 3
		{0,0,1,0}, //channel 4
		{1,0,1,0}, //channel 5
		{0,1,1,0}, //channel 6
		{1,1,1,0}, //channel 7
		{0,0,0,1}, //channel 8
		{1,0,0,1}, //channel 9
		{0,1,0,1}, //channel 10
		{1,1,0,1}, //channel 11
		{0,0,1,1}, //channel 12
		{1,0,1,1}, //channel 13
		{0,1,1,1}, //channel 14
		{1,1,1,1}  //channel 15
	};

	// loop through the 4 signal pins
	// to set path to desired channel
	for(int i = 0; i < 4; i ++){
		digitalWrite(controlPin[i], muxChannel[channel][i]);
	}

	// read values of desired channel
	int raw = analogRead(SIG_pin);
	int amp = analogRead(GAIN_pin);

	// make static integer array to store values
	static int output[2];

	// put values in array
	output[0] = raw;
	output[1] = amp;

	return output;
}
