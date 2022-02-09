/*
R2  - read sensor value as int instead of float
    - divide by 1023 instead of 1024
R3  - second sensor
R4  - smoothing
R5  - 4 sensors
R6  - 6 sensors
R7  - multiplexer (no smoothing)
R8  - multiplexer with smoothing
R9  - irrad as func of analog reading
*/

class sensor
{
    int id;

    float m_lin;
    float b_lin;

    float m_log;
    float b_log;

    public:
    sensor(int ID, float M_LIN, float B_LIN, float M_LOG, float B_LOG)
    // m and b are from reference of irr on x axis, analog on y axis
    {
        id = ID;

        m_lin = M_LIN;
        b_lin = B_LIN;

        m_log = M_LOG;
        b_log = B_LOG;
    } // end public


    float irr_lin(float ao)
    {
        float irr = (ao - b_lin)/m_lin;
        return irr;
    }

    float irr_log(float ao)
    {
        float expon = (ao - b_log)/m_log;
        float irr = pow(2.71828, expon);
        return irr;
    }

}; // end sensor class

//Mux control pins
const int s0 = 5;
const int s1 = 4;
const int s2 = 3;
const int s3 = 2;

//Mux in "SIG" pin
const int SIG_pin = 6;

// number of channels
const int max_channel = 8;

sensor sens_arr[max_channel]{
//sensor(int ID, float M_LIN, float B_LIN, float M_LOG, float B_LOG)
    {0,  160.46, -130.77,  1,1},
    {1,  66.228, -58.801,  1,1},
    {2,  111.10, -73.126,  1,1},
    {3,  105.10, -70.659,  1,1},
    {4,  53.724, -30.333,  1,1},
    {5,  68.834, -56.017,  1,1},
    {6,  69.828, -55.340,  1,1},
    {9,  179.27, -118.98,  1,1},
};

float total[max_channel];
float average[max_channel];
float irr[max_channel];
// const int numReadings = 25;
const int numReadings = 500;

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
    sensor s0(0, 1,1,1,1);

    Serial.begin(9600);

    Serial.println("-------------------------------------------------------");
    Serial.println("--------------------- BEGIN TEST ----------------------");
    Serial.println("-------------------------------------------------------");
}


void loop(){

    // reset the totals
    for (int i = 0; i < max_channel; i++){
        total[i] = 0;
    }

    // for each reading, read every channel and add to respective total
    for (int j = 0; j < numReadings; j++){
        for(int i = 0; i < max_channel; i ++){
            // Serial.print("Value at channel ");
            // Serial.print(i);
            // Serial.print(" is : ");
            // Serial.println(readMux(i));
            total[i] = total[i] + readMux(i);
        }
        delay(1);
    }


    // get average from each total
    for (int i = 0; i < max_channel; i++) {
        average[i] = total[i] / numReadings;

        irr[i] = sens_arr[i].irr_lin(average[i]);
        // irr[i] = sens_arr[i].irr_log(average[i])

        Serial.print(average[i]);
        Serial.print(", ");

        Serial.print(irr[i]);
        Serial.print(" | ");
    }

    Serial.println("");


}



int readMux(int channel){
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

    //loop through the 4 sig
    for(int i = 0; i < 4; i ++){
        digitalWrite(controlPin[i], muxChannel[channel][i]);
    }

    //read the value at the SIG pin
    int val = analogRead(SIG_pin);

    //return the value
    return val;
}
