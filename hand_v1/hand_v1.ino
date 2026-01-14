#include <Servo.h>

/* ===== EEG SETTINGS ===== */
const int eegPin = A0;

/* ===== AMPLITUDE LIMITS ===== */
const int AMP_CALM_MIN   = 200;
const int AMP_CALM_MAX   = 600;

const int AMP_STRESS_MIN = 50;
const int AMP_STRESS_MAX = 350;

/* ===== FREQUENCY LIMITS ===== */
const int CALM_FREQ_MAX   = 8;   // slow waves
const int STRESS_FREQ_MIN = 14;  // fast waves

/* ===== SERVOS ===== */
Servo s1, s2, s3, s4, s5;
const int P1 = 3, P2 = 5, P3 = 6, P4 = 9, P5 = 10;

const int OPEN_ANGLE = 0;
const int CLOSE_ANGLE = 180;

int currentAngle = 90;

/* ===== Frequency Detection ===== */
unsigned long lastCross = 0;
unsigned long period = 0;
float frequency = 0;
int prevSignal = 0;

/* ===== Write all servos ===== */
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

  writeAll(currentAngle);
}

void loop() {
  int eegValue = analogRead(eegPin);

  /* ===== Frequency Calculation ===== */
  int mid = 512; // center of ADC

  if (prevSignal < mid && eegValue >= mid) { // rising zero crossing
    unsigned long now = millis();
    period = now - lastCross;
    lastCross = now;

    if (period > 0)
      frequency = 1000.0 / period;
  }

  prevSignal = eegValue;

  /* ===== Calm Detection ===== */
  if (eegValue >= AMP_CALM_MIN && eegValue <= AMP_CALM_MAX &&
      frequency <= CALM_FREQ_MAX) {

    if (currentAngle != OPEN_ANGLE) {
      currentAngle = OPEN_ANGLE;
      writeAll(currentAngle);
    }
  }

  /* ===== Stress Detection ===== */
  else if (eegValue >= AMP_STRESS_MIN && eegValue <= AMP_STRESS_MAX &&
           frequency >= STRESS_FREQ_MIN) {

    if (currentAngle != CLOSE_ANGLE) {
      currentAngle = CLOSE_ANGLE;
      writeAll(currentAngle);
    }
  }

  /* ===== Serial Plot ===== */
  Serial.print(eegValue); Serial.print(" ");
  Serial.print(frequency); Serial.print(" ");
  Serial.println(mid);

  delay(10);
}
