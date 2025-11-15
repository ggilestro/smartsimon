/**
 * Power Manager Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "power_manager.h"
#include <esp_sleep.h>

PowerManager::PowerManager() :
    lastActivityTime(0),
    lastBatteryCheckTime(0),
    lastBatteryVoltage(0),
    currentStatus(BATTERY_GOOD),
    deepSleepEnabled(FEATURE_DEEP_SLEEP_ENABLED) {
}

void PowerManager::begin() {
    DEBUG_PRINTLN("[POWER] Initializing power manager...");

    if (FEATURE_BATTERY_MONITORING_ENABLED) {
        // Configure ADC for battery monitoring
        pinMode(GPIO_BATTERY_ADC, INPUT);

        // Reason: ESP32 ADC needs attenuation setting for full voltage range
        // ADC_11db gives 0-3.3V range which works for our voltage divider
        analogSetAttenuation(ADC_11db);

        // Initial battery check
        lastBatteryVoltage = getBatteryVoltage();
        updateBatteryStatus();

        DEBUG_PRINTF("[POWER] Battery voltage: %d mV\n", lastBatteryVoltage);
        DEBUG_PRINTF("[POWER] Battery percentage: %d%%\n", getBatteryPercentage());
    }

    // Configure wake-up sources for deep sleep
    if (deepSleepEnabled) {
        configureWakeup();
    }

    // Initialize activity timer
    lastActivityTime = millis();

    DEBUG_PRINTLN("[POWER] Power manager initialized");
}

void PowerManager::update() {
    uint32_t now = millis();

    // Check battery periodically
    if (FEATURE_BATTERY_MONITORING_ENABLED &&
        (now - lastBatteryCheckTime >= BATTERY_CHECK_INTERVAL_MS)) {

        lastBatteryCheckTime = now;
        lastBatteryVoltage = getBatteryVoltage();
        updateBatteryStatus();

        DEBUG_PRINTF("[POWER] Battery: %d mV (%d%%)\n",
                    lastBatteryVoltage, getBatteryPercentage());

        // Warn if battery is low
        if (currentStatus == BATTERY_LOW) {
            DEBUG_PRINTLN("[POWER] WARNING: Battery is low!");
        } else if (currentStatus == BATTERY_CRITICAL) {
            DEBUG_PRINTLN("[POWER] CRITICAL: Battery critically low!");
        }
    }
}

uint16_t PowerManager::getBatteryVoltage() {
    if (!FEATURE_BATTERY_MONITORING_ENABLED) {
        return 4500;  // Return nominal voltage if monitoring disabled
    }

    // Take multiple readings and average them for stability
    const uint8_t numReadings = 10;
    uint32_t sum = 0;

    for (uint8_t i = 0; i < numReadings; i++) {
        sum += readBatteryADC();
        delay(10);
    }

    uint16_t avgAdc = sum / numReadings;
    return adcToVoltage(avgAdc);
}

uint8_t PowerManager::getBatteryPercentage() {
    return voltageToPercentage(lastBatteryVoltage);
}

BatteryStatus PowerManager::getBatteryStatus() {
    return currentStatus;
}

bool PowerManager::isBatteryLow() {
    return (currentStatus == BATTERY_LOW || currentStatus == BATTERY_CRITICAL);
}

void PowerManager::enterDeepSleep() {
    DEBUG_PRINTLN("[POWER] Entering deep sleep mode...");
    DEBUG_PRINTLN("[POWER] Press any button to wake up");

    delay(100);  // Allow serial output to complete

    // Enter deep sleep (wake on button press)
    esp_deep_sleep_start();

    // Code never reaches here - ESP32 resets on wake
}

void PowerManager::checkSleepTimeout() {
    if (!deepSleepEnabled) {
        return;
    }

    uint32_t timeSinceActivity = millis() - lastActivityTime;

    if (timeSinceActivity >= DEEP_SLEEP_TIMEOUT_MS) {
        DEBUG_PRINTF("[POWER] No activity for %d seconds, entering sleep\n",
                    DEEP_SLEEP_TIMEOUT_MS / 1000);
        enterDeepSleep();
    }
}

void PowerManager::resetActivityTimer() {
    lastActivityTime = millis();
}

uint32_t PowerManager::getTimeSinceActivity() {
    return millis() - lastActivityTime;
}

void PowerManager::setDeepSleepEnabled(bool enabled) {
    deepSleepEnabled = enabled;
    DEBUG_PRINTF("[POWER] Deep sleep %s\n", enabled ? "enabled" : "disabled");

    if (enabled) {
        configureWakeup();
    }
}

bool PowerManager::isDeepSleepEnabled() const {
    return deepSleepEnabled;
}

uint16_t PowerManager::readBatteryADC() {
    // Read ADC value from battery monitoring pin
    return analogRead(GPIO_BATTERY_ADC);
}

uint16_t PowerManager::adcToVoltage(uint16_t adcValue) {
    // Reason: Convert ADC reading to actual voltage
    // ADC is 12-bit (0-4095) representing 0-3.3V
    // Voltage divider divides battery voltage by 2
    // So: actualVoltage = (adcValue / 4095) * 3.3V * 2

    // Using integer math to avoid floating point:
    // voltage_mV = (adcValue * 3300 * 2) / 4095
    uint32_t voltage = ((uint32_t)adcValue * ADC_REFERENCE_VOLTAGE_MV * (uint32_t)BATTERY_VOLTAGE_DIVIDER_RATIO);
    voltage /= ADC_RESOLUTION;

    return (uint16_t)voltage;
}

uint8_t PowerManager::voltageToPercentage(uint16_t voltageMv) {
    // Reason: Estimate battery percentage from voltage
    // 3x AAA batteries: 4.8V (fresh) to 3.6V (depleted)
    // This is a rough linear approximation

    const uint16_t VOLTAGE_FULL = 4800;   // 3 x 1.6V (fresh)
    const uint16_t VOLTAGE_EMPTY = 3600;  // 3 x 1.2V (depleted)

    if (voltageMv >= VOLTAGE_FULL) {
        return 100;
    }
    if (voltageMv <= VOLTAGE_EMPTY) {
        return 0;
    }

    // Linear interpolation
    uint32_t percentage = ((uint32_t)(voltageMv - VOLTAGE_EMPTY) * 100);
    percentage /= (VOLTAGE_FULL - VOLTAGE_EMPTY);

    return constrain((uint8_t)percentage, 0, 100);
}

void PowerManager::updateBatteryStatus() {
    if (lastBatteryVoltage <= BATTERY_CRITICAL_VOLTAGE_MV) {
        currentStatus = BATTERY_CRITICAL;
    } else if (lastBatteryVoltage <= BATTERY_LOW_VOLTAGE_MV) {
        currentStatus = BATTERY_LOW;
    } else {
        currentStatus = BATTERY_GOOD;
    }
}

void PowerManager::configureWakeup() {
    // Reason: Configure ESP32 to wake from deep sleep on button press
    // For active-LOW buttons, we use ext0 wakeup on the power button
    // ext1 doesn't support ANY_LOW mode, only ALL_LOW or ANY_HIGH

    // Use ext0 wakeup on power button (single pin, wake on LOW)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_POWER_BTN, 0); // 0 = wake on LOW

    // Alternatively, any game button press will also work as interrupt
    // during normal operation (not deep sleep)

    DEBUG_PRINTF("[POWER] Wake-up source configured (power button on GPIO %d)\n", GPIO_POWER_BTN);
}
