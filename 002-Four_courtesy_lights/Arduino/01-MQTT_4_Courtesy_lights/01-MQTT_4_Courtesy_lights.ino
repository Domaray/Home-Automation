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
 *  
 *  
 *  
 *  PINOUT:
 *  
 *  
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"
#define mqtt_server "IP OF MQTT SERVER"
#define mqtt_user "MQTT USER"
#define mqtt_password "MQTT PASSWORD"

#define switch_topic_state_1 "home/lights/cortesia_1/state"
#define switch_topic_command_1 "home/lights/cortesia_1/set"
#define switch_topic_available_1 "home/lights/cortesia_1/available"
#define switch_topic_state_2 "home/lights/cortesia_2/state"
#define switch_topic_command_2 "home/lights/cortesia_2/set"
#define switch_topic_available_2 "home/lights/cortesia_2/available"
#define switch_topic_state_3 "home/lights/cortesia_3/state"
#define switch_topic_command_3 "home/lights/cortesia_3/set"
#define switch_topic_available_3 "home/lights/cortesia_3/available"
#define switch_topic_state_4 "home/lights/cortesia_4/state"
#define switch_topic_command_4 "home/lights/cortesia_4/set"
#define switch_topic_available_4 "home/lights/cortesia_4/available"

#define switch_pin_1 16
#define switch_pin_2 5
#define switch_pin_3 4
#define switch_pin_4 0


WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

bool switch_state_1 = false;
bool switch_state_2 = false;
bool switch_state_3 = false;
bool switch_state_4 = false;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(switch_pin_1, OUTPUT);
  pinMode(switch_pin_2, OUTPUT);
  pinMode(switch_pin_3, OUTPUT);
  pinMode(switch_pin_4, OUTPUT);
    
}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 60000) {  // Every 60 seconds an availability message is sent and update the server information.
    lastMsg = now;

    client.publish(switch_topic_available_1, "online");
    client.publish(switch_topic_available_2, "online");
    client.publish(switch_topic_available_3, "online");
    client.publish(switch_topic_available_4, "online");

    if (switch_state_1) { client.publish(switch_topic_state_1, "ON"); } else { client.publish(switch_topic_state_1, "OFF"); }
    if (switch_state_2) { client.publish(switch_topic_state_2, "ON"); } else { client.publish(switch_topic_state_2, "OFF"); }
    if (switch_state_3) { client.publish(switch_topic_state_3, "ON"); } else { client.publish(switch_topic_state_3, "OFF"); }
    if (switch_state_4) { client.publish(switch_topic_state_4, "ON"); } else { client.publish(switch_topic_state_4, "OFF"); }
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
  if (String(switch_topic_command_1).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      switch_state_1 = true;
      client.publish(switch_topic_state_1, "ON");
      digitalWrite(switch_pin_1, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      switch_state_1 = false;
      client.publish(switch_topic_state_1, "OFF");
      digitalWrite(switch_pin_1, HIGH);
    }
  }

  /* SWITCH 2 */
  if (String(switch_topic_command_2).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      switch_state_2 = true;
      client.publish(switch_topic_state_2, "ON");
      digitalWrite(switch_pin_2, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      switch_state_2 = false;
      client.publish(switch_topic_state_2, "OFF");
      digitalWrite(switch_pin_2, HIGH);
    }
  }

  /* SWITCH 3 */
  if (String(switch_topic_command_3).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      switch_state_3 = true;
      client.publish(switch_topic_state_3, "ON");
      digitalWrite(switch_pin_3, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      switch_state_3 = false;
      client.publish(switch_topic_state_3, "OFF");
      digitalWrite(switch_pin_3, HIGH);
    }
  }

  /* SWITCH 4 */
  if (String(switch_topic_command_4).equals(topic)){
    if (strncmp((char *)payload, "ON", length) == 0){
      switch_state_4 = true;
      client.publish(switch_topic_state_4, "ON");
      digitalWrite(switch_pin_4, LOW);
    } if (strncmp((char *)payload, "OFF", length) == 0){
      switch_state_4 = false;
      client.publish(switch_topic_state_4, "OFF");
      digitalWrite(switch_pin_4, HIGH);
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
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(switch_topic_available_1, "online");
      client.publish(switch_topic_available_2, "online");
      client.publish(switch_topic_available_3, "online");
      client.publish(switch_topic_available_4, "online");
      // ... and resubscribe
      client.subscribe(switch_topic_command_1);
      client.subscribe(switch_topic_state_1);
      client.subscribe(switch_topic_available_1);
      client.subscribe(switch_topic_command_2);
      client.subscribe(switch_topic_state_2);
      client.subscribe(switch_topic_available_2);
      client.subscribe(switch_topic_command_3);
      client.subscribe(switch_topic_state_3);
      client.subscribe(switch_topic_available_3);
      client.subscribe(switch_topic_command_4);
      client.subscribe(switch_topic_state_4);
      client.subscribe(switch_topic_available_4);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
