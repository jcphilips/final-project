# IOT Based Robotic Open Drain Cleaner
## Created by Jarrod Christopher Philips - P2541323

This program makes use of [Firebase-ESP8266 library by mobizt] (https://github.com/mobizt/Firebase-ESP8266) and makes use of functions from the Firebasestream example sketch.  
The program connects to a predefined WiFi network and then connects to a predefined Firebase host to access its Realtime Database. Once connected, it will begin streaming data from a defined path and monitors the path for changes. If a change is detected it will update a variable which controls the motor corresponding to the change. Based on the value the variable is changed to, the program will execute a set of instructions to control the motor. For smooth movement and precision, the servo angle changes 1 degree every 50ms and the speed of the dc motors increases every 200ms when a command to move is given.   

```
Instruction keys:
                    0                 1                     2
    Base servo - hold position / rotate anticlockwise / rotate clockwise 
    Arm servo -  hold position /      raise arm       /    lower arm
    Gripper   -  hold position /     open gripper     /   close gripper  
    Wheels    -   stop         /    move forward      /     reverse
```  

```  
Pin connections:
    Forward pin for wheels - D1
    Reverse pin for wheels - D2
    Servo for base - D5
    Servo for arm - D6
    Servo for gripper - D7
    PWM signal for wheels - D8
```