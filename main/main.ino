/**********************************/
/*   ESP32 Code Serving JSON Data */
/**********************************/

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

// === DHT11 Setup ===
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === MQ135 Setup ===
#define MQ135PIN 34

// === GP2Y10 Setup ===
#define GP2Y10PIN 35
#define LED_PIN   13

// === Buzzer Setup ===
#define BUZZER_PIN 12

// === GPS Setup ===
HardwareSerial SerialGPS(1);   // Use UART1
static const int RXPin = 16;   // GPS TX -> ESP32 RX
static const int TXPin = 17;   // GPS RX -> ESP32 TX
TinyGPSPlus gps;

// === Wi-Fi Credentials ===
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// === Thresholds ===
float tempThreshold   = 33.0; 
float humidityThreshold = 55.0;
int mq135Threshold      = 300; 
int gp2y10Threshold     = 100; 

// === WebServer on port 80 ===
WebServer server(80);

// --------------------------------------------------------------------------
// Initialize sensors and pins
void initSensors() {
  dht.begin();
  pinMode(MQ135PIN, INPUT);
  pinMode(GP2Y10PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

// --------------------------------------------------------------------------
// Read GP2Y10 sensor value
float readGP2Y10() {
  digitalWrite(LED_PIN, LOW);      // Turn on the sensor LED
  delayMicroseconds(280);          // Wait for the sensor to stabilize
  int dustVal = analogRead(GP2Y10PIN);
  digitalWrite(LED_PIN, HIGH);     // Turn off the sensor LED
  delayMicroseconds(40);           // Wait for the sensor to cool

  // Convert to approximate dust density
  // (Adjust formula as needed for your sensor and ADC reference)
  return (dustVal * (5.0 / 4095.0)) * 1000.0;
}

// --------------------------------------------------------------------------
// Continuously read GPS data (call in loop)
void readGPSData() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }
}

// --------------------------------------------------------------------------
// Return JSON with sensor + GPS data
String getSensorDataJson() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();
  int mq135Value    = analogRead(MQ135PIN);
  float dustValue   = readGP2Y10();

  // GPS data
  double latitude  = gps.location.isValid() ? gps.location.lat() : 0.0;
  double longitude = gps.location.isValid() ? gps.location.lng() : 0.0;

  // Create JSON string
  // Example: {"temperature":24.5,"humidity":60,"mq135":350,"dust":120,"latitude":40.714,"longitude":-74.006}
  String jsonStr = "{";
  jsonStr += "\"temperature\":" + String(temperature) + ",";
  jsonStr += "\"humidity\":"    + String(humidity)    + ",";
  jsonStr += "\"mq135\":"       + String(mq135Value)  + ",";
  jsonStr += "\"dust\":"        + String(dustValue)   + ",";
  jsonStr += "\"latitude\":"    + String(latitude, 6) + ",";
  jsonStr += "\"longitude\":"   + String(longitude, 6);
  jsonStr += "}";

  return jsonStr;
}

// --------------------------------------------------------------------------
// Handle GET /data request
void handleSensorData() {
  String jsonResponse = getSensorDataJson();

  // Important for cross-origin requests (CORS):
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonResponse);
}

// --------------------------------------------------------------------------
// Check thresholds and beep buzzer if exceeded
void checkThresholds() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();
  int mq135Value    = analogRead(MQ135PIN);
  float dustValue   = readGP2Y10();

  if ((temperature > tempThreshold && humidity > humidityThreshold) ||
       mq135Value > mq135Threshold ||
       dustValue > gp2y10Threshold) {
    // Buzzer ON
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// --------------------------------------------------------------------------
// Connect to Wi-Fi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// --------------------------------------------------------------------------
// Setup web server routes
void startServer() {
  server.on("/data", handleSensorData);
  server.begin();
  Serial.println("Server started on port 80");
}

// --------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // GPS serial
  SerialGPS.begin(9600, SERIAL_8N1, RXPin, TXPin);

  connectToWiFi();
  initSensors();
  startServer();
}

// --------------------------------------------------------------------------
void loop() {
  server.handleClient();
  readGPSData();
  checkThresholds();
  delay(2000);
}
