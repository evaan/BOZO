#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);
String logs = "";
bool started = false;
DynamicJsonDocument doc(16384);

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin()) Serial.println("Failed to start LittleFS!");
  WiFi.softAP("EasternEdgeProfiler", "crazyasspassword73");
  WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/index.html");});
  server.on("/jquery.min.js", [](AsyncWebServerRequest *request) {request->send(LittleFS, "/jquery.min.js");});
  server.on("/dygraph.min.js", [](AsyncWebServerRequest *request) {request->send(LittleFS, "/dygraph.min.js");});
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
    started = true;
    request->send(200);
  });
  server.on("/started", HTTP_GET, [](AsyncWebServerRequest *request) {request->send((started ? 200 : 400)); started = false;});
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->params() != 1) {
      request->send(400);
      return;
    }
    //doc[0][0] = time
    //doc[0][1] = density
    deserializeJson(doc, request->getParam("data")->value());
    for (int i = 0; i < doc.size(); i++) {
      time_t time = (time_t)doc[i][0]/1000;
      char buffer[10];
      strftime(buffer, 10, "%H:%M:%S]", localtime(&time));
      if (logs.indexOf(buffer) == -1) 
        logs += "EX00 - " + String(buffer) + " - " + doc[i][0].as<String>() + " kPa - " + String(doc[i][1].as<int>()/9.81) +"m\n";
    }
    request->send(200);    
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String tmp;
    serializeJson(doc, tmp);
    request->send(200, tmp);
  });
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(200, logs);});
}

void loop() {}