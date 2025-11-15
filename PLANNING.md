# ESP32 Simon Says - Project Planning

**Project Owner:** Giorgio Gilestro (giorgio@gilest.ro)
**Start Date:** 2025-11-09
**Platform:** ESP32 with PlatformIO/Arduino Framework

## Project Overview

Modernization of a classic "Simon Says" electronic game by replacing the original microcontroller with an ESP32 module to add WiFi capabilities while maintaining the original hardware components (4 LEDs, 4 buttons, piezo speaker, power button).

### Goals
- Replace original electronics with ESP32 while reusing existing switches, LEDs, and speaker
- Add WiFi connectivity for web-based control and monitoring
- Implement score tracking and leaderboard system
- Add data analytics (reaction times, accuracy, learning curves)
- Support multiple difficulty modes
- Maintain battery operation (3xAAA batteries)

## Hardware Architecture

### Components
- **MCU:** ESP32-WROOM-32 or ESP32-DevKitC
- **Input:** 4x tactile switches (one per color: Red, Green, Blue, Yellow)
- **Output:** 4x LEDs (matching colors)
- **Audio:** 1x Piezo buzzer/speaker
- **Power:** 3x AAA batteries (4.5V nominal, 3.9V-4.5V range)
- **Power Management:** 1x power button, AMS1117-3.3 LDO voltage regulator
- **Optional:** Battery voltage monitoring via ADC

### Pin Mapping (ESP32)

```
// LED Outputs (Active HIGH with current limiting resistors)
GPIO_LED_RED     = 26
GPIO_LED_GREEN   = 25
GPIO_LED_BLUE    = 33
GPIO_LED_YELLOW  = 32

// Button Inputs (Active LOW with internal pull-ups)
GPIO_BTN_RED     = 13
GPIO_BTN_GREEN   = 12
GPIO_BTN_BLUE    = 14
GPIO_BTN_YELLOW  = 27

// Audio Output (PWM capable)
GPIO_SPEAKER     = 23

// Power Control
GPIO_POWER_BTN   = 39  // VP pin, input only
GPIO_BATTERY_ADC = 36  // VP pin for battery voltage monitoring

// Status LED (on ESP32 board)
GPIO_STATUS_LED  = 2   // Built-in LED
```

### Power Circuit Design

```
3x AAA → [Power Switch] → [AMS1117-3.3] → ESP32 (3.3V)
                              ↓
                         100µF + 10µF bypass caps
```

**Power Considerations:**
- 3x AAA batteries: 4.5V nominal (4.8V fresh, 3.6V depleted)
- ESP32 requires: 3.3V ± 0.3V
- AMS1117-3.3: Dropout voltage ~1.2V, works down to ~4.5V input
- Deep sleep current: ~10µA (enables weeks of standby)
- Active current: ~80-160mA (WiFi on)
- Battery monitoring via voltage divider (R1=10kΩ, R2=10kΩ) to ADC

### LED Circuit
```
GPIO → 330Ω resistor → LED → GND
(Assuming 20mA forward current, 2V forward voltage)
```

### Button Circuit
```
3.3V → Internal Pull-up (enabled in software) → GPIO
                                                   ↓
                                                 Button
                                                   ↓
                                                  GND
```

## Software Architecture

### Technology Stack
- **Framework:** Arduino/PlatformIO
- **Web Server:** ESPAsyncWebServer (non-blocking)
- **WiFi Setup:** WiFiManager (captive portal configuration)
- **Filesystem:** LittleFS (for web files, scores, settings)
- **Web Frontend:** HTML5 + CSS3 + Vanilla JavaScript
- **Data Format:** JSON (for API communication)
- **Charts:** Chart.js (for analytics visualization)

### Project Structure

```
Simon/
├── PLANNING.md              # This file
├── TASK.md                  # Task tracking
├── README.md                # Setup and usage instructions
├── platformio.ini           # PlatformIO configuration
├── src/
│   ├── main.cpp            # Entry point, setup, loop
│   ├── config.h            # Global configuration constants
│   ├── hardware/
│   │   ├── gpio_config.h   # Pin definitions
│   │   ├── led_controller.cpp/h      # LED control with effects
│   │   ├── button_handler.cpp/h      # Debounced button input
│   │   ├── audio_controller.cpp/h    # Piezo sound generation
│   │   └── power_manager.cpp/h       # Battery monitoring & sleep
│   ├── game/
│   │   ├── simon_game.cpp/h          # Core game logic
│   │   ├── difficulty_modes.cpp/h    # Difficulty settings
│   │   └── game_analytics.cpp/h      # Analytics tracking
│   └── web/
│       ├── wifi_manager.cpp/h        # WiFi setup
│       ├── web_server.cpp/h          # HTTP server
│       ├── api_endpoints.cpp/h       # REST API
│       ├── websocket_handler.cpp/h   # Real-time updates
│       └── data_storage.cpp/h        # Persistent storage
├── data/                    # Web interface files (uploaded to LittleFS)
│   ├── index.html
│   ├── styles.css
│   └── app.js
└── test/                    # Unit tests
    ├── test_game_logic.cpp
    ├── test_button_handler.cpp
    └── test_analytics.cpp
```

### Code Style & Conventions
- **Language:** C++ (Arduino framework)
- **Naming:**
  - Classes: `PascalCase` (e.g., `SimonGame`, `ButtonHandler`)
  - Functions: `camelCase` (e.g., `playSequence()`, `checkInput()`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `GPIO_LED_RED`, `MAX_SEQUENCE_LENGTH`)
  - Variables: `camelCase` (e.g., `currentLevel`, `isGameActive`)
- **Files:** Max 500 lines per file (per CLAUDE.md)
- **Comments:** Document all non-obvious logic with reason
- **Headers:** Include guards using `#pragma once`

## Game Design

### Game States
```cpp
enum GameState {
    IDLE,           // Waiting to start
    SHOWING,        // Displaying sequence
    WAITING_INPUT,  // Player's turn
    CORRECT,        // Correct input feedback
    WRONG,          // Wrong input - game over
    HIGH_SCORE      // New high score celebration
};
```

### Difficulty Modes

| Mode | Sequence Speed | Tone Duration | Max Length | Timing Window |
|------|----------------|---------------|------------|---------------|
| Easy | 800ms | 600ms | 8 | 3000ms |
| Medium | 600ms | 400ms | 14 | 2000ms |
| Hard | 400ms | 250ms | 20 | 1500ms |
| Expert | 250ms | 150ms | 31 | 1000ms |

### Audio Tones

| Color | Frequency | Note |
|-------|-----------|------|
| Red | 262 Hz | C4 |
| Green | 330 Hz | E4 |
| Blue | 392 Hz | G4 |
| Yellow | 523 Hz | C5 |
| Error | 100 Hz | Low buzz |
| Success | 1047 Hz | C6 |

### Analytics Tracked

- **Per Game:**
  - Sequence length reached
  - Total time played
  - Average reaction time per button
  - Timestamp
  - Difficulty mode

- **Aggregate:**
  - High scores per difficulty
  - Average performance trends
  - Most common failure points
  - Button-specific reaction times
  - Win/loss ratio

## Web Interface Design

### Pages/Sections

1. **Dashboard** (main page)
   - Start/Stop game controls
   - Current game status (live via WebSocket)
   - Quick stats (current score, high score)
   - Difficulty selector

2. **Leaderboard**
   - Top 10 scores per difficulty
   - Personal best highlighting
   - Date/time of achievement

3. **Analytics**
   - Charts: Score progression over time
   - Average reaction time trends
   - Heatmap of button press accuracy
   - Success rate by difficulty

4. **Settings**
   - WiFi configuration
   - Sound volume control
   - LED brightness
   - Deep sleep timeout
   - Factory reset

### API Endpoints

```
GET  /api/status           - Current game state
POST /api/game/start       - Start new game
POST /api/game/stop        - Stop current game
GET  /api/scores           - Get high scores
GET  /api/analytics        - Get analytics data
GET  /api/settings         - Get current settings
POST /api/settings         - Update settings
POST /api/reset            - Factory reset
WS   /ws                   - WebSocket for real-time updates
```

## Power Management Strategy

### Sleep Modes

1. **Active Play:** Full power, WiFi on
2. **Idle (< 2 min):** WiFi on, ready to play
3. **Deep Sleep (> 2 min):** WiFi off, 10µA current
   - Wake on power button press
   - Wake on any game button press

### Battery Life Estimates

- **Active WiFi:** 80-160mA → ~10-20 hours on fresh AAA (600mAh)
- **Deep Sleep:** 10µA → months of standby
- **Typical Use:** Mix of play/idle → ~2-3 days per battery set

### Low Battery Handling

- Monitor battery voltage via ADC
- Warning at 3.8V (status LED slow blink)
- Graceful shutdown at 3.6V (save game state)

## Development Phases

### Phase 1: Hardware Integration
- GPIO configuration
- LED control with PWM
- Button debouncing
- Audio tone generation
- Power/battery monitoring

### Phase 2: Core Game Logic
- Game state machine
- Sequence generation (PRNG)
- Input validation
- Difficulty modes
- Basic analytics

### Phase 3: WiFi & Web Services
- WiFiManager integration
- AsyncWebServer setup
- REST API implementation
- LittleFS data storage
- WebSocket communication

### Phase 4: Web Frontend
- Responsive HTML/CSS interface
- JavaScript game control
- Chart.js analytics
- WebSocket real-time updates

### Phase 5: Testing & Optimization
- Unit tests for game logic
- Integration testing
- Power consumption optimization
- Bug fixes and polish

## Design Decisions & Rationale

### Why ESP32-WROOM-32?
- Sufficient GPIO pins (we need ~10)
- Built-in WiFi
- Low power modes for battery operation
- Well-supported by Arduino/PlatformIO
- Cost-effective

### Why AsyncWebServer?
- Non-blocking operations prevent game lag
- Better performance than ESP8266WebServer
- WebSocket support for real-time updates

### Why LittleFS over SPIFFS?
- Better performance
- Active development (SPIFFS deprecated)
- More reliable wear leveling

### Why WiFiManager?
- Easy setup via captive portal
- No hardcoded credentials
- Standard solution for ESP32 projects

### Why Keep AAA Batteries?
- Maintains original portability
- Deep sleep makes it viable
- User preference
- No need to redesign power delivery

## Security Considerations

- **Web Interface:** No authentication (local network only)
- **OTA Updates:** Not implemented in v1 (potential security risk)
- **Data Privacy:** All data stored locally on device
- **Network:** Creates AP if no WiFi configured (open network)

## Future Enhancements (Out of Scope for v1)

- Multiplayer support (multiple devices)
- Sound file upload (replace tones with custom sounds)
- OTA firmware updates
- MQTT integration for IoT platforms
- Mobile app (native iOS/Android)
- RGB LEDs for advanced visual effects
- Haptic feedback
- Cloud leaderboards

## References & Resources

- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ESPAsyncWebServer Library](https://github.com/me-no-dev/ESPAsyncWebServer)
- [WiFiManager for ESP32](https://github.com/tzapu/WiFiManager)
- [Chart.js Documentation](https://www.chartjs.org/docs/)
- [Arduino Tone Library for ESP32](https://github.com/lbernstone/Tone32)
