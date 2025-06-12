#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

//======================================================================
// --- CONFIGURATION ---
//======================================================================

// --- WiFi & Server ---
// Set to 'true' to use a standard WPA2 network (SSID & password)
// Set to 'false' to use a WPA2-Enterprise network (e.g., University WiFi)
const bool USE_WPA2_PERSONAL = false;

// -- WPA2-Personal Credentials (like a phone hotspot) --
const char* WIFI_SSID_PERSONAL = "Hanif";
const char* WIFI_PASSWORD_PERSONAL = "wifigratis";

// -- WPA2-Enterprise Credentials (like Eduroam/University WiFi) --
#include "esp_wpa2.h" // Required for WPA2-Enterprise
const char* WIFI_SSID_ENTERPRISE = "ITB Hotspot";
const char* EAP_IDENTITY = "13224087@mahasiswa.itb.ac.id"; // Your full university email
const char* EAP_PASSWORD = ""; // Your email password

// -- Server --
const char* SERVER_IP = "10.97.59.226"; // if phone hotspot Hanif, then use 172.20.10.3
const int SERVER_PORT = 3000;
const char* SERVER_ENDPOINT = "/update-desk";

// --- Desk Identification ---
const String BUILDING_NAME = "GKU 1";
const int DESK_ID = 1;

// --- Sensor Configuration ---
const int TRIG_PIN = 5;  // HC-SR04 Trigger pin
const int ECHO_PIN = 18; // HC-SR04 Echo pin
const int TRIG2_PIN = 4;
const int ECHO2_PIN = 16;
const float DISTANCE_THRESHOLD_CM = 95.0; // If an object is closer than this, the desk is "Occupied"

// --- Timing ---
const unsigned long UPDATE_INTERVAL_MS = 1000; // Time between sensor reads and POST requests
unsigned long previousMillis = 0;


//======================================================================
// --- FUNCTION DECLARATIONS ---
//======================================================================
void connectToWiFi();
float readSensorDistance();
void sendDeskData(bool isOccupied);


//======================================================================
// --- SETUP ---
//======================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial to be ready

  // Initialize sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(TRIG2_PIN, OUTPUT);
  pinMode(ECHO2_PIN, INPUT);

  connectToWiFi();
}


//======================================================================
// --- MAIN LOOP ---
//======================================================================
void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking timer to control update frequency
  if (currentMillis - previousMillis >= UPDATE_INTERVAL_MS) {
    previousMillis = currentMillis;

    // Ensure WiFi is still connected before proceeding
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi Disconnected. Attempting to reconnect...");
      connectToWiFi();
      return; // Skip this iteration and try again after reconnecting
    }

    // 1. Read sensor and determine occupation status
    float distance = readSensorDistance();
    float distance2 = readSensor2Distance();
    bool isOccupied = ((distance < DISTANCE_THRESHOLD_CM && distance > 0) || (distance2 < DISTANCE_THRESHOLD_CM && distance2 > 0));

    Serial.print("\nSensor 1 distance: ");
    Serial.print(distance);
    Serial.print(" cm.");
    Serial.print("\nSensor 2 distance: ");
    Serial.print(distance2);
    Serial.print(" cm.\nisOccupied: ");
    Serial.println(isOccupied ? "true" : "false");

    // 2. Send the data to the server
    sendDeskData(isOccupied);
  }
}


//======================================================================
// --- FUNCTIONS ---
//======================================================================

/**
 * @brief Connects the ESP32 to a WiFi network.
 * It uses the configuration flags at the top of the file to determine
 * whether to use WPA2-Personal (SSID/Password) or WPA2-Enterprise (EAP).
 */
void connectToWiFi() {
  WiFi.disconnect(true); // Disconnect from any previous network
  delay(100);

  Serial.println("\n-------------------------------------");
  Serial.print("Connecting to WiFi: ");

  if (USE_WPA2_PERSONAL) {
    Serial.println(WIFI_SSID_PERSONAL);
    WiFi.begin(WIFI_SSID_PERSONAL, WIFI_PASSWORD_PERSONAL);
  } else {
    Serial.println(WIFI_SSID_ENTERPRISE);
    // Note: For EAP, the username is often the same as the identity.
    // If your network requires a different username, change EAP_IDENTITY in the line below.
    WiFi.begin(WIFI_SSID_ENTERPRISE, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_IDENTITY, EAP_PASSWORD);
  }

  // Wait for connection
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retries > 30) { // Timeout after ~15 seconds
        Serial.println("\nFailed to connect. Please check credentials and network. Retrying...");
        retries = 0;
    }
  }

  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Server Target: http://");
  Serial.print(SERVER_IP);
  Serial.print(":");
  Serial.println(SERVER_PORT);
  Serial.println("-------------------------------------");
}

/**
 * @brief Reads the distance from the HC-SR04 ultrasonic sensor.
 * @return The measured distance in centimeters.
 */
float readSensorDistance() {
  // Clear the trigger pin to ensure a clean pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echoPin, which returns the sound wave's travel time in microseconds
  // Set a timeout to avoid getting stuck if the echo is not received
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout, good for up to ~4m

  // Calculate the distance in centimeters
  // Formula: (duration * speed_of_sound) / 2 (for the round trip)
  // Speed of sound is approx. 0.034 cm/microsecond
  float distance = duration * 0.034 / 2.0;

  return distance;
}

float readSensor2Distance() {
  // Clear the trigger pin to ensure a clean pulse
  digitalWrite(TRIG2_PIN, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(TRIG2_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG2_PIN, LOW);

  // Read the echoPin, which returns the sound wave's travel time in microseconds
  // Set a timeout to avoid getting stuck if the echo is not received
  long duration = pulseIn(ECHO2_PIN, HIGH, 25000); // 25ms timeout, good for up to ~4m

  // Calculate the distance in centimeters
  // Formula: (duration * speed_of_sound) / 2 (for the round trip)
  // Speed of sound is approx. 0.034 cm/microsecond
  float distance = duration * 0.034 / 2.0;

  return distance;
}

/**
 * @brief Sends the desk's occupation status to the server via an HTTP POST request.
 * @param isOccupied The boolean status of the desk (true for occupied, false for vacant).
 */
void sendDeskData(bool isOccupied) {
  HTTPClient http;
  String serverPath = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + String(SERVER_ENDPOINT);

  // 1. Create JSON payload
  JSONVar postData;
  postData["buildingName"] = BUILDING_NAME;
  postData["deskID"] = DESK_ID;
  postData["isOccupied"] = isOccupied;
  String jsonString = JSON.stringify(postData);

  Serial.print("Sending POST to: ");
  Serial.println(serverPath);
  Serial.print("Payload: ");
  Serial.println(jsonString);

  // 2. Send the request
  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonString);

  // 3. Process the response
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response body: ");
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end(); // Free up resources
}
