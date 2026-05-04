#include <BleMouse.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h>

#define LEFTBUTTON  13
#define RIGHTBUTTON 12
#define SPEED 14

#define SDA_PIN 14
#define SCL_PIN 15

#define DEAD_ZONE     0.05f
#define SMOOTH_FACTOR 0.35f

Adafruit_MPU6050 mpu;
BleMouse bleMouse;

bool sleepMPU = true;
bool leftWasPressed = false;

float smoothX = 0.0f;
float smoothZ = 0.0f;

float applyDeadZone(float value, float threshold) {
  if (abs(value) < threshold) return 0.0f;
  return (value > 0) ? value - threshold : value + threshold;
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFTBUTTON, INPUT_PULLUP);
  pinMode(RIGHTBUTTON, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);
  bleMouse.begin();
  delay(1000);

  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Found!");

  mpu.enableSleep(sleepMPU);
  Serial.println("In attesa di connessione BLE...");
}

void loop() {
  if (!bleMouse.isConnected()) {
    delay(500);
    return; // aspetta connessione senza bloccare
  }

  // Prima connessione: sveglia MPU
  if (sleepMPU) {
    Serial.println("BLE connesso! Sveglio MPU6050...");
    delay(1000);
    sleepMPU = false;
    mpu.enableSleep(false);
    delay(500);
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float rawX = applyDeadZone(g.gyro.x, DEAD_ZONE);
  float rawZ = applyDeadZone(g.gyro.z, DEAD_ZONE);

  smoothX = (SMOOTH_FACTOR * rawX) + ((1.0f - SMOOTH_FACTOR) * smoothX);
  smoothZ = (SMOOTH_FACTOR * rawZ) + ((1.0f - SMOOTH_FACTOR) * smoothZ);

  int8_t moveX = (int8_t)constrain(smoothZ * -SPEED, -127, 127);
  int8_t moveY = (int8_t)constrain(smoothX * -SPEED, -127, 127);

  bleMouse.move(moveX, moveY);

  // Tasto sinistro — press e release separati
  bool leftPressed = !digitalRead(LEFTBUTTON);
  if (leftPressed && !leftWasPressed) {
    bleMouse.press(MOUSE_LEFT);
  } else if (!leftPressed && leftWasPressed) {
    bleMouse.release(MOUSE_LEFT);
  }
  leftWasPressed = leftPressed;

  // Tasto destro — click singolo
  if (!digitalRead(RIGHTBUTTON)) {
    bleMouse.click(MOUSE_RIGHT);
    delay(300);
  }

  delay(10); // ~100Hz
}