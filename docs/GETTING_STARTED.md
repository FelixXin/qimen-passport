# Qimen Passport 0.6.1 — User guide

## Three controls to remember

- **Hold OK**: open settings from the grid; return from other pages.
- **Double-OK**: enter black-screen deep sleep; any right-side key wakes.
- **Double-Up**: show the cover from the grid; any right-side key returns.

Up, Down and OK refer to the three right-side buttons. Hold the left power button to switch power on/off. Menu labels below include Chinese labels shown on the device.

## First start: set time and cast

1. Hold OK → **日期与时间** (Date & time).
2. Up increases and Down decreases. OK advances through year, month, day, hour and minute, then saves. Hold OK cancels.
3. Return to settings → **当前起盘** (Cast now) → OK.

Alternatively synchronize from a phone or external Wi-Fi as described below. All charts use Beijing time (UTC+8). Set time again after complete power loss.

## Read and change charts

| Grid control | Action |
| --- | --- |
| Up / Down | Select a palace or Previous/Next control |
| OK | Open selected palace details or activate the selected control |
| Hold Up / Hold Down | Move backward / forward two hours |
| Double-Down | Next chart |
| Hold OK → 当前起盘 | Cast at the current clock time |

Pillars and dates belong to the **chart snapshot**. Setting the clock does not replace an existing chart. **○ means void; 马 means travelling horse.** Details show Heaven/Earth/hidden stems, door/star/deity, twelve growth stages, separate month/day/hour strengths and tomb, punishment and door-pressure states.

## Seven symbol-meaning categories

1. Select a palace → OK for details → Down or OK for categories.
2. Up/Down selects **Palace, Heaven stem, Earth stem, Hidden stem, Door, Star, Deity**; OK opens the article.
3. Up/Down scrolls one line. Down again at the end opens the next article; Up at the beginning opens the previous article's end.
4. OK returns to categories; hold OK returns to details; Up in details returns to the grid.

Up at the first article's beginning returns to details. The last article does not loop. These are learning references, not automatic interpretations.

## Cast at a selected date

Settings → **指定时间起盘** (Cast at time). Up/Down changes year, month, day, hour and minute; OK advances and finally casts. Hold OK cancels. Holding Up/Down on the year repeats ten-year steps.

Range: **1900-01-01 00:00 through 2099-12-31 23:59**. This does not set the wall clock. Previous/Next stops at the range limits. Dates use the Gregorian calendar and fixed UTC+8, without historical daylight-saving or true-solar-time adjustments. The webpage Date & time tab also offers selected-time casting.

## Connect your phone to the settings webpage

1. Hold OK → **Wi-Fi 网络** → select **Wi-Fi 开关** and press OK to enable it.
2. Join the hotspot displayed on the device. Default name: **Qimen-Passport**; default password: **12345678**. Saved custom values take precedence. Stay connected if your phone warns that internet is unavailable.
3. Open **http://192.168.4.1** in your browser.
4. Enter the eight-character code shown under **网页管理码**. This is separate from the hotspot password. Check it again after a reboot or deep-sleep wake.

The webpage has Cover, Date & time and Network tabs. If your phone and the device share an external router, you can also use the LAN URL displayed under Browser addresses.

## External Wi-Fi and connection status

On the webpage **Network** tab, enter an external **2.4 GHz** Wi-Fi name/password and connect. **CONNECTED / 路由器已连接 with an IP address** confirms success; credentials are saved only then. On failure, check the name/password/signal and retry. Previously saved working credentials remain available.

In the device Wi-Fi list, Up/Down selects and OK operates:

| Item | Details / action |
| --- | --- |
| Wi-Fi 开关 | Enable or disable networking |
| 设备热点 | Hotspot name, operational state, connected-device count |
| 热点密码 | Current password and pending-change notice |
| 网页管理码 | Eight-character webpage access code |
| 浏览器地址 | Hotspot and acquired LAN addresses |
| 外部 Wi-Fi | Current/saved names, connecting/retrying/connected/failed state, IP, signal and failure reason; OK reconnects saved credentials |
| 网络校时 | Network-time result and last success in this boot; OK retries |

Up/Down scrolls details; hold OK returns. Hotspot, password, code and address details also return on a single OK. The webpage Network tab refreshes about every three seconds and offers reconnecting the saved network. Hotspot client count and the device's router connection are separate links.

### Change hotspot name/password

Save a new hotspot name and/or password on the Network tab; blank fields retain their values. An 8–63-character ASCII password is simplest. Turn device Wi-Fi off and back on, then reconnect your phone using the new settings. Changes survive power loss and do not change external-router credentials.

## Set or synchronize time

- **Automatic network time:** connecting external Wi-Fi attempts synchronization. Router connection alone does not prove access to the internet or time server.
- **Manual network time:** Wi-Fi 网络 → 网络校时 → OK. Allow about 30 seconds. Retry after failure; the existing clock is retained.
- **Phone time:** webpage → Date & time → synchronize phone time. While visible, the page attempts synchronization about every 30 seconds; background operation is not guaranteed.

Offline, use 日期与时间 to set the complete date (current-clock years: 2024–2099). Afterwards choose 当前起盘 if you want a chart for the new current time.

## Covers, name and brightness

- **Style:** settings → 封面与名称. Up/Down chooses Dark Taiji, Paper Taiji, Large clock, Name card or Custom image; OK saves/previews. A right-side key returns.
- **Name:** webpage → Cover; save up to eight supported characters. Empty uses the app title; unsupported glyphs are reported.
- **Image:** webpage → Cover; choose, crop/zoom, preview and save. Restore Taiji without clearing the name. Built-in styles retain the uploaded image.
- **Brightness:** settings → 屏幕亮度; Up/Down selects four levels, OK returns.

Name, style and image survive power loss. Failed uploads retain the previous image.

## Cover standby, deep sleep and power-off

| State | Enter | Clock / networking | Return |
| --- | --- | --- | --- |
| Dim cover | Double-Up on grid or 60 seconds idle | Clock displayed; application still runs | Any right-side key |
| Black-screen deep sleep | Double-OK while awake, release keys and wait | Internal timekeeping continues; chart work and radios stop | Any right-side key; wait for page/chart restoration |
| Hardware off | Hold left power button | No timekeeping after power loss | Power on and set/synchronize time |

**Use double-OK to save power.** Showing the cover alone is not deep sleep. The first wake gesture only wakes. Wi-Fi resumes its previous preference; reconnect the webpage if needed.

## Troubleshooting

| Problem | Action |
| --- | --- |
| Calibration required | Set the full date manually or synchronize, then Cast now |
| Router connected, time sync failed | Retry Network time or sync phone time on the webpage |
| Cannot join hotspot | Check displayed name/password and enable Wi-Fi; reconnect after password changes |
| Webpage unavailable / code rejected | Wake the device, keep the hotspot connection, recheck URL/code |
| Pillars unchanged after sync | Existing chart is frozen; choose Cast now |

## Bluetooth and installation

Bluetooth time needs a compatible client and the screen authentication code (settings → 蓝牙校时). System Bluetooth pairing alone does not sync time; prefer the webpage. See the repository protocol.

Use the community full image for first installation; it may replace the previous program/settings, so back up first. Data-preserving updates require partition verification; see repository release notes. Network pages, time sync, connected-Wi-Fi sleep and wake passed physical-board acceptance on 0.6.1. Standby current, battery life and long-term clock drift remain unmeasured.
