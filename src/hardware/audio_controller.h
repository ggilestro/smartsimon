/**
 * Audio Controller for ESP32 Simon Says
 *
 * Provides tone generation for the piezo speaker using PWM.
 * Handles game sounds, melodies, and volume control.
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#pragma once

#include <Arduino.h>
#include "gpio_config.h"
#include "../config.h"

class AudioController {
public:
    /**
     * Constructor - initializes audio controller
     */
    AudioController();

    /**
     * Initialize audio hardware (PWM for speaker)
     * Must be called in setup() before playing sounds
     */
    void begin();

    /**
     * Play a tone at specified frequency and duration
     *
     * Args:
     *     frequency: Frequency in Hz
     *     duration: Duration in milliseconds
     *     blocking: If true, wait until tone finishes. If false, return immediately.
     */
    void playTone(uint16_t frequency, uint16_t duration, bool blocking = true);

    /**
     * Play the tone associated with a color
     *
     * Args:
     *     color: Color enum value
     *     duration: Duration in milliseconds (0 = use default)
     *     blocking: If true, wait until tone finishes
     */
    void playColor(Color color, uint16_t duration = 0, bool blocking = true);

    /**
     * Play error sound (low buzz)
     *
     * Args:
     *     duration: Duration in milliseconds
     */
    void playError(uint16_t duration = 500);

    /**
     * Play success sound (high pitched)
     *
     * Args:
     *     duration: Duration in milliseconds
     */
    void playSuccess(uint16_t duration = 300);

    /**
     * Play startup melody (system boot)
     */
    void playStartup();

    /**
     * Play game start melody (when game begins)
     */
    void playGameStart();

    /**
     * Play game over melody (funny sad trombone)
     */
    void playGameOver();

    /**
     * Play high score celebration melody
     */
    void playHighScore();

    /**
     * Stop any currently playing tone
     */
    void stop();

    /**
     * Update audio state (for non-blocking tones)
     * Call this regularly in loop() to auto-stop tones
     */
    void update();

    /**
     * Set volume (0-100)
     *
     * Args:
     *     volume: Volume level 0 (mute) to 100 (full volume)
     */
    void setVolume(uint8_t volume);

    /**
     * Get current volume setting
     *
     * Returns:
     *     uint8_t: Current volume (0-100)
     */
    uint8_t getVolume() const;

    /**
     * Mute/unmute audio
     *
     * Args:
     *     mute: true to mute, false to unmute
     */
    void setMute(bool mute);

    /**
     * Check if audio is muted
     *
     * Returns:
     *     bool: true if muted, false otherwise
     */
    bool isMuted() const;

private:
    uint8_t volume;      // Volume level (0-100)
    bool muted;          // Mute state
    uint32_t toneEndTime;  // Time when current tone should end (for non-blocking)

    /**
     * Get frequency for a given color
     *
     * Args:
     *     color: Color enum value
     *
     * Returns:
     *     uint16_t: Frequency in Hz
     */
    uint16_t getColorFrequency(Color color);

    /**
     * Calculate PWM duty cycle from volume
     * Higher volume = higher duty cycle
     *
     * Returns:
     *     uint8_t: PWM duty cycle (0-127 for 50% max duty on speaker)
     */
    uint8_t calculateDutyCycle();

    /**
     * Internal function to start a tone
     *
     * Args:
     *     frequency: Frequency in Hz
     */
    void startTone(uint16_t frequency);
};
