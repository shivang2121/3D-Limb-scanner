/*
 * Written by Fabiola Martens and Shivang Srivastava for IGEN 230 3D PLS (Photogrammetric Longitudinal Scanner) project
 * 
 * Used to actuate the scanner, both in the linear and rotational directions using 3 different stepper motors
 * 3 possible scan paths, helical, zig zag and incremental
 *  To change between paths, call the appropriate scan function in the main loop
 */

// Declaration of step and direction pins
#define LINEAR_DIR_PIN 4 // Direction pin for linear motor
#define LINEAR_STEP_PIN 3 // Stepper pin for linear motor
#define ROT_DIR_PIN 7 // Direction pin for rotational motor closest to face
#define ROT_STEP_PIN 9 // Stepper pin for rotational motors

// Declaration of button pins
#define BUTTON_PIN 10
#define LIMIT_REST_PIN 5
#define LIMIT_PUSH_PIN 6

// Declaration of delay times to control speeds and pauses 
#define SCAN_DELAY 4000 // Controls the speed of the scan, a 4000 delay being the slowest and up to ~700 (in microseconds)
#define SETUP_DELAY 1500 // Controls the speed of the linear slider as it moves into setup positions, either for reset or moving to the start positon (in microseconds)
#define TRNIO_SETUP_DELAY 7000 // Controls the amount of time the linear slider waits in the start position before beginning the scan (in milliseconds)

// The sum of these two must be 12600, which is the number of steps for the linear slider to the reach the end
#define STEPS_TO_END 9600
#define STEPS_TO_START 3000

// Used for the incremental scan path
#define STEPS_FOR_ROT 6600
#define STEPS_FOR_INC 3200

// Used for the zig zag scan path
#define STEPS_FOR_PART_ROT 825

/* Setup of inputs and outputs on Arduino */
void setup(){
  // Sets step and direction pins to output
  pinMode(LINEAR_DIR_PIN, OUTPUT);
  pinMode(LINEAR_STEP_PIN, OUTPUT);
  pinMode(ROT_DIR_PIN, OUTPUT);
  pinMode(ROT_STEP_PIN, OUTPUT);

  // Sets button pins to inputs
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LIMIT_REST_PIN, INPUT);
  pinMode(LIMIT_PUSH_PIN, INPUT);

  Serial.begin(9600);

  moveToReset();
}

/* Moves the linear slider to the reset position when the power is first turned on to ensure the scan isn't started from an arbitrary position */
void moveToReset() {
  while (digitalRead(LIMIT_PUSH_PIN) == LOW) {
    digitalWrite(LINEAR_DIR_PIN, LOW); //Writes the direction to the EasyDriver DIR pin. (HIGH is counter clockwise with black wire at B1 on stepper)
    digitalWrite(LINEAR_STEP_PIN, HIGH);
    delayMicroseconds(SETUP_DELAY);
    digitalWrite(LINEAR_STEP_PIN, LOW);
    delayMicroseconds(SETUP_DELAY);
  }
}

/* Main loop once slider is set to reset position */
void loop(){
  // Can only run scan if both the limit switch and start button are pushed (the linear slider must be in the reset position)
  if (digitalRead(BUTTON_PIN) == HIGH && digitalRead(LIMIT_PUSH_PIN) == HIGH) {
    moveToStartPosition();
    /* Uncomment the appropriate line to call the function which runs the desired scan path */
    runHelicalScan();
    // runZigZagScan();
    // runIncrementalScan();
  }
  if (digitalRead(BUTTON_PIN) == HIGH) {
    moveToReset();
  }
}

/* Moves given stepper motor according to parameters:
 *  dirPin is the direction pin of the intended motor (HIGH is counter clockwise with the black wire at B1 on the driver for the small motor, and red at B1 for the larger motors)
 *  dir is the intended direction (either HIGH, or LOW)
 *  stepPin is the step pin of the intended motor
 *  numSteps is the number of steps it should move
 *  delayTime is the speed at which it should move (4000 being slowest, and 700 being the fastest)
 *  
 *  If the blue button is hit, the scan will stop and the slider will go back to the reset postion
 */
void moveSteps(int dirPin, int dir, int stepPin, int numSteps, int delayTime) {
  digitalWrite(dirPin, dir);
  for (int i = 0; i < numSteps; i++) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(delayTime);
    }
  }
}

/* Runs the linear slider to the start position of the scan and waits for TRNIO_SETUP_DELAY time*/
void moveToStartPosition() {
  moveSteps(LINEAR_DIR_PIN, HIGH, LINEAR_STEP_PIN, STEPS_TO_START, SETUP_DELAY);
  delay(TRNIO_SETUP_DELAY); // Pauses for 1000/TRNIO_SETUP_DELAY seconds to allow the operator to hit start on the TRNIO scan and put the cover on
}

/* Runs slider and rotation for the duration of one scan in a helical path.
 * If the blue button is hit, the scan will stop and the slider will go back to the reset position
 */
void runHelicalScan() {
  for (int i = 0; i < STEPS_TO_END; i++){
    if (digitalRead(BUTTON_PIN) == LOW){
      digitalWrite(LINEAR_STEP_PIN, HIGH);
      digitalWrite(ROT_STEP_PIN, HIGH);
      delayMicroseconds(SCAN_DELAY);
      digitalWrite(LINEAR_STEP_PIN, LOW);
      digitalWrite(ROT_STEP_PIN, LOW);
      delayMicroseconds(SCAN_DELAY);
    }
  }
}

/* Runs slider and rotation for the duration of one scan in a zig zag path.
 * Zig Zag means the slider moves the full length, then rotates an ~1/8th of a turn, then slides back to the start positon then rotates, etc.
 */
void runZigZagScan() {
  for (int i = 0; i < 4; i++){
      moveSteps(LINEAR_DIR_PIN, HIGH, LINEAR_STEP_PIN, STEPS_TO_END, SCAN_DELAY);
      moveSteps(ROT_DIR_PIN, LOW, ROT_STEP_PIN, STEPS_FOR_PART_ROT, SCAN_DELAY);
      moveSteps(LINEAR_DIR_PIN, LOW, LINEAR_STEP_PIN, STEPS_TO_END, SCAN_DELAY);
      moveSteps(ROT_DIR_PIN, LOW, ROT_STEP_PIN, STEPS_FOR_PART_ROT, SCAN_DELAY);
  }
}

/* Runs slider and rotation for the duration of one scan in an incremental path
 * Inrecmental means the a full rotation occurs, then the slider moves forward ~1/3 the scan length, then another rotation etc.
 */
void runIncrementalScan() {
  for (int i = 0; i < 4; i++){
      moveSteps(ROT_DIR_PIN, LOW, ROT_STEP_PIN, STEPS_FOR_ROT, SCAN_DELAY);
      moveSteps(LINEAR_DIR_PIN, HIGH, LINEAR_STEP_PIN, STEPS_FOR_INC, SCAN_DELAY);
      moveSteps(ROT_DIR_PIN, LOW, ROT_STEP_PIN, STEPS_FOR_ROT, SCAN_DELAY);
      moveSteps(LINEAR_DIR_PIN, HIGH, LINEAR_STEP_PIN, STEPS_FOR_INC, SCAN_DELAY);
  }
  moveSteps(ROT_DIR_PIN, LOW, ROT_STEP_PIN, STEPS_FOR_ROT, SCAN_DELAY); // ensures a final rotation occurs at the very end
}
