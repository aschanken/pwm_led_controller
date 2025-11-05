/*
 * ============================================================================
 * CUSTOMER CONFIGURATION EXAMPLE
 * ============================================================================
 * 
 * This file demonstrates how to configure the template for a specific
 * customer. Copy this approach when creating new customer firmware.
 * 
 * Customer: Example Racing Team
 * Drone Model: 75mm Whooper
 * LED Setup: 2x 20 LED WS2812B strips
 * 
 * ============================================================================
 */

// ============================================================================
// CUSTOMER SPECIFICATIONS
// ============================================================================

/*
 * MODE REQUIREMENTS:
 * 
 * Mode 1 (1000-1249µs): Solid White - 70% brightness for general flying
 * Mode 2 (1250-1499µs): Pulsating Green - Scrolling gradient effect
 * Mode 3 (1500-1749µs): Yellow/Green Blocks - Alternating fade between colors
 * Mode 4 (1750-2000µs): Police Lights - Red/blue alternating lightbar effect
 * 
 */

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

#define CUSTOMER_NUM_LEDS_1  20    // Strip 1: 20 LEDs
#define CUSTOMER_NUM_LEDS_2  20    // Strip 2: 20 LEDs
#define CUSTOMER_LED_TYPE    WS2812B

// ============================================================================
// MODE 1 CONFIGURATION - Solid White
// ============================================================================

#define MODE1_BRIGHTNESS     178   // 70% brightness (0.7 * 255)
#define MODE1_COLOR_R        255
#define MODE1_COLOR_G        255
#define MODE1_COLOR_B        255

// ============================================================================
// MODE 2 CONFIGURATION - Pulsating/Scrolling Green
// ============================================================================

#define MODE2_BRIGHTNESS     128   // 50% brightness
#define MODE2_UPDATE_MS      10    // Update every 10ms for smooth scrolling
#define MODE2_COLOR          CRGB::Green

// Animation parameters
#define MODE2_GRADIENT_CYCLE 2     // Multiplier for gradient length (2x strip length)

// ============================================================================
// MODE 3 CONFIGURATION - Yellow/Green Alternating Blocks
// ============================================================================

#define MODE3_BRIGHTNESS     128   // 50% brightness
#define MODE3_FADE_DURATION  250   // 250ms fade duration
#define MODE3_COLOR_A        CRGB(255, 255, 0)  // Yellow
#define MODE3_COLOR_B        CRGB(0, 255, 0)    // Green
#define MODE3_SPLIT_POINT    10    // Split at LED 10 (half of 20)

// ============================================================================
// MODE 4 CONFIGURATION - Police Lightbar
// ============================================================================

#define MODE4_BRIGHTNESS     255   // 100% brightness (manual intensity control)
#define MODE4_STAGE_MS       22    // 22ms per stage (fast strobe)

// Color definitions for police effect
#define MODE4_RED_HIGH       CRGB(178, 0, 0)
#define MODE4_RED_LOW        CRGB(77, 0, 0)
#define MODE4_BLUE_HIGH      CRGB(0, 0, 178)
#define MODE4_BLUE_LOW       CRGB(0, 0, 77)
#define MODE4_WHITE_HIGH     CRGB(178, 178, 178)
#define MODE4_WHITE_LOW      CRGB(77, 77, 77)

// Section sizes (must add up to NUM_LEDS)
#define MODE4_SECTION_1      8     // First section: 8 LEDs
#define MODE4_SECTION_2      4     // Middle section: 4 LEDs
#define MODE4_SECTION_3      8     // Last section: 8 LEDs

// ============================================================================
// MEMORY BUDGET ESTIMATE
// ============================================================================

/*
 * SRAM Usage Breakdown:
 * 
 * LED Arrays:
 *   - leds1: 20 LEDs * 3 bytes = 60 bytes
 *   - leds2: 20 LEDs * 3 bytes = 60 bytes
 *   - Total: 120 bytes
 * 
 * State Variables:
 *   - Mode tracking: ~5 bytes
 *   - Mode 2 state: ~6 bytes
 *   - Mode 3 state: ~8 bytes
 *   - Mode 4 state: ~6 bytes
 *   - Sanity mode: ~15 bytes
 *   - Total: ~40 bytes
 * 
 * Stack & FastLED overhead: ~150 bytes
 * 
 * TOTAL ESTIMATED: ~310 bytes (well under 512 byte target)
 * 
 */

// ============================================================================
// CUSTOMER NOTES
// ============================================================================

/*
 * Special Requirements:
 * - Customer wants quick mode transitions (3-sample debounce is fine)
 * - Brightness levels tested and approved by customer
 * - Police mode must alternate colors between strips
 * - Green gradient should scroll "upward" (increasing LED index)
 * 
 * Delivery Date: [DATE]
 * Testing Notes: [TESTED WITH BETAFLIGHT AUX CHANNEL]
 * 
 */
