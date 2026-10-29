idf_component_register(
 SRCS "qimen_app.c" "qimen_ui.c" "qimen_navigation.c" "reader_qimen.c" "reader_qimen_status.c" "reader_qimen_text.c" "reader_qimen_notes.c" "reader_clock.c" "reader_network.c" "reader_storage.c" "reader_ble.c" "reader_pairing.c" "reader_radio.c" "reader_local.c" "reader_remote.c" "reader_usb.c"
 "../assets/fonts/reader_font.c" "../assets/fonts/reader_chart_font.c"
 EMBED_TXTFILES "../assets/web/device.html"
 INCLUDE_DIRS "."
 PRIV_REQUIRES esp_app_format esp_driver_usb_serial_jtag
 REQUIRES bsp esp_event esp_netif esp_timer esp_wifi esp_http_server nvs_flash bt
)
