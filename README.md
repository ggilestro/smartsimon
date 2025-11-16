# ESP32 Simon Says

A modernized version of the classic "Simon Says" electronic memory game, powered by an ESP32 microcontroller with WiFi capabilities for web-based control, score tracking, and analytics.

**Author:** Giorgio Gilestro (giorgio@gilest.ro)
**Platform:** ESP32-WROOM-32 / ESP32-C3 (multi-platform support)
**Framework:** Arduino/PlatformIO
**Version:** 1.0.0

## Features

- 🎮 **Classic Simon Says gameplay** with 4 colors (Red, Green, Blue, Yellow)
- 🌐 **WiFi connectivity** for remote control and monitoring
- 📊 **Score tracking & leaderboard** system
- 📈 **Analytics dashboard** (reaction times, accuracy, learning curves)
- 🎚️ **Multiple difficulty modes** (Easy, Medium, Hard, Expert)
- 🔋 **Battery powered** (3x AAA batteries with deep sleep power management)
- 📱 **Responsive web interface** accessible from any device on the network

## Quick Start Guide

### Choosing Your Board

Both ESP32 variants are fully supported. Choose based on your needs:

| Feature | ESP32-WROOM-32 | ESP32-C3 |
|---------|----------------|----------|
| **CPU** | Dual-core Xtensa (240MHz) | Single-core RISC-V (160MHz) |
| **GPIO Pins** | 34 usable | 22 usable |
| **WiFi** | 2.4GHz 802.11 b/g/n | 2.4GHz 802.11 b/g/n |
| **Bluetooth** | Classic + BLE | BLE 5.0 only |
| **Size** | Larger | More compact |
| **Cost** | ~$4-6 | ~$2-4 |
| **Recommendation** | Best overall compatibility | Budget-friendly, smaller projects |

**Both boards run identical code with automatic pin configuration.**

### Hardware Setup Steps

1. **Wire the components** according to the pin diagrams below (choose your board)
2. **Connect power circuit** (3x AAA batteries → regulator → ESP32)
3. **Install PlatformIO** (VSCode extension or CLI)
4. **Build and upload** firmware to your board
5. **Upload filesystem** (web interface files)
6. **Connect to WiFi** and start playing!

### Pin Reference Table

Quick reference for wiring your board:

| Component | ESP32-WROOM-32 | ESP32-C3 | Notes |
|-----------|----------------|----------|-------|
| **LEDs** |
| Red LED | GPIO 26 | GPIO 3 | Through 330Ω resistor |
| Green LED | GPIO 33 | GPIO 4 | Through 330Ω resistor |
| Blue LED | GPIO 25 | GPIO 5 | Through 330Ω resistor |
| Yellow LED | GPIO 32 | GPIO 6 | Through 330Ω resistor |
| **Buttons** |
| Red Button | GPIO 13 | GPIO 7 | Active LOW, pull-up |
| Green Button | GPIO 14 | GPIO 10 | Active LOW, pull-up |
| Blue Button | GPIO 12 | GPIO 18 | Active LOW, pull-up |
| Yellow Button | GPIO 27 | GPIO 19 | Active LOW, pull-up |
| **Audio** |
| Speaker | GPIO 23 | GPIO 20 | Piezo buzzer |
| **Power** |
| Power Button | GPIO 15 | GPIO 21 | Active LOW, pull-up |
| Battery ADC | GPIO 36 (VP) | GPIO 1 (ADC1_CH1) | 10kΩ voltage divider |
| Status LED | GPIO 2 | GPIO 8 | Built-in LED |

## Hardware Requirements

### Supported Boards

This project supports multiple ESP32 variants with automatic GPIO configuration:

- **ESP32-WROOM-32** - Original dual-core Xtensa processor (recommended)
- **ESP32-C3** - RISC-V single-core processor (compact alternative)

Both boards are fully compatible and use the same code. Pin mappings are automatically configured at compile time.

### Components

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32-WROOM-32 or ESP32-C3 | 1 | Main microcontroller |
| Tactile switches | 4 | One per color (from original Simon) |
| LEDs (various colors) | 4 | Red, Green, Blue, Yellow (from original Simon) |
| Piezo buzzer/speaker | 1 | For sound effects (from original Simon) |
| Power button | 1 | Main on/off switch (from original Simon) |
| AAA batteries | 3 | 4.5V nominal power supply |
| AMS1117-3.3 voltage regulator | 1 | Steps down 4.5V to 3.3V for ESP32 |
| Capacitors | 2 | 100µF + 10µF for voltage regulator |
| Resistors (330Ω) | 4 | Current limiting for LEDs |
| Resistors (10kΩ) | 2 | Voltage divider for battery monitoring |

### Pin Connections (ESP32-WROOM-32)

```
┌─────────────────────────────────────────┐
│         ESP32-WROOM-32 PINOUT           │
└─────────────────────────────────────────┘

LEDs (Output, Active HIGH):
  GPIO 26  ──→  [330Ω] ──→  LED Red    ──→  GND
  GPIO 25  ──→  [330Ω] ──→  LED Green  ──→  GND
  GPIO 33  ──→  [330Ω] ──→  LED Blue   ──→  GND
  GPIO 32  ──→  [330Ω] ──→  LED Yellow ──→  GND

Buttons (Input, Active LOW with pull-ups):
  3.3V ──→  [Pull-up] ──→  GPIO 13  (Button Red)    ──→  Switch ──→  GND
  3.3V ──→  [Pull-up] ──→  GPIO 12  (Button Green)  ──→  Switch ──→  GND
  3.3V ──→  [Pull-up] ──→  GPIO 14  (Button Blue)   ──→  Switch ──→  GND
  3.3V ──→  [Pull-up] ──→  GPIO 27  (Button Yellow) ──→  Switch ──→  GND

Audio:
  GPIO 23  ──→  Piezo Speaker ──→  GND

Power Control:
  GPIO 39  ←──  Power Button ──→  GND

Battery Monitoring:
  Battery+ ──→  [10kΩ] ──→  GPIO 36 (ADC) ──→  [10kΩ] ──→  GND

Status:
  GPIO 2   ──→  Onboard LED (built-in on ESP32)
```

### Pin Connections (ESP32-C3)

```
┌─────────────────────────────────────────┐
│          ESP32-C3 PINOUT                │
└─────────────────────────────────────────┘

LEDs (Output, Active HIGH):
  GPIO 3   ──→  [330Ω] ──→  LED Red    ──→  GND
  GPIO 4   ──→  [330Ω] ──→  LED Green  ──→  GND
  GPIO 5   ──→  [330Ω] ──→  LED Blue   ──→  GND
  GPIO 6   ──→  [330Ω] ──→  LED Yellow ──→  GND

Buttons (Input, Active LOW with pull-ups):
  3.3V ──→  [Pull-up] ──→  GPIO 7   (Button Red)    ──→  Switch ──→  GND
  3.3V ──→  [Pull-up] ──→  GPIO 10  (Button Green)  ──→  Switch ──→  GND
  3.3V ──→  [Pull-up] ──→  GPIO 18  (Button Blue)   ──→  Switch ──→  GND
  3.3V ──→  [Pull-up] ──→  GPIO 19  (Button Yellow) ──→  Switch ──→  GND

Audio:
  GPIO 20  ──→  Piezo Speaker ──→  GND

Power Control:
  GPIO 21  ──→  Power Button ──→  GND

Battery Monitoring:
  Battery+ ──→  [10kΩ] ──→  GPIO 1 (ADC) ──→  [10kΩ] ──→  GND

Status:
  GPIO 8   ──→  Onboard LED (built-in on ESP32-C3)
```

> **Note:** Pin mappings are automatically selected based on the build environment. No code changes needed!

### Power Circuit

```
  3x AAA Batteries (4.5V nominal)
         │
         ├──→  Power Switch
         │
         └──→  AMS1117-3.3 Voltage Regulator
                   │
                   ├──→  [100µF]  ──→  GND
                   ├──→  [10µF]   ──→  GND
                   │
                   └──→  ESP32 VCC (3.3V)
```

**Important Notes:**
- ESP32 requires stable 3.3V (±0.3V tolerance)
- AMS1117-3.3 dropout voltage: ~1.2V (minimum input: 4.5V)
- Fresh AAA batteries: ~1.6V each = 4.8V total ✓
- Depleted AAA batteries: ~1.2V each = 3.6V total (too low - replace)
- Add 100µF and 10µF capacitors near regulator for stability

### Platform-Specific Considerations

#### ESP32-WROOM-32
- ✅ More GPIO pins available (34 usable)
- ✅ Dual-core processor for better multitasking
- ✅ Both Classic Bluetooth and BLE
- ✅ Well-established, extensive documentation
- ⚠️ Slightly larger footprint
- ⚠️ Higher cost (~$4-6)

#### ESP32-C3
- ✅ More affordable (~$2-4)
- ✅ Smaller form factor
- ✅ BLE 5.0 support
- ✅ Lower power consumption
- ✅ RISC-V architecture (open-source ISA)
- ⚠️ Single-core processor
- ⚠️ Fewer GPIO pins (22 total)
- ⚠️ No Classic Bluetooth (BLE only)
- ⚠️ GPIO 11-17 reserved for flash (avoid using)

**Important ESP32-C3 Notes:**
- GPIO 0, 8, 9 have special functions during boot - avoid for critical inputs
- GPIO 2 is used for USB D+ (if using USB serial)
- GPIO 8 is typically the built-in LED on most C3 dev boards
- ADC2 pins cannot be used when WiFi is active (we use GPIO 1 which is ADC1_CH1)

## Software Setup

### Prerequisites

1. **PlatformIO** - Install via VSCode extension or standalone
   ```bash
   # Via pip
   pip install platformio
   ```

2. **USB Drivers** - Install CP210x or CH340 drivers for ESP32

### Installation

1. **Clone or download this repository:**
   ```bash
   cd /path/to/projects
   git clone <repository-url>
   cd Simon
   ```

2. **Install dependencies:**
   ```bash
   pio lib install
   ```

3. **Build the project:**

   **For ESP32-WROOM-32:**
   ```bash
   pio run -e esp32dev
   ```

   **For ESP32-C3:**
   ```bash
   pio run -e esp32-c3
   ```

4. **Upload to ESP32:**

   **For ESP32-WROOM-32:**
   ```bash
   pio run -e esp32dev --target upload
   ```

   **For ESP32-C3:**
   ```bash
   pio run -e esp32-c3 --target upload
   ```

5. **Upload filesystem (web interface):**
   ```bash
   # Use -e esp32dev OR -e esp32-c3 depending on your board
   pio run -e esp32dev --target uploadfs
   ```

6. **Monitor serial output:**
   ```bash
   pio device monitor
   ```

> **Tip:** The default environment is `esp32dev`. To switch to ESP32-C3, always specify `-e esp32-c3` in your commands.

### WiFi Configuration

On first boot, the ESP32 will create a WiFi access point:

1. **Connect to the AP:**
   - SSID: `SimonSays-Setup`
   - Password: (none - open network)

2. **Configure WiFi:**
   - Your device should automatically open a captive portal
   - If not, navigate to `http://192.168.4.1`
   - Select your WiFi network and enter the password
   - Click "Save"

3. **Connect to the game:**
   - The ESP32 will restart and connect to your WiFi
   - Find the IP address in the serial monitor
   - Or use mDNS: `http://simon-says.local`

## Web Interface

Once connected to WiFi, access the web interface at `http://simon-says.local` or the IP address shown in the serial monitor.

### Dashboard
- **Start/Stop game** controls
- **Live game status** via WebSocket
- **Current score** and high score display
- **Difficulty selector** (Easy/Medium/Hard/Expert)

### Leaderboard
- Top 10 high scores per difficulty level
- Date and time of achievement
- Personal best highlighting

### Analytics
- Score progression charts over time
- Average reaction time trends
- Button-specific accuracy heatmap
- Success rate by difficulty

### Settings
- WiFi reconfiguration
- Sound volume control
- LED brightness adjustment
- Deep sleep timeout
- Factory reset option

## API Endpoints

The web server exposes a RESTful API for programmatic control:

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | Get current game state |
| POST | `/api/game/start` | Start a new game |
| POST | `/api/game/stop` | Stop current game |
| GET | `/api/scores` | Get high scores (all difficulties) |
| GET | `/api/analytics` | Get analytics data |
| GET | `/api/settings` | Get current settings |
| POST | `/api/settings` | Update settings |
| POST | `/api/reset` | Factory reset (clear all data) |
| WS | `/ws` | WebSocket for real-time updates |

### Example API Usage

```bash
# Get current game status
curl http://simon-says.local/api/status

# Start a new game with difficulty 2 (Hard)
curl -X POST http://simon-says.local/api/game/start \
  -H "Content-Type: application/json" \
  -d '{"difficulty": 2}'

# Get high scores
curl http://simon-says.local/api/scores
```

## Gameplay

### How to Play

1. **Power on** the device using the power button
2. **Press any colored button** to start the game
3. **Watch the sequence** as LEDs light up with corresponding tones
4. **Repeat the sequence** by pressing buttons in the same order
5. **Continue** - each round adds one more step to the sequence
6. **Game over** - make a mistake and the game ends with an error sound

### Difficulty Modes

| Mode | Speed | Tone Duration | Max Length | Input Window |
|------|-------|---------------|------------|--------------|
| **Easy** | 800ms | 600ms | 8 steps | 3000ms |
| **Medium** | 600ms | 400ms | 14 steps | 2000ms |
| **Hard** | 400ms | 250ms | 20 steps | 1500ms |
| **Expert** | 250ms | 150ms | 31 steps | 1000ms |

### Special Button Combinations

- **Long press power button (2s):** Force WiFi reconfiguration mode
- **Hold all 4 buttons on startup:** Factory reset (clears all scores/settings)

## Power Management

The ESP32 uses aggressive power management to extend battery life:

### Power States

1. **Active Play** - Full power, WiFi on (~100mA)
2. **Idle (< 2 min)** - WiFi on, ready to play (~80mA)
3. **Deep Sleep (> 2 min)** - Ultra-low power (~10µA)

### Battery Life Estimates

- **Active WiFi use:** 10-20 hours per battery set
- **Deep sleep standby:** Several months
- **Typical mixed use:** 2-3 days per battery set

### Battery Indicators

- **Normal:** Status LED off
- **Low battery (< 3.8V):** Status LED slow blink
- **Critical (< 3.6V):** Save game state and power down

## Troubleshooting

### ESP32 won't power on
- Check battery voltage (should be > 3.6V)
- Verify power switch is ON
- Check voltage regulator output (should be 3.3V)
- Inspect solder connections

### WiFi won't connect
- Long-press power button to enter config mode
- Check WiFi credentials
- Verify router is 2.4GHz (ESP32 doesn't support 5GHz)
- Check router firewall/MAC filtering

### Buttons not responding
- Check solder joints on button connections
- Verify pull-up resistors are enabled in code
- Test with multimeter (should read 3.3V when not pressed, 0V when pressed)

### LEDs not lighting
- Check LED polarity (long leg = anode/+, short leg = cathode/-)
- Verify current limiting resistors (330Ω)
- Test LEDs with multimeter in diode test mode

### No sound from speaker
- Verify piezo speaker polarity
- Check speaker GPIO connection (GPIO 23 for ESP32-WROOM, GPIO 20 for ESP32-C3)
- Ensure FEATURE_SOUND_ENABLED is true in config.h

### Web interface not loading
- Confirm ESP32 is connected to WiFi (check serial monitor)
- Verify filesystem was uploaded (`pio run --target uploadfs`)
- Try IP address instead of mDNS name
- Clear browser cache

### ESP32-C3 Specific Issues

#### Upload fails or can't connect to board
- **ESP32-C3 uses USB serial by default** - make sure you're using the correct USB port
- Some C3 boards require holding BOOT button while connecting
- Try reducing upload speed: `upload_speed = 115200` in platformio.ini
- Check USB cable - must support data transfer (not charging-only)

#### Code compiles but won't run
- Verify you're building with correct environment: `pio run -e esp32-c3`
- Check that platformio selected the right board variant
- Some C3 variants need specific board definitions (esp32-c3-devkitm-1, esp32-c3-devkitc-02)

#### GPIO not working as expected
- Avoid GPIO 0, 8, 9 during boot (used for boot mode selection)
- GPIO 11-17 are used for flash - never use these
- If battery ADC reads incorrectly, verify GPIO 1 is ADC1_CH1 capable on your board variant

#### WiFi connection issues (C3-specific)
- ESP32-C3 may have different WiFi calibration - try power cycling
- Some C3 boards have weaker WiFi antenna - ensure good signal strength
- Check that you're using a 2.4GHz network (5GHz not supported)

## Development

### Project Structure

```
Simon/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp           # Entry point
│   ├── config.h           # Global configuration
│   ├── hardware/          # Hardware abstraction layer
│   ├── game/              # Game logic
│   └── web/               # Web server & API
├── data/                  # Web interface files (LittleFS)
├── test/                  # Unit tests
├── PLANNING.md            # Architecture documentation
├── TASK.md                # Development task list
└── README.md              # This file
```

### Building & Testing

```bash
# Clean build (ESP32-WROOM-32)
pio run -e esp32dev --target clean
pio run -e esp32dev

# Clean build (ESP32-C3)
pio run -e esp32-c3 --target clean
pio run -e esp32-c3

# Upload code (specify environment)
pio run -e esp32dev --target upload    # For ESP32-WROOM-32
pio run -e esp32-c3 --target upload    # For ESP32-C3

# Upload filesystem (specify environment)
pio run -e esp32dev --target uploadfs  # For ESP32-WROOM-32
pio run -e esp32-c3 --target uploadfs  # For ESP32-C3

# Run unit tests
pio test

# Monitor serial output
pio device monitor

# Build and upload in one command
pio run -e esp32dev --target upload && pio device monitor
```

### Adding Features

See `PLANNING.md` for architecture details and `TASK.md` for the development roadmap.

## License

Open Source - feel free to modify and redistribute.

## Credits

- **Original Simon game** by Ralph H. Baer and Howard J. Morrison (1978)
- **ESP32 platform** by Espressif Systems
- **Libraries:** ESPAsyncWebServer, WiFiManager, ArduinoJson, Tone32

## References

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Arduino Reference](https://www.arduino.cc/reference/en/)
- [Project Planning](PLANNING.md)
- [Task List](TASK.md)

---

**Made with ❤️ by Giorgio Gilestro**
https://lab.gilest.ro | https://giorgio.gilest.ro
