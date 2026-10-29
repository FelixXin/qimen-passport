<p align="right"><strong>English</strong> · <a href="README.zh_CN.md">简体中文</a></p>

# Offline calendar

The 0.4.0 solar-term table spans 1899–2100. Charts accept proleptic Gregorian dates in 1900–2099 at fixed UTC+8, with signed 64-bit seconds.

The previous recorded SHA256 did not match the actual pinned Git blob. The official Git blob `355b7bcd6fcc773b79e8bd179f0df3c5fb73a4f1` and npm lunar-javascript 1.7.7 were independently retrieved and found byte-identical: SHA256 `9750324bfe1aa63c146f8c72b1143df924466c11c8a5277d7d9225c541a18aaa`. All 1,872 original 2023–2100 term timestamps and 516 prior pillar fixtures match the regenerated results.

The pinned generator dependency and MIT license are retained in tools/vendor and are not linked into firmware. There are 1,724 new historical samples plus daily range constraints. Comparison to the same calendar library does not independently verify every school or astronomical convention.

```sh
node tools/generate_qimen_terms.js tools/vendor/lunar.js main/reader_qimen_terms.inc
node tools/generate_history_fixtures.js tests/qimen_calendar_fixtures.json tests/qimen_history_fixtures.json
```
