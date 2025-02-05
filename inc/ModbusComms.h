

#ifndef _MODBUSCOMMS
#define _MODBUSCOMMS

#include <stdint.h>
#include <stdio.h>


/* Global type definitions ----------------------------------------------------*/
typedef void (*GetFloat_CallBack_t)(int valueStatus, float fval);



/* Global constants -----------------------------------------------------------*/
#define MODBUS_ADDRESS 0  // MODBUS address to interrogate - use 0 to be able to interrogate any address (broadcast address)

#define FRAME_TIMEOUT 3 // the number of 10ms steps of receiving no data after ahving been receiving data to determine that a packet has finished being received
#define MESSAGE_TIMEOUT 50 // the number of 10ms steps of receiving no response after a data requiest before timing out

#define MODBUS_FUNCTION_READ_INPUT_REGISTER     4
#define MODBUS_FUNCTION_READ_HOLDING_REGISTER   3
#define MODBUS_FUNCTION_WRITE_HOLDING_REGISTER  6
#define MODBUS_FUNCTION_WRITE_HOLDING_REGISTERS 16

// status responses to be used tin teh read measurand callback
#define READ_RESPONSE_TIMED_OUT           0
#define READ_RESPONSE_INVALID_REGISTER   1
#define READ_RESPONSE_VALUE_INVALID      2
#define READ_RESPONSE_VALUE_VALID        3

/* Global variables -----------------------------------------------------------*/


/* Function Prototypes --------------------------------------------------------*/
void DecodeMessage(void);
void ReadMeasurand( uint16_t address, GetFloat_CallBack_t cb );
void MessageTimedOut(void);
void ModbusCommsHandler();
void printModbusMessage();  // Function prototype

#endif

/*** end of file ***/
