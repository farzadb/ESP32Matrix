# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

16×16 WS2812B LED matrix console running on ESP32. Supports Tetris, Snake, Breakout, and a Pixel Art viewer, controlled via WebSocket from a browser.

## Build & Flash

This is an **Arduino sketch** project. Use Arduino IDE or `arduino-cli`:

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32 ESP32Matrix_ws.ino

# Upload (replace /dev/ttyUSB0 with your port)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 ESP32Matrix_ws.ino

# Monitor serial output
arduino-cli monitor -p /dev/ttyUSB0 --config baudrate=115200
```

Required libraries (install via Arduino Library Manager): FastLED, LEDMatrix, LEDSprites, LEDText, ESPAsyncWebServer, WebSocketsServer.

## Architecture

### Main Loop State Machine (`ESP32Matrix_ws.ino`)

The `.ino` file owns all global state and the main event loop. It initializes WiFi, an async HTTP server (port 80), a WebSocket server (port 81).

The loop is a switch on `currentApp` (enum: MENU, TETRIS, SNAKE, BREAKOUT, PIXEL). Each iteration calls the active app's `loop()`, which returns `false` to signal exit back to menu.

Input arrives from WebSocket messages and is normalized into a single `currentInput` (`btnInput` enum: UP/DOWN/LEFT/RIGHT/MENU/NONE). Apps read `currentInput` directly — there is no input queue.

### App Interface Contract

Every app (`Menu.h`, `Tetris.h`, `Snake.h`, `Breakout.h`, `Pixel.h`) follows this pattern:

```cpp
void setupAppName();          // Called once on app entry — clears matrix, resets state
bool loopAppName();           // Called every frame — returns false to exit to menu
void exitAppName();           // Called on exit — cleanup
```

Apps render directly to the global `leds` matrix object (`cLEDMatrix<16, 16, HORIZONTAL_ZIGZAG_MATRIX>`), then the main loop calls `FastLED.show()`.

### Shared Libraries

- **FastLED** — LED color control; `leds` is the global matrix object
- **LEDMatrix** — 2D indexing abstraction over FastLED
- **LEDSprites / cSprite** — Used by Breakout for ball, paddle, and block sprites
- **LEDText / cLEDText** — Used by Menu for scrolling text; font data in `FontMatrise.h`

### Web Control Interface

The full HTML/JS control page is stored as a string in `html.h` and served at `/`. WebSocket messages are single characters: `W` (up), `A` (left), `S` (down), `D` (right), `M` (menu). 

### Sprite Assets

Pixel art sprite data is embedded in header files under `sprites/`. Sprites use 2-bit color encoding with separate mask data. To add a new sprite, add its packed binary data and mask to the appropriate header.

## Key Constants (in `ESP32Matrix_ws.ino`)

| Constant | Value | Purpose |
|---|---|---|
| `DATA_PIN` | 2 | GPIO pin for LED data |
| `MATRIX_WIDTH/HEIGHT` | 16 | Matrix dimensions |
| `BRIGHTNESS` | 32 | FastLED max brightness (0–255) |
| `MAX_POWER_MILLIAMPS` | 2000 | Power budget |

## Adding a New App

1. Create `MyApp.h` with `setupMyApp()`, `bool loopMyApp()`, `exitMyApp()`
2. `#include "MyApp.h"` in `ESP32Matrix_ws.ino`
3. Add `MYAPP` to the `appId` enum
4. Add a case in the main `switch(currentApp)` loop
5. Add a menu entry in `Menu.h`
