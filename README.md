# PWM LED Controller v5.4

Bespoke ATtiny85-20PU firmware template for racing drone WS2812B LED lighting systems.

## Overview

This is a configurable firmware template for drone LED controllers with 4 customizable lighting modes, controlled via PWM input (0-2000µs range). Optimized for ATtiny85 microcontrollers with memory-efficient animations.

## Features

- **4 Configurable Modes**: Customizable static patterns, animations, and special effects
- **PWM Input Control**: 0-2000µs range (compatible with standard RC receivers)
- **WS2812B Support**: Dual-strip configuration (20 LEDs each by default)
- **Memory Optimized**: <512 bytes SRAM usage, efficient 8-bit arithmetic
- **Customer-Ready**: Template system for quick deployment

## Hardware Requirements

- **Microcontroller**: ATtiny85-20PU
- **LEDs**: WS2812B addressable LED strips
- **Input**: PWM signal (0-2000µs) on PB2
- **Output**: PB0, PB4 for LED strips

## Quick Start

### Compilation

```bash
arduino-cli compile --fqbn attiny:avr:ATtiny85 <customer_firmware>.ino
```

### Upload

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn attiny:avr:ATtiny85
```

## Project Structure

```
├── TEMPLATE_BASE.ino              # Base firmware template
├── AGENTS.md                      # AI agent instructions
├── ANIMATION_LIBRARY.md           # Animation reference
├── CUSTOMER_CONFIG_EXAMPLE.h      # Configuration template
├── PROJECT_OVERVIEW.md            # Detailed project docs
├── README_DEPLOYMENT.md           # Deployment guide
├── customers/                     # Customer-specific builds
│   └── 2025_01_yellow_purple_75mm/
├── main_led_controller_v5.4_updated/
└── .mcp/                          # Arduino MCP server (OpenCode integration)
```

## Mode Configuration

The firmware supports 4 PWM-controlled modes:

- **MODE_POWER_OFF** (<1000µs): LEDs off
- **MODE_1** (1000-1249µs): Customizable static/simple pattern
- **MODE_2** (1250-1499µs): Customizable animated pattern
- **MODE_3** (1500-1749µs): Customizable complex animation
- **MODE_4** (1750-2000µs): Customizable special effect

## Customization

1. Define customer requirements (see `CUSTOMER_CONFIG_EXAMPLE.h`)
2. Adjust LED counts and brightness
3. Implement mode update functions
4. Test PWM ranges and transitions
5. Validate memory usage (<512 bytes)

## Customer Deployments

### Yellow/Purple 75mm Configuration

A complete customer deployment example is included in `customers/2025_01_yellow_purple_75mm/`:
- Custom yellow/purple color scheme
- 4 distinct modes optimized for racing visibility
- Full compilation reports and test checklists

See the [customer README](customers/2025_01_yellow_purple_75mm/README.md) for details.

## OpenCode Integration

This project includes Arduino MCP server integration for seamless development in OpenCode:

```bash
cd .mcp
./setup.sh
```

See [.mcp/README.md](.mcp/README.md) for full setup instructions.

## Memory Budget

- **SRAM**: <512 bytes (ATtiny85 limit)
- **Flash**: Optimized with PROGMEM tables
- **Arithmetic**: 8-bit operations preferred

## Animation Library

Pre-built animations available:
- Solid colors
- Breathing effects
- Chase patterns
- Rainbow cycles
- Strobes
- Theater chase
- Police/fire patterns

See [ANIMATION_LIBRARY.md](ANIMATION_LIBRARY.md) for complete reference.

## Documentation

- **[AGENTS.md](AGENTS.md)**: Quick reference for AI agents
- **[PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md)**: Detailed architecture
- **[README_DEPLOYMENT.md](README_DEPLOYMENT.md)**: Deployment checklist
- **[ANIMATION_LIBRARY.md](ANIMATION_LIBRARY.md)**: Animation reference

## Build Tools

- **Arduino CLI**: Included in `bin/arduino-cli`
- **ATtiny Core**: attiny:avr:ATtiny85
- **FastLED**: For advanced animations (optional)

## Testing

PWM input testing covers:
- 0-2000µs range validation
- Mode transition boundaries
- Sanity mode via PB3 pin

## License

Proprietary - Custom firmware for client deployments

## Support

For customization requests or support, see project documentation or contact the developer.

---

**Current Version**: 5.4  
**Last Updated**: November 2025  
**Target Platform**: ATtiny85-20PU  
**LED Type**: WS2812B
