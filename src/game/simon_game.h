/**
 * Simon Says Game Logic
 *
 * Implements the core Simon Says game state machine and gameplay.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "../config.h"
#include "../hardware/gpio_config.h"
#include "../hardware/led_controller.h"
#include "../hardware/button_handler.h"
#include "../hardware/audio_controller.h"
#include "difficulty_modes.h"

// Forward declarations
class DataStorage;
class WebSocketHandler;

/**
 * Game state enumeration
 */
enum GameState {
    IDLE,              // Waiting to start
    SHOWING_SEQUENCE,  // Displaying sequence to player
    WAITING_INPUT,     // Waiting for player to repeat sequence
    INPUT_CORRECT,     // Player got it right, brief celebration
    INPUT_WRONG,       // Player made a mistake
    GAME_OVER,         // Game ended, show score
    HIGH_SCORE         // New high score celebration
};

/**
 * Game mode enumeration
 */
enum GameMode {
    SINGLE_PLAYER,     // Single player mode (default)
    PASS_AND_PLAY      // Players take turns, each plays until they fail
};

/**
 * Player score for multiplayer
 */
struct PlayerScore {
    String playerId;
    String playerName;
    uint8_t score;
    bool hasPlayed;    // Has finished their turn
};

/**
 * Simon Says Game Class
 */
class SimonGame {
public:
    /**
     * Constructor
     *
     * Args:
     *     leds: LED controller instance
     *     buttons: Button handler instance
     *     audio: Audio controller instance
     *     storage: Data storage instance (optional)
     */
    SimonGame(LEDController* leds, ButtonHandler* buttons, AudioController* audio, DataStorage* storage = nullptr);

    /**
     * Initialize the game
     * Call once in setup()
     */
    void begin();

    /**
     * Update game state
     * Call repeatedly in loop()
     */
    void update();

    /**
     * Start a new game
     *
     * Args:
     *     difficulty: Difficulty level to use
     */
    void startGame(DifficultyLevel difficulty = EASY);

    /**
     * Reset game to idle state
     */
    void reset();

    /**
     * Get current game state
     *
     * Returns:
     *     GameState: Current state
     */
    GameState getState() const;

    /**
     * Get current score (sequence length reached)
     *
     * Returns:
     *     uint8_t: Current score
     */
    uint8_t getScore() const;

    /**
     * Get high score for current difficulty
     *
     * Returns:
     *     uint8_t: High score
     */
    uint8_t getHighScore() const;

    /**
     * Get current difficulty level
     *
     * Returns:
     *     DifficultyLevel: Current difficulty
     */
    DifficultyLevel getDifficulty() const;

    /**
     * Set difficulty level
     *
     * Args:
     *     difficulty: New difficulty level
     */
    void setDifficulty(DifficultyLevel difficulty);

    /**
     * Check if game is active (not idle or game over)
     *
     * Returns:
     *     bool: true if game is in progress
     */
    bool isActive() const;

    /**
     * Set WebSocket handler for real-time updates
     *
     * Args:
     *     handler: WebSocket handler instance
     */
    void setWebSocketHandler(WebSocketHandler* handler);

    /**
     * Set current player for game session tracking
     *
     * Args:
     *     playerId: Player ID to set
     */
    void setCurrentPlayer(const String& playerId);

    /**
     * Start a multiplayer game
     *
     * Args:
     *     mode: Game mode (PASS_AND_PLAY or COMPETITIVE)
     *     playerIds: Array of player IDs
     *     numPlayers: Number of players
     *     difficulty: Difficulty level
     */
    void startMultiplayerGame(GameMode mode, const String* playerIds, uint8_t numPlayers, DifficultyLevel difficulty = EASY);

    /**
     * Get current game mode
     *
     * Returns:
     *     GameMode: Current game mode
     */
    GameMode getGameMode() const;

    /**
     * Get current player in multiplayer game
     *
     * Returns:
     *     String: Current player ID
     */
    String getCurrentPlayer() const;

    /**
     * Get all player scores in multiplayer game
     *
     * Returns:
     *     const PlayerScore*: Array of player scores
     */
    const PlayerScore* getPlayerScores() const;

    /**
     * Get number of players in multiplayer game
     *
     * Returns:
     *     uint8_t: Number of players
     */
    uint8_t getNumPlayers() const;

private:
    // Hardware references
    LEDController* led;
    ButtonHandler* btn;
    AudioController* audio;

    // Web references
    DataStorage* storage;
    WebSocketHandler* wsHandler;

    // Game state
    GameState state;
    DifficultyLevel currentDifficulty;
    DifficultySettings settings;

    // Session tracking
    String currentPlayerId;
    uint32_t gameStartTime;

    // Multiplayer support
    GameMode gameMode;
    PlayerScore players[4];  // Max 4 players
    uint8_t numPlayers;
    uint8_t currentPlayerIndex;
    uint8_t masterSequenceLength;  // Max sequence reached by any player

    // Sequence data
    Color sequence[MAX_SEQUENCE_LENGTH];
    uint8_t sequenceLength;
    uint8_t currentStep;

    // Score tracking
    uint8_t currentScore;
    uint8_t highScores[NUM_DIFFICULTIES];

    // Timing
    uint32_t stateStartTime;
    uint32_t lastInputTime;

    // Player input LED tracking (for non-blocking LED control)
    Color currentInputLED;
    uint32_t inputLEDEndTime;

    /**
     * State machine handlers
     */
    void handleIdle();
    void handleShowingSequence();
    void handleWaitingInput();
    void handleInputCorrect();
    void handleInputWrong();
    void handleGameOver();
    void handleHighScore();

    /**
     * Generate new random color for sequence
     *
     * Returns:
     *     Color: Random color
     */
    Color generateRandomColor();

    /**
     * Add one color to the sequence
     */
    void extendSequence();

    /**
     * Play the entire sequence for the player
     */
    void playSequence();

    /**
     * Play a single step in the sequence
     *
     * Args:
     *     index: Index of step to play
     *     toneDuration: Duration of the tone in milliseconds
     */
    void playSequenceStep(uint8_t index, uint16_t toneDuration);

    /**
     * Check if player input matches expected color
     *
     * Args:
     *     input: Color pressed by player
     *
     * Returns:
     *     bool: true if correct, false otherwise
     */
    bool validateInput(Color input);

    /**
     * Transition to a new game state
     *
     * Args:
     *     newState: State to transition to
     */
    void setState(GameState newState);

    /**
     * Get time elapsed since state started (milliseconds)
     *
     * Returns:
     *     uint32_t: Milliseconds in current state
     */
    uint32_t getStateTime() const;

    /**
     * Update high score if current score is better
     */
    void updateHighScore();

    /**
     * Load high scores from storage
     * (Currently just initializes to 0, storage comes later)
     */
    void loadHighScores();

    /**
     * Save high scores to storage
     * (Currently does nothing, storage comes later)
     */
    void saveHighScores();

    /**
     * Record completed game session to storage
     */
    void recordGameSession();

    /**
     * Send WebSocket update with current game state
     */
    void sendWebSocketUpdate();

    /**
     * Send WebSocket message about sequence being shown
     */
    void sendSequenceUpdate();

    /**
     * Send WebSocket message about button press
     *
     * Args:
     *     color: Button color pressed
     *     correct: Whether the press was correct
     */
    void sendButtonPressUpdate(Color color, bool correct);

    /**
     * Send WebSocket message about game over
     *
     * Args:
     *     newHighScore: Whether a new high score was achieved
     */
    void sendGameOverUpdate(bool newHighScore);

    /**
     * Multiplayer helper methods
     */
    void nextPlayer();
    bool allPlayersFinished();
    void sendMultiplayerUpdate();
};
