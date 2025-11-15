/**
 * WiFi Setup Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "wifi_setup.h"

WiFiSetup::WiFiSetup() : connected(false), lastCheckTime(0) {
}

bool WiFiSetup::begin() {
    DEBUG_PRINTLN("[WIFI] Initializing WiFi...");

    // Set WiFi mode
    WiFi.mode(WIFI_STA);

    // Configure WiFiManager
    wifiManager.setConfigPortalTimeout(WIFI_MANAGER_TIMEOUT_S);

    // Try to connect
    // If it fails, it starts an access point with the specified name
    // and goes into a blocking loop awaiting configuration
    DEBUG_PRINTF("[WIFI] Starting WiFiManager (AP: %s)\n", WIFI_AP_SSID);

    if (wifiManager.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
        connected = true;

        DEBUG_PRINTLN("[WIFI] Connected to WiFi!");
        DEBUG_PRINTF("[WIFI] IP address: %s\n", WiFi.localIP().toString().c_str());
        DEBUG_PRINTF("[WIFI] SSID: %s\n", WiFi.SSID().c_str());

        // Setup mDNS
        if (MDNS.begin(WIFI_HOSTNAME)) {
            DEBUG_PRINTF("[WIFI] mDNS responder started: http://%s.local\n", WIFI_HOSTNAME);
            MDNS.addService("http", "tcp", 80);
        } else {
            DEBUG_PRINTLN("[WIFI] ERROR: mDNS failed to start");
        }

        return true;
    } else {
        DEBUG_PRINTLN("[WIFI] Failed to connect or timeout reached");
        connected = false;
        return false;
    }
}

bool WiFiSetup::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiSetup::getIPAddress() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "";
}

String WiFiSetup::getHostname() {
    return String(WIFI_HOSTNAME) + ".local";
}

void WiFiSetup::resetSettings() {
    DEBUG_PRINTLN("[WIFI] Resetting WiFi settings...");
    wifiManager.resetSettings();
    DEBUG_PRINTLN("[WIFI] WiFi settings reset. Restarting...");
    delay(1000);
    ESP.restart();
}

void WiFiSetup::update() {
    // Check WiFi connection every 10 seconds
    if (millis() - lastCheckTime > 10000) {
        lastCheckTime = millis();

        if (!isConnected() && connected) {
            DEBUG_PRINTLN("[WIFI] Connection lost!");
            connected = false;
        } else if (isConnected() && !connected) {
            DEBUG_PRINTLN("[WIFI] Connection restored!");
            connected = true;
        }
    }
}

void WiFiSetup::printConnectionInfo() {
    if (isConnected()) {
        DEBUG_PRINTLN("\n========================================");
        DEBUG_PRINTLN("WIFI CONNECTION INFO");
        DEBUG_PRINTLN("========================================");
        DEBUG_PRINTF("SSID: %s\n", WiFi.SSID().c_str());
        DEBUG_PRINTF("IP Address: %s\n", getIPAddress().c_str());
        DEBUG_PRINTF("mDNS: http://%s\n", getHostname().c_str());
        DEBUG_PRINTLN("========================================\n");
    } else {
        DEBUG_PRINTLN("[WIFI] Not connected");
    }
}
