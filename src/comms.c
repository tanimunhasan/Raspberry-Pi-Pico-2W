#include"inc/comms.h"
#include<stdbool.h>
#include<string.h>
#include "inc/halgpio.h"
#include"inc/ModbusComms.h"
#include"hardware/uart.h"
#include "pico/stdlib.h"  
/* Global variables ------------------------------*/
volatile int frameTimeout = 0;  // used to determine when a frame/packet has been received (no data received within a period of time after data has been received) 
volatile int messageTimeout = 0; // used to determine when a message has been sent but no response recevied
volatile bool frameComplete = false;
volatile bool messageTimedOut = false;

/* Private variables */

struct repeating_timer timer_counter;
volatile uint16_t g_uiCommsTimeout  = 0;
uint8_t g_aucRxBuffer[P2P_BUFFER_SIZE]  __attribute__((aligned(16)));
volatile uint16_t g_uiRxBufferGet = 0;
volatile uint16_t g_uiRxBufferPut = 0;

/* Private function prototypes*/

/* Interrupt routines-------------------*/

static bool timer_counter_isr(struct repeating_timer *t)
{
        if ( g_uiCommsTimeout > 0)
    {
        g_uiCommsTimeout--;
    }
    else
    {
        // Nothing to do
    }

    // timeout for packets being received - when timeout expires, decode the data that has been receibed
    if (frameTimeout > 0)
    {
        --frameTimeout;
        if (frameTimeout <= 0)  // the last data has timed out - assume end of packet
        {
            frameComplete = true;

        }
    }

    // message timeout - if requested a data packet response, this is used to determine when no response has been recieved
    if (messageTimeout > 0)
    {
        --messageTimeout;
        if (messageTimeout <= 0)
        {
            // message timeout
            messageTimedOut = true;
        }
    }
}

//UART_RX interrupt handler 

static void uart_rx_isr()
{
    if(uart_is_readable(UART_SEN))
    {
        g_aucRxBuffer[g_uiRxBufferPut++] = uart_getc(UART_SEN); // Store character in buffer
        frameTimeout = FRAME_TIMEOUT;

        if(g_uiRxBufferPut == P2P_BUFFER_SIZE)
        {
            g_uiRxBufferPut = 0; // At the end wrap to beginning 

        }

        if(UART_UARTRSR_FE_BITS==(uart_get_hw(UART_SEN)-> rsr))
        {
            // Clear overrun error
            hw_clear_bits(&uart_get_hw(UART_SEN)-> rsr,UART_UARTRSR_OE_BITS);

        }

        else
        {
            // Nothing to do
        }

    }
}

/*User Code -------------------------------------*/

// Function: Set up UART1 to communicate with sensor at 38400 baud, no parity, 8 data bits and 1 stop bit.
//           Pin GP4 (Pico board pin 6) is used for Tx and Pin GP5 (Pico Board Pin 7) for Rx

void initialise_comms(void)
{
    add_repeating_timer_ms      (  10 , timer_counter_isr  , NULL , &timer_counter );

    //Set up UART hardware 
    uart_init                   (UART_SEN, UART_BAUD_RATE);
    gpio_set_function           (UART_SEN_RX_PIN, GPIO_FUNC_UART);
    
    // Set UART parameters
    uart_set_hw_flow            (UART_SEN , false  , false              );           // Disable CTS / RTS
    uart_set_format             (UART_SEN, DATA_BITS,  STOP_BITS, PARITY);      // Data form
    uart_set_fifo_enabled       (UART_SEN, false                        );                        // Turn off FIFO ( handled character by character )
    // Set up UART_RX interrupt
    irq_set_exclusive_handler ( UART1_IRQ , uart_rx_isr  );
    irq_set_enabled           ( UART1_IRQ , true         );
    irq_set_exclusive_handler ( UART0_IRQ , uart_rx_isr  );
    irq_set_enabled           ( UART0_IRQ , true         );
    uart_set_irq_enables      ( UART_SEN  , true , false );  // Enable UART interrupt ( RX only )


    // Clear comms buffer on spurious characters
    g_uiRxBufferGet = 0;
    g_uiRxBufferPut = 0;

}


// Function : transmit a given number of bytes of data
// Inputs : pucData - the array of data to send
//          len - the number of bytes to send
void p2pTxData ( uint8_t* pucData, int len)
{
    int x;

    for (x=0;x<len;x++)
        p2pTxByte (pucData[x]);

}



// Function : Transmit a single byte of data
// Inputs : ucData - the data byte to send
void p2pTxByte ( uint8_t ucData )
{
    uart_tx_wait_blocking ( UART_SEN );
    uart_putc ( UART_SEN , ucData );
}

// Function : Reads the next byte of received data if any are waiting to be read
// Inputs : pucData - pointer to a byte to transfer the next received byte to 
// Output :Returns p2pRxOk if a byte has been received else returns p2pRxNone if 
//         there is no data waiting to be read
uint8_t p2pRxByte ( uint8_t* pucData )
{
    uint8_t ucStatus = 0;

    if ( g_uiRxBufferGet != g_uiRxBufferPut )
    {
        *pucData = g_aucRxBuffer [ g_uiRxBufferGet++ ];

        if ( P2P_BUFFER_SIZE == g_uiRxBufferGet )
        {
            g_uiRxBufferGet = 0;
        }
        else
        {
            // Nothing to do
        }

        ucStatus = p2pRxOk;
    }
    else
    {
        ucStatus = p2pRxNone;
    }
    return ucStatus;
}

/*** end of file ***/
