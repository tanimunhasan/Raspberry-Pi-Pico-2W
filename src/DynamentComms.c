#include "inc/comms.h"

#include"inc/halgpio.h"
#include<stdbool.h>
#include<string.h>
#include"inc/DynamentComms.h"

#include <stdio.h>

/* Local constants -----------------------------------------------------------*/
// timeouts
#define DYNAMENT_FRAME_TIMEOUT 30
#define DYNAMENT_MAX_PACKET_SIZE 300
#define DYNAMENT_RESPONSE_TIMEOUT 70

// packet constants
#define READ_VAR        0x13
#define DLE             0x10
#define WRITE_REQUEST   0x15
#define ACK             0x16
#define NAK             0x19
#define DAT             0x1a

#define WRITE_PASSWORD_1 0xe5
#define WRITE_PASSWORD_2 0xa2

// variable ID's
#define LIVE_DATA_SIMPLE    0x06
#define LIVE_DATA_2         0x2c

// CRC calculation constants for SIL compatible comms
#define CRC_POLYNOMIAL  0x8005    //!< Polynomial for CRC Lookup Table Calculation
#define CRC_VALUE_MASK  0x8000


/* Private variables ---------------------------------------------------------*/

// Variables for monitoring the reception of packets of data
bool receivingPacket = false;
uint8_t rxBuffer[DYNAMENT_MAX_PACKET_SIZE];
int rxCount = 0;
bool DLEReceived = false;
bool waitingForCommand = false;
uint8_t command = 0;
bool EOFReceived = false;
uint16_t rcvCsum = 0;
uint16_t calcCsum = 0;
bool packetComplete = false;
bool csumError = false;
int csumByteReceived = 0;
bool packetNAKed = false;
uint8_t errorCode = 0;
bool packetACKed = false;
volatile bool messageTimeOut = false;  // Define it here

// state machine variables
enum
{
    CommsModeIdle,
    CommsModeRequestingLiveDataSimple,
    CommsModeRequestingLiveData2,
};
int currentMode=CommsModeIdle;
GetFloat_CallBack_t readGasCallBack = 0;  // callback pointer to return read values to if set to point to a function
GetDualFloat_CallBack_t readDualGasCallBack = 0;  // callback pointer to return  dual read values to if set to point to a function


/* Private function prototypes -----------------------------------------------*/
uint16_t UpdateChecksum (uint16_t currentCRC, uint8_t newByte);
uint16_t UpdateCRCTab (uint16_t index);
void Reset(void);
void CharReceived(uint8_t chr);
bool SendDynamentPacket(uint8_t cmd, uint8_t variableID, uint8_t dlen, uint8_t *dataPtr);
void ProcessReceivedPacket(void);
void ReadLiveDataResponse(uint8_t *dataPtr, int len);
void ReadLiveData2Response(uint8_t *dataPtr, int len);
void ACKReceived(void);
void NAKReceived(void);
void PacketChecksumError(void);
void PacketTimedOut(void);



/* User code -----------------------------------------------------------------*/

// Function : Set up comms system for communciations using the Dynament protocol
void InitialiseDynamentComms(void)
{
    Reset();  // set all the packet reception vaiables to their default to await an incoming packet
}

// Function : General purpose handlin routine for teh Dynament comms protocol
void DynamentCommsHandler(void)
{
    // check for received data
    // get the data to decocde first
    uint8_t ch;
    while ( p2pRxByte(&ch)==p2pRxOk)
    {
        CharReceived(ch);
    }

    

    // in this protocol - this indicates that the frame has timed out without being completed
    if (frameComplete)
    {
        frameComplete = false;
        PacketTimedOut();
    }

    // Check to see if no response has been received to a previously transmitted packet
    if (messageTimeOut)
    {
        messageTimeOut = false;
        PacketTimedOut();
    }

}


uint16_t UpdateChecksum (uint16_t currentCRC, uint8_t newByte)
{
    uint16_t retVal;

    if (CSUM_TYPE==CSUM_CRC)
    {
        retVal =(uint16_t)( (currentCRC << 8) ^ UpdateCRCTab((uint16_t)( (currentCRC >> 8) ^ newByte)));

    }
    else 
    {
        retVal = (uint16_t) (currentCRC + (uint16_t) newByte);
    }

    return retVal;
}

uint16_t UpdateCRCTab (uint16_t index)
{
    uint16_t uiIndex;
    uint16_t  uiCrcValue, uiTempCrcValue;
    uiCrcValue = 0;

    uiTempCrcValue = (uint16_t)(((uint) index) << 8);
    for (uiIndex = 0; uiIndex < 8; uiIndex++)
    {
        if (((uiCrcValue ^ uiTempCrcValue) & CRC_VALUE_MASK) > 0)
        {
            uiCrcValue =(uint16_t) ((uint16_t) ((uiCrcValue << 1)) ^ (uint16_t) CRC_POLYNOMIAL);
        }
        else
        {
            uiCrcValue = (uint16_t)(((uint)uiCrcValue) << 1);
        }
        uiTempCrcValue = (uint16_t)(((uint)uiTempCrcValue) << 1);
    }
    return uiCrcValue; 

}  

// Function: Reset all teh variables associated with a packet being received - typically called
//           when a packet has timed out or been aborted
void Reset(void)
{
    receivingPacket = false;
    messageTimeout = 0;
    frameTimeout = 0;
    rxCount = 0;
    DLEReceived = false;
    waitingForCommand = false;
    command = 0;
    EOFReceived = false;
    rcvCsum = 0;
    calcCsum = 0;
    packetComplete = false;
    csumError = false;
    csumByteReceived = 0;
    packetNAKed = false;
    packetACKed = false;
    errorCode = 0;
}

void PacketSent(void)
{
    Reset();
    messageTimeout = DYNAMENT_RESPONSE_TIMEOUT;    
}

// Function Called when a single uint8_t of data is received in order to process it as part of an ongoing packet frame being received
void CharReceived(uint8_t chr)
{
    int x;
    if (rxCount >= DYNAMENT_MAX_PACKET_SIZE)  // buffer full so reset - this is a bad packet
        Reset();

    if (chr == DLE && !EOFReceived)
    {
        if (!receivingPacket)
        {
            receivingPacket = true;
            rxCount = 0;
            rxBuffer[rxCount++] = chr;
            calcCsum = UpdateChecksum(calcCsum, chr);
            DLEReceived |= true;
        }
        else if (DLEReceived) // last char was a DLE so this is a uint8_t stuffed DLE character in the data (unless the checksum is being received whicvh is not uint8_t stuffed
        {
            // ignore this char as it was uint8_t stuffed and is already in the rx buffer but do include it in the checksum
            calcCsum = UpdateChecksum(calcCsum, chr);
        }
        else  // add characters to array as ber normal  but set DLE flag
        {
            DLEReceived = true;
            rxBuffer[rxCount++] = chr;
            calcCsum = UpdateChecksum(calcCsum, chr);
        }

    }
    else if (chr == EOF && DLEReceived && !EOFReceived)  // this is the end of frame
    {
        rxBuffer[rxCount++] = chr;
        calcCsum = UpdateChecksum(calcCsum, chr);
        EOFReceived = true;
        csumByteReceived = 0;
        rcvCsum = 0;

    }
    else if (EOFReceived)  // receiving the checksum
    {
        rxBuffer[rxCount++] = chr;
        ++csumByteReceived;
        if (csumByteReceived >= 2)  // enf of checksum receivin and end of packet
        {
            rcvCsum = (uint16_t)((rxBuffer[rxCount - 2] * 0x100) + rxBuffer[rxCount - 1]);
            packetComplete = true;
            if (rcvCsum != calcCsum)
            {
                 csumError = true; 
            }
            else  
            {
            }
        }

    }
    else if (packetNAKed)  // the packet has returned an error - this is the last uint8_t which is an error code
    {
        errorCode = chr;
        packetComplete = true;
        rxBuffer[rxCount++] = chr;
    }
    else if (receivingPacket) // normal character - added to the buffer if a packet reception is in progress
    {
        if (DLEReceived && rxCount == 1) //  
        {
            command = chr;   // the second uint8_t in the packet (if a DLE was the first) is the command character
            if (command == NAK)
            {
                packetNAKed = true;
            }
            if (command == ACK)
            {
                packetACKed = true;
                packetComplete = true;
            }
        }
        rxBuffer[rxCount++] = chr;
        calcCsum = UpdateChecksum(calcCsum, chr);
    }

    if (chr != DLE) DLEReceived = false;

    frameTimeout = DYNAMENT_FRAME_TIMEOUT;

    if (packetComplete)  // a complete packet has actually been received  -act on it according to the various flags
    {
        if (packetACKed)
            ACKReceived();
        else if (packetNAKed)
            NAKReceived();
        else if (csumError)
            PacketChecksumError();
        else  // complete data packet received intact without error
        {
            ProcessReceivedPacket();
        }

        Reset();
    }
}

// Function: Create a packet of data in the Dynament protocol
// Inputs : cmd - the type of packet (read or write variable typically)
//          variableID - the id of the variable to be read or written
//          dlen - the number of bytes of data within the packet
//           dataPtr - pointer to array of bytes of the packet data
// Outputs: Returns true if oacket sent else returns false
bool SendDynamentPacket(uint8_t cmd, uint8_t variableID, uint8_t dlen, uint8_t *dataPtr)
{
    int x;
    uint8_t txBuf[DYNAMENT_MAX_PACKET_SIZE];
    short txBufPtr = 0;
    uint16_t csum = 0;

    txBuf[txBufPtr++] =DLE;
    csum = UpdateChecksum(csum,DLE);
    txBuf[txBufPtr++] = cmd;
    csum = UpdateChecksum(csum, (uint16_t) cmd);
    if (cmd == READ_VAR)
    {
        txBuf[txBufPtr++] = variableID;
        csum = UpdateChecksum(csum, (uint16_t) variableID);
    }
    else if (cmd == WRITE_REQUEST)
    {
        txBuf[txBufPtr++] = WRITE_PASSWORD_1;
        csum = UpdateChecksum(csum, (uint16_t) WRITE_PASSWORD_1);

        txBuf[txBufPtr++] = WRITE_PASSWORD_2;
        csum = UpdateChecksum(csum, (uint16_t) WRITE_PASSWORD_2);

        txBuf[txBufPtr++] = variableID;
        csum = UpdateChecksum(csum, (uint16_t) variableID);
    }

    if (dlen > 0)
    {
        if (dlen == DLE)
        {
            txBuf[txBufPtr++] = DLE;
            csum = UpdateChecksum(csum, (uint16_t) DLE);
        }
        txBuf[txBufPtr++] = dlen;
        csum = UpdateChecksum(csum, (uint16_t) dlen);
        for (x = 0; x < dlen; x++)
        {
            if (dataPtr[x] == DLE)
            {
                txBuf[txBufPtr++] = DLE;
                csum = UpdateChecksum(csum, (uint16_t) DLE);
            }
            txBuf[txBufPtr++] = dataPtr[x];
            csum = UpdateChecksum(csum, (uint16_t) dataPtr[x]);
        }
    }
    txBuf[txBufPtr++] = DLE;
    csum = UpdateChecksum(csum, (uint16_t) DLE);
    
    txBuf[txBufPtr++] = EOF;
    csum = UpdateChecksum(csum, (uint8_t) (EOF & 0xFF));

    // add checksum
    txBuf[txBufPtr++] = (uint8_t)((csum >> 8) & 0x00ff);
    txBuf[txBufPtr++] = (uint8_t)(csum & 0x00ff);


    if (dlen > (DYNAMENT_MAX_PACKET_SIZE))  // if there is too much data for a full length packet, then abort
        return false;


    // transmit packet to serial port
    p2pTxData(txBuf, txBufPtr);
    PacketSent();

    return true;
}



void ProcessReceivedPacket(void)
{
    uint8_t cmd = rxBuffer[1];
    uint8_t len = rxBuffer[2];
    int x;

    if (cmd == DAT)  // data packet response to a read variable  received
    {
        uint8_t rcvData[200];
        for (x = 0; x < len && (x + 3) < rxCount; x++)
        {
            rcvData[x] = rxBuffer[x + 3];
        }

        switch (currentMode)
        {
            case CommsModeRequestingLiveDataSimple: // waiting for a response to a read live data simple packet
                ReadLiveDataResponse(rcvData, len);
                break;
            case CommsModeRequestingLiveData2: // waiting for a response to a read live data 2 packet
                ReadLiveData2Response(rcvData, len);
                break;
        }
    }
    else
    {
        // do nothing
    }
}


// Function: Called when a data response to a read simple live data packet has been received 
// Inputs : dataPtr - pointer to the data contents of the packet
//          len - the number of bytes of data within the packet
void ReadLiveDataResponse(uint8_t *dataPtr, int len)
{
    float gasValue=0;

    // get the status values from the message
    uint16_t statusVal1 = (uint16_t) ((dataPtr[3] * 0x100) + (dataPtr[2]));

    // get the float value from the two registers that cover the actual reading
    uint32_t intVal = (uint32_t) ((dataPtr[7] * 0x1000000) + (dataPtr[6] * 0x10000) + (dataPtr[5] * 0x100) + (dataPtr[4]));
    float *fPtr = (float *) &intVal;
    gasValue = *fPtr;    

    // call the callback routine to rerpot the received gas reading to the calling layer
    if (currentMode== CommsModeRequestingLiveDataSimple && readGasCallBack!= 0)    
    {
        if (gasValue < -1 || statusVal1 > 0)  
        // any gas reading less than -1 is likely to be an error signal (e.g. sensor warming up or in fault) or any 
        // status flags set is likely to be an error too
            readGasCallBack (READ_RESPONSE_VALUE_INVALID, gasValue);
        else
            readGasCallBack (READ_RESPONSE_VALUE_VALID, gasValue);
        
    }
    currentMode = CommsModeIdle;
    readGasCallBack = 0;

}

// Function: Called when a data response to a read live data 2 packet has been received 
// Inputs : dataPtr - pointer to the data contents of the packet
//          len - the number of bytes of data within the packet
void ReadLiveData2Response(uint8_t *dataPtr, int len)
{
    float gasValue1=0;
    float gasValue2=0;

    // get the status values from the message
    uint16_t statusVal1 = (uint16_t) ((dataPtr[3] * 0x100) + (dataPtr[2]));
    uint16_t statusVal2 = (uint16_t) ((dataPtr[41] * 0x100) + (dataPtr[40]));

    // get the float value for gas reading 1 from the 4 bytes that cover the actual reading
    uint32_t intVal = (uint32_t) ((dataPtr[7] * 0x1000000) + (dataPtr[6] * 0x10000) + (dataPtr[5] * 0x100) + (dataPtr[4]));
    float *fPtr = (float *) &intVal;
    gasValue1 = *fPtr;    

    // get the float value for gas reading 2 from the 4 bytes that cover the actual reading
    intVal = (uint32_t) ((dataPtr[15] * 0x1000000) + (dataPtr[14] * 0x10000) + (dataPtr[13] * 0x100) + (dataPtr[12]));
    fPtr = (float *) &intVal;
    gasValue2 = *fPtr;    


    // call the callback routine to rerpot the received gas reading to the calling layer
    if (currentMode== CommsModeRequestingLiveData2 && readGasCallBack!= 0)    
    {
        if (statusVal1 > 0 || statusVal2 > 0)  // any status flags set indicate an invalid value
            readDualGasCallBack (READ_RESPONSE_VALUE_INVALID, gasValue1, gasValue2);
        else
            readDualGasCallBack (READ_RESPONSE_VALUE_VALID, gasValue1, gasValue2);
        
    }
    currentMode = CommsModeIdle;
    readGasCallBack = 0;

}


// Function: Called when an ACK paclet is received to a previously transmitted messages e.g. a Write Variable message 
//             - the data to be written can now be sent
void ACKReceived(void)
{
    // currently not implemented - but data for a write variable would be transmitted from here
}

// Function: Called when a NAK packet to a previously transmitted message is received
void NAKReceived(void)
{
    // call the callback routone with a status PACKET_IINVALID
    if (currentMode== CommsModeRequestingLiveDataSimple && readGasCallBack!= 0)
    {
        readGasCallBack (READ_RESPONSE_INVALID_REGISTER, 0);
    }
    currentMode = CommsModeIdle;
    readGasCallBack = 0;
}

// Function: Called when a received packet has detecged a checksum error
void PacketChecksumError(void)
{
    PacketTimedOut();  // treat the packet as having received no response
}

//Function: Called when no response has been received to a previously transmitted packet
void PacketTimedOut()
{
    if (currentMode== CommsModeRequestingLiveDataSimple && readGasCallBack!= 0)
    {
        readGasCallBack (READ_RESPONSE_TIMED_OUT, 0);
    }
    currentMode = CommsModeIdle;
    readGasCallBack = 0;

    Reset();
}


// Function: Send the command to read a live data packet to obtain the current gas reading
//Inputs: cb - pointer to callback routine to be called when a result has been determined
void RequestLiveDataSimple (GetFloat_CallBack_t cb )
{
    SendDynamentPacket(READ_VAR, LIVE_DATA_SIMPLE, 0, 0);
    readGasCallBack = cb;
    currentMode = CommsModeRequestingLiveDataSimple;;
}

// Function: Send the command to read a live data 2 packet to obtain the current gas reading and status
//Inputs: cb - pointer to callback routine to be called when a result has been determined
void RequestLiveData2 (GetDualFloat_CallBack_t cb )
{
    SendDynamentPacket(READ_VAR, LIVE_DATA_2, 0, 0);
    readDualGasCallBack = cb;
    currentMode = CommsModeRequestingLiveData2;;
}


/*** end of file ***/
