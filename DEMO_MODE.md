# Hardware Demo Mode - User Guide

This guide explains how to use the comprehensive hardware demo/test routines for the ESP32 Simon Says project.

## Enabling Demo Mode

1. Open `src/config.h`
2. Set `DEMO_MODE_ENABLED` to `true`:
   ```cpp
   #define DEMO_MODE_ENABLED true
   ```
3. Build and upload to your ESP32

## Using Demo Mode

### Starting Demo Mode

1. Connect ESP32 via USB
2. Open Serial Monitor at **115200 baud**
3. Reset the ESP32
4. You'll see the demo menu:

```
========================================
HARDWARE DEMO MENU
========================================
1 - Test LEDs
2 - Test LED Brightness (PWM)
3 - Test LED Animations
4 - Test Buttons
5 - Test Speaker
6 - Test Frequency Sweep
7 - Test Volume Control
8 - Test Integrated (Button+LED+Sound)
9 - Test Power Management
T - Interactive Frequency Tuning
F - Run Full Demo
M - Show Menu
========================================
Enter selection:
```

## Test Descriptions

### 1. Test LEDs
Tests each LED individually in sequence:
- **Red** → **Green** → **Blue** → **Yellow** → **All Together**

**What to check:**
- ✓ All LEDs light up
- ✓ Correct colors match the buttons
- ✓ No dim or flickering LEDs
- ✓ LEDs turn off completely when not lit

### 2. Test LED Brightness (PWM)
Tests PWM brightness control by fading each LED in and out.

**What to check:**
- ✓ Smooth fade in/out (no jumps or flickers)
- ✓ Full brightness range (completely off → full bright)
- ✓ No buzzing sounds from LEDs

### 3. Test LED Animations
Demonstrates all LED animation effects:
- Blink (on/off rapid)
- Pulse (smooth fade in/out)
- Startup animation
- Success animation
- Error animation

**What to check:**
- ✓ Animations are smooth
- ✓ Timing feels right for game use

### 4. Test Buttons
Interactive button test - press each button to light its LED.

**How to use:**
- Press **RED button** → Red LED lights
- Press **GREEN button** → Green LED lights
- Press **BLUE button** → Blue LED lights
- Press **YELLOW button** → Yellow LED lights
- Press **ALL 4 buttons together** → Exit test

**What to check:**
- ✓ Each button responds correctly
- ✓ No missed button presses
- ✓ No "bouncing" (double triggers)
- ✓ LED lights immediately when button pressed
- ✓ LED turns off when button released

### 5. Test Speaker
Tests all audio tones and melodies:
- Color tones (Red, Green, Blue, Yellow)
- Error sound
- Success sound
- Startup melody
- Game over melody
- High score melody

**What to check:**
- ✓ All sounds are audible
- ✓ Different tones are distinguishable
- ✓ No distortion or crackling
- ✓ Volume is appropriate

### 6. Test Frequency Sweep
Sweeps through frequencies from 100 Hz to 2000 Hz in 50 Hz steps.

**Purpose:**
- Find "dead spots" where speaker doesn't respond well
- Identify frequencies that sound best with your specific piezo speaker
- Tune optimal frequencies for each color

**What to listen for:**
- ✓ Continuous sweep (no gaps)
- ✓ Note which frequencies sound loudest/clearest
- ✓ Some frequencies may be quieter (normal for piezo speakers)

### 7. Test Volume Control
Tests volume levels from 0% to 100% in 20% increments.

**What to check:**
- ✓ Clear difference between volume levels
- ✓ Volume 0% is silent
- ✓ Volume 100% is not distorted

### 8. Test Integrated (Button+LED+Sound)
Complete integration test - combines all hardware.

**How to use:**
- Press any **color button** → LED lights + tone plays
- Hold button → LED stays on, tone repeats
- Press **power button** → Exit test

**What to check:**
- ✓ Perfect synchronization between button, LED, and sound
- ✓ No lag between button press and LED/sound
- ✓ Everything works smoothly together

### 9. Test Power Management
Displays battery status information.

**What to check:**
- ✓ Battery voltage reading is reasonable (3.6V - 4.8V for 3xAAA)
- ✓ Battery percentage makes sense
- ✓ Status is correct (GOOD / LOW / CRITICAL)

**Note:** If battery monitoring is disabled in config.h, this test will be skipped.

### T - Interactive Frequency Tuning ⭐
**Most useful for frequency modulation!**

This mode lets you interactively tune the frequency for each color using the buttons.

**How to use:**

1. Press **T** to enter tuning mode
2. Current color LED will light up (starts with RED)
3. Use buttons to adjust:
   - **RED button** → Play current frequency
   - **GREEN button** → Decrease frequency by 10 Hz
   - **BLUE button** → Increase frequency by 10 Hz
   - **YELLOW button** → Switch to next color
   - **POWER button** → Exit and show final values

4. When you exit, the final frequencies will be displayed:
   ```
   Final tuned frequencies:
     Red: 262 Hz
     Green: 330 Hz
     Blue: 392 Hz
     Yellow: 523 Hz

   You can update these values in config.h:
   #define TONE_FREQ_RED    262
   #define TONE_FREQ_GREEN  330
   #define TONE_FREQ_BLUE   392
   #define TONE_FREQ_YELLOW 523
   ```

**Tuning tips:**
- Start with the default frequencies
- Adjust until tones are clearly distinguishable
- Common ranges: 200-800 Hz works well for most piezo speakers
- Musical notes sound pleasant (C, E, G, C is a nice pattern)
- Higher frequencies = more piercing/attention-getting

### F - Run Full Demo
Runs all tests in sequence automatically:
1. LEDs
2. Buttons (10 second test)
3. Speaker
4. Integrated test (10 second test)
5. Power management

**Use this for:**
- Quick overall validation
- Demonstrating all features
- Final check before assembly

## Troubleshooting with Demo Mode

### LEDs don't light
- Check GPIO connections (pins 25, 26, 27, 14)
- Verify 330Ω resistors are in place
- Check LED polarity (long leg = +, short leg = -)
- Test continuity with multimeter

### Buttons don't respond
- Check GPIO connections (pins 32, 33, 34, 35)
- Verify buttons are connected to GND when pressed
- For GPIO 34-35: External pull-up resistors required (10kΩ)
- Check for loose connections

### No sound from speaker
- Check GPIO 23 connection
- Verify piezo polarity
- Try increasing volume (Test 7)
- Some piezo speakers need higher frequencies (>200 Hz)
- Test with frequency sweep to find responsive range

### Battery voltage reads zero
- Check voltage divider resistors (2x 10kΩ)
- Verify GPIO 36 connection
- Check that batteries are installed
- Measure actual battery voltage with multimeter

### Dim LEDs
- Check current limiting resistors (should be 330Ω, not too high)
- Verify power supply voltage (should be 3.3V)
- Try adjusting global brightness in Test 2
- Check for voltage drop across long wires

## Recommended Testing Sequence

For first-time hardware validation:

1. **Visual Inspection**
   - Check all solder joints
   - Verify pin connections match PLANNING.md
   - Check component orientation (LED polarity, etc.)

2. **Power Check**
   - Test 9: Verify battery voltage is reasonable
   - Confirm ESP32 powers on (serial output appears)

3. **Individual Components**
   - Test 1: Verify all LEDs work
   - Test 4: Verify all buttons work
   - Test 5: Verify speaker works

4. **Frequency Tuning**
   - Test 6: Identify best frequency ranges
   - Test T: Fine-tune frequencies for each color
   - Update config.h with optimized values

5. **Integration**
   - Test 8: Verify everything works together
   - Test F: Run full demo as final validation

6. **Done!**
   - Set `DEMO_MODE_ENABLED` to `false` in config.h
   - Build and upload game firmware

## Tips for Frequency Modulation

### Musical Approach
Use musical notes for pleasant, distinguishable tones:
- **Red (C4):** 262 Hz
- **Green (E4):** 330 Hz
- **Blue (G4):** 392 Hz
- **Yellow (C5):** 523 Hz

This creates a major triad that sounds harmonious.

### Evenly Spaced Approach
For maximum distinguishability:
- Start at 200 Hz
- Increment by 150 Hz each color
- Example: 200, 350, 500, 650 Hz

### Trial and Error
Use the **Interactive Frequency Tuning (T)** mode:
1. Start with defaults
2. Adjust each until it "feels right"
3. Test them together to ensure they're distinguishable
4. Save final values to config.h

### Piezo Speaker Considerations
- Piezo speakers have resonant frequencies (usually 2-4 kHz)
- Lower frequencies (100-300 Hz) may be quieter
- Mid-range (300-800 Hz) works well for most applications
- Very high frequencies (>2000 Hz) can be harsh/painful

## Exit Demo Mode

When testing is complete:

1. Open `src/config.h`
2. Set `DEMO_MODE_ENABLED` to `false`
3. Rebuild and upload
4. Your ESP32 will now run the game firmware instead of demo mode

---

**Questions or Issues?**
Check the serial monitor output for detailed debugging information. All tests provide verbose feedback about what they're doing.
