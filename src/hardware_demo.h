/**
 * Hardware Demo and Test Routines for ESP32 Simon Says
 *
 * Provides comprehensive testing routines to validate all hardware connections:
 * - LEDs (all colors)
 * - Buttons (all colors + power button)
 * - Speaker (frequency modulation and tones)
 * - Power management (battery voltage)
 *
 * This is used during initial setup to verify wiring and tune frequencies.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "config.h"
#include "hardware/gpio_config.h"
#include "hardware/led_controller.h"
#include "hardware/button_handler.h"
#include "hardware/audio_controller.h"
#include "hardware/power_manager.h"

class HardwareDemo {
public:
    /**
     * Constructor
     */
    HardwareDemo(LEDController* leds, ButtonHandler* buttons,
                 AudioController* audio, PowerManager* power);

    /**
     * Run complete hardware demo sequence
     * Tests all components in order
     */
    void runFullDemo();

    /**
     * Test all LEDs individually
     */
    void testLEDs();

    /**
     * Test LED brightness levels (PWM)
     */
    void testLEDBrightness();

    /**
     * Test LED animations
     */
    void testLEDAnimations();

    /**
     * Test all buttons
     * Lights up corresponding LED when button is pressed
     */
    void testButtons();

    /**
     * Test speaker with various frequencies
     */
    void testSpeaker();

    /**
     * Test speaker frequency sweep
     * Useful for tuning and finding optimal frequencies
     */
    void testFrequencySweep();

    /**
     * Test volume levels
     */
    void testVolumeControl();

    /**
     * Test integrated button + LED + sound
     * Press button → light LED + play tone
     */
    void testIntegrated();

    /**
     * Test power management and battery monitoring
     */
    void testPowerManagement();

    /**
     * Interactive frequency tuning mode
     * Use buttons to adjust frequency up/down
     */
    void interactiveFrequencyTuning();

    /**
     * Display main demo menu via Serial
     */
    void showMenu();

    /**
     * Run interactive demo mode
     * User can select tests via Serial monitor
     */
    void runInteractive();

private:
    LEDController* led;
    ButtonHandler* btn;
    AudioController* audio;
    PowerManager* pwr;

    /**
     * Print test header
     */
    void printHeader(const char* testName);

    /**
     * Print test separator
     */
    void printSeparator();

    /**
     * Wait for any key press in serial monitor
     */
    void waitForSerial();
};
