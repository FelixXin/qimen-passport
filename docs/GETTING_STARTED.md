# Qimen Passport 0.4.1 — Getting started

Version 0.4.1 was installed on the original AI Passport and the user accepted cover selection, confirmation, wake-and-repeat behavior and removal of the English cover eyebrow. Flash integrity, USB version readback and time synchronization passed. The 1900/2099 chart limits and wall-clock isolation were checked on 0.4.0. Wireless connectivity, exhaustive historical-chart agreement, first full installation and power use are outside this acceptance. Cover artwork is an illustration; actual UI renders are also included.

## First use
The device starts in Qimen. No account, internet service or wordbook is required. Charts use Beijing time (UTC+8), with chart dates from 1900 to 2099 (current-clock synchronization remains 2024–2099). Hold the right-side OK button to open settings and choose local time. Up increases and Down decreases the selected field. OK advances through year, month, day, hour and minute, then saves. Hold OK to cancel. Cast a current chart after saving. Time must be set again after power is disconnected.

## Charts and meanings
On the grid, Up/Down selects a palace or the previous/next chart control. OK opens details or changes the chart. Hold Up/Down to move backward/forward by two hours. Double-press Down for the next chart. Double-press OK to cast the current time. The displayed pillars and date belong to the chart snapshot; synchronizing the clock does not silently replace it.

Palace details include stems, door, star and deity, twelve growth stages, month/day/hour element strengths, and tomb, punishment, door pressure, void and horse markers. Down or OK opens symbol meanings. Up or hold OK returns to the grid.

The seven categories are Palace, Heaven stem, Earth stem, Hidden stem, Door, Star and Deity. Choose with Up/Down, then OK to read. In an article, Up/Down scrolls one line. Press Down once more at the end to advance to the next article; Up at the start opens the end of the previous article. OK returns to category selection. Hold OK returns to palace details. Up at the beginning of the first article returns to details; the last article does not loop. These are learning references, not automatic interpretations.

## Phone webpage: time and Wi-Fi
Hold OK, open Wi-Fi settings, and press OK to enable the hotspot. Its name is Qimen-Passport. The random password and eight-character management code appear on the device. Join it from your phone, open http://192.168.4.1 , enter the management code and synchronize phone time. Cast a new current chart if needed. The page attempts time synchronization every 30 seconds while visible; background operation is not guaranteed.

The same page accepts an external 2.4 GHz Wi-Fi network name and password. Treat the matching request ID with CONNECTED and the corresponding IP as success. Receiving a request is not connection confirmation. Credentials are saved only after that connection succeeds. On failure, check the details and submit again, or keep using the hotspot. Use the local webpage only on trusted networks.

## Bluetooth time
Hold OK, open Bluetooth time and enable the three-minute window. Use a client supporting custom BLE services: subscribe to notifications, send AUTH with the six-digit code on screen, then TIME with the phone's current UTC timestamp. See the repository protocol documentation for service identifiers and framing. System Bluetooth settings do not automatically send time or passwords. iPhone Safari does not provide this client; use the Wi-Fi webpage instead. No WeChat mini program or signed phone app is included.

## Standby and installation
Double-press Up for the dim clock. It also appears after 60 seconds of inactivity. The first right-side button press only wakes the screen.

For a first installation, use the verified full merged image at offset 0x0. It may replace existing firmware and settings. Do not use an application-only image as a full image. Preserving data when updating a mixed-version device requires a separate review; publishing this preview does not modify your device. Source, licensing and checksums are available in the repository and preview Release.

## Cover and name

Settings → Cover lets Up/Down select a style; OK saves and previews it. In the phone webpage, save a name, choose and crop/zoom an image, upload it, or restore Taiji. Name and image survive power loss. Names allow up to eight supported characters; an empty name uses the application title. Updates from 0.2.0 require the new partition table and a recovery backup. For 0.3.x/0.4.x, an application-only update is possible after verifying the unchanged partition table.

## Historical charts and styles

Hold OK → Cast at time. Enter year/month/day/hour/minute in fixed Beijing time, from 1900-01-01 00:00 through 2099-12-31 23:59. Hold a year key for repeated ten-year steps; OK advances and finally casts. Hold OK cancels. The wall clock is unchanged, and double OK still casts the current time. Adjacent-chart navigation stops safely at range limits.

Settings → Cover lets Up/Down choose five styles; OK saves and previews. Upload a custom image first. Selecting built-ins retains it. The webpage supports Apply style and Cast at selected time; the latter never sends a clock-sync command.
