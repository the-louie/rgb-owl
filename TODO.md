# TODO — planning, sprints and backlog

Single planning file (user decision 2026-09-23). It serves as roadmap, active sprint and backlog.
Spec: [SPEC.md](SPEC.md). Agent rules and ledger: [AGENTS.md](AGENTS.md).

## Current position

**Active sprint:** Sprint 05, in progress (opened 2026-09-24)
**Current ticket:** T-36 (`todo` — App Settings: sliders, effect toggles, night schedule, GitHub project URL (`project` setting on the owl))
**Last completed:** T-35 (`done` — commit `a9043bc`)

## Conventions

- Tickets are 1–4 h. Status values: `todo` / `in-progress` / `done`.
- Commits per ticket: `T-XX: start`, `T-XX: <title>`, `T-XX: close`. No attribution trailer.
- DoD: `pio test -e native` passes, `pio run -e s3zero` builds, the app's JVM tests pass and
  `assembleDebug` succeeds (once `android/` exists), and docs are updated
  (or marked N/A). Hardware behaviour counts as **unverified** until the user flashes it.

## Roadmap

| Sprint | Goal |
|---|---|
| 01 | Skeleton: project builds for the S3; the layout maps to the strip; one effect renders |
| 02 | All 7 effects, plus auto-cycle with crossfade |
| 03 | WiFi (captive portal, mDNS), NVS settings, web UI, OTA |
| 04 | Bluetooth control: secrets, GATT server, bonding; Android app skeleton + Main screen |
| 05 | v2 WiFi model (app-provisioned, boot window, debug mode), boot status display, app Settings + Developer |
| 06 | GitHub-release updates (auto + app), rollback, app self-update, public repo + release tooling |

## Sprint 01 — skeleton (done 2026-09-23)

**Goal:** Flashable firmware that walks the layout and then shows pastel plasma.
**Demo:** `pio run -e s3zero` builds and `pio test -e native` passes. On hardware, a
column-walk test pattern confirms the zig-zag layout, then plasma runs.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-01 | PlatformIO project: `s3zero` + `native` envs, 4 MB dual-OTA partitions, `main.cpp` stub | 1h | — | done | SPEC §Software; Landed in `31a4091` |
| T-02 | `layout.h` + XY→index map (constexpr, zig-zag from bottom-right) + native tests | 3h | T-01 | done | SPEC §LED layout; Landed in `a843a77` |
| T-03 | LED driver: FastLED on GPIO1, GRB, 8 A power cap, fixed-FPS render loop, column-walk test pattern | 2h | T-02 | done | SPEC §Hardware; Landed in `8799738` |
| T-04 | Effect interface + registry, hardware-independent where possible + native tests | 2h | T-02 | done | Landed in `6dead73` |
| T-05 | Pastel plasma effect | 2h | T-03, T-04 | done | SPEC §Effects 1; Landed in `0b53a8f` |

## Sprint 02 — effects + auto-cycle (done 2026-09-23)

**Goal:** All 7 SPEC effects render and auto-cycle with a crossfade.
**Demo:** `pio run -e s3zero` builds and `pio test -e native` passes (scheduler/crossfade tests).
On hardware, after the boot walk the owl cycles through 7 effects, 60 s each, with 2 s fades.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-06 | Matrix green rain | 2h | — | done | SPEC §Effects 2; Landed in `c054c58` |
| T-07 | Flame (per-column heat, Fire2012-style, grid-aware) | 3h | — | done | SPEC §Effects 3; Landed in `0031e53` |
| T-08 | Aurora (pastel noise bands) | 2h | — | done | SPEC §Effects 4; Landed in `0abe358` |
| T-09 | Rainbow sweep + twinkle | 2h | — | done | SPEC §Effects 6; Landed in `a7a6c33` |
| T-10 | Breathing solid pastel colour | 1h | — | done | SPEC §Effects 7; Landed in `4a67765` |
| T-11 | Owl eyes (glow + random blink, `EYES[]`) | 2h | — | done | SPEC §Effects 5; Landed in `d424447` |
| T-12 | Auto-cycle scheduler + crossfade (60 s / 2 s, adjustable) + native tests | 3h | — | done | SPEC §Behaviour; Landed in `74b8910` |

## Sprint 03 — connectivity (done 2026-09-23)

**Goal:** Control the owl from a phone. Settings survive power loss, and firmware updates go over WiFi.
**Demo:** gate passes (settings/debounce tests). On hardware: first boot opens the `Owl-Setup`
portal; after joining WiFi, `http://owl.local` changes effect, brightness and speed live; the settings
survive a power cycle; `pio run -t upload --upload-port owl.local` works.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-13 | Settings model: fields, defaults, clamping, `apply(key, value)` from strings + native tests | 2h | — | done | Backlog row 1; Landed in `507e43e` |
| T-14 | NVS persistence (Preferences), 5 s debounce (logic in lib, tested); settings drive brightness/speed/cycle/on-off/breathing hue | 3h | T-13 | done | Backlog row 1; Landed in `6e13b03` |
| T-15 | WiFi: WiFiManager non-blocking portal `Owl-Setup`, STA, hostname + mDNS `owl.local` | 3h | — | done | Backlog row 2; Landed in `5ac9b51` |
| T-16 | HTTP API (WebServer): `GET /api/state`, `POST /api/state` (form params), `GET /api/effects` | 2h | T-14, T-15 | done | Backlog row 3; Landed in `c481023` |
| T-17 | Web UI: embedded single page using the API | 3h | T-16 | done | Backlog row 3; Landed in `db3819c` |
| T-18 | OTA: ArduinoOTA (`owl.local`) + `POST /update` .bin upload | 2h | T-15, T-16 | done | Backlog row 4; Landed in `893b1e6` |

## Sprint 04 — Bluetooth control (done 2026-09-24)

**Goal:** Control the owl from the Android app over bonded BLE, with no WiFi involved.
**Demo:** gate passes (firmware native tests + app JVM tests + both builds). On hardware: the phone
pairs with the PIN, sees the owl's state, and switches effect/on-off; after an app restart it reconnects.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-19 | Secrets to gitignored `secrets.ini` (`extra_configs`) + committed `secrets.ini.example`; OTA password + BLE PIN | 1h | — | done | SPEC v2 §Release; Landed in `5a0f77d` |
| T-20 | `OWL_VERSION` from `git describe` into build + `/api/debug`; enable the 2 MB PSRAM (qio_qspi) for BLE+WiFi+TLS heap | 2h | — | done | Landed in `637f569` |
| T-21 | Protocol lib: `verb k=v&k=v` URL-decoded parser, JSON event builder, semver compare + native tests | 3h | — | done | SPEC v2 §BLE; Landed in `faf35bf` |
| T-22 | NimBLE GATT server: `state`/`command`/`event`/`effects`, bonding with static passkey, auth-required; settings verbs via `app::set` | 4h | T-19, T-21 | done | check NimBLE version vs core 2.0.17; Landed in `699ad3c` |
| T-23 | BOOT held 5 s → clear bonds + WiFi creds (hold detection in lib, tested) | 2h | T-22 | done | Landed in `177e2a4` |
| T-27 | Power cap 4000 mA + FastLED colour correction/gamma hook (tuned later on hardware) | 1h | — | done | SPEC v2 §Improvements; Landed in `3b6db73` |
| T-24 | Android project `android/`: wrapper, Compose, minSdk 31, JVM tests, `tools/build-app.sh` → `dist/owl-app.apk`; gate docs | 2h | — | done | memory-limited Gradle settings; Landed in `c25c839` |
| T-25 | App BLE layer: scan by service UUID, bond (system PIN dialog), GATT client, remember owl + auto-reconnect; protocol codec + JVM tests | 4h | T-22, T-24 | done | Landed in `9ddbbcb` |
| T-26 | App Main screen: on/off, effect grid (selected + shown), auto-cycle | 3h | T-25 | done | Landed in `f748d28` |

## Sprint 05 — WiFi v2, boot status, settings

**Goal:** Everything except updates is configured from the app, and WiFi is only up in the boot
window or debug mode.
**Demo:** gate passes. On hardware: the boot shows the status phases; WiFi is set up from the app
(scan → Test → Save); after 3 min WiFi goes off unless debug mode is on; the Settings and Developer
screens work; the night schedule turns the LEDs off.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-28 | WiFi v2 core: drop WiFiManager/portal; creds in own NVS; boot connect (15 s) → 3 min window → off; policy FSM in lib + tests | 4h | — | done | SPEC v2 §WiFi; Landed in `a17eb98` |
| T-29 | BLE WiFi verbs: `wifi_scan`, `wifi_test` (≤15 s, reason), `wifi_save` (only after a passing test), `wifi_forget`; WiFi fields in state | 4h | T-28 | done | Landed in `ea83ff2` |
| T-30 | Debug mode: `devmode` verb + `POST /api/devmode` (OTA password); window serves only `/api/debug` + `/api/devmode`; debug mode = full HTTP + ArduinoOTA; not persisted | 3h | T-28 | done | Landed in `dd83621` |
| T-31 | Boot status LED phases (BLE, WiFi, update placeholder); phase sequencer in lib + tests; boot walk removed (test pattern stays) | 3h | T-28 | done | SPEC v2 §Boot status; Landed in `c5d50df` |
| T-32 | New-phone pairing only in the first 3 min after boot | 2h | — | done | Landed in `afff6b1` |
| T-33 | Auto-cycle effect selection: `cycle` bitmask setting, Cycler skips disabled; tests | 3h | — | done | Landed in `628a76b` |
| T-34 | Clock (`time epoch=&tz=` verb, NTP in WiFi windows) + night schedule (off between hours) in lib + tests | 4h | T-28 | done | Landed in `0f27f7a` |
| T-35 | Crash info: last panic/WDT reason + time in NVS → `/api/debug` + `debug` verb | 1h | — | done | Landed in `a9043bc` |
| T-36 | App Settings: sliders, effect toggles, night schedule, GitHub project URL (`project` setting on the owl) | 4h | T-33, T-34 | todo | |
| T-37 | App WiFi section: owl scan list + manual SSID, password, Test, Save gated on a pass | 3h | T-29, T-36 | todo | |
| T-38 | App Developer screen (hidden): debug toggle, IP, debug fields, test patterns | 3h | T-30, T-35 | todo | |
| T-39 | App pushes phone time + timezone on every connect | 1h | T-34 | todo | |

## Sprint 06 — updates + release (sketch)

- Firmware: GitHub latest-release lookup (HTTPS, cert bundle), semver compare, download
  `owl-firmware.bin` with progress events, auto-install at boot + every 24 h, rollback (valid after BLE up 60 s).
- App: firmware section (version, check, install, progress), self-update from `owl-app.apk`.
- Pre-release channel: ignored unless in debug mode.
- Signed firmware: signing key, signature appended in the release build, verified by the owl for GitHub updates.
- GitHub Actions: test + build on push; on a tag, sign firmware + APK (keystore as a CI secret) and publish the release.
- Release: scrub the OTA password from git history, create the public GitHub repo (user confirms
  name, after `gh auth login`), `tools/release.sh` tag → build → `gh release` with 3 assets.

## Backlog

Unscheduled, in rough priority order. Sprint 03 draws from here.

- ~~**Settings model + NVS persistence, debounced 5 s**~~ — **CLOSED, verified 2026-09-23 (Sprint 03 T-13/T-14):** `lib/owl/src/owl/settings.h`, `src/settings_store.cpp`, `src/app.cpp` (`saver.due` → `store::save`). Original row kept for the record: Settings model + NVS persistence, debounced 5 s (SPEC §Behaviour)
- ~~**WiFi: WiFiManager captive portal `Owl-Setup`, STA, mDNS `owl.local`**~~ — **CLOSED, verified 2026-09-23 (Sprint 03 T-15):** `src/net.cpp` (`startConfigPortal(SETUP_AP)`, `MDNS.begin(HOSTNAME)`), policy `lib/owl/src/owl/wifi_fsm.h`. Original row kept for the record: WiFi: WiFiManager captive portal `Owl-Setup`, STA, mDNS `owl.local` (SPEC §Connectivity)
- ~~**Web UI + JSON API: on/off, effect, auto-cycle, interval, fade, brightness, speed, colour**~~ — **CLOSED, verified 2026-09-23 (Sprint 03 T-16/T-17):** `src/http.cpp` (`/`, `/api/state`, `/api/effects`), `web/index.html`; headless-Chrome smoke test against a mock API: 7 effect buttons, 3 POSTs, no JS errors, no horizontal scroll at 390 px. Original row kept for the record: Web UI + JSON API: on/off, effect, auto-cycle, interval, fade, brightness, speed, colour (SPEC §Connectivity)
- ~~**OTA: ArduinoOTA + `.bin` upload endpoint**~~ — **CLOSED, verified 2026-09-23 (Sprint 03 T-18):** `src/ota.cpp` (`ArduinoOTA.begin`, `/update` handler), `[env:s3zero-ota]` in `platformio.ini`. Original row kept for the record: OTA: ArduinoOTA + `.bin` upload endpoint (SPEC §Connectivity)
- ~~**OTA / web UI authentication**~~ — **CLOSED, verified 2026-09-23 (user decision):** OTA password in `platformio.ini` `[owl] ota_password`, enforced in `src/ota.cpp` (`ArduinoOTA.setPassword`, `/update` → 401); web UI stays open. Original row kept for the record: OTA / web UI authentication (needs user decision)
- ~~**Real layout values in `layout.h`**~~ — **CLOSED, verified 2026-09-23:** 62 LEDs / 6x12 in `include/layout.h`, pinned by `test_config_matches_measured_owl`; flashed, `/api/debug` reports `"leds":62,"grid":"6x12"`. Original row kept for the record: Real layout values in `layout.h` (blocked: user must place the strip first)
- **`gh auth login` + repo name** (needs the user; blocks Sprint 06 release tooling)
- **Final eye LEDs** (provisional 6+13 right, 35+51 left; user confirms once the owl is mounted)
- **Hardware bring-up + effect tuning** (needs flashed hardware: 3.3 V data reliability, WiFi/RMT flicker, effect speeds and colours, power cap)

## Retros

### Sprint 01 (2026-09-23)

- **Demo:** 15 native tests pass; the s3zero build fits (RAM 8.1 %, flash 35.7 % of 1.9 MB).
  On hardware, the boot column walk and pastel plasma are **unverified** (not flashed yet).
- **Worked:** the constexpr layout map is fully host-tested, and a mapping error fails at compile time.
- **Didn't:** FastLED 3.10.5 won't build on host, so effect rendering can't be unit-tested.
  Mitigation: keep the maths that can be tested (scheduler, fade weights) in `lib/owl`.
- **Backlog:** 0 rows closed (Sprint 01 came from SPEC, not backlog rows), 0 added. Open: 4 plus 1 blocked.
- **Next:** Sprint 02 effects. The column walk becomes the tool for filling in the real `layout.h`.

### Sprint 02 (2026-09-23)

- **Demo:** 28 native tests pass. The s3zero build uses RAM 8.4 % and flash 36.2 %. The 7 effects
  cycle every 60 s with a 2 s crossfade after the boot walk. Visuals are **unverified on hardware**.
- **Worked:** logic that can be tested (blink envelope, cycler) lives in `lib/owl`, with 13 new tests.
  One file per effect in `src/effects/`.
- **Didn't:** effect visuals are tuned blind (speeds, colours, thresholds). Expect a tuning pass once flashed.
- **Backlog:** 0 closed, 0 added. The 4 open rows are verified still open (no WiFi/NVS/OTA code in `src/`)
  and scheduled as Sprint 03. `layout.h` values are still blocked on the user.
- **Next:** Sprint 03 connectivity. The breathing hue needs a setter reachable from settings.

### Sprint 03 (2026-09-23)

- **Demo:** 44 native tests pass. The s3zero and s3zero-ota builds use RAM 17.3 % and flash 66.1 %.
  The web UI was checked in headless Chrome against a mock API. WiFi, portal, NVS and OTA are **unverified on hardware**.
- **Worked:** the policy logic (settings, debounce, WiFi state machine, JSON) is host-tested, so the
  hardware glue in `src/` stays thin.
- **Didn't:** the WiFi stack costs +29 % flash. It fits only because T-01 chose `min_spiffs.csv`.
- **Backlog:** 4 rows closed (settings, WiFi, web UI/API, OTA). 2 rows added (OTA auth
  decision, hardware bring-up). Open: 3, and all of them need the user.
- **Next:** stop. The remaining work needs the user: flash the hardware, place the strip, decide on auth.

### Sprint 04 (2026-09-24)

- **Demo:** 65 native + 13 JVM tests pass. Firmware (0.0.0-dev) runs on the owl: BLE advertising, PSRAM
  2 MB, 62 fps, 4 A cap. APK in `dist/owl-app-debug.apk`. **Phone pairing/control is unverified**: this
  host has no BLE adapter, so the user has to try it.
- **Worked:** the protocol, semver and hold logic are host-tested. Checking the owl over WiFi after each
  OTA caught nothing broken.
- **Didn't:** the BLE end-to-end path can't be tested here. Sprint 05 builds on it before any phone test,
  which is a risk.
- **Backlog:** 0 rows closed/added (Sprint 04 came from the SPEC). Open: `gh auth` + repo name, final eye LEDs.
- **Next:** Sprint 05. Once WiFi goes off by default, the dev loop uses the boot window:
  OTA → reboot → `POST /api/devmode` within 3 min.
