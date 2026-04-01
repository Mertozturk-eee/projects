//Viral Science www.youtube.com/c/viralscience  www.viralsciencecreativity.com
//Arduino nRF24Lo1 Joystick Shield Car
//Transmitter Code
#include <nRF24L01.h>
#include <RF24.h>  //https://github.com/nRF24/RF24
#include <SPI.h>

#define CE_PIN  9
#define CSN_PIN 10

int up_button     = 2; // Button A
int down_button   = 4; // Button C 
int left_button   = 5; // Button D 
int right_button  = 3; // Button B
int start_button  = 6; // Button F
int select_button = 7; // Button E
int analog_button = 8; // Button Joystick
int x_axis = A0;  // Joystick x axis
int y_axis = A1;  // Joystick y axis

int buttons[]={up_button, down_button,left_button,
               right_button,start_button,select_button,analog_button};


const uint64_t pipe = 0xE8E8F0F0E1LL;
RF24 radio(CE_PIN,CSN_PIN);
char msg[20] = "";

void setup(){
     for(int i; i <7 ; i++)
     {
         pinMode(buttons[i],INPUT);
         digitalWrite(buttons[i],HIGH);  
     } 
     Serial.begin(115200);
     radio.begin();
     radio.openWritingPipe(pipe);
}

void loop(){
       
      if(digitalRead(up_button)==LOW)
      {
        char msg[]="up";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("UP Button Pressed");
      }
      
       if(digitalRead(down_button)==LOW)
       {
        char msg[]="down";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("Down Button Pressed");
      }
       if(digitalRead(left_button)==LOW)
      {
        char msg[]="left";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("Left Button Pressed");
      }
       if(digitalRead(right_button)==LOW)
       {
        char msg[]="right";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("Right Button Pressed");
      }
       if(digitalRead(start_button)==LOW)
       {
        char msg[]="start";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("Start Button Pressed");
      }
       if(digitalRead(select_button)==LOW)
       {
        char msg[]="select";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("Select Button Pressed");
      }
       if(digitalRead(analog_button)==LOW)
       {
        char msg[]="analogbutton";
      radio.write(&msg,sizeof(msg));
      delay(3);
        Serial.println("Analog Button Pressed");
      }
      Serial.print("\n X = "),Serial.print(analogRead(x_axis)),Serial.print(" \n Y = "), Serial.print(analogRead(y_axis));
      Serial.print("  ");
      delay(5);
}
