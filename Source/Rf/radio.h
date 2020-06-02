#ifndef RADIO_H
#define RADIO_H

#include "SPI1.h"	
#include "hal.h"
typedef enum
{
    RF_NONE,
    RF_BUSY,
    RF_RX_DONE,
    RF_RX_TIMEOUT,
    RF_RX_ID_ERROR,
    RF_TX_DONE,
    RF_TX_TIMEOUT,
    RF_LEN_ERROR,
    RF_CHANNEL_EMPTY,
    RF_CHANNEL_ACTIVITY_DETECTED
} tRFProcessReturnCodes;

extern int8 RxPacketSnrEstimate;
extern double RxPacketRssiValue;
extern uint8 RF_NUM;

void Rf_Check(void);
void Rf_StartRX(void);
void Rf_StartCAD(void);
void Rf_SleepMode( void );
void rfIRQIntHandler( void );
void Rf_SetTxPacket(uint8 *buffer, uint8 u8size);
void Rf_GetRxPacket(void *buffer, uint8 *u8size);

#endif

