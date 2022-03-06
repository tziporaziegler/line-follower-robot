#include <NewPing.h>
#include <Servo.h>

#include <SoftwareSerial.h>
#include <SD.h>  // Needed by the EMIC2 library, though not utilized directly in this program.
#include "EMIC2.h"

// Ultrasonic distance sensor - eyes
#define TRIGGER_PIN            7
#define ECHO_PIN               8
#define MAX_DISTANCE           70 // Max distance to ping for (in cm). Maximum distance is rated at 400-500cm. Target objects will be 30 - 50 cm away.
#define MIN_DISTANCE           7  // Min distance allowed between the robot and an object. Robot required to stop 5 - 8 cm before objects.

// Servo - head
#define SERVO_PIN              9
#define SERVO_MIN_POSITION     0
#define SERVO_MAX_POSITION     180
#define SERVO_MIDDLE_POSITION  (SERVO_MAX_POSITION - SERVO_MIN_POSITION) / 2

// Speed values
#define LEFT_SPEED              120
#define RIGHT_SPEED             60
#define ADJUST_DELAY            100 // use in servo delay
#define TURN_DELAY              690

// Emic 2
#define RX_PIN                  6  // Connect to SOUT pin
#define TX_PIN                  5  // Connect to SIN pin
#define EMIC_VOLUME             0  // -48 to 18 dB
#define EMIC_VOICE              6  // 9 choices: 0-8

#define RIGHT_WHEEL_PIN         10
#define LEFT_WHEEL_PIN          11

#define IR_RIGHT_PIN            12
#define IR_LEFT_PIN             3

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo servo;
EMIC2 emic;

Servo rightWheel;
Servo leftWheel;

bool checkForIntersections;
bool left_ir;
bool right_ir;// Use to make the drive home faster
int numPackagesDelivered;

int intersectionNum;

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud instead of 9600 to see ping results.

  servo.attach(SERVO_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.attach(LEFT_WHEEL_PIN);

  turnHeadToFaceForward();

  // Initializes the EMIC2 instance.
  // The library sets up a SoftwareSerial port for the communication with the Emic 2 module.
  //pinMode(RX_PIN,INPUT);
  //pinMode(TX_PIN,OUTPUT);
  emic.begin(RX_PIN, TX_PIN);
  emic.setVolume(EMIC_VOLUME);
  emic.setVoice(EMIC_VOICE);
  talk("Hello, there! I have some packages to deliver. Let's go!");

  checkForIntersections = true;
  numPackagesDelivered = 0;
  intersectionNum = 0;

  Serial.print("stopping wheels");
  rightWheel.write(90);
  leftWheel.write(90);
}

void loop() {

  followLine();

  if (checkForIntersections && intersectionDetected()) {
    intersectionNum++;
    talk(String("intersection number") + intersectionNum + "detected");

    if (intersectionNum == 1) {
      driveForward();
    }
    else  if (intersectionNum >= 2) {
      const bool objectOnRight = checkForObjectOnRight();
      const bool objectOnLeft = checkForObjectOnLeft();
      turnHeadToFaceForward();

      if (objectOnRight) {
        turnRight();
        // TODO: Drive until distance sensor detects that short enough distance from the object
        deliverPackage();
        reverseDirection();
        // TODO: Drive same distance back to main road
        turnRight();
      }

      while (!endDetected()) {
        followLine();
      }
      stop();

      deliverPackage();
      reverseDirection();
      delay(100);

      // Drive same distance back to main road
      while (!intersectionDetected()) {
        followLine();
      }
      stop();
      //
      //        // you are at intersection but check if objects are on left
      //        if (objectOnLeft) {
      //          while (!endDetected()) {
      //            followLine();
      //          }
      //          stop();
      //
      //          deliverPackage();
      //
      //          reverseDirection();
      //
      //          while (!intersectionDetected()) {
      //            followLine();
      //          }
      //          stop();
      //          turnLeft();
      //        }
      //        else {
      turnRight();
      //        }
      //
      //        // will only check if no objects on right.
    }
    //else if (objectOnLeft) {
    //        turnLeft();
    //        // Drive until distance sensor detects that short enough distance from the object
    //        while (!endDetected()) {
    //          driveForward();
    //        }
    //        deliverPackage();
    //        reverseDirection();
    //        while (!intersectionDetected()) {
    //          driveForward();
    //        }
    //        turnLeft();
    //      }
  }
  delay(200); //redundancy
  //celebrate();
  //goHome();
  //}
}

/** Drives forward and adjusts left and right */
void followLine()
{
  left_ir = digitalRead(IR_LEFT_PIN);
  right_ir = digitalRead(IR_RIGHT_PIN);
  Serial.print("left ir");
  Serial.println(left_ir);
  Serial.print("right ir");
  Serial.println(right_ir);

  if (left_ir == 1 && right_ir == 0)
  {
    Serial.print("adjust left");
    adjustLeft();
  }
  else if (left_ir == 0 && right_ir == 1)
  {
    Serial.print("adjust right");
    adjustRight();
  }
  else if (left_ir == 0 && right_ir == 0)
  {
    Serial.print("move forward");
    driveForward();
  }
}

void adjustLeft()
{
  rightWheel.write(RIGHT_SPEED + 20); //start spinning
  leftWheel.write(90);
  delay(ADJUST_DELAY);
  rightWheel.write(90);
}

void adjustRight()
{
  leftWheel.write(LEFT_SPEED - 20);
  rightWheel.write(90);
  delay(ADJUST_DELAY);
  leftWheel.write(90);
}

void stop() {
  rightWheel.write(90);
  leftWheel.write(90);
}

/**  Move wheels forward x amount */
void driveForward()
{
  rightWheel.write(RIGHT_SPEED);
  leftWheel.write(LEFT_SPEED);
  delay(ADJUST_DELAY);
  stop();
}

void turnRight() {
  leftWheel.write(LEFT_SPEED);
  rightWheel.write(LEFT_SPEED);
  delay(TURN_DELAY);
  stop();
}

void turnLeft()
{
  rightWheel.write(RIGHT_SPEED);
  leftWheel.write(RIGHT_SPEED);
  delay(TURN_DELAY);
  stop();
}

void reverseDirection()
{
  turnRight();
  turnRight();
}

bool intersectionDetected() {
  return digitalRead(IR_LEFT_PIN) == 1 && digitalRead(IR_RIGHT_PIN) == 1;
}

bool checkForObjectOnRight() {
  turnHeadToTheRight();
  delay(2000);
  return objectDetected();
}

bool checkForObjectOnLeft() {
  turnHeadToTheLeft();
  delay(2000);
  return objectDetected();
}

void turnHeadToTheRight() {
  servo.write(SERVO_MIN_POSITION);
}

void turnHeadToTheLeft() {
  servo.write(SERVO_MAX_POSITION);
}

void turnHeadToFaceForward() {
  servo.write(SERVO_MIDDLE_POSITION);
}

bool objectDetected() {
  delay(50);
  // Do multiple pings (default=5), discard out of range pings and return median in microseconds.
  int medianEchoTime = sonar.ping_median();

  // Converts microseconds to distance in centimeters.
  int currentDistance = sonar.convert_cm(medianEchoTime);

  bool objectDetected = currentDistance != 0 && currentDistance < MAX_DISTANCE;

  if (objectDetected) {
    String msg = String("I see someone waiting for a package. They are") + currentDistance + "centemeters away";
    Serial.println(msg);
    talk(msg);
    // TODO: Dislay currentDistnace on a LED screen?
  } else {
    talk("No object detected");
  }

  return objectDetected;
}

void deliverPackage() {
  talk("Delivery!");
  numPackagesDelivered++;
}

void talk(String message) {
  //emic.speak(message);
  //emic.ready(); // Wait for emic to finish speaking
  //delay(1000);
}

bool endDetected() {
  delay(50);
  // Do multiple pings (default=5), discard out of range pings and return median in microseconds.
  int medianEchoTime = sonar.ping_median();

  // Converts microseconds to distance in centimeters.
  int currentDistance = sonar.convert_cm(medianEchoTime);

  String msg = String("I see someone waiting for a package. They are") + currentDistance + "centemeters away";
  Serial.println(msg);

  bool endDetected = currentDistance != 0 && currentDistance < MIN_DISTANCE;

  return endDetected;
}

void celebrate() {
  // TODO: Spin around
  talk("Mission accomplished!");
}

void goHome() {
  reverseDirection();
  checkForIntersections = false;
}
