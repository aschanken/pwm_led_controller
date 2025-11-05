# Animation Pattern Library

This document catalogs reusable LED animation patterns for the drone LED controller template. Each pattern includes implementation code, memory usage, and customization parameters.

---

## Basic Patterns (Low Memory)

### 1. Solid Color
**Memory**: 0 bytes state variables  
**Use Case**: Navigation lights, team colors, race mode  
**Customization**: Color, brightness

```cpp
void updateModeSolidColor() {
  // Set once in setMode(), no animation updates needed
  // Example: fillBoth(CRGB(255, 255, 255));
}
```

### 2. Breathing/Pulse
**Memory**: 4 bytes (offset, lastUpdate)  
**Use Case**: Idle mode, subtle animation  
**Customization**: Speed, min/max brightness, color

```cpp
static uint8_t breatheOffset = 0;
static unsigned long lastBreatheUpdate = 0;
const uint8_t BREATHE_SPEED_MS = 20;

void updateModeBreathe() {
  if (millis() - lastBreatheUpdate < BREATHE_SPEED_MS) return;
  lastBreatheUpdate = millis();
  
  breatheOffset++;
  uint8_t brightness = sin8(breatheOffset);  // 0-255 sine wave
  CRGB color = CRGB(255, 255, 255);
  color.nscale8(brightness);
  fillBoth(color);
}
```

### 3. Blink/Strobe
**Memory**: 5 bytes (state, lastUpdate)  
**Use Case**: Emergency, attention-getting  
**Customization**: On/off duration, color

```cpp
static bool strobeOn = false;
static unsigned long lastStrobeUpdate = 0;
const uint16_t STROBE_ON_MS = 50;
const uint16_t STROBE_OFF_MS = 200;

void updateModeStrobe() {
  unsigned long now = millis();
  uint16_t interval = strobeOn ? STROBE_ON_MS : STROBE_OFF_MS;
  
  if (now - lastStrobeUpdate >= interval) {
    lastStrobeUpdate = now;
    strobeOn = !strobeOn;
    fillBoth(strobeOn ? CRGB::White : CRGB::Black);
  }
}
```

---

## Chase/Moving Patterns (Medium Memory)

### 4. Single Chase
**Memory**: 5 bytes (offset, lastUpdate)  
**Use Case**: Dynamic effect, following mode  
**Customization**: Speed, chase length, color, direction

```cpp
static uint8_t chaseOffset = 0;
static unsigned long lastChaseUpdate = 0;
const uint8_t CHASE_SPEED_MS = 30;
const uint8_t CHASE_LENGTH = 4;

void updateModeChase() {
  if (millis() - lastChaseUpdate < CHASE_SPEED_MS) return;
  lastChaseUpdate = millis();
  
  chaseOffset = (chaseOffset + 1) % NUM_LEDS_1;
  
  fill_solid(leds1, NUM_LEDS_1, CRGB::Black);
  fill_solid(leds2, NUM_LEDS_2, CRGB::Black);
  
  for (uint8_t i = 0; i < CHASE_LENGTH; i++) {
    uint8_t pos = (chaseOffset + i) % NUM_LEDS_1;
    leds1[pos] = CRGB::Blue;
    leds2[pos] = CRGB::Blue;
  }
  FastLED.show();
}
```

### 5. Scrolling Gradient
**Memory**: 5 bytes (offset, lastUpdate)  
**Use Case**: Smooth flowing effect  
**Customization**: Speed, colors, gradient shape

```cpp
static uint8_t gradOffset = 0;
static unsigned long lastGradUpdate = 0;
const uint8_t GRAD_SPEED_MS = 10;

void updateModeGradient() {
  if (millis() - lastGradUpdate < GRAD_SPEED_MS) return;
  lastGradUpdate = millis();
  
  gradOffset++;
  const uint8_t cycleLen = NUM_LEDS_1 * 2;
  
  for (uint8_t j = 0; j < NUM_LEDS_1; j++) {
    uint8_t t = (j + gradOffset) % cycleLen;
    uint8_t posVal = (t < NUM_LEDS_1) ? t : (cycleLen - 1 - t);
    uint8_t val = 255 - ((uint16_t)posVal * 255U) / (NUM_LEDS_1 - 1);
    leds1[j] = CRGB(0, val, 0);  // Green gradient
    leds2[j] = CRGB(0, val, 0);
  }
  FastLED.show();
}
```

---

## Color Transition Patterns

### 6. Alternating Fade
**Memory**: 9 bytes (state flags, timer)  
**Use Case**: Team colors, smooth transitions  
**Customization**: Colors, fade duration, split pattern

```cpp
static bool fading = false;
static bool swapped = false;
static unsigned long fadeStartTime = 0;
const uint16_t FADE_DURATION_MS = 500;

void updateModeAlternatingFade() {
  if (!fading) {
    fading = true;
    fadeStartTime = millis();
  }
  
  unsigned long dt = millis() - fadeStartTime;
  uint8_t progress = (dt >= FADE_DURATION_MS) ? 255 : 
                     (uint8_t)((dt * 255UL) / FADE_DURATION_MS);
  
  CRGB fromA = swapped ? CRGB::Green : CRGB::Yellow;
  CRGB toA = swapped ? CRGB::Yellow : CRGB::Green;
  CRGB cA = blendColours(fromA, toA, progress);
  
  fillBoth(cA);
  
  if (progress == 255) {
    fading = false;
    swapped = !swapped;
  }
}
```

### 7. Rainbow Cycle
**Memory**: 5 bytes (hueOffset, lastUpdate)  
**Use Case**: Party mode, visibility  
**Customization**: Speed, saturation, brightness

```cpp
static uint8_t rainbowHue = 0;
static unsigned long lastRainbowUpdate = 0;
const uint8_t RAINBOW_SPEED_MS = 20;

void updateModeRainbow() {
  if (millis() - lastRainbowUpdate < RAINBOW_SPEED_MS) return;
  lastRainbowUpdate = millis();
  
  rainbowHue++;
  
  for (uint8_t i = 0; i < NUM_LEDS_1; i++) {
    leds1[i] = CHSV(rainbowHue + (i * 255 / NUM_LEDS_1), 255, 255);
    leds2[i] = CHSV(rainbowHue + (i * 255 / NUM_LEDS_2), 255, 255);
  }
  FastLED.show();
}
```

---

## Complex Patterns (Higher Memory)

### 8. Police/Emergency Lights
**Memory**: 13 bytes (stage, timer, PROGMEM table)  
**Use Case**: Attention-getting, special effects  
**Customization**: Stage timing, colors, section sizes

```cpp
static uint8_t policeStage = 0;
static unsigned long lastPoliceUpdate = 0;
const uint8_t POLICE_INTERVAL_MS = 22;

const uint8_t policeFlags[8] PROGMEM = {
  0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07
};

void updateModePolice() {
  // See TEMPLATE_BASE.ino updateMode4() for full implementation
  // Alternates red/blue in sections with intensity variations
}
```

### 9. Fire Effect
**Memory**: 20+ bytes (per-LED heat array)  
**Use Case**: Dramatic effect, limited by memory  
**Customization**: Cooling, sparking, color palette

```cpp
// WARNING: High memory usage for ATtiny85
// Only suitable for strips <15 LEDs
static uint8_t heat1[NUM_LEDS_1];
const uint8_t COOLING = 55;
const uint8_t SPARKING = 120;

void updateModeFire() {
  // Simplified fire algorithm
  // Full implementation requires careful memory management
}
```

### 10. Theater Chase
**Memory**: 5 bytes (position, lastUpdate)  
**Use Case**: Professional looking chase effect  
**Customization**: Speed, spacing, color

```cpp
static uint8_t theaterPos = 0;
static unsigned long lastTheaterUpdate = 0;
const uint8_t THEATER_SPEED_MS = 100;

void updateModeTheaterChase() {
  if (millis() - lastTheaterUpdate < THEATER_SPEED_MS) return;
  lastTheaterUpdate = millis();
  
  theaterPos = (theaterPos + 1) % 3;
  
  for (uint8_t i = 0; i < NUM_LEDS_1; i++) {
    leds1[i] = ((i + theaterPos) % 3 == 0) ? CRGB::Red : CRGB::Black;
    leds2[i] = ((i + theaterPos) % 3 == 0) ? CRGB::Red : CRGB::Black;
  }
  FastLED.show();
}
```

---

## Memory Usage Summary

| Pattern | State Variables | PROGMEM | Total | Complexity |
|---------|----------------|---------|-------|------------|
| Solid Color | 0 bytes | 0 bytes | 0 bytes | Trivial |
| Breathing | 4 bytes | 0 bytes | 4 bytes | Low |
| Blink | 5 bytes | 0 bytes | 5 bytes | Low |
| Chase | 5 bytes | 0 bytes | 5 bytes | Low |
| Gradient | 5 bytes | 0 bytes | 5 bytes | Medium |
| Fade | 9 bytes | 0 bytes | 9 bytes | Medium |
| Rainbow | 5 bytes | 0 bytes | 5 bytes | Medium |
| Police | 5 bytes | 8 bytes | 13 bytes | High |
| Theater | 5 bytes | 0 bytes | 5 bytes | Low |

**Note**: Always validate total memory usage stays under 512 bytes for ATtiny85.

---

## Helper Functions

These utility functions are available in TEMPLATE_BASE.ino:

```cpp
// Fill both strips with same color
void fillBoth(const CRGB &colour);

// Blend two colors (weight 0-255)
CRGB blendColours(const CRGB &from, const CRGB &to, uint8_t weight);
```

Additional FastLED functions commonly used:
- `fill_solid()` - Fill array with one color
- `fill_rainbow()` - Fill with rainbow gradient
- `fadeToBlackBy()` - Fade entire strip
- `nscale8()` - Scale brightness
- `CHSV()` - HSV color space (great for rainbows)
