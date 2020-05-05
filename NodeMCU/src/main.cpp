/*
 * IOT Based Robotic Open Drain Cleaner
 * Created by Jarrod Christopher Philips - P2541323
 * This program makes use of Firebase-ESP8266 library by mobizt
 * and makes use of functions from the Stream example sketch.
 * 
 * The program connects to a predefined WiFi network and then connects
 * to a predefined Firebase host to access its Realtime Database.
 * Once connected, it will begin streaming data from a defined path
 * and monitors the path for changes. If a change is detected it will
 * update a variable which controls the motor corresponding to the 
 * change. Based on the value the variable is changed to, the program
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
 *    PWM signal for wheels - D3
 *    Servo for base - D5
 *    Servo for arm - D6
 *    Servo for gripper - D7
*/


#include "FirebaseESP8266.h"    //FirebaseESP8266.h must be included before ESP8266WiFi.h
#include <ESP8266WiFi.h>
#include <Servo.h>

#define FIREBASE_HOST "robot-controller-8cb59.firebaseio.com"
#define FIREBASE_AUTH "xwe8aG9HgXelueuQgrVglB6o7CRvGxantG01F71q"
#define WIFI_SSID "Robot Controller Network"
#define WIFI_PASSWORD "P2541323"


FirebaseData firebaseData;  // Define FirebaseESP8266 data object
String path = "/";          // Firebase stream path (database root for this project)
Servo baseServo, verticalServo, gripperServo;   // Declare objects of class Servo for the servo motors

// initial movement conditions are reset to their neutral positions
int gripperServoStatus = 0, baseServoStatus = 0, verticalServoStatus = 0, wheelsStatus = 0;
int baseAngle = 90, verticalAngle = 90, gripperAngle = 180, movementSpeed = 1023/4;

// function declarations
/*
Used to print the Firebase data in the serial monitor
and store the data into the relevant variable

JSON data is ignored and only printed in the serial
monitor.
*/
void printResult(FirebaseData &data);

// Update the condition and position of the motors.
void updateServo();



void setup()
{
  // Enable serial monitor
  Serial.begin(115200);

  // Connect to predefined Wi-Fi network
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

  // Connect to Firebase host
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  // firebaseData.setBSSLBufferSize(1024, 1024);

  // //Set the size of HTTP response buffers in the case where we want to work with large data.
  // firebaseData.setResponseSize(1024);

  // Begin data stream
  // If stream fails print error in serial monitor, then reboot device
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

  // Initialise motors to their neutral positions
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
  // Attempt to read stream
  // If stream fails print error in serial monitor, then reboot device
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

  // If stream timed out, print message and retry on next loop
  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  // If new data is available to be streamed
  // print data in serial monitor
  if (firebaseData.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + firebaseData.streamPath());  // path in database
    Serial.println("EVENT PATH: " + firebaseData.dataPath());     // data entry
    Serial.println("DATA TYPE: " + firebaseData.dataType());      // data type (JSON or int)
    Serial.println("EVENT TYPE: " + firebaseData.eventType());    // read or write (will always return read for this project)
    Serial.print("VALUE: ");
    printResult(firebaseData);                                    // print data using function printResult
    Serial.println("------------------------------------");
    Serial.println();
  }

  updateServo();


}


void printResult(FirebaseData &data)
{
  if (data.dataType() == "int")       // if data is type int
  {
    if(data.dataPath() == "/wheels")  // if path in database is /wheels
      {
        wheelsStatus = data.intData();    // store data in wheelsStatus
      } 
      // otherwise check if data path is /gripperServo, /baseServo, or /verticalServo
      // and store the variables accordingly
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
    Serial.println(data.intData());   // print the integer data to the serial monitor
  }

  /*
   * If the data is of type JSON
   * acquire the JSON and arrange the data for each entry in the JSON
   * in a readable way in the serial monitor 
   * 
   * This condition is only true at system startup when the device has no 
   * understanding of the database structure. After acquring the json
   * the code will only be acquiring ints as the remote application
   * is only changing single values and not changing a collection of 
   * values.
   */
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
    size_t len = json.iteratorBegin();  // return number of elements in the JSON
    String key, value = "";
    int type = 0;
    // iterate over every data entry in the JSON
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
}

void updateServo()
{
  // Declare and initialise variables to use as timers
  static unsigned long baseTimer = 0, verticalTimer = 0, gripperTimer = 0, movementTimer = 0;

  // Turn the built-in LED off when a motor is rotating
  if (baseServoStatus != 0 || verticalServoStatus != 0 || gripperServoStatus != 0 || wheelsStatus != 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  // LED stays on when stationary
  else 
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  /*
   * For each "Status" variable, the value is checked
   * and for each value a corresponding action takes place.
   * 
   * For the servos, every 50ms the servo angle is either incremented or decremented
   * For the dc motors, every 200ms the duty cycle increases by 1/1023 of 100%
   */
  if (baseServoStatus == 2) 
  {
    if (millis() - baseTimer > 50)
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
    if (millis() - baseTimer > 50)
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
    if (millis() - verticalTimer > 50)
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
    if (millis() - verticalTimer > 50)
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
    if (millis() - gripperTimer > 50)
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
    if (millis() - gripperTimer > 50)
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
    movementSpeed = 1023/4;
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