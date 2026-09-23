# TODO — planning, sprints and backlog

Single planning file (user decision 2026-09-23). It serves as roadmap, active sprint and backlog.
Spec: [SPEC.md](SPEC.md). Agent rules and ledger: [AGENTS.md](AGENTS.md).

## Current position

**Active sprint:** Sprint 01, in progress (opened 2026-09-23)
**Current ticket:** T-05 (`in-progress` — Pastel plasma effect)
**Last completed:** T-04 (`done` — commit `6dead73`)

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

## Sprint 01 — skeleton

**Goal:** Flashable firmware that walks the layout and then shows pastel plasma.
**Demo:** `pio run -e s3zero` builds and `pio test -e native` passes. On hardware, a
column-walk test pattern confirms the zig-zag layout, then plasma runs.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-01 | PlatformIO project: `s3zero` + `native` envs, 4 MB dual-OTA partitions, `main.cpp` stub | 1h | — | done | SPEC §Software; Landed in `31a4091` |
| T-02 | `layout.h` + XY→index map (constexpr, zig-zag from bottom-right) + native tests | 3h | T-01 | done | SPEC §LED layout; Landed in `a843a77` |
| T-03 | LED driver: FastLED on GPIO1, GRB, 8 A power cap, fixed-FPS render loop, column-walk test pattern | 2h | T-02 | done | SPEC §Hardware; Landed in `8799738` |
| T-04 | Effect interface + registry, hardware-independent where possible + native tests | 2h | T-02 | done | Landed in `6dead73` |
| T-05 | Pastel plasma effect | 2h | T-03, T-04 | in-progress | SPEC §Effects 1 |

## Sprint 02 — effects + auto-cycle (detailed)

| ID | Title | Est | Deps | Notes |
|---|---|---|---|---|
| T-06 | Matrix green rain | 2h | S01 | |
| T-07 | Flame (per-column heat, Fire2012-style, grid-aware) | 3h | S01 | |
| T-08 | Aurora (pastel noise bands) | 2h | S01 | |
| T-09 | Rainbow sweep + twinkle | 2h | S01 | |
| T-10 | Breathing solid pastel colour | 1h | S01 | |
| T-11 | Owl eyes (glow + random blink, `EYES[]`) | 2h | S01 | |
| T-12 | Auto-cycle scheduler + crossfade (60 s / 2 s, adjustable) + native tests | 3h | S01 | |

## Backlog

Unscheduled, in rough priority order. Sprint 03 draws from here.

- **Settings model + NVS persistence, debounced 5 s** (SPEC §Behaviour)
- **WiFi: WiFiManager captive portal `Owl-Setup`, STA, mDNS `owl.local`** (SPEC §Connectivity)
- **Web UI + JSON API: on/off, effect, auto-cycle, interval, fade, brightness, speed, colour** (SPEC §Connectivity)
- **OTA: ArduinoOTA + `.bin` upload endpoint** (SPEC §Connectivity)
- **Real layout values in `layout.h`** (blocked: user must place the strip first)

## Retros

(Filled in at each sprint boundary.)
