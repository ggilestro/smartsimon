/**
 * LED Controller for ESP32 Simon Says
 *
 * Provides LED control with PWM brightness control and animation effects.
 * Supports individual LED control and synchronized effects across all LEDs.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "gpio_config.h"
#include "../config.h"

class LEDController {
public:
    /**
     * Constructor - initializes LED controller
     */
    LEDController();

    /**
     * Initialize all LED pins and PWM channels
     * Must be called in setup() before using LEDs
     */
    void begin();

    /**
     * Turn on LED for a specific color at full brightness
     *
     * Args:
     *     color: Color enum value
     */
    void on(Color color);

    /**
     * Turn off LED for a specific color
     *
     * Args:
     *     color: Color enum value
     */
    void off(Color color);

    /**
     * Set LED brightness with PWM (0-255)
     *
     * Args:
     *     color: Color enum value
     *     brightness: PWM value 0 (off) to 255 (full brightness)
     */
    void setBrightness(Color color, uint8_t brightness);

    /**
     * Turn off all LEDs
     */
    void allOff();

    /**
     * Turn on all LEDs at full brightness
     */
    void allOn();

    /**
     * Flash an LED on and off once
     *
     * Args:
     *     color: Color enum value
     *     duration: How long to keep LED on (milliseconds)
     */
    void flash(Color color, uint16_t duration);

    /**
     * Blink an LED multiple times
     *
     * Args:
     *     color: Color enum value
     *     count: Number of times to blink
     *     onTime: Duration LED stays on (milliseconds)
     *     offTime: Duration LED stays off (milliseconds)
     */
    void blink(Color color, uint8_t count, uint16_t onTime, uint16_t offTime);

    /**
     * Fade LED in from off to full brightness
     *
     * Args:
     *     color: Color enum value
     *     duration: Total fade duration (milliseconds)
     */
    void fadeIn(Color color, uint16_t duration);

    /**
     * Fade LED out from full brightness to off
     *
     * Args:
     *     color: Color enum value
     *     duration: Total fade duration (milliseconds)
     */
    void fadeOut(Color color, uint16_t duration);

    /**
     * Pulse LED (fade in then fade out)
     *
     * Args:
     *     color: Color enum value
     *     duration: Total pulse duration (milliseconds)
     */
    void pulse(Color color, uint16_t duration);

    /**
     * Startup animation - sequence through all colors
     */
    void startupAnimation();

    /**
     * Success animation - all LEDs flash together
     */
    void successAnimation();

    /**
     * Error animation - all LEDs rapid blink
     */
    void errorAnimation();

    /**
     * Set global brightness multiplier (0-255)
     * Affects all subsequent LED operations
     *
     * Args:
     *     brightness: Global brightness level (0-255)
     */
    void setGlobalBrightness(uint8_t brightness);

    /**
     * Get current global brightness setting
     *
     * Returns:
     *     uint8_t: Current global brightness (0-255)
     */
    uint8_t getGlobalBrightness() const;

private:
    uint8_t globalBrightness;  // Global brightness multiplier (0-255)

    /**
     * Apply global brightness to a brightness value
     *
     * Args:
     *     brightness: Input brightness (0-255)
     *
     * Returns:
     *     uint8_t: Adjusted brightness with global multiplier applied
     */
    uint8_t applyGlobalBrightness(uint8_t brightness);
};
