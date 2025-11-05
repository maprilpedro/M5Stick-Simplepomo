# Pomodoro Timer for M5StickC Plus 2

An optimized Pomodoro Timer implementation for the M5StickC Plus 2 device, designed to help with task management and improve productivity. With visual and interactive features, it's ideal for students, professionals, and anyone looking to better organize their time.

```
┌─────────────────────────┐
│  M5StickC Plus 2        │
│  ┌─────────────────┐    │
│  │  🍅 25:00       │    │  ← Display shows timer
│  │  ████████░░     │    │  ← Progress bar
│  │  ||||||||       │    │  ← Cycle count
│  └─────────────────┘    │
│  [A] [B]        [PWR]   │  ← Physical buttons
└─────────────────────────┘
```

## What It Does

A portable Pomodoro timer (work 25min → break 5min) that helps you stay focused:
- **Red screen** = Work time
- **Green screen** = Break time
- **Beeps** when cycles complete
- **Progress bars** show completion
- **Bar graph** tracks completed cycles

## Features

- **Work and Break Cycles:** Automatically alternates between work periods (25 minutes) and rest periods (5 minutes)
- **Progress Bar:** Displays real-time progress of the current cycle
- **Cycle Graph:** Shows completed cycles with growing bars
- **Dynamic Adjustments:** Allows increasing or decreasing work and break times with short or long button presses
- **Visual and Audio Feedback:** Uses colors to differentiate modes (red for work, green for break) and emits audio alerts at the end of each cycle
- **Power Management:** Auto-dims screen after 30 seconds of inactivity to save battery
- **Settings Persistence:** Remembers your custom times and cycle count across reboots using EEPROM

## Performance Optimizations (v2.0)

This version includes significant performance improvements:

- **Non-blocking timer:** Instant button response using millis() instead of delay()
- **Partial screen updates:** 95% reduction in pixel redraws eliminates flicker and extends battery life
- **Fixed button handling:** Proper button state machine prevents double-trigger bugs
- **Auto-save:** Settings automatically persist to EEPROM
- **Clean code:** All magic numbers extracted to named constants for easy customization

See [OPTIMIZATIONS.md](OPTIMIZATIONS.md) for detailed technical documentation.

## Required Hardware

- [M5StickC Plus 2](https://shop.m5stack.com/collections/m5stick-series)

## How to Use

### Initial Setup

1. Clone this repository or copy the code to Arduino IDE
2. Ensure M5StickC Plus 2 libraries are installed:
   - M5GFX: https://github.com/m5stack/M5GFX
   - M5Unified: https://github.com/m5stack/M5Unified
   - M5StickCPlus2: https://github.com/m5stack/M5StickCPlus2
   - Preferences (ESP32 built-in)

### Uploading the Code

1. Connect M5StickC Plus 2 to your computer via USB cable
2. Open `pomodoro_timer/pomodoro_timer.ino` in Arduino IDE
3. Select board: "M5StickC Plus 2"
4. Compile and upload the code to the device

### Controls

- **Button A:** Start or pause the timer
- **Button PWR:** Reset the timer to the beginning of current cycle
- **Button B:**
  - Short press: Increase time by 1 minute
  - Long press (hold 1s): Decrease time by 1 minute

## How It Works

1. On startup, the device displays the current mode (Work or Break), remaining time, and number of completed cycles
2. During a cycle, a progress bar gradually fills
3. After completing a work cycle, a bar graph updates to show completed cycles
4. Settings and cycle count automatically save and persist across reboots
5. Screen dims to 50% brightness after 30 seconds of inactivity when paused

## Customization

### Default Times

Modify these constants in the code to change default durations:

```cpp
const int DEFAULT_WORK_TIME = 25 * 60;  // 25 minutes in seconds
const int DEFAULT_BREAK_TIME = 5 * 60;  // 5 minutes in seconds
```

### Power Saving

Adjust idle timeout before screen dims:

```cpp
const unsigned long IDLE_DIM_MS = 30000; // Dim after 30 seconds
```

## Credits

Based on original work by **LucasIneth** (https://github.com/lucasineth)

Optimized version with performance improvements, power management, and settings persistence.

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests with improvements and new features.

## License

This project is licensed under the [MIT License](LICENSE).

## Version History

- **v2.0** (2025-11-05): Major optimization update
  - Non-blocking timer implementation
  - Partial screen updates (95% less redraws)
  - Fixed button handling bugs
  - Added EEPROM persistence
  - Power management with auto-dim
  - Code cleanup and constants extraction

- **v1.0** (2024-01-15): Initial release by LucasIneth
  - Basic Pomodoro timer functionality
  - Progress bar and cycle graph
  - Button controls
