/**
 * Web Server Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "web_server.h"
#include "../game/simon_game.h"

SimonWebServer::SimonWebServer(DataStorage* stor, SimonGame* gm) :
    server(WEB_SERVER_PORT),
    ws("/ws"),
    storage(stor),
    game(gm) {

    wsHandler = new WebSocketHandler(&ws);
}

bool SimonWebServer::begin() {
    DEBUG_PRINTLN("[WEB] Initializing web server...");

    // Initialize WebSocket handler
    wsHandler->begin();

    // Setup WebSocket event handler
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
        wsHandler->onEvent(server, client, type, arg, data, len);
    });

    // Add WebSocket to server
    server.addHandler(&ws);

    // Setup routes
    setupRoutes();
    setupStaticFiles();

    // Enable CORS for all responses
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // Start server
    server.begin();

    DEBUG_PRINTF("[WEB] Web server started on port %d\n", WEB_SERVER_PORT);
    return true;
}

void SimonWebServer::update() {
    // Clean up disconnected WebSocket clients
    wsHandler->cleanupClients();
}

WebSocketHandler* SimonWebServer::getWebSocketHandler() {
    return wsHandler;
}

void SimonWebServer::setupRoutes() {
    // Player endpoints
    server.on("/api/players", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetPlayers(request);
    });

    server.on("/api/players", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleCreatePlayer(request, data, len);
    });

    server.on("^\\/api\\/players\\/([a-z0-9\\-]+)$", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetPlayer(request);
    });

    server.on("^\\/api\\/players\\/([a-z0-9\\-]+)$", HTTP_DELETE, [this](AsyncWebServerRequest *request) {
        handleDeletePlayer(request);
    });

    // Game control endpoints
    server.on("/api/game/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetGameStatus(request);
    });

    server.on("/api/game/start", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleStartGame(request, data, len);
    });

    server.on("/api/game/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleStopGame(request);
    });

    server.on("/api/game/player", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleSetPlayer(request, data, len);
    });

    server.on("/api/game/multiplayer/start", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleStartMultiplayer(request, data, len);
    });

    // Score endpoints
    server.on("/api/scores/high", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetHighScores(request);
    });

    server.on("^\\/api\\/scores\\/difficulty\\/([0-3])$", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetDifficultyScores(request);
    });

    server.on("/api/scores/recent", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetRecentGames(request);
    });

    server.on("^\\/api\\/scores\\/player\\/([a-z0-9\\-]+)$", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetPlayerStats(request);
    });

    // Settings endpoints
    server.on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetSettings(request);
    });

    server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleUpdateSettings(request, data, len);
    });

    // Utility endpoints
    server.on("/api/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleFactoryReset(request);
    });

    server.on("/api/storage", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetStorageStats(request);
    });

    // Debug endpoint to list files in LittleFS
    server.on("/api/files", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleListFiles(request);
    });

    // Time sync endpoint
    server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        handleSetTime(request, data, len);
    });
}

void SimonWebServer::setupStaticFiles() {
    // Serve static files from LittleFS (but not /api routes)
    server.serveStatic("/", LittleFS, "/")
        .setDefaultFile("index.html")
        .setFilter([](AsyncWebServerRequest *request) {
            // Don't serve static files for /api routes
            return !request->url().startsWith("/api");
        });

    // 404 handler for everything else
    server.onNotFound([](AsyncWebServerRequest *request) {
        DEBUG_PRINTF("[WEB] 404 Not Found: %s\n", request->url().c_str());

        // Check if this is the root path and index.html doesn't exist
        if (request->url() == "/" && !LittleFS.exists("/index.html")) {
            String html = "<html><body><h1>Simon Says - Filesystem Not Uploaded</h1>";
            html += "<p>The web interface files have not been uploaded to the ESP32.</p>";
            html += "<p><strong>To fix this:</strong></p>";
            html += "<ol><li>Run: <code>pio run --target uploadfs</code></li>";
            html += "<li>Then reload this page</li></ol>";
            html += "<p>For debugging, check: <a href='/api/files'>/api/files</a></p>";
            html += "</body></html>";
            request->send(404, "text/html", html);
        } else {
            request->send(404, "text/plain", "Not found: " + request->url());
        }
    });
}

// ============================================================================
// Player Endpoints
// ============================================================================

void SimonWebServer::handleGetPlayers(AsyncWebServerRequest *request) {
    std::vector<Player> players = storage->getAllPlayers();

    DynamicJsonDocument doc(4096);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& p : players) {
        JsonObject obj = array.createNestedObject();
        obj["id"] = p.id;
        obj["name"] = p.name;
        obj["gamesPlayed"] = p.gamesPlayed;
        obj["avgScore"] = p.gamesPlayed > 0 ? (float)p.totalScore / p.gamesPlayed : 0;
        obj["bestScore"] = p.bestScore;
        obj["wins"] = p.wins;
        obj["created"] = p.created;
    }

    sendJson(request, doc);
}

void SimonWebServer::handleCreatePlayer(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendError(request, "Invalid JSON");
        return;
    }

    String name = doc["name"].as<String>();
    if (name.length() == 0) {
        sendError(request, "Name is required");
        return;
    }

    String playerId = storage->createPlayer(name);
    if (playerId.length() == 0) {
        sendError(request, "Failed to create player", 500);
        return;
    }

    StaticJsonDocument<256> response;
    response["success"] = true;
    response["playerId"] = playerId;
    response["name"] = name;

    sendJson(request, response, 201);
}

void SimonWebServer::handleGetPlayer(AsyncWebServerRequest *request) {
    String playerId = request->pathArg(0);
    Player player;

    if (!storage->getPlayer(playerId, player)) {
        sendError(request, "Player not found", 404);
        return;
    }

    StaticJsonDocument<512> doc;
    doc["id"] = player.id;
    doc["name"] = player.name;
    doc["gamesPlayed"] = player.gamesPlayed;
    doc["avgScore"] = player.gamesPlayed > 0 ? (float)player.totalScore / player.gamesPlayed : 0;
    doc["bestScore"] = player.bestScore;
    doc["wins"] = player.wins;
    doc["created"] = player.created;

    sendJson(request, doc);
}

void SimonWebServer::handleDeletePlayer(AsyncWebServerRequest *request) {
    String playerId = request->pathArg(0);

    if (!storage->deletePlayer(playerId)) {
        sendError(request, "Player not found", 404);
        return;
    }

    StaticJsonDocument<128> doc;
    doc["success"] = true;
    doc["message"] = "Player deleted";

    sendJson(request, doc);
}

// ============================================================================
// Game Control Endpoints
// ============================================================================

void SimonWebServer::handleGetGameStatus(AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;

    doc["state"] = (int)game->getState();
    doc["score"] = game->getScore();
    doc["highScore"] = game->getHighScore();
    doc["difficulty"] = getDifficultyName(game->getDifficulty());
    doc["isActive"] = game->isActive();

    sendJson(request, doc);
}

void SimonWebServer::handleStartGame(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendError(request, "Invalid JSON");
        return;
    }

    DifficultyLevel difficulty = (DifficultyLevel)doc["difficulty"].as<int>();
    if (difficulty >= NUM_DIFFICULTIES) {
        difficulty = EASY;
    }

    game->startGame(difficulty);

    StaticJsonDocument<128> response;
    response["success"] = true;
    response["message"] = "Game started";

    sendJson(request, response);
}

void SimonWebServer::handleStopGame(AsyncWebServerRequest *request) {
    game->reset();

    StaticJsonDocument<128> doc;
    doc["success"] = true;
    doc["message"] = "Game stopped";

    sendJson(request, doc);
}

void SimonWebServer::handleSetPlayer(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendError(request, "Invalid JSON");
        return;
    }

    String playerId = doc["playerId"].as<String>();

    // Verify player exists (unless empty for guest mode)
    if (playerId.length() > 0) {
        Player player;
        if (!storage->getPlayer(playerId, player)) {
            sendError(request, "Player not found", 404);
            return;
        }
        game->setCurrentPlayer(playerId);
    } else {
        // Empty string = guest mode
        game->setCurrentPlayer("");
    }

    StaticJsonDocument<128> response;
    response["success"] = true;
    response["message"] = "Current player set";
    response["playerId"] = playerId;

    sendJson(request, response);
}

void SimonWebServer::handleStartMultiplayer(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendError(request, "Invalid JSON");
        return;
    }

    // Game mode is always PASS_AND_PLAY for multiplayer
    GameMode mode = PASS_AND_PLAY;

    // Parse difficulty
    DifficultyLevel difficulty = (DifficultyLevel)doc["difficulty"].as<int>();
    if (difficulty >= NUM_DIFFICULTIES) {
        difficulty = EASY;
    }

    // Parse player IDs
    JsonArray playerIdsArray = doc["playerIds"].as<JsonArray>();
    if (playerIdsArray.size() < 2 || playerIdsArray.size() > 4) {
        sendError(request, "Must have 2-4 players");
        return;
    }

    String playerIds[4];
    uint8_t numPlayers = playerIdsArray.size();

    for (uint8_t i = 0; i < numPlayers; i++) {
        playerIds[i] = playerIdsArray[i].as<String>();

        // Verify player exists
        Player player;
        if (!storage->getPlayer(playerIds[i], player)) {
            String errorMsg = "Player not found: " + playerIds[i];
            sendError(request, errorMsg.c_str(), 404);
            return;
        }
    }

    // Start multiplayer game
    game->startMultiplayerGame(mode, playerIds, numPlayers, difficulty);

    StaticJsonDocument<256> response;
    response["success"] = true;
    response["message"] = "Multiplayer game started";
    response["numPlayers"] = numPlayers;

    sendJson(request, response);
}

// ============================================================================
// Score Endpoints
// ============================================================================

void SimonWebServer::handleGetHighScores(AsyncWebServerRequest *request) {
    std::vector<HighScore> scores = storage->getAllTimeHighScores(MAX_HIGH_SCORES_TOTAL);

    DynamicJsonDocument doc(2048);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& hs : scores) {
        JsonObject obj = array.createNestedObject();
        obj["playerId"] = hs.playerId;
        obj["playerName"] = hs.playerName;
        obj["score"] = hs.score;
        obj["difficulty"] = getDifficultyName(hs.difficulty);
        obj["timestamp"] = hs.timestamp;
    }

    sendJson(request, doc);
}

void SimonWebServer::handleGetDifficultyScores(AsyncWebServerRequest *request) {
    int difficultyInt = request->pathArg(0).toInt();
    DifficultyLevel difficulty = (DifficultyLevel)difficultyInt;

    if (difficulty >= NUM_DIFFICULTIES) {
        sendError(request, "Invalid difficulty");
        return;
    }

    std::vector<HighScore> scores = storage->getHighScores(difficulty, 10);

    DynamicJsonDocument doc(2048);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& hs : scores) {
        JsonObject obj = array.createNestedObject();
        obj["playerId"] = hs.playerId;
        obj["playerName"] = hs.playerName;
        obj["score"] = hs.score;
        obj["timestamp"] = hs.timestamp;
    }

    sendJson(request, doc);
}

void SimonWebServer::handleGetRecentGames(AsyncWebServerRequest *request) {
    std::vector<GameSession> games = storage->getRecentGames(MAX_GAME_HISTORY);

    DynamicJsonDocument doc(8192);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& g : games) {
        JsonObject obj = array.createNestedObject();
        obj["playerId"] = g.playerId;
        obj["playerName"] = g.playerName;
        obj["score"] = g.score;
        obj["difficulty"] = getDifficultyName(g.difficulty);
        obj["timestamp"] = g.timestamp;
        obj["duration"] = g.duration;
    }

    sendJson(request, doc);
}

void SimonWebServer::handleGetPlayerStats(AsyncWebServerRequest *request) {
    String playerId = request->pathArg(0);
    Player player;

    if (!storage->getPlayer(playerId, player)) {
        sendError(request, "Player not found", 404);
        return;
    }

    // Get player's recent games
    std::vector<GameSession> games = storage->getPlayerGames(playerId, 20);

    StaticJsonDocument<4096> doc;
    doc["id"] = player.id;
    doc["name"] = player.name;
    doc["gamesPlayed"] = player.gamesPlayed;
    doc["avgScore"] = player.gamesPlayed > 0 ? (float)player.totalScore / player.gamesPlayed : 0;
    doc["bestScore"] = player.bestScore;
    doc["wins"] = player.wins;

    JsonArray recentGames = doc.createNestedArray("recentGames");
    for (const auto& g : games) {
        JsonObject obj = recentGames.createNestedObject();
        obj["score"] = g.score;
        obj["difficulty"] = getDifficultyName(g.difficulty);
        obj["timestamp"] = g.timestamp;
    }

    sendJson(request, doc);
}

// ============================================================================
// Settings Endpoints
// ============================================================================

void SimonWebServer::handleGetSettings(AsyncWebServerRequest *request) {
    GameSettings settings = storage->loadSettings();

    StaticJsonDocument<512> doc;
    doc["difficulty"] = (int)settings.defaultDifficulty;
    doc["difficultyName"] = getDifficultyName(settings.defaultDifficulty);
    doc["volume"] = settings.volume;
    doc["ledBrightness"] = settings.ledBrightness;
    doc["soundEnabled"] = settings.soundEnabled;
    doc["deepSleepEnabled"] = settings.deepSleepEnabled;

    sendJson(request, doc);
}

void SimonWebServer::handleUpdateSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendError(request, "Invalid JSON");
        return;
    }

    GameSettings settings;
    settings.defaultDifficulty = (DifficultyLevel)doc["difficulty"].as<int>();
    settings.volume = doc["volume"].as<uint8_t>();
    settings.ledBrightness = doc["ledBrightness"].as<uint8_t>();
    settings.soundEnabled = doc["soundEnabled"].as<bool>();
    settings.deepSleepEnabled = doc["deepSleepEnabled"].as<bool>();

    if (!storage->saveSettings(settings)) {
        sendError(request, "Failed to save settings", 500);
        return;
    }

    StaticJsonDocument<128> response;
    response["success"] = true;
    response["message"] = "Settings saved";

    sendJson(request, response);
}

// ============================================================================
// Utility Endpoints
// ============================================================================

void SimonWebServer::handleFactoryReset(AsyncWebServerRequest *request) {
    storage->factoryReset();

    StaticJsonDocument<128> doc;
    doc["success"] = true;
    doc["message"] = "Factory reset complete";

    sendJson(request, doc);
}

void SimonWebServer::handleGetStorageStats(AsyncWebServerRequest *request) {
    size_t total, used;
    storage->getStorageStats(total, used);

    StaticJsonDocument<256> doc;
    doc["totalBytes"] = total;
    doc["usedBytes"] = used;
    doc["freeBytes"] = total - used;
    doc["usedPercent"] = (float)used / total * 100;

    sendJson(request, doc);
}

void SimonWebServer::handleListFiles(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(2048);
    JsonArray files = doc.createNestedArray("files");

    File root = LittleFS.open("/");
    if (!root) {
        sendError(request, "Failed to open filesystem root");
        return;
    }

    if (!root.isDirectory()) {
        sendError(request, "Root is not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        JsonObject fileObj = files.createNestedObject();
        fileObj["name"] = String(file.name());
        fileObj["size"] = file.size();
        fileObj["isDir"] = file.isDirectory();
        file = root.openNextFile();
    }

    size_t total, used;
    storage->getStorageStats(total, used);
    doc["totalFiles"] = files.size();
    doc["totalBytes"] = total;
    doc["usedBytes"] = used;

    sendJson(request, doc);
}

void SimonWebServer::handleSetTime(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error) {
        sendError(request, "Invalid JSON");
        return;
    }

    uint32_t timestamp = doc["timestamp"].as<uint32_t>();

    if (timestamp == 0) {
        sendError(request, "Invalid timestamp");
        return;
    }

    storage->setTimeOffset(timestamp);

    StaticJsonDocument<128> response;
    response["success"] = true;
    response["message"] = "Time synchronized";
    response["timestamp"] = timestamp;

    sendJson(request, response);
}

// ============================================================================
// Helper Methods
// ============================================================================

void SimonWebServer::sendJson(AsyncWebServerRequest *request, const JsonDocument& doc, int statusCode) {
    String json;
    serializeJson(doc, json);

    request->send(statusCode, "application/json", json);
}

void SimonWebServer::sendError(AsyncWebServerRequest *request, const char* message, int statusCode) {
    StaticJsonDocument<256> doc;
    doc["error"] = message;

    sendJson(request, doc, statusCode);
}
