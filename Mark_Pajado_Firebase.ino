#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <time.h>


const char* ntpServer = "time.google.com";
const long gmtOffset_sec = 8 * 3600; 
const int daylightOffset_sec = 0;

const char* ssid = "Joseph";
const char* password = "12345678910";
const char* firestoreUrl = "https://firestore.googleapis.com/v1/projects/josephinteg/databases/(default)/documents/jo?key=AIzaSyCYBWIBrO-oxZwnCOud1u2qZGxWKEi-N6w";


#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


const int photoresistorPin = 27;

void setup() {
 
  Serial.begin(9600);


  dht.begin();


  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
   
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

   
    if (isnan(temp) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    
    int photoresistorValue = analogRead(photoresistorPin);
    int daylight = (photoresistorValue > 512) ? 1 : 0; 


    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);

   
    HTTPClient http;
    http.begin(firestoreUrl);
    http.addHeader("Content-Type", "application/json");

    
    String jsonData = String("{") +
                      "\"fields\": {" +
                      "\"temperature\": {\"stringValue\": \"" + String(temp, 1) + "\"}," +
                      "\"humidity\": {\"stringValue\": \"" + String(humidity, 1) + "\"}," +
                      "\"daylight\": {\"stringValue\": \"" + String(daylight) + "\"}," +
                      "\"timestamp\": {\"stringValue\": \"" + String(timestamp) + "\"}" +
                      "}" +
                      "}";

   
    int httpResponseCode = http.POST(jsonData);

    
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + http.getString());
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    WiFi.reconnect();
  }

  
  delay(5000);
}