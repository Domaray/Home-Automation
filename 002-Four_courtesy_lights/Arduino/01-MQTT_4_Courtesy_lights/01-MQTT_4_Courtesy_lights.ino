/*  
 *  COURTESY LIGHTS WITH NODEMCU + HOMEASSISTANT + MQTT 
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
 *  This program implements 4 lights activated by Home Assistant. 
 *  When the MQTT command sent by Home Assistant is received by the NodeMCU, it turns on the correspondng light
 *  
 *  
 *  
 *  PINOUT:
 *  
 *  Light 1: D0
 *  Light 2: D1
 *  Light 3: D2
 *  Light 4: D3
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"
#define mqtt_server "IP OF MQTT SERVER"
#define mqtt_user "MQTT USER"
#define mqtt_password "MQTT PASSWORD"
#define clientId "Courtesy_lights" 

#define light_topic_state_1 "home/lights/courtesy_1/state"
#define light_topic_command_1 "home/lights/courtesy_1/set"
#define light_topic_available_1 "home/lights/courtesy_1/available"
#define light_topic_state_2 "home/lights/courtesy_2/state"
#define light_topic_command_2 "home/lights/courtesy_2/set"
#define light_topic_available_2 "home/lights/courtesy_2/available"
#define light_topic_state_3 "home/lights/courtesy_3/state"
#define light_topic_command_3 "home/lights/courtesy_3/set"
#define light_topic_available_3 "home/lights/courtesy_3/available"
#define light_topic_state_4 "home/lights/courtesy_4/state"
#define light_topic_command_4 "home/lights/courtesy_4/set"
#define light_topic_available_4 "home/lights/courtesy_4/available"

#define light_pin_1 D0
#define light_pin_2 D1
#define light_pin_3 D2
#define light_pin_4 D3


WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

bool light_state_1 = false;
bool light_state_2 = false;
bool light_state_3 = false;
bool light_state_4 = false;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  pinMode(light_pin_1, OUTPUT);
  pinMode(light_pin_2, OUTPUT);
  pinMode(light_pin_3, OUTPUT);
  pinMode(light_pin_4, OUTPUT);
    
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 60000) {  // Every 60 seconds an availability message is sent and updates the server information.
    lastMsg = now;

    client.publish(light_topic_available_1, "online");
    client.publish(light_topic_available_2, "online");
    client.publish(light_topic_available_3, "online");
    client.publish(light_topic_available_4, "online");

    if (light_state_1) { client.publish(light_topic_state_1, "ON"); } else { client.publish(light_topic_state_1, "OFF"); }
    if (light_state_2) { client.publish(light_topic_state_2, "ON"); } else { client.publish(light_topic_state_2, "OFF"); }
    if (light_state_3) { client.publish(light_topic_state_3, "ON"); } else { client.publish(light_topic_state_3, "OFF"); }
    if (light_state_4) { client.publish(light_topic_state_4, "ON"); } else { client.publish(light_topic_state_4, "OFF"); }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  /* SWITCH 1 */
  if (String(light_topic_command_1).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      light_state_1 = true;
      client.publish(light_topic_state_1, "ON");
      digitalWrite(light_pin_1, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      light_state_1 = false;
      client.publish(light_topic_state_1, "OFF");
      digitalWrite(light_pin_1, HIGH);
    }
  }

  /* SWITCH 2 */
  if (String(light_topic_command_2).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      light_state_2 = true;
      client.publish(light_topic_state_2, "ON");
      digitalWrite(light_pin_2, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      light_state_2 = false;
      client.publish(light_topic_state_2, "OFF");
      digitalWrite(light_pin_2, HIGH);
    }
  }

  /* SWITCH 3 */
  if (String(light_topic_command_3).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      light_state_3 = true;
      client.publish(light_topic_state_3, "ON");
      digitalWrite(light_pin_3, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      light_state_3 = false;
      client.publish(light_topic_state_3, "OFF");
      digitalWrite(light_pin_3, HIGH);
    }
  }

  /* SWITCH 4 */
  if (String(light_topic_command_4).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      light_state_4 = true;
      client.publish(light_topic_state_4, "ON");
      digitalWrite(light_pin_4, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      light_state_4 = false;
      client.publish(light_topic_state_4, "OFF");
      digitalWrite(light_pin_4, HIGH);
    }
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(light_topic_available_1, "online");
      client.publish(light_topic_available_2, "online");
      client.publish(light_topic_available_3, "online");
      client.publish(light_topic_available_4, "online");
      // ... and resubscribe
      client.subscribe(light_topic_command_1);
      client.subscribe(light_topic_state_1);
      client.subscribe(light_topic_available_1);
      client.subscribe(light_topic_command_2);
      client.subscribe(light_topic_state_2);
      client.subscribe(light_topic_available_2);
      client.subscribe(light_topic_command_3);
      client.subscribe(light_topic_state_3);
      client.subscribe(light_topic_available_3);
      client.subscribe(light_topic_command_4);
      client.subscribe(light_topic_state_4);
      client.subscribe(light_topic_available_4);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
