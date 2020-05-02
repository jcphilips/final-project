/*
 * IOT Based Robotic Open Drain Cleaner
 * Created by Jarrod Christopher Philips - P2541323
 * This program makes use of Firebase-ESP8266 library by mobizt
 * and makes use of functions from the Firebasestream example sketch.
 * 
 * The program connects to a predefined WiFi network and then connects
 * to a predefined Firebase host to access its Realtime Database.
 * Once connected, it will begin streaming data from a defined path
 * and monitors the path for changes. If a change is detected it will
 * update a variable which controls the motor corresponding to the 
 * chage. Based on the value the variable is changed to, the program
 * will execute a set of instructions to control the motor. For smooth
 * movement and precision, the servo angle changes 1 degree every 
 * 50ms and the speed of the dc motors increases every 200ms when
 * a command to move is given.
 * 
 * Instruction keys:
 *                       0                 1                     2
 *    Base servo - hold position / rotate anticlockwise / rotate clockwise 
 *    Arm servo -  hold position /      raise arm       /    lower arm
 *    Gripper   -  hold position /     open gripper     /   close gripper  
 *    Wheels    -   stop         /    move forward      /     reverse
 * 
 * 
 * Pin connections:
 *    Forward pin for wheels - D1
 *    Reverse pin for wheels - D2
 *    Servo for base - D5
 *    Servo for arm - D6
 *    Servo for gripper - D7
 *    PWM signal for wheels - D8
*/


//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Servo.h>

#define FIREBASE_HOST "robot-controller-8cb59.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "xwe8aG9HgXelueuQgrVglB6o7CRvGxantG01F71q"
#define WIFI_SSID "Robot Controller Network"
#define WIFI_PASSWORD "P2541323"
#define timerValue 50

//Define FirebaseESP8266 data object
FirebaseData firebaseData;

String path = "/";
Servo baseServo, verticalServo, gripperServo;

void printResult(FirebaseData &data);
void updateMotors();
void updateHelperServo(int componentStatus, unsigned long &timer, int &servoAngle, Servo i, int minAngle = 0, int incrementStatus = 1, int decrementStatus = 2);
void updateHelperDCmotor(unsigned long &timer);


int gripperServoStatus = 0, baseServoStatus = 0, verticalServoStatus = 0, wheelsStatus = 0;
int baseAngle = 90, verticalAngle = 90, gripperAngle = 180, movementSpeed = 0;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

  if (!Firebase.beginStream(firebaseData, path))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  baseServo.attach(D5);
  baseServo.write(baseAngle);
  delay(300);
  verticalServo.attach(D6);
  verticalServo.write(verticalAngle);
  delay(300);
  gripperServo.attach(D7);
  gripperServo.write(gripperAngle); 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  analogWrite(D3, 0);
}

void loop()
{
  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (firebaseData.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + firebaseData.streamPath());
    Serial.println("EVENT PATH: " + firebaseData.dataPath());
    Serial.println("DATA TYPE: " + firebaseData.dataType());
    Serial.println("EVENT TYPE: " + firebaseData.eventType());
    Serial.print("VALUE: ");
    printResult(firebaseData);
    Serial.println("------------------------------------");
    Serial.println();
  }

  updateMotors();


}

void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
  {
    if(data.dataPath() == "/wheels")
      {
        wheelsStatus = data.intData();
      } 
      else if (data.dataPath() == "/gripperServo")
      {
        gripperServoStatus = data.intData();
      }
      else if (data.dataPath() == "/baseServo")
      {
        baseServoStatus = data.intData();
      }
      else if (data.dataPath() == "/verticalServo")
      {
        verticalServoStatus = data.intData();
      }
    Serial.println(data.intData());
  }
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == JSON_OBJECT ? "object" : "array");
      if (type == JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == JSON_STRING ||
               jsonData.typeNum == JSON_NULL ||
               jsonData.typeNum == JSON_OBJECT ||
               jsonData.typeNum == JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
}

void updateMotors()
{
  static unsigned long baseTimer = 0, verticalTimer = 0, gripperTimer = 0, movementTimer = 0;

  if (baseServoStatus != 0 || verticalServoStatus != 0 || gripperServoStatus != 0 || wheelsStatus != 0)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else 
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  updateHelperServo(baseServoStatus, baseTimer, baseAngle, baseServo);
  updateHelperServo(verticalServoStatus, verticalTimer, verticalAngle, verticalServo);
  updateHelperServo(gripperServoStatus, gripperTimer, gripperAngle,gripperServo, 90, 2, 1);
  updateHelperDCmotor(movementTimer);  
}

void updateHelperServo(int componentStatus, unsigned long &timer, int &servoAngle, Servo i, int minAngle, int incrementStatus, int decrementStatus)
{
  if (componentStatus == incrementStatus) 
  {
    if (millis() - timer > timerValue)
    {
      timer = millis();
      if (servoAngle < 180) {
        servoAngle++;
        i.write(servoAngle);
      }
    }
  }
  else if (componentStatus == decrementStatus) 
  {
    if (millis() - timer > timerValue)
    {
      timer = millis();
      if (servoAngle > minAngle) {
        servoAngle--;
        i.write(servoAngle);
      }
    }
  }
}

void updateHelperDCmotor(unsigned long &timer)
{

  if (wheelsStatus == 1)
  {
    if (millis() - timer > timerValue)
    {
      timer = millis();
      digitalWrite(D1, HIGH);
      digitalWrite(D2, LOW);
      if (movementSpeed < 1023)
      {
        movementSpeed++;
        analogWrite(D3, movementSpeed);
      }
    }
  }
  else if (wheelsStatus == 0)
  {
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    movementSpeed = 0;
    analogWrite(D3, movementSpeed);
  }
  else if (wheelsStatus == 2)
  {
    if (millis() - timer > timerValue) 
    {
      timer = millis();
      digitalWrite(D1, LOW);
      digitalWrite(D2, HIGH);
      if (movementSpeed < 1023)
      {
        movementSpeed++;
        analogWrite(D3, movementSpeed);
      }
    }   
  }  
}