#include <Servo.h>
#include <arduinoFFT.h>

const uint16_t samples = 64;
const double samplingFrequency = 64;

double vReal[samples];
double vImag[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

const int ALPHA_START = 8;
const int ALPHA_END   = 12;

const int BETA_START  = 13;
const int BETA_END    = 30; // or 32 its just an assumptiong

const int eegPin = A0;

Servo s1, s2, s3, s4, s5;
const int P1 = 3;
const int P2 = 5;
const int P3 = 6;
const int P4 = 9;
const int P5 = 10;

const int OPEN_ANGLE  = 0;
const int CLOSE_ANGLE = 180;
int currentAngle = 90;

unsigned long microseconds;
unsigned int sampling_period_us;

void writeAll(int angle) {
  s1.write(angle);
  s2.write(angle);
  s3.write(angle);
  s4.write(angle);
  s5.write(angle);
}

void setup() {
  Serial.begin(115200);
  
  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));

  s1.attach(P1);
  s2.attach(P2);
  s3.attach(P3);
  s4.attach(P4);
  s5.attach(P5);

  writeAll(currentAngle);
  delay(1000);
}

void loop() {
  for (int i = 0; i < samples; i++) {
    microseconds = micros();
    vReal[i] = analogRead(eegPin);
    vImag[i] = 0;
    while (micros() < (microseconds + sampling_period_us)) {
    }
  }

  FFT.dcRemoval();
  
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  
  FFT.compute(FFT_FORWARD);
  
  FFT.complexToMagnitude();

  double alphaPower = 0;
  double betaPower = 0;

  for (int i = ALPHA_START; i <= ALPHA_END; i++) {
    alphaPower += vReal[i];
  }
  
  for (int i = BETA_START; i <= BETA_END; i++) {
    betaPower += vReal[i];
  }


  if (alphaPower > (betaPower * 1.1)) { 
    if (currentAngle != OPEN_ANGLE) {
      currentAngle = OPEN_ANGLE;
      writeAll(currentAngle);
    }
  }
  else if (betaPower > alphaPower) {
    if (currentAngle != CLOSE_ANGLE) {
      currentAngle = CLOSE_ANGLE;
      writeAll(currentAngle);
    }
  }
  Serial.print("Alpha:");
  Serial.print(alphaPower);
  Serial.print(" Beta:");
  Serial.println(betaPower);
}