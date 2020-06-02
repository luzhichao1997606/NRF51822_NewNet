#include "radio.h"

#include "common.h"
#include "SPI1.h"	


/*!
 * SX1276 registers variable
 */

bool LoRaOn = true;
static bool LoRaOnState = false;

//uint32 u32RfPreWorkTime;
//uint8 u8RfAddr[6]={0X05,0X00,0X00,0X00,0X00,0X00};
//uint16 u8SendWakeSignalTimer = 0;


//en_RF_FLG u8RfWorkFlg =RF_INIT_WORK; //RF_INIT_WORK;// RF_INTO_SLEEP;
////void  ReadLoraOnVal(void);
////void  ReadLoraOnVal(void)
////{
////  //if(halPara[MODULE_WORKMODE]==LORA_MODE)
////     LoRaOn = true;
//////  RFLRWaitTimeout = 100;
////}

void SX1276Init(void)
{
	uint8_t TempReg;
	// Initialize FSK and LoRa registers structure
	//ReadLoraOnVal();
	//ReadCenterFre();
//	SX1276 = (tSX1276*)SX1276Regs;
//	SX1276LR = (tSX1276LR*)SX1276Regs;
////	hal_Delay1ms(10);
	LoRaOnState = false;
	SX1276Reset();
//	for(u32delay= 0; u32delay < 0xFFFF; u32delay++);
//	while(halSpiReadByte(REG_VERSION) != 0x12);
	do
    {
		SX1276Write( REG_OPMODE, 0x08 );
		SX1276Read( REG_OPMODE, &TempReg);
	} while (TempReg != 0x08 );
	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY); // add by leo for test 20131114
	
	SX1276SetLoRaOn(LoRaOn);
	if(LoRaOn==false)
	{   // Initialize FSK modem
		SX1276FskInit();
	}
	else
	{	// Initialize LoRa modem
		SX1276LoRaInit();
	}
}
  
  
  
void SX1276SetRFFrequency(uint32 freq)
{
    uint8_t RegFrf[3];
	
//    freq*=1000;

	freq = (uint32)((double)freq / (double)FREQ_STEP);
	// SX1276SetOpMode(RF_OPMODE_STANDBY);
	RegFrf[0] = (uint8)((freq >> 16) & 0xFF);
	RegFrf[1] = (uint8)((freq >> 8) & 0xFF);
	RegFrf[2] = (uint8)(freq & 0xFF);
	SX1276WriteBuffer(REG_FRFMSB, RegFrf, 3);
//	SX1276ReadBuffer(REG_FRFMSB, &SX1276->RegFrfMsb, 3);
	
	SX1276Read(REG_PACONFIG, RegFrf);
	
	
	RegFrf[0] = (RegFrf[0] & RF_PACONFIG_PASELECT_MASK) | RF_PACONFIG_PASELECT_PABOOST;
	SX1276Write(REG_PACONFIG, RegFrf[0]);
	
	
}

void SX1276Reset(void)
{
	if(RF_NUM == 0)
	{
		RF_RST0_OUT();       
		RF_RST0_LOW();
	}
	else if(RF_NUM == 1)
	{
		RF_RST1_OUT();       
		RF_RST1_LOW();
	}
	else if(RF_NUM == 2)
	{
		RF_RST2_OUT();       
		RF_RST2_LOW();
	}
//	RF_RST0_OUT();       
//	RF_RST0_LOW();
	nrf_delay_ms(2);	//5ms
	
	if(RF_NUM == 0)
	{
		RF_RST0_HIGH(); 
	}
	else if(RF_NUM == 1)
	{
		RF_RST1_HIGH(); 
	}
	else if(RF_NUM == 2)
	{
		RF_RST2_HIGH(); 
	}
//	RF_RST0_HIGH(); 
	nrf_delay_ms(10);	//5ms

}

void SX1276SetLoRaOn(bool enable)
{
	uint8_t TempReg;
    if(LoRaOnState == enable)
    {
        return;
    }
    LoRaOnState = enable;
    LoRaOn = enable;

    if(LoRaOn == true)
    {
        SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);
        SX1276Read(REG_LR_OPMODE, &TempReg);
        TempReg = (TempReg & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_ON;
        SX1276Write(REG_LR_OPMODE, TempReg);

        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
                                        // RxDone               RxTimeout                   FhssChangeChannel           CadDone
//        SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
		SX1276Write(REG_LR_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 
										| RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00);
                                        // CadDetected          ModeReady
//        SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
		SX1276Write(REG_LR_DIOMAPPING2, RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00); 
//        SX1276WriteBuffer(REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2);

//        SX1276ReadBuffer(REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1);
    }
    else
    {
        SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);

        SX1276Read(REG_LR_OPMODE, &TempReg);
        TempReg = (TempReg & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_OFF;
        SX1276Write(REG_LR_OPMODE, TempReg);

        SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

//        SX1276ReadBuffer(REG_OPMODE, SX1276Regs + 1, 0x70 - 1);
    }
}

bool SX1276GetLoRaOn(void)
{
    return LoRaOn;
}

void SX1276SetOpMode(uint8 opMode)
{
    if(LoRaOn == false)
    {
        SX1276FskSetOpMode(opMode);
    }
    else
    {
        SX1276LoRaSetOpMode(opMode);
    }
}

//uint8 SX1276GetOpMode(void)
//{
//    if(LoRaOn == false)
//    {
//        return SX1276FskGetOpMode();
//    }
//    else
//    {
//        return SX1276LoRaGetOpMode();
//    }
//}

//double SX1276ReadRssi(void)
//{
//    if(LoRaOn == false)
//    {
//        return SX1276FskReadRssi();
//    }
//	else 
//	  return 0;
//   
//}

//uint8 SX1276ReadRxGain(void)
//{
//    if(LoRaOn == false)
//    {
//        return SX1276FskReadRxGain();
//    }
//    else
//    {
//        return SX1276LoRaReadRxGain();
//    }
//}

//uint8 SX1276GetPacketRxGain(void)
//{
//    if(LoRaOn == false)
//    {
//        return SX1276FskGetPacketRxGain();
//    }
//    else
//    {
//        return SX1276LoRaGetPacketRxGain();
//    }
//}

int8 SX1276GetPacketSnr(void)
{
    if(LoRaOn == true)
    {
        return SX1276LoRaGetPacketSnr();
    }
//	else
	  return 0;
}

double SX1276GetPacketRssi(void)
{
    if(LoRaOn == false)
    {
        return SX1276FskGetPacketRssi();
    }
//    else
    {
        return SX1276LoRaGetPacketRssi();
    }
}

uint32 SX1276GetPacketAfc(void)
{
    if(LoRaOn == false)
    {
        return SX1276FskGetPacketAfc();
    }
//	else
	  return 0;
	
}

void SX1276StartRx(void)
{
    if(LoRaOn == true)
    {  
        SX1276LoRaSetRFState(RFLR_STATE_RX_INIT);
    }
}

void SX1276StartCad(void)
{
	if(LoRaOn == true)
	{
		SX1276LoRaSetRFState(RFLR_STATE_CAD_INIT);
	}
}

////void SX1276GetRxPacket(void *buffer, uint8 *u8size)
////{
////    if(LoRaOn == true)    
////    {
////        SX1276LoRaGetRxPacket(buffer, u8size);
////    }

////}

////void SX1276SetTxPacket(const void *buffer, uint16 u8size)
////{
////    SX1276LoRaSetTxPacket(buffer, u8size);
////}


uint32 SX1276Process(void)
{
//    return SX1276LoRaProcess();
	return 0;
}



void SX1276Sleep(void)
{

//  if(LoRaOn == false)
//  {
//	SX1276FskSlp();
//        
//  }
//  else
//  {
//	SX1276LoRaSlp();
//  }
//   RF_PA_OFF();
//   RF_LNA_OFF();
  
}

//void SX1276DetectRadio(void)
//{
//  //SX1276InitIo();
//  if(LoRaOn == true)
//  {
//	SX1276LoRaDetectRadio();
//  }
//}
//void SX1276ResetFre(void)
//{
//  
////   if(LoRaOn == false)
////  {
////  // SX1276SetOpMode(RF_OPMODE_STANDBY);
////   //SX1276SetOpMode(RF_OPMODE_RECEIVER);
////  }
//}
