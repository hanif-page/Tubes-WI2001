#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// --- WIFI AND SERVER CONFIGURATION ---
const char* ssid = "Hanif";                 // Your WiFi network SSID
const char* password = "wifigratis";          // Your WiFi network password
const char* serverIp = "172.20.10.3";    // <--- IMPORTANT: REPLACE WITH YOUR LAPTOP'S IP ADDRESS
const int serverPort = 3000;                // The port your Node.js server is running on

// --- DESK IDENTIFICATION ---
// These values identify this specific ESP32 sensor
const String buildingName = "GKU 1";
const int deskID = 1;

// --- SENSOR CONFIGURATION ---
const int trigPin = 5; // HC-SR04 Trigger pin
const int echoPin = 18; // HC-SR04 Echo pin
// Set a threshold distance (in cm). If an object is closer than this, the desk is "Occupied".
const float distanceThreshold = 95.0;

// --- TIMING ---
const unsigned long interval = 1000; // Time between requests in milliseconds (60 seconds)
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // --- Connect to WiFi ---
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n-------------------------------------");
  Serial.println("WiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Server Target: http://");
  Serial.print(serverIp);
  Serial.print(":");
  Serial.println(serverPort);
  Serial.println("-------------------------------------");
}

float readSensorDistance() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  // Speed of sound wave divided by two (go and back)
  float distance = duration * 0.034 / 2;

  return distance;
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if the interval has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // 1. Read the sensor and determine occupation status
    float distance = readSensorDistance();
    bool isOccupied = (distance < distanceThreshold && distance > 0);

    Serial.print("\nSensor distance: ");
    Serial.print(distance);
    Serial.print(" cm.  => isOccupied: ");
    Serial.println(isOccupied ? "true" : "false");

    // 2. Create JSON payload
    JSONVar postData;
    postData["buildingName"] = buildingName;
    postData["deskID"] = deskID;
    postData["isOccupied"] = isOccupied;
    
    String jsonString = JSON.stringify(postData);

    // 3. Send HTTP POST request
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String serverPath = "http://" + String(serverIp) + ":" + String(serverPort) + "/update-desk";
      
      Serial.print("Sending POST to: ");
      Serial.println(serverPath);
      Serial.print("Payload: ");
      Serial.println(jsonString);

      http.begin(serverPath);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response body: ");
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        Serial.println(http.errorToString(httpResponseCode));
      }

      http.end(); // Free resources
    } else {
      Serial.println("WiFi Disconnected. Cannot send data.");
    }
  }
}