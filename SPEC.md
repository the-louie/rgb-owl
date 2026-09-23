# RGB Owl Light Sign — Specification

3D-printed flat stylised owl, backlit by an RGB LED strip mounted on the back,
facing forward through the white parts.

## Hardware

| Item | Decision |
|---|---|
| MCU | Waveshare ESP32-S3-Zero (ESP32-S3FH4R2: 4 MB flash, 2 MB PSRAM) |
| LEDs | WS2812B, 5 V, GRB colour order |
| LED power | Separate 5 V 10 A PSU; not through the MCU board |
| MCU power | USB-C, or 5 V pad from the LED PSU (3.7–6 V accepted, onboard ME6217 LDO) |
| Data pin | **GPIO1** (header pin next to 3V3/GND) |
| Level shifting | None — 3.3 V direct to strip DIN (user decision) |
| Common ground | PSU GND ↔ MCU GND is mandatory |

### Pin selection rationale (ESP32-S3 datasheet, Zero schematic)
- Avoided strapping pins GPIO0, 3, 45, 46.
- Avoided GPIO19/20 (native USB D-/D+), GPIO21 (onboard WS2812), GPIO43/44 (UART0), GPIO33–37 (octal PSRAM).
- GPIO1 is on the edge header, no boot function, RMT-capable like any GPIO.

### Wiring notes
- Keep the MCU → first-LED data wire short (<30 cm); 3.3 V is below the WS2812B
  VIH spec (0.7 × VDD = 3.5 V) so it is not guaranteed. Fallback if flicker: 74AHCT125 buffer or sacrificial first pixel.
- Recommended: 330 Ω series resistor on data, 1000 µF across 5 V/GND at the strip.

## LED layout

- ~6 columns, up to 15 LEDs each, max ~90 LEDs total.
- Zig-zag, starting **bottom-right**: column 0 is rightmost and runs **up**,
  column 1 runs **down**, alternating, progressing leftwards.
- Columns evenly spaced; each column has its own LED count and **bottom Y offset** (ears/feet).
- Defined at compile time in `include/layout.h`, e.g.:
  ```cpp
  // {count, yOffset} from right to left
  constexpr Column COLUMNS[] = {{8,3},{13,1},{15,0},{15,0},{13,1},{8,3}};
  constexpr ColumnLed EYES[] = {{2,11},{3,11}}; // {column, row from column bottom} — placeholder
  ```
- Firmware builds an XY → strip-index map; effects render on a virtual grid
  (width = columns, height = max(count + yOffset); x = 0 leftmost, y = 0 bottom);
  cells without an LED are skipped. Mapping is `constexpr` (`lib/owl/src/owl/layout_map.h`).

## Effects

1. Pastel plasma
2. Matrix green rain
3. Flame
4. Aurora / northern lights (pastel)
5. Owl eyes (glow + occasional blink; eye LEDs from `EYES[]`)
6. Rainbow sweep + twinkle
7. Breathing solid pastel colour

Palette bias: pastel (desaturated) colours where the effect allows.

## Behaviour

- Boot: column-walk test pattern lights LEDs one by one in wiring order (40 ms each),
  hue per column, to check `layout.h` against the hardware.
- Auto-cycle through effects, fixed order, every 60 s, 2 s crossfade (both adjustable).
- Power limit: FastLED `setMaxPowerInVoltsAndMilliamps(5, 8000)` (configurable; 10 A PSU).
- Settings persisted to NVS (Preferences), written ~5 s after last change.

## Connectivity

- Joins home WiFi; hostname/mDNS `owl.local`.
- No stored credentials / no connection within 15 s → open AP `Owl-Setup` with captive portal
  (WiFiManager, non-blocking so LEDs keep running). Portal closes after 5 min and home WiFi is
  retried, so a router that boots slower than the owl never leaves it stuck in setup mode.
- Web UI (embedded single page): on/off, effect select, auto-cycle on/off, cycle interval,
  fade time, brightness, speed, breathing colour.
- OTA: ArduinoOTA (`pio run -t upload --upload-port owl.local`) and `.bin` upload in web UI.

## Software

- PlatformIO, `espressif32` platform, Arduino framework, FastLED (RMT driver).
- Board: `esp32-s3-devkitc-1` overridden to 4 MB flash, USB CDC on boot.
- Partition table `min_spiffs.csv`: two 1.9 MB OTA app slots, 128 KB SPIFFS (unused).
- Host unit tests: `[env:native]` with Unity; hardware-independent logic lives in `lib/`.
- Toolchain verified 2026-09-23: FastLED smoke build OK (RAM 7.9 %, flash 53 %).

## Reference docs

Downloaded to `__docs/` (gitignored): ESP32-S3 datasheet, TRM, HW design guidelines,
ESP32-S3-Zero schematic/pinout/wiki, WS2812B / SK6812 datasheets, SN74AHCT125 /
SN74LVC1T45 datasheets, ESP-IDF RMT & GPIO pages, FastLED README.

## Open items

- Final column counts, Y offsets and eye positions — fill in `layout.h` once the strip is placed.
