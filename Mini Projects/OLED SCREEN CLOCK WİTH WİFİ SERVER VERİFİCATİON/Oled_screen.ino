#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const char* ssid = "YOUR WIFI NAME";
const char* password = "YOUR WIFI PASSWORD";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WebServer server(80);

int hours = 0;
int minutes = 0;
int second = 0;

unsigned long beforems = 0;
const unsigned long asecond = 1000;

void handleRoot() {
  String html = "<!DOCTYPE html><html><body style='text-align:center;font-family:sans-serif;'>";
  html += "<h1>ESP32 Clock Setup</h1>";
  html += "<form action='/settime' method='GET'>";
  html += "<label>Hours: <input type='number' name='h' min='0' max='23' value='" + String(hours) + "'></label><br><br>";
  html += "<label>Minutes: <input type='number' name='m' min='0' max='59' value='" + String(minutes) + "'></label><br><br>";
  html += "<label>Seconds: <input type='number' name='s' min='0' max='59' value='" + String(second) + "'></label><br><br>";
  html += "<input type='submit' value='Set Time' style='font-size:20px;padding:10px 20px;'>";
  html += "</form>";
  html += "<br><p>Current time: <strong>" + String(hours) + ":" + String(minutes) + ":" + String(second) + "</strong></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (server.hasArg("h")) hours   = server.arg("h").toInt();
  if (server.hasArg("m")) minutes = server.arg("m").toInt();
  if (server.hasArg("s")) second  = server.arg("s").toInt();
  beforems = millis();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("NO SCREEN"));
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/settime", handleSetTime);
  server.begin();
}

void loop() {
  server.handleClient();

  unsigned long nowms = millis();
  if (nowms - beforems >= asecond) {
    beforems = nowms;
    second++;
    if (second >= 60) { second = 0; minutes++; }
    if (minutes >= 60) { minutes = 0; hours++;  }
    if (hours >= 24)   { hours = 0;              }
  }

  char buffer[20];
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, second);

  display.clearDisplay();
  display.setCursor(20, 28);
  display.print(buffer);
  display.display();

  delay(30);
}