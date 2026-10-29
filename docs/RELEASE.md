# 0.5.0 changes since 0.4.1
- Double-OK enters deep sleep: shut down the display, radios and peripherals, stop application work and retain internal timekeeping without periodic wakeups.
- Any right-side key wakes the device and restores the page, frozen chart and elapsed clock, while consuming the wake gesture.
- Double-Up still shows the cover; Cast now moves to the first settings action.
- Wait for complete radio shutdown and key release; report preparation failures and recover after terminal peripheral failures.
- Suspend unused audio circuitry and keep recovery state without per-sleep storage writes, preserving existing data and network settings.
- Sleep, wake, continuing time and cover operation passed physical acceptance. Standby current and battery life remain unmeasured.

Install the full merged image at `0x0`. An application-only update at `0x10000` requires verification of the unchanged partition table. A first full installation may replace firmware/settings; back up first.

Full image: 2837392 bytes; SHA-256 `1d2e6ed02b0f95c32d3914cafd79388024a30fd789f3a556f9368bb6d6538c67`.

Application: 2771856 bytes; SHA-256 `68c03ace91dc679ad778e8cf6fe433094549e38d68aca22e273db799f81dbc88`.

Source: https://github.com/FelixXin/qimen-passport/tree/v0.5.0 .

Community project 1758 / revision 4106: 0.5.0 submitted and pending review. The server-reported full-image checksum matches the artifact above. Approved 0.4.1 remains public until this update is approved.
