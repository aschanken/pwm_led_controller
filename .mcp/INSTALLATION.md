# Arduino MCP Server Installation Guide

## What You Have

The Arduino MCP Server has been cloned to:
`/home/ais/projects/main_led_controller_v5.4/.mcp/arduino-server/`

Your arduino-cli is located at:
`/home/ais/projects/main_led_controller_v5.4/bin/arduino-cli`

## Installation Steps

### 1. Install Required Python Packages

You need to install these dependencies for the MCP server:

```bash
# Option A: Using pip (if available)
pip3 install --user "mcp[cli]" "thefuzz[speedup]" wireviz openai

# Option B: Using apt (if pip not available)
sudo apt update
sudo apt install python3-pip python3-venv
pip3 install --user "mcp[cli]" "thefuzz[speedup]" wireviz openai
```

### 2. Configure OpenCode

Add this configuration to your OpenCode MCP settings.

To do this:
1. Open your terminal
2. Edit the OpenCode config:
   ```bash
   # The config file might be at one of these locations:
   nano ~/.config/opencode/mcp_config.json
   # OR use OpenCode's built-in config editor (Ctrl+P > "MCP: Add Server")
   ```

3. Add this JSON configuration:

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

### 3. Test the Installation

Test if the arduino-cli works:

```bash
/home/ais/projects/main_led_controller_v5.4/bin/arduino-cli version
```

Test if Python can import the MCP packages:

```bash
python3 -c "import mcp; print('MCP installed!')"
```

### 4. Restart OpenCode

After configuration, restart OpenCode to load the new MCP server.

## What the MCP Server Provides

Once configured, you'll have access to these Arduino tools via OpenCode:

- **create_new_sketch** - Create new Arduino sketches
- **list_sketches** - List all your sketches
- **read_file** - Read sketch files
- **write_file** - Write/edit sketch files (with auto-compile!)
- **verify_code** - Compile sketches to check for errors
- **upload_sketch** - Upload to connected Arduino boards
- **list_boards** - Detect connected Arduino boards
- **board_search** - Search for board FQBNs
- **lib_search** - Search for Arduino libraries
- **lib_install** - Install libraries
- **list_library_examples** - View library examples

## Troubleshooting

### If python3-pip is not available:
```bash
sudo apt update
sudo apt install python3-pip python3-venv
```

### If MCP installation fails:
```bash
# Try installing from source
git clone https://github.com/modelcontextprotocol/python-sdk.git
cd python-sdk
pip3 install --user -e .
```

### Check logs:
The MCP server logs to stderr. Check OpenCode's output panel for errors.

### Arduino CLI not found:
Make sure your arduino-cli binary has execute permissions:
```bash
chmod +x /home/ais/projects/main_led_controller_v5.4/bin/arduino-cli
```

## Additional Configuration

### Set OpenAI API Key (for WireViz circuit diagrams):
If you want to use the AI-powered circuit diagram generation feature:

```bash
export OPENAI_API_KEY="your-api-key-here"
```

Or add it to the `env` section of your MCP configuration.
