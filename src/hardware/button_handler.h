/**
 * Button Handler for ESP32 Simon Says
 *
 * Provides debounced button input handling with interrupt support.
 * Tracks button states, detects presses/releases, and handles multi-button detection.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "gpio_config.h"
#include "../config.h"

class ButtonHandler {
public:
    /**
     * Constructor - initializes button handler
     */
    ButtonHandler();

    /**
     * Initialize all button pins
     * Must be called in setup() before reading buttons
     */
    void begin();

    /**
     * Update button states (call this in main loop)
     * Handles debouncing and state transitions
     */
    void update();

    /**
     * Check if a button is currently pressed (debounced)
     *
     * Args:
     *     color: Color enum value
     *
     * Returns:
     *     bool: true if button is pressed, false otherwise
     */
    bool isPressed(Color color);

    /**
     * Check if a button was just pressed (state changed from not pressed to pressed)
     *
     * Args:
     *     color: Color enum value
     *
     * Returns:
     *     bool: true if button was just pressed, false otherwise
     */
    bool wasPressed(Color color);

    /**
     * Check if a button was just released (state changed from pressed to not pressed)
     *
     * Args:
     *     color: Color enum value
     *
     * Returns:
     *     bool: true if button was just released, false otherwise
     */
    bool wasReleased(Color color);

    /**
     * Check if any button is currently pressed
     *
     * Returns:
     *     Color: Color of pressed button, or NONE if no button pressed
     */
    Color getPressed();

    /**
     * Check if any button was just pressed
     *
     * Returns:
     *     Color: Color of button that was just pressed, or NONE if none
     */
    Color getJustPressed();

    /**
     * Wait for any button press with timeout
     *
     * Args:
     *     timeoutMs: Maximum time to wait (milliseconds), 0 = wait forever
     *
     * Returns:
     *     Color: Color of pressed button, or NONE if timeout
     */
    Color waitForPress(uint32_t timeoutMs = 0);

    /**
     * Wait for a specific button press with timeout
     *
     * Args:
     *     color: Expected button color
     *     timeoutMs: Maximum time to wait (milliseconds), 0 = wait forever
     *
     * Returns:
     *     bool: true if correct button pressed, false if timeout
     */
    bool waitForSpecificPress(Color color, uint32_t timeoutMs = 0);

    /**
     * Check if power button is pressed
     *
     * Returns:
     *     bool: true if power button is pressed
     */
    bool isPowerButtonPressed();

    /**
     * Check if power button is long-pressed (> 2 seconds)
     *
     * Returns:
     *     bool: true if power button held for long press duration
     */
    bool isPowerButtonLongPressed();

    /**
     * Clear all button states (useful after game over or mode change)
     */
    void clearAll();

    /**
     * Get the time (in milliseconds) since a button was last pressed
     *
     * Args:
     *     color: Color enum value
     *
     * Returns:
     *     uint32_t: Milliseconds since last press, or 0 if never pressed
     */
    uint32_t getTimeSincePress(Color color);

private:
    // Button state tracking
    struct ButtonState {
        bool current;          // Current debounced state
        bool previous;         // Previous state for edge detection
        bool raw;              // Raw GPIO reading
        uint32_t lastChangeTime;  // Time of last state change
        uint32_t lastPressTime;   // Time of last press event
    };

    ButtonState buttonStates[NUM_COLORS];  // State for game buttons
    ButtonState powerButtonState;           // State for power button

    /**
     * Read raw button state from GPIO
     *
     * Args:
     *     pin: GPIO pin number
     *
     * Returns:
     *     bool: true if button pressed (pin is LOW due to active-low design)
     */
    bool readRawButton(uint8_t pin);

    /**
     * Update state for a single button
     *
     * Args:
     *     state: Pointer to ButtonState structure
     *     pin: GPIO pin number
     */
    void updateButtonState(ButtonState* state, uint8_t pin);
};
