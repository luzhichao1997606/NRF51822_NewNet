
#define LORA_C
                  
#include "SPI1.h"	

//#include <string.h>

/*!
* Constant values need to compute the RSSI value
*/
#define RSSI_OFFSET_LF                              -155.0
#define RSSI_OFFSET_HF                              -150.0

#define NOISE_ABSOLUTE_ZERO                         -174.0

#define NOISE_FIGURE_LF                                4.0
#define NOISE_FIGURE_HF                                6.0


void SX1276LoRaReset(void);


/*!
* Precomputed signal bandwidth log values
* Used to compute the Packet RSSI value.
*/
//const double SignalBwLog[] =
//{
//  3.8927900303521316335038277369285,  // 7.8 kHz
//  4.0177301567005500940384239336392,  // 10.4 kHz
//  4.193820026016112828717566631653,   // 15.6 kHz
//  4.31875866931372901183597627752391, // 20.8 kHz
//  4.4948500216800940239313055263775,  // 31.2 kHz
//  4.6197891057238405255051280399961,  // 41.6 kHz
//  4.795880017344075219145044421102,   // 62.5 kHz
//  5.0969100130080564143587833158265,  // 125 kHz
//  5.397940008672037609572522210551,   // 250 kHz
//  5.6989700043360188047862611052755   // 500 kHz
//};



/*!
* Frequency hopping frequencies table
*/


// Default settings
tLoRaSettings LoRaSettings =
{
  433000000,        // RFFrequency	收发频率
  20,               // Power	发射功率
  7,                // SignalBw [0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,	带宽
  // 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved]
  9,               // SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]扩频因子
  2,                // ErrorCoding [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]	循环纠错编码
  true,             // CrcOn [0: OFF, 1: ON]	CRC校验
  false,             // ImplicitHeaderOn [0: OFF, 1: ON]	 序头模式
  0,                // RxSingleOn [0: Continuous, 1 Single]	 接收模式中的single模式开关  0代表continue模式
  0,                // FreqHopOn [0: OFF, 1: ON]	跳频开关
  4,                // HopPeriod Hops every frequency hopping period symbols	跳频周期
  100,              // TxPacketTimeout	发送超时时间
  100,              // RxPacketTimeout	接收超时时间
  255,              // PayloadLength (used for implicit header mode)	负载数据长度
};

const uint8_t DataRateTab[][3] = {
		//BW,SF,EC
		{ 7,10, 2},             //0.81K
		{ 7, 9, 2},              //1.46K
		
//		{ 7, 7, 1},              //1.46K
		{ 7, 8, 2},              //2.6K
		{ 7, 7, 2},              //4.56K
		
//		{ 9, 8, 1}, 			 // 10.4k
		{ 8, 7, 2},              //9.11
//		{ 9, 7, 2},              //18.23
		{ 9, 7, 1},				 // 21.87
};

const uint16_t PreambleLengthTab[][3] = {
	{  35,  408,  655},
	{  75,  820, 1310},
	{ 152, 1640, 2620},
	{ 310, 3300, 5250},
	{ 625, 6500,10500},
	{1250,13000,21000},
};
/*!
* SX1276 LoRa registers variable
*/
//tSX1276LR* SX1276LR;

/*!
* Local RF buffer for communication support
*/
////static uint8 RFBuffer[RF_BUFFER_SIZE];


/*!
* RF state machine variable
*/
uint8 RFLRState = RFLR_STATE_RX_INIT;
//uint32 RFLRWaitTimeout = 0;

/*!
* Rx management support variables
*/
uint16 RxPacketSize = 0;
uint8 SyncWord = 0x16;

static uint8 RxGain = 1;

//static uint32 RxTimeoutTimer = 0;
/*!
* PacketTimeout Stores the Rx window time value for packet reception
*/


/*!
* Tx management support variables
*/
////static uint16 TxPacketSize = 0;

void SX1276LoRaInit(void)
{
	
	uint8 RfPower_st,RfRate_st;
	RFLRState = RFLR_STATE_RX_INIT;
	
	
	SX1276Write(REG_LR_LNA, RFLR_LNA_GAIN_G1);
////	if(punModuleParam->stWorkModeStruct3_4.u8WorkMode != WORK_MODE5_IO)
////	{
////		RfPower_st = punModuleParam->stWorkModeStruct3_4.u8RfPower;
////		if(punModuleParam->stWorkModeStruct3_4.u8WorkMode == WORK_MODE0_BROADCAST_L)	// 长包模式下，无线波特率跟随串口波特率自匹配
////		{
////			if(punModuleParam->stWorkModeStruct3_4.u8ComRate <= COM_19200)
////				RfRate_st = punModuleParam->stWorkModeStruct3_4.u8ComRate;
////			else
////				RfRate_st = COM_19200;
////		}
////		else
////			RfRate_st = punModuleParam->stWorkModeStruct3_4.u8RfRate;
////	}
////	else
////	{
////		RfPower_st = punModuleParam->stWorkModeStruct5.u8RfPower;
////		RfRate_st = punModuleParam->stWorkModeStruct5.u8RfRate;
////	}
	RfPower_st = 7;
	RfRate_st = 5;
	switch (RfPower_st)
    {
		case 0:
			LoRaSettings.Power = 2;
			break;
		case 1:
			LoRaSettings.Power = 5;
			break;
		case 2:
			LoRaSettings.Power = 8;
			break;
		case 3:
			LoRaSettings.Power = 11;
			break;
		case 4:
			LoRaSettings.Power = 14;
			break;
		case 5:
			LoRaSettings.Power = 15;
			break;
		case 6:
			LoRaSettings.Power = 17;
			break;
		case 7:
			LoRaSettings.Power = 20;
			break;
		default:
            LoRaSettings.Power = 20;
            break;
	}
	
////	if(punModuleParam->stWorkModeStruct3_4.u8WorkMode != WORK_MODE5_IO)
////		LoRaSettings.RFFrequency = (433 + punModuleParam->stWorkModeStruct3_4.u8RfChannel) * 1000000;
////	else
////		LoRaSettings.RFFrequency = (433 + punModuleParam->stWorkModeStruct5.u8RfChannel) * 1000000;
	if(RF_NUM == 0)
	{
		LoRaSettings.RFFrequency = (433 + 4) * 1000000;
	}
	else if(RF_NUM == 1)
	{
		LoRaSettings.RFFrequency = (433 + 7) * 1000000;
	}
	else if(RF_NUM == 2)
	{
		LoRaSettings.RFFrequency = (433 + 10) * 1000000;
	}
//	LoRaSettings.RFFrequency = (433 + 4) * 1000000;
	
	//SX1276Write(REG_LR_SYMBTIMEOUTLSB, 0xff);
	// set the RF settings
	SX1276SetRFFrequency(LoRaSettings.RFFrequency);
	SX1276Write(REG_LR_SYNCWORD,SyncWord);

	if(LoRaSettings.Power < 5)
		SX1276LoRaSetPa20dBm(false);
	else
		SX1276LoRaSetPa20dBm(true);
	SX1276LoRaSetRFPower(LoRaSettings.Power);
	SX1276LoRaSetSignalBandwidth(DataRateTab[RfRate_st][0]);
	SX1276LoRaSetSpreadingFactor(DataRateTab[RfRate_st][1]); // SF6 only operates in implicit header mode.
	SX1276LoRaSetErrorCoding(DataRateTab[RfRate_st][2]);
	SX1276LoRaSetPacketCrcOn(LoRaSettings.CrcOn);
	
////	if(((punModuleParam->stWorkModeStruct3_4.u8WakePack == true)||(punModuleParam->stWorkModeStruct3_4.u8WorkMode == WORK_MODE2_WAKEUP))
////	   &&(punModuleParam->stWorkModeStruct3_4.u8WorkMode != WORK_MODE5_IO)
////	   &&(punModuleParam->stWorkModeStruct3_4.u8WorkMode != WORK_MODE0_BROADCAST_L))
////	{
////		if(punModuleParam->stWorkModeStruct3_4.u8WakeTime <= 7)
////			stRf.u16PreambleLength = (PreambleLengthTab[RfRate_st][0]*(punModuleParam->stWorkModeStruct3_4.u8WakeTime + 1));
////		else if(punModuleParam->stWorkModeStruct3_4.u8WakeTime == 8)
////			stRf.u16PreambleLength = PreambleLengthTab[RfRate_st][1];
////		else if(punModuleParam->stWorkModeStruct3_4.u8WakeTime == 9)
////			stRf.u16PreambleLength = PreambleLengthTab[RfRate_st][2];
////		
////	}
////	else
////		stRf.u16PreambleLength = 8;
	SX1276LoRaSetPreambleLength(8);
	SX1276LoRaSetImplicitHeaderOn(LoRaSettings.ImplicitHeaderOn);
	SX1276LoRaSetSymbTimeout(0x3FF);
	SX1276LoRaSetPayloadLength(LoRaSettings.PayloadLength);
	if(RfRate_st < 2)
		SX1276LoRaSetLowDatarateOptimize(true);
	else
		SX1276LoRaSetLowDatarateOptimize(false);
	
	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
}

//void SX1276LoRaSetDefaults(void)
//{
//  // REMARK: See SX1276 datasheet for modified default values.
//  
//  SX1276Read(REG_LR_VERSION, &SX1276LR->RegVersion);
//}

void SX1276LoRaReset(void)
{
  //uint32 startTick;
  //
  //    SX1276SetReset(RADIO_RESET_ON);
  //
  //    // Wait 1ms
  //    halWait(1);
  //
  //    SX1276SetReset(RADIO_RESET_OFF);
  //
  //    halWait(6);
}

void SX1276LoRaSetOpMode(uint8 opMode)
{
	uint8_t RegOpMode;
////	if(opMode==RFLR_OPMODE_TRANSMITTER)
////	{
////		TX_EN_ON();
////		RX_EN_OFF();
////		PA_EN_ON();
////	}
////	else 
////	{ 
////		TX_EN_OFF();
////		RX_EN_ON();
////		PA_EN_OFF();
////	}
	SX1276Read(REG_LR_OPMODE, &RegOpMode);
	RegOpMode = (RegOpMode & RFLR_OPMODE_MASK) | opMode;
	SX1276Write(REG_LR_OPMODE, RegOpMode);
	// SX1276Write(REG_LR_OPMODE, SX1276LR->RegOpMode);
	
//	SX1276Read(REG_LR_OPMODE, &SX1276LR->RegOpMode);  
	//  SX1276Read(REG_LR_OPMODE, &SX1276LR->RegOpMode); 
  
}

uint8 SX1276LoRaGetOpMode(void)
{
	uint8_t RegOpMode;
    SX1276Read(REG_LR_OPMODE, &RegOpMode);

    return RegOpMode & ~RFLR_OPMODE_MASK;
}

uint8 SX1276LoRaReadRxGain(void)
{
	uint8_t RegLna;
  SX1276Read(REG_LR_LNA, &RegLna);
  return(RegLna >> 5) & 0x07;
}

//double SX1276LoRaReadRssi(void)
//{
//    // Reads the RSSI value
//    SX1276Read(REG_LR_RSSIVALUE, &SX1276LR->RegRssiValue);
//
//    if(LoRaSettings.RFFrequency < 860000000)  // LF
//    {
//        return -155 + (double)SX1276LR->RegRssiValue;
//    }
//    else
//    {
//        return -150 + (double)SX1276LR->RegRssiValue;
//    }
//}

uint8 SX1276LoRaGetPacketRxGain(void)
{
  return RxGain;
}

int8 SX1276LoRaGetPacketSnr(void)
{
  return RxPacketSnrEstimate;
}

double SX1276LoRaGetPacketRssi(void)
{
  return RxPacketRssiValue;
}

void SX1276LoRaStartRx(void)
{
  SX1276LoRaSetRFState(RFLR_STATE_RX_INIT);
}

////void SX1276LoRaGetRxPacket(void *buffer, uint8 *u8size)
////{
////  *u8size = RxPacketSize;
////  RxPacketSize = 0;
////  comMEMCPY(buffer, RFBuffer, (uint8)*u8size);
////}

////void SX1276LoRaSetTxPacket(const void *buffer, uint16 u8size)
////{
////  //   if(RFLRState !=RFLR_STATE_RX_RUNNING)
////  //	 return ;
////  if(LoRaSettings.FreqHopOn == false)
////  {
////    TxPacketSize = u8size;
////  }
////  else
////  {
////    TxPacketSize = 255;
////  }
////  comMEMCPY((void *)RFBuffer, buffer, (uint8)TxPacketSize);
////  
////  RFLRState = RFLR_STATE_TX_INIT;
////}

uint8 SX1276LoRaGetRFState(void)
{
  return RFLRState;
}

void SX1276LoRaSetRFState(uint8 state)
{
  RFLRState = state;
}
/*!
* SX1276 definitions
*/


//void SX1276LoRaSetRFFrequency(uint32 freq)
//{
//    LoRaSettings.RFFrequency = freq;
//
//    freq = (uint32)((double)freq / (double)FREQ_STEP);
//    SX1276LR->RegFrfMsb = (uint8)((freq >> 16) & 0xFF);
//    SX1276LR->RegFrfMid = (uint8)((freq >> 8) & 0xFF);
//    SX1276LR->RegFrfLsb = (uint8)(freq & 0xFF);
//    SX1276WriteBuffer(REG_LR_FRFMSB, &SX1276LR->RegFrfMsb, 3);
//
//    SX1276Read(REG_LR_PACONFIG, &SX1276LR->RegPaConfig);
//
//    if(LoRaSettings.RFFrequency > 860000000)
//    {
//        SX1276LR->RegPaConfig = (SX1276LR->RegPaConfig & RFLR_PACONFIG_PASELECT_MASK) | RFLR_PACONFIG_PASELECT_RFO;
//    }
//    else
//    {
//        SX1276LR->RegPaConfig = (SX1276LR->RegPaConfig & RFLR_PACONFIG_PASELECT_MASK) | RFLR_PACONFIG_PASELECT_PABOOST;
//    }
//    SX1276Write(REG_LR_PACONFIG, SX1276LR->RegPaConfig);
//}
//
//uint32 SX1276LoRaGetRFFrequency(void)
//{
//    SX1276ReadBuffer(REG_LR_FRFMSB, &SX1276LR->RegFrfMsb, 3);
//    LoRaSettings.RFFrequency = ((uint32)SX1276LR->RegFrfMsb << 16) | ((uint32)SX1276LR->RegFrfMid << 8) | ((uint32)SX1276LR->RegFrfLsb);
//    LoRaSettings.RFFrequency = (uint32)((double)LoRaSettings.RFFrequency * (double)FREQ_STEP);
//
//    return LoRaSettings.RFFrequency;
//}

void SX1276LoRaSetRFPower(int8 power)
{
	uint8_t RegPaConfig, RegPaDac;
  SX1276Read(REG_LR_PACONFIG, &RegPaConfig);
  SX1276Read(REG_LR_PADAC, &RegPaDac);
  
  if((RegPaConfig & RFLR_PACONFIG_PASELECT_PABOOST) == RFLR_PACONFIG_PASELECT_PABOOST)
  {
    if((RegPaDac & 0x87) == 0x87)
    {
      if(power < 5)
      {
        power = 5;
      }
      if(power > 20)
      {
        power = 20;
      }
      RegPaConfig = (RegPaConfig & RFLR_PACONFIG_MAX_POWER_MASK) | 0x70;
      RegPaConfig = (RegPaConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8)((uint16)(power - 5) & 0x0F);
    }
    else
    {
      if(power < 2)
      {
        power = 2;
      }
      if(power > 17)
      {
        power = 17;
      }
      RegPaConfig = (RegPaConfig & RFLR_PACONFIG_MAX_POWER_MASK) | 0x70;
      RegPaConfig = (RegPaConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8)((uint16)(power - 2) & 0x0F);
    }
  }
  else
  {
    if(power < -1)
    {
      power = -1;
    }
    if(power > 14)
    {
      power = 14;
    }
    RegPaConfig = (RegPaConfig & RFLR_PACONFIG_MAX_POWER_MASK) | 0x70;
    RegPaConfig = (RegPaConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8)((uint16)(power + 1) & 0x0F);
  }
  SX1276Write(REG_LR_PACONFIG, RegPaConfig);
  LoRaSettings.Power = power;
}

//int8 SX1276LoRaGetRFPower(void)
//{
//    SX1276Read(REG_LR_PACONFIG, &SX1276LR->RegPaConfig);
//    SX1276Read(REG_LR_PADAC, &SX1276LR->RegPaDac);
//
//    if((SX1276LR->RegPaConfig & RFLR_PACONFIG_PASELECT_PABOOST) == RFLR_PACONFIG_PASELECT_PABOOST)
//    {
//        if((SX1276LR->RegPaDac & 0x07) == 0x07)
//        {
//            LoRaSettings.Power = 5 + (SX1276LR->RegPaConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK);
//        }
//        else
//        {
//            LoRaSettings.Power = 2 + (SX1276LR->RegPaConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK);
//        }
//    }
//    else
//    {
//        LoRaSettings.Power = -1 + (SX1276LR->RegPaConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK);
//    }
//    return LoRaSettings.Power;
//}

void SX1276LoRaSetSignalBandwidth(uint8 bw)
{
	uint8_t RegModemConfig1;
  SX1276Read(REG_LR_MODEMCONFIG1, &RegModemConfig1);
  RegModemConfig1 = (RegModemConfig1 & RFLR_MODEMCONFIG1_BW_MASK) | (bw << 4);
  SX1276Write(REG_LR_MODEMCONFIG1, RegModemConfig1);
  LoRaSettings.SignalBw = bw;
}

//uint8 SX1276LoRaGetSignalBandwidth(void)
//{
//    SX1276Read(REG_LR_MODEMCONFIG1, &SX1276LR->RegModemConfig1);
//    LoRaSettings.SignalBw = (SX1276LR->RegModemConfig1 & ~RFLR_MODEMCONFIG1_BW_MASK) >> 4;
//    return LoRaSettings.SignalBw;
//}

void SX1276LoRaSetSpreadingFactor(uint8 factor)
{
	uint8_t RegModemConfig2;
  
  if(factor > 12)
  {
    factor = 12;
  }
  else if(factor < 6)
  {
    factor = 6;
  }
  
  if(factor == 6)
  {
    SX1276LoRaSetNbTrigPeaks(5);
  }
  else
  {
    SX1276LoRaSetNbTrigPeaks(3);
  }
  
  SX1276Read(REG_LR_MODEMCONFIG2, &RegModemConfig2);
  RegModemConfig2 = (RegModemConfig2 & RFLR_MODEMCONFIG2_SF_MASK) | (factor << 4);
  SX1276Write(REG_LR_MODEMCONFIG2, RegModemConfig2);
  
  LoRaSettings.SpreadingFactor = factor;
}

//uint8 SX1276LoRaGetSpreadingFactor(void)
//{
//    SX1276Read(REG_LR_MODEMCONFIG2, &SX1276LR->RegModemConfig2);
//    LoRaSettings.SpreadingFactor = (SX1276LR->RegModemConfig2 & ~RFLR_MODEMCONFIG2_SF_MASK) >> 4;
//    return LoRaSettings.SpreadingFactor;
//}

void SX1276LoRaSetErrorCoding(uint8 value)
{
	uint8_t RegModemConfig1;
  SX1276Read(REG_LR_MODEMCONFIG1, &RegModemConfig1);
  RegModemConfig1 = (RegModemConfig1 & RFLR_MODEMCONFIG1_CODINGRATE_MASK) | (value << 1);
  SX1276Write(REG_LR_MODEMCONFIG1, RegModemConfig1);
  LoRaSettings.ErrorCoding = value;
}

//uint8 SX1276LoRaGetErrorCoding(void)
//{
//    SX1276Read(REG_LR_MODEMCONFIG1, &SX1276LR->RegModemConfig1);
//    LoRaSettings.ErrorCoding = (SX1276LR->RegModemConfig1 & ~RFLR_MODEMCONFIG1_CODINGRATE_MASK) >> 1;
//    return LoRaSettings.ErrorCoding;
//}

void SX1276LoRaSetPacketCrcOn(uint8 enable)
{
	uint8_t RegModemConfig2;
	SX1276Read(REG_LR_MODEMCONFIG2, &RegModemConfig2);
	RegModemConfig2 = (RegModemConfig2 & RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK) | (enable << 2);
	SX1276Write(REG_LR_MODEMCONFIG2, RegModemConfig2);
	LoRaSettings.CrcOn = enable;
}

void SX1276LoRaSetPreambleLength(uint16 value)
{
	uint8_t RegPreamble[2];
	SX1276ReadBuffer(REG_LR_PREAMBLEMSB, RegPreamble, 2);
	
	RegPreamble[0] = (value >> 8) & 0x00FF;
	RegPreamble[1] = value & 0xFF;
	SX1276WriteBuffer(REG_LR_PREAMBLEMSB, RegPreamble, 2);
}

//uint16 SX1276LoRaGetPreambleLength(void)
//{
//    SX1276ReadBuffer(REG_LR_PREAMBLEMSB, &SX1276LR->RegPreambleMsb, 2);
//    return ((SX1276LR->RegPreambleMsb & 0x00FF) << 8) | SX1276LR->RegPreambleLsb;
//}
//
//uint8 SX1276LoRaGetPacketCrcOn(void)
//{
//    SX1276Read(REG_LR_MODEMCONFIG2, &SX1276LR->RegModemConfig2);
//    LoRaSettings.CrcOn = (SX1276LR->RegModemConfig2 & RFLR_MODEMCONFIG2_RXPAYLOADCRC_ON) >> 1;
//    return LoRaSettings.CrcOn;
//}

void SX1276LoRaSetImplicitHeaderOn(uint8 enable)
{
	uint8_t RegModemConfig1;
  SX1276Read(REG_LR_MODEMCONFIG1, &RegModemConfig1);
  RegModemConfig1 = (RegModemConfig1 & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) | (enable);
  SX1276Write(REG_LR_MODEMCONFIG1, RegModemConfig1);
  LoRaSettings.ImplicitHeaderOn = enable;
}

//uint8 SX1276LoRaGetImplicitHeaderOn(void)
//{
//    SX1276Read(REG_LR_MODEMCONFIG1, &SX1276LR->RegModemConfig1);
//    LoRaSettings.ImplicitHeaderOn = (SX1276LR->RegModemConfig1 & RFLR_MODEMCONFIG1_IMPLICITHEADER_ON);
//    return LoRaSettings.ImplicitHeaderOn;
//}

void SX1276LoRaSetRxSingleOn(uint8 enable)
{
  LoRaSettings.RxSingleOn = enable;
}

//uint8 SX1276LoRaGetRxSingleOn(void)
//{
//    return LoRaSettings.RxSingleOn;
//}

void SX1276LoRaSetFreqHopOn(uint8 enable)
{
  LoRaSettings.FreqHopOn = enable;
}

//uint8 SX1276LoRaGetFreqHopOn(void)
//{
//    return LoRaSettings.FreqHopOn;
//}

void SX1276LoRaSetHopPeriod(uint8 value)
{
//  SX1276LR->RegHopPeriod = value;
  SX1276Write(REG_LR_HOPPERIOD, value);
  LoRaSettings.HopPeriod = value;
}

//uint8 SX1276LoRaGetHopPeriod(void)
//{
//    SX1276Read(REG_LR_HOPPERIOD, &SX1276LR->RegHopPeriod);
//    LoRaSettings.HopPeriod = SX1276LR->RegHopPeriod;
//    return LoRaSettings.HopPeriod;
//}

//void SX1276LoRaSetTxPacketTimeout(uint32 value)
//{
//    LoRaSettings.TxPacketTimeout = value;
//}

//uint32 SX1276LoRaGetTxPacketTimeout(void)
//{
//    return LoRaSettings.TxPacketTimeout;
//}

//void SX1276LoRaSetRxPacketTimeout(uint32 value)
//{
//    LoRaSettings.RxPacketTimeout = value;
//}
//
//uint32 SX1276LoRaGetRxPacketTimeout(void)
//{
//    return LoRaSettings.RxPacketTimeout;
//}

void SX1276LoRaSetPayloadLength(uint8 value)
{
//  SX1276LR->RegPayloadLength = value;
  SX1276Write(REG_LR_PAYLOADLENGTH, value);
  LoRaSettings.PayloadLength = value;
}

//uint8 SX1276LoRaGetPayloadLength(void)
//{
//    SX1276Read(REG_LR_PAYLOADLENGTH, &SX1276LR->RegPayloadLength);
//    LoRaSettings.PayloadLength = SX1276LR->RegPayloadLength;
//    return LoRaSettings.PayloadLength;
//}

void SX1276LoRaSetPa20dBm(uint8 enale)
{
	uint8_t RegPaDac, RegPaConfig;
  SX1276Read(REG_LR_PADAC, &RegPaDac);
  SX1276Read(REG_LR_PACONFIG, &RegPaConfig);
  
  if((RegPaConfig & RFLR_PACONFIG_PASELECT_PABOOST) == RFLR_PACONFIG_PASELECT_PABOOST)
  {
    if(enale == true)
    {
      RegPaDac = 0x87;
    }
  }
  else
  {
    RegPaDac = 0x84;
  }
  SX1276Write(REG_LR_PADAC, RegPaDac);
}

//uint8 SX1276LoRaGetPa20dBm(void)
//{
//    SX1276Read(REG_LR_PADAC, &SX1276LR->RegPaDac);
//
//    return ((SX1276LR->RegPaDac & 0x07) == 0x07) ? true : false;
//}

void SX1276LoRaSetPaRamp(uint8 value)
{
	uint8_t RegPaRamp;
  SX1276Read(REG_LR_PARAMP, &RegPaRamp);
  RegPaRamp = ( uint8 )((RegPaRamp & RFLR_PARAMP_MASK) | (value & ~RFLR_PARAMP_MASK));
  SX1276Write(REG_LR_PARAMP, RegPaRamp);
}

//uint8 SX1276LoRaGetPaRamp(void)
//{
//    SX1276Read(REG_LR_PARAMP, &SX1276LR->RegPaRamp);
//    return SX1276LR->RegPaRamp & ~RFLR_PARAMP_MASK;
//}

void SX1276LoRaSetSymbTimeout(uint16 value)
{
	uint8_t RegBuf[2];
  SX1276ReadBuffer( REG_LR_MODEMCONFIG2, RegBuf, 2 );

    RegBuf[0] = ( RegBuf[0] & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) | ( ( value >> 8 ) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK );
    RegBuf[1] = value & 0xFF;
    SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RegBuf, 2 );
}

//uint16 SX1276LoRaGetSymbTimeout(void)
//{
//    SX1276ReadBuffer(REG_LR_MODEMCONFIG2, &SX1276LR->RegModemConfig2, 2);
//    return ((SX1276LR->RegModemConfig2 & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK) << 8) | SX1276LR->RegSymbTimeoutLsb;
//}

void SX1276LoRaSetLowDatarateOptimize(uint8 enable)
{
  uint8_t RegModemConfig3;
    SX1276Read( REG_LR_MODEMCONFIG3, &RegModemConfig3 );
    RegModemConfig3 = ( RegModemConfig3 & RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) | ( enable << 3 );
    SX1276Write( REG_LR_MODEMCONFIG3, RegModemConfig3 );
}

//uint8 SX1276LoRaGetLowDatarateOptimize(void)
//{
//    SX1276Read(REG_LR_MODEMCONFIG3, &SX1276LR->RegModemConfig3);
//    return ((SX1276LR->RegModemConfig3 & RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_ON) >> 3);
//}

void SX1276LoRaSetNbTrigPeaks(uint8 value)
{
  uint8_t RegDetectOptimize;
    SX1276Read( 0x31, &RegDetectOptimize );
    RegDetectOptimize = ( RegDetectOptimize & 0xF8 ) | value;
    SX1276Write( 0x31, RegDetectOptimize );
}

uint8 SX1276LoRaGetNbTrigPeaks(void)
{
	uint8_t RegTestReserved31;
  SX1276Read(0x31, &RegTestReserved31);
  return (RegTestReserved31 & 0x07);
}

void SX1276LoRaSlp(void)   
{
  
  RFLRState = RFLR_STATE_IDLE;
  SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
  SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);
  //SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);
}

void SX1276LoRaDetectRadio(void)
{
  SX1276StartCad();
  // u8RfWorkFlg = RF_WAKE_TO_DETECT;
}

