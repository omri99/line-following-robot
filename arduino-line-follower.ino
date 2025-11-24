#include <QTRSensors.h>

// Motor pins (PWM + direction)
#define PWMA 3   
#define AIN1 5   
#define AIN2 6   
#define PWMB 9   
#define BIN1 10  
#define BIN2 11  

// Speed settings
int MAX_SPEED = 145;     
int BASE_SPEED = 105;    

QTRSensors qtr;

const uint8_t SensorCount = 5; 
uint16_t sensorValues[SensorCount];
int threshold[SensorCount];

// PID constants
float Kp = 1.65;
float Ki = 0.0;
float Kd = 3.70;

uint8_t multiP = 1, multiI = 1, multiD = 1; 
float Pvalue, Ivalue, Dvalue;

uint16_t position;
int P, D, I, previousError, PIDvalue, error;
int lsp, rsp;
uint16_t lastPosition = 0;
bool onLine = true;

void setup() {
  // Motor pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // QTR sensors setup
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A1, A2, A3, A4, A5}, SensorCount);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED on during calibration

  // Sensor calibration loop
  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // LED off after calibration

  // Set threshold for each sensor
  for (uint8_t i = 0; i < SensorCount; i++) {
    threshold[i] = (qtr.calibrationOn.minimum[i] + qtr.calibrationOn.maximum[i]) / 2;
  }

  delay(1000);
}

void loop() {
  robot_control();
}

// Main control function
void robot_control() {
  position = readLine();
  error = 2000 - position; // Center is 2000 for 5 sensors
  PID_Linefollow(error);
}

// Return last known line position
uint16_t continueWithLastPosition() {
  return lastPosition;
}

// Default drift value
uint16_t driftLeftFromCenter() {
  return 2000; 
}

// Read sensors and compute line position
uint16_t readLine() {
  uint16_t position = qtr.readLineBlack(sensorValues);
  onLine = false;
  bool allOnBlack = true;

  // Check each sensor
  for (uint8_t i = 0; i < SensorCount; i++) {
    if (sensorValues[i] > threshold[i]) {
      onLine = true;
    } else {
      allOnBlack = false;
    }
  }

  // No line detected
  if (!onLine) {
    if (lastPosition < 2000 || lastPosition > -2000) {
      return driftLeftFromCenter();
    } else {
      return continueWithLastPosition();
    }
  }

  // All sensors on black → keep last position
  if (allOnBlack) {
    return continueWithLastPosition();
  }

  lastPosition = position;
  return position;
}

// PID control
void PID_Linefollow(int error) {
  P = error;
  I += error;
  D = error - previousError;

  Pvalue = (Kp / pow(10, multiP)) * P;
  Ivalue = (Ki / pow(10, multiI)) * I;
  Dvalue = (Kd / pow(10, multiD)) * D;

  PIDvalue = Pvalue + Ivalue + Dvalue;

  previousError = error;

  lsp = BASE_SPEED - PIDvalue;
  rsp = BASE_SPEED + PIDvalue;

  motor_drive(lsp, rsp);
}

// Drive motors with speed + direction
void motor_drive(int left, int right) {
  left = constrain(left, -MAX_SPEED, MAX_SPEED);
  right = constrain(right, -MAX_SPEED, MAX_SPEED);

  // Left motor
  analogWrite(PWMA, abs(left));
  digitalWrite(AIN1, left > 0 ? HIGH : LOW);
  digitalWrite(AIN2, left < 0 ? HIGH : LOW);

  // Right motor
  analogWrite(PWMB, abs(right));
  digitalWrite(BIN1, right > 0 ? HIGH : LOW);
  digitalWrite(BIN2, right < 0 ? HIGH : LOW);
}
