# AGENTS.md — RGB Owl Light Sign

For all commands assume '/lr', unless explicitly stated 'be verbose'

Guidance for any coding agent working in this repo. Read this first; load the
referenced docs only when the task needs them.

## Project in one paragraph

Firmware for a 3D-printed, flat, stylised owl light sign. A WS2812B strip on the
back shines forward through the white parts. An ESP32-S3-Zero drives it with pastel
effects (plasma, matrix rain, flame, aurora, owl eyes, rainbow + twinkle, breathing).
It has a WiFi web UI, OTA updates and auto-cycling with crossfades. The full spec is
in `SPEC.md`, and it is the source of truth for behaviour.

## Working rules

1. **Decisions are the user's.** When a requirement is ambiguous or a design choice is
   open, ask in interview form (multiple-choice, recommended option first). Don't guess.
   Record every answer in the ledger below and update `SPEC.md` if it changes.
2. **Smallest change that fixes the ticket.** Don't add unrequested features.
3. **Commits:** only when asked, or inside the sprint loop, which is standing permission.
   The loop makes 3 commits per ticket (`T-XX: start` / `T-XX: <title>` / `T-XX: close`)
   as The Louie, with no attribution trailer.
4. **Gate:** `pio test -e native` must pass, `pio run -e s3zero` must build, and
   `tools/build-app.sh` must pass for app changes, before work counts as done. Hardware behaviour can't be verified here, so say so.
5. Keep `SPEC.md` (what), `AGENTS.md` (how and why, ledger) and the code consistent.
6. Answers to the user: outcome first, short, and numbers instead of adjectives.
7. **Planning / backlog:** `TODO.md` (roadmap, active sprint, backlog, retros in one file).

## Hard constraints (do not change without asking the user)

| Topic | Constraint | Why |
|---|---|---|
| MCU | Waveshare ESP32-S3-Zero, ESP32-S3FH4R2, **4 MB flash**, 2 MB PSRAM (quad) | User choice |
| LED data pin | **GPIO1** | See pin rules below |
| LED type | WS2812B, 5 V, GRB | Interview 2026-09-23 |
| Level shifting | None. 3.3 V goes directly to DIN | User choice; accepted risk |
| LED power | Separate 5 V 10 A PSU; firmware current cap 4000 mA (62 LEDs) | Interview 2026-09-24 |
| Layout | Compile-time `include/layout.h` | Interview |
| Stack | PlatformIO + Arduino + FastLED | Interview; build verified |
| App | Android only, Kotlin + Jetpack Compose, minSdk 31, in `android/` | Interview 2026-09-24 |
| Control | BLE (bonded, static PIN); WiFi only for updates, 3-min boot window, debug mode | Interview 2026-09-24 |
| Secrets | OTA password + BLE PIN in gitignored `secrets.ini`, never in git (repo goes public) | Interview 2026-09-24 |

### Pin rules (from the ESP32-S3 datasheet and the Zero schematic)
- **Never use** GPIO0, 3, 45, 46 (strapping), GPIO19/20 (native USB D-/D+),
  GPIO21 (onboard WS2812 status LED), GPIO43/44 (UART0 TX/RX),
  GPIO33–37 (not broken out; used by PSRAM), or GPIO47/48 (not broken out).
- Edge header pins: 5V, GND, 3V3, GPIO1–13, TX(43), RX(44).
  GPIO14–18 are bottom pads only (for soldering or pogo pins).
- GPIO0 = BOOT button, read at runtime for the 5 s recovery reset (input only; never drive it).
- Free for future use: GPIO2, 4–13. GPIO21 can serve as a status LED (onboard WS2812).
- 5 V pad input range is 3.7–6 V. The onboard LDO is an ME6217C33M5G (800 mA max).
  Never route LED current through the board.
- Flashing: there is no USB-UART bridge, so hold BOOT (GPIO0) while plugging in USB-C
  the first time. After that, OTA or USB CDC work.

## LED layout model

- Columns are counted **from right to left**. Column 0 is the rightmost column.
- Zig-zag wiring: column 0 runs **bottom → top**, column 1 runs top → bottom, and so on,
  alternating.
- Each column is `{count, yOffset}`, where yOffset is the height of its bottom LED above
  grid row 0. Columns are evenly spaced in X.
- Virtual grid: width = number of columns, height = max(count + yOffset). Effects
  draw in XY, and a lookup table maps each XY cell to a strip index, or −1 where there
  is no LED.
- Measured owl (2026-09-23): 6 columns, 62 LEDs, grid 6×12; see `include/layout.h`.
- Eye LEDs are `(column,row)` pairs in `layout.h`: strip LEDs 6+13 (right), 35+51 (left),
  **provisional** until the owl is mounted.

## Adding an effect

Subclass `owl::Effect` (`src/effect.h`), draw with `Canvas::set(x, y, c)` into the buffer
passed to `render()` (never into `leds::strip` directly, because crossfade renders two
effects), use `Frame::t` / `Frame::dt` (speed-scaled ms) instead of `millis()`, and add the
instance (one file per effect in `src/effects/`, accessor in `src/effects/effects.h`) to `ALL[]` in `src/effects.cpp` (array order = auto-cycle order).

## Android build

- JDK 17: `~/.local/jdk/current`. Android SDK: `~/.local/android-sdk` (platform 35, build-tools 35/34).
  Gradle 8.11.1: `~/.local/gradle-8.11.1` (projects use the wrapper). Installed 2026-09-24.
- `tools/build-app.sh` runs the JVM tests + `assembleDebug` (sets JAVA_HOME/ANDROID_HOME) and copies
  the APK to `dist/owl-app-debug.apk`. It takes about 3–6 min on this host. App package: `se.louie.owl`.
- Release APK: `tools/build-app.sh release` → `dist/owl-app-release.apk`, signed with `keys/owl-app.jks`
  (passwords in `keys/owl-app.properties`; CI uses `OWL_KEYSTORE_FILE` / `OWL_KEYSTORE_PASSWORD`).
  Cert SHA-256 `ec53962f…ab06`. Every release must use this key.
- App version = firmware scheme (`git describe`); versionCode X·10⁶+Y·10⁴+Z·100+(99 release | N dev),
  computed in `android/app/build.gradle.kts`.
- **Memory:** the host has ~1.5 GB free. With default settings the Gradle daemon is OOM-killed. Keep
  `org.gradle.jvmargs=-Xmx1024m`, `org.gradle.workers.max=1` and
  `kotlin.compiler.execution.strategy=in-process` in `gradle.properties`.
- Smoke build verified (AGP 8.7.3, Kotlin 2.0.21, Compose BOM 2024.12.01): 8.7 MB debug APK, JUnit OK.
- Disk: about 6.9 GB free after the install (`~/.gradle` 1.1 GB).

## Talking to the device

The owl is reachable from the dev host at **10.13.110.163** (`owl.local` may not resolve here).
Since T-28/T-30 the owl is only reachable in the 3-minute boot window or in debug mode.
**`tools/owl-dev.sh 10.13.110.163 flash`** OTA-flashes it (debug mode must be on) and turns debug mode
back on inside the new boot window. If debug mode is off and the window has closed, only the user can
help: a power cycle (then `tools/owl-dev.sh <ip> devmode` within 3 min) or the app's debug toggle.
- Flash: `pio run -e s3zero-ota -t upload --upload-port 10.13.110.163`, then poll `/api/debug` until
  `version` shows the new `git describe` string (from `tools/version.py`).
- Inspect: `curl http://10.13.110.163/api/debug`, `/api/log`, and `/api/frame` (what the effect
  actually rendered; use it before guessing at a visual bug report).
- Test patterns: `curl -d "mode=column&index=0" http://10.13.110.163/api/test` (put back with `mode=none`).
- Never reboot a freshly flashed image before `/api/debug` shows `ota_state: valid` (60 s):
  the bootloader treats that as a failed image and rolls back (2026-09-24, cost 2 false alarms).
- Crashes: `curl …/api/coredump`, then addr2line against the ELF of the *same* build.
- Anything that reboots the owl (crash tests, rollback) turns debug mode off. Keep a poller running
  that POSTs `/api/devmode` every 1–3 s for 5 min, or the owl drops off WiFi when the window closes
  (happened on 2026-09-24 T-40).
  Test patterns are visible to the user on the real sign, so keep them short and always reset to `none`.

## Build and flash

First: `cp secrets.ini.example secrets.ini` and fill in (OTA password, BLE PIN). Without it
every PlatformIO command fails with `No section: 'owl_secrets'` (on purpose).

```sh
pio run                                      # build
pio run -t upload                            # USB (first time: hold BOOT while plugging in)
pio run -e s3zero-ota -t upload              # OTA to owl.local, once firmware is running
pio device monitor                           # USB CDC serial
```

Board config: `board = esp32-s3-devkitc-1` with `board_upload.flash_size = 4MB`,
`-DARDUINO_USB_CDC_ON_BOOT=1 -DARDUINO_USB_MODE=1`, partitions `min_spiffs.csv`
(2 × 1.9 MB OTA slots). `pio test -e native` runs Unity tests in `test/` against `lib/`
(src/ is not built for tests, so keep testable logic Arduino-free in `lib/`).
FastLED does not compile on host (3.10.5 stub layer), so effect rendering stays in `src/`.
Constants: `include/config.h`; layout: `include/layout.h`; user settings + defaults:
`lib/owl/src/owl/settings.h`. Web UI source: `web/index.html` (embedded via
`board_build.embed_txtfiles`, served at `/`). `src/app.cpp` owns the render loop and settings; other
modules change settings only through `app::set(key, value)`.

Installed toolchain (user-level, verified 2026-09-23):
- PlatformIO Core 6.1.19, platform `espressif32` 6.11.0, Arduino core 2.0.17
  (ESP-IDF 4.4), `toolchain-xtensa-esp32s3`, esptool 4.5.1
- arduino-cli 1.5.1 with `esp32:esp32` 3.3.11 (alternative; not the primary build)
- FastLED pinned to **3.9.20**: 3.10.x silently falls back to a bit-banged generic driver on
  Arduino core 2.0.17 / IDF 4.4 (serial warning `Using GENERIC fallback clockless controller`),
  which made every LED white on hardware. Check the serial log/`/api/log` after any FastLED bump.

## Reference documentation index (`__docs/`, gitignored)

`__docs` and `dist` are symlinks to the file server (`/mnt/backup/owl/`, NFS) because the dev host
is short on disk. Throwaway experiments go to `/mnt/backup/owl/scratch-experiments/`.

Load these into context only when needed. If `__docs/` is missing, re-download from the URLs.

| File | Load when | Source |
|---|---|---|
| `ESP32-S3-Zero-Sch.pdf` | Pin mapping, power path, onboard LED/buttons | files.waveshare.com/wiki/ESP32-S3-Zero/ESP32-S3-Zero-Sch.pdf |
| `ESP32-S3-Zero.png`, `ESP32-S3-Zero-2D-size.jpg` | Physical pinout, board dimensions for mounting | Waveshare wiki |
| `waveshare-esp32-s3-zero-wiki.html` | Board FAQ, flashing procedure, power notes | waveshare.com/wiki/ESP32-S3-Zero |
| `esp32-s3_datasheet_en.pdf` | Strapping pins, GPIO electrical specs, pin functions | documentation.espressif.com |
| `esp32-s3_technical_reference_manual_en.pdf` (15 MB, large) | RMT peripheral registers, only for low-level driver issues | documentation.espressif.com |
| `esp32-s3_hardware_design_guidelines_en.pdf` | Power, decoupling, GPIO drive design questions | docs.espressif.com |
| `esp-idf-rmt-esp32s3.html` | RMT driver behaviour (FastLED uses RMT on the S3) | docs.espressif.com ESP-IDF stable |
| `esp-idf-gpio-esp32s3.html` | GPIO restrictions and drive strength | docs.espressif.com ESP-IDF stable |
| `WS2812B.pdf` | LED timing, VIH = 0.7 × VDD, current per LED | cdn-shop.adafruit.com |
| `XL-0807RGBC-WS2812B.pdf` | Onboard status LED on GPIO21 | Waveshare |
| `SK6812_LED_datasheet_.pdf` | Only if the strip type changes to RGBW | cdn-shop.adafruit.com |
| `sn74ahct125.pdf`, `sn74lvc1t45.pdf` | Level-shifter fallback if 3.3 V data flickers | ti.com |
| `FastLED-README.md` | FastLED API, ESP32 driver notes | github.com/FastLED/FastLED |

Useful but not downloaded (fetch on demand): the FastLED docs site
(fastled.io/docs), the WiFiManager README (github.com/tzapu/WiFiManager), and the
PlatformIO espressif32 docs (docs.platformio.org/en/latest/platforms/espressif32.html).

## Known risks

- **3.3 V data to a 5 V WS2812B** is outside spec (it needs ≥3.5 V). Symptoms: random
  flicker, or wrong colours on the first LED. Fix options, in order: shorten the data
  wire (<30 cm), add a 330 Ω series resistor, add a 74AHCT125 buffer, or use a
  sacrificial first pixel. Ask the user before changing the hardware plan.
- WiFi activity can disturb RMT timing. If flicker correlates with WiFi, check the
  FastLED ESP32 RMT settings (e.g. `FASTLED_RMT_MAX_CHANNELS`, and pinning the LED task
  to core 1).
- Disk space on the dev host is 88 % used (13 GB free). Avoid installing extra
  toolchains such as a full ESP-IDF unless needed.

## Ledger

Append-only. Newest entries go at the bottom. Format: `date | type | entry`.
Types: DECISION, EVENT, OPEN, CLOSED.

```
2026-09-23 | EVENT    | Reference docs downloaded to __docs/ (15 files); __docs/ in .gitignore; git init (main)
2026-09-23 | EVENT    | Toolchain verified: PlatformIO + espressif32 6.11.0 FastLED smoke build OK; nothing installed
2026-09-23 | DECISION | MCU = Waveshare ESP32-S3-Zero (user's choice, no reason to deviate)
2026-09-23 | DECISION | LED data pin = GPIO1 (agent decision per datasheet/schematic; rules above)
2026-09-23 | DECISION | Strip = WS2812B 5V GRB
2026-09-23 | DECISION | Level shifting = none, 3.3V direct (user accepted risk)
2026-09-23 | DECISION | Stack = PlatformIO + Arduino + FastLED
2026-09-23 | DECISION | Control = WiFi web UI + auto-cycle (no physical button, no Home Assistant)
2026-09-23 | DECISION | Layout = compile-time include/layout.h; ~6 cols x <=15 LEDs; zig-zag from bottom-right, col 0 up
2026-09-23 | DECISION | Geometry = even column spacing, per-column bottom Y offset
2026-09-23 | DECISION | PSU = 5V 10A separate; firmware cap 8000 mA
2026-09-23 | DECISION | WiFi = home STA + AP fallback captive portal "Owl-Setup" (WiFiManager); mDNS owl.local
2026-09-23 | DECISION | Effects = pastel plasma, matrix rain, flame, aurora, owl eyes, rainbow+twinkle, breathing
2026-09-23 | DECISION | Auto-cycle = fixed order, 60 s interval, 2 s crossfade, both adjustable in UI
2026-09-23 | DECISION | Settings persisted to NVS, debounced ~5 s
2026-09-23 | DECISION | OTA = ArduinoOTA + .bin upload in web UI
2026-09-23 | DECISION | Eyes = lit from strip, positions in layout.h
2026-09-23 | EVENT    | SPEC.md written; user asked to stop for review before implementation
2026-09-23 | OPEN     | Real column counts, Y offsets, eye positions (after strip placement)
2026-09-23 | OPEN     | User review of SPEC.md -> then implement
2026-09-23 | CLOSED   | User review of SPEC.md: approved by starting implementation (/sprint-superloop)
2026-09-23 | DECISION | Backlog = TODO.md at repo root (single planning file)
2026-09-23 | DECISION | Loop commits: start/work/close per ticket, no trailer
2026-09-23 | DECISION | Superloop runs autonomously, no review stop between sprints
2026-09-23 | DECISION | Gate = native Unity tests + s3zero build; hardware unverified until user flashes
2026-09-23 | DECISION | Partitions = min_spiffs.csv (2x1.9 MB OTA; agent decision: web UI embedded, no FS needed)
2026-09-23 | DECISION | C++17 for s3zero (unflag gnu++11) and native; layout mapping is constexpr in lib/owl
2026-09-23 | DECISION | Grid coords for effects: x=0 leftmost, y=0 bottom; EYES as {column,row-from-column-bottom}
2026-09-23 | DECISION | FastLED 3.10.5 does not build on host (stub layer errors); effects live in src/ (s3zero only), pure logic in lib/owl with native tests
2026-09-23 | EVENT    | T-03: FastLED driver in firmware = 35.7% of 1.9 MB app slot
2026-09-23 | DECISION | WiFi policy: 15 s STA attempt -> Owl-Setup portal for 5 min -> retry STA (agent decision, closes gap: router down at boot)
2026-09-23 | EVENT    | T-15: WiFi stack raises flash to 65.3% of 1.9 MB slot (default.csv 1.25 MB would not fit)
2026-09-23 | OPEN     | OTA/web auth: none for now (anyone on home LAN can reflash); needs user decision
2026-09-23 | EVENT    | Sprints 01-03 done (18 tickets): 7 effects, auto-cycle, NVS, WiFi portal, web UI, OTA; 44 native tests; hardware unverified
2026-09-23 | CLOSED   | OTA auth (user decision): password for ArduinoOTA + web /update, web UI open; single source [owl] ota_password in platformio.ini
2026-09-23 | EVENT    | Hardware bring-up: all LEDs white; cause FastLED 3.10.5 GENERIC fallback driver on IDF 4.4; pinned 3.9.20 (RMT) -> user confirms it works
2026-09-23 | DECISION | Debug API added (/api/debug, /api/log, /api/test, /api/reboot), open like the web UI; device at 10.13.110.163, agent may OTA it
2026-09-23 | DECISION | Layout = measured owl, 62 LEDs, COLUMNS {10,2},{11,1},{11,1},{12,0},{11,1},{7,0}; user-verified via ASCII diagram
2026-09-23 | OPEN     | Eye LEDs provisional (6,13 right; 35,51 left) until owl is mounted
2026-09-24 | EVENT    | Installed JDK 17 (Temurin), Android cmdline-tools + SDK 35 + build-tools 35/34, Gradle 8.11.1 under ~/.local; SDK licences accepted via sdkmanager --licenses; smoke APK OK with 1 GB heap
2026-09-24 | DECISION | Android app: Kotlin + Compose, minSdk 31, Android only; Main/Settings/Developer screens; auto-reconnect; self-update from GitHub release
2026-09-24 | DECISION | BLE control with bonding + static passkey (PIN generated, stored in gitignored secrets.ini)
2026-09-24 | DECISION | WiFi: no captive portal; creds via app with owl-side scan + mandatory passing Test before Save
2026-09-24 | DECISION | Every boot: WiFi on, check GitHub latest release, auto-install if newer, WiFi up 3 min then off; also every 24 h uptime
2026-09-24 | DECISION | Boot window HTTP = /api/debug + /api/devmode (OTA password); debug mode = web UI + debug API + ArduinoOTA, not persisted
2026-09-24 | DECISION | Firmware source = GitHub latest release of project URL set in app (public repo, anonymous HTTPS); auto-install; rollback if new image not valid after BLE up 60 s
2026-09-24 | DECISION | Recovery: BOOT held 5 s clears bonds + WiFi creds
2026-09-24 | DECISION | Repo goes public: scrub OTA password from git history before first push (keep password); secrets move to gitignored secrets.ini
2026-09-24 | OPEN     | User must run 'gh auth login' and confirm repo name before the first push/release
2026-09-24 | DECISION | Boot status = whole-owl phases (BLE, WiFi, update); WiFi failed = rapid red/black blink; shown until update check done +3 s; boot walk moves to test patterns
2026-09-24 | EVENT    | Sprints 04-06 planned in TODO.md; waiting for user go / further instructions
2026-09-24 | DECISION | Improvements: colour correction; power cap 4000 mA; per-effect auto-cycle toggle; night schedule = LEDs off (default 23-07), time via NTP + phone push over BLE
2026-09-24 | DECISION | Safety: pre-release channel (ignored unless debug mode); signed firmware for GitHub updates (debug uploads may be unsigned); GitHub Actions CI builds+signs+publishes on tag; crash info in NVS
2026-09-24 | DECISION | APK keystore + firmware signing key live as CI secrets; user keeps offline backups
2026-09-24 | DECISION | New-phone pairing only in first 3 min after boot
2026-09-24 | EVENT    | Disk: __docs + dist moved to /mnt/backup/owl (symlinked), scratch experiments (474 MB) offloaded; root 20 GB free
2026-09-24 | EVENT    | Firmware signing key generated: keys/owl-signing.pem (gitignored, local only) - user must back it up; public key committed in include/signing_key.h
2026-09-24 | OPEN     | Back up keys/owl-signing.pem offline (losing it = owls can only be updated over USB/debug mode)
2026-09-24 | EVENT    | APK release keystore generated: keys/owl-app.jks + keys/owl-app.properties (gitignored, local only) - user must back them up with keys/owl-signing.pem
```
