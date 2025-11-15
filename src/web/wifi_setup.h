/**
 * WiFi Setup and Management for ESP32 Simon Says
 *
 * Handles WiFi connection using WiFiManager for easy configuration.
 * Provides captive portal for initial setup and automatic reconnection.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "../config.h"

class WiFiSetup {
public:
    /**
     * Constructor
     */
    WiFiSetup();

    /**
     * Initialize WiFi connection
     * Uses WiFiManager to create captive portal if not configured
     *
     * Returns:
     *     bool: true if connected successfully
     */
    bool begin();

    /**
     * Check if WiFi is connected
     *
     * Returns:
     *     bool: true if connected
     */
    bool isConnected();

    /**
     * Get IP address
     *
     * Returns:
     *     String: IP address or empty string if not connected
     */
    String getIPAddress();

    /**
     * Get mDNS hostname
     *
     * Returns:
     *     String: Hostname (e.g., "simon-says.local")
     */
    String getHostname();

    /**
     * Reset WiFi settings and restart configuration
     */
    void resetSettings();

    /**
     * Update WiFi connection status
     * Call periodically in loop()
     */
    void update();

    /**
     * Print connection info to serial
     */
    void printConnectionInfo();

private:
    WiFiManager wifiManager;
    bool connected;
    unsigned long lastCheckTime;
};
