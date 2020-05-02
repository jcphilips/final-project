/*
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2019 mobizt
 * 
 * This example is for FirebaseESP8266 Arduino library v 2.6.0 and later
 *
*/

//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Servo.h>

#define FIREBASE_HOST "robot-controller-8cb59.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "xwe8aG9HgXelueuQgrVglB6o7CRvGxantG01F71q"
#define WIFI_SSID "Robot Controller Network"
#define WIFI_PASSWORD "P2541323"

//Define FirebaseESP8266 data object
FirebaseData firebaseData;

String path = "/";
Servo baseServo, verticalServo, gripperServo;

void printResult(FirebaseData &data);
void updateServo();

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
    digitalWrite(LED_BUILTIN, HIGH);
    ESP.restart();
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
  digitalWrite(LED_BUILTIN, LOW);
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
    digitalWrite(LED_BUILTIN, HIGH);
    ESP.restart();
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
    digitalWrite(LED_BUILTIN, HIGH);
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
    digitalWrite(LED_BUILTIN, LOW);
  }

  updateServo();


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

void updateServo()
{
  static unsigned long baseTimer = 0, verticalTimer = 0, gripperTimer = 0, movementTimer = 0;

  if (baseServoStatus != 0 || verticalServoStatus != 0 || gripperServoStatus != 0 || wheelsStatus != 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else 
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  if (baseServoStatus == 2) 
  {
    if (millis() - baseTimer > 25)
    {
      baseTimer = millis();
      if (baseAngle < 180) {
        baseAngle++;
        baseServo.write(baseAngle);
      }
    }
  }
  else if (baseServoStatus == 1)
  {
    if (millis() - baseTimer > 25)
    {
      baseTimer = millis();
      if (baseAngle > 0) {
        baseAngle--;
        baseServo.write(baseAngle);
      }
    }
  }
  if (verticalServoStatus == 1)
  {
    if (millis() - verticalTimer > 25)
    {
      verticalTimer = millis();
      if (verticalAngle < 180)
      {
        verticalAngle++;
        verticalServo.write(verticalAngle);
      }
    }
  }
  else if (verticalServoStatus == 2)
  {
    if (millis() - verticalTimer > 25)
    {
      verticalTimer = millis();
      if (verticalAngle > 0)
      {
        verticalAngle--;
        verticalServo.write(verticalAngle);
      }
    }
  }
  if (gripperServoStatus == 1)
  {
    if (millis() - gripperTimer > 25)
    {
      gripperTimer = millis();
      if (gripperAngle > 90)
      {
        gripperAngle--;
        gripperServo.write(gripperAngle);
      }
    }
  }
  else if (gripperServoStatus == 2)
  {
    if (millis() - gripperTimer > 25)
    {
      gripperTimer = millis();
      if (gripperAngle < 180)
      {
        gripperAngle++;
        gripperServo.write(gripperAngle);
      }
    }
  }
  if (wheelsStatus == 1)
  {
    if (millis() - movementTimer > 200)
    {
      movementTimer = millis();
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
    movementSpeed = 1023/3;
    analogWrite(D3, movementSpeed);
  }
  else if (wheelsStatus == 2)
  {
    if (millis() - movementTimer > 200) 
    {
      movementTimer = millis();
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