/**
 * WebSocket Handler Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "websocket_handler.h"

WebSocketHandler::WebSocketHandler(AsyncWebSocket* ws) : webSocket(ws) {
}

void WebSocketHandler::begin() {
    DEBUG_PRINTLN("[WS] WebSocket handler initialized");
}

void WebSocketHandler::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            DEBUG_PRINTF("[WS] Client #%u connected from %s\n",
                        client->id(), client->remoteIP().toString().c_str());
            break;

        case WS_EVT_DISCONNECT:
            DEBUG_PRINTF("[WS] Client #%u disconnected\n", client->id());
            break;

        case WS_EVT_ERROR:
            DEBUG_PRINTF("[WS] Client #%u error\n", client->id());
            break;

        case WS_EVT_DATA:
            // We don't expect data from clients (broadcast only)
            DEBUG_PRINTF("[WS] Received data from client #%u\n", client->id());
            break;

        case WS_EVT_PONG:
            // Pong response
            break;
    }
}

void WebSocketHandler::broadcastGameState(const char* state, uint16_t score, DifficultyLevel difficulty) {
    StaticJsonDocument<256> doc;
    doc["type"] = "gameState";
    doc["state"] = state;
    doc["score"] = score;
    doc["difficulty"] = getDifficultyName(difficulty);
    doc["timestamp"] = millis();

    broadcast(doc);
}

void WebSocketHandler::broadcastSequence(const Color* sequence, uint8_t length) {
    DynamicJsonDocument doc(512);
    doc["type"] = "sequence";
    doc["length"] = length;

    JsonArray arr = doc.createNestedArray("colors");
    for (uint8_t i = 0; i < length; i++) {
        arr.add(colorToString(sequence[i]));
    }

    broadcast(doc);
}

void WebSocketHandler::broadcastButtonPress(Color color, bool correct) {
    StaticJsonDocument<256> doc;
    doc["type"] = "buttonPress";
    doc["color"] = colorToString(color);
    doc["correct"] = correct;
    doc["timestamp"] = millis();

    broadcast(doc);
}

void WebSocketHandler::broadcastGameOver(uint16_t finalScore, bool isHighScore) {
    StaticJsonDocument<256> doc;
    doc["type"] = "gameOver";
    doc["score"] = finalScore;
    doc["highScore"] = isHighScore;
    doc["timestamp"] = millis();

    broadcast(doc);
}

void WebSocketHandler::broadcastPlayerChange(const String& playerId, const String& playerName) {
    StaticJsonDocument<256> doc;
    doc["type"] = "playerChange";
    doc["playerId"] = playerId;
    doc["playerName"] = playerName;

    broadcast(doc);
}

void WebSocketHandler::cleanupClients() {
    webSocket->cleanupClients();
}

void WebSocketHandler::broadcast(const JsonDocument& doc) {
    String json;
    serializeJson(doc, json);

    DEBUG_PRINTF("[WS] Broadcasting: %s\n", json.c_str());
    webSocket->textAll(json);
}
