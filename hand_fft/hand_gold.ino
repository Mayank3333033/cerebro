#include <Servo.h>
#include <arduinoFFT.h>

/* =========================================================
   THE GOLD STANDARD BCI (Smoothed & Stable)
   ---------------------------------------------------------
   Improvements:
   1. SMOOTHING: Removes servo jitter.
   2. ARTIFACT REJECTION: Ignores violent head movements.
   3. HYSTERESIS: Prevents "flickering" between Open/Close.
   ========================================================= */

/* 1. FFT SETTINGS */
const uint16_t samples = 128;
const double samplingFrequency = 128;

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT(vReal, vImag, samples, samplingFrequency);

/* 2. FREQUENCY BANDS */
const int ALPHA_START = 8;  const int ALPHA_END = 12;
const int BETA_START  = 13; const int BETA_END  = 30;

/* 3. THRESHOLDS (Adjustable) */
const double FOCUS_THRESH = 1.1; // Easy to hit
const double RELAX_THRESH = 0.9;
const int NOISE_LIMIT     = 800; // If signal > 800, it's a movement artifact

/* 4. SMOOTHING FACTOR (0.0 to 1.0) */
// 0.1 = Very Smooth (Slow reaction)
// 0.5 = Balanced
// 0.9 = Very Twitchy (Fast reaction)
const float SMOOTHING = 0.3;
double smoothedRatio = 1.0; // Start neutral

/* 5. HARDWARE */
const int eegPin = A0;
Servo s1, s2, s3, s4, s5;
const int PINS[] = {3, 5, 6, 9, 10};

// SERVO ANGLES
const int ANGLE_OPEN  = 0;
const int ANGLE_CLOSE = 180;
int targetAngle = 90;

void setup() {
  Serial.begin(115200);

  // Attach all servos
  for(int i=0; i<5; i++) {
    // Note: Standard Servo library requires explicit attach
  }
  s1.attach(3); s2.attach(5); s3.attach(6); s4.attach(9); s5.attach(10);

  writeAll(0);
  delay(1000);
}

void loop() {

  bool artifactDetected = false;

  /* STEP 1: SAMPLING */
  unsigned long sampling_period_us = round(1000000 * (1.0 / samplingFrequency));

  for (int i = 0; i < samples; i++) {
    unsigned long microseconds = micros();
    int val = analogRead(eegPin);

    // ARTIFACT CHECK: If signal is hitting the roof (1023) or floor (0)
    // It means you moved your head too hard. Ignore this batch.
    if (val > 1000 || val < 20) artifactDetected = true;

    vReal[i] = val;
    vImag[i] = 0;
    while (micros() < microseconds + sampling_period_us) {}
  }

  // If we detected a sneeze/movement, skip math to save time
  if (artifactDetected) {
    Serial.println("Artifact detected - Ignoring");
    return;
  }

  /* STEP 2: FFT */
  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  /* STEP 3: CALCULATE RATIO */
  double alphaSum = 0, betaSum = 0;
  for (int i = ALPHA_START; i <= ALPHA_END; i++) alphaSum += vReal[i];
  for (int i = BETA_START; i <= BETA_END; i++)   betaSum += vReal[i];

  double alphaAvg = alphaSum / 5.0;
  double betaAvg  = betaSum / 18.0;
  if (alphaAvg < 0.1) alphaAvg = 0.1;

  double currentRatio = betaAvg / alphaAvg;

  /* STEP 4: EXPONENTIAL SMOOTHING (The Secret Sauce) */
  // New = (Old * 0.7) + (Current * 0.3)
  smoothedRatio = (smoothedRatio * (1.0 - SMOOTHING)) + (currentRatio * SMOOTHING);

  /* STEP 5: LOGIC WITH HYSTERESIS */
  if (smoothedRatio >= FOCUS_THRESH) {
     targetAngle = ANGLE_CLOSE;
  }
  else if (smoothedRatio <= RELAX_THRESH) {
     targetAngle = ANGLE_OPEN;
  }
  // If between 0.9 and 1.1, stay in previous state (don't twitch)

  /* STEP 6: ACTUATE */
  writeAll(targetAngle);

  /* PLOTTER OUTPUT */
  Serial.print(smoothedRatio);  Serial.print(" "); // Blue Line (Smooth)
  Serial.print(currentRatio);   Serial.print(" "); // Orange Line (Jittery Raw)
  Serial.print(FOCUS_THRESH);   Serial.print(" "); // Red Line
  Serial.println(RELAX_THRESH);                    // Green Line
}

void writeAll(int angle) {
  s1.write(angle); s2.write(angle); s3.write(angle); s4.write(angle); s5.write(angle);
}