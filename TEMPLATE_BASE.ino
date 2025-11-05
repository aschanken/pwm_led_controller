/*
 * ============================================================================
 * DRONE LED CONTROLLER TEMPLATE - ATTiny85-20PU
 * ============================================================================
 * 
 * This is a configurable template for creating bespoke racing drone LED
 * lighting controllers. Customize the sections marked with "CUSTOMIZE HERE"
 * to create customer-specific firmware.
 * 
 * HARDWARE:
 *   - MCU: ATTiny85-20PU
 *   - LEDs: Dual WS2812B strips
 *   - Input: PWM signal (0-2000µs) for mode selection
 *   - Memory: <512 bytes SRAM target
 * 
 * PWM MODE MAPPING:
 *   - <1000µs: Power off (all LEDs off)
 *   - 1000-1249µs: Mode 1 (customizable)
 *   - 1250-1499µs: Mode 2 (customizable)
 *   - 1500-1749µs: Mode 3 (customizable)
 *   - 1750-2000µs: Mode 4 (customizable)
 * 
 * SANITY TEST MODE:
 *   Pull SANITY_PIN low for 50ms to cycle through all modes (1s each)
 * 
 * ============================================================================
 */

#include <FastLED.h>
#include <avr/pgmspace.h>

// ============================================================================
// HARDWARE CONFIGURATION - CUSTOMIZE HERE FOR EACH CUSTOMER
// ============================================================================

// LED strip configuration
#define LED_PIN_1   0     // PB0 (physical pin 5)
#define LED_PIN_2   4     // PB4 (physical pin 3)
#define NUM_LEDS_1  20    // CUSTOMIZE: Number of LEDs on strip 1
#define NUM_LEDS_2  20    // CUSTOMIZE: Number of LEDs on strip 2
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// Control pins (usually fixed)
#define SERVO_PIN   2     // PB2 (physical pin 7) - PWM input
#define SANITY_PIN  3     // PB3 (physical pin 2) - Test mode trigger

// PWM signal configuration (standard 1000-2000µs)
const int SERVO_MIN_PULSE = 1000;
const int SERVO_MAX_PULSE = 2000;
const int QUARTER_WIDTH   = (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / 4; // 250µs

// ============================================================================
// MODE DEFINITIONS - CUSTOMIZE HERE FOR EACH CUSTOMER
// ============================================================================

enum Mode : uint8_t {
  MODE_POWER_OFF = 0,
  MODE_1         = 1,  // CUSTOMIZE: Rename to describe pattern (e.g., MODE_SOLID_WHITE)
  MODE_2         = 2,  // CUSTOMIZE: Rename to describe pattern (e.g., MODE_PULSE_GREEN)
  MODE_3         = 3,  // CUSTOMIZE: Rename to describe pattern (e.g., MODE_FADE_YELLOW_GREEN)
  MODE_4         = 4   // CUSTOMIZE: Rename to describe pattern (e.g., MODE_POLICE_LIGHTS)
};

// ============================================================================
// GLOBAL STATE
// ============================================================================

// LED arrays (memory usage: NUM_LEDS * 3 bytes each)
CRGB leds1[NUM_LEDS_1];
CRGB leds2[NUM_LEDS_2];

// Current mode tracking
static Mode currentMode = MODE_POWER_OFF;
static int8_t lastQuarter = -1;

// ============================================================================
// MODE-SPECIFIC STATE VARIABLES - CUSTOMIZE HERE
// ============================================================================

// Mode 1 state (example: static color - no state needed)
// (Add variables if needed for your Mode 1 pattern)

// Mode 2 state (example: scrolling animation)
static uint8_t mode2Offset = 0;
static unsigned long lastMode2Update = 0;
const uint8_t MODE2_UPDATE_INTERVAL_MS = 10;  // CUSTOMIZE: Animation speed

// Mode 3 state (example: fading animation)
static bool mode3Fading = false;
static bool mode3Swapped = false;
static unsigned long mode3FadeStartTime = 0;
const unsigned long MODE3_FADE_DURATION_MS = 250;  // CUSTOMIZE: Fade duration

// Mode 4 state (example: multi-stage pattern)
static uint8_t mode4Stage = 0;
static unsigned long lastMode4Update = 0;
const uint8_t MODE4_INTERVAL_MS = 22;  // CUSTOMIZE: Stage duration

// ============================================================================
// MODE-SPECIFIC PROGMEM DATA - CUSTOMIZE HERE
// ============================================================================

// Example: Police light pattern stages (for Mode 4)
// CUSTOMIZE: Replace with your own lookup tables as needed
const uint8_t mode4StageFlags[8] PROGMEM = {
  0x01, 0x02, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07
};

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
// HELPER FUNCTIONS (REUSABLE ACROSS ALL CUSTOMER FIRMWARE)
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
          sanityIndex = MODE_1;
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
        if (next > MODE_4) next = MODE_1;
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
          case 0: m = MODE_1; break;
          case 1: m = MODE_2; break;
          case 2: m = MODE_3; break;
          default: m = MODE_4; break;
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
  mode2Offset = 0;
  lastMode2Update = 0;
  mode3Fading = false;
  mode3Swapped = false;
  mode3FadeStartTime = 0;
  mode4Stage = 0;
  lastMode4Update = 0;
  
  // Clear LED strips
  fillBoth(CRGB::Black);
  
  // CUSTOMIZE HERE: Configure brightness and initial state for each mode
  switch (currentMode) {
    case MODE_POWER_OFF:
      FastLED.setBrightness(0);
      break;
      
    case MODE_1:  // Example: Solid white at 70%
      FastLED.setBrightness((uint8_t)(0.7 * 255));  // CUSTOMIZE: Brightness
      fillBoth(CRGB::White);  // CUSTOMIZE: Initial color
      break;
      
    case MODE_2:  // Example: Animated pattern
      FastLED.setBrightness(128);  // CUSTOMIZE: Brightness (50%)
      break;
      
    case MODE_3:  // Example: Fading pattern
      FastLED.setBrightness(128);  // CUSTOMIZE: Brightness (50%)
      break;
      
    case MODE_4:  // Example: Complex pattern
      FastLED.setBrightness(255);  // CUSTOMIZE: Brightness (100%)
      break;
  }
}

// Dispatch to current mode update function
void updateAnimations() {
  switch (currentMode) {
    case MODE_POWER_OFF:
      updatePowerOff();
      break;
    case MODE_1:
      updateMode1();
      break;
    case MODE_2:
      updateMode2();
      break;
    case MODE_3:
      updateMode3();
      break;
    case MODE_4:
      updateMode4();
      break;
  }
}

// ============================================================================
// MODE UPDATE FUNCTIONS - CUSTOMIZE THESE FOR EACH CUSTOMER
// ============================================================================

// Power Off - No animation needed
void updatePowerOff() {
  // LEDs remain off as set in setMode()
}

// ============================================================================
// MODE 1 - CUSTOMIZE HERE
// ============================================================================
// Example: Solid white color (no animation)
// Replace with customer-specific pattern

void updateMode1() {
  // Example: Static color - no per-frame updates needed
  // LEDs already set in setMode()
  
  // CUSTOMIZE: Add animation code here if needed
  // Example patterns you could implement:
  //   - Breathing effect (slow brightness pulse)
  //   - Sparkle/twinkle
  //   - Static color with different brightness
}

// ============================================================================
// MODE 2 - CUSTOMIZE HERE
// ============================================================================
// Example: Scrolling green gradient
// Replace with customer-specific pattern

void updateMode2() {
  unsigned long now = millis();
  if (now - lastMode2Update < MODE2_UPDATE_INTERVAL_MS) return;
  lastMode2Update = now;
  
  // CUSTOMIZE: Replace this example with customer pattern
  // Example: Scrolling gradient
  mode2Offset++;
  const uint8_t cycleLen = NUM_LEDS_1 * 2;
  
  for (uint8_t j = 0; j < NUM_LEDS_1; j++) {
    uint8_t t = (j + mode2Offset) % cycleLen;
    uint8_t posVal = (t < NUM_LEDS_1) ? t : (cycleLen - 1 - t);
    uint8_t val = 255 - ((uint16_t)posVal * 255U) / (NUM_LEDS_1 - 1);
    leds1[j] = CRGB(0, val, 0);  // Green gradient
  }
  
  for (uint8_t j = 0; j < NUM_LEDS_2; j++) {
    uint8_t t = (j + mode2Offset) % cycleLen;
    uint8_t posVal = (t < NUM_LEDS_2) ? t : (cycleLen - 1 - t);
    uint8_t val = 255 - ((uint16_t)posVal * 255U) / (NUM_LEDS_2 - 1);
    leds2[j] = CRGB(0, val, 0);  // Green gradient
  }
  
  FastLED.show();
}

// ============================================================================
// MODE 3 - CUSTOMIZE HERE
// ============================================================================
// Example: Yellow/Green alternating fade
// Replace with customer-specific pattern

void updateMode3() {
  unsigned long now = millis();
  
  if (!mode3Fading) {
    mode3Fading = true;
    mode3FadeStartTime = now;
  }
  
  unsigned long dt = now - mode3FadeStartTime;
  uint8_t progress;
  if (dt >= MODE3_FADE_DURATION_MS) {
    progress = 255;
  } else {
    progress = (uint8_t)((dt * 255UL) / MODE3_FADE_DURATION_MS);
  }
  
  // CUSTOMIZE: Replace colors and fade logic
  CRGB fromA = mode3Swapped ? CRGB(0, 255, 0) : CRGB(255, 255, 0);
  CRGB toA   = mode3Swapped ? CRGB(255, 255, 0) : CRGB(0, 255, 0);
  CRGB fromB = mode3Swapped ? CRGB(255, 255, 0) : CRGB(0, 255, 0);
  CRGB toB   = mode3Swapped ? CRGB(0, 255, 0) : CRGB(255, 255, 0);
  
  CRGB cA = blendColours(fromA, toA, progress);
  CRGB cB = blendColours(fromB, toB, progress);
  
  // Split strips in half
  for (uint8_t j = 0; j < NUM_LEDS_1 / 2; j++) {
    leds1[j] = cA;
    leds2[j] = cA;
  }
  for (uint8_t j = NUM_LEDS_1 / 2; j < NUM_LEDS_1; j++) {
    leds1[j] = cB;
    leds2[j] = cB;
  }
  
  FastLED.show();
  
  if (progress == 255) {
    mode3Fading = false;
    mode3Swapped = !mode3Swapped;
  }
}

// ============================================================================
// MODE 4 - CUSTOMIZE HERE
// ============================================================================
// Example: Police lightbar effect
// Replace with customer-specific pattern

void updateMode4() {
  unsigned long now = millis();
  if (now - lastMode4Update >= MODE4_INTERVAL_MS) {
    lastMode4Update = now;
    mode4Stage = (mode4Stage + 1) & 0x07;  // Cycle 0-7
  }
  
  // CUSTOMIZE: Replace with customer pattern
  // Example: Police lights with red/blue alternating sections
  const CRGB RED_HIGH   = CRGB(178, 0, 0);
  const CRGB RED_LOW    = CRGB(77, 0, 0);
  const CRGB BLUE_HIGH  = CRGB(0, 0, 178);
  const CRGB BLUE_LOW   = CRGB(0, 0, 77);
  const CRGB WHITE_HIGH = CRGB(178, 178, 178);
  const CRGB WHITE_LOW  = CRGB(77, 77, 77);
  
  const uint8_t secEight = 8;
  const uint8_t secFour  = 4;
  
  for (uint8_t strip = 0; strip < 2; strip++) {
    CRGB *leds = (strip == 0) ? leds1 : leds2;
    bool stripReverse = (strip == 1);
    
    uint8_t flags = pgm_read_byte(&mode4StageFlags[mode4Stage]);
    bool firstHigh  = (flags & 0x01);
    bool secondHigh = (flags & 0x02);
    bool invertColoursStage = (flags & 0x04);
    
    // First section (8 LEDs)
    for (uint8_t i = 0; i < secEight; i++) {
      bool isFirstHalf = (i < (secEight / 2));
      bool useRed = invertColoursStage ? !isFirstHalf : isFirstHalf;
      bool finalUseRed = stripReverse ? !useRed : useRed;
      bool highIntensity = isFirstHalf ? firstHigh : secondHigh;
      
      if (finalUseRed) {
        leds[i] = highIntensity ? RED_HIGH : RED_LOW;
      } else {
        leds[i] = highIntensity ? BLUE_HIGH : BLUE_LOW;
      }
    }
    
    // Middle section (4 LEDs white)
    bool whiteHigh = (mode4Stage & 0x01);
    for (uint8_t i = 0; i < secFour; i++) {
      uint8_t idx = secEight + i;
      leds[idx] = whiteHigh ? WHITE_HIGH : WHITE_LOW;
    }
    
    // Last section (8 LEDs)
    for (uint8_t i = 0; i < secEight; i++) {
      uint8_t idx = secEight + secFour + i;
      bool isFirstHalf = (i < (secEight / 2));
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
