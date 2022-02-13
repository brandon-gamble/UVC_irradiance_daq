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


to add:
- integrator to get total dosage


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

    // raw stuff
    float m_r; // slope (m) and intercept (b) of y=mx+b regression
    float b_r;
    float anlg_cutoff_r; // analog values below this will print "X"

    // amplified stuff (gain)
    float m_g0; //gain0 - may expand to have multiple op amp channels
    float b_g0;
    float anlg_cutoff_g0;

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
        float cut = 1.1*anlg_cutoff_r;
        return cut;
    }

    float get_anlg_cutoff_g0()
    {
        float cut = 1.1*anlg_cutoff_g0;
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

// for raw readings
float total_raw[max_channel];
float average_raw[max_channel];

// for op amp readings
float total_g0[max_channel];
float average_g0[max_channel];

float irr[max_channel];

const int numReadings = 50;

sensor sens_arr[max_channel]{
//sensor(int ID, float M_R, float B_R, float CUTOFF_R, float M_G0, float B_G0, float CUTOFF_G0)
    {0,  0.1206,  -9.4596,  0.0,      5.5157, -326.190,   15},
    {1,  0.0902,  -8.2601,  0.1,      4.2468, -282.230,  110},
    {2,  0.2096,   0.5314,  0.0,     11.3250,   27.652,   95},
    {3,  0.1952,  -7.2172,  0.0,     10.0040, -306.580,   66},
    {4,  0.1035,   0.2135,  0.8,      5.4794,   63.844,  135},
    {5,  0.1024, -16.3890,  4.0,      4.6573, -626.020,  296},
    {6,  0.1074, -11.2690,  1.8,      4.8570, -379.020,  180},
    {9,  0.1309,   2.2296,  0.7,      6.5901,  177.160,  131},
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

    Serial.println("-------------------------------------------------------");
    Serial.println("--------------------- BEGIN TEST ----------------------");
    Serial.println("-------------------------------------------------------");
}


void loop(){

    // reset the totals
    for (int i = 0; i < max_channel; i++){
        total_raw[i] = 0;
        total_g0[i] = 0;
    }

    // for each reading, read every channel and add to respective total
    for (int j = 0; j < numReadings; j++){
        for(int i = 0; i < max_channel; i ++){
            // Serial.print("Value at channel ");
            // Serial.print(i);
            // Serial.print(" is : ");
            // Serial.println(readMux(i));

            int *p;
            p = readMux(i);

            total_raw[i] = total_raw[i] + *p;
            total_g0[i] = total_g0[i] + *(p+1);
        }
        delay(1);
    }


    for (int i = 0; i < max_channel; i++) {
        // get averages (smoothing) and print analog outputs
        average_raw[i] = total_raw[i] / numReadings;
        average_g0[i] = total_g0[i] / numReadings;
//        Serial.print(average_raw[i]);
//        Serial.print(", ");
//        Serial.print(average_g0[i]);
//        Serial.print(", ");

        // compute irradiance and print
        if (average_g0[i] < 1.1*sens_arr[i].get_anlg_cutoff_g0()) {
            // if the amp signal is too low, output 0 irradiance and warning
            irr[i] = 0;
            Serial.print("amp cut, ");
        } else if (average_g0[i] < 1000) {
            // if amp signal is <1000 (not too close to 1024 saturation), use it
            irr[i] = sens_arr[i].irr_g0(average_g0[i]);
            Serial.print("amp, ");
        } else if (average_raw[i] < 5) {
            // if raw signal is too low, output 0 irrad and warning
            irr[i] = 0;
            Serial.print("raw cut, ");
        } else if (average_raw[i] > 1000) {
            // if raw signal is >1000 (close to saturated)
            irr[i] = 0;
            Serial.print("TOO HIGH, ");
        } else {
            // if amp signal close to saturation, use raw signal
            irr[i] = sens_arr[i].irr_raw(average_raw[i]);
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
