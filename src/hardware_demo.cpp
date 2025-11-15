/**
 * Hardware Demo Implementation
 *
 * Author: Giorgio Gilestro (giorgio@gilest.ro)
 * Date: 2025-11-09
 */

#include "hardware_demo.h"

HardwareDemo::HardwareDemo(LEDController* leds, ButtonHandler* buttons,
                           AudioController* audio, PowerManager* power) :
    led(leds), btn(buttons), audio(audio), pwr(power) {
}

void HardwareDemo::runFullDemo() {
    printHeader("FULL HARDWARE DEMO");

    Serial.println("Running complete hardware validation...\n");

    // Test LEDs
    testLEDs();
    delay(1000);

    // Test buttons
    testButtons();
    delay(1000);

    // Test speaker
    testSpeaker();
    delay(1000);

    // Test integrated (button + LED + sound)
    testIntegrated();
    delay(1000);

    // Test power management
    testPowerManagement();

    printSeparator();
    Serial.println("FULL DEMO COMPLETE!");
    printSeparator();
}

void HardwareDemo::testLEDs() {
    printHeader("LED TEST");

    Serial.println("Testing each LED individually...\n");

    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        Color color = (Color)i;
        Serial.print("Testing ");
        Serial.print(colorToString(color));
        Serial.println(" LED...");

        led->on(color);
        delay(500);
        led->off(color);
        delay(200);
    }

    Serial.println("\nTesting all LEDs together...");
    led->allOn();
    delay(1000);
    led->allOff();

    Serial.println("LED test complete!\n");
}

void HardwareDemo::testLEDBrightness() {
    printHeader("LED BRIGHTNESS TEST (PWM)");

    Serial.println("Testing brightness levels for each LED...\n");

    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        Color color = (Color)i;
        Serial.print("Testing ");
        Serial.print(colorToString(color));
        Serial.println(" LED brightness...");

        // Fade in
        Serial.println("  Fading in...");
        for (uint16_t brightness = 0; brightness <= 255; brightness += 5) {
            led->setBrightness(color, brightness);
            delay(20);
        }

        delay(300);

        // Fade out
        Serial.println("  Fading out...");
        for (int16_t brightness = 255; brightness >= 0; brightness -= 5) {
            led->setBrightness(color, brightness);
            delay(20);
        }

        delay(200);
    }

    Serial.println("Brightness test complete!\n");
}

void HardwareDemo::testLEDAnimations() {
    printHeader("LED ANIMATION TEST");

    Serial.println("Testing LED animations...\n");

    Serial.println("1. Blink test...");
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        led->blink((Color)i, 3, 150, 150);
        delay(200);
    }

    Serial.println("2. Pulse test...");
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        led->pulse((Color)i, 800);
        delay(200);
    }

    Serial.println("3. Startup animation...");
    led->startupAnimation();
    delay(500);

    Serial.println("4. Success animation...");
    led->successAnimation();
    delay(500);

    Serial.println("5. Error animation...");
    led->errorAnimation();

    Serial.println("Animation test complete!\n");
}

void HardwareDemo::testButtons() {
    printHeader("BUTTON TEST");

    Serial.println("Press each button to test...");
    Serial.println("Press all 4 buttons together to exit\n");

    led->allOff();

    while (true) {
        btn->update();

        // Check if all buttons pressed (exit condition)
        bool allPressed = true;
        for (uint8_t i = 0; i < NUM_COLORS; i++) {
            if (!btn->isPressed((Color)i)) {
                allPressed = false;
                break;
            }
        }

        if (allPressed) {
            Serial.println("\nAll buttons pressed - exiting button test");
            led->allOff();
            delay(500);
            break;
        }

        // Light up LED for pressed button
        for (uint8_t i = 0; i < NUM_COLORS; i++) {
            Color color = (Color)i;

            if (btn->wasPressed(color)) {
                Serial.print(colorToString(color));
                Serial.println(" button pressed!");
                led->on(color);
            }

            if (btn->wasReleased(color)) {
                led->off(color);
            }
        }

        // Check power button
        if (btn->isPowerButtonPressed()) {
            Serial.println("Power button pressed!");
        }

        delay(10);
    }

    Serial.println("Button test complete!\n");
}

void HardwareDemo::testSpeaker() {
    printHeader("SPEAKER TEST");

    Serial.println("Testing speaker with different frequencies...\n");

    // Test each color tone
    Serial.println("Testing color tones:");
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        Color color = (Color)i;
        Serial.print("  ");
        Serial.print(colorToString(color));
        Serial.print(" tone");
        audio->playColor(color, 500);
        delay(200);
    }

    // Test special sounds
    Serial.println("\nTesting special sounds:");

    Serial.print("  Error sound...");
    audio->playError(500);
    delay(200);

    Serial.print("  Success sound...");
    audio->playSuccess(300);
    delay(200);

    Serial.print("  Startup melody...");
    audio->playStartup();
    delay(200);

    Serial.print("  Game over melody...");
    audio->playGameOver();
    delay(200);

    Serial.print("  High score melody...");
    audio->playHighScore();

    Serial.println("\nSpeaker test complete!\n");
}

void HardwareDemo::testFrequencySweep() {
    printHeader("FREQUENCY SWEEP TEST");

    Serial.println("Sweeping frequencies from 100 Hz to 2000 Hz...");
    Serial.println("Useful for tuning and finding optimal tones\n");

    for (uint16_t freq = 100; freq <= 2000; freq += 50) {
        Serial.print("Frequency: ");
        Serial.print(freq);
        Serial.println(" Hz");

        audio->playTone(freq, 200);
        delay(100);
    }

    Serial.println("\nFrequency sweep complete!\n");
}

void HardwareDemo::testVolumeControl() {
    printHeader("VOLUME CONTROL TEST");

    Serial.println("Testing volume levels (0-100%)...\n");

    uint16_t testFreq = TONE_FREQ_YELLOW;  // Use a mid-range frequency

    for (uint8_t vol = 0; vol <= 100; vol += 20) {
        Serial.print("Volume: ");
        Serial.print(vol);
        Serial.println("%");

        audio->setVolume(vol);
        audio->playTone(testFreq, 500);
        delay(300);
    }

    // Reset to default volume
    audio->setVolume(DEFAULT_VOLUME);

    Serial.println("\nVolume test complete!\n");
}

void HardwareDemo::testIntegrated() {
    printHeader("INTEGRATED TEST (Button + LED + Sound)");

    Serial.println("Press any button to light LED and play tone");
    Serial.println("Press power button to exit\n");

    led->allOff();

    while (true) {
        btn->update();

        // Exit on power button
        if (btn->isPowerButtonPressed()) {
            Serial.println("\nPower button pressed - exiting integrated test");
            led->allOff();
            audio->stop();
            delay(500);
            break;
        }

        // Check each button
        for (uint8_t i = 0; i < NUM_COLORS; i++) {
            Color color = (Color)i;

            if (btn->isPressed(color)) {
                led->on(color);
                // Play tone non-blocking so LED stays on while button held
                if (btn->wasPressed(color)) {
                    Serial.print(colorToString(color));
                    Serial.println(" - Button pressed!");
                    audio->playColor(color, 300, false);
                }
            } else {
                // Turn off LED and stop sound when button released
                if (btn->wasReleased(color)) {
                    led->off(color);
                    audio->stop();
                }
            }
        }

        delay(10);
    }

    Serial.println("Integrated test complete!\n");
}

void HardwareDemo::testPowerManagement() {
    printHeader("POWER MANAGEMENT TEST");

    if (!FEATURE_BATTERY_MONITORING_ENABLED) {
        Serial.println("Battery monitoring is disabled in config.h");
        Serial.println("Skipping power management test\n");
        return;
    }

    Serial.println("Reading battery status...\n");

    uint16_t voltage = pwr->getBatteryVoltage();
    uint8_t percentage = pwr->getBatteryPercentage();
    BatteryStatus status = pwr->getBatteryStatus();

    Serial.print("Battery Voltage: ");
    Serial.print(voltage);
    Serial.println(" mV");

    Serial.print("Battery Percentage: ");
    Serial.print(percentage);
    Serial.println("%");

    Serial.print("Battery Status: ");
    switch (status) {
        case BATTERY_GOOD:
            Serial.println("GOOD");
            break;
        case BATTERY_LOW:
            Serial.println("LOW (Warning)");
            break;
        case BATTERY_CRITICAL:
            Serial.println("CRITICAL (Replace batteries!)");
            break;
    }

    Serial.println("\nPower management test complete!\n");
}

void HardwareDemo::interactiveFrequencyTuning() {
    printHeader("INTERACTIVE FREQUENCY TUNING");

    Serial.println("Use buttons to tune frequencies:");
    Serial.println("  RED    - Play current frequency");
    Serial.println("  GREEN  - Decrease frequency by 10 Hz");
    Serial.println("  BLUE   - Increase frequency by 10 Hz");
    Serial.println("  YELLOW - Cycle through colors to tune");
    Serial.println("  POWER  - Exit tuning mode\n");

    Color currentColor = RED;
    uint16_t frequencies[NUM_COLORS] = {
        TONE_FREQ_RED,
        TONE_FREQ_GREEN,
        TONE_FREQ_BLUE,
        TONE_FREQ_YELLOW
    };

    led->allOff();
    led->on(currentColor);

    Serial.print("Currently tuning: ");
    Serial.print(colorToString(currentColor));
    Serial.print(" (");
    Serial.print(frequencies[currentColor]);
    Serial.println(" Hz)");

    while (true) {
        btn->update();

        // Exit on power button
        if (btn->wasPressed(NONE) && btn->isPowerButtonPressed()) {
            Serial.println("\nExiting frequency tuning mode");
            led->allOff();
            break;
        }

        // Play current frequency
        if (btn->wasPressed(RED)) {
            Serial.print("Playing ");
            Serial.print(frequencies[currentColor]);
            Serial.println(" Hz");
            audio->playTone(frequencies[currentColor], 500);
        }

        // Decrease frequency
        if (btn->wasPressed(GREEN)) {
            if (frequencies[currentColor] > 50) {
                frequencies[currentColor] -= 10;
                Serial.print("Frequency decreased to ");
                Serial.print(frequencies[currentColor]);
                Serial.println(" Hz");
                audio->playTone(frequencies[currentColor], 300);
            }
        }

        // Increase frequency
        if (btn->wasPressed(BLUE)) {
            if (frequencies[currentColor] < 5000) {
                frequencies[currentColor] += 10;
                Serial.print("Frequency increased to ");
                Serial.print(frequencies[currentColor]);
                Serial.println(" Hz");
                audio->playTone(frequencies[currentColor], 300);
            }
        }

        // Cycle color
        if (btn->wasPressed(YELLOW)) {
            led->off(currentColor);
            currentColor = (Color)((currentColor + 1) % NUM_COLORS);
            led->on(currentColor);

            Serial.print("\nNow tuning: ");
            Serial.print(colorToString(currentColor));
            Serial.print(" (");
            Serial.print(frequencies[currentColor]);
            Serial.println(" Hz)");
        }

        delay(10);
    }

    // Print final frequencies
    Serial.println("\nFinal tuned frequencies:");
    for (uint8_t i = 0; i < NUM_COLORS; i++) {
        Serial.print("  ");
        Serial.print(colorToString((Color)i));
        Serial.print(": ");
        Serial.print(frequencies[i]);
        Serial.println(" Hz");
    }

    Serial.println("\nYou can update these values in config.h:");
    Serial.println("#define TONE_FREQ_RED    " + String(frequencies[RED]));
    Serial.println("#define TONE_FREQ_GREEN  " + String(frequencies[GREEN]));
    Serial.println("#define TONE_FREQ_BLUE   " + String(frequencies[BLUE]));
    Serial.println("#define TONE_FREQ_YELLOW " + String(frequencies[YELLOW]));
    Serial.println();
}

void HardwareDemo::showMenu() {
    printSeparator();
    Serial.println("HARDWARE DEMO MENU");
    printSeparator();
    Serial.println("1 - Test LEDs");
    Serial.println("2 - Test LED Brightness (PWM)");
    Serial.println("3 - Test LED Animations");
    Serial.println("4 - Test Buttons");
    Serial.println("5 - Test Speaker");
    Serial.println("6 - Test Frequency Sweep");
    Serial.println("7 - Test Volume Control");
    Serial.println("8 - Test Integrated (Button+LED+Sound)");
    Serial.println("9 - Test Power Management");
    Serial.println("T - Interactive Frequency Tuning");
    Serial.println("F - Run Full Demo");
    Serial.println("M - Show Menu");
    printSeparator();
    Serial.println("Enter selection:");
}

void HardwareDemo::runInteractive() {
    showMenu();

    while (true) {
        if (Serial.available()) {
            char choice = Serial.read();

            // Clear remaining newline characters
            while (Serial.available()) {
                Serial.read();
            }

            switch (choice) {
                case '1':
                    testLEDs();
                    break;
                case '2':
                    testLEDBrightness();
                    break;
                case '3':
                    testLEDAnimations();
                    break;
                case '4':
                    testButtons();
                    break;
                case '5':
                    testSpeaker();
                    break;
                case '6':
                    testFrequencySweep();
                    break;
                case '7':
                    testVolumeControl();
                    break;
                case '8':
                    testIntegrated();
                    break;
                case '9':
                    testPowerManagement();
                    break;
                case 'T':
                case 't':
                    interactiveFrequencyTuning();
                    break;
                case 'F':
                case 'f':
                    runFullDemo();
                    break;
                case 'M':
                case 'm':
                    showMenu();
                    break;
                default:
                    Serial.println("Invalid selection");
                    break;
            }

            Serial.println("\nReady for next test (M for menu):");
        }

        delay(100);
    }
}

void HardwareDemo::printHeader(const char* testName) {
    printSeparator();
    Serial.println(testName);
    printSeparator();
}

void HardwareDemo::printSeparator() {
    Serial.println("========================================");
}

void HardwareDemo::waitForSerial() {
    Serial.println("Press any key to continue...");
    while (!Serial.available()) {
        delay(100);
    }
    while (Serial.available()) {
        Serial.read();
    }
}
