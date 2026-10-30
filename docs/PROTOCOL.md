<p align="right"><strong>English</strong> · <a href="PROTOCOL.zh_CN.md">简体中文</a></p>

# Local protocol

USB and BLE accept UTF-8 LF-terminated lines, maximum 383 bytes excluding LF. BLE uses the own 128-bit GATT service declared in main/reader_ble.c (RX write, TX notify); subscribe before AUTH, send bounded chunks (20 bytes works with the default ATT MTU), and wait for one complete LF reply. Connection is not authorization. Open on the physical device, then `AUTH` TAB six-digit on-screen code. This application-level code is not Bluetooth link encryption; use a trusted nearby environment. Avoid sending network secrets through untrusted clients.

Commands: `HELLO`, `GET`, `TIME` TAB UTC Unix seconds, `CAST`, `NET`, `WIFI` TAB SSID-UTF8-hex TAB password-UTF8-hex, `AP` TAB name-hex TAB password-hex, `STATS`. `PROFILE`, `AVATAR_*`, and book upload are unsupported. RADIO is refused; enable/disable Wi-Fi with physical keys so an HTTP request cannot stop its own server before replying.

All replies start QIMEN TAB OK or ERR. HELLO adds version; GET adds epoch (zero means no complete date); TIME/CAST have no additional field. WIFI adds a nonzero ticket. NET adds enabled, connected, connecting, provisioned, request ID, phase and station IP (10 fields total). Success requires matching request ID + CONNECTED + connected/provisioned flags. STATS adds free heap, largest heap block, free internal DMA heap and app stack margin (7 fields total; byte units). No credentials appear in status replies.

HTTP POST `/api/command` uses the same single-line command without LF, with `X-Qimen-Key` set to the device's eight-character management code. No key or password is put in a URL, log or localStorage. Use the board's own hotspot or another trusted network; HTTP is local cleartext, not a remote secure administration service.

Full time values are range-checked; chart support is Beijing 1900–2099; wall-clock synchronization remains 2024–2099. Time changes do not replace an already valid frozen chart until CAST, adjacent chart operation or manual on-device save. No periodic BLE time sync occurs unless the connected client actually sends TIME.

## Cover extension (0.3.0)

`NAME\t<UTF-8 hex>` saves up to eight supported display characters; an empty value restores the app title. `COVERINFO` returns six tab-separated fields: `QIMEN / OK / COVERINFO / name_hex / custom / generation`. `COVERRESET` restores Taiji without changing the name. Names are checked for UTF-8, length and glyph coverage.

`POST /api/cover` requires exactly 153600 bytes of 240×320 little-endian RGB565, `X-Qimen-Key` and an eight-digit hexadecimal `X-Cover-CRC32`. It streams to the inactive bank and commits after validation. Failure retains the current image. Authorized `GET /api/cover` returns current RGB565, or 204 for default. No filename, path or arbitrary Flash address is accepted.

## 0.4.0 extension

`CASTAT\t<signed UTC seconds>` casts and displays a chart from -2209017600 through 4102415999 without changing the wall clock. `TIME` still synchronizes 2024–2099 only. `STYLE\t<0..4>` persists Dark Taiji, Paper Taiji, Clock, Name card or Custom image; 4 is rejected without an upload. `STYLEINFO` returns five fields: `QIMEN / OK / STYLEINFO / style / image_exists`. Switching styles preserves the uploaded image. The webpage sends STYLE 4 after an upload. These commands retain USB, authenticated BLE and management-key HTTP access rules.

## 0.5.1 hotspot password

`AP\t<ssid_hex>\t<password_hex>` saves hotspot settings. An empty SSID preserves the current hotspot name; `AP\t\t3132333435363738` saves password 12345678. Empty SSIDs remain invalid for WIFI. Station credentials are unchanged. Success confirms persistence only; turn Wi-Fi off/on from the device to apply it. The HTTP request does not stop its own server. Saved custom hotspot settings override defaults.

## 0.6.0 network status

NETINFO returns 16 tab-separated fields: QIMEN, OK, NETINFO, enabled, portal, hotspot client count (-1 unknown), current SSID hex, saved SSID hex, state, IP, RSSI (-127 unknown), disconnect reason number, retry count, hotspot name hex, hotspot-change pending, error class. States: 0 off, 1 unconfigured, 2 connecting, 3 retrying, 4 router connected, 5 failed, 6 ready. Error classes: 0 none, 1 link, 2 lost IP, 3 timeout, 4 save, 5 queue, 6 start, 7 no saved network, 8 Bluetooth busy. Disconnect reasons retain SDK values without guessing a wrong password. No passwords are returned. The original ten-field NET remains compatible.

RECONNECT queues a saved-network connection and returns QIMEN/OK/RECONNECT/request-ID. Acceptance is not success: observe NET and NETINFO. No saved network returns ERR/NO_SAVED. AP permits either name or password to be empty, preserving that existing value; both empty is rejected. Existing management-key HTTP, authenticated BLE and USB rules apply.
