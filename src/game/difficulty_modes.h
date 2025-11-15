/**
 * Difficulty Modes for Simon Says
 *
 * Defines different difficulty levels with varying speed, timing, and sequence length.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "../config.h"

/**
 * Difficulty level enumeration
 */
enum DifficultyLevel : uint8_t {
    EASY = 0,
    MEDIUM = 1,
    HARD = 2,
    EXPERT = 3,
    NUM_DIFFICULTIES = 4
};

/**
 * Difficulty settings structure
 */
struct DifficultySettings {
    const char* name;          // Display name
    uint16_t sequenceSpeed;    // Delay between showing each color (ms)
    uint16_t toneDuration;     // How long each tone plays (ms)
    uint8_t maxLength;         // Maximum sequence length
    uint16_t timingWindow;     // Time allowed for player input (ms)
};

/**
 * Get difficulty settings for a specific level
 *
 * Args:
 *     level: Difficulty level enum
 *
 * Returns:
 *     DifficultySettings: Settings for the requested difficulty
 */
inline const DifficultySettings& getDifficultySettings(DifficultyLevel level) {
    static const DifficultySettings difficulties[NUM_DIFFICULTIES] = {
        {"Easy", DIFF_EASY_SPEED, DIFF_EASY_DURATION, DIFF_EASY_MAX_LENGTH, DIFF_EASY_WINDOW},
        {"Medium", DIFF_MEDIUM_SPEED, DIFF_MEDIUM_DURATION, DIFF_MEDIUM_MAX_LENGTH, DIFF_MEDIUM_WINDOW},
        {"Hard", DIFF_HARD_SPEED, DIFF_HARD_DURATION, DIFF_HARD_MAX_LENGTH, DIFF_HARD_WINDOW},
        {"Expert", DIFF_EXPERT_SPEED, DIFF_EXPERT_DURATION, DIFF_EXPERT_MAX_LENGTH, DIFF_EXPERT_WINDOW}
    };

    if (level < NUM_DIFFICULTIES) {
        return difficulties[level];
    }
    return difficulties[DEFAULT_DIFFICULTY];
}

/**
 * Get difficulty name as string
 *
 * Args:
 *     level: Difficulty level enum
 *
 * Returns:
 *     const char*: Name of the difficulty
 */
inline const char* getDifficultyName(DifficultyLevel level) {
    return getDifficultySettings(level).name;
}
