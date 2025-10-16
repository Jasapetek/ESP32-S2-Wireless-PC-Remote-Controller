# ESP32-S2 Wireless Computer Remote Controller

## Screenshots

**Main Control Interface (Light Mode)**
![Main Interface](Screenshot%202025-10-16%20182645.png)

**Main Control Interface (Dark Mode)**
![Action Buttons](Screenshot%202025-10-16%20182705.png)

**Power Controls**
![Connected Web UI](Screenshot%202025-10-16%20182726.png)

**Created by:** Jaša Petek  
**Hackster.io:** [https://www.hackster.io/petekjasa](https://www.hackster.io/petekjasa)  
**LinkedIn:** [https://www.linkedin.com/in/ja%C5%A1a-petek-2504ba387/](https://www.linkedin.com/in/ja%C5%A1a-petek-2504ba387/)

⚠️ **Important:** This project emulates a USB HID device. Only use it on computers you own or have permission to control. Misuse can cause serious issues.

## Overview

This project allows you to control your PC remotely using your phone via WiFi. The ESP32-S2 acts as a wireless USB HID device, providing seamless computer control through a modern web interface.

🎥[Demo Video](https://vimeo.com/1127934041?share=copy&fl=sv&fe=ci)

### Features

- **Mouse and keyboard control** - Full cursor movement, clicks, and text input
- **Shortcut commands** - Run dialog, Task Manager, Show Desktop
- **Power controls** - Shutdown, Restart, Lock Computer, Sleep, Wake-up (BETA)
- **Modern web interface** - Clean, mobile-optimized UI with dark/light mode toggle
- **Fast performance** - Instant boot (~10 seconds) and ~50ms input delay
- **USB-stick form factor** - ESP32-S2 with Type-A connector for plug-and-play convenience

⚠️ **Note:** Some advanced features such as wake computer from sleep are still in development (BETA) and may not work reliably on all systems.

## What You Need

### Hardware
- ESP32-S2 board with native USB support (can be built into a USB-stick form factor)
- USB cable (for programming and optional power)

### Software
- [Arduino IDE](https://www.arduino.cc/en/software) (latest version recommended)
- ESP32 Board Support in Arduino IDE
- Required libraries (install via Library Manager)

## Board Settings in Arduino IDE

1. Go to **Tools > Board > ESP32S2 Dev Module** (or your ESP32-S2 variant)
2. **USB CDC On Boot:** Enabled
3. **Upload Mode:** Internal USB (if available)

⚠️ **Important:** Ensure your board has native USB support. HID functionality will not work on boards without it.

## Installation & Upload

1. Open the `.ino` file in Arduino IDE
2. **IMPORTANT:** Change the default WiFi credentials in the code before uploading:
3. Install any missing libraries via Library Manager
4. Connect your ESP32-S2 via USB (or use the built-in Type-A connector if your board allows)
5. Press **Upload** in Arduino IDE
6. Wait a few seconds for the ESP32-S2 to boot and create its WiFi network

## Using the Remote Controller

1. **Connect to WiFi:** Connect your phone to the ESP32-S2 WiFi network using your custom credentials
2. **Open Web Interface:** Navigate to the displayed IP address in your browser (`http://192.168.4.1`)
3. **Control Your PC:** Use the modern UI with the following features:

### Power Controls
- 🔴 **Shutdown** - Safely shutdown your computer
- 🔄 **Restart** - Restart your computer
- 🔒 **Lock Computer** - Lock your Windows session
- 😴 **Sleep** - Put your computer to sleep mode
- ⚡ **Wake Up (BETA)** - Attempt to wake computer from sleep

### Additional Features
- **Mouse Control** - Touchpad-style movement and clicks (left, right, double-click, scroll)
- **Keyboard Input** - Type text directly or use common shortcuts (Ctrl+C, Ctrl+V, Alt+Tab)
- **System Actions** - Quick access to Task Manager, Run Dialog, Show Desktop
- **Theme Toggle** - Switch between dark and light mode (located in header)

⚠️ **Note:** Wake-up functionality is experimental and may not work on all PCs. Screenshot functionality has been temporarily removed and will be re-added in future updates.

## WiFi Network Configuration

### Default Credentials (⚠️ CHANGE THESE!)
- **Default SSID:** `WirelessRemote-ESP32`
- **Default Password:** `RemoteControl123`

### ⚠️ Security Warning
**You MUST change the default WiFi credentials before deploying this project!** The default credentials are publicly known and pose a security risk.

### How to Change Credentials
1. Open the Arduino code (`.ino` file)
2. Find these lines at the top:
   ```cpp
   // WiFi credentials for Access Point mode
   const char* ssid = "WirelessRemote-ESP32";     // Change this!
   const char* password = "RemoteControl123";     // Change this password!
   ```
3. Replace with your own secure network name and password:
   ```cpp
   // WiFi credentials for Access Point mode
   const char* ssid = "YourCustomNetworkName";
   const char* password = "YourSecurePassword123";
   ```
4. Save and upload the modified code to your ESP32-S2

### Password Requirements
- **Minimum 8 characters** (WPA2 requirement)
- **Use a strong, unique password** - avoid common words or patterns
- **Don't use the same password** as your main WiFi network

## Safety & Usage Tips

- **Security First:** Do not plug into untrusted computers. HID devices can send commands without user confirmation.
- **Change Default Credentials:** Never use the default WiFi credentials in production
- **Confirmation Dialogs:** All power controls (shutdown, restart, sleep) include confirmation prompts to prevent accidental execution.
- **Physical Protection:** If using USB-stick form factor, protect the exposed Type-A connector when not in use to prevent damage.
- **Network Security:** The WiFi access point is open to anyone who knows the credentials

## Troubleshooting

- **HID Not Working:** Ensure your ESP32-S2 board supports native USB HID
- **Can't Connect to WiFi:** Verify you changed the credentials in the code and uploaded successfully
- **Commands Not Responding:** Verify the device is enabled via the Toggle Device button
- **Upload Failures:** Make sure "USB CDC On Boot" is enabled in Arduino IDE settings

## Credits

**Project Author:** Jaša Petek  
**Hackster.io:** [https://www.hackster.io/petekjasa](https://www.hackster.io/petekjasa)  
**LinkedIn:** [https://www.linkedin.com/in/ja%C5%A1a-petek-2504ba387/](https://www.linkedin.com/in/ja%C5%A1a-petek-2504ba387/)

Please credit Jaša Petek if you reference or use this project.

## Future Updates

- [ ] Screenshot download functionality
- [ ] Additional macros and system commands
- [ ] Improved Wake PC from sleep reliability

## License

This project is provided as-is for educational and personal use. Use responsibly and only on systems you own or have explicit permission to control.

---

**Disclaimer:** This tool is designed for legitimate remote control purposes. The author is not responsible for any misuse or damage caused by this software. Always ensure you have proper authorization before using HID emulation tools.
