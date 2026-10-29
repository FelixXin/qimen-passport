# Qimen Passport 0.3.0 — Getting started

This is a public preview. Firmware builds, host tests and native desktop rendering passed. This standalone revision has not been accepted on physical hardware; wireless behavior, first installation and power use still need device testing. The cover is an illustration; actual rendered UI previews are included in the repository.

## First use
The device starts in Qimen. No account, internet service or wordbook is required. Charts use Beijing time (UTC+8), with dates from 2024 to 2099. Hold the right-side OK button to open settings and choose local time. Up increases and Down decreases the selected field. OK advances through year, month, day, hour and minute, then saves. Hold OK to cancel. Cast a current chart after saving. Time must be set again after power is disconnected.

## Charts and meanings
On the grid, Up/Down selects a palace or the previous/next chart control. OK opens details or changes the chart. Hold Up/Down to move backward/forward by two hours. Double-press OK to cast the current time. The displayed pillars and date belong to the chart snapshot; synchronizing the clock does not silently replace it.

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

Settings → Cover opens cover information; OK previews the default Taiji design. In the phone webpage, save a name, choose and crop/zoom an image, upload it, or restore Taiji. Name and image survive power loss. Names allow up to eight supported characters; an empty name uses the application title. This revision adds image storage, so an existing-device update must review and update the partition table with a recovery backup.
