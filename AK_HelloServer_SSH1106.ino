#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "Password"
#endif

const char* ssid = "BATCAVE";
const char* password = "arduinoIsFun";

ESP8266WebServer server(80);

#define LEDON 0
#define LEDOFF 1
const int led = D4;
int ledVal = LEDOFF;

/************************************************************************
 * SH1106 includes and variables
 ************************************************************************/
 #include "AK_SH1106.h"
 const uint8_t SH1106_addr = 0x3c; // I2C address of display
 int SDApin = D1; //D2;
 int SCLpin = D2; //D1;

/* 
 *  Display is 128 x 64
 *  Rows 0-15 (top) are for status of battery, antenna, etc
 *  Rows 16-31 are for song status (progress bar)
 *  Rows 32-47 are for scrolling text
 *  Rows 48-63 are for whatever
 *  
 *  For menu, use rows 16-63
 */

 const int16_t LINE1 = 0; // 0 to LINE2-1
 const int16_t LINE2 = 16; // 0 to LINE3-1
 const int16_t LINE3 = 32; // 0 to LINE4-1
 const int16_t LINE4 = 48; // 0 to LINE5-1

 // Initialize the OLED display using Wire library
 SH1106 display(SH1106_addr, SDApin, SCLpin, GEOMETRY_128_64);  // address, SDApin, SCLpin


void handleRoot() {
  digitalWrite(led, LEDON);
  server.send(200, "text/plain", "hello from esp8266!");
  delay(10);
  digitalWrite(led, LEDOFF);
}

void handleLED() {
  String res = "<html><body>";
  res += "Click here to toggle LED";
  res += "<form action=\"/toggle\"><input type=\"submit\" value=\"Click Me\"></input>";
  res += "</body></html>";
   server.send(200, "text/html", res);
}

void handleToggle() {
      if (ledVal == LEDON)
      ledVal = LEDOFF;
    else 
      ledVal = LEDON;
    digitalWrite(led, ledVal);
    handleLED();
}

void handleNotFound() {
  digitalWrite(led, LEDON);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, LEDOFF);
}

void setup(void) {

  display.init();
  display.setContrast(255);  // maximum contrast
  display.setColor(WHITE);
  display.flipScreenVertically();
  display.clear();
  //ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  pinMode(led, OUTPUT);
  digitalWrite(led, LEDON);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  int cursorPos = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (ledVal == LEDON)
      ledVal = LEDOFF;
    else 
      ledVal = LEDON;
    digitalWrite(led, ledVal);
    display.drawString(cursorPos, LINE1, ".");
    display.display();
    if (++cursorPos > 127)
      cursorPos = 0;
  }
  
  String connect_msg = "Connected to " + String(ssid);
  display.drawString(0, LINE1, connect_msg);
  connect_msg = "IP address: " + WiFi.localIP().toString();
  display.drawString(0, LINE2, connect_msg);
  display.display();
  
  if (MDNS.begin("esp8266")) {
    delay(1000);
  }

  server.on("/", handleRoot);
  server.on("/led", handleLED);
  server.on("/toggle", handleToggle);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  display.drawString(0, LINE3, "HTTP server started");
  display.display();
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
