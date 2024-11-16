#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h> // For temperature/humidity sensor
#include <MAX30105.h> // For SpO2 and heart rate sensor
#include <Wire.h>

// Define pin for DHT sensor
#define DHTPIN 4          
#define DHTTYPE DHT11     
DHT dht(DHTPIN, DHTTYPE);

// Define pin for ECG
#define ECG_PIN 34 // Adjust pin as needed

MAX30105 particleSensor;

const char* ssid = "YOUR_SSID";         // Your WiFi SSID
const char* password = "YOUR_PASSWORD"; // Your WiFi Password

const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "YOUR_API_KEY"; // Your ThingSpeak Write API Key

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    // Connect to Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize DHT sensor
    dht.begin();

    // Initialize MAX30100 sensor
    if (!particleSensor.begin()) {
        Serial.println("MAX30100 not found. Check wiring.");
        while (1);
    }

    // Initialize ECG (AD8232)
    pinMode(ECG_PIN, INPUT);
}

void loop() {
    // Read temperature and humidity
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    // Read SpO2 and heart rate
    long irValue = particleSensor.getIR();
    int heartRate = 0;
    int spo2 = 0;
    if (irValue > 50000) {
        particleSensor.getHeartRateAndSpO2(&heartRate, &spo2);
    }

    // Read ECG signal
    int ecgValue = analogRead(ECG_PIN);

    // Print to Serial
    Serial.printf("Temp: %.2f C, Humidity: %.2f%%, Heart Rate: %d bpm, SpO2: %d%%, ECG: %d\n", 
                  temperature, humidity, heartRate, spo2, ecgValue);

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        String url = String(server) + "?api_key=" + apiKey + 
                     "&field1=" + temperature + 
                     "&field2=" + humidity + 
                     "&field3=" + heartRate + 
                     "&field4=" + spo2 + 
                     "&field5=" + ecgValue; // ECG data
        
        HTTPClient http;
        http.begin(url);
        int httpResponseCode = http.GET();
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.print("Error on sending GET: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }

    delay(30000); // Send data every 30 seconds
}
