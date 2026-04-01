#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <ESP32Servo.h>
#include <WebServer.h>
#include "esp_http_server.h"


const char* ssid = "Your wifi";
const char* password = "your password";

// ====== Servo pins ======
#define PAN_PIN 13
#define TILT_PIN 14
Servo panServo;
Servo tiltServo;
int panAngle = 90;
int tiltAngle = 90;

// ====== LED pins ======
#define LED_PIN 4  
bool ledState = false;

// ====== Cam pins (AI Thinker) ======
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WebServer server(80);

// ====== Servo function ======
void setServo(Servo &servo, int angle) {
  angle = constrain(angle, 0, 180);
  servo.write(angle);
}

// ====== Servo control end ======
void handleServo() {
  if (server.hasArg("pan")) {
    panAngle = server.arg("pan").toInt();
    setServo(panServo, panAngle);
  }
  if (server.hasArg("tilt")) {
    tiltAngle = server.arg("tilt").toInt();
    setServo(tiltServo, tiltAngle);
  }
  Serial.printf("Servo → pan:%d tilt:%d\n", panAngle, tiltAngle);
  server.send(200, "text/plain", "OK");
}

// ====== LED control endpoint ======
void handleLed() {
  if (server.hasArg("state")) {
    int state = server.arg("state").toInt();
    ledState = (state == 1);
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    Serial.printf("LED situation: %s\n", ledState ? "on" : "off");
    server.send(200, "text/plain", ledState ? "LED ON" : "LED OFF");
  } else {
    server.send(400, "text/plain", "missing parametre: state");
  }
}

// ====== camera stream service ======
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
  if(res != ESP_OK) return res;

  while(true){
    fb = esp_camera_fb_get();
    if(!fb) continue;
    httpd_resp_send_chunk(req, "--frame\r\n", strlen("--frame\r\n"));
    httpd_resp_send_chunk(req, "Content-Type: image/jpeg\r\n\r\n", strlen("Content-Type: image/jpeg\r\n\r\n"));
    httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    httpd_resp_send_chunk(req, "\r\n", strlen("\r\n"));
    esp_camera_fb_return(fb);
  }
  return res;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  httpd_handle_t stream_httpd = NULL;
  httpd_uri_t stream_uri = { .uri="/", .method=HTTP_GET, .handler=stream_handler, .user_ctx=NULL };
  if(httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Servolar
  panServo.attach(PAN_PIN);
  tiltServo.attach(TILT_PIN);
  setServo(panServo, panAngle);
  setServo(tiltServo, tiltAngle);

  // Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("WiFi Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  // Kamera start
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 15;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Kamera did not start!");
    return;
  }

  startCameraServer();

  // Endpoints save
  server.on("/servo", handleServo);
  server.on("/led", handleLed);
  server.begin();
  Serial.println("Server ready!");
}

void loop() {
  server.handleClient();
}
