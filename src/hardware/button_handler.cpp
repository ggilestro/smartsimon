/**
 * Button Handler Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "button_handler.h"

ButtonHandler::ButtonHandler() {
    // Initialize all button states to default
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        buttonStates[i].current = false;
        buttonStates[i].previous = false;
        buttonStates[i].raw = false;
        buttonStates[i].lastChangeTime = 0;
        buttonStates[i].lastPressTime = 0;
    }

    powerButtonState.current = false;
    powerButtonState.previous = false;
    powerButtonState.raw = false;
    powerButtonState.lastChangeTime = 0;
    powerButtonState.lastPressTime = 0;
}

void ButtonHandler::begin() {
    DEBUG_PRINTLN("[BTN] Initializing button handler...");

    // Configure all game button pins as inputs with pull-ups
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        // Reason: Pins 34-39 are input-only and don't have internal pull-ups
        // So we check and handle them differently
        if (BUTTON_PINS[i] >= 34 && BUTTON_PINS[i] <= 39) {
            pinMode(BUTTON_PINS[i], INPUT);
            DEBUG_PRINTF("[BTN] Configured %s button on GPIO %d (input-only, external pull-up required)\n",
                        colorToString((Color)i), BUTTON_PINS[i]);
        } else {
            pinMode(BUTTON_PINS[i], INPUT_PULLUP);
            DEBUG_PRINTF("[BTN] Configured %s button on GPIO %d (with internal pull-up)\n",
                        colorToString((Color)i), BUTTON_PINS[i]);
        }
    }

    // Configure power button with internal pull-up
    pinMode(GPIO_POWER_BTN, INPUT_PULLUP);
    DEBUG_PRINTF("[BTN] Configured power button on GPIO %d (with internal pull-up)\n", GPIO_POWER_BTN);

    DEBUG_PRINTLN("[BTN] All buttons initialized");
}

void ButtonHandler::update() {
    // Update all game button states
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        updateButtonState(&buttonStates[i], BUTTON_PINS[i]);
    }

    // Update power button state
    updateButtonState(&powerButtonState, GPIO_POWER_BTN);
}

bool ButtonHandler::isPressed(Color color) {
    if (color >= NUM_COLORS) {
        return false;
    }
    return buttonStates[color].current;
}

bool ButtonHandler::wasPressed(Color color) {
    if (color >= NUM_COLORS) {
        return false;
    }
    return buttonStates[color].current && !buttonStates[color].previous;
}

bool ButtonHandler::wasReleased(Color color) {
    if (color >= NUM_COLORS) {
        return false;
    }
    return !buttonStates[color].current && buttonStates[color].previous;
}

Color ButtonHandler::getPressed() {
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        if (buttonStates[i].current) {
            return (Color)i;
        }
    }
    return NONE;
}

Color ButtonHandler::getJustPressed() {
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        if (wasPressed((Color)i)) {
            return (Color)i;
        }
    }
    return NONE;
}

Color ButtonHandler::waitForPress(uint32_t timeoutMs) {
    uint32_t startTime = millis();

    while (true) {
        update();

        // Check for any button press
        Color pressed = getJustPressed();
        if (pressed != NONE) {
            return pressed;
        }

        // Check timeout
        if (timeoutMs > 0 && (millis() - startTime >= timeoutMs)) {
            return NONE;
        }

        delay(10);  // Small delay to prevent tight loop
    }
}

bool ButtonHandler::waitForSpecificPress(Color color, uint32_t timeoutMs) {
    uint32_t startTime = millis();

    while (true) {
        update();

        // Check for the specific button press
        if (wasPressed(color)) {
            return true;
        }

        // Check timeout
        if (timeoutMs > 0 && (millis() - startTime >= timeoutMs)) {
            return false;
        }

        delay(10);
    }
}

bool ButtonHandler::isPowerButtonPressed() {
    return powerButtonState.current;
}

bool ButtonHandler::isPowerButtonLongPressed() {
    if (!powerButtonState.current) {
        return false;
    }

    // Check if button has been held for long press duration
    uint32_t pressDuration = millis() - powerButtonState.lastPressTime;
    return pressDuration >= BUTTON_LONG_PRESS_MS;
}

void ButtonHandler::clearAll() {
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        buttonStates[i].previous = buttonStates[i].current;
    }
    powerButtonState.previous = powerButtonState.current;
}

uint32_t ButtonHandler::getTimeSincePress(Color color) {
    if (color >= NUM_COLORS) {
        return 0;
    }

    if (buttonStates[color].lastPressTime == 0) {
        return 0;  // Never pressed
    }

    return millis() - buttonStates[color].lastPressTime;
}

bool ButtonHandler::readRawButton(uint8_t pin) {
    // Reason: Buttons are active-LOW (pressed = LOW, not pressed = HIGH)
    // So we invert the reading: LOW = pressed (true), HIGH = not pressed (false)
    return digitalRead(pin) == LOW;
}

void ButtonHandler::updateButtonState(ButtonState* state, uint8_t pin) {
    // Store previous state for edge detection
    state->previous = state->current;

    // Read raw button state
    state->raw = readRawButton(pin);

    uint32_t now = millis();

    // Debounce logic
    // Reason: Only update state if raw reading has been stable for debounce period
    if (state->raw != state->current) {
        // State is changing, check if debounce time has elapsed
        if (now - state->lastChangeTime >= BUTTON_DEBOUNCE_MS) {
            // Update debounced state
            state->current = state->raw;
            state->lastChangeTime = now;

            // Record press time
            if (state->current && !state->previous) {
                state->lastPressTime = now;
            }
        }
    } else {
        // Raw reading matches current state, reset change timer
        state->lastChangeTime = now;
    }
}
