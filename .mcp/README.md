# Arduino MCP Server for OpenCode

This directory contains the Arduino MCP (Model Context Protocol) server setup that enables OpenCode to interact directly with Arduino development tools.

## Quick Start

### 1. Run the Setup Script

```bash
cd /home/ais/projects/main_led_controller_v5.4/.mcp
./setup.sh
```

This script will:
- Check your Python installation
- Install required Python packages (mcp, thefuzz, wireviz, openai)
- Verify arduino-cli is working
- Create necessary directories
- Test the MCP server

### 2. Configure OpenCode

After running setup, you need to add the Arduino MCP server to OpenCode:

**Option A: Using OpenCode UI**
1. Press `Ctrl+P` in OpenCode
2. Type "MCP: Add Server"
3. Enter the configuration from the output of setup.sh

**Option B: Manual Configuration**
Edit your OpenCode MCP configuration file and add:

```json
{
  "mcpServers": {
    "arduino": {
      "command": "/home/ais/projects/main_led_controller_v5.4/.mcp/run-arduino-mcp.sh",
      "args": [],
      "env": {
        "ARDUINO_CLI_PATH": "/home/ais/projects/main_led_controller_v5.4/bin/arduino-cli",
        "MCP_SKETCH_DIR": "/home/ais/Documents/Arduino_MCP_Sketches",
        "LOG_LEVEL": "INFO"
      }
    }
  }
}
```

### 3. Restart OpenCode

Restart OpenCode to load the new MCP server.

### 4. Test It Out!

Once configured, you can ask me (OpenCode) things like:
- "Create a new Arduino sketch called Blink"
- "Show me all my Arduino sketches"
- "Compile my LED controller sketch for ATtiny85"
- "Upload the sketch to my Arduino"
- "Search for the FastLED library"
- "What example sketches does FastLED provide?"

## What's Included

### Files

- `arduino-server/` - The MCP server source code (cloned from Volt23/mcp-arduino-server)
- `run-arduino-mcp.sh` - Wrapper script to launch the server with correct environment
- `setup.sh` - One-command setup script
- `INSTALLATION.md` - Detailed installation and troubleshooting guide
- `README.md` - This file

### Features

The Arduino MCP Server provides these tools:

#### Sketch Management
- Create new sketches
- List all sketches
- Read/write sketch files (with auto-compile!)

#### Compilation & Upload
- Verify code (compile without upload)
- Upload sketches to boards
- Automatic FQBN detection

#### Board Management
- List connected Arduino boards
- Search for board information
- Get board specifications

#### Library Management
- Search Arduino libraries (online + local fuzzy search)
- Install libraries
- List library examples

#### File Operations
- Read/write files
- File management within sketch directories

#### Circuit Diagrams (Optional)
- Generate WireViz circuit diagrams from descriptions
- Requires OpenAI API key

## Directory Structure

```
.mcp/
├── arduino-server/          # MCP server source
│   ├── src/
│   │   └── mcp_arduino_server/
│   │       └── server.py    # Main server code
│   ├── pyproject.toml       # Python package config
│   └── README.md           # Server documentation
├── run-arduino-mcp.sh      # Launch script
├── setup.sh                # Setup automation
├── INSTALLATION.md         # Detailed guide
└── README.md              # This file
```

## Troubleshooting

### Server won't start
1. Check Python packages are installed: `python3 -c "import mcp; print('OK')"`
2. Check arduino-cli works: `../bin/arduino-cli version`
3. Check logs in OpenCode's output panel

### Can't find arduino-cli
Make sure the binary is executable:
```bash
chmod +x /home/ais/projects/main_led_controller_v5.4/bin/arduino-cli
```

### Permission errors
The server only operates within:
- Your home directory (`~`)
- The sketch directory (`~/Documents/Arduino_MCP_Sketches`)

### Dependencies missing
Re-run the setup script:
```bash
./setup.sh
```

## Advanced Configuration

### Environment Variables

You can customize the MCP server by setting these environment variables in the OpenCode config:

- `ARDUINO_CLI_PATH` - Path to arduino-cli binary
- `MCP_SKETCH_DIR` - Where to store/read sketches
- `LOG_LEVEL` - Logging verbosity (DEBUG, INFO, WARNING, ERROR)
- `OPENAI_API_KEY` - For AI circuit diagram generation
- `WIREVIZ_PATH` - Path to wireviz binary (if not in PATH)

### Custom Sketch Location

To use a different sketch directory, update `MCP_SKETCH_DIR` in your OpenCode config.

## More Information

- MCP Server Source: https://github.com/Volt23/mcp-arduino-server
- Arduino CLI: https://arduino.github.io/arduino-cli/
- Model Context Protocol: https://modelcontextprotocol.io/

## Support

For issues with:
- **The MCP server itself**: Check the [arduino-server README](arduino-server/README.md)
- **OpenCode integration**: See INSTALLATION.md
- **Arduino CLI**: See arduino-cli documentation
- **Your LED controller project**: Ask me directly in OpenCode!
