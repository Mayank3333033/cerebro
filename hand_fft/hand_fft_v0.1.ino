#include <Servo.h>
#include <arduinoFFT.h>

/* ============================================
   SETTINGS
   ============================================ */

const uint16_t samples = 128;
const double samplingFrequency = 128;

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT(vReal, vImag, samples, samplingFrequency);

const int ALPHA_START = 8;
const int ALPHA_END   = 12;
const int BETA_START  = 13;
const int BETA_END    = 30;

// CALCULATE BIN COUNTS FOR NORMALIZATION
const int ALPHA_BINS = ALPHA_END - ALPHA_START + 1; // equals 5
const int BETA_BINS  = BETA_END - BETA_START + 1;   // equals 18


const double STRESS_RATIO_THRESH = 1.1;
const double CALM_RATIO_THRESH   = 0.9;

const int eegPin = A0;
Servo s1, s2, s3, s4, s5;

const int OPEN_ANGLE  = 0;
const int CLOSE_ANGLE = 180;
int currentAngle = 90;
String currentState = "NEUTRAL";

unsigned long microseconds;
unsigned int sampling_period_us;

void setup() {
  Serial.begin(115200);
  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));

  s1.attach(3);
  s2.attach(5);
  s3.attach(6);
  s4.attach(9);
  s5.attach(10);

  writeAll(90);
  delay(1000);
}

void loop() {

  for (int i = 0; i < samples; i++) {
    microseconds = micros();
    vReal[i] = analogRead(eegPin);
    vImag[i] = 0;
    while (micros() < microseconds + sampling_period_us) {}
  }

  FFT.dcRemoval();
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();


  double alphaSum = 0;
  double betaSum  = 0;

  for (int i = ALPHA_START; i <= ALPHA_END; i++) alphaSum += vReal[i];
  for (int i = BETA_START; i <= BETA_END; i++)   betaSum += vReal[i];

  double alphaAvg = alphaSum / ALPHA_BINS;
  double betaAvg  = betaSum / BETA_BINS;

  if (alphaAvg < 0.1) alphaAvg = 0.1;

  double ratio = betaAvg / alphaAvg;


  if (ratio >= STRESS_RATIO_THRESH) {
    if (currentAngle != CLOSE_ANGLE) {
      currentAngle = CLOSE_ANGLE;
      writeAll(currentAngle);
    }
    currentState = "FOCUSED (Beta High)";
  }
  else if (ratio <= CALM_RATIO_THRESH) {
    if (currentAngle != OPEN_ANGLE) {
      currentAngle = OPEN_ANGLE;
      writeAll(currentAngle);
    }
    currentState = "RELAXED (Alpha High)";
  }
  else {
    currentState = "NEUTRAL (Holding)";
  }

  /* 5. DEBUGGING */
  // Use Serial Monitor to read this clearly
  Serial.print("Ratio: ");
  Serial.print(ratio, 2); // Print only 2 decimal places
  Serial.print(" \t| AlphaAvg: ");
  Serial.print(alphaAvg, 1);
  Serial.print(" \t| BetaAvg: ");
  Serial.print(betaAvg, 1);
  Serial.print(" \t| Status: ");
  Serial.println(currentState);
}

void writeAll(int angle) {
  s1.write(angle);
  s2.write(angle);
  s3.write(angle);
  s4.write(angle);
  s5.write(angle);
}