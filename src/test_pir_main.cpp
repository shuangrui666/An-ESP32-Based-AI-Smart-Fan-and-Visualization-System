/**
 * @file test_pir_main.cpp
 * @brief SmartFan 2.0 - PIR raw signal read test
 * @version 0.1.0
 * @date 2026-11-20
 *
 * Purpose:
 * - Read the raw digital level from the PIR OUT signal on GPIO5.
 * - Print HIGH/LOW to Serial Monitor.
 * - Do not control the MOSFET or fan in this test.
 *
 * Safety:
 * - GPIO5 is not 5V tolerant.
 * - Do not upload/run this test with PIR OUT connected until its
 *   output-high voltage has been confirmed to be <= 3.3V.
 * - This file has not been hardware-tested.
 */

#include <Arduino.h>

static constexpr uint8_t PIR_OUT_PIN = 5;
static constexpr unsigned long SERIAL_BAUD = 115200;
static constexpr unsigned long SAMPLE_INTERVAL_MS = 200;
static constexpr unsigned long REPORT_INTERVAL_MS = 2000;

static unsigned long lastSampleTime = 0;
static unsigned long lastReportTime = 0;
static int currentPirLevel = LOW;
static int previousPirLevel = -1;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(300);

    // Use INPUT only; do not enable internal pull-up/down because the
    // PIR module is expected to provide its own digital output level.
    pinMode(PIR_OUT_PIN, INPUT);

    currentPirLevel = digitalRead(PIR_OUT_PIN);
    previousPirLevel = currentPirLevel;

    Serial.println();
    Serial.println("========================================");
    Serial.println(" SmartFan 2.0 - PIR Read Test");
    Serial.println("========================================");
    Serial.printf("[CONFIG] PIR OUT GPIO: %u\r\n", PIR_OUT_PIN);
    Serial.println("[INFO] Raw signal monitor only; fan/MOSFET control disabled.");
    Serial.println("[SAFETY] Run only after confirming PIR OUT <= 3.3V.");
    Serial.println("[PIR] Initial raw level:");
    Serial.println(currentPirLevel == HIGH ? "HIGH" : "LOW");
    Serial.println("========================================");
}

void loop() {
    const unsigned long now = millis();

    if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
        lastSampleTime = now;
        currentPirLevel = digitalRead(PIR_OUT_PIN);

        if (currentPirLevel != previousPirLevel) {
            Serial.printf("[PIR] GPIO%u changed to %s at %lu ms\r\n",
                          PIR_OUT_PIN,
                          currentPirLevel == HIGH ? "HIGH" : "LOW",
                          now);
            previousPirLevel = currentPirLevel;
        }
    }

    if (now - lastReportTime >= REPORT_INTERVAL_MS) {
        lastReportTime = now;
        Serial.printf("[PIR] Current raw level: %s | uptime: %lu s\r\n",
                      currentPirLevel == HIGH ? "HIGH" : "LOW",
                      now / 1000UL);
    }
}
