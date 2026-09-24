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

- 6 columns, 62 LEDs (measured 2026-09-23): counts 10/11/11/12/11/7 right→left.
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
- Power limit: FastLED `setMaxPowerInVoltsAndMilliamps(5, 4000)` (62 LEDs × 60 mA = 3.7 A; 10 A PSU).
- Settings persisted to NVS (Preferences), written ~5 s after last change.

## Connectivity — v2: Android app over Bluetooth (planned 2026-09-24, Sprints 04–06)

Supersedes the v1 connectivity below once Sprint 05 lands.

### Bluetooth LE (primary control)
- NimBLE GATT server, advertised as `Owl`, custom 128-bit service.
- Security: bonding with a static 6-digit passkey (MITM, LE Secure Connections). Every
  characteristic needs an encrypted, authenticated link. The PIN is in gitignored `secrets.ini`.
- Characteristics: `state` (read/notify, JSON: settings + shown effect + firmware version + update/WiFi
  status), `command` (write, `verb key=value&key=value`, URL-encoded), `event` (notify, JSON replies:
  WiFi scan list, test result, update progress, errors), `effects` (read, JSON names).
- UUIDs in `src/ble.h` (base `4f574c00-8a1b-4c2e-9d3f-2b1a6c7e00xx`: 01 service, 02 state, 03 command,
  04 event, 05 effects). Links that are not authenticated within 30 s are dropped.
- Verbs so far: `get` (push state), `set k=v&…` (Settings keys as in the HTTP API; `effect` takes an
  index or a name). Each command gets an event `{"type":"ok"|"error","verb":…,"msg"?}`. The state JSON is
  the HTTP state plus `"version"`.

### Android app (Kotlin + Jetpack Compose, minSdk 31, Android only)
- **Main:** on/off, effect grid (selected + currently shown), auto-cycle.
- **Settings:** brightness, speed, interval, fade, breathing colour. WiFi: the owl scans and the app
  lists the networks (manual SSID entry too) → password → **Test** (the owl tries to join, ≤15 s, and
  reports OK + RSSI or the reason) → **Save** is enabled only after a passing test. GitHub project URL
  (the owl uses it to find the latest release). Firmware version, **Check**, **Install**, progress.
- **Developer** (hidden): debug-mode toggle, IP, `/api/debug` fields, test patterns.
- Remembers the bonded owl and reconnects on launch. Self-update: offers a newer `owl-app.apk`
  from the same GitHub release.

### WiFi (updates + debugging only)
- There is no captive portal. Credentials are provisioned only from the app (stored in NVS).
- **Every boot:** join WiFi → check the GitHub latest release → install it if newer → keep WiFi up
  for a **3-minute window**, then turn WiFi off unless debug mode is on. Also re-checks every 24 h of
  uptime (WiFi on only for that).
- During the window, HTTP only serves `GET /api/debug` and `POST /api/devmode`
  (`password` = OTA password → 401 otherwise).
- **Debug mode** (from the app or `/api/devmode`) keeps WiFi on with the full web UI, the debug API
  (`/api/log`, `/api/frame`, `/api/test`, …) and ArduinoOTA. It is not persisted: it lasts until
  switched off or the next reboot.

### Improvements (interview 2026-09-24)
- **Colour correction:** FastLED strip correction + gamma, tuned in debug mode on the real owl.
- **Power cap:** 4000 mA (62 LEDs × 60 mA = 3.7 A), replacing 8000 mA.
- **Auto-cycle selection:** each effect can be enabled/disabled in the app; auto-cycle skips disabled
  ones (persisted; all enabled by default).
- **Night schedule:** LEDs **off** between set hours (default 23:00–07:00, editable in app Settings).
  Time comes from NTP during WiFi windows, and the app pushes phone time + timezone over BLE on every
  connect. The schedule is inactive until the clock has been set once.
- **New-phone pairing** only during the first 3 minutes after boot; bonded phones connect any time.
- **Crash info:** last panic/watchdog reason + time kept in NVS, shown in `/api/debug` and the app Developer screen.

### Boot status display (whole-owl phases)
Replaces the boot column walk (the walk moves to the Developer screen / `/api/test mode=walk`).
1. **BLE:** green flash = phone(s) bonded; blue flash = no phone bonded.
2. **WiFi:** steady red = not configured (skip step 3); yellow pulse = connecting; green flash =
   connected; **rapid red/black blinking** = failed to connect.
3. **Update:** purple pulse = checking; green flash = up to date; cyan fill bottom → top =
   downloading (shows % until reboot); red flash = check failed.
Then effects start, 3 s after the update check finishes (typically 5–15 s after power-on).

### Firmware updates
- Source: latest release of the configured GitHub project (public repo; the owl downloads anonymously
  over HTTPS with the certificate bundle). Asset `owl-firmware.bin`. Tag `vX.Y.Z` = firmware version
  (`OWL_VERSION` from `git describe`).
- A newer release installs automatically (at boot and in the daily check), or on demand from the app.
- **Pre-releases** are ignored by the owl unless it is in debug mode (test channel).
- **Signed firmware:** release images carry a signature from a private key; the owl rejects
  unsigned/invalid images for GitHub updates. Debug-mode uploads (ArduinoOTA, web `.bin`) may be
  unsigned (password-protected).
- **Rollback:** a new image must reach "BLE advertising + 60 s up" to be marked valid; otherwise the
  bootloader reverts to the previous image.
- Recovery: hold BOOT for 5 s → forget bonded phones and WiFi credentials, three red flashes, restart
  (implemented T-23).

### Release
- **GitHub Actions CI:** tests + builds on every push; a pushed tag builds, signs and publishes the
  release. CI secrets: OTA password, BLE PIN, firmware signing key, APK keystore (+ passwords). The user
  keeps an offline backup of both keys (a lost APK key means reinstalling the app once).
- A GitHub release per version carries `owl-firmware.bin`, `owl-s3zero-merged.bin` (for the web
  flasher) and `owl-app.apk`.
- Before the repo goes public, the OTA password is scrubbed from git history (user decision:
  rewrite history, keep the password).

## Connectivity — v1 (history: the portal and always-on WiFi were removed in T-28; the HTTP API is debug-mode only since T-30)

- Joins home WiFi; hostname/mDNS `owl.local`.
- No stored credentials / no connection within 15 s → open AP `Owl-Setup` with captive portal
  (WiFiManager, non-blocking so LEDs keep running). Portal closes after 5 min and home WiFi is
  retried, so a router that boots slower than the owl never leaves it stuck in setup mode.
- Web UI (embedded single page): on/off, effect select, auto-cycle on/off, cycle interval,
  fade time, brightness, speed, breathing colour.
- HTTP API (port 80, only while on the home network):
  - `GET /api/state` → `{"on","effect","current","auto","interval","fade","brightness","speed","hue"}`
    (`effect` = selected/saved, `current` = shown now)
  - `POST /api/state` with form fields of the same names (`effect` accepts index or name) → new state,
    or 400 `{"error"}`
  - `GET /api/effects` → effect names in cycle order
- Debug API (open, like the web UI):
  - `GET /api/debug`: version, build, uptime, reset reason, heap, fps, show() time, estimated LED mA vs limit,
    brightness after power limit, effect, test mode, WiFi state/SSID/RSSI/IP
  - `GET /api/frame`: last rendered frame, strip order, `rrggbb` per LED (before brightness/power scaling)
  - `GET /api/log`: last 4 KB of the firmware log (same lines as USB serial)
  - `POST /api/test`: `mode=none|off|solid|pixel|column|row|walk`, optional `r,g,b` and `index`
    (pixel = strip index, column = physical column 0 = rightmost, row = grid y); overrides effects until `none`
  - `POST /api/color`: `correction=RRGGBB&gamma=1.0..3.0` (live tuning, not persisted; bake into `config.h`)
  - `POST /api/reboot`
- OTA: ArduinoOTA (`pio run -e s3zero-ota -t upload`, espota to `owl.local`) and `.bin` upload in
  the web UI (`POST /update`, multipart, `password` field before the file). Both require the OTA
  password (`ota_password` in gitignored `secrets.ini`); the rest of the web UI is open.

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
