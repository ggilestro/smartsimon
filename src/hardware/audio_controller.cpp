/**
 * Audio Controller Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "audio_controller.h"

AudioController::AudioController() :
    volume(DEFAULT_VOLUME),
    muted(false),
    toneEndTime(0) {
}

void AudioController::begin() {
    DEBUG_PRINTLN("[AUDIO] Initializing audio controller...");

    // Setup PWM for speaker
    // Reason: Using ledcSetup with 10-bit resolution for better frequency control
    ledcSetup(PWM_CHANNEL_SPEAKER, 1000, 10);  // Start with 1kHz, 10-bit resolution
    ledcAttachPin(GPIO_SPEAKER, PWM_CHANNEL_SPEAKER);
    ledcWrite(PWM_CHANNEL_SPEAKER, 0);  // Start silent

    DEBUG_PRINTF("[AUDIO] Configured speaker on GPIO %d (PWM channel %d)\n",
                GPIO_SPEAKER, PWM_CHANNEL_SPEAKER);
    DEBUG_PRINTF("[AUDIO] Default volume: %d\n", volume);

    DEBUG_PRINTLN("[AUDIO] Audio initialized");
}

void AudioController::playTone(uint16_t frequency, uint16_t duration, bool blocking) {
    if (muted || !FEATURE_SOUND_ENABLED) {
        if (blocking && duration > 0) {
            delay(duration);  // Still delay to maintain timing
        }
        return;
    }

    startTone(frequency);

    if (blocking) {
        delay(duration);
        stop();
    } else {
        toneEndTime = millis() + duration;
    }
}

void AudioController::playColor(Color color, uint16_t duration, bool blocking) {
    uint16_t freq = getColorFrequency(color);

    if (duration == 0) {
        duration = TONE_DURATION_MS;
    }

    DEBUG_PRINTF("[AUDIO] Playing %s tone (%d Hz) for %d ms\n",
                colorToString(color), freq, duration);

    playTone(freq, duration, blocking);
}

void AudioController::playError(uint16_t duration) {
    DEBUG_PRINTLN("[AUDIO] Playing error sound");
    playTone(TONE_FREQ_ERROR, duration, true);
}

void AudioController::playSuccess(uint16_t duration) {
    DEBUG_PRINTLN("[AUDIO] Playing success sound");
    playTone(TONE_FREQ_SUCCESS, duration, true);
}

void AudioController::playStartup() {
    DEBUG_PRINTLN("[AUDIO] Playing startup melody");

    // Simple ascending tone sequence
    playTone(TONE_FREQ_RED, 150, true);
    playTone(TONE_FREQ_GREEN, 150, true);
    playTone(TONE_FREQ_BLUE, 150, true);
    playTone(TONE_FREQ_YELLOW, 200, true);
}

void AudioController::playGameStart() {
    DEBUG_PRINTLN("[AUDIO] Playing game start melody");

    // Fun upbeat "let's go!" melody
    // Reason: Play an energetic tune to get player excited
    playTone(523, 100, true);  // C5
    playTone(659, 100, true);  // E5
    playTone(784, 100, true);  // G5
    playTone(1047, 150, true); // C6
    delay(50);
    playTone(1047, 100, true); // C6
    playTone(784, 200, true);  // G5
}

void AudioController::playGameOver() {
    DEBUG_PRINTLN("[AUDIO] Playing game over melody");

    // Funny "Price is Right" losing horn / sad trombone
    // Reason: Make it comical instead of just disappointing
    playTone(415, 250, true);  // G#4
    delay(50);
    playTone(370, 250, true);  // F#4
    delay(50);
    playTone(330, 250, true);  // E4
    delay(50);
    playTone(294, 250, true);  // D4
    delay(50);
    playTone(247, 600, true);  // B3 - long sad note
    delay(100);
    // Add a little "wah wah wah" at the end
    playTone(220, 150, true);  // A3
    delay(50);
    playTone(196, 150, true);  // G3
    delay(50);
    playTone(175, 400, true);  // F3 - final sad note
}

void AudioController::playHighScore() {
    DEBUG_PRINTLN("[AUDIO] Playing high score celebration");

    // Happy ascending fanfare
    for (uint8_t i = 0; i < 3; i++) {
        playTone(TONE_FREQ_YELLOW, 100, true);
        playTone(TONE_FREQ_YELLOW * 1.5, 100, true);
        delay(50);
    }
    playTone(TONE_FREQ_SUCCESS, 400, true);
}

void AudioController::stop() {
    ledcWrite(PWM_CHANNEL_SPEAKER, 0);
    toneEndTime = 0;
}

void AudioController::update() {
    // Check if we need to stop a non-blocking tone
    if (toneEndTime > 0 && millis() >= toneEndTime) {
        stop();
    }
}

void AudioController::setVolume(uint8_t vol) {
    volume = constrain(vol, 0, 100);
    DEBUG_PRINTF("[AUDIO] Volume set to %d\n", volume);
}

uint8_t AudioController::getVolume() const {
    return volume;
}

void AudioController::setMute(bool mute) {
    muted = mute;
    if (muted) {
        stop();
    }
    DEBUG_PRINTF("[AUDIO] Audio %s\n", muted ? "muted" : "unmuted");
}

bool AudioController::isMuted() const {
    return muted;
}

uint16_t AudioController::getColorFrequency(Color color) {
    switch (color) {
        case RED:    return TONE_FREQ_RED;
        case GREEN:  return TONE_FREQ_GREEN;
        case BLUE:   return TONE_FREQ_BLUE;
        case YELLOW: return TONE_FREQ_YELLOW;
        default:     return 0;
    }
}

uint8_t AudioController::calculateDutyCycle() {
    // Reason: For piezo speakers, we use 50% duty cycle as maximum
    // and scale it with volume. PWM resolution is 10-bit (0-1023),
    // so 50% = 512. We scale this by volume percentage.
    uint16_t maxDuty = 512;  // 50% of 1024 (10-bit)
    return (maxDuty * volume) / 100;
}

void AudioController::startTone(uint16_t frequency) {
    if (frequency == 0) {
        stop();
        return;
    }

    // Update PWM frequency
    ledcSetup(PWM_CHANNEL_SPEAKER, frequency, 10);
    ledcAttachPin(GPIO_SPEAKER, PWM_CHANNEL_SPEAKER);

    // Set duty cycle based on volume
    uint8_t duty = calculateDutyCycle();
    ledcWrite(PWM_CHANNEL_SPEAKER, duty);
}
