# AGENTS.md - Drone LED Controller Firmware Template

## Project Overview
Bespoke ATtiny85-20PU firmware template for racing drone WS2812B LED lighting systems.
Configurable for various customer requirements with 4 customizable lighting modes.

## Build/Test Commands
- **Compile**: `arduino-cli compile --fqbn attiny:avr:ATtiny85 {customer_firmware}.ino`
- **Upload**: `arduino-cli upload -p /dev/ttyUSB0 --fqbn attiny:avr:ATtiny85`
- **Test**: PWM input testing (0-2000µs range) + sanity mode via PB3 pin

## Template Architecture & Customization Points

### Mode Configuration (Primary Customization)
- **MODE_POWER_OFF**: <1000µs (fixed off state)
- **MODE_1 (1000-1249µs)**: Customizable static/simple pattern
- **MODE_2 (1250-1499µs)**: Customizable animated pattern
- **MODE_3 (1500-1749µs)**: Customizable complex animation
- **MODE_4 (1750-2000µs)**: Customizable special effect

### Hardware Parameters (Customer-Specific)
- `NUM_LEDS_1/2`: Strip lengths (default: 20 each)
- `LED_TYPE`: WS2812B variants supported
- Pin assignments (PB0, PB4 for LEDs; PB2 for PWM)

### Code Style for Template Modularity
- Each mode in separate `updateMode*()` function for easy swapping
- Animation parameters as `const` variables for quick tuning
- PROGMEM tables for complex patterns (police, fire, rainbow, etc.)
- Memory budget: <512 bytes SRAM, maintain 8-bit arithmetic

### Customer Deployment Checklist
1. Define customer's 4 mode requirements
2. Adjust LED counts and brightness levels
3. Implement/modify mode update functions
4. Test PWM ranges and transitions
5. Validate memory usage (<512 bytes)
