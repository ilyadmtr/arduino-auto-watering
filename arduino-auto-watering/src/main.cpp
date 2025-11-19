#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>


// Moisture Sensor
const int MOISTURE_SENSOR_PIN = A0; // Analog pin for moisture sensor
const int MOISTURE_POWER_PIN = 7;            // Digital pin to power the sensor
const int DRY_VALUE = 1023;          // Value when the sensor is dry
const int WET_VALUE = 300;             // Value when the sensor is wet

// Pump
const int PUMP_POWER_PIN = 4;              // Digital pin to control the pump

//WiFi Module
const int ESP_RX_PIN = 2;         // RX pin of ESP8266
const int ESP_TX_PIN = 3;         // TX pin of ESP8266
#define ESP_BAUD_RATE 115200     // Baud rate for ESP8266 communication

SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); // RX, TX

//WiFi Credentials
const char* WIFI_SSID = "TEST";
const char* WIFI_PASSWORD = "TEST";

//Backend Server
const char* SERVER_IP = "192.168.1.10"; // Backend server IP
const int SERVER_PORT = 8080;        // Backend server port
const char* API_PATH_DATA = "/api/data"; // Endpoint to send sensor data
const char* API_PATH_CMD = "/api/command"; // Endpoint to receive commands

int readMoisturePercent() {
  digitalWrite(MOISTURE_POWER_PIN, HIGH); // Power the moisture sensor
  delay(300); // Allow sensor to stabilize
  int sensorValue = analogRead(MOISTURE_SENSOR_PIN);
  digitalWrite(MOISTURE_POWER_PIN, LOW); // Turn off the sensor to save power
  sensorValue = constrain(sensorValue, WET_VALUE, DRY_VALUE);
  int moisturePercent = map(sensorValue, WET_VALUE, DRY_VALUE, 100, 0);
  return moisturePercent;
}

void controlPump(bool state) {
  digitalWrite(PUMP_POWER_PIN, state ? LOW : HIGH);
}

void sendATCommand(String command, const int timeout){
  espSerial.println(command);
  long int time = millis();
  while( (time + timeout) > millis()){
    while(espSerial.available()){
      char c = espSerial.read();
      Serial.write(c);
    }
  }
}

void sendDataToServer(int moisture){
  // Prepare JSON payload
  String jsonPayload = "{\"moisture\":" + String(moisture) + "}";

  // Prepare HTTP POST request
  String postRequest = "POST " + String(API_PATH_DATA) + " HTTP/1.1\r\nHost: " + String(SERVER_IP) + ":" + String(SERVER_PORT) + "\r\nContent-Type: application/json\r\nContent-Length: " + String(jsonPayload.length()) + "\r\n\r\n" + jsonPayload;

  // Establish TCP connection
  sendATCommand("AT+CIPSTART=\"TCP\",\"" + String(SERVER_IP) + "\"," + String(SERVER_PORT), 2000);

  // Send POST request
  sendATCommand("AT+CIPSEND=" + String(postRequest.length()), 2000);
  espSerial.readStringUntil('>'); // Wait for '>' prompt
  espSerial.print(postRequest);
  
  // Close connection
  sendATCommand("AT+CIPCLOSE", 2000);
}

String getServerCommand(){
  String command = "NONE";

  // Establish TCP connection
  sendATCommand("AT+CIPSTART=\"TCP\",\"" + String(SERVER_IP) + "\"," + String(SERVER_PORT), 2000);

  // Prepare GET request
  String getRequest = "GET " + String(API_PATH_CMD) + " HTTP/1.1\r\nHost: " + String(SERVER_IP) + ":" + String(SERVER_PORT) + "\r\n\r\n";

  // Send GET request
  sendATCommand("AT+CIPSEND=" + String(getRequest.length()), 2000);
  espSerial.readStringUntil('>'); // Wait for '>' prompt
  espSerial.print(getRequest);

  // Check server response
  command = checkServerResponse();

  // Close connection
  sendATCommand("AT+CIPCLOSE", 2000);

  return command;
}

String checkServerResponse(){
  // Read response from ESP8266
  String response = "";
  long int time = millis();

  while (millis() - time < 5000) {
    if (espSerial.available()) {
      response += (char)espSerial.read();
      delay(1); 
    }
  }

  // Parse HTTP response to extract JSON payload
  int jsonStart = response.indexOf("\r\n\r\n");
  
  // Check if the end of headers was found
  if (jsonStart == -1) {
    Serial.println("ERROR: Could not find end of HTTP headers.");
    return "NONE"; 
  }
  
  // Extract JSON payload
  String jsonPayload = response.substring(jsonStart + 4);

  // Parse JSON payload
  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);

  // Check for parsing errors
  if (error) {
    Serial.print("JSON Parsing failed: ");
    Serial.println(error.f_str());
    return "NONE"; // Ошибка парсинга
  }

  // Extract command from JSON
  if (doc.containsKey("action")) {
    String action = doc["action"].as<String>();
    Serial.print("Server Command Received: ");
    Serial.println(action);
    return action;
  }
  
  return "NONE";
}

void setup() {
  pinMode(MOISTURE_POWER_PIN, OUTPUT);
  digitalWrite(MOISTURE_POWER_PIN, LOW); // Ensure sensor is off initially

  pinMode(PUMP_POWER_PIN, OUTPUT);
  digitalWrite(PUMP_POWER_PIN, HIGH); // Ensure pump is off initially

  pinMode(MOISTURE_SENSOR_PIN, INPUT);

  Serial.begin(9600);
  espSerial.begin(ESP_BAUD_RATE);
}

void loop() {
  delay(30000); // Wait between readings
  
  int moisture = readMoisturePercent();
  Serial.print("Soil Moisture: ");
  Serial.print(moisture);
  Serial.println("%");

  sendDataToServer(moisture);

  String command = getServerCommand();

  if(command == "WATER_ON"){
    Serial.println("Activating Pump");
    controlPump(true);
  } else if(command == "WATER_OFF"){
    Serial.println("Deactivating Pump");
    controlPump(false);
  } else {
    Serial.println("No valid command received.");
  }
}


