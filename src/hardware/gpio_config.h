/**
 * GPIO Pin Configuration for ESP32 Simon Says
 *
 * Defines all GPIO pin assignments for the ESP32-WROOM-32 board.
 * This includes LED outputs, button inputs, audio output, and power management pins.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>

// ============================================================================
// LED OUTPUT PINS (Active HIGH with 330Ω current limiting resistors)
// ============================================================================

#define GPIO_LED_RED     26
#define GPIO_LED_GREEN   33
#define GPIO_LED_BLUE    25
#define GPIO_LED_YELLOW  32

// LED PWM channels (ESP32 has 16 PWM channels, we use 0-3)
#define PWM_CHANNEL_RED     0
#define PWM_CHANNEL_GREEN   1
#define PWM_CHANNEL_BLUE    2
#define PWM_CHANNEL_YELLOW  3

// ============================================================================
// BUTTON INPUT PINS (Active LOW with internal pull-ups enabled)
// ============================================================================

#define GPIO_BTN_RED     13
#define GPIO_BTN_GREEN   14
#define GPIO_BTN_BLUE    12
#define GPIO_BTN_YELLOW  27

// ============================================================================
// AUDIO OUTPUT PIN (PWM capable for tone generation)
// ============================================================================

#define GPIO_SPEAKER     23
#define PWM_CHANNEL_SPEAKER 4  // Separate PWM channel for audio

// ============================================================================
// POWER MANAGEMENT PINS
// ============================================================================

#define GPIO_POWER_BTN   15  // Power button (has internal pull-up, can leave disconnected for testing)
#define GPIO_BATTERY_ADC 36  // VP pin for battery voltage monitoring via ADC

// ============================================================================
// STATUS LED (Built-in on ESP32 DevKit)
// ============================================================================

#define GPIO_STATUS_LED  2   // Built-in LED (useful for debugging/status)

// ============================================================================
// PIN ARRAYS FOR EASY ITERATION
// ============================================================================

// Array of LED pins in color order [Red, Green, Blue, Yellow]
const uint8_t LED_PINS[] = {
    GPIO_LED_RED,
    GPIO_LED_GREEN,
    GPIO_LED_BLUE,
    GPIO_LED_YELLOW
};

// Array of LED PWM channels corresponding to LED_PINS
const uint8_t LED_PWM_CHANNELS[] = {
    PWM_CHANNEL_RED,
    PWM_CHANNEL_GREEN,
    PWM_CHANNEL_BLUE,
    PWM_CHANNEL_YELLOW
};

// Array of button pins in color order [Red, Green, Blue, Yellow]
const uint8_t BUTTON_PINS[] = {
    GPIO_BTN_RED,
    GPIO_BTN_GREEN,
    GPIO_BTN_BLUE,
    GPIO_BTN_YELLOW
};

// Number of colors/buttons/LEDs
#define NUM_COLORS 4

// ============================================================================
// COLOR ENUMERATION
// ============================================================================

enum Color : uint8_t {
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    YELLOW = 3,
    NONE = 255  // Special value for "no color"
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Convert Color enum to human-readable string
 *
 * Args:
 *     color: Color enum value
 *
 * Returns:
 *     const char*: String representation of the color
 */
inline const char* colorToString(Color color) {
    switch (color) {
        case RED:    return "Red";
        case GREEN:  return "Green";
        case BLUE:   return "Blue";
        case YELLOW: return "Yellow";
        default:     return "None";
    }
}

/**
 * Get LED pin for a given color
 *
 * Args:
 *     color: Color enum value
 *
 * Returns:
 *     uint8_t: GPIO pin number for the LED
 */
inline uint8_t getLEDPin(Color color) {
    if (color < NUM_COLORS) {
        return LED_PINS[color];
    }
    return 0;
}

/**
 * Get PWM channel for a given color's LED
 *
 * Args:
 *     color: Color enum value
 *
 * Returns:
 *     uint8_t: PWM channel number
 */
inline uint8_t getLEDPWMChannel(Color color) {
    if (color < NUM_COLORS) {
        return LED_PWM_CHANNELS[color];
    }
    return 0;
}

/**
 * Get button pin for a given color
 *
 * Args:
 *     color: Color enum value
 *
 * Returns:
 *     uint8_t: GPIO pin number for the button
 */
inline uint8_t getButtonPin(Color color) {
    if (color < NUM_COLORS) {
        return BUTTON_PINS[color];
    }
    return 0;
}
