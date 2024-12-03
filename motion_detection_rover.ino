#include <ESP32Servo.h>

#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "rc_car"


#include <RemoteXY.h>


#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 94 bytes
  { 255,2,0,4,0,87,0,19,0,0,0,0,31,1,106,200,1,1,4,0,
  72,26,15,53,53,108,166,140,2,26,0,0,0,0,0,0,200,66,0,0,
  0,0,1,0,0,0,0,0,0,32,65,94,0,0,32,65,0,0,200,65,
  135,0,0,200,65,0,0,200,66,5,23,91,60,60,0,2,26,31,70,43,
  32,18,18,16,26,37,0,69,44,60,20,20,0,1 };
  

struct {

    // input variables
  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100

    // output variables
  int8_t circularBar_01; // from 0 to 100
  uint8_t led_01; // from 0 to 1
  int16_t sound_01; // =0 no sound, else ID of sound, =1001

  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)


const int servoPin = 16;       // Servo signal pin
const int motorAPin = 9;     // Motor A pin
const int motorBPin = 10;     // Motor B pin
const int trigPin = 4;       // Trigger pin for distance sensor
const int echoPin = 5;       // Echo pin for distance sensor
const int boardRedLed = 7;    // GPIO pin for the onboard red LED

Servo steeringServo;

void setup() {
  RemoteXY_Init();
  Serial.begin(115200);

  pinMode(motorAPin, OUTPUT);
  pinMode(motorBPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(boardRedLed, OUTPUT);

  steeringServo.attach(servoPin);
}

void loop() {
  RemoteXY_Handler();

  float distance = getDistance();

  if (distance <= 10) {
    RemoteXY.circularBar_01 = 0;
    RemoteXY.led_01 = 1;
    RemoteXY.sound_01 = 1001;
    digitalWrite(boardRedLed, HIGH);
  } else if (distance > 10 && distance <= 25) {
    RemoteXY.circularBar_01 = map(distance, 10, 25, 33, 66);
    RemoteXY.led_01 = 0;
    RemoteXY.sound_01 = 0;
    digitalWrite(boardRedLed, LOW);
  } else {
    RemoteXY.circularBar_01 = map(distance, 25, 100, 67, 100);
    RemoteXY.led_01 = 0;
    RemoteXY.sound_01 = 0;
    digitalWrite(boardRedLed, LOW);
  }

  // Car driving logic
  int steeringAngle = map(RemoteXY.joystick_01_x, -100, 100, 30, 150);
  steeringServo.write(steeringAngle);

  if (RemoteXY.joystick_01_y < -30) {
    digitalWrite(motorAPin, LOW);
    digitalWrite(motorBPin, HIGH); // Forward
  } else if (RemoteXY.joystick_01_y > 30) {
    digitalWrite(motorAPin, HIGH);
    digitalWrite(motorBPin, LOW); // Reverse
  } else {
    digitalWrite(motorAPin, LOW);
    digitalWrite(motorBPin, LOW); // Stop
  }


  delay(50);
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return (duration * 0.034) / 2;
}
