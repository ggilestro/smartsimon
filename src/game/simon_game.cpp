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

    // Check if max length reached
    if (sequenceLength >= settings.maxLength) {
        DEBUG_PRINTLN("[GAME] Maximum length reached - you win!");
        updateHighScore();
        setState(GAME_OVER);
        return;
    }

    // Brief pause before next round
    delay(200);

    // Add next color and show sequence again
    extendSequence();
    setState(SHOWING_SEQUENCE);
}

void SimonGame::handleInputWrong() {
    // Error animation
    led->errorAnimation();
    audio->playGameOver();

    // Check if new high score
    bool isNewHighScore = (currentScore > highScores[currentDifficulty]);

    // Update high score if needed
    updateHighScore();

    // Record game session
    recordGameSession();

    // Send game over update
    sendGameOverUpdate(isNewHighScore);

    // Transition to game over
    setState(GAME_OVER);
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
        sequence[sequenceLength] = generateRandomColor();
        sequenceLength++;
        DEBUG_PRINTF("[GAME] Sequence extended to length %d\n", sequenceLength);
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
