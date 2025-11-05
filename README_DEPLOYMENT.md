# Deployment Guide - Creating Custom Drone LED Firmware

This guide walks you through creating bespoke LED controller firmware for customers using the template system.

---

## Overview

The template system allows you to quickly create custom firmware by:
1. Defining customer requirements (4 LED modes)
2. Selecting patterns from the animation library
3. Customizing colors, speeds, and behaviors
4. Compiling and testing the firmware
5. Deploying to the customer's ATtiny85

**Typical turnaround time**: 30-60 minutes per customer

---

## Prerequisites

### Hardware
- ATtiny85-20PU microcontroller
- USBasp or similar AVR programmer
- 2x WS2812B LED strips (customer-specific lengths)
- Bench power supply (5V)
- Servo tester or PWM signal generator

### Software
- Arduino IDE or arduino-cli
- ATtiny board support package
- FastLED library (install via Library Manager)

### Installation Commands
```bash
# Install arduino-cli (if not already installed)
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Install ATtiny board support
arduino-cli core install attiny:avr

# Install FastLED library
arduino-cli lib install FastLED
```

---

## Workflow: Creating Custom Firmware

### Step 1: Customer Requirements Meeting

**Questions to ask:**
1. How many LEDs per strip? (typically 12-24)
2. What's your drone size? (affects strip placement)
3. What are your 4 desired lighting modes?
4. Any specific colors or team colors?
5. Desired brightness levels? (battery consideration)
6. Any special effects wanted?

**Document answers** in a customer profile file.

### Step 2: Create Customer Firmware File

```bash
cd /path/to/led_controller_template
cp TEMPLATE_BASE.ino customer_name_led_controller.ino
```

### Step 3: Configure Hardware Parameters

Edit the new `.ino` file:

```cpp
// Update LED counts
#define NUM_LEDS_1  16    // Customer's strip 1 length
#define NUM_LEDS_2  16    // Customer's strip 2 length
```

### Step 4: Rename and Configure Modes

Update the Mode enum with descriptive names:

```cpp
enum Mode : uint8_t {
  MODE_POWER_OFF     = 0,
  MODE_RACE_WHITE    = 1,  // Customer mode 1
  MODE_CHASE_BLUE    = 2,  // Customer mode 2
  MODE_TEAM_COLORS   = 3,  // Customer mode 3
  MODE_RAINBOW_PARTY = 4   // Customer mode 4
};
```

### Step 5: Implement Each Mode

For each mode, refer to `ANIMATION_LIBRARY.md` and implement in the corresponding `updateMode*()` function.

**Example: Mode 1 - Solid White**
```cpp
void updateMode1() {
  // Static - already set in setMode()
  // No per-frame updates needed
}

// In setMode():
case MODE_RACE_WHITE:
  FastLED.setBrightness(255);  // 100% brightness
  fillBoth(CRGB::White);
  break;
```

**Example: Mode 2 - Chase Effect**
```cpp
void updateMode2() {
  if (millis() - lastMode2Update < MODE2_UPDATE_INTERVAL_MS) return;
  lastMode2Update = millis();
  
  mode2Offset = (mode2Offset + 1) % NUM_LEDS_1;
  
  fill_solid(leds1, NUM_LEDS_1, CRGB::Black);
  fill_solid(leds2, NUM_LEDS_2, CRGB::Black);
  
  for (uint8_t i = 0; i < 4; i++) {  // 4 LED chase
    uint8_t pos = (mode2Offset + i) % NUM_LEDS_1;
    leds1[pos] = CRGB::Blue;
    leds2[pos] = CRGB::Blue;
  }
  FastLED.show();
}
```

### Step 6: Adjust Brightness Levels

Set appropriate brightness in `setMode()` for each mode:

```cpp
case MODE_1:
  FastLED.setBrightness(255);  // 100%
  break;
case MODE_2:
  FastLED.setBrightness(204);  // 80%
  break;
```

**Brightness guide:**
- Racing/visibility: 80-100%
- General flying: 50-70%
- Battery saving: 30-50%

### Step 7: Configure Animation Speeds

Adjust timing constants for each mode:

```cpp
const uint8_t MODE2_UPDATE_INTERVAL_MS = 15;   // Fast chase
const uint16_t MODE3_FADE_DURATION_MS = 500;   // Smooth fade
const uint8_t MODE4_INTERVAL_MS = 20;          // Rapid effect
```

### Step 8: Verify Memory Usage

Calculate total SRAM usage:
- LED arrays: `(NUM_LEDS_1 + NUM_LEDS_2) * 3` bytes
- State variables: Sum all `static` variables
- Target: **< 512 bytes total**

**Example calculation:**
```
LEDs: (16 + 16) * 3 = 96 bytes
Mode state vars: ~30 bytes
FastLED overhead: ~50 bytes
Stack: ~40 bytes
TOTAL: ~216 bytes ✓ (well under limit)
```

---

## Testing Procedure

### Bench Test Setup

1. **Connect hardware:**
   - ATtiny85 pin 5 (PB0) → LED Strip 1 data
   - ATtiny85 pin 3 (PB4) → LED Strip 2 data
   - ATtiny85 pin 7 (PB2) → Servo tester PWM signal
   - ATtiny85 pin 2 (PB3) → Test button (pull to ground)
   - 5V and GND to power

2. **Power on LED strips** with bench supply (5V, adequate current)

### Test Sequence

**Test 1: Sanity Mode**
```
1. Pull PB3 (pin 2) to ground for 100ms
2. Should see white blink
3. Modes should cycle every 1 second (1→2→3→4→1...)
4. Release PB3 to return to normal operation
```

**Test 2: PWM Mode Selection**
```
1. Set servo tester to 1125µs → Should activate Mode 1
2. Set servo tester to 1375µs → Should activate Mode 2
3. Set servo tester to 1625µs → Should activate Mode 3
4. Set servo tester to 1875µs → Should activate Mode 4
5. Set servo tester to 500µs → Should turn off (power save)
```

**Test 3: Transition Smoothness**
```
1. Slowly sweep servo tester from 1000µs to 2000µs
2. Mode changes should be debounced (no flickering)
3. Verify each mode activates in correct PWM range
```

**Test 4: Verify Each Animation**
```
For each mode:
- Check colors are correct
- Verify animation speed matches requirements
- Confirm brightness level
- Test for any glitches or artifacts
```

---

## Compilation and Upload

### Using arduino-cli

```bash
# Compile
arduino-cli compile --fqbn attiny:avr:ATtiny85 customer_name_led_controller.ino

# Upload (adjust port as needed)
arduino-cli upload -p /dev/ttyUSB0 --fqbn attiny:avr:ATtiny85 customer_name_led_controller.ino
```

### Using Arduino IDE

1. Open `customer_name_led_controller.ino`
2. Select: **Tools → Board → ATtiny25/45/85**
3. Select: **Tools → Processor → ATtiny85**
4. Select: **Tools → Clock → 8MHz (internal)**
5. Select: **Tools → Programmer → USBasp**
6. Click **Sketch → Upload**

### Common Compilation Issues

**Error: "FastLED.h not found"**
```bash
arduino-cli lib install FastLED
```

**Error: "regions 'text' overflowed"**
- Code too large for ATtiny85 (8KB limit)
- Simplify patterns or reduce PROGMEM tables
- Remove unused animations

**Error: "SRAM overflow"**
- Using > 512 bytes RAM
- Reduce NUM_LEDS or simplify state variables
- Use more PROGMEM storage

---

## Customer Delivery

### What to Provide

1. **Programmed ATtiny85** (tested and verified)
2. **Wiring diagram** (pin connections for their drone)
3. **Mode reference card:**
   ```
   PWM Range     Mode
   ---------     ----
   < 1000µs      OFF
   1000-1249µs   Mode 1: [Description]
   1250-1499µs   Mode 2: [Description]
   1500-1749µs   Mode 3: [Description]
   1750-2000µs   Mode 4: [Description]
   ```
4. **Installation video** (if first-time customer)
5. **Source code** (backup copy on USB drive)

### Documentation Template

Create a simple text file for the customer:

```
==============================================
CUSTOM LED CONTROLLER - [Customer Name]
==============================================

MODES:
1. [1000-1249µs] Race Mode - Bright white for visibility
2. [1250-1499µs] Chase Blue - Blue chase effect
3. [1500-1749µs] Team Colors - Orange/black alternating
4. [1750-2000µs] Rainbow Party - Full spectrum rotation

WIRING:
- Red wire (5V) → Battery + (through BEC)
- Black wire (GND) → Battery -
- Yellow wire (Data 1) → Front strip data
- Green wire (Data 2) → Rear strip data
- White wire (PWM) → Radio channel [X]

TROUBLESHOOTING:
- No lights: Check 5V power and GND
- Wrong modes: Verify PWM signal wire
- Flickering: Check data wire connections

FIRMWARE VERSION: v5.4_[Customer]_[Date]
==============================================
```

---

## Version Control Best Practices

### File Naming Convention
```
[customer]_[drone_type]_led_controller_v[X.Y].ino

Examples:
- johndoe_75mm_whoop_led_controller_v1.0.ino
- raceteam_5inch_freestyle_led_controller_v2.1.ino
```

### Git Repository Structure
```
led_controllers/
├── TEMPLATE_BASE.ino
├── ANIMATION_LIBRARY.md
├── README_DEPLOYMENT.md
├── customers/
│   ├── 2024_01_johndoe/
│   │   ├── johndoe_led_controller.ino
│   │   ├── customer_notes.txt
│   │   └── test_results.txt
│   ├── 2024_02_raceteam/
│   │   └── ...
```

### Backup Strategy
- Commit each customer firmware to git
- Tag releases: `git tag customer_johndoe_v1.0`
- Keep notes on customizations made
- Store test results for warranty support

---

## Advanced Customization

### Creating New Patterns

If a customer requests a pattern not in the library:

1. Prototype in `updateMode*()` function
2. Test memory usage with `avr-size`
3. Document in customer notes
4. Consider adding to animation library for reuse

### Multi-Strip Variations

For different effects per strip:

```cpp
void updateMode2() {
  // Strip 1: Chase forward
  // Strip 2: Chase backward
  for (uint8_t i = 0; i < NUM_LEDS_1; i++) {
    leds1[i] = (i == mode2Offset) ? CRGB::Blue : CRGB::Black;
    leds2[NUM_LEDS_2-1-i] = (i == mode2Offset) ? CRGB::Blue : CRGB::Black;
  }
  FastLED.show();
}
```

### Color Customization

Create customer-specific color palettes:

```cpp
// Team colors
#define TEAM_PRIMARY   CRGB(255, 100, 0)   // Orange
#define TEAM_SECONDARY CRGB(0, 0, 0)       // Black
#define TEAM_ACCENT    CRGB(255, 255, 255) // White
```

---

## Troubleshooting Guide

### Problem: LEDs don't light up
- Check 5V power supply (adequate current?)
- Verify data pin connections
- Test with known-good LED strip
- Confirm FastLED.show() is called

### Problem: Wrong colors displayed
- Check COLOR_ORDER (GRB vs RGB)
- Verify LED_TYPE matches strip model
- Test with simple solid color

### Problem: Modes don't change
- Verify SERVO_PIN receiving PWM signal
- Check debounce logic in loop()
- Test with servo tester (known PWM values)

### Problem: Erratic behavior
- Memory overflow? Check SRAM usage
- Power supply brownout? Add capacitor
- Interference on data line? Add resistor

### Problem: Compilation errors
- Missing library? Install FastLED
- Syntax error? Check semicolons/brackets
- Memory overflow? Simplify code

---

## Quick Reference

### PWM Ranges (Standard)
| Range | Mode | Typical Use |
|-------|------|-------------|
| 0-999µs | OFF | Power saving |
| 1000-1249µs | Mode 1 | Primary/racing |
| 1250-1499µs | Mode 2 | Secondary |
| 1500-1749µs | Mode 3 | Tertiary |
| 1750-2000µs | Mode 4 | Special/party |

### Memory Budget (ATtiny85)
- Flash: 8192 bytes (program code)
- SRAM: 512 bytes (variables)
- EEPROM: 512 bytes (unused in template)

### Typical Pattern Memory Usage
- Static: 0-5 bytes
- Animated: 5-15 bytes
- Complex: 15-30 bytes per mode

### FastLED Common Functions
```cpp
fill_solid(leds, count, color);      // Fill with one color
FastLED.show();                      // Update LEDs
FastLED.setBrightness(0-255);        // Master brightness
CRGB(r, g, b);                       // RGB color
CHSV(hue, sat, val);                 // HSV color
blendColours(from, to, weight);      // Custom blend (template)
```

---

## Support and Maintenance

### Warranty Period
- Offer free firmware updates for [X] months
- Bug fixes provided at no charge
- New pattern requests quoted separately

### Customer Support Checklist
- Keep copy of firmware source
- Document any special requests
- Provide troubleshooting guide
- Offer installation support video call

---

## Next Steps

1. **Practice run**: Create a test firmware using TEMPLATE_BASE.ino
2. **Build animation library**: Test and document 10+ patterns
3. **Streamline workflow**: Create scripts for common tasks
4. **Market your service**: Showcase custom patterns on social media
5. **Gather feedback**: Improve template based on customer requests

---

**Template Version**: 5.4  
**Last Updated**: 2024  
**Author**: [Your Name/Business]  
**Support**: [Contact Info]
