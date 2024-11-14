#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

#define DHT_PIN 15         // DHT11 data pin
#define DHT_TYPE DHT11      // Sensor type: DHT11
DHT dht(DHT_PIN, DHT_TYPE);


// WiFi and MQTT settings
const char* wifiSSID = "cawang78/2";
const char* wifiPassword = "28012004";
const char* mqttCloud = "thingsboard.cloud";
const char* mqttToken = "1Wnemxw1VFEIbXSdtrlh";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi network
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting WiFi connection...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT server
  mqttClient.setServer(mqttCloud, 1883);
}

void connectToMQTT() {
  // Repeatedly try to connect to the ThingsBoard if disconnected
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32Client", mqttToken, NULL)) {
      Serial.println("Connected to MQTT Cloud");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  // Ensure connection to MQTT server
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();

  // Read temperature and humidity data from DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Sensor read failed!");
    return;
  }

  // Display temperature and humidity
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %");

  // Send telemetry data to ThingsBoard
  if (mqttClient.connected()) {
    String jsonPayload = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
    mqttClient.publish("v1/devices/me/telemetry", jsonPayload.c_str());
    Serial.println("Telemetry sent: " + jsonPayload);
  }

  delay(5000);  // Wait for 5 seconds before next reading
}
