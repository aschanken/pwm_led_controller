#!/bin/bash
# Arduino MCP Server Setup Script

set -e  # Exit on error

echo "==================================="
echo "Arduino MCP Server Setup"
echo "==================================="
echo

# Check if we're in the right directory
if [ ! -d "arduino-server" ]; then
    echo "Error: Must run from .mcp directory"
    exit 1
fi

# Step 1: Check Python
echo "Step 1: Checking Python installation..."
if ! command -v python3 &> /dev/null; then
    echo "Error: python3 not found. Please install Python 3.10 or higher."
    exit 1
fi

PYTHON_VERSION=$(python3 --version | cut -d ' ' -f2 | cut -d '.' -f1,2)
echo "Found Python $PYTHON_VERSION"

# Step 2: Check/Install pip
echo
echo "Step 2: Checking pip..."
if ! python3 -m pip --version &> /dev/null; then
    echo "pip not found. Attempting to install..."
    if command -v apt &> /dev/null; then
        echo "Installing python3-pip via apt..."
        sudo apt update && sudo apt install -y python3-pip python3-venv
    else
        echo "Error: pip not found and cannot auto-install. Please install python3-pip manually."
        exit 1
    fi
fi

echo "pip is available"

# Step 3: Install Python dependencies
echo
echo "Step 3: Installing Python dependencies..."
echo "This may take a few minutes..."

pip3 install --user "mcp[cli]" "thefuzz[speedup]" wireviz openai || {
    echo "Warning: Some packages failed to install. Trying without optional dependencies..."
    pip3 install --user mcp thefuzz wireviz openai
}

# Step 4: Verify installations
echo
echo "Step 4: Verifying installations..."

if python3 -c "import mcp" 2>/dev/null; then
    echo "✓ MCP SDK installed successfully"
else
    echo "✗ MCP SDK installation failed"
    exit 1
fi

if python3 -c "import thefuzz" 2>/dev/null; then
    echo "✓ thefuzz installed successfully"
else
    echo "✗ thefuzz installation failed"
fi

# Step 5: Check arduino-cli
echo
echo "Step 5: Checking arduino-cli..."
ARDUINO_CLI_PATH="/home/ais/projects/main_led_controller_v5.4/bin/arduino-cli"

if [ -f "$ARDUINO_CLI_PATH" ]; then
    chmod +x "$ARDUINO_CLI_PATH"
    if "$ARDUINO_CLI_PATH" version &> /dev/null; then
        echo "✓ arduino-cli is working"
        "$ARDUINO_CLI_PATH" version
    else
        echo "✗ arduino-cli found but not executable"
        exit 1
    fi
else
    echo "✗ arduino-cli not found at $ARDUINO_CLI_PATH"
    exit 1
fi

# Step 6: Create sketches directory
echo
echo "Step 6: Creating sketches directory..."
mkdir -p "$HOME/Documents/Arduino_MCP_Sketches"
echo "✓ Sketch directory created at $HOME/Documents/Arduino_MCP_Sketches"

# Step 7: Test the server
echo
echo "Step 7: Testing MCP server import..."
cd arduino-server
if python3 -c "from mcp_arduino_server import server" 2>/dev/null; then
    echo "✓ MCP server module can be imported"
else
    echo "✗ MCP server module import failed"
    echo "  This might be OK if the server runs via package installation"
fi
cd ..

echo
echo "==================================="
echo "Setup Complete!"
echo "==================================="
echo
echo "Next steps:"
echo "1. Read INSTALLATION.md for configuration instructions"
echo "2. Configure OpenCode to use this MCP server"
echo "3. Restart OpenCode"
echo
echo "Configuration snippet for OpenCode:"
echo '{'
echo '  "mcpServers": {'
echo '    "arduino": {'
echo '      "command": "'$(pwd)'/run-arduino-mcp.sh",'
echo '      "args": [],'
echo '      "env": {'
echo '        "ARDUINO_CLI_PATH": "'$ARDUINO_CLI_PATH'",'
echo '        "MCP_SKETCH_DIR": "'$HOME'/Documents/Arduino_MCP_Sketches"'
echo '      }'
echo '    }'
echo '  }'
echo '}'
echo
