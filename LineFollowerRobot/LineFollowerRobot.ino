#include <NewPing.h>
#include <Servo.h>

// Ultrasonic distance sensor - eyes
#define TRIGGER_PIN            7
#define ECHO_PIN               8
#define MAX_DISTANCE           50 // Max distance to ping for (in cm). Maximum distance is rated at 400-500cm. Target objects will be 30 - 50 cm away.
#define MIN_DISTANCE           7  // Min distance allowed between the robot and an object. Robot required to stop 5 - 8 cm before objects.

// Servo - head
#define SERVO_PIN              9
#define SERVO_MIN_POSITION     10
#define SERVO_MAX_POSITION     180
#define SERVO_MIDDLE_POSITION  90

// Speed values
#define LEFT_SPEED              94
#define RIGHT_SPEED             86
#define DRIVE_DELAY             50
#define ADJUST_DELAY            100 // use in servo delay
#define TURN_DELAY              2000
#define REVERSE_DELAY           1800
#define TURN_SPEED_RIGHT        80
#define TURN_SPEED_LEFT         100
#define LOOP_COUNT              9

#define RIGHT_WHEEL_PIN         10
#define LEFT_WHEEL_PIN          11

#define IR_RIGHT_PIN            12
#define IR_LEFT_PIN             3

#define GREEN_LED               4
#define RED_LED                 6
#define YELLOW_LED              5

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
Servo servo;

Servo rightWheel;
Servo leftWheel;

bool checkForIntersections;
bool left_ir;
bool right_ir; // Use to make the drive home faster
int numPackagesDelivered;
int intersectionNum;

void setup() {
  Serial.begin(9600);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  // Servo setups
  servo.attach(SERVO_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.attach(LEFT_WHEEL_PIN);
  turnHeadToFaceForward();

  checkForIntersections = true;
  numPackagesDelivered = 0;
  intersectionNum = 0;

  stopWheels();
}

void loop() {
  if (checkForIntersections && intersectionDetected()) {
    intersectionNum++;
    String msg = String("intersection number ") + intersectionNum + " detected";
    Serial.println(msg);

    if (intersectionNum == 1) {
      while (intersectionDetected()) {
        for (int i = 0; i < 5; i++)
        {
          driveForward(); //clearing the merge intersection
        }
      }
      stopWheels();
    }
    else if (intersectionNum == 2) { 
      stopWheels(); //stop at i0

      digitalWrite(RED_LED, HIGH);
      delay(1000);
      digitalWrite(RED_LED, LOW);

      for (int i = 0; i < LOOP_COUNT; i++)
      {
        driveForward();
      }
    }
    else if (intersectionNum >= 3) {
      digitalWrite(RED_LED, HIGH); //anything other than i0 check for objects
      delay(500);
      digitalWrite(RED_LED, LOW);

      const bool objectOnRight = checkForObjectOnRight();
      const bool objectOnLeft = checkForObjectOnLeft();
      turnHeadToFaceForward();

      delay(100);

      if (objectOnRight) {
        turnRight();

        while (!endDetected()) {
          followLine();
        }
        stopWheels(); //follow line till you reach the object then stop

        deliverPackage();
        reverseDirection();
        delay(100);

        // Drive same distance back to main road
        while (!intersectionDetected()) {
          followLine();
        }
        stopWheels();
        Serial.println("Back at main road");

        // You are at intersection but check if objects are on left
        if (objectOnLeft) {
          for (int i = 0; i < LOOP_COUNT; i++)
          {
            driveForward(); //object on both left and right
          }
          stopWheels(); //clear the intersection to follow line

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
        delay(100);

        while (!intersectionDetected()) {
          followLine();
        }
        stopWheels();
        Serial.println("Back at main road");

        turnLeft();
      }
      else {
        Serial.println("No objects to deliver at this intersection. Moving on.");
        for (int i = 0; i < LOOP_COUNT; i++)
        {
          driveForward();
        }
      }
      if (intersectionNum == 7)
      {
        checkForIntersections = false;
      }
    }
  }
  else if (intersectionNum == 7) {
    while (!endDetected())
    {
      followLine();
    }
    stopWheels();
    deliverPackage();
    reverseDirection();
    celebrate();
  }
  else
  {
    followLine();
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
  rightWheel.write(RIGHT_SPEED); // Start spinning
  leftWheel.write(90);
  delay(ADJUST_DELAY);
  rightWheel.write(90);
}

void adjustRight()
{
  Serial.println("Adjusting right");
  leftWheel.write(LEFT_SPEED);
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
  leftWheel.write(TURN_SPEED_LEFT);
  rightWheel.write(90);
  delay(TURN_DELAY);
  stopWheels();
}

void turnLeft()
{
  Serial.println("Turning left");
  rightWheel.write(TURN_SPEED_RIGHT);
  leftWheel.write(90);
  delay(TURN_DELAY);
  stopWheels();
}

void reverseDirection()
{
  Serial.println("Reversing direction");
  leftWheel.write(RIGHT_SPEED);
  rightWheel.write(LEFT_SPEED);
  delay(800);
  leftWheel.write(TURN_SPEED_LEFT);
  rightWheel.write(TURN_SPEED_LEFT);
  delay(REVERSE_DELAY);
  stopWheels();

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

  bool objectDetected = currentDistance != 0 && currentDistance <= MAX_DISTANCE;

  if (objectDetected) {
    String msg = String("I see someone waiting for a package. They are ") + currentDistance + " centimeters away";
    Serial.println(msg);

    digitalWrite(YELLOW_LED, HIGH);
    delay(500);
    digitalWrite(YELLOW_LED, LOW);

  } else {
    String msg = "No object detected";
    Serial.println(msg);
  }

  return objectDetected;
}

void deliverPackage() {
  Serial.println("Delivering package");

  digitalWrite(GREEN_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, LOW);

  numPackagesDelivered++;

  delay(500);
}

bool endDetected() {
  int currentDistance = sonar.ping_cm();

  String msg = String("Person is ") + currentDistance + " centimeters away";
  Serial.println(msg);

  bool endDetected = currentDistance != 0 && currentDistance <= MIN_DISTANCE;

  return endDetected;
}

void celebrate() {
  digitalWrite(RED_LED, HIGH);
  delay(200);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(200);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  delay(200);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(200);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(200);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  delay(200);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(200);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(200);
  digitalWrite(RED_LED, LOW);
}

void goHome() {
  reverseDirection();
  checkForIntersections = false;
}
