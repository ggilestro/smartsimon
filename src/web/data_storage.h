/**
 * Data Storage System for ESP32 Simon Says
 *
 * Handles persistent storage of players, scores, and settings using LittleFS.
 * All data is stored in JSON format for easy portability.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include "../config.h"
#include "../game/difficulty_modes.h"

// Maximum limits for data storage
#define MAX_PLAYERS 20
#define MAX_GAME_HISTORY 50
#define MAX_HIGH_SCORES_TOTAL 10

/**
 * Player profile structure
 */
struct Player {
    String id;              // Unique UUID
    String name;            // Player display name
    uint32_t gamesPlayed;   // Total games played
    uint32_t totalScore;    // Sum of all scores (for average)
    uint16_t bestScore;     // Personal best score
    uint16_t wins;          // Games completed successfully
    uint32_t created;       // Timestamp when created
};

/**
 * Game session record
 */
struct GameSession {
    String playerId;        // Player who played this game
    String playerName;      // Player name (denormalized for easy display)
    uint16_t score;         // Score achieved
    DifficultyLevel difficulty;  // Difficulty level
    uint32_t timestamp;     // When the game was played
    uint32_t duration;      // How long the game lasted (ms)
};

/**
 * High score entry
 */
struct HighScore {
    String playerId;
    String playerName;
    uint16_t score;
    DifficultyLevel difficulty;
    uint32_t timestamp;
};

/**
 * Game settings
 */
struct GameSettings {
    DifficultyLevel defaultDifficulty;
    uint8_t volume;              // 0-100
    uint8_t ledBrightness;       // 0-255
    bool soundEnabled;
    bool deepSleepEnabled;
};

/**
 * Data Storage Manager Class
 */
class DataStorage {
public:
    /**
     * Constructor
     */
    DataStorage();

    /**
     * Initialize LittleFS and load data
     *
     * Returns:
     *     bool: true if successful, false on error
     */
    bool begin();

    /**
     * Set the current time offset from browser
     *
     * Args:
     *     unixTimestamp: Current Unix timestamp from browser (seconds)
     */
    void setTimeOffset(uint32_t unixTimestamp);

    /**
     * Get current Unix timestamp
     *
     * Returns:
     *     uint32_t: Current Unix timestamp (seconds)
     */
    uint32_t getCurrentTimestamp() const;

    // ========================================================================
    // Player Management
    // ========================================================================

    /**
     * Create a new player
     *
     * Args:
     *     name: Player display name
     *
     * Returns:
     *     String: Player ID (UUID) or empty string on error
     */
    String createPlayer(const String& name);

    /**
     * Get player by ID
     *
     * Args:
     *     id: Player UUID
     *     player: Output player structure
     *
     * Returns:
     *     bool: true if found, false otherwise
     */
    bool getPlayer(const String& id, Player& player);

    /**
     * Get all players
     *
     * Returns:
     *     std::vector<Player>: List of all players
     */
    std::vector<Player> getAllPlayers();

    /**
     * Update player statistics
     *
     * Args:
     *     id: Player UUID
     *     player: Updated player data
     *
     * Returns:
     *     bool: true if successful
     */
    bool updatePlayer(const String& id, const Player& player);

    /**
     * Delete player
     *
     * Args:
     *     id: Player UUID
     *
     * Returns:
     *     bool: true if successful
     */
    bool deletePlayer(const String& id);

    // ========================================================================
    // Game History
    // ========================================================================

    /**
     * Record a completed game session
     *
     * Args:
     *     session: Game session data
     *
     * Returns:
     *     bool: true if successful
     */
    bool recordGame(const GameSession& session);

    /**
     * Get recent game history
     *
     * Args:
     *     limit: Maximum number of games to return (default 50)
     *
     * Returns:
     *     std::vector<GameSession>: List of recent games
     */
    std::vector<GameSession> getRecentGames(uint8_t limit = MAX_GAME_HISTORY);

    /**
     * Get games for a specific player
     *
     * Args:
     *     playerId: Player UUID
     *     limit: Maximum number of games
     *
     * Returns:
     *     std::vector<GameSession>: Player's game history
     */
    std::vector<GameSession> getPlayerGames(const String& playerId, uint8_t limit = 20);

    // ========================================================================
    // High Scores
    // ========================================================================

    /**
     * Get high scores for a specific difficulty
     *
     * Args:
     *     difficulty: Difficulty level
     *     limit: Number of scores to return (default 10)
     *
     * Returns:
     *     std::vector<HighScore>: High scores list
     */
    std::vector<HighScore> getHighScores(DifficultyLevel difficulty, uint8_t limit = 10);

    /**
     * Get all-time high scores across all difficulties
     *
     * Args:
     *     limit: Number of scores to return
     *
     * Returns:
     *     std::vector<HighScore>: All-time high scores
     */
    std::vector<HighScore> getAllTimeHighScores(uint8_t limit = MAX_HIGH_SCORES_TOTAL);

    /**
     * Check if a score qualifies as a high score and add it
     *
     * Args:
     *     session: Game session to check
     *
     * Returns:
     *     bool: true if it's a new high score
     */
    bool addHighScore(const GameSession& session);

    // ========================================================================
    // Settings
    // ========================================================================

    /**
     * Load settings from storage
     *
     * Returns:
     *     GameSettings: Current settings
     */
    GameSettings loadSettings();

    /**
     * Save settings to storage
     *
     * Args:
     *     settings: Settings to save
     *
     * Returns:
     *     bool: true if successful
     */
    bool saveSettings(const GameSettings& settings);

    // ========================================================================
    // Utility
    // ========================================================================

    /**
     * Factory reset - clear all data
     *
     * Returns:
     *     bool: true if successful
     */
    bool factoryReset();

    /**
     * Get storage statistics
     *
     * Args:
     *     totalBytes: Output total filesystem size
     *     usedBytes: Output used space
     *
     * Returns:
     *     bool: true if successful
     */
    bool getStorageStats(size_t& totalBytes, size_t& usedBytes);

private:
    bool initialized;
    uint32_t timeOffsetSeconds;  // Offset to convert millis() to Unix timestamp

    // File paths
    static const char* PLAYERS_FILE;
    static const char* HISTORY_FILE;
    static const char* SCORES_FILE;
    static const char* SETTINGS_FILE;

    /**
     * Generate unique UUID for players
     *
     * Returns:
     *     String: UUID string
     */
    String generateUUID();

    /**
     * Load players from file
     *
     * Returns:
     *     std::vector<Player>: List of players
     */
    std::vector<Player> loadPlayers();

    /**
     * Save players to file
     *
     * Args:
     *     players: List of players to save
     *
     * Returns:
     *     bool: true if successful
     */
    bool savePlayers(const std::vector<Player>& players);

    /**
     * Load game history from file
     *
     * Returns:
     *     std::vector<GameSession>: Game history
     */
    std::vector<GameSession> loadHistory();

    /**
     * Save game history to file
     *
     * Args:
     *     history: Game history to save
     *
     * Returns:
     *     bool: true if successful
     */
    bool saveHistory(const std::vector<GameSession>& history);

    /**
     * Load high scores from file
     *
     * Returns:
     *     std::vector<HighScore>: High scores
     */
    std::vector<HighScore> loadHighScores();

    /**
     * Save high scores to file
     *
     * Args:
     *     scores: High scores to save
     *
     * Returns:
     *     bool: true if successful
     */
    bool saveHighScores(const std::vector<HighScore>& scores);
};
