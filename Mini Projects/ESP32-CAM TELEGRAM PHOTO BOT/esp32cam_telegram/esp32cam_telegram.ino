#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "WİFİ";
const char* password = "PASS2";

#define BOTtoken "TOKEN"
#define CHAT_ID "ID"

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

bool continuousPhoto = false;
unsigned long lastPhotoTime = 0;
const unsigned long photoInterval = 500; 

int botRequestDelay = 1000;
unsigned long lastTimeBotRan = 0;


#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

void configInitCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed 0x%x\n", err);
    delay(1000);
    ESP.restart();
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + "\n";
      welcome += "/photo : Start continuous photo sending\n";
      welcome += "/stop : Stop photo sending\n";
      welcome += "/flash : Toggle flash LED\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    } 
    else if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
    }
    else if (text == "/photo") {
      continuousPhoto = true;
      bot.sendMessage(CHAT_ID, "Continuous photo sending started.", "");
    }
    else if (text == "/stop") {
      continuousPhoto = false;
      bot.sendMessage(CHAT_ID, "Photo sending stopped.", "");
    }
  }
}

void sendPhotoTelegram() {
  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  const char* myDomain = "api.telegram.org";
  if (clientTCP.connect(myDomain, 443)) {
    String head = String("--electroniclinic\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n") 
                + CHAT_ID 
                + "\r\n--electroniclinic\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--electroniclinic--\r\n";

    uint16_t totalLen = fb->len + head.length() + tail.length();

    clientTCP.println(String("POST /bot") + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=electroniclinic");
    clientTCP.println();
    clientTCP.print(head);

    clientTCP.write(fb->buf, fb->len);

    clientTCP.print(tail);
    esp_camera_fb_return(fb);

    delay(10);
    while(clientTCP.available()) clientTCP.read(); // Flush response
    clientTCP.stop();

    Serial.println("Photo sent!");
  } else {
    Serial.println("Connection failed");
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  configInitCamera();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages) handleNewMessages(numNewMessages);
    lastTimeBotRan = millis();
  }


  if (continuousPhoto && millis() - lastPhotoTime >= photoInterval) {
    sendPhotoTelegram();
    lastPhotoTime = millis();
  }
}
