#include <Servo.h>

/* ===== EEG SETTINGS ===== */
const int eegPin = A0;

/* ===== EEG BANDS ===== */
const int CALM_LOW   = 20;
const int CALM_HIGH  = 450;

const int STRESS_LOW  = 550;
const int STRESS_HIGH = 900;

/* ===== SERVOS ===== */
Servo s1, s2, s3, s4, s5;

// Servo pins
const int P1 = 3;
const int P2 = 5;
const int P3 = 6;
const int P4 = 9;
const int P5 = 10;

/* ===== SERVO POSITIONS ===== */
const int OPEN_ANGLE  = 0;
const int CLOSE_ANGLE = 180;

int currentAngle = 90;

/* ===== SYNC WRITE ===== */
void writeAll(int angle) {
  s1.write(angle);
  s2.write(angle);
  s3.write(angle);
  s4.write(angle);
  s5.write(angle);
}

void setup() {
  Serial.begin(9600);

  s1.attach(P1);
  s2.attach(P2);
  s3.attach(P3);
  s4.attach(P4);
  s5.attach(P5);

  writeAll(currentAngle); // start holding
}

void loop() {
  int eegValue = analogRead(eegPin);

  /* ===== CALM BAND → OPEN & HOLD ===== */
  if (eegValue >= CALM_LOW && eegValue <= CALM_HIGH) {
    if (currentAngle != OPEN_ANGLE) {
      currentAngle = OPEN_ANGLE;
      writeAll(currentAngle);
    }
  }

  /* ===== STRESS BAND → CLOSE & HOLD ===== */
  else if (eegValue >= STRESS_LOW && eegValue <= STRESS_HIGH) {
    if (currentAngle != CLOSE_ANGLE) {
      currentAngle = CLOSE_ANGLE;
      writeAll(currentAngle);
    }
  }

  /* ===== SERIAL PLOTTER (0–600 ONLY) ===== */
  Serial.print(constrain(eegValue, 0, 1000)); Serial.print(" ");
  Serial.print(CALM_HIGH);                   Serial.print(" ");
  Serial.println(STRESS_LOW);

  delay(20); // stable servo refresh
}
