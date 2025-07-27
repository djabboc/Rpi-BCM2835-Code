#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// Mini UART and GPIO register offsets
#define AUX_ENABLES_OFFSET     0x004
#define AUX_MU_IO_OFFSET       0x040
#define AUX_MU_IER_OFFSET      0x044
#define AUX_MU_IIR_OFFSET      0x048 // For FIFO clear
#define AUX_MU_LCR_OFFSET      0x04C
#define AUX_MU_MCR_OFFSET      0x050
#define AUX_MU_LSR_OFFSET      0x054
#define AUX_MU_CNTL_OFFSET     0x060
#define AUX_MU_BAUD_OFFSET     0x068
#define GPFSEL1_OFFSET         0x004
#define GPPUD_OFFSET           0x094
#define GPPUDCLK0_OFFSET       0x098

// Timeout in seconds
#define TIMEOUT_SEC 1

int main(int argc, char **argv) {
    printf("Attempting bcm2835_init...\n");
    if (!bcm2835_init()) {
        printf("bcm2835_init failed. Are you running as root?\n");
        return 1;
    }
    printf("bcm2835_init succeeded.\n");

    // Enable Mini UART
    printf("Enabling Mini UART...\n");
    bcm2835_peri_set_bits(bcm2835_aux + AUX_ENABLES_OFFSET/4, 1, 1);

    // Configure GPIO 14 and 15 for Mini UART (ALT5)
    printf("Configuring GPIO 14 and 15...\n");
    uint32_t selector = bcm2835_peri_read(bcm2835_gpio + GPFSEL1_OFFSET/4);
    selector &= ~(7 << 12); // Clear GPIO 14
    selector |= 2 << 12;    // Set ALT5 for GPIO 14 (TXD)
    selector &= ~(7 << 15); // Clear GPIO 15
    selector |= 2 << 15;    // Set ALT5 for GPIO 15 (RXD)
    bcm2835_peri_write(bcm2835_gpio + GPFSEL1_OFFSET/4, selector);

    // Disable pull-up/down for GPIO 14 and 15
    printf("Disabling pull-up/down for GPIO 14 and 15...\n");
    bcm2835_peri_write(bcm2835_gpio + GPPUD_OFFSET/4, 0);
    bcm2835_delay(150);
    bcm2835_peri_write(bcm2835_gpio + GPPUDCLK0_OFFSET/4, (1 << 14) | (1 << 15));
    bcm2835_delay(150);
    bcm2835_peri_write(bcm2835_gpio + GPPUDCLK0_OFFSET/4, 0);

    // Configure Mini UART
    printf("Configuring Mini UART...\n");
    bcm2835_peri_write(bcm2835_aux + AUX_MU_CNTL_OFFSET/4, 0);  // Disable TX, RX
    bcm2835_peri_write(bcm2835_aux + AUX_MU_IER_OFFSET/4, 0);   // Disable interrupts
    bcm2835_peri_write(bcm2835_aux + AUX_MU_LCR_OFFSET/4, 3);   // 8-bit mode
    bcm2835_peri_write(bcm2835_aux + AUX_MU_MCR_OFFSET/4, 0);   // RTS line high
    bcm2835_peri_write(bcm2835_aux + AUX_MU_IIR_OFFSET/4, 0xC6); // Clear TX/RX FIFOs
    bcm2835_peri_write(bcm2835_aux + AUX_MU_BAUD_OFFSET/4, 270); // Baud rate 115200
    bcm2835_peri_write(bcm2835_aux + AUX_MU_CNTL_OFFSET/4, 3);  // Enable TX, RX

    // Test string
    const char *test_string = "Hello, UART Loopback!\n";
    int i = 0;

    // Transmit and receive loop
    printf("Starting loopback test...\n");
    while (test_string[i]) {
        // Wait until TX FIFO can accept data
        time_t start_time = time(NULL);
        while (!(bcm2835_peri_read(bcm2835_aux + AUX_MU_LSR_OFFSET/4) & 0x20)) {
            printf("Waiting for TX FIFO (LSR=0x%02x)...\n", 
                   bcm2835_peri_read(bcm2835_aux + AUX_MU_LSR_OFFSET/4));
            if (time(NULL) - start_time > TIMEOUT_SEC) {
                printf("Timeout waiting for TX FIFO\n");
                break;
            }
        }
        bcm2835_peri_write(bcm2835_aux + AUX_MU_IO_OFFSET/4, test_string[i]);

        // Wait for received data
        start_time = time(NULL);
        while (!(bcm2835_peri_read(bcm2835_aux + AUX_MU_LSR_OFFSET/4) & 0x01)) {
            printf("Waiting for RX data (LSR=0x%02x)...\n", 
                   bcm2835_peri_read(bcm2835_aux + AUX_MU_LSR_OFFSET/4));
            if (time(NULL) - start_time > TIMEOUT_SEC) {
                printf("Timeout waiting for RX data\n");
                break;
            }
        }
        char received = bcm2835_peri_read(bcm2835_aux + AUX_MU_IO_OFFSET/4);
        printf("Sent: %c, Received: %c\n", test_string[i], received);
        i++;
    }

    // Clean up
    printf("Cleaning up...\n");
    bcm2835_close();
    return 0;
}
