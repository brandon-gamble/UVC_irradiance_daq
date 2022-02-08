/*
R2  - read sensor value as int instead of float
    - divide by 1023 instead of 1024
R3  - second sensor
R4  - smoothing
*/

const int numReadings = 15;

int total_1 = 0;                  // the running total
int average_1 = 0;                // the average
float sensorVolt_1 = 0;

int total_2 = 0;                  // the running total
int average_2 = 0;                // the average
float sensorVolt_2 = 0;

int total_3 = 0;                  // the running total
int average_3 = 0;                // the average
float sensorVolt_3 = 0;

int guva_1 = A0;
int guva_2 = A1;
int guva_3 = A2;

void setup() 
{
  Serial.begin(9600);
}
 
void loop() 
{
  total_1 = 0;
  total_2 = 0;
  total_3 = 0;
  for (int i=0; i < numReadings; i++){
    total_1 = total_1 + analogRead(guva_1);
    total_2 = total_2 + analogRead(guva_2);
    total_3 = total_3 + analogRead(guva_3);
//    delay(1);
  }
  
  average_1 = total_1 / numReadings;
  average_2 = total_2 / numReadings;
  average_3 = total_3 / numReadings;

  sensorVolt_1 = float(average_1) / 1023 * 5;
  sensorVolt_2 = float(average_2) / 1023 * 5;
  sensorVolt_3 = float(average_3) / 1023 * 5;


 
  Serial.print(average_1);
  Serial.print(",");
  Serial.print(average_2);
  Serial.print(",");
  Serial.println(average_3);
  
//  Serial.print(sensorVolt_1);
//  Serial.print(",");
//  Serial.println(sensorVolt_2);


}
