// Include Library
#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h> // Library Publish/Subcribe MQTT
#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>
#include <ssl_client.h>
#include "MQ135.h"

#define DHTPIN 4
#define DHTTYPE DHT11
#define SLAVE_ID "0" // Unique ID for each slave //Master
#define PIN_MQ135 35

// Set Wifi
const char* ssid = "DEDE";
const char* password = "mautahuaja";

// Set MQTT Broker Connection Details
const char* mqtt_server = "xxx";
const char* mqtt_username = "xxx";
const char* mqtt_password = "xxx";
const int mqtt_port = 8883;
const char* mqtt_topic = "xxx";

// Set Client
WiFiClientSecure espClient;
PubSubClient client(espClient);

//Sensor
DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135(PIN_MQ135);

// MQUnifiedsensor MQ135(board, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

// Set Root Certificate
static const char *root_ca PROGMEM = R"EOF(
xxx
)EOF";

// Fungsi Wifi Setup
bool connectWiFi() {
  Serial.println();
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  // Connect to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    Serial.println(F("Connecting to WiFi..."));
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    return true;
  } else {
    Serial.println("Failed to connect to WiFi");
    return false;
  }
}

// Fungsi ke Server Reconnect
void reconnect() {

  // Loop Hingga Reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Membuat Random Client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);

    // Mencoba Untuk Connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {

      // Connection Gagal
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      // Tunggu 5 Detik Dan Coba Lagi
      delay(5000);
    }
  }
}

// Fungsi Publish
void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}

// Setup
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Setup Connection
  if(connectWiFi()) client.setServer(mqtt_server, mqtt_port);

  // Setup Connection Security
  #ifdef ESP8266
    espClient.setInsecure();
  #else
    espClient.setCACert(root_ca);
  #endif
}

String btData() {
  String btData;
  //Untuk nerima data dari slave menggunakan bluetooth
  
  return btData;
}

// Loop
void loop() {
  // Cek Apabila Client Connected
  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    reconnect();
  }
  client.loop();

  int h = dht.readHumidity();
  int t = dht.readTemperature();
  // float ppm = mq135.getPPM();
  float mq135_value = analogRead(PIN_MQ135);
  float resistance = ((1023./(float)mq135_value) - 1.)*10.0;
  float rzero = resistance * pow((415.58/116.6020682), (1./2.769034857));
  float ppm = 116.6020682 * pow((resistance/rzero), -2.769034857);

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  String sensorData = "{\"id\":\"" + String(SLAVE_ID) + "\",\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + ",\"ppm\":" + String(ppm) + "}\n";

  //Publish data master ke MQTT
  if(WiFi.status() == WL_CONNECTED) publishMessage(mqtt_topic, sensorData, true);
  Serial.println(sensorData);

  String data = btData();

  delay(10000);
}
