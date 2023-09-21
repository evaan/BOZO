#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
String logs = "";
int status = 0;

String getTime() {
  time_t time = (time_t)(millis())/1000;
  char buffer[10];
  strftime(buffer, 10, "[%H:%M:%S]", localtime(&time));
  return buffer;
}

void start(AsyncWebServerRequest *request) {
  if (status == 0) {
    logs += "Profiler started at " + getTime() + "\n";
  }
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin()) Serial.println("Failed to start LittleFS!");
  WiFi.softAP("EasternEdgeProfiler", "crazyasspassword73");
  WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/index.html", "text/html");});
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/jquery.min.js");});
  server.on("/dygraph.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/dygraph.min.js");});
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {start(request);});
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {request->send_P(200, "text/plain", logs.c_str());});
}

void loop() {
  //status 0: idle
  //status 1: going down
  //status 2 going up
  switch(status) {
    case 1:
      //if pressure/depth is at a certain point then switch to status 2 and change servo/whatever controls buoyancy engine
      if (millis() % 5000) logs += "EX99 - " + getTime() + " - XX.X KPA - XX.X M\n";
      break;
    case 2:
      //same as first case
      if (millis() % 5000) logs += "EX99 - " + getTime() + " - XX.X KPA - XX.X M\n";
      break;
    default:
      return;
  }
}