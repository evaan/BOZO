#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
String logs = "";
int unixTime = 0;

String getTime() {
  time_t time = (time_t)(unixTime + millis())/1000;
  char buffer[10];
  strftime(buffer, 10, "[%H:%M:%S]", gmtime(&time));
  return buffer;
}

void start(void* parameter) {
  AsyncWebServerRequest *request = (AsyncWebServerRequest*)parameter;
  if (unixTime = 0) {request->send_P(420, "text/plain", "Refresh needed."); return;}
  request->send_P(200, "text/plain", "Success!");
  logs+=getTime() + " Starting profile...";
  //do the profile thing
  sleep(10); //idk change this
  logs+=getTime() + " Resurfacing profiler...";
  //do the profile thing that make it go back up
  sleep(10); //change it once again use a sensor or somethin
  logs+=getTime() + " Profile complete!";
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin()) Serial.println("Failed to start LittleFS!");
  WiFi.softAP("EasternEdgeProfiler", "crazyasspassword73");
  WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/index.html", "text/html");});
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(LittleFS, "/jquery.min.js");});
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {xTaskCreatePinnedToCore(start, "start", 1024, request, 0, NULL, 0);});
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {unixTime = request->getParam("time")->value().toInt() - millis(); request->send_P(200, "text/plain", "Time set!");});
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {request->send_P(200, "text/plain", logs.c_str());});
}

void loop() {}