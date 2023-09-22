#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

DynamicJsonDocument doc(16384);
bool started = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin("EasternEdgeProfiler", "crazyasspassword73");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
}

void loop() {
  if (started) {
    //profiler stuff i dont wanna do right now
    //here is the code for getting time and sensors and putting it into the json document
    if (millis() % 5000) {
      doc[0][0] = millis();
      doc[0][1] = 0; //GET THE PRESSURE HERE!!!!!
    }
    //here is the code to send back the json after the profiler resurfaces
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      Serial.println("Waiting to reconnect to control box");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
    }
    HTTPClient http;
    http.begin("http://192.168.0.1/submit");
    http.addHeader("Content-Type", "application/json");
    String tmp;
    serializeJson(doc, tmp);
    if (http.POST(tmp) == 200) Serial.println("Sent to control box!");
    http.end();
  } else if (millis()%1000 == 0) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin("http://192.168.0.1/started");
      if (http.GET() == 200) started = true;
      http.end();
    } else WiFi.reconnect();
  }
}