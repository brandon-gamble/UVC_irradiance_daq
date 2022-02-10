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
*/

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
//const int max_channel = 1;

// for raw readings
float total_raw[max_channel];
float average_raw[max_channel];
float sensorVolt_raw[max_channel];

// for op amp readings
float total_oa[max_channel];
float average_oa[max_channel];
float sensorVolt_oa[max_channel];

const int numReadings = 50;
//const int numReadings = 1;

void setup(){
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  Serial.begin(9600);

  Serial.println("-------------------------------------------------------");
  Serial.println("--------------------- BEGIN TEST ----------------------");
  Serial.println("-------------------------------------------------------");
}


void loop(){
    // total_1 = 0;
    // total_2 = 0;
    // total_3 = 0;
    // total_4 = 0;
    // total_5 = 0;
    // total_6 = 0;
    // total_7 = 0;

    // reset the totals
    for (int i = 0; i < max_channel; i++){
        total_raw[i] = 0;
        total_oa[i] = 0;
    }


    for (int j = 0; j < numReadings; j++){
        for(int i = 0; i < max_channel; i ++){
            // Serial.print("Value at channel ");
            // Serial.print(i);
            // Serial.print(" is : ");
            // Serial.println(readMux(i));
            
            int *p;
            p = readMux(i);
            
            total_raw[i] = total_raw[i] + *p;
            total_oa[i] = total_oa[i] + *(p+1);
        }
        delay(1);
    }



    for (int i = 0; i < max_channel; i++) {
        average_raw[i] = total_raw[i] / numReadings;
        average_oa[i] = total_oa[i] / numReadings;
        Serial.print(average_raw[i]);
        Serial.print(", ");
        Serial.print(average_oa[i]);
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
