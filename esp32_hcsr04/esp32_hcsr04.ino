#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = ""; // CHANGE THIS
const char* password = ""; // AND THIS

// MQTT Broker details
const char* mqtt_server = ""; // CHANGE THIS to your server's IP
const int mqtt_port = 1883;
const char* mqtt_user = ""; // Leave empty if no authentication
const char* mqtt_password = ""; // Leave empty if no authentication
const char* mqtt_topic = "sensor/ultrasonic";

// HC-SR04 pins
const int trigPin = 5;  // D5
const int echoPin = 18; // D18

// Variables for sensor
long duration;
int distance;
bool objectDetected = false;
const int detectionThreshold = 20; // Distance in cm to consider an object detected

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

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
    
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void readSensor() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Set the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance (in cm)
  distance = duration * 0.034 / 2;
  
  // Determine if object is detected
  objectDetected = (distance <= detectionThreshold && distance > 0);
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm - Object detected: ");
  Serial.println(objectDetected ? "TRUE" : "FALSE");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize HC-SR04 pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  readSensor();
  
  // Publish the detection status
  if (objectDetected) {
    client.publish(mqtt_topic, "TRUE");
  } else {
    client.publish(mqtt_topic, "FALSE");
  }
  
  delay(30000); // Wait 30 seconds before next reading
}