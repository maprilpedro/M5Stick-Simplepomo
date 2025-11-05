/**
 * @file pomodoro_timer.ino
 * @brief Pomodoro Timer with progress bar and cycle graph for M5StickC Plus 2
 * @version 2.0
 * @date 2024-01-15
 * @modified 2025-11-05 - Optimized version with non-blocking timer, partial redraws, power management
 *
 * Based on original work by LucasIneth (https://github.com/lucasineth)
 *
 * @Hardware: M5StickC Plus 2
 * @Platform Version: Arduino M5Stack Board Manager v2.1.3
 * @Dependencies:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5StickCPlus2: https://github.com/m5stack/M5StickCPlus2
 * Preferences: ESP32 built-in EEPROM library
 */

#include <M5StickCPlus2.h>
#include <Preferences.h>

// UI Constants
const int BAR_WIDTH = 200;
const int BAR_HEIGHT = 20;
const int BAR_X = 10;
const int BAR_Y = 140;
const int TEXT_SIZE = 2;
const int MODE_Y = 20;
const int TIME_Y = 60;
const int CYCLES_Y = 100;

// Timer Constants
const int DEFAULT_WORK_TIME = 25 * 60;
const int DEFAULT_BREAK_TIME = 5 * 60;
const int MIN_TIME = 60;
const int TIME_ADJUSTMENT = 60;
const unsigned long SECOND_MS = 1000;
const unsigned long IDLE_DIM_MS = 30000; // Dim after 30s idle
const unsigned long BTN_DEBOUNCE_MS = 200;

// Audio Constants
const int BEEP_FREQ = 1000;
const int BEEP_DURATION = 200;
const int BEEP_PAUSE = 300;

// Graph Constants
const int GRAPH_BAR_WIDTH = 15;
const int GRAPH_BAR_SPACING = 5;
const int GRAPH_BAR_HEIGHT_MAX = 60;
const int GRAPH_X_START = 10;
const int GRAPH_Y_START = 120;
const int MAX_GRAPH_BARS = 10;

// State Variables
int workTime = DEFAULT_WORK_TIME;
int breakTime = DEFAULT_BREAK_TIME;
int timer = workTime;
bool isWorking = true;
bool isRunning = false;
int completedCycles = 0;

// Non-blocking timer
unsigned long lastSecondMillis = 0;
unsigned long lastActivityMillis = 0;
bool isScreenDimmed = false;

// Button state tracking
unsigned long btnBPressStart = 0;
bool btnBLongPressHandled = false;

// Display cache to avoid unnecessary redraws
int lastDisplayedMinutes = -1;
int lastDisplayedSeconds = -1;
bool lastWorkingState = true;
int lastCompletedCycles = -1;

// EEPROM persistence
Preferences preferences;

// Load settings from EEPROM
void loadSettings() {
  preferences.begin("pomodoro", false);
  workTime = preferences.getInt("workTime", DEFAULT_WORK_TIME);
  breakTime = preferences.getInt("breakTime", DEFAULT_BREAK_TIME);
  completedCycles = preferences.getInt("cycles", 0);
  preferences.end();
  timer = workTime;
}

// Save settings to EEPROM
void saveSettings() {
  preferences.begin("pomodoro", false);
  preferences.putInt("workTime", workTime);
  preferences.putInt("breakTime", breakTime);
  preferences.putInt("cycles", completedCycles);
  preferences.end();
}

// Power management - dim screen when idle
void updatePowerManagement() {
  unsigned long now = millis();

  if (!isRunning && !isScreenDimmed && (now - lastActivityMillis > IDLE_DIM_MS)) {
    M5.Lcd.setBrightness(50); // Dim to 50%
    isScreenDimmed = true;
  }
}

// Wake screen on activity
void wakeScreen() {
  if (isScreenDimmed) {
    M5.Lcd.setBrightness(255);
    isScreenDimmed = false;
  }
  lastActivityMillis = millis();
}

// Full screen redraw (only when mode changes)
void fullScreenRedraw() {
  uint16_t bgColor = isWorking ? RED : GREEN;
  M5.Lcd.fillScreen(bgColor);

  M5.Lcd.setTextColor(WHITE, bgColor);
  M5.Lcd.setTextSize(TEXT_SIZE);

  // Mode label
  M5.Lcd.setCursor(BAR_X, MODE_Y);
  M5.Lcd.print(F("Mode: "));
  M5.Lcd.print(isWorking ? F("Work") : F("Break"));

  // Force full redraw of all elements
  lastDisplayedMinutes = -1;
  lastDisplayedSeconds = -1;
  lastCompletedCycles = -1;
  lastWorkingState = isWorking;
}

// Partial update - only redraw changed elements
void displayTime() {
  int minutes = timer / 60;
  int seconds = timer % 60;

  // Full redraw if mode changed
  if (lastWorkingState != isWorking) {
    fullScreenRedraw();
  }

  uint16_t bgColor = isWorking ? RED : GREEN;
  M5.Lcd.setTextColor(WHITE, bgColor);
  M5.Lcd.setTextSize(TEXT_SIZE);

  // Only update time if changed
  if (minutes != lastDisplayedMinutes || seconds != lastDisplayedSeconds) {
    M5.Lcd.fillRect(BAR_X, TIME_Y, 220, 30, bgColor); // Clear time area only
    M5.Lcd.setCursor(BAR_X, TIME_Y);
    M5.Lcd.printf("Time: %02d:%02d", minutes, seconds);
    lastDisplayedMinutes = minutes;
    lastDisplayedSeconds = seconds;
  }

  // Only update cycles if changed
  if (completedCycles != lastCompletedCycles) {
    M5.Lcd.fillRect(BAR_X, CYCLES_Y, 220, 30, bgColor); // Clear cycles area only
    M5.Lcd.setCursor(BAR_X, CYCLES_Y);
    M5.Lcd.printf("Cycles: %d", completedCycles);
    lastCompletedCycles = completedCycles;
  }

  // Update progress bar
  int maxTime = isWorking ? workTime : breakTime;
  float progress = 1.0 - ((float)timer / maxTime);
  int progressWidth = progress * BAR_WIDTH;

  M5.Lcd.fillRect(BAR_X, BAR_Y, BAR_WIDTH, BAR_HEIGHT, DARKGREY);
  M5.Lcd.fillRect(BAR_X, BAR_Y, progressWidth, BAR_HEIGHT, WHITE);
}

// Draw bar graph showing completed cycles
void drawGraph() {
  uint16_t bgColor = isWorking ? RED : GREEN;

  // Clear graph area with correct background color
  M5.Lcd.fillRect(0, GRAPH_Y_START - GRAPH_BAR_HEIGHT_MAX - 10, 240, GRAPH_BAR_HEIGHT_MAX + 20, bgColor);

  // Draw bars
  for (int i = 0; i < completedCycles && i < MAX_GRAPH_BARS; i++) {
    int barHeight = map(i + 1, 1, MAX_GRAPH_BARS, 10, GRAPH_BAR_HEIGHT_MAX);
    int barX = GRAPH_X_START + i * (GRAPH_BAR_WIDTH + GRAPH_BAR_SPACING);
    int barY = GRAPH_Y_START - barHeight;
    M5.Lcd.fillRect(barX, barY, GRAPH_BAR_WIDTH, barHeight, BLUE);
  }
}

// Toggle between start and pause
void toggleTimer() {
  isRunning = !isRunning;
  wakeScreen();
  lastSecondMillis = millis(); // Reset timer to prevent immediate tick
}

// Reset timer to beginning of current cycle
void resetTimer() {
  isRunning = false;
  timer = isWorking ? workTime : breakTime;
  wakeScreen();
  displayTime();
}

// Adjust work or break time
void adjustTime(bool increase) {
  if (isWorking) {
    workTime = max(MIN_TIME, workTime + (increase ? TIME_ADJUSTMENT : -TIME_ADJUSTMENT));
    timer = workTime;
  } else {
    breakTime = max(MIN_TIME, breakTime + (increase ? TIME_ADJUSTMENT : -TIME_ADJUSTMENT));
    timer = breakTime;
  }
  wakeScreen();
  saveSettings(); // Auto-save on adjustment
  displayTime();
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.setBrightness(255);

  // Load saved settings
  loadSettings();

  lastActivityMillis = millis();
  lastSecondMillis = millis();

  fullScreenRedraw();
  displayTime();
}

void loop() {
  M5.update();

  // Button A: Start or pause the timer
  if (M5.BtnA.wasPressed()) {
    toggleTimer();
  }

  // Button PWR: Reset the timer
  if (M5.BtnPWR.wasPressed()) {
    resetTimer();
  }

  // Button B: Dynamic adjustment with double-trigger fix
  // Track button press start
  if (M5.BtnB.wasPressed()) {
    btnBPressStart = millis();
    btnBLongPressHandled = false;
  }

  // Check for long press (held for 1 second)
  if (M5.BtnB.isPressed() && !btnBLongPressHandled) {
    if (millis() - btnBPressStart > 1000) {
      adjustTime(false); // Decrease time on long press
      btnBLongPressHandled = true;
      delay(BTN_DEBOUNCE_MS); // Debounce
    }
  }

  // Short press on release (only if long press wasn't handled)
  if (M5.BtnB.wasReleased() && !btnBLongPressHandled) {
    if (millis() - btnBPressStart < 1000) {
      adjustTime(true); // Increase time on short press
    }
  }

  // Non-blocking timer update
  unsigned long currentMillis = millis();
  if (isRunning && (currentMillis - lastSecondMillis >= SECOND_MS)) {
    lastSecondMillis = currentMillis;
    timer--;

    // Check if time is up
    if (timer <= 0) {
      isWorking = !isWorking;
      timer = isWorking ? workTime : breakTime;

      // Increment cycle counter at the end of a work cycle
      if (!isWorking) {
        completedCycles++;
        saveSettings(); // Auto-save cycle count
        drawGraph();
      }

      // Audio alert (non-blocking)
      M5.Speaker.tone(BEEP_FREQ, BEEP_DURATION);
      delay(BEEP_PAUSE);
      M5.Speaker.tone(BEEP_FREQ, BEEP_DURATION);
    }

    displayTime();
  }

  // Power management
  updatePowerManagement();
}
