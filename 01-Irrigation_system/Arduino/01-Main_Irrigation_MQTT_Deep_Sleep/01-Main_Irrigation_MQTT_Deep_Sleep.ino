/*  
 *  IRRIGATION SYSTEM WITH NODEMCU + HOMEASSISTANT + MQTT 
 *  
 *   _____                                                     
 *  |  __ \                                                    
 *  | |  | |   ___    _ __ ___     __ _   _ __    __ _   _   _ 
 *  | |  | |  / _ \  | '_ ` _ \   / _` | | '__|  / _` | | | | |
 *  | |__| | | (_) | | | | | | | | (_| | | |    | (_| | | |_| |
 *  |_____/   \___/  |_| |_| |_|  \__,_| |_|     \__,_|  \__, |
 *                                                        __/ |
 *                                                       |___/                                                   
 *  DEVELOPED BY DOMARAY. Sept'20. 
 *  See www.domaray.com for more info.
 *  
 *  
 *  DESCRIPCION:
 *  
 *  This low-cost irrigation system integrates a nodeMCU board into HomeAssistant to control the irrigation from your dashboard / phone app.
 *  The communication between the board and HomeAssistant is done using MQTT. The MQTT server used is addon of HomeAssistant.
 *  In order to increase the live of the batteries, a sleep mode has been included.
 *  
 *  WORKING MODES:
 *  
 *  Manual: The relay of the pump is activated manually by HomeAssitant button
 *  Trigger mode: The relay of the pump is activated once the Analogic percentage value is bigger than trigger value set by HomeAssistant
 *  
 *  
 *  PINOUT:
 *  
 *  D0: Connected to RESET pin after programming in order to wake up after a Deep Sleep. REMOVE IT BEFORE PROGRAMMING THE BOARD
 *  D1: Output - Relay activation for the pump
 *  D2: Input - Level sensor of the water tank
 *  A0: Input - Analogic signal of the humidity sensor (soil moisture sensor) YL69 + YL38
 *  VIN: Powered by batteries
 *  GND: Powered by batteries
 *  
 */

#define DEBUG 1

#if DEBUG
#define PRINTLN(s)   { Serial.println(s); } // GENERIC LN
#define PRINT(s)   { Serial.print(s); } // GENERIC
#define PRINTS(s)   { Serial.println(F(s)); } // STRING
#define PRINTN(s,n)  { Serial.print(F(s)); Serial.println(n); } // NUMBERS
#define PRINTH(s,n) { Serial.print(F(s)); Serial.print(F("0x")); Serial.println(n, HEX); } // HEXADECIMAL
#else
#define PRINT(s)
#define PRINTS(s)
#define PRINTN(s,n)
#define PRINTH(s,n)
#endif

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"
#define mqtt_server "IP OF MQTT SERVER"
#define mqtt_user "MQTT USER"
#define mqtt_password "MQTT PASSWORD"
#define clientId "Irrigation_1"  // THIS ID MUST BE UNIQUE. In case of having same ID connected to MQTT server, connections and disconnections will occur continuously.

#define topic_pump_state "home/irrigation/zone_1/pump/state"
#define topic_pump_command "home/irrigation/zone_1/pump/set"
#define topic_pump_available "home/irrigation/zone_1/pump/available"
#define topic_analog_value "home/irrigation/zone_1/analog/value"
#define topic_percentage_value "home/irrigation/zone_1/percentage/value"
#define topic_setPointMode_value "home/irrigation/zone_1/setpointmode/value"
#define topic_setPointMode_state "home/irrigation/zone_1/setpointmode/state"
#define topic_setPointMode_command "home/irrigation/zone_1/setpointmode/set"
#define topic_levelSensor_value "home/irrigation/zone_1/levelsensor/value"
#define topic_deepSleepMode_state "home/irrigation/zone_1/deepsleepmode/state"
#define topic_deepSleepMode_command "home/irrigation/zone_1/deepsleepmode/set"


#define pin_soilSensorAnalogic A0
#define pin_pump D1  
#define pin_levelSensor D2

#define deepSleepTime 60000 // 60 seconds
#define DeepSleepDuration 18e8 // 10e6 is 10.000.000 microseconds  120e6 = 2 minutes, 18e8 = 30 minutes

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
long now = 0;

bool pumpActive = false;
bool setPointMode = false;
bool levelSensor = true;  // Is full?   True = Full = 1 (open); False = Empty = 0 (close)  --> As there is a pullup resistor, normally the tank will be full and won't be power consumption 
int setPointValue = 0;
bool deepSleepMode = false;

int soilSensorAnalogic_value = 0;
int soilSensorPercentage_value = 100; // Setting up to 100, the pump won't activate at boot
char buf[4]; // Buffer to store the sensor value


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(pin_soilSensorAnalogic, INPUT); 
  pinMode(pin_pump, OUTPUT);
  pinMode(pin_levelSensor, INPUT_PULLUP);
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  now = millis();

  if (((now - lastMsg) % 5000) == 0) { // Sends a message every 5 seconds
    soilSensorAnalogic_value = analogRead(pin_soilSensorAnalogic);
    soilSensorPercentage_value = map(soilSensorAnalogic_value,1024,433,0,100);
    levelSensor = digitalRead(pin_levelSensor);

    PRINTN("Analogic value: ",soilSensorAnalogic_value);
    PRINTN("Percentage: ",soilSensorPercentage_value);

    client.publish(topic_percentage_value, itoa(soilSensorPercentage_value, buf, 10));
   
    if(setPointMode){
      if(soilSensorPercentage_value < setPointValue){
        pumpActive = true;
        digitalWrite(pin_pump, LOW);
        PRINTS("Low humidty - Activate pump");
      } else { 
        pumpActive = false;
        digitalWrite(pin_pump, HIGH);
        PRINTS("High humidty - Deactivate pump");
      }
    } 

    if (pumpActive) { client.publish(topic_pump_state, "ON"); } else { client.publish(topic_pump_state, "OFF"); } 
    if (setPointMode) { client.publish(topic_setPointMode_state, "ON"); } else { client.publish(topic_setPointMode_state, "OFF"); }
    if (levelSensor) { client.publish(topic_levelSensor_value, "HIGH"); } else { client.publish(topic_levelSensor_value, "LOW"); }
    if (deepSleepMode) { client.publish(topic_deepSleepMode_state, "ON"); } else { client.publish(topic_deepSleepMode_state, "OFF"); }
  }

  if ((now - lastMsg > deepSleepTime) && !pumpActive && deepSleepMode) {  // When deeSleepTime is reached, the boards goes to sleep, except when pump is active.
    lastMsg = now;
    client.publish(topic_pump_available, "offline");
    delay(1000);
    PRINTS("Deep Sleep mode activated");
    ESP.deepSleep(DeepSleepDuration); 
  }
}


void callback(char* topic, byte* payload, unsigned int length) {

  PRINT("Message arrived [");
  PRINT(topic);
  PRINT("] ");
  for (int i = 0; i < length; i++) {
    PRINT((char)payload[i]);
  }
  PRINTS("");

  /* PUMP */
  if (String(topic_pump_command).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      pumpActive = true;
      client.publish(topic_pump_state, "ON");
      digitalWrite(pin_pump, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      pumpActive = false;
      client.publish(topic_pump_state, "OFF");
      digitalWrite(pin_pump, HIGH);
    }
    lastMsg = now + (deepSleepMode - 20000); // Gives 20 second to send data of the pup before entering into Sleep mode
  }

  /* SETPOINT MODE */
  if (String(topic_setPointMode_command).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      setPointMode = true;
      client.publish(topic_setPointMode_state, "ON");
    } if (strncmp((char *)payload, "OFF", length) == 0){
      setPointMode = false;
      client.publish(topic_setPointMode_state, "OFF");
    }
  }

    /* SETPOINT VALUE */
  if (String(topic_setPointMode_value).equals(topic)){
      setPointValue = atoi((char *)payload);
      PRINTN("SetPoint: ", setPointValue);    
  }


    /* DEEP SLEEP MODE */
  if (String(topic_deepSleepMode_command).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      deepSleepMode = true;
    } if (strncmp((char *)payload, "OFF", length) == 0){
      deepSleepMode = false;
    }
    lastMsg = now + (deepSleepMode - 20000); // Gives 20 second to send data of the pup before entering into Sleep mode
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  PRINTS("Connecting to: ");
  PRINT(wifi_ssid);
  
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PRINTS(".");
  }
  PRINTS("");
  PRINTS("WiFi connected");
  PRINTS("IP address: ");
  PRINT(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    PRINTS("Attempting MQTT connection...");
    if (client.connect(clientId, mqtt_user, mqtt_password)) {
      PRINTS("connected");
      // Once connected, publish the availability
      client.publish(topic_pump_available, "online");

      // and resubscribe to all topics
      client.subscribe(topic_pump_command);
      client.subscribe(topic_pump_state);
      client.subscribe(topic_pump_available);
      client.subscribe(topic_analog_value);
      client.subscribe(topic_percentage_value);
      client.subscribe(topic_setPointMode_state);
      client.subscribe(topic_setPointMode_command);
      client.subscribe(topic_setPointMode_value);
      client.subscribe(topic_levelSensor_value);
      client.subscribe(topic_deepSleepMode_state);
      client.subscribe(topic_deepSleepMode_command);

    } else {
      PRINTS("failed, rc=");
      PRINTS(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
