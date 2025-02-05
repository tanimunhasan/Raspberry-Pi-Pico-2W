#ifndef _DYNAMENTCOMMS
#define _DYNAMENTCOMMS

#include <stdint.h>
#include "inc/ModbusComms.h"

/* Global type definitions ----------------------------------------------------*/
typedef void (*GetDualFloat_CallBack_t)(int valueStatus, float fval1, float fval2);


/* Global constants -----------------------------------------------------------*/


/* Global variables -----------------------------------------------------------*/

/* Function Prototypes --------------------------------------------------------*/
void InitialiseDynamentComms();
void DynamentCommsHandler();

void DecodeMessage(void);
void ReadMeasurand( uint16_t address, GetFloat_CallBack_t cb );
void MessageTimedOut(void);
void RequestLiveDataSimple (GetFloat_CallBack_t cb );
void RequestLiveData2 (GetDualFloat_CallBack_t cb );

#endif

/*** end of file ***/
