/*
R2  - read sensor value as int instead of float
    - divide by 1023 instead of 1024
    -

*/
void setup() 
{
  Serial.begin(9600);
  
}
 
void loop() 
{
  float sensorVoltage; 
  float sensorValue; // THIS SHOULD BE AN INT
  
  sensorValue = analogRead(A0);
  sensorVoltage = sensorValue/1023*5;
//  Serial.print("sensor reading = ");
  Serial.print(sensorValue);
  Serial.println("");
//  Serial.print("sensor voltage = ");
//  Serial.print(sensorVoltage);
//  Serial.println(" V");
  delay(5);
}
