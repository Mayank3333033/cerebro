#include <Servo.h>
#include <arduinoFFT.h>

const uint16_t samples = 128;
const double samplingFrequency = 128;

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT(vReal, vImag, samples, samplingFrequency);

const int ALPHA_START = 8;
const int ALPHA_END   = 12;
const int BETA_START  = 13;
const int BETA_END    = 30;


const double STRESS_RATIO_THRESH = 1.1; // Above this = Focus
const double CALM_RATIO_THRESH   = 0.9; // Below this = Relax


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


  double alphaPower = 0;
  double betaPower  = 0;

  for (int i = ALPHA_START; i <= ALPHA_END; i++) alphaPower += vReal[i];
  for (int i = BETA_START; i <= BETA_END; i++)   betaPower += vReal[i];

  if (alphaPower < 0.1) alphaPower = 0.1;
  double ratio = betaPower / alphaPower;
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

  Serial.print("Ratio: ");
  Serial.print(ratio);
  Serial.print(" \t| Angle: ");
  Serial.print(currentAngle);
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