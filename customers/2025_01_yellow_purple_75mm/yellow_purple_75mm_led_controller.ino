/*
 * ============================================================================
 * YELLOW/PURPLE 75MM DRONE - CUSTOM LED CONTROLLER
 * ============================================================================
 * 
 * Customer: Yellow Frame / Purple Canopy 75mm Drone
 * Date: 2025-11-02
 * Firmware Version: v1.2 (Seamless + Faster Rainbow)
 * 
 * HARDWARE:
 *   - MCU: ATTiny85-20PU
 *   - LEDs: 2x 18 WS2812B strips (36 total)
 *   - Input: PWM signal (0-2000µs) for mode selection
 *   - Memory Usage: ~222 bytes / 512 bytes SRAM (43%)
 * 
 * PWM MODE MAPPING:
 *   - <1000µs: Power off (all LEDs off)
 *   - 1000-1249µs: Mode 1 - Yellow Spinning Blocks (70%→30% brightness)
 *   - 1250-1499µs: Mode 2 - Purple/Yellow Fading Blocks (50% brightness)
 *   - 1500-1749µs: Mode 3 - Rainbow Scroll (scrolling spectrum w/ color shift)
 *   - 1750-2000µs: Mode 4 - Police Lights (red/blue emergency)
 * 
 * SANITY TEST MODE:
 *   Pull SANITY_PIN low for 50ms to cycle through all modes (1s each)
 * 
 * ============================================================================
 */

#include <FastLED.h>
#include <avr/pgmspace.h>

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// LED strip configuration - Customer: 18 LEDs per strip
#define LED_PIN_1   0     // PB0 (physical pin 5) - Front strip
#define LED_PIN_2   4     // PB4 (physical pin 3) - Rear strip
#define NUM_LEDS_1  18    // Front strip LED count
#define NUM_LEDS_2  18    // Rear strip LED count
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// Control pins (standard)
#define SERVO_PIN   2     // PB2 (physical pin 7) - PWM input
#define SANITY_PIN  3     // PB3 (physical pin 2) - Test mode trigger

// PWM signal configuration (standard 1000-2000µs)
const int SERVO_MIN_PULSE = 1000;
const int SERVO_MAX_PULSE = 2000;
const int QUARTER_WIDTH   = (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / 4; // 250µs

// ============================================================================
// MODE DEFINITIONS
// ============================================================================

enum Mode : uint8_t {
  MODE_POWER_OFF         = 0,
  MODE_YELLOW_SPIN       = 1,  // Yellow spinning blocks (70%→30%)
  MODE_PURPLE_YELLOW_FADE = 2,  // Purple→Yellow fading blocks (50%)
  MODE_RAINBOW_SCROLL    = 3,  // Scrolling rainbow with color shift
  MODE_POLICE_LIGHTS     = 4   // Red/Blue emergency lights
};

// ============================================================================
// GLOBAL STATE
// ============================================================================

// LED arrays (memory: 36 LEDs × 3 bytes = 108 bytes)
CRGB leds1[NUM_LEDS_1];
CRGB leds2[NUM_LEDS_2];

// Current mode tracking
static Mode currentMode = MODE_POWER_OFF;
static int8_t lastQuarter = -1;

// ============================================================================
// MODE-SPECIFIC STATE VARIABLES
// ============================================================================

// Mode 1: Yellow Spinning Blocks (5 bytes)
static uint8_t mode1Offset = 0;
static unsigned long lastMode1Update = 0;
const uint8_t MODE1_SPIN_SPEED_MS = 25;  // Smooth rotation speed
const uint8_t MODE1_BLOCK_SIZE = 6;      // Size of bright block

// Mode 2: Purple-Yellow Fading Blocks (9 bytes)
static uint8_t mode2Offset = 0;
static unsigned long lastMode2Update = 0;
static uint8_t mode2FadeProgress = 0;
static unsigned long lastMode2FadeUpdate = 0;
const uint8_t MODE2_ROTATE_SPEED_MS = 50;    // Block rotation speed
const uint16_t MODE2_FADE_CYCLE_MS = 1000;   // Color morph duration

// Mode 3: Rainbow Scroll (5 bytes)
static uint8_t mode3RainbowHue = 0;
static unsigned long lastMode3Update = 0;
const uint8_t MODE3_RAINBOW_SPEED_MS = 7;   // Fast scrolling speed (2x faster)
const uint8_t MODE3_HUE_SPACING = 7;        // Color spread across LEDs

// Mode 4: Police Lights (5 bytes + 8 bytes PROGMEM)
static uint8_t mode4Stage = 0;
static unsigned long lastMode4Update = 0;
const uint8_t MODE4_INTERVAL_MS = 22;  // Stage duration

// Police light pattern stages (stored in flash memory)
const uint8_t policeFlags[8] PROGMEM = {
  0x01, // stage 0: firstHigh=1, secondHigh=0, invertColours=0
  0x02, // stage 1: firstHigh=0, secondHigh=1, invertColours=0
  0x00, // stage 2: firstHigh=0, secondHigh=0, invertColours=0
  0x03, // stage 3: firstHigh=1, secondHigh=1, invertColours=0
  0x04, // stage 4: firstHigh=0, secondHigh=0, invertColours=1
  0x05, // stage 5: firstHigh=1, secondHigh=0, invertColours=1
  0x06, // stage 6: firstHigh=0, secondHigh=1, invertColours=1
  0x07  // stage 7: firstHigh=1, secondHigh=1, invertColours=1
};

// ============================================================================
// CUSTOMER COLOR DEFINITIONS
// ============================================================================

// Yellow (frame color)
const CRGB YELLOW = CRGB(255, 255, 0);

// Purple (canopy color)
const CRGB PURPLE = CRGB(128, 0, 255);

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void setMode(Mode newMode);
void updateAnimations();
void updatePowerOff();
void updateMode1();
void updateMode2();
void updateMode3();
void updateMode4();

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Fill both LED strips with the same color and update
inline void fillBoth(const CRGB &colour) {
  fill_solid(leds1, NUM_LEDS_1, colour);
  fill_solid(leds2, NUM_LEDS_2, colour);
  FastLED.show();
}

// Linear color blend without floating point (weight: 0-255)
static inline CRGB blendColours(const CRGB &from, const CRGB &to, uint8_t weight) {
  uint16_t r = (uint16_t)from.r * (255 - weight) + (uint16_t)to.r * weight;
  uint16_t g = (uint16_t)from.g * (255 - weight) + (uint16_t)to.g * weight;
  uint16_t b = (uint16_t)from.b * (255 - weight) + (uint16_t)to.b * weight;
  return CRGB((uint8_t)(r >> 8), (uint8_t)(g >> 8), (uint8_t)(b >> 8));
}

// ============================================================================
// ARDUINO SETUP
// ============================================================================

void setup() {
  // Configure pins
  pinMode(SERVO_PIN, INPUT);
  pinMode(SANITY_PIN, INPUT_PULLUP);
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(128);  // Default 50% brightness
  
  // Start with all LEDs off
  fillBoth(CRGB::Black);
}

// ============================================================================
// ARDUINO MAIN LOOP
// ============================================================================

void loop() {
  // --- Sanity mode handling (testing mode - DO NOT MODIFY) ---
  {
    static bool inSanity = false;
    static Mode savedMode = MODE_POWER_OFF;
    static uint8_t sanityIndex = 0;
    static unsigned long sanityLastSwitch = 0;
    static unsigned long sanityLowStart = 0;
    unsigned long now = millis();
    bool sanityLow = (digitalRead(SANITY_PIN) == LOW);
    
    if (!inSanity) {
      if (sanityLow) {
        if (sanityLowStart == 0) {
          sanityLowStart = now;
        } else if (now - sanityLowStart > 50) {
          // Enter sanity mode after 50ms low
          inSanity = true;
          savedMode = currentMode;
          // Blink white to indicate entry
          uint8_t prevBrightness = FastLED.getBrightness();
          FastLED.setBrightness(255);
          fillBoth(CRGB::White);
          delay(80);
          fillBoth(CRGB::Black);
          FastLED.setBrightness(prevBrightness);
          // Start with first mode
          sanityIndex = MODE_YELLOW_SPIN;
          sanityLastSwitch = now;
          setMode(static_cast<Mode>(sanityIndex));
        }
      } else {
        sanityLowStart = 0;
      }
    } else {
      // In sanity mode; wait until pin high for >50ms to exit
      if (!sanityLow) {
        if (sanityLowStart == 0) {
          sanityLowStart = now;
        } else if (now - sanityLowStart > 50) {
          inSanity = false;
          sanityLowStart = 0;
          setMode(savedMode);
        }
      } else {
        sanityLowStart = 0;
      }
      // Cycle through modes every 1s
      if (now - sanityLastSwitch >= 1000UL) {
        sanityLastSwitch = now;
        uint8_t next = sanityIndex + 1;
        if (next > MODE_POLICE_LIGHTS) next = MODE_YELLOW_SPIN;
        sanityIndex = next;
        setMode(static_cast<Mode>(sanityIndex));
      }
      updateAnimations();
      return;
    }
  }
  
  // --- Normal operation: Read PWM input and select mode ---
  unsigned long pulseWidth = pulseIn(SERVO_PIN, HIGH, 50000UL);
  
  // Power off for signals <1000µs (or no signal)
  if (pulseWidth == 0 || pulseWidth < 1000) {
    if (currentMode != MODE_POWER_OFF) {
      setMode(MODE_POWER_OFF);
    }
  } else {
    // Clamp to valid range
    if (pulseWidth < SERVO_MIN_PULSE) pulseWidth = SERVO_MIN_PULSE;
    if (pulseWidth > SERVO_MAX_PULSE) pulseWidth = SERVO_MAX_PULSE;
    
    // Calculate quarter (0-3)
    int quarter = (pulseWidth - SERVO_MIN_PULSE) / QUARTER_WIDTH;
    if (quarter < 0) quarter = 0;
    if (quarter > 3) quarter = 3;
    
    // Debounce quarter transitions
    {
      static int8_t pendingQuarter = -1;
      static uint8_t pendingCount = 0;
      const uint8_t MODE_DEBOUNCE_COUNT = 3;
      
      if (quarter != pendingQuarter) {
        pendingQuarter = quarter;
        pendingCount = 1;
      } else if (pendingCount < MODE_DEBOUNCE_COUNT) {
        pendingCount++;
      }
      
      if (pendingCount >= MODE_DEBOUNCE_COUNT && quarter != lastQuarter) {
        lastQuarter = quarter;
        // Map quarter to mode
        Mode m;
        switch (quarter) {
          case 0: m = MODE_YELLOW_SPIN; break;
          case 1: m = MODE_PURPLE_YELLOW_FADE; break;
          case 2: m = MODE_RAINBOW_SCROLL; break;
          default: m = MODE_POLICE_LIGHTS; break;
        }
        setMode(m);
      }
    }
  }
  
  // Update current animation
  updateAnimations();
}

// ============================================================================
// MODE MANAGEMENT
// ============================================================================

void setMode(Mode newMode) {
  if (newMode == currentMode) return;
  currentMode = newMode;
  
  // Reset all mode state variables
  mode1Offset = 0;
  lastMode1Update = 0;
  mode2Offset = 0;
  lastMode2Update = 0;
  mode2FadeProgress = 0;
  lastMode2FadeUpdate = 0;
  mode3RainbowHue = 0;
  lastMode3Update = 0;
  mode4Stage = 0;
  lastMode4Update = 0;
  
  // Clear LED strips
  fillBoth(CRGB::Black);
  
  // Configure brightness and initial state for each mode
  switch (currentMode) {
    case MODE_POWER_OFF:
      FastLED.setBrightness(0);
      break;
      
    case MODE_YELLOW_SPIN:
      FastLED.setBrightness(255);  // Will be modulated in update function
      break;
      
    case MODE_PURPLE_YELLOW_FADE:
      FastLED.setBrightness(128);  // 50% brightness
      break;
      
    case MODE_RAINBOW_SCROLL:
      FastLED.setBrightness(255);  // Brightness controlled in HSV value
      break;
      
    case MODE_POLICE_LIGHTS:
      FastLED.setBrightness(255);  // Intensities handled in pattern
      break;
  }
}

// Dispatch to current mode update function
void updateAnimations() {
  switch (currentMode) {
    case MODE_POWER_OFF:
      updatePowerOff();
      break;
    case MODE_YELLOW_SPIN:
      updateMode1();
      break;
    case MODE_PURPLE_YELLOW_FADE:
      updateMode2();
      break;
    case MODE_RAINBOW_SCROLL:
      updateMode3();
      break;
    case MODE_POLICE_LIGHTS:
      updateMode4();
      break;
  }
}

// ============================================================================
// MODE UPDATE FUNCTIONS
// ============================================================================

// Power Off - No animation needed
void updatePowerOff() {
  // LEDs remain off as set in setMode()
}

// ============================================================================
// MODE 1: Yellow Spinning Blocks
// ============================================================================
// Bright yellow blocks spin around drone perimeter (both strips as one ring)
// Brightness: 70% at peak, fading to 30% at valley

void updateMode1() {
  unsigned long now = millis();
  if (now - lastMode1Update < MODE1_SPIN_SPEED_MS) return;
  lastMode1Update = now;
  
  mode1Offset++;
  
  // Treat both strips as a virtual 36-LED ring (0-35)
  const uint8_t TOTAL_LEDS = NUM_LEDS_1 + NUM_LEDS_2;
  
  // Create spinning gradient effect
  for (uint8_t virtualPos = 0; virtualPos < TOTAL_LEDS; virtualPos++) {
    // Calculate distance from current "bright spot"
    uint8_t distance = (virtualPos + TOTAL_LEDS - mode1Offset) % TOTAL_LEDS;
    
    // Create gradient: bright in center, fading outward
    uint8_t brightness;
    if (distance < MODE1_BLOCK_SIZE) {
      // Bright section (70% = 178)
      brightness = 178;
    } else if (distance < MODE1_BLOCK_SIZE + 12) {
      // Fade from 70% down to 30%
      uint8_t fadePos = distance - MODE1_BLOCK_SIZE;
      brightness = 178 - ((uint16_t)fadePos * 102) / 12;  // 178 → 76 (70% → 30%)
    } else {
      // Dim section (30% = 76)
      brightness = 76;
    }
    
    // Create yellow color with calculated brightness
    CRGB color = YELLOW;
    color.nscale8(brightness);
    
    // Map virtual position to physical LED
    if (virtualPos < NUM_LEDS_1) {
      leds1[virtualPos] = color;
    } else {
      leds2[virtualPos - NUM_LEDS_1] = color;
    }
  }
  
  FastLED.show();
}

// ============================================================================
// MODE 2: Purple-Yellow Fading Blocks
// ============================================================================
// Blocks rotate and fade between purple and yellow at 50% brightness
// Seamless across both strips (36-LED virtual ring)

void updateMode2() {
  unsigned long now = millis();
  
  // Update rotation
  if (now - lastMode2Update >= MODE2_ROTATE_SPEED_MS) {
    lastMode2Update = now;
    mode2Offset++;
  }
  
  // Update color fade
  if (now - lastMode2FadeUpdate >= (MODE2_FADE_CYCLE_MS / 256)) {
    lastMode2FadeUpdate = now;
    mode2FadeProgress++;
  }
  
  const uint8_t BLOCK_LENGTH = 6;  // LEDs per color block
  const uint8_t TOTAL_LEDS = NUM_LEDS_1 + NUM_LEDS_2;  // 36-LED virtual ring
  
  // Apply pattern to virtual ring (seamless across strip boundary)
  for (uint8_t virtualPos = 0; virtualPos < TOTAL_LEDS; virtualPos++) {
    // Calculate position in scrolling pattern (seamless wrap)
    uint8_t pos = (virtualPos + mode2Offset) % TOTAL_LEDS;
    
    // Determine which block this LED is in (seamless across entire ring)
    bool isBlockA = ((pos / BLOCK_LENGTH) % 2) == 0;
    
    // Apply color morphing
    CRGB color;
    if (isBlockA) {
      // Block A: Purple → Yellow
      color = blendColours(PURPLE, YELLOW, mode2FadeProgress);
    } else {
      // Block B: Yellow → Purple
      color = blendColours(YELLOW, PURPLE, mode2FadeProgress);
    }
    
    // Map virtual position to physical LED
    if (virtualPos < NUM_LEDS_1) {
      leds1[virtualPos] = color;
    } else {
      leds2[virtualPos - NUM_LEDS_1] = color;
    }
  }
  
  FastLED.show();
}

// ============================================================================
// MODE 3: Rainbow Scroll
// ============================================================================
// Scrolling rainbow with color shifting - full spectrum around both strips

void updateMode3() {
  unsigned long now = millis();
  if (now - lastMode3Update < MODE3_RAINBOW_SPEED_MS) return;
  lastMode3Update = now;
  
  mode3RainbowHue++;  // Shift colors each frame
  
  // Treat both strips as a virtual 36-LED ring
  const uint8_t TOTAL_LEDS = NUM_LEDS_1 + NUM_LEDS_2;
  
  for (uint8_t virtualPos = 0; virtualPos < TOTAL_LEDS; virtualPos++) {
    // Calculate hue for this position (scrolling rainbow)
    uint8_t hue = mode3RainbowHue + (virtualPos * MODE3_HUE_SPACING);
    
    // Create color using HSV (FastLED built-in)
    // Hue: 0-255 (full spectrum), Saturation: 255 (full), Value: 200 (78% brightness)
    CRGB color = CHSV(hue, 255, 200);
    
    // Map virtual position to physical LED
    if (virtualPos < NUM_LEDS_1) {
      leds1[virtualPos] = color;
    } else {
      leds2[virtualPos - NUM_LEDS_1] = color;
    }
  }
  
  FastLED.show();
}

// ============================================================================
// MODE 4: Police Lights
// ============================================================================
// Red/blue emergency pattern adapted for 18 LEDs per strip (7+4+7 sections)

void updateMode4() {
  unsigned long now = millis();
  if (now - lastMode4Update >= MODE4_INTERVAL_MS) {
    lastMode4Update = now;
    mode4Stage = (mode4Stage + 1) & 0x07;  // Cycle 0-7
  }
  
  // Color definitions
  const CRGB RED_HIGH   = CRGB(178, 0, 0);
  const CRGB RED_LOW    = CRGB(77, 0, 0);
  const CRGB BLUE_HIGH  = CRGB(0, 0, 178);
  const CRGB BLUE_LOW   = CRGB(0, 0, 77);
  const CRGB WHITE_HIGH = CRGB(178, 178, 178);
  const CRGB WHITE_LOW  = CRGB(77, 77, 77);
  
  // Section sizes for 18 LEDs: 7 + 4 + 7 = 18
  const uint8_t secSeven = 7;
  const uint8_t secFour  = 4;
  
  for (uint8_t strip = 0; strip < 2; strip++) {
    CRGB *leds = (strip == 0) ? leds1 : leds2;
    bool stripReverse = (strip == 1);  // Strip 2 has inverted colors
    
    uint8_t flags = pgm_read_byte(&policeFlags[mode4Stage]);
    bool firstHigh  = (flags & 0x01);
    bool secondHigh = (flags & 0x02);
    bool invertColoursStage = (flags & 0x04);
    
    // First section (7 LEDs - red/blue)
    for (uint8_t i = 0; i < secSeven; i++) {
      bool isFirstHalf = (i < (secSeven / 2));
      bool useRed = invertColoursStage ? !isFirstHalf : isFirstHalf;
      bool finalUseRed = stripReverse ? !useRed : useRed;
      bool highIntensity = isFirstHalf ? firstHigh : secondHigh;
      
      if (finalUseRed) {
        leds[i] = highIntensity ? RED_HIGH : RED_LOW;
      } else {
        leds[i] = highIntensity ? BLUE_HIGH : BLUE_LOW;
      }
    }
    
    // Middle section (4 LEDs - white strobe)
    bool whiteHigh = (mode4Stage & 0x01);
    for (uint8_t i = 0; i < secFour; i++) {
      uint8_t idx = secSeven + i;
      leds[idx] = whiteHigh ? WHITE_HIGH : WHITE_LOW;
    }
    
    // Last section (7 LEDs - blue/red)
    for (uint8_t i = 0; i < secSeven; i++) {
      uint8_t idx = secSeven + secFour + i;
      bool isFirstHalf = (i < (secSeven / 2));
      bool useRed = invertColoursStage ? !isFirstHalf : isFirstHalf;
      bool finalUseRed = stripReverse ? !useRed : useRed;
      bool highIntensity = isFirstHalf ? firstHigh : secondHigh;
      
      if (finalUseRed) {
        leds[idx] = highIntensity ? RED_HIGH : RED_LOW;
      } else {
        leds[idx] = highIntensity ? BLUE_HIGH : BLUE_LOW;
      }
    }
  }
  
  FastLED.show();
}
