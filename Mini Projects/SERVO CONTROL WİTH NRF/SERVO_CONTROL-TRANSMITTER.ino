#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define MPU_ADDR  0x68
RF24 radio(9, 10);               
const byte address[6] = "00001"; 
char buffer[32];               

int16_t ax, ay, az;

void setup() {
  Wire.begin();
 
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(9600);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();


  int xA = constrain(map(ax, -17000, 17000, 0, 180), 0, 180);
  int yA = constrain(map( ay, -17000, 17000, 0, 180), 0, 180);
  int zA = constrain(map( az, -17000, 17000, 0, 180), 0, 180);


  String msg = String(xA) + ";" + String(yA) + ";" + String(zA) + ";";

  memset(buffer, 0, sizeof(buffer));
  msg.toCharArray(buffer, sizeof(buffer));


  radio.write(buffer, sizeof(buffer));

  Serial.print("Gönderilen veri: ");
  Serial.println(msg);

  delay(200);
}
