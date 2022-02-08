/*
R2  - read sensor value as int instead of float
    - divide by 1023 instead of 1024
R3  - second sensor
R4  - smoothing
R5  - 4 sensors
R6  - 6 sensors
R7  - multiplexer (no smoothing)
R8  - multiplexer with smoothing
*/

//Mux control pins
const int s0 = 5;
const int s1 = 4;
const int s2 = 3;
const int s3 = 2;

//Mux in "SIG" pin
const int SIG_pin = 6;

// number of channels
const int max_channel = 8;


float total[max_channel];
float average[max_channel];
float sensorVolt[max_channel];
// const int numReadings = 25;
const int numReadings = 50;

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
        total[i] = 0;
    }


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



    for (int i = 0; i < max_channel; i++) {
        average[i] = total[i] / numReadings;
        Serial.print(average[i]);
        Serial.print(", ");
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
