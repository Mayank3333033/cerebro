#include <Servo.h>


const int eegPin = A0;
Servo s1, s2, s3, s4, s5;
const int PINS[] = {3, 5, 6, 9, 10};

const int OPEN_ANGLE  = 0;
const int CLOSE_ANGLE = 180;
int currentAngle = 90;


const int TRIGGER_THRESHOLD = 50;

const int BUFFER_SIZE = 50;

void setup() {
  Serial.begin(115200);

  s1.attach(PINS[0]);
  s2.attach(PINS[1]);
  s3.attach(PINS[2]);
  s4.attach(PINS[3]);
  s5.attach(PINS[4]);

  writeAll(OPEN_ANGLE);
  delay(1000);
}

void loop() {

  long totalDeviation = 0;
  int baseline = 512;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    int val = analogRead(eegPin);


    totalDeviation += abs(val - baseline);

    delayMicroseconds(200);
  }

  int averageShake = totalDeviation / BUFFER_SIZE;


  if (averageShake > TRIGGER_THRESHOLD) {
    if (currentAngle != CLOSE_ANGLE) {
      currentAngle = CLOSE_ANGLE;
      writeAll(currentAngle);
    }
  }
  else {
    if (currentAngle != OPEN_ANGLE) {
      currentAngle = OPEN_ANGLE;
      writeAll(currentAngle);
    }
  }


  Serial.print(averageShake);      Serial.print(" ");
  Serial.println(TRIGGER_THRESHOLD);
}

void writeAll(int angle) {
  s1.write(angle);
  s2.write(angle);
  s3.write(angle);
  s4.write(angle);
  s5.write(angle);
}