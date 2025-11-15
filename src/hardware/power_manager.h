/**
 * Power Manager for ESP32 Simon Says
 *
 * Handles battery voltage monitoring, low battery warnings,
 * and deep sleep power management for extended battery life.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "gpio_config.h"
#include "../config.h"

// Battery status enumeration
enum BatteryStatus {
    BATTERY_GOOD,      // Battery voltage is good
    BATTERY_LOW,       // Battery voltage is low (warning)
    BATTERY_CRITICAL   // Battery voltage is critical (shutdown soon)
};

class PowerManager {
public:
    /**
     * Constructor - initializes power manager
     */
    PowerManager();

    /**
     * Initialize power management hardware
     * Must be called in setup()
     */
    void begin();

    /**
     * Update power management state
     * Call this regularly in main loop
     */
    void update();

    /**
     * Read battery voltage in millivolts
     *
     * Returns:
     *     uint16_t: Battery voltage in mV
     */
    uint16_t getBatteryVoltage();

    /**
     * Get battery percentage estimate (0-100%)
     *
     * Returns:
     *     uint8_t: Battery charge percentage
     */
    uint8_t getBatteryPercentage();

    /**
     * Get current battery status
     *
     * Returns:
     *     BatteryStatus: Current battery state
     */
    BatteryStatus getBatteryStatus();

    /**
     * Check if battery is low
     *
     * Returns:
     *     bool: true if battery is low or critical
     */
    bool isBatteryLow();

    /**
     * Enter deep sleep mode
     * ESP32 will wake up on button press or power button
     */
    void enterDeepSleep();

    /**
     * Check if device should enter deep sleep due to inactivity
     * Call this regularly to enable auto-sleep
     */
    void checkSleepTimeout();

    /**
     * Reset the inactivity timer
     * Call this whenever user interacts with the device
     */
    void resetActivityTimer();

    /**
     * Get time since last activity (in milliseconds)
     *
     * Returns:
     *     uint32_t: Milliseconds since last activity
     */
    uint32_t getTimeSinceActivity();

    /**
     * Enable or disable deep sleep feature
     *
     * Args:
     *     enabled: true to enable deep sleep, false to disable
     */
    void setDeepSleepEnabled(bool enabled);

    /**
     * Check if deep sleep is enabled
     *
     * Returns:
     *     bool: true if deep sleep enabled
     */
    bool isDeepSleepEnabled() const;

private:
    uint32_t lastActivityTime;      // Timestamp of last user activity
    uint32_t lastBatteryCheckTime;  // Timestamp of last battery check
    uint16_t lastBatteryVoltage;    // Last measured battery voltage (mV)
    BatteryStatus currentStatus;    // Current battery status
    bool deepSleepEnabled;          // Deep sleep feature flag

    /**
     * Read raw ADC value from battery monitoring pin
     *
     * Returns:
     *     uint16_t: Raw ADC value (0-4095 for 12-bit ADC)
     */
    uint16_t readBatteryADC();

    /**
     * Convert ADC reading to voltage in millivolts
     *
     * Args:
     *     adcValue: Raw ADC reading
     *
     * Returns:
     *     uint16_t: Voltage in millivolts
     */
    uint16_t adcToVoltage(uint16_t adcValue);

    /**
     * Estimate battery percentage from voltage
     * Assumes 3x AAA alkaline batteries (4.8V fresh, 3.6V depleted)
     *
     * Args:
     *     voltageMs: Battery voltage in millivolts
     *
     * Returns:
     *     uint8_t: Estimated percentage (0-100)
     */
    uint8_t voltageToPercentage(uint16_t voltageMv);

    /**
     * Update battery status based on voltage
     */
    void updateBatteryStatus();

    /**
     * Configure wake-up sources for deep sleep
     */
    void configureWakeup();
};
