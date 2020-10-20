
#include "c:\Users\jesse\Documents\Arduino\config.h"
// TODO: Configure your WiFi here

const char* WIFI_SSID = ssid;
const char* WIFI_PSK = password;

// We will use wifi
#include <WiFi.h>

// We will use SPIFFS and FS
#include <SPIFFS.h>
#include <FS.h>

// We use JSON as data format. Make sure to have the lib available
#include <ArduinoJson.h>

// Working with c++ strings
#include <string>

// Define the name of the directory for public files in the SPIFFS parition
#define DIR_PUBLIC "/public"

// We need to specify some content-type mapping, so the resources get delivered with the
// right content type and are displayed correctly in the browser
char contentTypes[][2][32] = {
  {".html", "text/html"},
  {".css",  "text/css"},
  {".js",   "application/javascript"},
  {".json", "application/json"},
  {".png",  "image/png"},
  {".jpg",  "image/jpg"},
  {"", ""}
};

// Includes for the server
#include <HTTPSServer.hpp>
#include <HTTPServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <util.hpp>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

void handleSPIFFS(HTTPRequest * req, HTTPResponse * res);
void handleGetUptime(HTTPRequest * req, HTTPResponse * res);
void handleGetEvents(HTTPRequest * req, HTTPResponse * res);
void handlePostEvent(HTTPRequest * req, HTTPResponse * res);
void handleDeleteEvent(HTTPRequest * req, HTTPResponse * res);
void handleRoot(HTTPRequest * req, HTTPResponse * res);

// We use the following struct to store GPIO events:
#define MAX_EVENTS 20
struct {
  // is this event used (events that have been run will be set to false)
  bool active;
  // when should it be run?
  unsigned long time;
  // which GPIO should be changed?
  int gpio;
  // and to which state?
  int state;
} events[MAX_EVENTS];

HTTPServer myServer = HTTPServer();

void init_web(){
  for(int i = 0; i < MAX_EVENTS; i++) {
    events[i].active = false;
    events[i].gpio = 0;
    events[i].state = LOW;
    events[i].time = 0;
  }

  // Connect to WiFi
  Serial.println("Setting up WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  // We create a node for the main page of the server, available via get
  ResourceNode * nodeRoot     = new ResourceNode("/", "GET", &handleRoot);


  Serial.println("Starting server...");
  myServer.start();
  if (myServer.isRunning()) {
    Serial.println("Server ready.");
  }
}

void do_web(){
  myServer.loop();
  // Here we handle the events
  unsigned long now = millis() / 1000;
  for (int i = 0; i < MAX_EVENTS; i++) {
    // Only handle active events:
    if (events[i].active) {
      // Only if the counter has recently been exceeded
      if (events[i].time < now) {
      // Apply the state change
      digitalWrite(events[i].gpio, events[i].state);

      // Deactivate the event so it doesn't fire again
      events[i].active = false;
      }
    }
  }
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->println("SUP");
}