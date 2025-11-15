/**
 * Data Storage Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "data_storage.h"

// File paths
const char* DataStorage::PLAYERS_FILE = "/players.json";
const char* DataStorage::HISTORY_FILE = "/history.json";
const char* DataStorage::SCORES_FILE = "/scores.json";
const char* DataStorage::SETTINGS_FILE = "/settings.json";

DataStorage::DataStorage() : initialized(false), timeOffsetSeconds(0) {
}

bool DataStorage::begin() {
    DEBUG_PRINTLN("[STORAGE] Initializing LittleFS...");

    if (!LittleFS.begin(true)) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to mount LittleFS");
        return false;
    }

    initialized = true;
    DEBUG_PRINTLN("[STORAGE] LittleFS mounted successfully");

    // Get storage stats
    size_t total, used;
    if (getStorageStats(total, used)) {
        DEBUG_PRINTF("[STORAGE] Total: %d bytes, Used: %d bytes, Free: %d bytes\n",
                    total, used, total - used);
    }

    // Initialize default settings file if it doesn't exist
    if (!LittleFS.exists(SETTINGS_FILE)) {
        DEBUG_PRINTLN("[STORAGE] Creating default settings file...");
        GameSettings defaults;
        defaults.defaultDifficulty = EASY;
        defaults.volume = DEFAULT_VOLUME;
        defaults.ledBrightness = DEFAULT_LED_BRIGHTNESS;
        defaults.soundEnabled = true;
        defaults.deepSleepEnabled = true;
        saveSettings(defaults);
    }

    return true;
}

void DataStorage::setTimeOffset(uint32_t unixTimestamp) {
    uint32_t currentMillis = millis() / 1000;
    timeOffsetSeconds = unixTimestamp - currentMillis;
    DEBUG_PRINTF("[STORAGE] Time offset set: %d seconds\n", timeOffsetSeconds);
    DEBUG_PRINTF("[STORAGE] Current timestamp: %d\n", getCurrentTimestamp());
}

uint32_t DataStorage::getCurrentTimestamp() const {
    if (timeOffsetSeconds == 0) {
        // Time not synced yet, return millis-based time
        return millis() / 1000;
    }
    return (millis() / 1000) + timeOffsetSeconds;
}

// ============================================================================
// Player Management
// ============================================================================

String DataStorage::createPlayer(const String& name) {
    if (!initialized) return "";

    DEBUG_PRINTF("[STORAGE] Creating player: %s\n", name.c_str());

    // Load existing players
    std::vector<Player> players = loadPlayers();

    // Check if we're at the limit
    if (players.size() >= MAX_PLAYERS) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Maximum players reached");
        return "";
    }

    // Create new player
    Player newPlayer;
    newPlayer.id = generateUUID();
    newPlayer.name = name;
    newPlayer.gamesPlayed = 0;
    newPlayer.totalScore = 0;
    newPlayer.bestScore = 0;
    newPlayer.wins = 0;
    newPlayer.created = getCurrentTimestamp();  // Use synced timestamp

    players.push_back(newPlayer);

    if (savePlayers(players)) {
        DEBUG_PRINTF("[STORAGE] Player created with ID: %s\n", newPlayer.id.c_str());

        // Verify the file was actually written
        if (LittleFS.exists(PLAYERS_FILE)) {
            File verifyFile = LittleFS.open(PLAYERS_FILE, "r");
            if (verifyFile) {
                DEBUG_PRINTF("[STORAGE] Verified: %s exists, size: %d bytes\n",
                            PLAYERS_FILE, verifyFile.size());
                verifyFile.close();
            }
        } else {
            DEBUG_PRINTF("[STORAGE] WARNING: %s was not created!\n", PLAYERS_FILE);
        }

        return newPlayer.id;
    }

    DEBUG_PRINTLN("[STORAGE] ERROR: Failed to save player!");
    return "";
}

bool DataStorage::getPlayer(const String& id, Player& player) {
    std::vector<Player> players = loadPlayers();

    for (const auto& p : players) {
        if (p.id == id) {
            player = p;
            return true;
        }
    }

    return false;
}

std::vector<Player> DataStorage::getAllPlayers() {
    return loadPlayers();
}

bool DataStorage::updatePlayer(const String& id, const Player& player) {
    std::vector<Player> players = loadPlayers();

    for (auto& p : players) {
        if (p.id == id) {
            p = player;
            return savePlayers(players);
        }
    }

    return false;
}

bool DataStorage::deletePlayer(const String& id) {
    std::vector<Player> players = loadPlayers();

    for (auto it = players.begin(); it != players.end(); ++it) {
        if (it->id == id) {
            players.erase(it);
            return savePlayers(players);
        }
    }

    return false;
}

// ============================================================================
// Game History
// ============================================================================

bool DataStorage::recordGame(const GameSession& session) {
    if (!initialized) return false;

    // Create a mutable copy to fill in missing data
    GameSession gameSession = session;

    // Fill in player name if we have a valid player ID
    if (gameSession.playerId.length() > 0 && gameSession.playerId != "guest") {
        Player player;
        if (getPlayer(gameSession.playerId, player)) {
            gameSession.playerName = player.name;
        } else {
            DEBUG_PRINTF("[STORAGE] WARNING: Player ID %s not found!\n", gameSession.playerId.c_str());
            gameSession.playerName = "Unknown";
        }
    } else {
        gameSession.playerName = "Guest";
    }

    // Use current timestamp (synced from browser)
    gameSession.timestamp = getCurrentTimestamp();

    DEBUG_PRINTF("[STORAGE] Recording game: Player=%s (%s), Score=%d, Time=%d\n",
                gameSession.playerName.c_str(), gameSession.playerId.c_str(),
                gameSession.score, gameSession.timestamp);

    // Load existing history
    std::vector<GameSession> history = loadHistory();

    // Add new session at the beginning
    history.insert(history.begin(), gameSession);

    // Keep only the most recent games
    if (history.size() > MAX_GAME_HISTORY) {
        history.resize(MAX_GAME_HISTORY);
    }

    // Save history
    if (!saveHistory(history)) {
        return false;
    }

    // Update player statistics (only for non-guest players)
    if (gameSession.playerId.length() > 0 && gameSession.playerId != "guest") {
        Player player;
        if (getPlayer(gameSession.playerId, player)) {
            player.gamesPlayed++;
            player.totalScore += gameSession.score;
            if (gameSession.score > player.bestScore) {
                player.bestScore = gameSession.score;
            }
            // Consider a "win" if they reached a certain threshold (e.g., score > 5)
            if (gameSession.score >= 5) {
                player.wins++;
            }
            updatePlayer(gameSession.playerId, player);
            DEBUG_PRINTF("[STORAGE] Updated player %s stats: games=%d, best=%d\n",
                        player.name.c_str(), player.gamesPlayed, player.bestScore);
        }
    }

    // Check if it's a high score
    addHighScore(gameSession);

    return true;
}

std::vector<GameSession> DataStorage::getRecentGames(uint8_t limit) {
    std::vector<GameSession> history = loadHistory();

    if (history.size() > limit) {
        history.resize(limit);
    }

    return history;
}

std::vector<GameSession> DataStorage::getPlayerGames(const String& playerId, uint8_t limit) {
    std::vector<GameSession> allGames = loadHistory();
    std::vector<GameSession> playerGames;

    for (const auto& game : allGames) {
        if (game.playerId == playerId) {
            playerGames.push_back(game);
            if (playerGames.size() >= limit) break;
        }
    }

    return playerGames;
}

// ============================================================================
// High Scores
// ============================================================================

std::vector<HighScore> DataStorage::getHighScores(DifficultyLevel difficulty, uint8_t limit) {
    std::vector<HighScore> allScores = loadHighScores();
    std::vector<HighScore> filteredScores;

    for (const auto& score : allScores) {
        if (score.difficulty == difficulty) {
            filteredScores.push_back(score);
        }
    }

    // Sort by score descending
    std::sort(filteredScores.begin(), filteredScores.end(),
        [](const HighScore& a, const HighScore& b) {
            return a.score > b.score;
        });

    if (filteredScores.size() > limit) {
        filteredScores.resize(limit);
    }

    return filteredScores;
}

std::vector<HighScore> DataStorage::getAllTimeHighScores(uint8_t limit) {
    std::vector<HighScore> allScores = loadHighScores();

    // Sort by score descending
    std::sort(allScores.begin(), allScores.end(),
        [](const HighScore& a, const HighScore& b) {
            return a.score > b.score;
        });

    if (allScores.size() > limit) {
        allScores.resize(limit);
    }

    return allScores;
}

bool DataStorage::addHighScore(const GameSession& session) {
    std::vector<HighScore> scores = loadHighScores();

    // Create high score entry
    HighScore newScore;
    newScore.playerId = session.playerId;
    newScore.playerName = session.playerName;
    newScore.score = session.score;
    newScore.difficulty = session.difficulty;
    newScore.timestamp = session.timestamp;

    // Add to list
    scores.push_back(newScore);

    // Sort by score descending
    std::sort(scores.begin(), scores.end(),
        [](const HighScore& a, const HighScore& b) {
            return a.score > b.score;
        });

    // Keep only top scores
    if (scores.size() > MAX_HIGH_SCORES_TOTAL * NUM_DIFFICULTIES) {
        scores.resize(MAX_HIGH_SCORES_TOTAL * NUM_DIFFICULTIES);
    }

    return saveHighScores(scores);
}

// ============================================================================
// Settings
// ============================================================================

GameSettings DataStorage::loadSettings() {
    GameSettings settings;

    // Default settings
    settings.defaultDifficulty = (DifficultyLevel)DEFAULT_DIFFICULTY;
    settings.volume = DEFAULT_VOLUME;
    settings.ledBrightness = DEFAULT_LED_BRIGHTNESS;
    settings.soundEnabled = FEATURE_SOUND_ENABLED;
    settings.deepSleepEnabled = FEATURE_DEEP_SLEEP_ENABLED;

    if (!initialized) return settings;

    File file = LittleFS.open(SETTINGS_FILE, "r");
    if (!file) {
        DEBUG_PRINTLN("[STORAGE] Settings file not found, using defaults");
        return settings;
    }

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        DEBUG_PRINTF("[STORAGE] ERROR: Failed to parse settings: %s\n", error.c_str());
        return settings;
    }

    settings.defaultDifficulty = (DifficultyLevel)doc["difficulty"].as<int>();
    settings.volume = doc["volume"].as<uint8_t>();
    settings.ledBrightness = doc["ledBrightness"].as<uint8_t>();
    settings.soundEnabled = doc["soundEnabled"].as<bool>();
    settings.deepSleepEnabled = doc["deepSleepEnabled"].as<bool>();

    DEBUG_PRINTLN("[STORAGE] Settings loaded");
    return settings;
}

bool DataStorage::saveSettings(const GameSettings& settings) {
    if (!initialized) return false;

    StaticJsonDocument<512> doc;

    doc["difficulty"] = (int)settings.defaultDifficulty;
    doc["volume"] = settings.volume;
    doc["ledBrightness"] = settings.ledBrightness;
    doc["soundEnabled"] = settings.soundEnabled;
    doc["deepSleepEnabled"] = settings.deepSleepEnabled;

    File file = LittleFS.open(SETTINGS_FILE, "w");
    if (!file) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to open settings file for writing");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to write settings");
        file.close();
        return false;
    }

    file.close();
    DEBUG_PRINTLN("[STORAGE] Settings saved");
    return true;
}

// ============================================================================
// Utility
// ============================================================================

bool DataStorage::factoryReset() {
    if (!initialized) return false;

    DEBUG_PRINTLN("[STORAGE] Performing factory reset...");

    LittleFS.remove(PLAYERS_FILE);
    LittleFS.remove(HISTORY_FILE);
    LittleFS.remove(SCORES_FILE);
    LittleFS.remove(SETTINGS_FILE);

    DEBUG_PRINTLN("[STORAGE] Factory reset complete");
    return true;
}

bool DataStorage::getStorageStats(size_t& totalBytes, size_t& usedBytes) {
    if (!initialized) return false;

    totalBytes = LittleFS.totalBytes();
    usedBytes = LittleFS.usedBytes();

    return true;
}

String DataStorage::generateUUID() {
    // Simple UUID generation using random numbers
    // Format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    String uuid = "";
    const char* hex = "0123456789abcdef";

    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid += "-";
        } else {
            uuid += hex[random(0, 16)];
        }
    }

    return uuid;
}

// ============================================================================
// Private Load/Save Methods
// ============================================================================

std::vector<Player> DataStorage::loadPlayers() {
    std::vector<Player> players;

    if (!initialized) return players;

    File file = LittleFS.open(PLAYERS_FILE, "r");
    if (!file) {
        DEBUG_PRINTLN("[STORAGE] Players file not found");
        return players;
    }

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        DEBUG_PRINTF("[STORAGE] ERROR: Failed to parse players: %s\n", error.c_str());
        return players;
    }

    JsonArray array = doc.as<JsonArray>();
    for (JsonVariant v : array) {
        Player p;
        p.id = v["id"].as<String>();
        p.name = v["name"].as<String>();
        p.gamesPlayed = v["gamesPlayed"].as<uint32_t>();
        p.totalScore = v["totalScore"].as<uint32_t>();
        p.bestScore = v["bestScore"].as<uint16_t>();
        p.wins = v["wins"].as<uint16_t>();
        p.created = v["created"].as<uint32_t>();
        players.push_back(p);
    }

    DEBUG_PRINTF("[STORAGE] Loaded %d players\n", players.size());
    return players;
}

bool DataStorage::savePlayers(const std::vector<Player>& players) {
    if (!initialized) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Storage not initialized!");
        return false;
    }

    DynamicJsonDocument doc(4096);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& p : players) {
        JsonObject obj = array.createNestedObject();
        obj["id"] = p.id;
        obj["name"] = p.name;
        obj["gamesPlayed"] = p.gamesPlayed;
        obj["totalScore"] = p.totalScore;
        obj["bestScore"] = p.bestScore;
        obj["wins"] = p.wins;
        obj["created"] = p.created;
    }

    DEBUG_PRINTF("[STORAGE] Opening %s for writing...\n", PLAYERS_FILE);
    File file = LittleFS.open(PLAYERS_FILE, "w");
    if (!file) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to open players file for writing");
        return false;
    }

    size_t bytesWritten = serializeJson(doc, file);
    file.close();

    if (bytesWritten == 0) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to write players (0 bytes written)");
        return false;
    }

    DEBUG_PRINTF("[STORAGE] Saved %d players (%d bytes written to %s)\n",
                players.size(), bytesWritten, PLAYERS_FILE);
    return true;
}

std::vector<GameSession> DataStorage::loadHistory() {
    std::vector<GameSession> history;

    if (!initialized) return history;

    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) {
        return history;
    }

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        DEBUG_PRINTF("[STORAGE] ERROR: Failed to parse history: %s\n", error.c_str());
        return history;
    }

    JsonArray array = doc.as<JsonArray>();
    for (JsonVariant v : array) {
        GameSession s;
        s.playerId = v["playerId"].as<String>();
        s.playerName = v["playerName"].as<String>();
        s.score = v["score"].as<uint16_t>();
        s.difficulty = (DifficultyLevel)v["difficulty"].as<int>();
        s.timestamp = v["timestamp"].as<uint32_t>();
        s.duration = v["duration"].as<uint32_t>();
        history.push_back(s);
    }

    return history;
}

bool DataStorage::saveHistory(const std::vector<GameSession>& history) {
    if (!initialized) return false;

    DynamicJsonDocument doc(8192);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& s : history) {
        JsonObject obj = array.createNestedObject();
        obj["playerId"] = s.playerId;
        obj["playerName"] = s.playerName;
        obj["score"] = s.score;
        obj["difficulty"] = (int)s.difficulty;
        obj["timestamp"] = s.timestamp;
        obj["duration"] = s.duration;
    }

    File file = LittleFS.open(HISTORY_FILE, "w");
    if (!file) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to open history file for writing");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to write history");
        file.close();
        return false;
    }

    file.close();
    return true;
}

std::vector<HighScore> DataStorage::loadHighScores() {
    std::vector<HighScore> scores;

    if (!initialized) return scores;

    File file = LittleFS.open(SCORES_FILE, "r");
    if (!file) {
        return scores;
    }

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        DEBUG_PRINTF("[STORAGE] ERROR: Failed to parse scores: %s\n", error.c_str());
        return scores;
    }

    JsonArray array = doc.as<JsonArray>();
    for (JsonVariant v : array) {
        HighScore hs;
        hs.playerId = v["playerId"].as<String>();
        hs.playerName = v["playerName"].as<String>();
        hs.score = v["score"].as<uint16_t>();
        hs.difficulty = (DifficultyLevel)v["difficulty"].as<int>();
        hs.timestamp = v["timestamp"].as<uint32_t>();
        scores.push_back(hs);
    }

    return scores;
}

bool DataStorage::saveHighScores(const std::vector<HighScore>& scores) {
    if (!initialized) return false;

    DynamicJsonDocument doc(4096);
    JsonArray array = doc.to<JsonArray>();

    for (const auto& hs : scores) {
        JsonObject obj = array.createNestedObject();
        obj["playerId"] = hs.playerId;
        obj["playerName"] = hs.playerName;
        obj["score"] = hs.score;
        obj["difficulty"] = (int)hs.difficulty;
        obj["timestamp"] = hs.timestamp;
    }

    File file = LittleFS.open(SCORES_FILE, "w");
    if (!file) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to open scores file for writing");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        DEBUG_PRINTLN("[STORAGE] ERROR: Failed to write scores");
        file.close();
        return false;
    }

    file.close();
    return true;
}
