#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32-MOTOR";
const char* password = "12345678";

// Motor pinleri
const int motorPins[4] = {16, 17, 18, 19};
WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Motor PWM ayarları
  for (int i = 0; i < 4; i++) {
    ledcSetup(i, 5000, 8);              // kanal, frekans, çözünürlük
    ledcAttachPin(motorPins[i], i);     // pini kanala bağla
    ledcWrite(i, 0);                    // başlangıçta motorlar kapalı
  }

  // Access Point oluştur
  WiFi.softAP(ssid, password);
  Serial.println("Access Point kuruldu. IP adresi:");
  Serial.println(WiFi.softAPIP());

  // Ana sayfa (slider arayüzü)
  server.on("/", HTTP_GET, []() {
    String page = R"rawliteral(
      <!DOCTYPE html><html>
      <head><meta charset='utf-8'>
      <title>Motor Kontrol</title>
      <style>
        body { font-family: sans-serif; text-align: center; }
        input[type=range] { width: 80%; }
        h2 { margin-top: 30px; }
      </style>
      </head><body>
      <h1>ESP32 Motor Kontrol</h1>
      %SLIDERS%
      </body>
      <script>
        const sliders = document.querySelectorAll('input[type=range]');
        sliders.forEach(slider => {
          slider.addEventListener('input', e => {
            fetch(`/${slider.id}?value=${slider.value}`);
          });
        });
      </script>
      </html>
    )rawliteral";

    String sliders = "";
    for (int i = 0; i < 4; i++) {
      sliders += "<h2>Motor " + String(i+1) + "</h2>";
      sliders += "<input type='range' min='0' max='255' id='motor" + String(i) + "' value='0'><br>";
    }

    page.replace("%SLIDERS%", sliders);
    server.send(200, "text/html", page);
  });

  // Slider değerlerini al
  for (int i = 0; i < 4; i++) {
    String path = "/motor" + String(i);
    server.on(path.c_str(), HTTP_GET, [i]() {
      if (server.hasArg("value")) {
        int val = server.arg("value").toInt();
        val = constrain(val, 0, 255);
        ledcWrite(i, val);
        Serial.printf("Motor %d hızı: %d\n", i+1, val);
        server.send(200, "text/plain", "OK");
      }
    });
  }

  server.begin();
}

void loop() {
  server.handleClient();
}
