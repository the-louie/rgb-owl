# TODO — planning, sprints and backlog

Single planning file (user decision 2026-09-23). It serves as roadmap, active sprint and backlog.
Spec: [SPEC.md](SPEC.md). Agent rules and ledger: [AGENTS.md](AGENTS.md).

## Current position

**Active sprint:** Sprint 02, in progress (opened 2026-09-23)
**Current ticket:** T-09 (`todo` — Rainbow sweep + twinkle)
**Last completed:** T-08 (`done` — commit `0abe358`)

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

## Sprint 02 — effects + auto-cycle

**Goal:** All 7 SPEC effects render and auto-cycle with a crossfade.
**Demo:** `pio run -e s3zero` builds and `pio test -e native` passes (scheduler/crossfade tests).
On hardware, after the boot walk the owl cycles through 7 effects, 60 s each, with 2 s fades.

| ID | Title | Est | Deps | Status | Notes |
|---|---|---|---|---|---|
| T-06 | Matrix green rain | 2h | — | done | SPEC §Effects 2; Landed in `c054c58` |
| T-07 | Flame (per-column heat, Fire2012-style, grid-aware) | 3h | — | done | SPEC §Effects 3; Landed in `0031e53` |
| T-08 | Aurora (pastel noise bands) | 2h | — | done | SPEC §Effects 4; Landed in `0abe358` |
| T-09 | Rainbow sweep + twinkle | 2h | — | todo | SPEC §Effects 6 |
| T-10 | Breathing solid pastel colour | 1h | — | todo | SPEC §Effects 7 |
| T-11 | Owl eyes (glow + random blink, `EYES[]`) | 2h | — | todo | SPEC §Effects 5 |
| T-12 | Auto-cycle scheduler + crossfade (60 s / 2 s, adjustable) + native tests | 3h | — | todo | SPEC §Behaviour |

## Sprint 03 — connectivity (sketch)

Goal: settings in NVS, WiFi with captive portal + mDNS, web UI + JSON API, OTA. Drawn from the backlog.

## Backlog

Unscheduled, in rough priority order. Sprint 03 draws from here.

- **Settings model + NVS persistence, debounced 5 s** (SPEC §Behaviour)
- **WiFi: WiFiManager captive portal `Owl-Setup`, STA, mDNS `owl.local`** (SPEC §Connectivity)
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
