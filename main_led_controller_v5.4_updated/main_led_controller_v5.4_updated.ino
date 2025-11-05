/*
 * ATTiny85 dual‑strip LED controller for a racing drone (20 LEDs per strip)
 *
 * This firmware reads a servo‑style PWM input on SERVO_PIN and selects
 * one of four lighting programmes based on the pulse width.  The radio
 * channel is conditioned by an external switch so that a zero pulse
 * turns the LEDs completely off.  Otherwise the 1000–2000 µs range is
 * divided into four equal 250 µs quarters:
 *
 *   • 0 µs: Power off – all LEDs off.
 *   • 1000–1249 µs (Q0): White – both strips steady white at 60 % brightness.
 *   • 1250–1499 µs (Q1): Green gradient – a vertical green gradient scrolls
 *     from bottom to top on each strip at high speed.
 *   • 1500–1749 µs (Q2): Yellow/Green fade – each strip is split into two
 *     10‑LED groups.  One group fades from yellow to green while the
 *     other fades from green to yellow, then they swap.
 *   • 1750–2000 µs (Q3): Police lights – the strip is divided into
 *     sections of 8, 4 and 8 LEDs.  The 8‑LED sections alternate
 *     between red and blue with high/low intensity flashes over an
 *     eight‑stage cycle.  The 4‑LED middle section flashes between
 *     low and high white.  On the second strip the red/blue colours
 *     are swapped.
 *
 * A sanity‑check mode is available: pull SANITY_PIN low for 50 ms to
 * cycle through the four non‑off programmes (white, green, yellow/green,
 * police), spending 1 s on each.  A brief white blink indicates
 * entry into sanity mode.  Release the pin to return to the current
 * programme.
 *
 * Memory usage is kept below 512 bytes by storing static tables in
 * PROGMEM and using 8‑bit variables where possible.  Each strip has
 * 20 LEDs, so the two CRGB arrays consume 120 bytes.  The remaining
 * state variables fit comfortably within the ATtiny85’s SRAM.
 */

#include <FastLED.h>
#include <avr/pgmspace.h>

// LED strip configuration
#define LED_PIN_1   0     // PB0 (physical pin 5)
#define LED_PIN_2   4     // PB4 (physical pin 3)
#define NUM_LEDS_1  20
#define NUM_LEDS_2  20
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// Servo input and sanity mode pins
#define SERVO_PIN   2     // PB2 (physical pin 7)
#define SANITY_PIN  3     // PB3 (physical pin 2)

// Servo pulse range and quarter width (1000–2000 µs divided into 4)
const int SERVO_MIN_PULSE = 1000;
const int SERVO_MAX_PULSE = 2000;
const int QUARTER_WIDTH   = (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / 4; // 250 µs

// LED arrays (consume 120 bytes)
CRGB leds1[NUM_LEDS_1];
CRGB leds2[NUM_LEDS_2];

// Lighting programmes (5 states: power off plus four programmes)
enum Mode : uint8_t {
  MODE_POWER_OFF    = 0,
  MODE_WHITE        = 1,
  MODE_GREEN        = 2,
  MODE_YELLOW_GREEN = 3,
  MODE_POLICE       = 4
};

// Current mode and last detected quarter (for debouncing)
static Mode currentMode = MODE_POWER_OFF;
static int8_t lastQuarter = -1;

// ----- State for Green gradient (Mode 2) -----
static uint8_t greenOffset = 0;                 // position offset of gradient
static unsigned long lastGreenUpdate = 0;       // timestamp of last update
const uint8_t GREEN_UPDATE_INTERVAL_MS = 10;    // update every 10 ms

// ----- State for Yellow/Green fade (Mode 3) -----
static bool groupFading = false;                // whether a fade is in progress
static bool groupsSwapped = false;              // which half is yellow/green
static unsigned long groupFadeStartTime = 0;    // when current fade began
const unsigned long GROUP_FADE_DURATION_MS = 250; // fade duration (ms)

// ----- State for Police lights (Mode 4) -----
static uint8_t policeStage = 0;                 // current stage index (0..7)
static unsigned long lastPoliceUpdate = 0;      // timestamp of last stage update
const uint8_t POLICE_INTERVAL_MS = 22;          // stage duration (~22 ms)

// Stage flags for the police pattern stored in flash.  Each entry encodes
// three bits: bit0 = first half high intensity, bit1 = second half high,
// bit2 = invert colours (swap red/blue).  See updatePolice() for details.
const uint8_t stageFlags[8] PROGMEM = {
  0x01, // stage 0: firstHigh=1, secondHigh=0, invertColours=0
  0x02, // stage 1: firstHigh=0, secondHigh=1, invertColours=0
  0x00, // stage 2: firstHigh=0, secondHigh=0, invertColours=0
  0x03, // stage 3: firstHigh=1, secondHigh=1, invertColours=0
  0x04, // stage 4: firstHigh=0, secondHigh=0, invertColours=1
  0x05, // stage 5: firstHigh=1, secondHigh=0, invertColours=1
  0x06, // stage 6: firstHigh=0, secondHigh=1, invertColours=1
  0x07  // stage 7: firstHigh=1, secondHigh=1, invertColours=1
};

// Forward declarations
void setMode(Mode newMode);
void updateAnimations();
void updatePowerOff();
void updateWhite();
void updateGreen();
void updateYellowGreen();
void updatePolice();

// Helper to fill both strips and update
inline void fillBoth(const CRGB &colour) {
  fill_solid(leds1, NUM_LEDS_1, colour);
  fill_solid(leds2, NUM_LEDS_2, colour);
  FastLED.show();
}

void setup() {
  pinMode(SERVO_PIN, INPUT);
  pinMode(SANITY_PIN, INPUT_PULLUP);
  FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
  // Default brightness (50 %)
  FastLED.setBrightness(128);
  // Start with all LEDs off
  fillBoth(CRGB::Black);
}

void loop() {
  // --- Sanity mode handling ---
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
          // Enter sanity mode after 50 ms low
          inSanity = true;
          savedMode = currentMode;
          // Blink white to indicate entry (use full brightness)
          uint8_t prevBrightness = FastLED.getBrightness();
          FastLED.setBrightness(255);
          fillBoth(CRGB::White);
          delay(80);
          fillBoth(CRGB::Black);
          FastLED.setBrightness(prevBrightness);
          // Start with first programme (skip power off)
          sanityIndex = MODE_WHITE;
          sanityLastSwitch = now;
          setMode(static_cast<Mode>(sanityIndex));
        }
      } else {
        sanityLowStart = 0;
      }
    } else {
      // Already in sanity mode; wait until pin is high for >50 ms to exit
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
      // Cycle through programmes every 1 s
      if (now - sanityLastSwitch >= 1000UL) {
        sanityLastSwitch = now;
        // Cycle through four non‑off programmes: white, green, yellow/green, police
        uint8_t next = sanityIndex + 1;
        if (next > MODE_POLICE) next = MODE_WHITE;
        sanityIndex = next;
        setMode(static_cast<Mode>(sanityIndex));
      }
      updateAnimations();
      return;
    }
  }
  // --- Normal operation ---
  // Read servo pulse; treat any pulse below 1000 µs (or no pulse) as an off‑state.
  // When the user’s switch is off the radio outputs ~988 µs, which should
  // disable the LEDs entirely.  Values >=1000 µs select the four programmes.
  unsigned long pulseWidth = pulseIn(SERVO_PIN, HIGH, 50000UL);
  if (pulseWidth == 0 || pulseWidth < 1000) {
    if (currentMode != MODE_POWER_OFF) {
      setMode(MODE_POWER_OFF);
    }
  } else {
    // Clamp to 1000–2000
    if (pulseWidth < SERVO_MIN_PULSE) pulseWidth = SERVO_MIN_PULSE;
    if (pulseWidth > SERVO_MAX_PULSE) pulseWidth = SERVO_MAX_PULSE;
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
        // Quarter indices map to modes 1..4
        Mode m;
        switch (quarter) {
          case 0: m = MODE_WHITE; break;
          case 1: m = MODE_GREEN; break;
          case 2: m = MODE_YELLOW_GREEN; break;
          default: m = MODE_POLICE; break;
        }
        setMode(m);
      }
    }
  }
  updateAnimations();
}

// Set a new mode and reset animation state
void setMode(Mode newMode) {
  if (newMode == currentMode) return;
  currentMode = newMode;
  // Reset per‑mode state variables
  greenOffset = 0;
  lastGreenUpdate = 0;
  groupFading = false;
  groupsSwapped = false;
  groupFadeStartTime = 0;
  policeStage = 0;
  lastPoliceUpdate = 0;
  // Clear LED strips
  fillBoth(CRGB::Black);
  // Configure brightness and initial colours for each mode
  switch (currentMode) {
    case MODE_POWER_OFF:
      FastLED.setBrightness(0);
      break;
    case MODE_WHITE:
      FastLED.setBrightness((uint8_t)(0.6 * 255)); // ≈60 %
      fillBoth(CRGB::White);
      break;
    case MODE_GREEN:
      FastLED.setBrightness(128); // 50 %
      break;
    case MODE_YELLOW_GREEN:
      FastLED.setBrightness(128); // 50 %
      break;
    case MODE_POLICE:
      FastLED.setBrightness(255); // full brightness; intensities scaled manually
      break;
  }
}

// Dispatch to the current animation
void updateAnimations() {
  switch (currentMode) {
    case MODE_POWER_OFF:
      updatePowerOff();
      break;
    case MODE_WHITE:
      updateWhite();
      break;
    case MODE_GREEN:
      updateGreen();
      break;
    case MODE_YELLOW_GREEN:
      updateYellowGreen();
      break;
    case MODE_POLICE:
      updatePolice();
      break;
  }
}

// Mode 0: Power off – nothing to do per frame
void updatePowerOff() {
  // Nothing; strips are cleared when mode set
}

// Mode 1: White – nothing to animate per frame
void updateWhite() {
  // LEDs remain white as set in setMode()
}

// Mode 2: Scrolling green gradient
void updateGreen() {
  unsigned long now = millis();
  if (now - lastGreenUpdate < GREEN_UPDATE_INTERVAL_MS) return;
  lastGreenUpdate = now;
  // Advance offset
  greenOffset++;
  // For each strip compute a seamless vertical gradient that scrolls upward.
  // To avoid a visible jump when the gradient wraps around, we generate a
  // triangular wave over twice the strip length: brightness decreases from
  // bottom to top and then increases back, producing a continuous loop.
  const uint8_t cycleLen = NUM_LEDS_1 * 2;
  for (uint8_t j = 0; j < NUM_LEDS_1; j++) {
    uint8_t t = (j + greenOffset) % cycleLen;
    uint8_t posVal = (t < NUM_LEDS_1) ? t : (cycleLen - 1 - t);
    uint8_t val = 255 - ((uint16_t)posVal * 255U) / (NUM_LEDS_1 - 1);
    leds1[j] = CRGB(0, val, 0);
  }
  for (uint8_t j = 0; j < NUM_LEDS_2; j++) {
    uint8_t t = (j + greenOffset) % cycleLen;
    uint8_t posVal = (t < NUM_LEDS_2) ? t : (cycleLen - 1 - t);
    uint8_t val = 255 - ((uint16_t)posVal * 255U) / (NUM_LEDS_2 - 1);
    leds2[j] = CRGB(0, val, 0);
  }
  FastLED.show();
}

// Helper: linear blend of two colours without floating point.  weight in 0..255.
static inline CRGB blendColours(const CRGB &from, const CRGB &to, uint8_t weight) {
  uint16_t r = (uint16_t)from.r * (255 - weight) + (uint16_t)to.r * weight;
  uint16_t g = (uint16_t)from.g * (255 - weight) + (uint16_t)to.g * weight;
  uint16_t b = (uint16_t)from.b * (255 - weight) + (uint16_t)to.b * weight;
  return CRGB((uint8_t)(r >> 8), (uint8_t)(g >> 8), (uint8_t)(b >> 8));
}

// Mode 3: Yellow/Green fade between two 10‑LED groups
void updateYellowGreen() {
  unsigned long now = millis();
  if (!groupFading) {
    groupFading = true;
    groupFadeStartTime = now;
  }
  unsigned long dt = now - groupFadeStartTime;
  uint8_t progress;
  if (dt >= GROUP_FADE_DURATION_MS) {
    progress = 255;
  } else {
    progress = (uint8_t)((dt * 255UL) / GROUP_FADE_DURATION_MS);
  }
  CRGB fromA = groupsSwapped ? CRGB(0, 255, 0) : CRGB(255, 255, 0);
  CRGB toA   = groupsSwapped ? CRGB(255, 255, 0) : CRGB(0, 255, 0);
  CRGB fromB = groupsSwapped ? CRGB(255, 255, 0) : CRGB(0, 255, 0);
  CRGB toB   = groupsSwapped ? CRGB(0, 255, 0) : CRGB(255, 255, 0);
  CRGB cA = blendColours(fromA, toA, progress);
  CRGB cB = blendColours(fromB, toB, progress);
  for (uint8_t j = 0; j < 10; j++) {
    leds1[j] = cA;
    leds2[j] = cA;
  }
  for (uint8_t j = 10; j < 20; j++) {
    leds1[j] = cB;
    leds2[j] = cB;
  }
  FastLED.show();
  if (progress == 255) {
    groupFading = false;
    groupsSwapped = !groupsSwapped;
  }
}

// Mode 4: Police lights adapted for 20‑LED strips
void updatePolice() {
  unsigned long now = millis();
  if (now - lastPoliceUpdate >= POLICE_INTERVAL_MS) {
    lastPoliceUpdate = now;
    policeStage = (policeStage + 1) & 0x07;
  }
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
    uint8_t flags = pgm_read_byte(&stageFlags[policeStage]);
    bool firstHigh  = (flags & 0x01);
    bool secondHigh = (flags & 0x02);
    bool invertColoursStage = (flags & 0x04);
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
    bool whiteHigh = (policeStage & 0x01);
    for (uint8_t i = 0; i < secFour; i++) {
      uint8_t idx = secEight + i;
      leds[idx] = whiteHigh ? WHITE_HIGH : WHITE_LOW;
    }
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