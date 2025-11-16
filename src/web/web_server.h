/**
 * Web Server for ESP32 Simon Says
 *
 * Manages AsyncWebServer, serves static files, and handles API endpoints.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include "../config.h"
#include "data_storage.h"
#include "websocket_handler.h"

// Forward declarations
class SimonGame;

class SimonWebServer {
public:
    /**
     * Constructor
     *
     * Args:
     *     storage: Data storage instance
     *     game: Game instance
     */
    SimonWebServer(DataStorage* storage, SimonGame* game);

    /**
     * Initialize and start web server
     *
     * Returns:
     *     bool: true if successful
     */
    bool begin();

    /**
     * Update web server (call in main loop)
     */
    void update();

    /**
     * Get WebSocket handler
     *
     * Returns:
     *     WebSocketHandler*: Pointer to WebSocket handler
     */
    WebSocketHandler* getWebSocketHandler();

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    DataStorage* storage;
    SimonGame* game;
    WebSocketHandler* wsHandler;

    /**
     * Setup all API routes
     */
    void setupRoutes();

    /**
     * Setup static file serving
     */
    void setupStaticFiles();

    // ========================================================================
    // API Endpoint Handlers
    // ========================================================================

    // Player endpoints
    void handleGetPlayers(AsyncWebServerRequest *request);
    void handleCreatePlayer(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleGetPlayer(AsyncWebServerRequest *request);
    void handleDeletePlayer(AsyncWebServerRequest *request);

    // Game control endpoints
    void handleGetGameStatus(AsyncWebServerRequest *request);
    void handleStartGame(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleStopGame(AsyncWebServerRequest *request);
    void handleSetPlayer(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleStartMultiplayer(AsyncWebServerRequest *request, uint8_t *data, size_t len);

    // Score endpoints
    void handleGetHighScores(AsyncWebServerRequest *request);
    void handleGetDifficultyScores(AsyncWebServerRequest *request);
    void handleGetRecentGames(AsyncWebServerRequest *request);
    void handleGetPlayerStats(AsyncWebServerRequest *request);

    // Settings endpoints
    void handleGetSettings(AsyncWebServerRequest *request);
    void handleUpdateSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);

    // Utility endpoints
    void handleFactoryReset(AsyncWebServerRequest *request);
    void handleGetStorageStats(AsyncWebServerRequest *request);
    void handleListFiles(AsyncWebServerRequest *request);
    void handleSetTime(AsyncWebServerRequest *request, uint8_t *data, size_t len);

    /**
     * Send JSON response
     *
     * Args:
     *     request: Web request
     *     doc: JSON document
     *     statusCode: HTTP status code
     */
    void sendJson(AsyncWebServerRequest *request, const JsonDocument& doc, int statusCode = 200);

    /**
     * Send error response
     *
     * Args:
     *     request: Web request
     *     message: Error message
     *     statusCode: HTTP status code
     */
    void sendError(AsyncWebServerRequest *request, const char* message, int statusCode = 400);
};
