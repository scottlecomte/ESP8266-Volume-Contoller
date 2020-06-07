//ESP32 Code

//* This code is for a line level Wi-Fi volume controller to be used with Home Assiatant *//

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>

// Update these variables.

const char* ssid = "........."; // Your SSID 
const char* password = "........."; // Your Wi-Fi password
const char* mqtt_server = "..........."; // Your MQTT broker
const char* mqtt_topic = "............"; // You can leave this topic or change if you would like If you change, you will need to update your Home Assistant automation topic
const char* mqtt_client_id = ".............."; // Name for this device on your MQTT network
int ChipSelect = 5; // SS or CS Pin (GPIO) on your board. On the ESP32 DEVKIT C V4, the CS pin in GPIO5

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

   payload[length] = '\0'; // Add a NULL to the end of the char* to make it a string.
   int volume = atoi((char *)payload);
   
  Serial.print(volume);
  writeMCP4241(0,volume);
  writeMCP4241(1,volume);
}

void writeMCP4241(byte address, char value) {
  digitalWrite(ChipSelect, LOW); 
  SPI.transfer(address << 4);
  SPI.transfer(value & 127);
  digitalWrite(ChipSelect, HIGH);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = mqtt_client_id;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(ChipSelect, OUTPUT); 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println(SS);
  Serial.println(MISO);
  Serial.println(SCK);
  SPI.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
