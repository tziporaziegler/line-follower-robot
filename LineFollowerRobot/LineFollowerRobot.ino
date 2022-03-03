#include <NewPing.h>
#include <Servo.h>

#include <SoftwareSerial.h>
#include <SD.h>  // Needed by the EMIC2 library, though not utilized directly in this program.
#include "EMIC2.h"

// Ultrasonic distance sensor - eyes
#define TRIGGER_PIN            7
#define ECHO_PIN               8
#define MAX_DISTANCE           70 // Max distance to ping for (in cm). Maximum distance is rated at 400-500cm.
// Target objects will be 30 - 50 cm away.
#define MIN_DISTANCE           6  // Min distance allowed between the robot and an object.
// Robot required to stop 5 - 8 cm before objects.

// Servo - head
#define SERVO_PIN              9
#define SERVO_MIN_POSITION     0
#define SERVO_MAX_POSITION     180
#define SERVO_MIDDLE_POSITION  (SERVO_MAX_POSITION - SERVO_MIN_POSITION) / 2

// Emic 2
#define RX_PIN                 6  // Connect to SOUT pin
#define TX_PIN                 5  // Connect to SIN pin
#define EMIC_VOLUME            0 // -48 to 18 dB
#define EMIC_VOICE             6   // 9 choices: 0-8

#define RIGHT_WHEEL_PIN        10
#define LEFT_WHEEL_PIN         11

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo servo;
EMIC2 emic;

Servo rightWheel;
Servo leftWheel;

bool checkForIntersections; // Use to make the drive home faster
int numPackagesDelivered;

int intersectionNum = 0;

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud instead of 9600 to see ping results.
  servo.attach(SERVO_PIN);

  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.attach(LEFT_WHEEL_PIN);

  turnHeadToFaceForward();

  // Initializes the EMIC2 instance.
  // The library sets up a SoftwareSerial port for the communication with the Emic 2 module.
  emic.begin(RX_PIN, TX_PIN);
  emic.setVolume(EMIC_VOLUME);
  emic.setVoice(EMIC_VOICE);

//  talk("Hello, there! I have some packages to deliver. Let's go!");
talk("Starting");

  checkForIntersections = true;
  numPackagesDelivered = 0;
}

void loop() {
  driveForward();

  if (checkForIntersections && intersectionDetected()) {
    talk("Intersection detected");
    intersectionNum++;

    if (intersectionNum == 1) {
      // Figure out which way to continue
    } else if (intersectionNum >= 3) {
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

      if (objectOnLeft) {
        turnLeft();
        // TODO: Drive until distance sensor detects that short enough distance from the object
        deliverPackage();
        reverseDirection();
        // TODO: Drive same distance back to main road
        turnLeft();
      }

      // TODO: Drive enough distance to ensure doesn't detect intersection again
    }

    if (endDetected()) {
      celebrate();
      goHome();
    }
  }
}

void driveForward() {
  rightWheel.write(180);
  leftWheel.write(180);
}

void turnRight() {
  // TODO: adjust wheel motors to turn right
}

void turnLeft() {
  // TODO: adjust wheel motors to turn left
}

void reverseDirection() {
  // TODO: adjust wheel motors to turn around, or just do 2 right turns
}

bool intersectionDetected() {
  return false;
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
    talk(String("I see someone waiting for a package. They are") + currentDistance + "centemeters away");
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
  emic.speak(message);
  emic.ready(); // Wait for emic to finish speaking
}

bool endDetected() {
  // TODO: Return true if IR sensors detect end of line while going straight or ultrasonic detects object close enough while going straight.
  return false;
}

void celebrate() {
  // TODO: Spin around
  talk("Mission accomplished!");
}

void goHome() {
  reverseDirection();
  checkForIntersections = false;
}
