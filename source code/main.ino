#include <ESP32Servo.h>

// -------- Ultrasonic Pins --------
#define TRIG_L 16
#define ECHO_L 4

#define TRIG_M 17
#define ECHO_M 5

#define TRIG_R 15
#define ECHO_R 2

// -------- Servo Pins --------
#define SERVO_H 26
#define SERVO_V 32
#define LASER_H 25
#define LASER_V 33

Servo servoH, servoV, laserH, laserV;

// -------- Variables --------
float posH = 60;
float posV = 45;
float smoothV = 45;

float laserPosH = 60;
float laserPosV = 45;

int dirH = 1;

int threshold = 70;
bool detected = false;

// -------- Distance --------
long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return 300;

  return duration * 0.034 / 2;
}

// -------- SYSTEM CHECK --------
void systemCheck() {

  Serial.println("=== SYSTEM CHECK ===");

  // Servo sweep test
  for (int i = 30; i <= 150; i += 5) {
    servoH.write(i);
    servoV.write(i % 90);
    laserH.write(i);
    laserV.write(i % 90);
    delay(25);
  }

  for (int i = 150; i >= 30; i -= 5) {
    servoH.write(i);
    servoV.write(i % 90);
    laserH.write(i);
    laserV.write(i % 90);
    delay(25);
  }

  // Sensor check
  long dL = getDistance(TRIG_L, ECHO_L);
  long dM = getDistance(TRIG_M, ECHO_M);
  long dR = getDistance(TRIG_R, ECHO_R);

  Serial.print("Sensors -> L:");
  Serial.print(dL);
  Serial.print(" M:");
  Serial.print(dM);
  Serial.print(" R:");
  Serial.println(dR);

  Serial.println("=== READY ===");
}

// -------- Setup --------
void setup() {

  Serial.begin(115200);

  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);

  pinMode(TRIG_M, OUTPUT);
  pinMode(ECHO_M, INPUT);

  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);

  servoH.attach(SERVO_H, 500, 2400);
  servoV.attach(SERVO_V, 500, 2400);
  laserH.attach(LASER_H, 500, 2400);
  laserV.attach(LASER_V, 500, 2400);

  servoV.write(45);
  laserV.write(45);

  systemCheck();
}

// -------- Loop --------
void loop() {

  long dL = getDistance(TRIG_L, ECHO_L);
  delay(20);
  long dM = getDistance(TRIG_M, ECHO_M);
  delay(20);
  long dR = getDistance(TRIG_R, ECHO_R);

  detected = (dL < threshold || dM < threshold || dR < threshold);

  // ================= TARGET MODE =================
  if (detected) {

    Serial.println("[TARGET FOUND]");

    // Stop horizontal movement
    servoH.write(posH);

    // -------- Laser horizontal (-20 offset) --------
    laserPosH = posH - 20;
    laserPosH = constrain(laserPosH, 0, 180);
    laserH.write(laserPosH);

    // -------- Vertical control (smooth) --------
    float targetV;

    if (dM < 20) targetV = 60;
    else if (dM > 40) targetV = 35;
    else targetV = 45;

    smoothV = smoothV + (targetV - smoothV) * 0.1;
    servoV.write(smoothV);

    // -------- Laser vertical (+20 offset) --------
    laserPosV = smoothV + 20;
    laserPosV = constrain(laserPosV, 0, 90);
    laserV.write(laserPosV);

    Serial.print("COORD -> H:");
    Serial.print(posH);
    Serial.print(" V:");
    Serial.print(smoothV);
    Serial.print(" LaserH:");
    Serial.print(laserPosH);
    Serial.print(" LaserV:");
    Serial.print(laserPosV);
    Serial.print(" Dist:");
    Serial.println(dM);

    delay(30);
    return;
  }

  // ================= SCAN MODE =================
  Serial.println("[SCANNING]");

  // Horizontal sweep (120°)
  posH += dirH * 1.2;

  if (posH >= 150) { posH = 150; dirH = -1; }
  if (posH <= 30)  { posH = 30;  dirH = 1; }

  servoH.write(posH);

  // Vertical fixed
  servoV.write(45);
  laserV.write(45);

  // Laser horizontal follows scan (optional but useful)
  laserPosH = posH - 20;
  laserPosH = constrain(laserPosH, 0, 180);
  laserH.write(laserPosH);

  Serial.print("SCAN -> H:");
  Serial.print(posH);
  Serial.print(" V:45 Dist:");
  Serial.println(dM);

  delay(30);
}