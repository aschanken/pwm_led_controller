# Arduino MCP Server - Quick Reference

## Installation (One Command!)

```bash
cd /home/ais/projects/main_led_controller_v5.4/.mcp && ./setup.sh
```

Then configure OpenCode using the JSON snippet output by setup.sh.

## What You Can Ask OpenCode

Once the MCP server is configured, you can interact with Arduino directly through OpenCode!

### Sketch Management

```plaintext
"Create a new Arduino sketch called BlinkTest"
"Show me all my Arduino sketches"
"Read my LED_Controller sketch"
"Update my sketch to blink faster"
```

### Compilation

```plaintext
"Compile my sketch for Arduino Uno"
"Verify my ATtiny85 code"
"Check if my sketch has any errors"
```

### Uploading

```plaintext
"Upload my sketch to the Arduino on /dev/ttyUSB0"
"Flash my LED controller to the board"
```

### Board Management

```plaintext
"What Arduino boards are connected?"
"Find the FQBN for ESP32"
"Search for Arduino Nano boards"
```

### Library Management

```plaintext
"Search for the FastLED library"
"Install the Adafruit NeoPixel library"
"Show me examples from the FastLED library"
"What libraries are available for WS2812 LEDs?"
```

### Your ATtiny85 LED Project

```plaintext
"Compile main_led_controller_v5.4 for ATtiny85"
"What's the FQBN for ATtiny85?"
"Verify my LED controller code"
"Upload to ATtiny85 on /dev/ttyUSB0"
```

## Command Examples

### Create and Edit a Sketch

```plaintext
User: "Create a new sketch called RainbowLED"
OpenCode: [Creates sketch at ~/Documents/Arduino_MCP_Sketches/RainbowLED/]

User: "Add code to make a rainbow pattern with FastLED"
OpenCode: [Writes code with FastLED rainbow effect]

User: "Compile it for ATtiny85"
OpenCode: [Runs arduino-cli compile with attiny:avr:ATtiny85 FQBN]
```

### Install and Use a Library

```plaintext
User: "Search for WS2812 libraries"
OpenCode: [Shows FastLED, Adafruit_NeoPixel, etc.]

User: "Install FastLED"
OpenCode: [Installs via arduino-cli]

User: "Show me FastLED examples"
OpenCode: [Lists example sketches with paths]

User: "Read the FastLED Blink example"
OpenCode: [Shows example code]
```

### Working with Your Existing Project

```plaintext
User: "Compile customers/2025_01_yellow_purple_75mm/yellow_purple_75mm_led_controller.ino"
OpenCode: [Compiles with appropriate FQBN]

User: "Check memory usage"
OpenCode: [Shows program storage and RAM usage from compile output]
```

## Board FQBNs for Your Project

Common boards you might use:

```plaintext
ATtiny85 (8MHz): attiny:avr:ATtiny85:cpu=attiny85,clock=8internal
ATtiny85 (1MHz): attiny:avr:ATtiny85:cpu=attiny85,clock=1internal
Arduino Uno: arduino:avr:uno
Arduino Nano: arduino:avr:nano
```

To find others: "Search for [board name] FQBN"

## Sketch Directory Structure

The MCP server creates sketches at:
```
~/Documents/Arduino_MCP_Sketches/
├── MySketch/
│   ├── MySketch.ino  (main file)
│   ├── config.h      (optional)
│   └── helpers.cpp   (optional)
└── AnotherSketch/
    └── AnotherSketch.ino
```

## File Paths

When referencing files:
- Use `~` for home directory
- Absolute paths work: `/home/ais/projects/...`
- Relative to sketch dir: `MySketch/MySketch.ino`

## Compilation Notes

- Auto-compilation happens when you write .ino files
- Default FQBN: `arduino:avr:uno`
- Specify FQBN with: "Compile for ATtiny85"
- Build temp files: `~/Documents/Arduino_MCP_Sketches/_build_temp/`

## Troubleshooting Quick Fixes

### "Arduino CLI not found"
```bash
chmod +x /home/ais/projects/main_led_controller_v5.4/bin/arduino-cli
```

### "MCP server not responding"
Check OpenCode output panel for errors, restart OpenCode

### "Missing Python packages"
```bash
pip3 install --user "mcp[cli]" thefuzz wireviz openai
```

### "Permission denied"
Operations are restricted to your home directory for security

## Tips

1. **Be specific**: "Compile for ATtiny85" is better than "compile my sketch"
2. **Use sketch names**: "Update my BlinkTest sketch" 
3. **Check errors**: If upload fails, try "verify code" first
4. **Board detection**: Connect your board before asking "what boards are connected?"
5. **Library search**: Both online and local libraries are searched automatically

## Environment

- Python: 3.12.3
- Arduino CLI: Use `../bin/arduino-cli version` to check
- Sketches: `~/Documents/Arduino_MCP_Sketches/`
- Your project: `/home/ais/projects/main_led_controller_v5.4/`

## Getting Help

Ask me:
- "How do I [do something with Arduino]?"
- "What Arduino tools are available?"
- "Help with Arduino MCP server"
- "Show me my MCP server configuration"

Happy coding! 🚀
