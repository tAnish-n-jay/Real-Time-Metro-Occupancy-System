#define BLYNK_TEMPLATE_ID "BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Coach Occupancy"
#define BLYNK_AUTH_TOKEN "BLYNK_AUTH_TOKEN"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define DOUT 18
#define SCK  19
#define BUZZER 23

#define MAX_CAPACITY 10
#define AVG_WEIGHT 0.065

char ssid[] = "Tanish";
char pass[] = "password";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HX711 scale;
BlynkTimer timer;

void sendToBlynk() {
  if (scale.is_ready()) {
    float weight = scale.get_units(10);
    if (weight < 0.010) weight = 0;

    int passengers = (int)(weight / AVG_WEIGHT);
    int occupancy = (passengers * 100) / MAX_CAPACITY;
    if (occupancy > 100) occupancy = 100;

    Blynk.virtualWrite(V0, weight);
    Blynk.virtualWrite(V1, passengers);
    Blynk.virtualWrite(V2, occupancy);

    static unsigned long lastNotify = 0;
    if (millis() - lastNotify >= 120000) {
      String msg = "Coach 01 | P:" + String(passengers) +
                   " | Occ:" + String(occupancy) + "%";
      Blynk.logEvent("occupancy_alert", msg);
      lastNotify = millis();
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("C01 P:");
    display.print(passengers);
    display.setCursor(90, 0);
    if (occupancy >= 80) {
      display.print("FULL!");
    } else {
      display.print("NORM");
    }

    display.setCursor(0, 16);
    display.print("Weight: ");
    display.print(weight, 3);
    display.print(" kg");

    display.setCursor(0, 32);
    display.print("Occ: ");
    display.print(occupancy);
    display.print("%");

    if (occupancy >= 100) {
      display.setCursor(70, 32);
      display.print("!! FULL");
    }

    display.drawRect(0, 54, 128, 10, WHITE);
    int barWidth = (occupancy * 124) / 100;
    display.fillRect(2, 56, barWidth, 6, WHITE);
    display.display();

    Serial.print("Weight: ");
    Serial.print(weight, 3);
    Serial.print(" kg | P: ");
    Serial.print(passengers);
    Serial.print(" | Occ: ");
    Serial.print(occupancy);
    Serial.println("%");

    if (occupancy >= 100) {
      digitalWrite(BUZZER, HIGH);
      delay(300);
      digitalWrite(BUZZER, LOW);
      delay(300);
    } else {
      digitalWrite(BUZZER, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER, OUTPUT);
  
  digitalWrite(BUZZER, LOW);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println("Connecting WiFi...");
  display.display();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  display.clearDisplay();
  display.setCursor(20, 20);
  display.println("WiFi Connected!");
  display.display();
  delay(1000);

  scale.begin(DOUT, SCK);
  scale.set_scale(-500448);
  scale.tare();

  display.clearDisplay();
  display.setCursor(20, 20);
  display.println("System Ready!");
  display.display();
  delay(1000);

  timer.setInterval(500L, sendToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}