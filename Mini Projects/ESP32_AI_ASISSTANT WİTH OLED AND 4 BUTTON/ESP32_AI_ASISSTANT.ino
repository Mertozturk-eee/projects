#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define BTN_LEFT 2
#define BTN_RIGHT 4
#define BTN_SELECT 5
#define BTN_SEND 18

const char* ssid = "WİFİ";
const char* password = "PASSWORD";
String apiKey = "APIKEY";
String model = "models/gemini-2.5-flash";

WiFiClientSecure client;


String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; 
int cursor = 0;
String message = "";
const int maxCols = 16;
const int maxRows = 3;
int currentRow = 0;


bool showAI = false;
String aiAnswer = "";
int aiX = 0;

void setup() {
  Serial.begin(115200);

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_SEND, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED bulunamadı!");
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Baglaniyor...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client.setInsecure();
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Baglandi!");
  display.display();
  delay(1000);

  showMessage();
}

void loop() {
  if(!showAI) {
  
    if(digitalRead(BTN_LEFT) == LOW){
      cursor = (cursor - 1 + alphabet.length()) % alphabet.length();
      showMessage();
      delay(200);
    }
    if(digitalRead(BTN_RIGHT) == LOW){
      cursor = (cursor + 1) % alphabet.length();
      showMessage();
      delay(200);
    }
    if(digitalRead(BTN_SELECT) == LOW){
      message += alphabet[cursor];
      if(message.length() % maxCols == 0) currentRow++;
      if(currentRow >= maxRows) currentRow = maxRows - 1;
      showMessage();
      delay(200);
    }
    if(digitalRead(BTN_SEND) == LOW){
      
      showAI = true;
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Gonderiliyor...");
      display.display();

      aiAnswer = askGemini(message);
      aiX = SCREEN_WIDTH; 
      message = "";
      currentRow = 0;
      delay(500);
    }
  } else {
 
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("AI Cevabi:");
    display.setCursor(aiX, 20); 
    display.println(aiAnswer);
    display.display();

    aiX--;
    if(aiX + int(aiAnswer.length()*6) < 0) aiX = SCREEN_WIDTH;

    
    if(digitalRead(BTN_SEND) == LOW){
      showAI = false;
      cursor = 0;
      showMessage();
      delay(500);
    }
    delay(50);
  }
}


void showMessage(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  for(int i=0; i<=currentRow; i++){
    int start = i*maxCols;
    int len = min(maxCols, int(message.length() - start));
    if(len > 0) display.println(message.substring(start, start+len));
  }

  display.print("Secilen: ");
  display.println(alphabet[cursor]);
  display.display();
}


String askGemini(String question){
  if (!client.connect("generativelanguage.googleapis.com", 443)) {
    return "Baglanti hatasi!";
  }

  String url = "/v1beta/" + model + ":generateContent?key=" + apiKey;
  String body = "{\"contents\":[{\"parts\":[{\"text\":\"" + question + "\"}]}]}";

  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: generativelanguage.googleapis.com");
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(body.length()));
  client.println("Connection: close");
  client.println();
  client.print(body);

  String response = "";
  while(client.connected() || client.available()){
    if(client.available()){
      response += client.readString();
    }
  }

  client.stop();

  int idx1 = response.indexOf("\"text\":");
  if(idx1 == -1) return "AI cevabi bulunamadı!";
  int start = response.indexOf("\"", idx1 + 7) + 1;
  int end = response.indexOf("\"", start);
  String answer = response.substring(start, end);
  return answer;
}
