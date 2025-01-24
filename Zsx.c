#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 9600  // Set your desired baud rate
#define TX_PIN 0
#define RX_PIN 1


int main()
{
    stdio_init_all();
    
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);

    // // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Example to turn on the Pico W LED
   
    while (1) {
        // printf("Hello, world!\n");
        // sleep_ms(1000);
        printf("UART Baud Rate Test\n");
        sleep_ms(1000);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(250);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(250);


    }
}
