#include "reader_usb.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include <fcntl.h>
#include <unistd.h>

bool reader_usb_init(void) {
    /* IDF 5.5.3 nonblocking VFS reads count the driver's RX ring buffer.
       The default ROM/FIFO console can print, but reports zero readable bytes. */
    if (!usb_serial_jtag_is_driver_installed()) {
        usb_serial_jtag_driver_config_t config = {.tx_buffer_size=1024,.rx_buffer_size=1024};
        if (usb_serial_jtag_driver_install(&config) != ESP_OK) return false;
    }
    usb_serial_jtag_vfs_use_driver();
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    return flags >= 0 && fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == 0;
}
