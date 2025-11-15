/**
 * WebSocket Handler for ESP32 Simon Says
 *
 * Provides real-time game state updates to all connected web clients.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include "../config.h"
#include "../hardware/gpio_config.h"
#include "../game/difficulty_modes.h"

class WebSocketHandler {
public:
    /**
     * Constructor
     *
     * Args:
     *     ws: Pointer to AsyncWebSocket instance
     */
    WebSocketHandler(AsyncWebSocket* ws);

    /**
     * Initialize WebSocket handler
     */
    void begin();

    /**
     * Handle WebSocket events
     */
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                 AwsEventType type, void *arg, uint8_t *data, size_t len);

    /**
     * Broadcast game state to all clients
     *
     * Args:
     *     state: Game state name
     *     score: Current score
     *     difficulty: Current difficulty
     */
    void broadcastGameState(const char* state, uint16_t score, DifficultyLevel difficulty);

    /**
     * Broadcast sequence being shown
     *
     * Args:
     *     sequence: Array of colors
     *     length: Sequence length
     */
    void broadcastSequence(const Color* sequence, uint8_t length);

    /**
     * Broadcast button press event
     *
     * Args:
     *     color: Button color pressed
     *     correct: Was it the correct button?
     */
    void broadcastButtonPress(Color color, bool correct);

    /**
     * Broadcast game over event
     *
     * Args:
     *     finalScore: Final score achieved
     *     isHighScore: Was it a high score?
     */
    void broadcastGameOver(uint16_t finalScore, bool isHighScore);

    /**
     * Broadcast player change (for multiplayer)
     *
     * Args:
     *     playerId: Current player ID
     *     playerName: Current player name
     */
    void broadcastPlayerChange(const String& playerId, const String& playerName);

    /**
     * Clean up disconnected clients
     */
    void cleanupClients();

    /**
     * Send JSON message to all clients
     *
     * Args:
     *     doc: JSON document to send
     */
    void broadcast(const JsonDocument& doc);

private:
    AsyncWebSocket* webSocket;
};
