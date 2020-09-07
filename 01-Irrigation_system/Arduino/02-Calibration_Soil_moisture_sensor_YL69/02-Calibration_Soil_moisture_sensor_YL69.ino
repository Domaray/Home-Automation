int sensor_pin = A0;
int output_value_analogic ;
int output_value_percentage ;

void setup() {
  Serial.begin(115200); 
  Serial.println("Moisture sensor reading ..."); 
  delay(2000);  
 }

 
void loop() {  
  output_value_analogic = analogRead(sensor_pin);
  output_value_percentage = map(output_value_analogic,1024,433,0,100); 
  Serial.print("Analogic value: "); 
  Serial.print(output_value_analogic);
  Serial.print("   - Percentage: "); 
  Serial.print(output_value_percentage);
  Serial.print("%\n");  
  delay(1000);  
}
