# Qimen Passport 0.5.0 — Getting started

## First use
The device starts in Qimen. No account, internet service or wordbook is needed. Hold right-side OK, choose Date & time, then use Up/Down to adjust year, month, day, hour and minute. OK advances and finally saves; hold OK to cancel. Current-clock calibration supports 2024–2099. All charts use Beijing time, UTC+8. Choose Cast now, the first settings action, to cast the current clock time.

## Charts and controls
On the grid, Up/Down selects a palace or Previous/Next; OK opens details or changes the chart. Hold Up/Down to move backward/forward two hours. Double-Down advances one chart. Hold OK opens settings or returns one level. Pillars, dates and palace contents belong to the chart snapshot; synchronizing the clock does not silently replace it.

In details, Down or OK opens symbol meanings, and Up returns to the grid. The seven categories are Palace, Heaven stem, Earth stem, Hidden stem, Door, Star and Deity. Select with Up/Down and press OK to read. In an article, Up/Down scrolls one line; Down again at the end advances, while Up at the beginning opens the previous article's end. OK returns to categories; hold OK returns to details. Up at the first article's beginning returns to details; the last article does not loop. Meanings are study references, not automatic readings.

## Deep sleep and wake
Double-press right-side OK on an awake page, release the buttons and wait for radios, display and peripherals to shut down. During deep sleep there is no UI refresh, chart processing or Wi-Fi/Bluetooth service. Only internal timekeeping continues; no periodic wakeup is scheduled.

Press any right-side key to wake and allow initialization to finish. The previous page, frozen chart and clock including elapsed sleep time are restored. The wake gesture does not activate a page action. Wi-Fi resumes according to its saved preference; a temporary Bluetooth window must be reopened. The left hardware power button retains its hold-to-power behavior. Complete power loss also stops timekeeping, so set or synchronize time again after switching on.

Double-Up still opens the cover; the dim cover also appears after 60 seconds of inactivity. Cover standby differs from double-OK deep sleep. Double-OK no longer casts the current chart: use Cast now in settings instead.

## Cast at a selected time
Settings → Cast at time accepts Gregorian dates from 1900-01-01 00:00 through 2099-12-31 23:59. Up/Down edits fields; holding a year key repeatedly moves ten years. OK advances and finally casts; hold OK cancels. This does not change the wall clock. Adjacent-chart navigation stops at date limits without replacing the original chart. Historical dates use fixed UTC+8, without historical daylight-saving or true-solar-time adjustments.

## Phone webpage, time and Wi-Fi
Hold OK → Wi-Fi network → OK to enable it. The default hotspot is Qimen-Passport; its random password and eight-character management code appear on the device. Join it from your phone, open http://192.168.4.1 , enter the code and connect. Date & time offers phone-time synchronization and a separate selected-time cast. While visible, the page attempts time synchronization every 30 seconds; background operation is not guaranteed.

On Network, enter an external 2.4 GHz Wi-Fi name and password. Success requires the matching request to show CONNECTED and an IP address; only then are credentials saved. Check details and retry after a failure, or continue using the device hotspot. The webpage is unavailable during deep sleep; reconnect and refresh after waking. Use local settings only on trusted networks.

## Cover and name
Settings → Cover lets Up/Down choose Dark Taiji, Paper Taiji, Clock, Name card or Custom image. OK saves and previews; a right-side key wakes and returns. On the webpage's Cover tab, save up to eight supported name characters, or leave it empty for the app title. Choose, crop and zoom an image before uploading, or restore Taiji. Name, style and image survive power loss; choosing built-ins retains the upload, and failed uploads retain the previous image.

## Bluetooth time
Settings → Bluetooth time opens a three-minute window. Use a client supporting the custom service: subscribe to notifications, send AUTH with the six-digit screen code, then TIME with the phone's UTC timestamp. See repository protocol documentation. System Bluetooth settings do not send time/passwords automatically; iPhone Safari lacks this client, so use the Wi-Fi webpage. No WeChat mini program or signed phone app is included.

## Installation and validation
Use the community full image for a first installation. It may replace firmware/settings, so back up first. An application-only image is not a full installation. A data-preserving update from 0.4.1 requires verification of the unchanged partition table and the repository instructions; updating from 0.2.0 also requires the new table. The author's local update did not erase names, networks or wordbooks.

Version 0.5.0 passed physical acceptance for sleep, key wake, continuing time and cover operation, plus build, host fault and native UI checks. Standby current, battery life and long-duration clock drift are not measured; no specific endurance is claimed. Community cover artwork is an illustration; actual UI renders are included in the repository.
