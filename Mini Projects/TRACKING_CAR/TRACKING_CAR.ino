#include <Servo.h>


const int trigPin = 12;
const int echoPin = 13;
long duration;
int distance, distance2, distance3, distances;
Servo s1;

#define LED1R 10

bool AUTO=true;

int IN1 = 3; 
int IN2 = 4; 
int IN3 = 5; 
int IN4 = 6;

int joystickX = A2;
int joystickY = A3;
int X,Y;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  s1.attach(11);
  s1.write(90);

  pinMode(LED1R, OUTPUT);  
  
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 

  digitalWrite(LED1R, LOW);
  X=550;
  Y=550;
}

void loop() {
if(!AUTO)
{
  Serial.println("X");
  Serial.println(X);
  Serial.println("Y");
  Serial.println(Y);
  X = analogRead(joystickX);
  Y = analogRead(joystickY);
  delay(200);

  if(X>600)
  {
    sag();
  }
  else if(X<500)
  {
    sol();
  }
  if(Y>600)
  {
    ileri();
  }
  else if(Y<500)
  {
    geri();
  }
 
}
else
{

  
  distances = calDist();
  

  Serial.print("\nDistance: ");
  Serial.println(distances);
   

  if(distance>22)
   {
    ileri();
    
   }
  
  else if (distances > 0 && distances <= 22)
  {
    dur();
    digitalWrite(LED1R, LOW);
    s1.write(20);
    delay(2000);
    distance2 = calDist();
    delay(1500);
    s1.write(160);
    delay(2000);
    distance3 = calDist();
    delay(1500);
    
    // Karar verme
  if (distance2 >= 22 && distance2 > distance3) {
      s1.write(90); // Servoyu ortaya çevir
      Serial.print("Left is clear: ");
      Serial.println(distance2);
      sag();
      delay(250);
    } 
  else if (distance3 >= 22 && distance3 > distance2) {
      s1.write(90); // Servoyu ortaya çevir
      Serial.print("Right is clear: ");
      Serial.println(distance3);
      sol();
      delay(250);
    }
  else if (distance3 >= 22 && distance2 >= 22 && distance3 == distance2) 
  {
      s1.write(90);
      Serial.print("Right is clear: ");
      Serial.println(distance3);
      sol();
      delay(250);
    } 
  else if(distance3 < 22 && distance2<22)
  {
      Serial.println("No clear path detected!");
      geri();
      digitalWrite(LED1R, HIGH);
      s1.write(90);
      delay(2500);
      digitalWrite(LED1R, LOW);
      dur();
    }
  }
  
}


  

  
}


int calDist() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  duration = pulseIn(echoPin, HIGH, 30000); 
  
  if (duration == 0) {
    Serial.println("Error: No echo received");
    return -1; 
  }
  

  distance = duration * 0.034 / 2;
  return distance;
}

void geri(){
 
 
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
   
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}
void ileri(){

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}
void sag(){
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
   
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}
void sol(){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}
void dur(){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}
