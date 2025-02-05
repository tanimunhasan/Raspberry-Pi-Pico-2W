
#ifndef _COMMS
#define _COMMS

#include<stdint.h>
#include<stdbool.h>

#define P2P_BUFFER_SIZE  256

enum{p2pRxNone, p2pRxOk, p2pRxError};

extern volatile int frameTimeout;
extern volatile int messageTimeout;
extern volatile bool frameComplete;
extern volatile bool messageTimeOut;

void initialise_comms(void);
void p2pTxData(uint8_t* pucData, int len);
void p2pTxByte(uint8_t ucData);
uint8_t p2pRxByte(uint8_t* pucData);


#endif