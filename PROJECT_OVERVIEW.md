# Drone LED Controller - Template System

## 🎯 Project Purpose

This is a **modular firmware template system** for creating bespoke ATtiny85-20PU based LED controllers for racing drones. The template allows you to quickly produce custom firmware for customers with different lighting requirements.

---

## 📁 Project Files

### Core Template Files

1. **TEMPLATE_BASE.ino** (17KB)
   - Main firmware template with full example implementation
   - Contains all core functionality (PWM reading, mode switching, sanity testing)
   - Clearly marked customization sections
   - Includes 4 example animation modes ready for modification

2. **AGENTS.md** (1.6KB)
   - Quick reference for AI coding agents
   - Build/compile/upload commands
   - Code style guidelines
   - Memory management best practices

3. **ANIMATION_LIBRARY.md** (7.7KB)
   - Catalog of 10+ reusable LED animation patterns
   - Memory usage for each pattern
   - Copy-paste ready code snippets
   - Organized by complexity (basic, chase, color transitions, complex)

4. **CUSTOMER_CONFIG_EXAMPLE.h** (4.8KB)
   - Example customer configuration
   - Shows how to document requirements
   - Demonstrates parameter customization
   - Memory usage calculations

5. **README_DEPLOYMENT.md** (12KB)
   - Complete step-by-step deployment guide
   - Testing procedures
   - Troubleshooting common issues
   - Customer delivery checklist
   - Version control best practices

6. **PROJECT_OVERVIEW.md** (this file)
   - High-level project summary
   - Quick start guide
   - File structure explanation

### Legacy Files

- **main_led_controller_v5.4_updated.ino** - Original working firmware (reference)

---

## 🚀 Quick Start

### For First-Time Use

1. **Install prerequisites:**
   ```bash
   # Install arduino-cli
   curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
   
   # Install ATtiny support
   arduino-cli core install attiny:avr
   
   # Install FastLED library
   arduino-cli lib install FastLED
   ```

2. **Read the documentation:**
   - Start with `README_DEPLOYMENT.md` for full workflow
   - Review `ANIMATION_LIBRARY.md` for available patterns
   - Check `AGENTS.md` for coding guidelines

3. **Create your first custom firmware:**
   ```bash
   cp TEMPLATE_BASE.ino my_first_customer.ino
   # Edit the file, customize modes 1-4
   arduino-cli compile --fqbn attiny:avr:ATtiny85 my_first_customer.ino
   ```

### For AI Agents

- Read `AGENTS.md` first - contains all build commands and code style
- Reference `ANIMATION_LIBRARY.md` when implementing patterns
- Follow memory constraints: <512 bytes SRAM

---

## 🎨 Customization Workflow

```
Customer Request
      ↓
Define 4 Modes
      ↓
Copy TEMPLATE_BASE.ino → customer_name.ino
      ↓
Configure:
  - LED counts (NUM_LEDS_1, NUM_LEDS_2)
  - Mode names (enum Mode)
  - Brightness levels (setMode)
  - Animation patterns (updateMode1-4)
      ↓
Compile & Test
      ↓
Deploy to Customer
```

---

## 🔧 Hardware Specifications

### Target MCU
- **Microcontroller**: ATtiny85-20PU
- **Flash**: 8KB (program storage)
- **SRAM**: 512 bytes (working memory - **critical constraint**)
- **Clock**: 8MHz internal

### LED Support
- **Type**: WS2812B (NeoPixel compatible)
- **Strips**: 2 independent strips
- **Max LEDs**: ~30 per strip (memory limited)
- **Typical**: 12-24 LEDs per strip

### Control Interface
- **PWM Input**: Servo-style signal (1000-2000µs)
- **Mode Selection**: 4 modes via PWM ranges (250µs quarters)
- **Power Off**: <1000µs triggers all LEDs off
- **Test Mode**: Sanity pin for cycling through modes

### Pin Assignments
```
PB0 (pin 5) → LED Strip 1 Data
PB2 (pin 7) → PWM Input (from radio receiver)
PB3 (pin 2) → Sanity Test Pin (pull low to test)
PB4 (pin 3) → LED Strip 2 Data
```

---

## 💡 Example Customer Scenarios

### Scenario 1: Racing Pilot
**Requirements**: High visibility, minimal distraction
- Mode 1: Solid white 100% (race mode)
- Mode 2: Solid white 50% (practice mode)
- Mode 3: Solid red 80% (battery warning indicator)
- Mode 4: Off (battery saving)

### Scenario 2: Freestyle Pilot
**Requirements**: Style and visibility for video
- Mode 1: Rainbow cycle (video mode)
- Mode 2: Team colors fade (orange/black)
- Mode 3: Chase effect (blue)
- Mode 4: Police lights (attention grabbing)

### Scenario 3: Racing Team
**Requirements**: Team identification
- Mode 1: Team color solid (orange)
- Mode 2: Alternating team colors
- Mode 3: White (high visibility)
- Mode 4: Strobe white (found my drone!)

---

## 📊 Template Features

### Core Functionality (Always Included)
- ✅ PWM signal reading with debouncing
- ✅ 4 customizable mode slots
- ✅ Power-off mode (<1000µs)
- ✅ Sanity test mode (pin PB3)
- ✅ Dual strip support
- ✅ Memory-efficient design

### Customizable Elements
- 🎨 LED counts per strip
- 🎨 Animation patterns (10+ in library)
- 🎨 Colors and brightness levels
- 🎨 Animation speeds
- 🎨 Mode-specific behaviors

### Built-in Safety Features
- ⚡ Debounced mode switching (prevents flickering)
- ⚡ PWM timeout handling (handles no signal)
- ⚡ Memory overflow protection (compile-time checks)
- ⚡ Sanity test mode (validates all modes work)

---

## 🧪 Testing & Validation

### Bench Testing Requirements
1. **Sanity Test**: Pull PB3 low, verify all 4 modes cycle
2. **PWM Test**: Use servo tester to verify each mode range
3. **Transition Test**: Smooth mode changes without glitching
4. **Visual Test**: Verify colors, brightness, animation speed

### Memory Validation
```bash
# Check compiled size
arduino-cli compile --fqbn attiny:avr:ATtiny85 firmware.ino

# Look for:
# - Sketch uses X bytes (30%) of program storage (max 8192)
# - Global variables use Y bytes (50%) of RAM (max 512) ← CRITICAL
```

**Memory Budget Example (20 LEDs per strip):**
```
LED Arrays:     120 bytes  (20+20 LEDs × 3 bytes RGB)
Mode State:      30 bytes  (animation variables)
FastLED:         50 bytes  (library overhead)
Stack/Heap:      40 bytes  (runtime)
-------------------------------------------------
TOTAL:          240 bytes  ✓ (47% of 512 byte limit)
```

---

## 🛠️ Maintenance & Updates

### Version Control Strategy
```
project_root/
├── TEMPLATE_BASE.ino         # Master template (version controlled)
├── ANIMATION_LIBRARY.md      # Pattern catalog (add new patterns here)
├── customers/
│   ├── 2024_01_customer1/    # Per-customer folder
│   │   ├── firmware.ino
│   │   ├── notes.txt
│   │   └── test_results.txt
│   └── 2024_02_customer2/
│       └── ...
```

### When to Update Template
- New animation pattern requested by multiple customers
- Bug fix in core functionality
- FastLED library update
- Hardware revision

### Backward Compatibility
- Keep old customer firmware files for warranty support
- Tag each customer delivery: `git tag customer_name_v1.0`
- Document any breaking changes in template updates

---

## 📚 Learning Path

### New Users
1. Read `README_DEPLOYMENT.md` (full guide)
2. Study `TEMPLATE_BASE.ino` (understand structure)
3. Browse `ANIMATION_LIBRARY.md` (available patterns)
4. Create test firmware (practice customization)
5. Bench test with hardware

### Experienced Arduino Users
1. Skim `AGENTS.md` (coding standards)
2. Jump to `ANIMATION_LIBRARY.md` (copy patterns)
3. Modify `TEMPLATE_BASE.ino` directly
4. Deploy and test

### For AI Coding Agents
1. Parse `AGENTS.md` first (commands and constraints)
2. Reference `ANIMATION_LIBRARY.md` (pattern implementations)
3. Follow memory budget strictly (<512 bytes SRAM)
4. Maintain code style (camelCase, UPPERCASE constants, etc.)

---

## 🎯 Business Model Suggestions

### Pricing Strategy
- **Basic package**: Simple patterns, standard LED counts ($XX)
- **Custom package**: Unique patterns, customer colors ($XX)
- **Premium package**: Complex multi-mode, custom animations ($XX)
- **Firmware updates**: Maintenance/warranty ($XX/year)

### Value Propositions
- "Bespoke LED lighting for your racing drone"
- "4 custom modes - switch with your radio"
- "Professional patterns: chase, fade, strobe, team colors"
- "Optimized for battery life and visibility"
- "30-minute turnaround on custom requests"

### Marketing Ideas
- Video showcase of animation library
- Before/after comparison videos
- Social media: #CustomDroneLEDs
- Partner with drone shops/teams
- Offer team discounts

---

## 🔗 Related Resources

### Hardware
- [ATtiny85 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf)
- [WS2812B LED Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)

### Software
- [FastLED Library Documentation](http://fastled.io/)
- [Arduino-CLI Reference](https://arduino.github.io/arduino-cli/)
- [ATtiny Core Support](https://github.com/SpenceKonde/ATTinyCore)

### Community
- FPV Racing forums (customer base)
- Arduino forums (technical support)
- GitHub (template sharing)

---

## ⚠️ Important Notes

### Memory Constraints
The ATtiny85 has only **512 bytes of SRAM**. This is the #1 limitation:
- LED arrays consume: `(NUM_LEDS_1 + NUM_LEDS_2) × 3` bytes
- With 20+20 LEDs: 120 bytes (23% used before any code!)
- Always validate total usage stays under 512 bytes
- Use PROGMEM for lookup tables (stores in flash, not RAM)

### Power Considerations
- WS2812B LEDs: ~60mA max per LED at full white
- 40 LEDs at full brightness: 2.4A peak!
- Advise customers on appropriate BEC/power supply
- Lower brightness = longer flight times

### Timing Considerations
- PWM reading blocks for up to 50ms (pulseIn timeout)
- Keep animation update intervals reasonable (10-50ms)
- Sanity mode delay() calls are acceptable (test mode only)

---

## 🐛 Known Issues & Limitations

1. **Single PWM channel**: Can only control 4 modes (hardware limitation)
2. **Memory constrained**: Large strip counts (>30 LEDs) challenging
3. **No wireless config**: Must reprogram chip for changes
4. **8-bit processor**: Complex math/floating point slow
5. **No EEPROM usage**: Template doesn't save settings (could be added)

---

## 📝 TODO / Future Enhancements

- [ ] Add more patterns to animation library (fire, sparkle, etc.)
- [ ] Create web-based pattern designer tool
- [ ] Build automated testing rig
- [ ] Develop mobile app for wireless config (requires hardware change)
- [ ] Create video tutorials for each pattern
- [ ] Add EEPROM support for saving last mode
- [ ] Optimize memory usage further

---

## 📞 Support

For issues with the template system:
1. Check `README_DEPLOYMENT.md` troubleshooting section
2. Review `AGENTS.md` for coding guidelines
3. Verify memory usage with `arduino-cli compile`
4. Test with known-good hardware setup

---

**Template Version**: 5.4  
**Created**: 2024  
**License**: [Specify your license]  
**Author**: [Your name/business]

---

## 🎉 You're Ready!

You now have a complete, production-ready template system for creating custom drone LED controller firmware. Start by reading `README_DEPLOYMENT.md` and creating your first customer firmware!

Happy coding! 🚁💡✨
