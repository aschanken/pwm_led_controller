#!/bin/bash
# Wrapper script to run Arduino MCP Server with proper environment

# Set Arduino CLI path to your project's arduino-cli
export ARDUINO_CLI_PATH="/home/ais/projects/main_led_controller_v5.4/bin/arduino-cli"

# Set sketch directory
export MCP_SKETCH_DIR="$HOME/Documents/Arduino_MCP_Sketches"

# Run the Python MCP server using system Python
cd "/home/ais/projects/main_led_controller_v5.4/.mcp/arduino-server"
exec /usr/bin/python3 -m mcp_arduino_server.server
