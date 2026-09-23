# TODO — planning, sprints and backlog

Single planning file (user decision 2026-09-23). It serves as roadmap, active sprint and backlog.
Spec: [SPEC.md](SPEC.md). Agent rules and ledger: [AGENTS.md](AGENTS.md).

## Current position

**Active sprint:** Sprint 03, in progress (opened 2026-09-23)
**Current ticket:** T-17 (`in-progress` — Web UI: embedded single page using the API)
**Last completed:** T-16 (`done` — commit `c481023`)

## Conventions

- Tickets are 1–4 h. Status values: `todo` / `in-progress` / `done`.
- Commits per ticket: `T-XX: start`, `T-XX: <title>`, `T-XX: close`. No attribution trailer.
- DoD: `pio test -e native` passes, `pio run -e s3zero` builds, and docs are updated
  (or marked N/A). Hardware behaviour counts as **unverified** until the user flashes it.

## Roadmap

| Sprint | Goal |
|---|---|
| 01 | Skeleton: project builds for the S3; the layout maps to the strip; one effect renders |
| 02 | All 7 effects, plus auto-cycle with crossfade |
| 03 | WiFi (captive portal, mDNS), NVS settings, web UI, OTA |

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

## Sprint 03 — connectivity

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
| T-17 | Web UI: embedded single page using the API | 3h | T-16 | in-progress | Backlog row 3 |
| T-18 | OTA: ArduinoOTA (`owl.local`) + `POST /update` .bin upload | 2h | T-15, T-16 | todo | Backlog row 4 |

## Backlog

Unscheduled, in rough priority order. Sprint 03 draws from here.

- ~~**Settings model + NVS persistence, debounced 5 s**~~ — **CLOSED, verified 2026-09-23 (Sprint 03 T-13/T-14):** `lib/owl/src/owl/settings.h`, `src/settings_store.cpp`, `src/app.cpp` (`saver.due` → `store::save`). Original row kept for the record: Settings model + NVS persistence, debounced 5 s (SPEC §Behaviour)
- ~~**WiFi: WiFiManager captive portal `Owl-Setup`, STA, mDNS `owl.local`**~~ — **CLOSED, verified 2026-09-23 (Sprint 03 T-15):** `src/net.cpp` (`startConfigPortal(SETUP_AP)`, `MDNS.begin(HOSTNAME)`), policy `lib/owl/src/owl/wifi_fsm.h`. Original row kept for the record: WiFi: WiFiManager captive portal `Owl-Setup`, STA, mDNS `owl.local` (SPEC §Connectivity)
- **Web UI + JSON API: on/off, effect, auto-cycle, interval, fade, brightness, speed, colour** (SPEC §Connectivity)
- **OTA: ArduinoOTA + `.bin` upload endpoint** (SPEC §Connectivity)
- **Real layout values in `layout.h`** (blocked: user must place the strip first)

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
