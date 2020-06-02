
#define RADIO_c

//#include "common.h"
//#include "hal.h"
#include "sx1276.h"
#include "sx1276_lora.h"
#include "sx1276_hal.h"
//#include "string.h"
#include "math.h"
int8 RxPacketSnrEstimate;
double RxPacketRssiValue;
uint8 RF_NUM = 0;


// ��ƵоƬ�쳣���
void Rf_Check(void)
{
	uint8_t TempReg=0,TempReg2=0;
	uint8_t i;
	/********���RF�Ĵ������ж���ƵоƬ�Ƿ�λ********/
	for(i = 0; i < 3; i++)
	{
		RF_NUM = i;
		SX1276Read(REG_LR_OPMODE, &TempReg);
		SX1276Read(REG_LR_SYNCWORD, &TempReg2);
		if(((TempReg & RFLR_OPMODE_LONGRANGEMODE_ON) != RFLR_OPMODE_LONGRANGEMODE_ON)
		   || (TempReg2 != SyncWord))
		{
			// ���³�ʼ����Ƶ
			DISABLE_GLOBAL_INTERRUPT();			
			SX1276Init();
			Rf_StartRX();
	//		Rf_SleepMode();
			ENABLE_GLOBAL_INTERRUPT();
		}
	}
	/********���RF�Ĵ������ж���ƵоƬ�Ƿ�λ********/
}

/*******************************************************************************
 * Function Name: Rf_StartRX
 * Decription   : �������ģʽ
 * Calls        : 
 * Called By    :
 * Arguments    :            
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Rf_StartRX( void )
{
//	clr_ES;
	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);	// ���ù���ģʽ-����ģʽ
	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);	// ���ù���ģʽ-����ģʽ
//	set_ES;
	SX1276LoRaSetPreambleLength(0x55F0);	//22000 ���ֵ65531 fffb
    SX1276Write(REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
									//RFLR_IRQFLAGS_RXDONE |
									//RFLR_IRQFLAGS_PAYLOADCRCERROR |
									RFLR_IRQFLAGS_VALIDHEADER |
									RFLR_IRQFLAGS_TXDONE |
										RFLR_IRQFLAGS_CADDONE |
											RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
												RFLR_IRQFLAGS_CADDETECTED);
    
    
    SX1276Write(REG_LR_HOPPERIOD, 0);	//By the FreqHopping is disabled
    
    								// RxDone					 RxTimeout					
									// FhssChangeChannel			 CadDone
	SX1276Write(REG_LR_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 
										| RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00);
  								    // CadDetected				 ModeReady
	SX1276Write(REG_LR_DIOMAPPING2, RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00); 

    SX1276Write(REG_LR_FIFOADDRPTR, 0x00);
    
//    comMEMSET(RFBuffer, 0, (uint8)RF_BUFFER_SIZE);
//	clr_ES;
    SX1276LoRaSetOpMode(RFLR_OPMODE_RECEIVER);	// ���ù���ģʽ-��������
	SX1276LoRaSetOpMode(RFLR_OPMODE_RECEIVER);	// ���ù���ģʽ-��������
//	set_ES;
}

/*******************************************************************************
 * Function Name: Rf_StartCAD
 * Decription   : �������ģʽ
 * Calls        : 
 * Called By    :
 * Arguments    :            
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Rf_StartCAD( void )
{
	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);

    SX1276Write(REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
									RFLR_IRQFLAGS_RXDONE |
									RFLR_IRQFLAGS_PAYLOADCRCERROR |
									RFLR_IRQFLAGS_VALIDHEADER |
									RFLR_IRQFLAGS_TXDONE |
									//	RFLR_IRQFLAGS_CADDONE |
											RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL );//|
									//			RFLR_IRQFLAGS_CADDETECTED);
    
    
    								// CadDone					 RxTimeout					
									// FhssChangeChannel			 CadDone
    SX1276Write(REG_LR_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_10 | RFLR_DIOMAPPING1_DIO1_00 
										| RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00);
  								    // CadDetected				 ModeReady
	SX1276Write(REG_LR_DIOMAPPING2, RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00); 
	
    SX1276LoRaSetOpMode(RFLR_OPMODE_CAD);
//	SET_PIN = 1;
	
}

/*******************************************************************************
 * Function Name: Rf_SleepMode
 * Decription   : ��������ģʽ
 * Calls        : 
 * Called By    :
 * Arguments    :            
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Rf_SleepMode( void )
{
	SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
	SX1276Write(REG_LR_IRQFLAGSMASK,	0xFF);	//���������ж�
	SX1276Write( REG_LR_DIOMAPPING1, 0X00 );
	SX1276Write( REG_LR_DIOMAPPING2, 0X00 );	
	SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );	
}
/*******************************************************************************
 * Function Name: Rf_SetTxPacket
 * Decription   : ���߷��ͺ���
 * Calls        : 
 * Called By    :
 * Arguments    : buffer: �������߶˷������ݣ�u8size: �������߶˷������ݳ���               
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Rf_SetTxPacket(uint8 *buffer, uint8 u8size)
{
////	stRf.RfState = 1;
////	if(((punModuleParam->stWorkModeStruct3_4.u8WakePack == true)||(punModuleParam->stWorkModeStruct3_4.u8WorkMode == WORK_MODE2_WAKEUP))
////	   &&(punModuleParam->stWorkModeStruct3_4.u8WorkMode != WORK_MODE5_IO)
////	   &&(punModuleParam->stWorkModeStruct3_4.u8WorkMode != WORK_MODE0_BROADCAST_L))
////		stRf.u16Timer = (RF_TIMEOUT_TX + ((punModuleParam->stWorkModeStruct3_4.u8WakeTime+1) * 600));
////	else
////		stRf.u16Timer = RF_TIMEOUT_TX;
	
	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY) ;
	SX1276LoRaSetPreambleLength(8);
    SX1276Write(REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
										RFLR_IRQFLAGS_RXDONE |
										  RFLR_IRQFLAGS_PAYLOADCRCERROR |
											RFLR_IRQFLAGS_VALIDHEADER |
											  //RFLR_IRQFLAGS_TXDONE |
											  RFLR_IRQFLAGS_CADDONE |
												RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
												  RFLR_IRQFLAGS_CADDETECTED);
  
    SX1276Write(REG_LR_PAYLOADLENGTH, u8size);
    
    SX1276Write(REG_LR_FIFOTXBASEADDR, 0x00);
    
    SX1276Write(REG_LR_FIFOADDRPTR, 0x00);
    SX1276WriteFifo(buffer, u8size);

    SX1276Write(REG_LR_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO1_00 | 
									RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00);

	SX1276Write(REG_LR_DIOMAPPING2, RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00); 
    
    
//    SX1276LoRaSetOpMode(RFLR_OPMODE_SYNTHESIZER_TX);
    SX1276LoRaSetOpMode(RFLR_OPMODE_TRANSMITTER);
	
}

/*******************************************************************************
 * Function Name: Rf_GetRxPacket
 * Decription   : ���߽��պ���
 * Calls        : 
 * Called By    :
 * Arguments    : buffer: �������߶˽������ݣ�u8size: �������߶˽������ݳ���               
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Rf_GetRxPacket(void *buffer, uint8 *u8size)
{
	uint8 TempReg;
//	*size = RxPacketSize;
	
	SX1276Read(REG_LR_PKTSNRVALUE, &TempReg);
	if(TempReg & 0x80) // The SNR sign bit is 1
	{
        // Invert and divide by 4
        RxPacketSnrEstimate = ((~TempReg + 1) & 0xFF) >> 2;
        RxPacketSnrEstimate = -RxPacketSnrEstimate;
	}
	else
	{
        // Divide by 4
        RxPacketSnrEstimate = (TempReg & 0xFF) >> 2;
	}
	
	if(LoRaSettings.RFFrequency < 860000000)  // LF
    {
      SX1276Read(REG_LR_PKTRSSIVALUE, &TempReg);
      RxPacketRssiValue = 161.0 - (double)TempReg;
      if(RxPacketSnrEstimate < 0 && RxPacketRssiValue > 117)
      {
        RxPacketRssiValue = 119 - RxPacketSnrEstimate;
      }
    }
    else                                        // HF
    {
      SX1276Read(REG_LR_PKTRSSIVALUE, &TempReg);
      RxPacketRssiValue = -161.0 + (double)TempReg;
      if(RxPacketSnrEstimate < 0 && RxPacketRssiValue < -117)
      {
        RxPacketRssiValue = -119 + RxPacketSnrEstimate;
      }
    }
	
	SX1276Read(REG_LR_FIFORXCURRENTADDR, &TempReg);
	SX1276Write(REG_LR_FIFOADDRPTR, TempReg);
	
	
////	if(punModuleParam->stWorkModeStruct3_4.u8WorkMode == WORK_MODE0_BROADCAST_L)
////	{
////		SX1276ReadFifo(&TempReg, 1);	// ��ȡ��һ������
////		if(TempReg != 0x78)
////			return;
////		SX1276Read(REG_LR_NBRXBYTES, &TempReg);
////		if(TempReg > RF_BUF_LEN)	// �ж��ֽڳ����Ƿ�Ϸ�
////			return;
////		*u8size = (TempReg - 1);
////		SX1276ReadFifo(buffer, TempReg-1);
////	}
////	else
	{
		SX1276Read(REG_LR_NBRXBYTES, &TempReg);
		if(TempReg > 30)	// �ж��ֽڳ����Ƿ�Ϸ�
			return;
		*u8size = TempReg;
		SX1276ReadFifo(buffer, TempReg);
	}
}


/*******************************************************************************
 * Function Name: rfIRQIntHandler
 * Decription   : �����շ��ж�(DIO0)������
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
uint8 tempnum;
extern uint16 CADTim;
extern uint8 u8buff[];
uint16 u16RstCount;

void rfIRQIntHandler( void )
{
	uint8 TempReg = 0;
	uint8 i;
	uint16 u16CRC;
	uint8	u8len;
	uint8	u8RxBuff[30];
//	if(DIO0_READ())
    {
		SX1276Read(REG_LR_IRQFLAGS, &TempReg);
		SX1276Write(REG_LR_IRQFLAGS,0xFF);
		if((TempReg & RFLR_IRQFLAGS_CADDONE) == RFLR_IRQFLAGS_CADDONE)
		{
			if((TempReg & RFLR_IRQFLAGS_CADDETECTED) == RFLR_IRQFLAGS_CADDETECTED)
			{	 				
				//��ʾCAD ��⵽��Ƶ�ź� ģ������˽���״̬����������
				Rf_StartRX();
////				stRf.u16Timer = 10000;	// ���ճ�ʱ���� 10s			
			}
			else
			{					   
				// û��⵽
				Rf_SleepMode();
////				stRf.u16Timer = 0;				
			}
		}
		
		else if((TempReg & RFLR_IRQFLAGS_PAYLOADCRCERROR) == RFLR_IRQFLAGS_PAYLOADCRCERROR)
		{
			// Clear Irq
			SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR);
			
			Rf_StartRX();
////			stRf.u16Timer = 1000;
		}
		else if( (TempReg & RFLR_IRQFLAGS_RXDONE) == RFLR_IRQFLAGS_RXDONE)
		{
			SX1276Write(REG_LR_IRQFLAGS,RFLR_IRQFLAGS_RXDONE);
	
			u8len = 0;
			Rf_GetRxPacket(u8RxBuff, &u8len);
			
			Rf_StartRX();

			if(u8len == 0)
			{
//				stRf.RfState = 0;
				return;
			}
			if(u8len == 19)
			{
//////				xTimerStartFromISR(m_heart_timerout_handle,NULL);
				u16CRC = ((u8RxBuff[18] << 8) | u8RxBuff[17]);
				u16RstCount = comCalCRC16(&u8RxBuff[0],17,0x8408);
				if((comCalCRC16(&u8RxBuff[0],17,0x8408) == u16CRC))
				{
					for(i=0; i<3; i++)
					{
						if((u8RxBuff[5+4*i]|(u8RxBuff[5+4*i+1]<<8)) != 0x00)
						{
//							u8RxBuff[5+4*i+2] = (uint8)(pow(10,(u8RxBuff[5+4*i+3]-63)/(10*3.2))*10);
							u8RxBuff[5+4*i+2] = (uint8)(pow(10,(u8RxBuff[5+4*i+3]-m_rssi_Calibration)/m_cfgpara.factor)*10);
						}
					}
					DISABLE_GLOBAL_INTERRUPT();
////					taskENTER_CRITICAL();
					insert_data_into_double_link(&RADIO_DATA_LIST_HEAD,&u8RxBuff[2],15);
////					taskEXIT_CRITICAL();
					ENABLE_GLOBAL_INTERRUPT();
				}
			}
		}
		else if( (TempReg & RFLR_IRQFLAGS_TXDONE) == RFLR_IRQFLAGS_TXDONE)
		{
//			SET_PIN = 0;
			SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE);
			{
				Rf_StartRX();	//���¿�������
			}
		}
		Rf_StartRX();
    }
//	else
//	{
//		//stRf.RfState = 0;
//		Rf_StartRX();	//���¿�������
//	//	return;
//	}

}
