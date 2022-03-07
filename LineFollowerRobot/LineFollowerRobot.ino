#include <NewPing.h>
#include <Servo.h>

#include <HardwareSerial.h>
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
#define LEFT_SPEED              93
#define RIGHT_SPEED             87
#define DRIVE_DELAY             50
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

//#define mySerial Serial2
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
//HardwareSerial Serial2;
void setup() {
  Serial.begin(9600);
  // Initializes the EMIC2 instance.
  // The library sets up a SoftwareSerial port for the communication with the Emic 2 module.
  emic.begin(RX_PIN, TX_PIN);
  //emic.begin(&Serial2)
  emic.setVolume(EMIC_VOLUME);
  emic.setVoice(EMIC_VOICE);
  talk("Hello, there! I have some packages to deliver. Let's go!");

  //servo setups
  servo.attach(SERVO_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.attach(LEFT_WHEEL_PIN);

  turnHeadToFaceForward();

  //Variables
  checkForIntersections = true;
  numPackagesDelivered = 0;
  intersectionNum = 0;

  stopWheels();
}

void loop() {
  followLine();

  if (checkForIntersections && intersectionDetected()) {
    intersectionNum++;
    String msg = String("intersection number ") + intersectionNum + " detected";
    Serial.println(msg);
    talk(msg);

    if (intersectionNum == 1) {
      while (intersectionDetected()) {
        driveForward();
      }
      stopWheels();
    }
    else if (intersectionNum >= 2) {
      const bool objectOnRight = checkForObjectOnRight();
      const bool objectOnLeft = checkForObjectOnLeft();
      turnHeadToFaceForward();

      if (objectOnRight) {
        turnRight();

        while (!endDetected()) {
          followLine();
        }
        stopWheels();

        deliverPackage();
        reverseDirection();
        delay(100);

        // Drive same distance back to main road
        while (!intersectionDetected()) {
          followLine();
        }
        stopWheels();
        Serial.println("Back at main road");

        // you are at intersection but check if objects are on left
        if (objectOnLeft) {
          while (intersectionDetected()) {
            driveForward();
          }
          stopWheels();

          while (!endDetected()) {
            followLine();
          }
          stopWheels();

          deliverPackage();
          reverseDirection();
          delay(100);

          while (!intersectionDetected()) {
            followLine();
          }
          stopWheels();
          Serial.println("Back at main road");

          turnLeft();
        }
        else {
          turnRight();
        }
      }
      else if (objectOnLeft) {
        turnLeft();
        // Drive until distance sensor detects that short enough distance from the object
        while (!endDetected()) {
          followLine();
        }
        stopWheels();

        deliverPackage();
        reverseDirection();

        while (!intersectionDetected()) {
          followLine();
        }
        stopWheels();
        Serial.println("Back at main road");

        turnLeft();
      }
      else {
        Serial.println("No objects to deliver at this intersection. Moving on.");
      }

      while (intersectionDetected()) {
        driveForward();
      }
      stopWheels();
    }
  }

  if (numPackagesDelivered == 5) {
    Serial.println("All packages delivered");
    celebrate();
    goHome();
  }
}

/** Drives forward and adjusts left and right */
void followLine()
{
  left_ir = digitalRead(IR_LEFT_PIN);
  right_ir = digitalRead(IR_RIGHT_PIN);
  //  Serial.print("left ir");
  //  Serial.println(left_ir);
  //  Serial.print("right ir");
  //  Serial.println(right_ir);

  if (left_ir == 1 && right_ir == 0)
  {
    adjustLeft();
  }
  else if (left_ir == 0 && right_ir == 1)
  {
    adjustRight();
  }
  else if (!checkForIntersections || ( left_ir == 0 && right_ir == 0))
  {
    driveForward();
  }
}

void adjustLeft()
{
  Serial.println("Adjusting left");
  rightWheel.write(RIGHT_SPEED + 20); //start spinning
  leftWheel.write(90);
  delay(ADJUST_DELAY);
  rightWheel.write(90);
}

void adjustRight()
{
  Serial.println("Adjusting right");
  leftWheel.write(LEFT_SPEED - 20);
  rightWheel.write(90);
  delay(ADJUST_DELAY);
  leftWheel.write(90);
}

void stopWheels() {
  rightWheel.write(90);
  leftWheel.write(90);
}

/**  Move wheels forward x amount */
void driveForward()
{
  Serial.println("Driving forward");
  rightWheel.write(RIGHT_SPEED);
  leftWheel.write(LEFT_SPEED);
  delay(DRIVE_DELAY);
  stopWheels();
}

void turnRight() {
  Serial.println("Turning right");
  leftWheel.write(LEFT_SPEED);
  rightWheel.write(LEFT_SPEED);
  delay(TURN_DELAY);
  stopWheels();
}

void turnLeft()
{
  Serial.println("Turning left");
  rightWheel.write(RIGHT_SPEED);
  leftWheel.write(RIGHT_SPEED);
  delay(TURN_DELAY);
  stopWheels();
}

void reverseDirection()
{
  Serial.println("Reversing direction");
  turnRight();
  turnRight();
}

bool intersectionDetected() {
  return digitalRead(IR_LEFT_PIN) == 1 && digitalRead(IR_RIGHT_PIN) == 1;
}

bool checkForObjectOnRight() {
  Serial.println("Checking for object on the right");
  turnHeadToTheRight();
  delay(2000);
  return objectDetected();
}

bool checkForObjectOnLeft() {
  Serial.println("Checking for object on the left");
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
  delay(100);
  // Do multiple pings (default=5), discard out of range pings and return median in microseconds.
  int medianEchoTime = sonar.ping_median();

  // Converts microseconds to distance in centimeters.
  int currentDistance = sonar.convert_cm(medianEchoTime);

  bool objectDetected = currentDistance != 0 && currentDistance < MAX_DISTANCE;

  if (objectDetected) {
    String msg = String("I see someone waiting for a package. They are ") + currentDistance + " centemeters away";
    Serial.println(msg);
    talk(msg);
    // TODO: Dislay currentDistnace on a LED screen?
  } else {
    String msg = "No object detected";
    Serial.println(msg);
    talk(msg);
  }

  return objectDetected;
}

void deliverPackage() {
  Serial.println("Delivering package");
  talk("Delivery!");
  numPackagesDelivered++;
}

void talk(String message) {
  emic.speak(message);  
  emic.ready(); // Wait for emic to finish speaking
}

bool endDetected() {
  delay(100);
  // Do multiple pings (default=5), discard out of range pings and return median in microseconds.
  int medianEchoTime = sonar.ping_median();

  // Converts microseconds to distance in centimeters.
  int currentDistance = sonar.convert_cm(medianEchoTime);

  String msg = String("Person is ") + currentDistance + " centemeters away";
  Serial.println(msg);

  bool endDetected = currentDistance != 0 && currentDistance < MIN_DISTANCE;

  return endDetected;
}

void celebrate() {
  talk("Mission accomplished!");
}

void goHome() {
  reverseDirection();
  checkForIntersections = false;
}
