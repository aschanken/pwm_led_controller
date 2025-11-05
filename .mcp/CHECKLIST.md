# Arduino MCP Server - Installation Checklist

Follow this checklist to complete the installation.

## ☐ Step 1: Run Setup Script

```bash
cd /home/ais/projects/main_led_controller_v5.4/.mcp
./setup.sh
```

**Expected output:**
- ✓ Python 3.12.3 detected
- ✓ pip available
- ✓ MCP SDK installed
- ✓ thefuzz installed
- ✓ arduino-cli working
- ✓ Sketch directory created
- Configuration JSON displayed

**If it fails:** Check INSTALLATION.md for troubleshooting

---

## ☐ Step 2: Copy the Configuration

After setup.sh completes, it will display a JSON configuration.

Copy that entire JSON block (it starts with `{` and ends with `}`)

---

## ☐ Step 3: Add to OpenCode

**Method 1: Using OpenCode UI (Recommended)**

1. Press `Ctrl+P` in OpenCode
2. Type: `MCP: Add Server`
3. Paste the JSON configuration
4. Save

**Method 2: Manual Configuration**

1. Find your OpenCode config directory:
   - Usually: `~/.config/opencode/`
2. Look for or create: `mcp_config.json` or similar
3. Add the `arduino` server configuration
4. Save the file

**The configuration should look like this:**
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

---

## ☐ Step 4: Restart OpenCode

1. Close all OpenCode windows
2. Restart OpenCode
3. Wait for it to fully load

---

## ☐ Step 5: Verify Installation

Test that the MCP server is working by asking OpenCode:

### Test 1: Basic Connection
```
"What Arduino tools are available?"
```
**Expected:** List of available Arduino MCP tools

### Test 2: Create Sketch
```
"Create a new Arduino sketch called TestBlink"
```
**Expected:** Success message with sketch path

### Test 3: List Sketches
```
"Show me all my Arduino sketches"
```
**Expected:** List including "TestBlink"

### Test 4: Board Detection
```
"What Arduino boards are connected?"
```
**Expected:** List of connected boards (or "No boards detected" if none connected)

---

## ☐ Step 6: (Optional) Test with Your Project

If everything works, try compiling your LED controller:

```
"Compile main_led_controller_v5.4_updated.ino for ATtiny85"
```

---

## Troubleshooting Checklist

### ☐ If setup.sh fails:

- [ ] Python 3.10+ installed? Check with: `python3 --version`
- [ ] pip available? Try: `sudo apt install python3-pip`
- [ ] Permissions OK? Run: `chmod +x .mcp/*.sh bin/arduino-cli`

### ☐ If OpenCode doesn't see the server:

- [ ] Configuration added correctly?
- [ ] OpenCode restarted?
- [ ] Check OpenCode output panel for errors
- [ ] Verify paths in config match your system

### ☐ If server starts but commands fail:

- [ ] Python packages installed? Test: `python3 -c "import mcp"`
- [ ] arduino-cli works? Test: `bin/arduino-cli version`
- [ ] Check logs in OpenCode output panel

### ☐ If compilation fails:

- [ ] ATtiny core installed? Run: `bin/arduino-cli core install attiny:avr`
- [ ] Correct FQBN? Try: "Search for ATtiny85 FQBN"
- [ ] Sketch path correct?

---

## Quick Reference

### Important Paths
- **MCP Server:** `/home/ais/projects/main_led_controller_v5.4/.mcp/`
- **Arduino CLI:** `/home/ais/projects/main_led_controller_v5.4/bin/arduino-cli`
- **Sketches:** `~/Documents/Arduino_MCP_Sketches/`
- **Your Project:** `/home/ais/projects/main_led_controller_v5.4/`

### Important Commands
- **Setup:** `cd .mcp && ./setup.sh`
- **Test Arduino CLI:** `bin/arduino-cli version`
- **Test Python:** `python3 -c "import mcp"`
- **View logs:** Check OpenCode output panel

### Documentation
- **Start:** `SETUP_COMPLETE.md`
- **Full guide:** `README.md`
- **Commands:** `QUICK_REFERENCE.md`
- **Troubleshooting:** `INSTALLATION.md`

---

## Success Indicators

You'll know it's working when:

✓ setup.sh completes without errors
✓ OpenCode shows Arduino tools in MCP menu (if available)
✓ You can ask "Create a new sketch" and it works
✓ You can ask "What boards are connected?" and get a response
✓ Compilation commands work
✓ You can search for and install libraries

---

## Next Steps After Installation

Once everything works:

1. **Explore:** Ask "What can I do with Arduino MCP?"
2. **Learn:** Read `QUICK_REFERENCE.md` for command examples
3. **Build:** Start working on your LED controller project!
4. **Ask me anything:** I'm here to help with Arduino development

---

**Need Help?**

- Check `INSTALLATION.md` for detailed troubleshooting
- Review `SETUP_COMPLETE.md` for step-by-step instructions
- Ask me: "Help with Arduino MCP installation"
- Check OpenCode output panel for error messages

Happy coding! 🚀
