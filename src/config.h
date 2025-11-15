/**
 * Global Configuration for ESP32 Simon Says
 *
 * This file contains all global configuration constants, default settings,
 * and feature flags for the Simon Says game.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

// ============================================================================
// VERSION INFORMATION
// ============================================================================
#define VERSION "1.0.0"
#define PROJECT_NAME "ESP32 Simon Says"

// ============================================================================
// SERIAL DEBUG
// ============================================================================
#define SERIAL_BAUD_RATE 115200
#define DEBUG_ENABLED true

// Debug macro
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

// ============================================================================
// GAME SETTINGS
// ============================================================================

// Maximum sequence length across all difficulties
#define MAX_SEQUENCE_LENGTH 31

// Default difficulty mode (0=Easy, 1=Medium, 2=Hard, 3=Expert)
#define DEFAULT_DIFFICULTY 1

// Timeout for player input (milliseconds)
#define INPUT_TIMEOUT_MS 5000

// ============================================================================
// AUDIO SETTINGS
// ============================================================================

// Tone frequencies (in Hz) for each color
#define TONE_FREQ_RED    218   // Custom frequency
#define TONE_FREQ_GREEN  163   // Custom frequency
#define TONE_FREQ_BLUE   330   // Custom frequency
#define TONE_FREQ_YELLOW 276   // Custom frequency

// Special tones
#define TONE_FREQ_ERROR   100   // Low buzz for errors
#define TONE_FREQ_SUCCESS 1047  // C6 for success/high score

// Default tone duration (milliseconds)
#define TONE_DURATION_MS 400

// Default volume (0-100)
#define DEFAULT_VOLUME 80

// ============================================================================
// LED SETTINGS
// ============================================================================

// Default LED brightness (0-255)
#define DEFAULT_LED_BRIGHTNESS 200

// LED PWM settings
#define LED_PWM_FREQUENCY 5000
#define LED_PWM_RESOLUTION 8  // 8-bit resolution (0-255)

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

// Deep sleep timeout (milliseconds of inactivity)
#define DEEP_SLEEP_TIMEOUT_MS 120000  // 2 minutes

// Battery monitoring
#define BATTERY_CHECK_INTERVAL_MS 60000  // Check every minute
#define BATTERY_LOW_VOLTAGE_MV 3800      // Warning threshold (3.8V)
#define BATTERY_CRITICAL_VOLTAGE_MV 3600 // Shutdown threshold (3.6V)

// ADC voltage divider for battery monitoring
// R1 = 10kΩ, R2 = 10kΩ (divides voltage by 2)
#define BATTERY_VOLTAGE_DIVIDER_RATIO 2.0

// ESP32 ADC reference voltage (in millivolts)
#define ADC_REFERENCE_VOLTAGE_MV 3300

// ADC resolution (12-bit = 4096 levels)
#define ADC_RESOLUTION 4096

// ============================================================================
// WIFI SETTINGS
// ============================================================================

// WiFi AP mode settings (fallback when no network configured)
#define WIFI_AP_SSID "SimonSays-Setup"
#define WIFI_AP_PASSWORD ""  // Open network for easy setup

// WiFiManager timeout (seconds)
#define WIFI_MANAGER_TIMEOUT_S 180  // 3 minutes

// Hostname for mDNS
#define WIFI_HOSTNAME "simon-says"

// ============================================================================
// WEB SERVER SETTINGS
// ============================================================================

// Web server port
#define WEB_SERVER_PORT 80

// WebSocket update interval (milliseconds)
#define WEBSOCKET_UPDATE_INTERVAL_MS 100

// Maximum number of WebSocket clients
#define MAX_WEBSOCKET_CLIENTS 4

// ============================================================================
// DATA STORAGE SETTINGS
// ============================================================================

// LittleFS paths
#define STORAGE_SCORES_FILE "/scores.json"
#define STORAGE_SETTINGS_FILE "/settings.json"
#define STORAGE_ANALYTICS_FILE "/analytics.json"

// Maximum number of high scores to store per difficulty
#define MAX_HIGH_SCORES_PER_DIFFICULTY 10

// Maximum number of analytics records to keep
#define MAX_ANALYTICS_RECORDS 100

// ============================================================================
// BUTTON DEBOUNCING
// ============================================================================

// Debounce delay (milliseconds)
#define BUTTON_DEBOUNCE_MS 50

// Long press threshold (milliseconds)
#define BUTTON_LONG_PRESS_MS 2000

// ============================================================================
// FEATURE FLAGS
// ============================================================================

// Enable/disable features
#define FEATURE_WEBSOCKET_ENABLED true
#define FEATURE_ANALYTICS_ENABLED true
#define FEATURE_DEEP_SLEEP_ENABLED true
#define FEATURE_BATTERY_MONITORING_ENABLED true
#define FEATURE_SOUND_ENABLED true

// Demo mode - set to true to run hardware demo instead of game
#define DEMO_MODE_ENABLED false

// ============================================================================
// DIFFICULTY MODE DEFINITIONS
// ============================================================================

// Note: DifficultySettings struct is defined in game/difficulty_modes.h
// Difficulty presets are defined here for easy configuration

// Difficulty preset constants (referenced by difficulty_modes.h)
#define DIFF_EASY_SPEED 800
#define DIFF_EASY_DURATION 600
#define DIFF_EASY_MAX_LENGTH 8
#define DIFF_EASY_WINDOW 3000

#define DIFF_MEDIUM_SPEED 600
#define DIFF_MEDIUM_DURATION 400
#define DIFF_MEDIUM_MAX_LENGTH 14
#define DIFF_MEDIUM_WINDOW 2000

#define DIFF_HARD_SPEED 400
#define DIFF_HARD_DURATION 250
#define DIFF_HARD_MAX_LENGTH 20
#define DIFF_HARD_WINDOW 1500

#define DIFF_EXPERT_SPEED 250
#define DIFF_EXPERT_DURATION 150
#define DIFF_EXPERT_MAX_LENGTH 31
#define DIFF_EXPERT_WINDOW 1000
