#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(9, 10);
const byte address[6] = "00001";
char buffer[32];

Servo servoX;
Servo servoY;
Servo servoZ;

#define SERVO_X_PIN 3
#define SERVO_Y_PIN 5
#define SERVO_Z_PIN 6

void setup() {
  Serial.begin(9600);

  servoX.attach(SERVO_X_PIN);
  servoY.attach(SERVO_Y_PIN);
  servoZ.attach(SERVO_Z_PIN);

  servoX.write(90);
  servoY.write(90);
  servoZ.write(90);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  Serial.println("Alici hazir, bekleniyor...");
}

void loop() {
  if (radio.available()) {
    memset(buffer, 0, sizeof(buffer));
    radio.read(buffer, sizeof(buffer));

    String msg = String(buffer);
    Serial.print("Gelen veri: ");
    Serial.println(msg);

    
    int idx1 = msg.indexOf(';');
    int idx2 = msg.indexOf(';', idx1 + 1);
    int idx3 = msg.indexOf(';', idx2 + 1);

    if (idx1 != -1 && idx2 != -1 && idx3 != -1) {
      int xA = msg.substring(0, idx1).toInt();
      int yA = msg.substring(idx1 + 1, idx2).toInt();
      int zA = msg.substring(idx2 + 1, idx3).toInt();

      servoX.write(xA);
      servoY.write(yA);
      servoZ.write(zA);

      Serial.print("Servo X: "); Serial.print(xA);
      Serial.print(" | Y: ");    Serial.print(yA);
      Serial.print(" | Z: ");    Serial.println(zA);
    } else {
      Serial.println("Veri formati hatali!");
    }
  }
}