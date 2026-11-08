/*
  Smart Pedestrian Walkway
  Arduino Nano + 4 IR obstacle sensors + 4 white LEDs

  Four LEDs act as street lights along a pedestrian walkway. At rest they
  glow dimly. When a pedestrian passes an IR sensor, the LED belonging to
  that sensor fades up to full brightness, stays bright for a short time
  after the last detection, then fades back to the dim level.

  Brightness is controlled with PWM (analogWrite), so every LED must be
  connected to a PWM-capable pin. On the Arduino Nano these are D3, D5,
  D6, D9, D10 and D11.
*/

const uint8_t NUM_LIGHTS = 4;

// One IR sensor and one LED per light position (left to right).
const uint8_t SENSOR_PINS[NUM_LIGHTS] = {2, 4, 7, 8};
const uint8_t LED_PINS[NUM_LIGHTS]    = {3, 5, 6, 9};   // PWM pins

// Most IR obstacle-sensor modules pull their output LOW when an object is
// detected. Set to false if your module outputs HIGH on detection.
const bool SENSOR_ACTIVE_LOW = true;

// Brightness levels (0 to 255).
const uint8_t DIM_LEVEL    = 30;    // idle brightness
const uint8_t BRIGHT_LEVEL = 255;   // brightness when a pedestrian is detected

// Timing.
const unsigned long HOLD_TIME_MS       = 1500;  // stay bright after the last detection
const unsigned long UPDATE_INTERVAL_MS = 10;    // how often the fade is updated

// Fade speed (brightness change per update).
const uint8_t FADE_UP_STEP   = 25;  // fast rise
const uint8_t FADE_DOWN_STEP = 3;   // slow fall

uint8_t brightness[NUM_LIGHTS];
unsigned long lastDetected[NUM_LIGHTS];
bool holding[NUM_LIGHTS];
unsigned long lastUpdate = 0;

bool personDetected(uint8_t i) {
  bool state = digitalRead(SENSOR_PINS[i]);
  return SENSOR_ACTIVE_LOW ? (state == LOW) : (state == HIGH);
}

void setup() {
  for (uint8_t i = 0; i < NUM_LIGHTS; i++) {
    pinMode(SENSOR_PINS[i], INPUT);
    pinMode(LED_PINS[i], OUTPUT);
    brightness[i] = DIM_LEVEL;
    lastDetected[i] = 0;
    holding[i] = false;
    analogWrite(LED_PINS[i], DIM_LEVEL);
  }
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate < UPDATE_INTERVAL_MS) {
    return;
  }
  lastUpdate = now;

  for (uint8_t i = 0; i < NUM_LIGHTS; i++) {
    // Update the detection state for this light.
    if (personDetected(i)) {
      lastDetected[i] = now;
      holding[i] = true;
    } else if (holding[i] && (now - lastDetected[i] >= HOLD_TIME_MS)) {
      holding[i] = false;
    }

    // Choose the target brightness and fade towards it.
    int target = holding[i] ? BRIGHT_LEVEL : DIM_LEVEL;
    int level = brightness[i];

    if (level < target) {
      level = min(level + FADE_UP_STEP, target);
    } else if (level > target) {
      level = max(level - FADE_DOWN_STEP, target);
    }

    brightness[i] = level;
    analogWrite(LED_PINS[i], level);
  }
}
