/**
 * Simon Says Game Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "simon_game.h"
#include "../web/data_storage.h"
#include "../web/websocket_handler.h"

SimonGame::SimonGame(LEDController* leds, ButtonHandler* buttons, AudioController* audio, DataStorage* stor) :
    led(leds),
    btn(buttons),
    audio(audio),
    storage(stor),
    wsHandler(nullptr),
    state(IDLE),
    currentDifficulty(EASY),
    gameMode(SINGLE_PLAYER),
    numPlayers(0),
    currentPlayerIndex(0),
    masterSequenceLength(0),
    sequenceLength(0),
    currentStep(0),
    currentScore(0),
    stateStartTime(0),
    lastInputTime(0),
    gameStartTime(0) {

    // Initialize sequence array
    for (uint8_t i = 0; i < MAX_SEQUENCE_LENGTH; i++) {
        sequence[i] = NONE;
    }

    // Initialize high scores
    for (uint8_t i = 0; i < NUM_DIFFICULTIES; i++) {
        highScores[i] = 0;
    }

    // Initialize player scores
    for (uint8_t i = 0; i < 4; i++) {
        players[i].playerId = "";
        players[i].playerName = "";
        players[i].score = 0;
        players[i].hasPlayed = false;
    }
}

void SimonGame::begin() {
    DEBUG_PRINTLN("[GAME] Initializing Simon Says game...");

    // Load settings for default difficulty
    settings = getDifficultySettings(currentDifficulty);

    // Load high scores from storage
    loadHighScores();

    // Seed random number generator
    // Reason: Use analog read from floating pin for randomness
    randomSeed(analogRead(0));

    // Start in idle state
    setState(IDLE);

    DEBUG_PRINTLN("[GAME] Game initialized");
    DEBUG_PRINTF("[GAME] Difficulty: %s\n", settings.name);
}

void SimonGame::update() {
    // Update button states
    btn->update();

    // Handle current state
    switch (state) {
        case IDLE:
            handleIdle();
            break;
        case SHOWING_SEQUENCE:
            handleShowingSequence();
            break;
        case WAITING_INPUT:
            handleWaitingInput();
            break;
        case INPUT_CORRECT:
            handleInputCorrect();
            break;
        case INPUT_WRONG:
            handleInputWrong();
            break;
        case GAME_OVER:
            handleGameOver();
            break;
        case HIGH_SCORE:
            handleHighScore();
            break;
    }
}

void SimonGame::startGame(DifficultyLevel difficulty) {
    DEBUG_PRINTLN("[GAME] Starting new game!");

    // Play fun game start melody
    audio->playGameStart();

    // Set difficulty
    setDifficulty(difficulty);

    // Reset game state
    sequenceLength = 0;
    currentStep = 0;
    currentScore = 0;
    gameStartTime = millis();

    // Clear sequence
    for (uint8_t i = 0; i < MAX_SEQUENCE_LENGTH; i++) {
        sequence[i] = NONE;
    }

    // Start first round
    extendSequence();
    setState(SHOWING_SEQUENCE);

    // Send WebSocket update
    sendWebSocketUpdate();
}

void SimonGame::reset() {
    DEBUG_PRINTLN("[GAME] Resetting to idle");
    setState(IDLE);
    sendWebSocketUpdate();
}

GameState SimonGame::getState() const {
    return state;
}

uint8_t SimonGame::getScore() const {
    return currentScore;
}

uint8_t SimonGame::getHighScore() const {
    return highScores[currentDifficulty];
}

DifficultyLevel SimonGame::getDifficulty() const {
    return currentDifficulty;
}

void SimonGame::setDifficulty(DifficultyLevel difficulty) {
    if (difficulty < NUM_DIFFICULTIES) {
        currentDifficulty = difficulty;
        settings = getDifficultySettings(difficulty);
        DEBUG_PRINTF("[GAME] Difficulty set to %s\n", settings.name);
    }
}

bool SimonGame::isActive() const {
    return (state != IDLE && state != GAME_OVER);
}

void SimonGame::setWebSocketHandler(WebSocketHandler* handler) {
    wsHandler = handler;
    DEBUG_PRINTLN("[GAME] WebSocket handler set");
}

void SimonGame::setCurrentPlayer(const String& playerId) {
    currentPlayerId = playerId;
    DEBUG_PRINTF("[GAME] Current player set to: %s\n", playerId.c_str());
    sendWebSocketUpdate();
}

void SimonGame::startMultiplayerGame(GameMode mode, const String* playerIds, uint8_t numPlayers_, DifficultyLevel difficulty) {
    DEBUG_PRINTF("[GAME] Starting multiplayer game! Mode: %d, Players: %d\n", mode, numPlayers_);

    if (numPlayers_ < 2 || numPlayers_ > 4) {
        DEBUG_PRINTLN("[GAME] Error: Invalid number of players (2-4 required)");
        return;
    }

    // Set game mode
    gameMode = mode;
    numPlayers = numPlayers_;
    currentPlayerIndex = 0;
    masterSequenceLength = 0;  // Will grow as first player progresses

    // Initialize player data
    for (uint8_t i = 0; i < numPlayers; i++) {
        players[i].playerId = playerIds[i];
        players[i].score = 0;
        players[i].hasPlayed = false;

        // Load player name from storage
        if (storage) {
            Player player;
            if (storage->getPlayer(playerIds[i], player)) {
                players[i].playerName = player.name;
            } else {
                players[i].playerName = "Player " + String(i + 1);
            }
        } else {
            players[i].playerName = "Player " + String(i + 1);
        }

        DEBUG_PRINTF("[GAME] Player %d: %s (%s)\n", i + 1, players[i].playerName.c_str(), players[i].playerId.c_str());
    }

    // Set current player
    currentPlayerId = players[0].playerId;

    // Play fun game start melody
    audio->playGameStart();

    // Set difficulty
    setDifficulty(difficulty);

    // Reset game state
    sequenceLength = 0;
    currentStep = 0;
    currentScore = 0;
    gameStartTime = millis();

    // Clear sequence
    for (uint8_t i = 0; i < MAX_SEQUENCE_LENGTH; i++) {
        sequence[i] = NONE;
    }

    // Start first round
    extendSequence();
    setState(SHOWING_SEQUENCE);

    // Send WebSocket update
    sendMultiplayerUpdate();
}

GameMode SimonGame::getGameMode() const {
    return gameMode;
}

String SimonGame::getCurrentPlayer() const {
    if (gameMode == SINGLE_PLAYER) {
        return currentPlayerId;
    }
    return players[currentPlayerIndex].playerId;
}

const PlayerScore* SimonGame::getPlayerScores() const {
    return players;
}

uint8_t SimonGame::getNumPlayers() const {
    return numPlayers;
}

// ============================================================================
// State Handlers
// ============================================================================

void SimonGame::handleIdle() {
    // Wait for any button press to start
    Color pressed = btn->getJustPressed();
    if (pressed != NONE) {
        DEBUG_PRINTLN("[GAME] Button pressed, starting game!");
        startGame(currentDifficulty);
    }
}

void SimonGame::handleShowingSequence() {
    // This state is handled synchronously in playSequence()
    // Just play the sequence and transition to waiting for input
    playSequence();
    setState(WAITING_INPUT);
    sendWebSocketUpdate();
}

void SimonGame::handleWaitingInput() {
    // Check for timeout (time since LAST input, not since state started)
    // Reason: Each button press should have its own timeout window
    if ((millis() - lastInputTime) > settings.timingWindow) {
        DEBUG_PRINTLN("[GAME] Input timeout!");
        setState(INPUT_WRONG);
        return;
    }

    // Check for button press
    Color pressed = btn->getJustPressed();
    if (pressed != NONE) {
        DEBUG_PRINTF("[GAME] Player pressed %s\n", colorToString(pressed));

        // Light up LED and play tone for feedback
        led->on(pressed);
        audio->playColor(pressed, settings.toneDuration);
        led->off(pressed);

        // Validate input
        bool correct = validateInput(pressed);
        if (correct) {
            DEBUG_PRINTLN("[GAME] Correct!");
            currentStep++;

            // Send button press update
            sendButtonPressUpdate(pressed, true);

            // Check if sequence is complete
            if (currentStep >= sequenceLength) {
                DEBUG_PRINTLN("[GAME] Sequence complete!");
                setState(INPUT_CORRECT);
            } else {
                // Reset timeout for next input
                lastInputTime = millis();
            }
        } else {
            DEBUG_PRINTLN("[GAME] Wrong!");
            sendButtonPressUpdate(pressed, false);
            setState(INPUT_WRONG);
        }
    }
}

void SimonGame::handleInputCorrect() {
    // No visual/audio feedback - keeps game fast!
    // Reason: User requested no positive feedback to speed up gameplay

    // Increment score
    currentScore++;
    DEBUG_PRINTF("[GAME] Score: %d\n", currentScore);

    // Update player score in multiplayer
    if (gameMode == PASS_AND_PLAY) {
        players[currentPlayerIndex].score = currentScore;
        sendMultiplayerUpdate();
    }

    // Check if max length reached
    if (sequenceLength >= settings.maxLength) {
        DEBUG_PRINTLN("[GAME] Maximum length reached - you win!");

        if (gameMode == SINGLE_PLAYER) {
            updateHighScore();
        }

        setState(GAME_OVER);
        return;
    }

    // Brief pause before next round
    delay(200);

    // Extend sequence and continue (same for single and multiplayer)
    extendSequence();
    setState(SHOWING_SEQUENCE);
}

void SimonGame::handleInputWrong() {
    // Error animation
    led->errorAnimation();
    audio->playGameOver();

    if (gameMode == SINGLE_PLAYER) {
        // Single player mode - game over
        bool isNewHighScore = (currentScore > highScores[currentDifficulty]);
        updateHighScore();
        recordGameSession();
        sendGameOverUpdate(isNewHighScore);
        setState(GAME_OVER);

    } else if (gameMode == PASS_AND_PLAY) {
        // Pass & Play mode - player failed, their turn is over
        DEBUG_PRINTF("[GAME] Player %s finished with score %d\n",
                    players[currentPlayerIndex].playerName.c_str(), currentScore);

        // Save this player's final score and mark as played
        players[currentPlayerIndex].score = currentScore;
        players[currentPlayerIndex].hasPlayed = true;

        // Record their game session
        currentPlayerId = players[currentPlayerIndex].playerId;
        recordGameSession();

        // Check if all players have had their turn
        if (allPlayersFinished()) {
            DEBUG_PRINTLN("[GAME] All players finished - game over!");
            setState(GAME_OVER);
        } else {
            // Move to next player and reset for their turn
            nextPlayer();

            DEBUG_PRINTF("[GAME] Next player: %s\n", players[currentPlayerIndex].playerName.c_str());

            delay(2000);  // Pause between players

            // Reset score and position for new player
            // NOTE: Keep the same sequence - all players play the same challenge
            currentScore = 0;
            currentStep = 0;
            sequenceLength = 1;  // Start from beginning of same sequence

            setState(SHOWING_SEQUENCE);
        }

        sendMultiplayerUpdate();
    }
}

void SimonGame::handleGameOver() {
    // Display score only once when we first enter this state
    // Reason: Prevent spamming the terminal with repeated messages
    if (getStateTime() < 100) {
        DEBUG_PRINTLN("\n========================================");
        DEBUG_PRINTLN("GAME OVER");
        DEBUG_PRINTLN("========================================");
        DEBUG_PRINTF("Score: %d\n", currentScore);
        DEBUG_PRINTF("High Score: %d\n", highScores[currentDifficulty]);
        DEBUG_PRINTLN("Press any button to play again");
        DEBUG_PRINTLN("========================================\n");
    }

    // Wait for button press to restart
    if (getStateTime() > 2000) {  // Wait at least 2 seconds
        Color pressed = btn->getJustPressed();
        if (pressed != NONE) {
            startGame(currentDifficulty);
        }
    }
}

void SimonGame::handleHighScore() {
    // Only play celebration once when entering this state
    if (getStateTime() < 100) {
        // Special high score celebration
        led->successAnimation();
        audio->playHighScore();

        DEBUG_PRINTLN("\n========================================");
        DEBUG_PRINTLN("NEW HIGH SCORE!");
        DEBUG_PRINTLN("========================================");
        DEBUG_PRINTF("Score: %d\n", currentScore);
        DEBUG_PRINTLN("========================================\n");
    }

    // Transition to game over after celebration
    if (getStateTime() > 2000) {
        setState(GAME_OVER);
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

Color SimonGame::generateRandomColor() {
    return (Color)random(0, NUM_COLORS);
}

void SimonGame::extendSequence() {
    if (sequenceLength < MAX_SEQUENCE_LENGTH) {
        // In multiplayer, reuse existing sequence up to masterSequenceLength
        if (gameMode == PASS_AND_PLAY && sequenceLength < masterSequenceLength) {
            // Just increment - color already exists in sequence array
            sequenceLength++;
            DEBUG_PRINTF("[GAME] Reusing sequence at length %d (master: %d)\n",
                        sequenceLength, masterSequenceLength);
        } else {
            // Generate new random color (single player or extending beyond master)
            sequence[sequenceLength] = generateRandomColor();
            sequenceLength++;

            // Update master sequence length in multiplayer
            if (gameMode == PASS_AND_PLAY && sequenceLength > masterSequenceLength) {
                masterSequenceLength = sequenceLength;
                DEBUG_PRINTF("[GAME] Master sequence extended to %d\n", masterSequenceLength);
            } else {
                DEBUG_PRINTF("[GAME] Sequence extended to length %d\n", sequenceLength);
            }
        }
    }
}

void SimonGame::playSequence() {
    DEBUG_PRINTLN("[GAME] Playing sequence...");

    // Send sequence update to WebSocket
    sendSequenceUpdate();

    // Small delay before starting
    delay(500);

    // Play each step in the sequence
    for (uint8_t i = 0; i < sequenceLength; i++) {
        playSequenceStep(i);

        // Only delay between tones, NOT after the last one
        // Reason: Player should be able to input immediately after last tone
        if (i < sequenceLength - 1) {
            delay(settings.sequenceSpeed);
        }
    }

    // Reset step counter for input
    currentStep = 0;
    lastInputTime = millis();

    DEBUG_PRINTLN("[GAME] Sequence complete, waiting for input");
}

void SimonGame::playSequenceStep(uint8_t index) {
    if (index >= sequenceLength) {
        return;
    }

    Color color = sequence[index];
    DEBUG_PRINTF("[GAME] Step %d: %s\n", index + 1, colorToString(color));

    // Light LED and play tone
    led->on(color);
    audio->playColor(color, settings.toneDuration);
    led->off(color);
}

bool SimonGame::validateInput(Color input) {
    if (currentStep >= sequenceLength) {
        return false;
    }

    return (input == sequence[currentStep]);
}

void SimonGame::setState(GameState newState) {
    DEBUG_PRINTF("[GAME] State: %d -> %d\n", state, newState);
    state = newState;
    stateStartTime = millis();

    // Turn off all LEDs on state change
    led->allOff();
}

uint32_t SimonGame::getStateTime() const {
    return millis() - stateStartTime;
}

void SimonGame::updateHighScore() {
    if (currentScore > highScores[currentDifficulty]) {
        DEBUG_PRINTF("[GAME] New high score: %d (was %d)\n",
                    currentScore, highScores[currentDifficulty]);
        highScores[currentDifficulty] = currentScore;
        saveHighScores();

        // Transition to high score celebration
        if (state != HIGH_SCORE) {
            setState(HIGH_SCORE);
        }
    }
}

void SimonGame::loadHighScores() {
    if (!storage) {
        // No storage available, initialize to 0
        for (uint8_t i = 0; i < NUM_DIFFICULTIES; i++) {
            highScores[i] = 0;
        }
        DEBUG_PRINTLN("[GAME] High scores initialized (no storage available)");
        return;
    }

    // Load high scores from storage for each difficulty
    for (uint8_t i = 0; i < NUM_DIFFICULTIES; i++) {
        DifficultyLevel diff = (DifficultyLevel)i;
        std::vector<HighScore> scores = storage->getHighScores(diff, 1);

        if (scores.size() > 0) {
            highScores[i] = scores[0].score;
        } else {
            highScores[i] = 0;
        }
    }

    DEBUG_PRINTLN("[GAME] High scores loaded from storage");
}

void SimonGame::saveHighScores() {
    if (!storage) {
        DEBUG_PRINTLN("[GAME] High scores not saved (no storage available)");
        return;
    }

    // High scores are automatically saved when game sessions are recorded
    DEBUG_PRINTLN("[GAME] High scores saved via game session");
}

void SimonGame::recordGameSession() {
    if (!storage) {
        DEBUG_PRINTLN("[GAME] Game session not recorded (no storage available)");
        return;
    }

    // Calculate game duration in seconds
    uint32_t duration = (millis() - gameStartTime) / 1000;

    // Create game session
    GameSession session;
    session.playerId = currentPlayerId.length() > 0 ? currentPlayerId : "guest";
    session.playerName = "Guest"; // Will be filled by storage
    session.score = currentScore;
    session.difficulty = currentDifficulty;
    session.timestamp = millis() / 1000; // Unix timestamp (will be set by storage)
    session.duration = duration;

    // Save to storage
    if (storage->recordGame(session)) {
        DEBUG_PRINTF("[GAME] Game session recorded: score=%d, duration=%ds\n",
                    currentScore, duration);
    } else {
        DEBUG_PRINTLN("[GAME] Failed to save game session");
    }
}

// ============================================================================
// WebSocket Update Methods
// ============================================================================

void SimonGame::sendWebSocketUpdate() {
    if (!wsHandler) {
        return;
    }

    StaticJsonDocument<256> doc;
    doc["type"] = "gameState";
    doc["state"] = (int)state;
    doc["score"] = currentScore;
    doc["highScore"] = highScores[currentDifficulty];
    doc["difficulty"] = getDifficultyName(currentDifficulty);
    doc["isActive"] = isActive();

    wsHandler->broadcast(doc);
}

void SimonGame::sendSequenceUpdate() {
    if (!wsHandler) {
        return;
    }

    StaticJsonDocument<512> doc;
    doc["type"] = "sequence";

    JsonArray colors = doc.createNestedArray("colors");
    for (uint8_t i = 0; i < sequenceLength; i++) {
        colors.add(colorToString(sequence[i]));
    }

    wsHandler->broadcast(doc);
}

void SimonGame::sendButtonPressUpdate(Color color, bool correct) {
    if (!wsHandler) {
        return;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "buttonPress";
    doc["color"] = colorToString(color);
    doc["correct"] = correct;

    wsHandler->broadcast(doc);
}

void SimonGame::sendGameOverUpdate(bool newHighScore) {
    if (!wsHandler) {
        return;
    }

    StaticJsonDocument<128> doc;
    doc["type"] = "gameOver";
    doc["score"] = currentScore;
    doc["highScore"] = newHighScore;

    wsHandler->broadcast(doc);
}

// ============================================================================
// Multiplayer Helper Methods
// ============================================================================

void SimonGame::nextPlayer() {
    // Find next player who hasn't played yet
    uint8_t startIndex = currentPlayerIndex;

    do {
        currentPlayerIndex = (currentPlayerIndex + 1) % numPlayers;

        // If we've gone full circle, break
        if (currentPlayerIndex == startIndex && players[currentPlayerIndex].hasPlayed) {
            break;
        }

        // Found a player who hasn't played yet
        if (!players[currentPlayerIndex].hasPlayed) {
            currentPlayerId = players[currentPlayerIndex].playerId;

            DEBUG_PRINTF("[GAME] Next player: %s (index %d)\n",
                        players[currentPlayerIndex].playerName.c_str(), currentPlayerIndex);

            return;
        }
    } while (true);

    DEBUG_PRINTLN("[GAME] Warning: All players have already played!");
}

bool SimonGame::allPlayersFinished() {
    // Check if all players have had their turn
    for (uint8_t i = 0; i < numPlayers; i++) {
        if (!players[i].hasPlayed) {
            return false;  // Found a player who hasn't played yet
        }
    }
    return true;  // All players have finished
}

void SimonGame::sendMultiplayerUpdate() {
    if (!wsHandler || gameMode == SINGLE_PLAYER) {
        return;
    }

    StaticJsonDocument<512> doc;
    doc["type"] = "multiplayer";
    doc["gameMode"] = (int)gameMode;
    doc["currentPlayerIndex"] = currentPlayerIndex;
    doc["currentPlayerId"] = players[currentPlayerIndex].playerId;
    doc["currentPlayerName"] = players[currentPlayerIndex].playerName;

    JsonArray playersArray = doc.createNestedArray("players");
    for (uint8_t i = 0; i < numPlayers; i++) {
        JsonObject playerObj = playersArray.createNestedObject();
        playerObj["id"] = players[i].playerId;
        playerObj["name"] = players[i].playerName;
        playerObj["score"] = players[i].score;
        playerObj["hasPlayed"] = players[i].hasPlayed;
    }

    wsHandler->broadcast(doc);
}
