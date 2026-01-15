#include <Servo.h>
#include <arduinoFFT.h>

/* =========================================================
   MIND CONTROL ARM (FFT LOGIC)
   ---------------------------------------------------------
   Logic:
   - If Beta (Focus) is stronger -> Ratio goes UP -> Hand CLOSES (180)
   - If Alpha (Relax) is stronger -> Ratio goes DOWN -> Hand OPENS (0)
   ========================================================= */

/* 1. FFT SETTINGS (128Hz Sampling) */
const uint16_t samples = 128;
const double samplingFrequency = 128;

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT(vReal, vImag, samples, samplingFrequency);

/* 2. FREQUENCY BANDS */
const int ALPHA_START = 8;
const int ALPHA_END   = 12;
const int BETA_START  = 13;
const int BETA_END    = 30;

// Bin counts for Normalization (To make the fight fair)
const int ALPHA_BINS = ALPHA_END - ALPHA_START + 1; // 5 bins
const int BETA_BINS  = BETA_END - BETA_START + 1;   // 18 bins

/* 3. THRESHOLDS (The "Battle Lines") */
// If Ratio > 1.2, Beta is winning -> FOCUS
const double FOCUS_THRESH = 1.2;

// If Ratio < 0.8, Alpha is winning -> RELAX
const double RELAX_THRESH = 0.8;

/* 4. HARDWARE */
const int eegPin = A0;
Servo s1, s2, s3, s4, s5;

// Servo Pins (Change if yours are different)
const int PINS[] = {3, 5, 6, 9, 10};

/* 5. SERVO STATE */
const int ANGLE_OPEN  = 0;   // Relaxed Hand
const int ANGLE_CLOSE = 180; // Fist
int currentAngle = 90;       // Start neutral

/* TIMING */
unsigned long microseconds;
unsigned int sampling_period_us;

void setup() {
  Serial.begin(115200); // Use 115200 for Plotter
  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));

  // Attach Servos
  s1.attach(PINS[0]);
  s2.attach(PINS[1]);
  s3.attach(PINS[2]);
  s4.attach(PINS[3]);
  s5.attach(PINS[4]);

  writeAll(currentAngle);
  delay(1000);
}

void loop() {

  /* ============================================
     STEP 1: GATHER DATA (1 Second)
     ============================================ */
  for (int i = 0; i < samples; i++) {
    microseconds = micros();
    vReal[i] = analogRead(eegPin);
    vImag[i] = 0;
    while (micros() < microseconds + sampling_period_us) {}
  }

  /* ============================================
     STEP 2: FFT MATH
     ============================================ */
  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  /* ============================================
     STEP 3: CALCULATE POWER (Normalized)
     ============================================ */
  double alphaSum = 0;
  double betaSum  = 0;

  // Sum Alpha
  for (int i = ALPHA_START; i <= ALPHA_END; i++) alphaSum += vReal[i];
  // Sum Beta
  for (int i = BETA_START; i <= BETA_END; i++)   betaSum += vReal[i];

  // Calculate Averages (Total Power / Number of Bins)
  double alphaAvg = alphaSum / ALPHA_BINS;
  double betaAvg  = betaSum / BETA_BINS;

  // Prevent divide by zero
  if (alphaAvg < 0.1) alphaAvg = 0.1;

  // THE MAGIC NUMBER: Ratio
  double ratio = betaAvg / alphaAvg;

  /* ============================================
     STEP 4: CONTROL LOGIC
     ============================================ */

  // LOGIC 1: FOCUS (Beta is Winning) -> CLOSE HAND
  if (ratio >= FOCUS_THRESH) {
    if (currentAngle != ANGLE_CLOSE) {
      currentAngle = ANGLE_CLOSE;
      writeAll(currentAngle);
    }
  }

  // LOGIC 2: RELAX (Alpha is Winning) -> OPEN HAND
  else if (ratio <= RELAX_THRESH) {
    if (currentAngle != ANGLE_OPEN) {
      currentAngle = ANGLE_OPEN;
      writeAll(currentAngle);
    }
  }

  // LOGIC 3: NEUTRAL (In between) -> DO NOTHING

  /* ============================================
     STEP 5: OUTPUT FOR SERIAL PLOTTER
     Blue Line: Your Mind (Ratio)
     Red Line:  Focus Threshold
     Green Line: Relax Threshold
     ============================================ */
  Serial.print("Ratio: ");
  Serial.print(ratio);         Serial.print(" ");
  Serial.print(FOCUS_THRESH);  Serial.print(" ");
  Serial.println(RELAX_THRESH);
    /* ALTERNATIVE STEP 5: FOR SERIAL MONITOR (TEXT) */


  if (ratio >= FOCUS_THRESH) {
        Serial.println(" | STATUS: FOCUSING (Beta High) -> HAND CLOSED");
  }
  else if (ratio <= RELAX_THRESH) {
        Serial.println(" | STATUS: RELAXING (Alpha High) -> HAND OPEN");
  }
  else {
        Serial.println(" | STATUS: NEUTRAL -> HOLDING POS");
  }
}

void writeAll(int angle) {
  s1.write(angle); s2.write(angle); s3.write(angle); s4.write(angle); s5.write(angle);
}