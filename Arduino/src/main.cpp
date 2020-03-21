#include <Arduino.h>
#include <Servo.h>

Servo claw;

void setup() {
  Serial.begin(9600);
  Serial.println("Begin");
  claw.attach(11);
  delay(1000);
}

void loop() {
  claw.write(180);
  delay(1000);
  claw.write(0);
  delay(1000);
}