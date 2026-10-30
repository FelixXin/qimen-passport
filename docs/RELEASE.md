# 0.6.1 changes since community 0.5.0

- Make Wi-Fi rows selectable with working detail pages; show hotspot client count, router names, connection/retry/failure state, IP and signal, and reconnect saved credentials.
- Default the hotspot password to 12345678; edit hotspot name/password independently on the webpage and restart Wi-Fi to apply. Preserve saved custom settings.
- Add Network time details with results, last success in this boot and manual retry; attempt synchronization after external Wi-Fi connects.
- Refresh webpage network state about every three seconds. Save new credentials only after matching IP acquisition, retaining old settings on failure.
- Fix deep sleep failing after external Wi-Fi connects; retain continuing time and key wake.
- Rewrite Chinese/English instructions around first use, controls, charts, meanings, networking, covers and sleep.
- Network pages, time synchronization, connected-Wi-Fi sleep and wake passed physical acceptance.

First installation uses the full image at 0x0 and may replace firmware/settings. Data-preserving application updates at 0x10000 require verified matching partitions. Standby current, endurance and long-term drift are unmeasured.

FoloToy-AI-Passport-full.bin: 2853056 bytes; SHA-256 `00070c9162865f11ff985d2f80453ec5cc6172604629560c6e6f547da82825b6`.

FoloToy-AI-Passport.bin: 2787520 bytes; SHA-256 `98c9484aa74fbd94ed13e9e6b5d93f0276171257831315c20d1dab03d9798f1a`.

Source: https://github.com/FelixXin/qimen-passport/tree/v0.6.1
