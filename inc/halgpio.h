#ifndef __HALGPIO_H
#define __HALGPIO_H
#include <hardware/pio_instructions.h>
#include <pico/stdlib.h>


/* Program Version*/
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define BUILD_VERSION 0

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/


#define GAS_READING_MEASURED    30057
#define POLL_COUNT              2

/* Exported function prototypes ----------------------------------------------*/
void     BaudRate_Update ( uint16_t baudrate );
void     Watchdog        ( void );



/* Exported defines ----------------------------------------------------------*/

// Comms type
#define MODBUS_PROTOCOL     0
#define DYNAMENT_PROTOCOL   1
#define COMMS_PROTOCOL      DYNAMENT_PROTOCOL

// Checksum type
#define CSUM_STANDARD   0
#define CSUM_CRC        1         
#define CSUM_TYPE       CSUM_CRC

// System setup
#define NOP                     pio_encode_nop ( )
#define WATCHDOG_MILLISECONDS   8000    // Maximum 8 300 ms


// GPIO
//#define LED_PICO_PIN        25
#define UART_SEN_RX_PIN     5  // UART0_TX
#define UART_SEN_TX_PIN     4  // UART0_RX

//#define LED_PICO_OFF    gpio_put ( LED_PICO_PIN   , 0 )
//#define LED_PICO_ON     gpio_put ( LED_PICO_PIN   , 1 )


// UART
#define DATA_BITS           8
#define PARITY              UART_PARITY_NONE
#define STOP_BITS           1
#define UART_SEN            uart1
#define UART_BAUD_RATE      38400
#define UART_BUFFER_LENGTH  500
#define UART_TIMEOUT        1000

#endif /* _HALGPIO_H*/

/* End of file -----------------------*/

