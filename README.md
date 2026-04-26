# ESP32Matrix

ESP32 firmware for a 16×16 WS2812B LED matrix running Tetris, Snake, Breakout, and a Pixel Art viewer.

Based on the original work by [Scott Marley](https://github.com/s-marley/ESP32Matrix) — see his [YouTube video](https://www.youtube.com/watch?v=apmOSQmeKJA) for a full walkthrough of the original project.

## Changes from the original

- **Bluetooth removed** — replaced with WiFi + WebSocket control
- **Web-based UI** — a browser controller page is served directly from the ESP32 at `http://<device-ip>/`. No app install required, works on any device
- **OTA updates** — flash new firmware over WiFi (hostname: `esp32matrix`, password: `matrix-ota`)
- **Config page** — adjust brightness, milliamps, and voltage at `http://<device-ip>/config`, saved to NVS so settings persist across reboots
- **Breakout paddle fix** — paddle now reaches both edges of the screen
- **Pixel art smiley fix** — smiley no longer loops back for a second partial pass

## Hardware

- ESP32 (tested on LOLIN D32)
- 16×16 WS2812B LED matrix
- Data pin: GPIO 26

## Setup

1. Copy `secrets.h.sample` to `secrets.h` and fill in your WiFi credentials:
   ```cpp
   const char* ssid     = "your_wifi_ssid";
   const char* password = "your_wifi_password";
   ```

2. Install the required libraries via Arduino Library Manager:
   - FastLED
   - LEDMatrix
   - LEDSprites
   - LEDText
   - ESPAsyncWebServer
   - WebSocketsServer

3. Compile and upload using Arduino IDE or `arduino-cli`:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:d32 ESP32Matrix.ino
   arduino-cli upload -p <PORT> --fqbn esp32:esp32:d32 ESP32Matrix.ino
   ```

4. On first boot the IP address scrolls across the matrix. Open that address in a browser to control the device.

## Controls

The web UI has buttons for up/down/left/right, a menu button, and a reset button. The same inputs work from any WebSocket client sending single characters: `w` `a` `s` `d` `m` `r`.
