//Viral Science www.youtube.com/c/viralscience  www.viralsciencecreativity.com
//Arduino nRF24Lo1 Joystick Shield Car
//Receiver Car Code
#include <nRF24L01.h>
#include <RF24.h>  //https://github.com/nRF24/RF24
#include <RF24_config.h>
#include <SPI.h>


#define CE_PIN  9
#define CSN_PIN 10

const int RightMotorF= 3;  //in1
const int RightMotorB= 4;  //in2
const int LeftMotorF= 5;  //in3
const int LeftMotorB= 6;  //in4

char data[20]="";

RF24 radio(CE_PIN,CSN_PIN);
const uint64_t pipe = 0xE8E8F0F0E1LL;

void setup()
{
  pinMode(RightMotorF,OUTPUT);
  pinMode(RightMotorB,OUTPUT);
  pinMode(LeftMotorF,OUTPUT);
  pinMode(LeftMotorB,OUTPUT);
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}
 
void loop(){
  String msg="";
  if ( radio.available() )
  {
    radio.read( data,sizeof(data) );
    Serial.println(data);
    msg=data;

    if(msg=="up")
    {
  digitalWrite(RightMotorB,LOW);
  digitalWrite(LeftMotorB,LOW);
  digitalWrite(RightMotorF,HIGH);
  digitalWrite(LeftMotorF,HIGH);
  Serial.println("Motor forward");
  }
  else if(msg=="down")
  {
  digitalWrite(RightMotorB,HIGH);
  digitalWrite(LeftMotorB,HIGH);
  digitalWrite(RightMotorF,LOW);
  digitalWrite(LeftMotorF,LOW);
  Serial.println("Motor Back");
  }
   else if(msg=="left")
  {
  digitalWrite(RightMotorB,LOW);
  digitalWrite(LeftMotorB,HIGH);
  digitalWrite(RightMotorF,HIGH);
  digitalWrite(LeftMotorF,LOW);
  Serial.println("Left");
  delay(3);
  digitalWrite(RightMotorB,LOW);
  digitalWrite(LeftMotorB,LOW);
  digitalWrite(RightMotorF,LOW);
  digitalWrite(LeftMotorF,LOW);
  }
   else if(msg=="right")
  {
  digitalWrite(RightMotorB,HIGH);
  digitalWrite(LeftMotorB,LOW);
  digitalWrite(RightMotorF,LOW);
  digitalWrite(LeftMotorF,HIGH);
  Serial.println("right");
  delay(3);
  digitalWrite(RightMotorB,LOW);
  digitalWrite(LeftMotorB,LOW);
  digitalWrite(RightMotorF,LOW);
  digitalWrite(LeftMotorF,LOW);
  }
  else if(msg=="analogbutton")
  {
  digitalWrite(RightMotorB,LOW);
  digitalWrite(LeftMotorB,LOW);
  digitalWrite(RightMotorF,LOW);
  digitalWrite(LeftMotorF,LOW);
  Serial.println("STOP it");
  }
  
  }
}

    
