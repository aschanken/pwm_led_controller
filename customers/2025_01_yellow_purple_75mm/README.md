# Yellow/Purple 75mm Drone - Custom LED Controller

**Customer**: Yellow Frame / Purple Canopy 75mm Racing Drone  
**Firmware Version**: v1.2 (Seamless + Faster Rainbow)  
**Build Date**: 2025-11-02  
**LED Configuration**: 2x 18 WS2812B strips (36 total LEDs)

---

## 📦 Package Contents

- `yellow_purple_75mm_led_controller.ino` - Custom firmware (ready to upload)
- `customer_notes.txt` - Detailed specifications and requirements
- `test_checklist.txt` - Complete testing procedure
- `README.md` - This file

---

## 🎨 Lighting Modes

### Mode 1: Yellow Spinning Blocks (1000-1249µs)
Bright yellow blocks spin seamlessly around the entire drone perimeter. Both LED strips act as one continuous 36-LED ring with brightness gradient from 70% (bright) to 30% (dim).

**Best for**: High-speed racing, maximum visibility

### Mode 2: Purple/Yellow Fading Blocks (1250-1499µs)
Color blocks rotate seamlessly around the entire drone perimeter while smoothly morphing between purple (canopy color) and yellow (frame color). Both strips act as one 36-LED ring. Set at 50% brightness for battery efficiency.

**Best for**: Freestyle flying, visually interesting patterns, seamless color flow

### Mode 3: Rainbow Scroll (1500-1749µs)
Full spectrum rainbow scrolls FAST and seamlessly around both strips while colors continuously shift through the spectrum. Both strips act as one 36-LED ring displaying the complete rainbow at 78% brightness. Ultra-smooth at 143 fps!

**Best for**: High visibility, party mode, spectacular visual effect, orientation via color position

### Mode 4: Police Lights (1750-2000µs)
Red/blue emergency light pattern adapted for 18 LEDs per strip. High-intensity strobing effect for maximum attention.

**Best for**: Locating crashed drone, attention-getting

---

## 🔧 Hardware Specifications

- **Microcontroller**: ATtiny85-20PU (8MHz internal clock)
- **Memory Usage**: 350 bytes RAM (68% of 512 bytes available) ✓
- **Flash Usage**: 7,304 bytes (89% of 8,192 bytes available) ✓
- **Power**: 5V via BEC (recommend >2A capacity)
- **Pin Connections**:
  - PB0 (pin 5) → Front LED strip (18 LEDs)
  - PB4 (pin 3) → Rear LED strip (18 LEDs)
  - PB2 (pin 7) → PWM input from radio receiver
  - PB3 (pin 2) → Test mode button (optional)

---

## 🚀 Quick Start

### 1. Programming the ATtiny85

```bash
# Compile
arduino-cli compile --fqbn attiny:avr:ATtinyX5:cpu=attiny85,clock=internal8 yellow_purple_75mm_led_controller.ino

# Upload via USBtinyISP
arduino-cli upload -c usbtinyisp --fqbn attiny:avr:ATtinyX5:cpu=attiny85,clock=internal8 yellow_purple_75mm_led_controller
```

### 2. Bench Testing

1. Connect 5V power and ground
2. Connect both LED strips
3. Connect PWM signal from servo tester
4. Test each mode by varying PWM signal (1000-2000µs)
5. Optional: Test sanity mode by pulling PB3 to ground

See `test_checklist.txt` for complete testing procedure.

### 3. Installation on Drone

1. Connect ATtiny85 to drone's 5V power (via BEC)
2. Wire LED strips to PB0 and PB4
3. Connect PWM signal to available radio channel
4. Secure all connections and test before flight

---

## 📻 Radio Setup

Configure an auxiliary channel on your radio transmitter:

**Recommended**: 3-position switch or continuous dial

| Switch Position | PWM Range | Mode |
|----------------|-----------|------|
| Position 1 | 1000-1249µs | Yellow Spinning Blocks |
| Position 2 | 1250-1499µs | Purple/Yellow Fade |
| Position 3 | 1500-1749µs | Alternating Pulse |
| Position 4* | 1750-2000µs | Police Lights |

*Requires 4-position switch or dial

---

## 🧪 Test Mode (Sanity Check)

The firmware includes a built-in test mode:

1. Pull PB3 (pin 2) to ground for 100ms
2. LEDs blink white to confirm entry
3. All 4 modes cycle automatically (1 second each)
4. Release PB3 to return to normal operation

Use this to verify all modes work before installation.

---

## 🔋 Power Consumption

| Mode | Brightness | Power Draw | Battery Impact |
|------|-----------|------------|----------------|
| Mode 1 | 70%→30% | Medium-High | Moderate |
| Mode 2 | 50% | Medium | Good |
| Mode 3 | 20%→78% pulse | Low-Medium | Best |
| Mode 4 | 100% strobe | High | High |

**Tip**: Use Modes 2 or 3 for longer flight times.

---

## 🛠️ Troubleshooting

### LEDs don't light up
- Check 5V power connection
- Verify ground connection
- Test LED strips separately
- Check data wire connections

### Wrong colors displayed
- Verify LED_TYPE in firmware (WS2812B)
- Check COLOR_ORDER (GRB vs RGB)
- Test with known-good LED strip

### Modes don't change
- Verify PWM signal connection to PB2
- Test PWM signal with servo tester
- Check radio channel configuration
- Verify ATtiny85 is programmed correctly

### Erratic behavior
- Check for loose connections
- Verify adequate power supply (>2A)
- Add 470µF capacitor across power rails
- Ensure LED strips are within spec (18 LEDs each)

---

## 📊 Technical Details

**Memory Budget**:
```
LED Arrays:          108 bytes (36 LEDs × 3 bytes)
Mode State Vars:      24 bytes (all modes)
FastLED Overhead:    ~50 bytes
Stack/Runtime:       ~40 bytes
-------------------------
TOTAL:              ~222 bytes (43% of 512 bytes) ✓
```

**Animation Frame Rates**:
- Mode 1: ~40 fps (25ms update interval)
- Mode 2: ~20 fps (50ms rotation + color fade)
- Mode 3: ~50 fps (20ms pulse update)
- Mode 4: ~45 fps (22ms stage cycle)

---

## 📞 Support

For technical support or firmware modifications:

- **Documentation**: See `customer_notes.txt` for detailed specs
- **Testing**: Use `test_checklist.txt` for systematic verification
- **Firmware Source**: `yellow_purple_75mm_led_controller.ino`

---

## ✅ Delivery Checklist

Before delivering to customer:

- [ ] Firmware compiled successfully
- [ ] Memory usage verified (<512 bytes RAM)
- [ ] All 4 modes tested and working
- [ ] PWM ranges verified
- [ ] Sanity mode tested
- [ ] No overheating issues
- [ ] Visual quality approved
- [ ] Documentation package complete

---

**Firmware Version**: v1.0  
**Build System**: Arduino IDE / arduino-cli  
**Libraries Required**: FastLED  
**ATtiny Board Package**: attiny:avr  

**Status**: ✅ Ready for Testing & Deployment

---

*Built with the Drone LED Controller Template System v5.4*
