<p align="right"><strong>English</strong> · <a href="README.zh_CN.md">简体中文</a></p>

# Qimen Passport

An independent offline Qimen study chart for FoloToy AI Passport. Current version: **0.6.1**.

**[Open the user guide](docs/GETTING_STARTED.md)** for setup, controls, historical charts, seven symbol categories, phone networking, covers and sleep.

## Features

- Cast now or at a selected Gregorian date from 1900–2099; browse adjacent two-hour charts.
- Four pillars, nine palaces, Heaven/Earth/hidden stems, doors/stars/deities, void and horse; growth stages, month/day/hour strengths and colored states.
- Seven symbol-meaning articles with line scrolling and sequential navigation.
- Four built-in covers plus custom image/name, configured on a phone and retained after power loss.
- Local, phone-web, custom Bluetooth and network time; Wi-Fi detail pages show client count, external-network state, addresses and signal.
- Double-OK deep sleep; a right-side key restores the chart and continuing clock. Double-Up opens the cover.

## Quick start

1. Hold OK → 日期与时间; set the complete date, then choose 当前起盘.
2. Up/Down selects a palace; OK opens details; Down opens symbol meanings.
3. Enable Wi-Fi under Wi-Fi 网络, join the displayed hotspot, open http://192.168.4.1 and enter the management code from the device. Defaults: Qimen-Passport / 12345678; saved custom settings take precedence.

![Native seven-category preview](assets/images/seven-categories.png)

[Built-in cover renders](assets/images/covers-0.4.1.png) · [Community cover illustration](assets/images/qimen-passport-cover.png). Artwork is not a photograph; UI previews are software renders.

## Build

Install and activate **ESP-IDF 5.5.3** and the ESP32-C3 toolchain. Managed dependencies are pinned in `dependencies.lock`. Host tests need Python 3, a C compiler and Node.js for the webpage tests.

```sh
./tools/validate.sh --static
./tools/validate.sh --firmware
# Or both:
./tools/validate.sh
```

Optional actual-LVGL desktop render (after Managed Components are installed; CMake and Ninja required):

```sh
cmake -S tests/native -B build/native -G Ninja
cmake --build build/native
mkdir -p build/previews
build/native/preview build/previews
```

This checks the real 240×320 renderer with a 48 KiB LVGL pool and writes PPM previews.

Source: https://github.com/FelixXin/qimen-passport · Derived from https://github.com/FoloToy/ai-passport (MIT).

The firmware gate builds from tracked `sdkconfig.defaults` in an isolated directory, verifies image offsets/partitions, and produces `build/FoloToy-AI-Passport-full.bin` plus the matching ELF/MAP/hash archive in `build/firmware/`. The embedded project filename follows the upstream tooling; the application version starts at 0.2.0 independently of the earlier mixed firmware.

For a fresh install, use the verified **merged full image at offset 0x0**. It may clear existing settings; never flash an application-only image at 0x0. This independent layout has NVS, PHY and one application partition; no wordbook filesystem is required. Existing mixed-application updates need an explicitly reviewed segmented-flash/data-preservation plan. This project does not change or flash an attached device automatically.

## Validation and scope

Version 0.6.1 passed physical acceptance for network pages, network time, sleep while connected to Wi-Fi and wake. Standby current, battery life and long-term clock drift remain unmeasured. Charts use fixed UTC+8 without historical daylight-saving or true-solar-time adjustments. Current-clock calibration accepts 2024–2099; selected-time charts accept 1900–2099.

The convention follows recorded OpenFate default four-pillar/hour observations. Tests do not establish equivalence across all schools/dates. Month/day/hour strengths use the corresponding pillar branch, not numeric energy scores. Symbol meanings are learning references, not automated readings.

No vocabulary reader or private profile is included. Board support derives from FoloToy's MIT project. Target hardware: ESP32-C3, 240×320 display, 8 MB Flash, no PSRAM.

[User guide](docs/GETTING_STARTED.md) · [Protocol](docs/PROTOCOL.md) · [Architecture](docs/ARCHITECTURE.md) · [Sources/licenses](docs/SOURCES.md) · [Release notes/checksums](docs/RELEASE.md)
