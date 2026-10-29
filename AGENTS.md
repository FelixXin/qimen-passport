# Qimen Passport development

Independent ESP32-C3 / 8 MB / no PSRAM application derived from FoloToy AI Passport.

- Preserve the existing source checkout and stored device data. Never erase, flash, commit, upload or publish implicitly.
- Keep board support in components/bsp, application logic in main, reusable resources in assets.
- Read README.md, docs/ARCHITECTURE.md and the changed modules before editing.
- ESP-IDF 5.5.3; run tools/validate.sh. Separate Build, Host tests, Device tests and Unverified.
- Hold BSP LVGL lock outside the display task; callbacks only enqueue, no blocking radio work.
- Check actual font coverage and rendering after Chinese text changes.
- Treat build/ and device logs as private local outputs. Include no personal data or credentials.
- Paired English / Simplified Chinese Markdown; retain all upstream and third-party notices.
