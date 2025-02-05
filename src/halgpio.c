
/* Includes ------------------------------------------------------------------*/
#include "inc/halgpio.h"

#include <stdio.h>
#include <string.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <pico/binary_info.h>
#include "inc/comms.h"
#include "inc/ModbusComms.h"
#include "inc/DynamentComms.h"
#include "pico/cyw43_arch.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
// #define GAS_READING_MEASURAND   30057
// #define POLL_COUNT              2

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// struct repeating_timer timer_heartbeat;
// int pollCounter = POLL_COUNT;

/* Local prototypes ----------------------------------------------------------*/
// void RequestGasReading();
// void ReadingReceived(int status, float value);
// void DualReadingReceived(int status, float reading1, float reading2);

/* User code -----------------------------------------------------------------*/

// Function : Once a second timer interrupt for flashing Pico LED as a heartbeat and determining when to poll sensor
// Inputs : t - timer structure
// Outputs : Always returns true
/*

static bool timer_heartbeat_isr ( struct repeating_timer *t )
{
    if (cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN))
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,0);
    }
    else
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,1);
    }

    // determine if it's time to poll the sensor
    if (pollCounter>0)
    {
        --pollCounter;
        if (pollCounter <= 0)
        {
            RequestGasReading();
            pollCounter = POLL_COUNT;
        }
    }
    return true;
}





*/

/*


int main ( void )
{
    stdio_init_all();
 
    // Set up watchdog
    watchdog_enable ( WATCHDOG_MILLISECONDS , 1 );

     if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    
    struct repeating_timer timer;
    // Set up timer interrupts
    add_repeating_timer_ms (  1000 , timer_heartbeat_isr , NULL , &timer_heartbeat );

    // initialise UART1 & comms syste,
    initialise_comms();
    if (COMMS_PROTOCOL==DYNAMENT_PROTOCOL)
        InitialiseDynamentComms();

    // Infinite loop
    for ( ; ; )
    {
        Watchdog ( );

        // Communications system handler routines        
        if (COMMS_PROTOCOL==DYNAMENT_PROTOCOL)
            DynamentCommsHandler();
        else
            ModbusCommsHandler();  // Modbus protocol handler
            

        //  Place main program loop code here
    }

    return 0;
}









*/
/* 

// Function : Watchdog pump routine
void Watchdog ( void )
{
    watchdog_update ( );
}

// Functions for interfacing with Premier sensor via Modbus. Request a gas reading
void RequestGasReading()
{
    if (COMMS_PROTOCOL==DYNAMENT_PROTOCOL)
    {
        RequestLiveData2 (DualReadingReceived);

    }
    else
    {
        ReadMeasurand (GAS_READING_MEASURAND, ReadingReceived);
    }
}
*/
// Function : Callback rotine that will be called from the comms layer when a gas reading has been
//            received, an invalid reading has been received or a requested reading has timed out
// Inputs : status - the validity of the reading as one of:
//                       TimedOut=0,   - no response from sensor
//                       InvalidRegister=1,  - response received saying the requested Modbus registers are not valid (bad address?)
//                       ValueNotValid=2   - response received saying the gas reading is not valid (sensor may be warning up or in a fault condition)
//                       ValueValid=3   - valid response received and the parameter value contains the actual gas reading
//          value - the actual gas reading if status is set to ValueValid
// Outputs : None

/*  
void ReadingReceived(int status, float value)
{
    if (status==READ_RESPONSE_VALUE_VALID)  // valid reading received
    {
        /*** value contains the current gas reading - add code here to use it ***/
       // printf("Testing Phase");

//     }

// }

// Function : Callback rotine that will be called from the comms layer when a gas reading has been
//            received, an invalid reading has been received or a requested reading has timed out
// Inputs : status - the validity of the reading as one of:
//                       TimedOut=0,   - no response from sensor
//                       InvalidRegister=1,  - response received saying the requested Modbus registers are not valid (bad address?)
//                       ValueNotValid=2   - response received saying the gas reading is not valid (sensor may be warning up or in a fault condition)
//                       ValueValid=3   - valid response received and the parameter value contains the actual gas reading
//          reading1 - the actual gas reading for Gas Reading 1 for if status is set to ValueValid
//          reading2 - the actual gas reading for Gas Reading 2 for if status is set to ValueValid
// Outputs : None


//  void DualReadingReceived(int status, float reading1, float reading2)
// {
//     if (status==READ_RESPONSE_VALUE_VALID)  // valid reading received
//     {
//         /*** reading1 amd reading2 contains the current gas readings - add code here to use it ***/


//     }
// }
