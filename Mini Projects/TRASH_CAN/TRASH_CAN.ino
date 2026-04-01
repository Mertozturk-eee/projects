#include <Servo.h>


const int trigPin = 2;
const int echoPin = 3;

Servo myServo;
const int servoPin = 9;


long duration;
int distance;

void setup() {

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  myServo.attach(servoPin);
  myServo.write(0); 
  
  Serial.begin(9600);
}

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  

  duration = pulseIn(echoPin, HIGH);
  

  distance = duration * 0.034 / 2;
  
  Serial.print("Mesafe: ");
  Serial.print(distance);
  Serial.println(" cm");


  if (distance < 20) { 
    myServo.write(90); 
    delay(2000);
    myServo.write(0); 
  }

  delay(500); 

