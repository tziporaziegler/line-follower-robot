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

// speed values
#define leftSpd                 100
#define rightSpd                80
#define adjust_delay            100 // use in servo delay
#define turn_delay              500

// Emic 2
#define RX_PIN                 6  // Connect to SOUT pin
#define TX_PIN                 5  // Connect to SIN pin
#define EMIC_VOLUME            0 // -48 to 18 dB
#define EMIC_VOICE             6   // 9 choices: 0-8

#define RIGHT_WHEEL_PIN        10
#define LEFT_WHEEL_PIN         11

#define IR_RIGHT               12
#define IR_LEFT                13

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
  //driveForward()
  followLine();// adjusts left right and goes straight

  if (checkForIntersections && intersectionDetected()) {
    talk("Intersection detected");
    intersectionNum++;

    if (intersectionNum == 1) {
      // have to add adjust right and left functions based on IR sensors measurements.
    } else if{ intersectionNum == 2){
      Talk("First intersection, No Object checking");
    }
      else if (intersectionNum >= 3) {
        //add second intersection
        const bool objectOnRight = checkForObjectOnRight();
        const bool objectOnLeft = checkForObjectOnLeft();
        turnHeadToFaceForward();
      
      if (objectOnRight) {
        turnRight();
        // TODO: Drive until distance sensor detects that short enough distance from the object
        while(!objectDetected()){
          driveForward();
        }
        deliverPackage();
        reverseDirection();
        // TODO: Drive same distance back to main road
        while(!intersectionDetected()){
          driveForward();
        }
      
        // you are at intersection but check if objects are on left
        if (objectonLeft){
          while(!objectDetected()){
            driveForward();
          }
          deliverPackage();
          reverseDirection();
          while(!intersectionDetected()){
            driveForward();
          }
          turnLeft();
        }
        else{
          turnRight();
        }

       // will only check if no objects on right.
      }else if (objectOnLeft) {
        turnLeft();
        // TODO: Drive until distance sensor detects that short enough distance from the object
        while(!objectDetected()){
            driveForward();
          }
          deliverPackage();
          reverseDirection();
          while(!intersectionDetected()){
            driveForward();
          }
          turnLeft();
      }
    }
  
    
// this will run if no intersection detected
  } else if (endDetected()) {
      celebrate();
      goHome();
    }
  }
}
void adjustLeft()
{
  right_wheel.write(rightSpd); //start spinning
  delay(adjust_delay);
  right_wheel.write(90); //stop spinning
}

void adjustRight()
{
  left_wheel.write(leftSpd);
  delay(adjust_delay);
  left_wheel.write(90);
  
}

void driveForward()
{
  // TODO: Move wheels forward x amount using IR sensors to follow the line
  right_wheel.write(rightSpd);
  left_wheel.write(leftSpd);
  delay(adjust_delay);
  right_wheel.write(90);
  right_wheel.write(90);        
}

void turnRight() {
  // TODO: adjust wheel motors to turn right
  left_wheel.write(leftSpd);
  delay(turn_delay);
  left_wheel.write(90);
}

void turnLeft() 
{
  right_wheel.write(rightSpd);
  delay(turn_delay);
  right_wheel.write(90);
}

void reverseDirection() 
{
  turnRight();
  turnRight();
}
void followLine()
{
  if(intersectionDetected())
  {
      intersectionNum++; //do nothing?? but i wanna check this condition first for some reason
  }
  else if((digitalRead(IR_LEFT)==1)&&(digitalRead(IR_RIGHT==0)))
  {
    adjustLeft();
  }
  else if((digitalRead(IR_LEFT)==0)&&(digitalRead(IR_RIGHT==1)))
  {
    adjustRight();
  }
  else //both are 1
  {
    driveForward();
  }
}


bool intersectionDetected() {
  return digitalRead(IR_LEFT) && digitalRead(IR_RIGHT);
  // returns true when both meeasurements from IR sensors are high (1). 
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
