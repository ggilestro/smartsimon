/**
 * LED Controller Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "led_controller.h"

LEDController::LEDController() :
    globalBrightness(DEFAULT_LED_BRIGHTNESS) {
}

void LEDController::begin() {
    DEBUG_PRINTLN("[LED] Initializing LED controller...");

    // Configure all LED pins as outputs and setup PWM channels
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        pinMode(LED_PINS[i], OUTPUT);

        // Setup PWM channel for this LED
        // Reason: Using ledcSetup to configure PWM channel with frequency and resolution
        ledcSetup(LED_PWM_CHANNELS[i], LED_PWM_FREQUENCY, LED_PWM_RESOLUTION);

        // Attach the channel to the GPIO pin
        ledcAttachPin(LED_PINS[i], LED_PWM_CHANNELS[i]);

        // Start with LED off
        ledcWrite(LED_PWM_CHANNELS[i], 0);

        DEBUG_PRINTF("[LED] Configured %s LED on GPIO %d (PWM channel %d)\n",
                    colorToString((Color)i), LED_PINS[i], LED_PWM_CHANNELS[i]);
    }

    // Configure status LED
    pinMode(GPIO_STATUS_LED, OUTPUT);
    digitalWrite(GPIO_STATUS_LED, LOW);

    DEBUG_PRINTLN("[LED] All LEDs initialized");
}

void LEDController::on(Color color) {
    setBrightness(color, 255);
}

void LEDController::off(Color color) {
    setBrightness(color, 0);
}

void LEDController::setBrightness(Color color, uint8_t brightness) {
    if (color >= NUM_COLORS) {
        return;  // Invalid color
    }

    // Apply global brightness multiplier
    uint8_t adjustedBrightness = applyGlobalBrightness(brightness);

    // Write PWM value to the channel
    ledcWrite(LED_PWM_CHANNELS[color], adjustedBrightness);
}

void LEDController::allOff() {
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        off((Color)i);
    }
}

void LEDController::allOn() {
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        on((Color)i);
    }
}

void LEDController::flash(Color color, uint16_t duration) {
    on(color);
    delay(duration);
    off(color);
}

void LEDController::blink(Color color, uint8_t count, uint16_t onTime, uint16_t offTime) {
    for (uint8_t i = 0; i < count; i++) {
        on(color);
        delay(onTime);
        off(color);
        if (i < count - 1) {  // Don't delay after the last blink
            delay(offTime);
        }
    }
}

void LEDController::fadeIn(Color color, uint16_t duration) {
    // Reason: Calculate step delay to achieve smooth fade over the duration
    // 256 steps (0-255), so delay = duration / 256
    uint16_t stepDelay = duration / 256;
    if (stepDelay == 0) stepDelay = 1;  // Minimum 1ms delay

    for (uint16_t brightness = 0; brightness <= 255; brightness++) {
        setBrightness(color, brightness);
        delay(stepDelay);
    }
}

void LEDController::fadeOut(Color color, uint16_t duration) {
    uint16_t stepDelay = duration / 256;
    if (stepDelay == 0) stepDelay = 1;

    for (int16_t brightness = 255; brightness >= 0; brightness--) {
        setBrightness(color, brightness);
        delay(stepDelay);
    }
}

void LEDController::pulse(Color color, uint16_t duration) {
    uint16_t halfDuration = duration / 2;
    fadeIn(color, halfDuration);
    fadeOut(color, halfDuration);
}

void LEDController::startupAnimation() {
    DEBUG_PRINTLN("[LED] Playing startup animation");

    // Sequence through each color
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        flash((Color)i, 150);
        delay(50);
    }

    // All on briefly
    allOn();
    delay(200);
    allOff();
}

void LEDController::successAnimation() {
    DEBUG_PRINTLN("[LED] Playing success animation");

    // Flash all LEDs together 3 times
    for (uint8_t i = 0; i < 3; i++) {
        allOn();
        delay(150);
        allOff();
        delay(150);
    }
}

void LEDController::errorAnimation() {
    DEBUG_PRINTLN("[LED] Playing error animation");

    // Rapid blink all LEDs
    for (uint8_t i = 0; i < 5; i++) {
        allOn();
        delay(100);
        allOff();
        delay(100);
    }
}

void LEDController::setGlobalBrightness(uint8_t brightness) {
    globalBrightness = brightness;
    DEBUG_PRINTF("[LED] Global brightness set to %d\n", brightness);
}

uint8_t LEDController::getGlobalBrightness() const {
    return globalBrightness;
}

uint8_t LEDController::applyGlobalBrightness(uint8_t brightness) {
    // Reason: Calculate adjusted brightness using integer multiplication
    // to avoid floating point operations
    return (brightness * globalBrightness) / 255;
}
