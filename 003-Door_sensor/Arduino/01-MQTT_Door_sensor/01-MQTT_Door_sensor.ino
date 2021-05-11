/*  
 *  DOOR SENSOR WITH ESP01 + HOMEASSISTANT + MQTT 
 *  
 *   _____                                                     
 *  |  __ \                                                    
 *  | |  | |   ___    _ __ ___     __ _   _ __    __ _   _   _ 
 *  | |  | |  / _ \  | '_ ` _ \   / _` | | '__|  / _` | | | | |
 *  | |__| | | (_) | | | | | | | | (_| | | |    | (_| | | |_| |
 *  |_____/   \___/  |_| |_| |_|  \__,_| |_|     \__,_|  \__, |
 *                                                        __/ |
 *                                                       |___/                                                   
 *  DEVELOPED BY DOMARAY. May'21. 
 *  See www.domaray.com for more info.
 *  
 *  
 *  DESCRIPCION:
 *  
 *  The following circuit uses an ESP01 with additional electronic components to create a door sensor activated by a magnet.
 *  When the door is opened, the ESP01 wakes up from deep sleep mode, connectd to WiFi and sends an "open" MQTT message. Keeps awake until door is closed.
 *  When the door is closed, the ESP01 send a "close" MQTT message and goes into sleep mode to save power.
 *  
 *  
 *  PINOUT:
 *  
 *  GPIO_2: Short terminal of the reed sensor
 *  RESET: for wake up when door is opened
 *  CH_PD: connected to VCC
 */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"
#define mqtt_server "IP OF MQTT SERVER"
#define mqtt_user "MQTT USER"
#define mqtt_password "MQTT PASSWORD"
#define clientId "Entry_door"  // The ID must be unique in the local network
IPAddress ip(192, 168, 1, 120); 
IPAddress gateway(192, 168, 1, 1); 
IPAddress subnet(255, 255, 255, 0);
ADC_MODE(ADC_VCC);

#define topic_door_state "home/sensor/door_1/state"
#define topic_door_available "home/sensor/door_1/available"
#define topic_door_battery "home/sensor/door_1/battery"

#define reedPin 2


WiFiClient espClient;
PubSubClient client(espClient);


int batteryVoltage;
char batteryLevel[5];
bool reedSensor = true;  // NC Sensor --> TRUE = CLOSE; FALSE = OPEN  
bool doorOpen = true;
bool firstBoot = true;


void setup() {

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(reedPin, INPUT);

  while(doorOpen){
    
    if (!client.connected()) {
      reconnect();
    }
    client.loop();  
 
    reedSensor = digitalRead(reedPin);
    
      if(reedSensor && firstBoot){
        client.publish(topic_door_state, "ON");
        // Send battery level
        //batteryVoltage = map(ESP.getVcc(), 2300, 3000, 0, 100); // I convert into a percentage  2,3V --> 0% ,  3V (2xAAA Batteries) --> 100% 
        batteryVoltage = map(ESP.getVcc(), 2200, 2910, 0, 100); // I convert into a percentage  2,2V --> 0% ,  2,4V (2xAAA Rechargable Batteries) --> 100% || Doing a test, when the batteries are fully charged the value obtained is 2910.
        itoa(batteryVoltage, batteryLevel, 10);
        client.publish(topic_door_battery, batteryLevel);
        firstBoot = false;
      }
      if (!reedSensor) {
        client.publish(topic_door_state, "OFF"); 
        doorOpen = false;
      }
      
  }
  delay(200);
  ESP.deepSleep(0); 
}


void loop() {}




void setup_wifi() 
{
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) 
    {
      delay(50);
    }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientId, mqtt_user, mqtt_password)) {
      // Once connected, publish an announcement...
      client.publish(topic_door_available, "online");
    } else {
      // Wait 200 miliseconds before retrying
      delay(200);
    }
  }
}
