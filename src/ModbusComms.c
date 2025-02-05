#include"inc/halgpio.h"
#include<stdbool.h>
#include<string.h>
#include "inc/comms.h"
#include "inc/ModbusComms.h"
#include <stdio.h>

/* Type definitions- -------------------------------------*/


/* Constant declarations --------------------------------*/

#define PACKET_RX_TIMEOUT   3

// Table of CRC values for high–order byte
const uint8_t auchCRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

// Table of CRC values for low–order byte 
const uint8_t auchCRCLo[] = 
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};


/* Variable declaration ---------------------------*/

uint8_t modbusRxBuffer[300];
uint16_t modbusRxBufferCount = 0;

GetFloat_CallBack_t readMeasurandCallBack = 0;
bool readingMeasurand = false; // set true if a floating point measureand value is being read for a callback routine

/*Local function prototypes*/

void DecodeMeasurand(uint16_t *registers, uint numReg);
void ReadRegistersResponse(uint8_t *data, uint len);


uint CRC16(uint8_t *puchMsg, uint usDataLen)
{
    uint8_t uchCRCHi = 0xFF; // high byte of CRC initialized 
    uint8_t uchCRCLo = 0xFF; // low byte of CRC initialized 
    uint8_t uIndex; // will index into CRC lookup table 
    uint len = usDataLen;
    int x = 0;

    while ((len--) > 0) // pass through message buffer 
    {
        uIndex = (uint8_t)(uchCRCLo ^ puchMsg[x++]); // calculate the CRC 
        uchCRCLo = (uint8_t)(uchCRCHi ^ auchCRCHi[uIndex]);
        uchCRCHi = auchCRCLo[uIndex];
    }
    return ((uint)(uchCRCHi) << 8 | uchCRCLo);
}


// Function : Formulate a MODBUS packet of data from the given parameters and sends to the serial port
// Inputs : function - the type of packet to send (Read Input REgister, Write Holding REgister, etc.
//          data - the data portion of the packet to be sent
//          len - the number of data bytes in teh data portion of the packet
// Outputs : None
void SendPacket(uint8_t function, uint8_t *data, int len)
{
    uint8_t txBuffer[256];
    uint txLen = 0;

    txBuffer[txLen++] = MODBUS_ADDRESS;  // use broadcast address
    txBuffer[txLen++] = function;

    int x;
    for (x = 0; x < len; x++) { txBuffer[txLen++] = data[x]; }

    uint crc = CRC16(txBuffer, txLen);

    txBuffer[txLen++] = (uint8_t)(crc & 0xff);
    txBuffer[txLen++] = (uint8_t)((crc >> 8) & 0xff);

    p2pTxData (txBuffer, txLen);


}

// Function : Request the values of a given number of status of holding registers
// Inputs   : func - type of register read request (input registers or holding registers)
//            address - address of the first register to read
//            numReg - the number of registers to read
// Output   : None
void ReadRegisters(uint8_t func, uint16_t address, uint numReg )
{
    uint8_t data[4];

    // create the data portion of the packet - first add the address of the first register and then the number of registers
    data[0] = (uint8_t)((address >> 8) & 0xff);
    data[1] = (uint8_t)(address & 0xff);
    data[2] = (uint8_t)((numReg >> 8) & 0xff);
    data[3] = (uint8_t)(numReg & 0xff);

    SendPacket(func, data, 4);  // send packet to request the regsiter data
    messageTimeout = MESSAGE_TIMEOUT;
}


// Function : Request a float measurand value to be read and passed onto a callback callback routine
// Inputs   : address - address of the first register to read
//            cb - callback routine in which to return the gas reading and its validity status
// Output   : None
void ReadMeasurand(uint16_t address, GetFloat_CallBack_t cb )
{
    readingMeasurand = true;
    readMeasurandCallBack = cb;
    ReadRegisters (MODBUS_FUNCTION_READ_INPUT_REGISTER ,address, 5);

}

// Function : Called when a frame of data has been received to download the received data and decode the packet if valid
// Inputs   : None
// Output   : None
void DecodeMessage()
{
    uint8_t data[257];
    int len=0;
    uint8_t ch;
    // get the data to decocde first
    while ( p2pRxByte(&ch)==p2pRxOk  && len<256)
    {
        data[len++] = ch;
    }

    // now determine if this is a valid packet
    if (len < 4) return;  // smallest packet size is 5 bytes

    // get the received checkum and calculate what it should be for the received data
    uint16_t rcvCsum = (uint)((data[len - 1] * 0x100) + data[len - 2]);
    uint16_t calcCsum = CRC16(data, len - 2);
    if (rcvCsum != calcCsum)  // checksums don't match - ignore packet
    {
        return;
    }


    // check to see if packet is a NAK
    if ((data[1] & 0x80) > 0)  // bit 7 set indicates a NAK to a message
    {
        // if a read measurand operation has been  requested and a callback routine set, then return a negaitve result
        // this will almost certainly be an ainvalid register address
        if (readingMeasurand && readMeasurandCallBack != 0)
        {
            readMeasurandCallBack (READ_RESPONSE_INVALID_REGISTER, 0);

            // clear the read operation
            readingMeasurand = false;
            readMeasurandCallBack = 0;
        }
    }
    else  // valid data received 
    {
        switch (data[1])  // check the Function/Command byte of the pacet
        {
            case MODBUS_FUNCTION_READ_INPUT_REGISTER:
            case MODBUS_FUNCTION_READ_HOLDING_REGISTER:
                ReadRegistersResponse(data, len);
                break;
            case MODBUS_FUNCTION_WRITE_HOLDING_REGISTER:
                break;
            case MODBUS_FUNCTION_WRITE_HOLDING_REGISTERS:
                break;
        }

        messageTimeout = 0; // cancel the timeout counter as a response has been received
    }

}

// Function : Decode a data packet containing received registers data
// Inputs : data - data bytes received
//          len - number of bytes received in data portion
// Outputs : None
void ReadRegistersResponse(uint8_t *data, uint len)
{
    // determine the number of registers tha the data contains information for
    uint16_t regs = (uint)data[2] / 2;
    if (len < ((regs * 2) + 5)) return;  // not enough data for the specificed number of registersd and given data length

    // convert the bytes in the data packet into 16 register data
    int x;
    uint16_t registers[128];

    for (x = 0; x < regs; x++)
    {
        registers[x] = (uint16_t)((data[(x * 2) + 3] * 0x100) + data[(x * 2) + 4]);
    }


    // if data has been received then a unit is connected and responding to the correct register request
    if (readingMeasurand)
    {
        DecodeMeasurand(registers, regs);
    }
}

// Function : Decode a series of registers received that make up a PST protocol measurand
// Inputs : registers - registers received that make up measurand
//          numReg - number of registers received
// Outputs : None
void DecodeMeasurand(uint16_t *registers, uint numReg)
{
    if (readingMeasurand && readMeasurandCallBack != 0)
    {
        if (registers[0]>0)  // reading is valid
        {
            // get the float value from the two registers that cover the actual reading
            uint32_t intVal = (uint32_t) ((registers[1] * 0x10000) + (registers[2]));
            float *fPtr = (float *) &intVal;
            float f = *fPtr;


            readMeasurandCallBack (READ_RESPONSE_VALUE_VALID, f);
        }
        else // reading is not valid (either sensor is in error condition or is warming up))
        {
            readMeasurandCallBack (READ_RESPONSE_VALUE_INVALID, 0);
        }
        
    }

    readingMeasurand = false;
    readMeasurandCallBack = 0;
}

// Function : Called when no response has been receibed to a previously transmitted packet
void MessageTimedOut(void)
{
    if (readingMeasurand && readMeasurandCallBack != 0) 
    { 
         readMeasurandCallBack (READ_RESPONSE_TIMED_OUT, 0);  // pass on the result via the callback routine
    }
    readingMeasurand = false;
    readMeasurandCallBack = 0;
}

// Function : Called from the main program loop to check for recevied packets or message timeouts
void ModbusCommsHandler()
{
    // check to see if a packet has been received
    if (frameComplete)
    {
        frameComplete = false;
        DecodeMessage();  // assume MODBUS protocol message    
    }

    // Check to see if no response has been received to a previously transmitted packet
    if (messageTimeout)
    {
        messageTimeOut = false;
        MessageTimedOut();
    }
}



/*** end of file ***/
