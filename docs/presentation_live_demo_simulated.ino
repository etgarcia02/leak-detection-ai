#include <Arduino.h>

#define LED_PIN 25

const int REPORT_DELAY_MS = 1200;

struct SimulatedFrame {
  int waterLevelSound;
  const char* status;
  bool ledOn;
};

// First half = normal flow / no leak
// Second half = leak / stronger abnormal sound
SimulatedFrame FRAMES[] = {
  {1024, "NORMAL FLOW / NO LEAK", false},
  {1118, "NORMAL FLOW / NO LEAK", false},
  {1187, "NORMAL FLOW / NO LEAK", false},
  {1263, "NORMAL FLOW / NO LEAK", false},
  {2875, "POSSIBLE LEAK / STRONG SOUND", true},
  {3012, "POSSIBLE LEAK / STRONG SOUND", true},
  {3156, "POSSIBLE LEAK / STRONG SOUND", true},
  {3288, "POSSIBLE LEAK / STRONG SOUND", true}
};

const int FRAME_COUNT = sizeof(FRAMES) / sizeof(FRAMES[0]);
int frameIndex = 0;

void printFrame(const SimulatedFrame& frame) {
  digitalWrite(LED_PIN, frame.ledOn ? HIGH : LOW);

  Serial.print("Water Level Sound : ");
  Serial.println(frame.waterLevelSound);
  Serial.print("Status       : ");
  Serial.println(frame.status);
  Serial.print("LED State    : ");
  Serial.println(frame.ledOn ? "ON" : "OFF");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  printFrame(FRAMES[frameIndex]);
  frameIndex = (frameIndex + 1) % FRAME_COUNT;
  delay(REPORT_DELAY_MS);
}
