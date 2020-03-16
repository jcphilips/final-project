#include <Arduino.h>
#include <Servo.h>

Servo claw;
void setup() {
  claw.attach(11);
  delay(1000);
  claw.read();
}

void loop() {
  // put your main code here, to run repeatedly:
}