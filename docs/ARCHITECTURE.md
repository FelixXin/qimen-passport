<p align="right"><strong>English</strong> · <a href="ARCHITECTURE.zh_CN.md">简体中文</a></p>

# Architecture

`main/reader_qimen*` contains the offline calculation, status tables and bounded rich-text formatter. It has no ESP-IDF or LVGL dependency. `qimen_navigation` is a pure state machine, including complete-date validation, wake suppression and seven-article boundaries. `qimen_ui` owns a reused LVGL object tree; callers hold the BSP lock. `qimen_app` owns clock, chart and state, drains button/USB/BLE/HTTP queues, and never lets a transport callback touch UI directly.

Network and BLE workers retain the tested rollback/retry lifecycle from the mixed application, with book upload removed. HTTP commands use a bounded mailbox; BLE pairing uses a three-minute application AUTH session and generation tokens. The protocol has no profile/avatar/book operations. It is an independent `QIMEN` protocol, not compatible with the FoloToy vendor installer or the older CET6 GET response.

Settings use `qimen_app`, `qimen_net` and `qimen_cover` namespaces. No filesystem is mounted. Clock anchors live in RAM. Board support remains in `components/bsp` and is copied unchanged; unused audio driver sources remain upstream infrastructure and no application audio feature is registered.

Original mixed-app sources, managed dependencies, build logs and installed hardware are not changed by extraction. Managed dependencies and build outputs stay ignored. The source snapshot is not a clone with upstream history; this independent repository preserves upstream attribution rather than claiming upstream Git history.

## Cover storage and concurrency

`qimen_cover` is a portable dual-bank RGB565 store with generation, length, payload CRC and header CRC. The header is committed last; reset creates a tombstone. `qimen_cover_store` adapts the partition, name NVS, memory mapping and mutex. Rendering acknowledges the current bank before the previous bank may be erased. Upload uses a 1 KiB buffer, with no full-image heap decoding. Image GET/POST and command routes share management-key authorization; name writes pass through the application queue.
