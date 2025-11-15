# ESP32 Simon Says - Task List

**Last Updated:** 2025-11-09
**Board:** ESP32-WROOM-32
**Power:** 3x AAA Batteries (4.5V with AMS1117-3.3 regulator)

## Active Tasks

### Phase 1: Project Setup & Documentation ✅
- [x] Create PLANNING.md - 2025-11-09
- [x] Create TASK.md - 2025-11-09
- [ ] Setup PlatformIO project structure
- [ ] Create README.md with hardware documentation
- [ ] Create .gitignore for PlatformIO projects

### Phase 2: Hardware Integration Layer
- [ ] Create src/hardware/gpio_config.h with pin definitions
- [ ] Implement src/hardware/led_controller.cpp
  - [ ] Basic LED on/off control
  - [ ] PWM brightness control
  - [ ] LED animation effects (blink, fade, pulse)
- [ ] Implement src/hardware/button_handler.cpp
  - [ ] Button debouncing logic
  - [ ] Interrupt-based button detection
  - [ ] Multi-button press detection
- [ ] Implement src/hardware/audio_controller.cpp
  - [ ] Tone generation for 4 game colors
  - [ ] Error and success sounds
  - [ ] Volume control
- [ ] Implement src/hardware/power_manager.cpp
  - [ ] Battery voltage monitoring via ADC
  - [ ] Deep sleep mode implementation
  - [ ] Wake-up handling (power button, game buttons)
  - [ ] Low battery warning

### Phase 3: Core Game Logic
- [ ] Implement src/game/simon_game.cpp
  - [ ] Game state machine (IDLE, SHOWING, WAITING_INPUT, etc.)
  - [ ] Sequence generation using PRNG
  - [ ] Sequence playback (LED + audio)
  - [ ] Input validation
  - [ ] Score tracking
  - [ ] Game over handling
- [ ] Implement src/game/difficulty_modes.cpp
  - [ ] Define difficulty structures (Easy, Medium, Hard, Expert)
  - [ ] Difficulty selection and switching
  - [ ] Per-difficulty timing and parameters
- [ ] Implement src/game/game_analytics.cpp
  - [ ] Track reaction times per button press
  - [ ] Calculate accuracy metrics
  - [ ] Store game session data
  - [ ] Generate analytics summaries
  - [ ] Learning curve tracking

### Phase 4: WiFi & Web Services
- [ ] Implement src/web/wifi_manager.cpp
  - [ ] WiFiManager library integration
  - [ ] Captive portal setup for initial configuration
  - [ ] Reconnection logic
  - [ ] AP fallback mode
- [ ] Implement src/web/data_storage.cpp
  - [ ] LittleFS initialization
  - [ ] High score persistence (JSON format)
  - [ ] Settings storage
  - [ ] Analytics data storage
  - [ ] Data retrieval functions
- [ ] Implement src/web/web_server.cpp
  - [ ] AsyncWebServer initialization
  - [ ] Serve static files from LittleFS
  - [ ] CORS handling if needed
  - [ ] Server start/stop
- [ ] Implement src/web/api_endpoints.cpp
  - [ ] GET /api/status - current game state
  - [ ] POST /api/game/start - start new game
  - [ ] POST /api/game/stop - stop game
  - [ ] GET /api/scores - retrieve high scores
  - [ ] GET /api/analytics - analytics data
  - [ ] GET /api/settings - get settings
  - [ ] POST /api/settings - update settings
  - [ ] POST /api/reset - factory reset
- [ ] Implement src/web/websocket_handler.cpp
  - [ ] WebSocket connection handling
  - [ ] Real-time game state broadcast
  - [ ] Score updates
  - [ ] Connection management

### Phase 5: Web Frontend
- [ ] Create data/index.html
  - [ ] Responsive layout structure
  - [ ] Dashboard section (game controls, status)
  - [ ] Leaderboard section
  - [ ] Analytics section
  - [ ] Settings section
- [ ] Create data/styles.css
  - [ ] Mobile-first responsive design
  - [ ] Color scheme matching game (red, green, blue, yellow)
  - [ ] Button and control styling
  - [ ] Chart container styling
- [ ] Create data/app.js
  - [ ] API communication functions
  - [ ] WebSocket connection and handlers
  - [ ] Game control logic
  - [ ] Score display updates
  - [ ] Chart.js analytics visualization
  - [ ] Settings form handling
  - [ ] Real-time game state updates

### Phase 6: Main Application
- [ ] Implement src/main.cpp
  - [ ] Setup function (hardware init, WiFi, web server)
  - [ ] Main loop (game logic, web server handling)
  - [ ] Integration of all modules
- [ ] Create src/config.h
  - [ ] Global configuration constants
  - [ ] Default settings
  - [ ] Feature flags

### Phase 7: Testing & Validation
- [ ] Create test/test_game_logic.cpp
  - [ ] Test sequence generation
  - [ ] Test input validation
  - [ ] Test score calculation
- [ ] Create test/test_button_handler.cpp
  - [ ] Test debouncing
  - [ ] Test multi-press detection
- [ ] Create test/test_analytics.cpp
  - [ ] Test data collection
  - [ ] Test metrics calculation
- [ ] Integration testing
  - [ ] Test complete game flow
  - [ ] Test web interface functionality
  - [ ] Test API endpoints
  - [ ] Test WebSocket updates
- [ ] Hardware testing
  - [ ] Verify all GPIO connections
  - [ ] Test power consumption
  - [ ] Test battery life
  - [ ] Test deep sleep and wake-up
- [ ] Performance optimization
  - [ ] Optimize power consumption
  - [ ] Optimize web server response times
  - [ ] Optimize game loop timing

### Phase 8: Documentation & Polish
- [ ] Update README.md with complete setup instructions
- [ ] Document API endpoints
- [ ] Create wiring diagrams or schematics
- [ ] Add code comments and documentation
- [ ] Create user manual
- [ ] Add troubleshooting section

## Future Enhancements (Deferred)
- [ ] OTA firmware update support
- [ ] Multiplayer mode
- [ ] Custom sound file upload
- [ ] MQTT integration
- [ ] Mobile native app
- [ ] RGB LED support
- [ ] Cloud leaderboard sync

## Discovered During Work
(Tasks discovered during implementation will be added here)

## Completed Tasks
- [x] Create PLANNING.md - 2025-11-09
- [x] Create TASK.md - 2025-11-09

## Notes
- Using ESP32-WROOM-32 (full size) - has sufficient GPIO pins
- Keeping 3xAAA battery power with AMS1117-3.3 voltage regulator
- Prioritize getting a basic working prototype before adding advanced features
- Test each hardware component individually before integration
- Keep code modular (no file > 500 lines)
- Document all non-obvious decisions
