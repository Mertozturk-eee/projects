#include <WiFi.h>

const char* ssid = "yOUR WİFİ NAME";   
const char* password = "YOUR WİFİ PASSWORD";

WiFiServer server(80);

const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  Serial.println("WiFi connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP Adresi: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("request!");

    String request = "";
    unsigned long timeout = millis() + 2000;

    while (millis() < timeout) {
      while (client.available()) {
        char c = client.read();
        request += c;
      }
      if (request.length() > 0) break;
      delay(10);
    }

    Serial.println("request content:");
    Serial.println(request);

    if (request.indexOf("/on") >= 0) {
      digitalWrite(ledPin, HIGH);
    } 
    else if (request.indexOf("/off") >= 0) {
      digitalWrite(ledPin, LOW);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.println("<html><body style='text-align:center;'>");
    client.println("<h1>ESP32 LED Control</h1>");
    client.print("<p>LED Situation: <strong>");
    client.print(digitalRead(ledPin) == HIGH ? "on" : "off");
    client.println("</strong></p>");
    client.println("<a href=\"/on\"><button style='font-size:24px;'>LED Aç</button></a><br><br>");
    client.println("<a href=\"/off\"><button style='font-size:24px;'>LED Kapat</button></a>");
    client.println("</body></html>");

    delay(1);
    client.stop();
  }
}
