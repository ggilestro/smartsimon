/**
 * ESP32 Simon Says - Main Entry Point
 *
 * This is the main application file that initializes all subsystems
 * and runs the main game loop.
 *
 * To run hardware demo mode: Set DEMO_MODE_ENABLED to true in config.h
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include <Arduino.h>
#include "config.h"

// Hardware includes
#include "hardware/gpio_config.h"
#include "hardware/led_controller.h"
#include "hardware/button_handler.h"
#include "hardware/audio_controller.h"
#include "hardware/power_manager.h"

// Demo mode
#include "hardware_demo.h"

// Game includes
#include "game/simon_game.h"
#include "game/difficulty_modes.h"

// Web includes
#include "web/data_storage.h"
#include "web/wifi_setup.h"
#include "web/web_server.h"

// Global hardware objects
LEDController* ledController;
ButtonHandler* buttonHandler;
AudioController* audioController;
PowerManager* powerManager;

// Demo mode object
HardwareDemo* demo;

// Game objects
SimonGame* game;

// Web objects
DataStorage* storage;
WiFiSetup* wifiSetup;
SimonWebServer* webServer;

/**
 * Setup function - runs once at startup
 *
 * Initializes all hardware, WiFi, and game systems.
 */
void setup() {
    // Initialize serial for debugging
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);

    DEBUG_PRINTLN("\n\n========================================");
    DEBUG_PRINTLN(PROJECT_NAME);
    DEBUG_PRINT("Version: ");
    DEBUG_PRINTLN(VERSION);
    DEBUG_PRINTLN("========================================\n");

    // Initialize hardware subsystems
    DEBUG_PRINTLN("[INIT] Initializing hardware...");

    // Create hardware objects
    ledController = new LEDController();
    buttonHandler = new ButtonHandler();
    audioController = new AudioController();
    powerManager = new PowerManager();

    // Initialize each subsystem
    ledController->begin();
    buttonHandler->begin();
    audioController->begin();
    powerManager->begin();

    DEBUG_PRINTLN("[OK] Hardware initialized");

    // Check if running in demo mode
    #if DEMO_MODE_ENABLED
        DEBUG_PRINTLN("\n*** DEMO MODE ACTIVE ***");
        DEBUG_PRINTLN("Set DEMO_MODE_ENABLED to false in config.h to run the game\n");

        // Create demo object
        demo = new HardwareDemo(ledController, buttonHandler,
                               audioController, powerManager);

        // Play startup animation
        ledController->startupAnimation();
        audioController->playStartup();
        delay(500);

        // Run interactive demo menu
        demo->runInteractive();

    #else
        // Normal game mode initialization

        // Initialize storage first
        DEBUG_PRINTLN("[INIT] Initializing data storage...");
        storage = new DataStorage();
        if (!storage->begin()) {
            DEBUG_PRINTLN("[ERROR] Failed to initialize storage!");
        } else {
            DEBUG_PRINTLN("[OK] Storage initialized");
        }

        // Initialize game
        DEBUG_PRINTLN("[INIT] Initializing game...");
        game = new SimonGame(ledController, buttonHandler, audioController, storage);
        game->begin();
        DEBUG_PRINTLN("[OK] Game initialized");

        // Initialize WiFi
        DEBUG_PRINTLN("[INIT] Initializing WiFi...");
        wifiSetup = new WiFiSetup();
        if (!wifiSetup->begin()) {
            DEBUG_PRINTLN("[WARN] WiFi setup failed - continuing without WiFi");
        } else {
            DEBUG_PRINTLN("[OK] WiFi initialized");
            wifiSetup->printConnectionInfo();
        }

        // Initialize web server
        DEBUG_PRINTLN("[INIT] Initializing web server...");
        webServer = new SimonWebServer(storage, game);
        if (!webServer->begin()) {
            DEBUG_PRINTLN("[ERROR] Failed to start web server!");
        } else {
            DEBUG_PRINTLN("[OK] Web server started");

            // Pass WebSocket handler to game for real-time updates
            game->setWebSocketHandler(webServer->getWebSocketHandler());
        }

        // Play startup animation
        ledController->startupAnimation();
        audioController->playStartup();
        delay(500);

        DEBUG_PRINTLN("\n========================================");
        DEBUG_PRINTLN("SIMON SAYS - READY TO PLAY!");
        DEBUG_PRINTLN("========================================");
        DEBUG_PRINTF("Difficulty: %s\n", getDifficultyName(game->getDifficulty()));
        DEBUG_PRINTLN("\nPress any button to start!");
        DEBUG_PRINTLN("========================================\n");
    #endif
}

/**
 * Main loop - runs continuously
 *
 * Handles game logic, web server, and power management.
 */
void loop() {
    #if DEMO_MODE_ENABLED
        // Demo mode loop - everything handled in interactive menu
        delay(100);

    #else
        // Normal game mode loop

        // Update game state (includes button handling)
        game->update();

        // Update web server / WebSocket cleanup
        if (webServer) {
            webServer->update();
        }

        // Update WiFi connection
        if (wifiSetup) {
            wifiSetup->update();
        }

        // Update power management
        powerManager->update();

        // Check for deep sleep timeout (when idle)
        if (!game->isActive()) {
            powerManager->checkSleepTimeout();
        } else {
            // Reset activity timer when game is active
            powerManager->resetActivityTimer();
        }

        // Small delay to prevent watchdog timer issues
        delay(10);
    #endif
}
