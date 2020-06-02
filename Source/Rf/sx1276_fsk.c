
#include "common.h"
#include "SPI1.h"	
#include "hal.h"

//#include <string.h>
//#include <math.h>
//#include <stdlib.h>
// Default settings

tFskSettings FskSettings = 
{
  // RFFrequency  Bitrate   Fdev    Power   RxBw    RxBwAfc
     433000000,    9600,     25000,  20,    100000, 150000,
               
  
};

/*!
 * SX1276 FSK registers variable
 */
//tSX1276* SX1276;

/*!
 * Local RF buffer for communication support
 */
////static uint8 RFBuffer[RF_BUFFER_SIZE];

/*!
 * Chunk u8size of data write in buffer 
 */

////static double RxPacketRssiValue;
static uint32 RxPacketAfcValue;
static uint8 RxGain = 1;





//void SX1276FskSetRFFrequency( uint32 freq )
//{
//    //FskSettings.RFFrequency = freq;
//
//    freq = ( uint32 )( ( double )freq / ( double )FREQ_STEP );
//    SX1276->RegFrfMsb = ( uint8 )( ( freq >> 16 ) & 0xFF );
//    SX1276->RegFrfMid = ( uint8 )( ( freq >> 8 ) & 0xFF );
//    SX1276->RegFrfLsb = ( uint8 )( freq & 0xFF );
//    SX1276WriteBuffer( REG_FRFMSB, &SX1276->RegFrfMsb, 3 );
//
//    SX1276Read( REG_PACONFIG, &SX1276->RegPaConfig );
//    
//    if( freq > 860000000 )
//    {
//        SX1276->RegPaConfig = ( SX1276->RegPaConfig & RF_PACONFIG_PASELECT_MASK ) | RF_PACONFIG_PASELECT_RFO;
//        SX1276Write( REG_PACONFIG, SX1276->RegPaConfig ); 
//
//        
//            SX1276FskSetPa20dBm( false );
//       
//    }
//    else
//    {
//        SX1276->RegPaConfig = ( SX1276->RegPaConfig & RF_PACONFIG_PASELECT_MASK ) | RF_PACONFIG_PASELECT_PABOOST;
//        SX1276Write( REG_PACONFIG, SX1276->RegPaConfig );
//    }
//}



void SX1276FskRxCalibrate( void )
{
    // the function RadioRxCalibrate is called just after the reset so all register are at their default values
    uint8 regPaConfigInitVal,TempReg;
    //uint32 initialFreq;

    // save register values;
    SX1276Read( REG_PACONFIG, &regPaConfigInitVal );
   // initialFreq = SX1276FskGetRFFrequency( );

    // Cut the PA just in case
//    SX1276->RegPaConfig = 0x00; // RFO output, power = -1 dBm
    SX1276Write( REG_PACONFIG, 0x00 );

    // Set Frequency in HF band
    SX1276SetRFFrequency( 860000);

    // Rx chain re-calibration workaround
    SX1276Read( REG_IMAGECAL, &TempReg );    
    TempReg = ( TempReg & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START;
    SX1276Write( REG_IMAGECAL, TempReg );

    SX1276Read( REG_IMAGECAL, &TempReg );
    // rx_cal_run goes low when calibration in finished
    while( ( TempReg & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
        SX1276Read( REG_IMAGECAL, &TempReg );
    }

    // reload saved values into the registers
    TempReg = regPaConfigInitVal;
    SX1276Write( REG_PACONFIG, TempReg);

    SX1276SetRFFrequency( FskSettings.RFFrequency );

}

void SX1276FskSetBitrate( uint32 bitrate )
{
	uint8_t RegBitrate[2];
    FskSettings.Bitrate = bitrate;
    
    bitrate = ( uint16 )( ( double )XTAL_FREQ / ( double )bitrate );
    RegBitrate[0] = ( uint8 )( bitrate >> 8 );
    RegBitrate[1] = ( uint8 )( bitrate & 0xFF );
    SX1276WriteBuffer( REG_BITRATEMSB, RegBitrate, 2 );    
}



void SX1276FskSetFdev( uint32 fdev )
{
	uint8_t RegFdev[2];
    FskSettings.Fdev = fdev;

    SX1276Read( REG_FDEVMSB, RegFdev ); 

    fdev = ( uint16 )( ( double )fdev / ( double )FREQ_STEP );
    RegFdev[0] = ( uint8 )( ( RegFdev[0] & RF_FDEVMSB_FDEV_MASK ) | ( ( ( uint8 )( fdev >> 8 ) ) & ~RF_FDEVMSB_FDEV_MASK ) );
    RegFdev[1] = ( uint8 )( fdev & 0xFF );
    SX1276WriteBuffer( REG_FDEVMSB, RegFdev, 2 );    
}



void SX1276FskSetRFPower( int8 power )
{
	uint8_t RegPaDac,RegPaConfig;
    SX1276Read( REG_PACONFIG, &RegPaConfig );
    SX1276Read( REG_PADAC, &RegPaDac );
    
	RegPaConfig = ( RegPaConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;
	RegPaConfig = ( RegPaConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8 )( ( uint16 )( power - 5 ) & 0x0F );
    SX1276Write( REG_PACONFIG, RegPaConfig );
    FskSettings.Power = power;
}





/*!
 * \brief Computes the mantisse and exponent from the bandwitdh value
 *
 * \param [IN] rxBwValue Bandwidth value
 * \param [OUT] mantisse Mantisse of the bandwidth value
 * \param [OUT] exponent Exponent of the bandwidth value
 */
/*static void SX1276FskComputeRxBwMantExp( uint32 rxBwValue, uint8* mantisse, uint8* exponent )
{
    uint8 tmpExp = 0;
    uint8 tmpMant = 0;

    double tmpRxBw = 0;
    double rxBwMin = 10e6;

    for( tmpExp = 0; tmpExp < 8; tmpExp++ )
    {
        for( tmpMant = 16; tmpMant <= 24; tmpMant += 4 )
        {
            if( ( SX1276->RegOpMode & RF_OPMODE_MODULATIONTYPE_FSK ) == RF_OPMODE_MODULATIONTYPE_FSK )
            {
                tmpRxBw = ( double )XTAL_FREQ / ( tmpMant * ( double )pow( 2, tmpExp + 2 ) );
            }
            else
            {
                tmpRxBw = ( double )XTAL_FREQ / ( tmpMant * ( double )pow( 2, tmpExp + 3 ) );
            }
            if( fabs( tmpRxBw - rxBwValue ) < rxBwMin )
            {
                rxBwMin = fabs( tmpRxBw - rxBwValue );
                *mantisse = tmpMant;
                *exponent = tmpExp;
            }
        }
    }  
}*/

void SX1276FskSetDccBw( uint8* reg, uint32 dccValue, uint32 rxBwValue )
{
    uint8 mantisse = 0;
    uint8 exponent = 0;
	uint8 RegRxBw = 0;
	
	SX1276Read(REG_RXBW, &RegRxBw);
    
    if( reg == &RegRxBw )
    {
        *reg = ( uint8 )dccValue & 0x60;
    }
    else
    {
        *reg = 0;
    }

////    SX1276FskComputeRxBwMantExp( rxBwValue, &mantisse, &exponent );

    switch( mantisse )
    {
        case 16:
            *reg |= ( uint8 )( 0x00 | ( exponent & 0x07 ) );
            break;
        case 20:
            *reg |= ( uint8 )( 0x08 | ( exponent & 0x07 ) );
            break;
        case 24:
            *reg |= ( uint8 )( 0x10 | ( exponent & 0x07 ) );
            break;
        default:
            // Something went terribely wrong
            break;
    }

    if( reg == &RegRxBw )
    {
        SX1276Write( REG_RXBW, *reg );
        FskSettings.RxBw = rxBwValue;
    }
    else
    {
        SX1276Write( REG_AFCBW, *reg );
        FskSettings.RxBwAfc = rxBwValue;
    }
}



void SX1276FskSetAfcOn( uint8 enable )
{
	uint8 RegRxConfig;
    SX1276Read( REG_RXCONFIG, &RegRxConfig );
    RegRxConfig = ( RegRxConfig & RF_RXCONFIG_AFCAUTO_MASK ) | ( enable << 4 );
    SX1276Write( REG_RXCONFIG, RegRxConfig );
    
}



void SX1276FskSetPa20dBm( uint8 enale )
{
	uint8 RegPaDac,RegPaConfig;
    SX1276Read( REG_PADAC, &RegPaDac );
    SX1276Read( REG_PACONFIG, &RegPaConfig );

    if( ( RegPaConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {    
        if( enale == true )
        {
            RegPaDac = 0x87;
        }
    }
    else
    {
        RegPaDac = 0x84;
    }
    SX1276Write( REG_PADAC, RegPaDac );
}



void SX1276FskSetPaRamp( uint8 value )
{   uint8 RegPaRamp;
    SX1276Read( REG_PARAMP, &RegPaRamp );
    RegPaRamp = ( uint8 )(( RegPaRamp & RF_PARAMP_MASK ) | ( value & ~RF_PARAMP_MASK ));
    SX1276Write( REG_PARAMP, RegPaRamp );
}


void SX1276FskSetRssiOffset( int8 offset )
{
	uint8 RegRssiConfig;
    SX1276Read( REG_RSSICONFIG, &RegRssiConfig );
    if( offset < 0 )
    {
        offset = ( ~offset & 0x1F );
        offset += 1;
        offset = -offset;
    }
    RegRssiConfig |= ( uint8 )( ( offset & 0x1F ) << 3 );
    SX1276Write( REG_RSSICONFIG, RegRssiConfig );
}

//int8 SX1276FskGetRssiOffset( void )
//{
//    int8 offset;
//    SX1276Read( REG_RSSICONFIG, &SX1276->RegRssiConfig );
//    offset = SX1276->RegRssiConfig >> 3;
//    if( ( offset & 0x10 ) == 0x10 )
//    {
//        offset = ( ~offset & 0x1F );
//        offset += 1;
//        offset = -offset;
//    }
//    return offset;
//}

int8 SX1276FskGetRawTemp( void )
{
    int8 temp = 0;
    uint8 regValue = 0;
    
    SX1276Read( REG_TEMP, &regValue );
    
    temp = regValue & 0x7F;
    
    if( ( regValue & 0x80 ) == 0x80 )
    {
        temp *= -1;
    }
    return temp;
}



void SX1276FskInit( void )
{
	uint8 TempReg;
//  if(halPara[MODULE_CONSUME]==LOW_CONSUME)
//  {
//    RF_PRE_BYTE_LEN = 3600*halPara[WAKE_PERIOD];
//  }
//  else
//  {
//    RF_PRE_BYTE_LEN = 8;
//  }
  
//  SX1276FskSetDefaults( );
  
//  SX1276ReadBuffer( REG_OPMODE, SX1276Regs + 1, 0x70 - 1 );
  
  // Set the device in FSK mode and Sleep Mode
//  SX1276->RegOpMode = RF_OPMODE_MODULATIONTYPE_FSK | RF_OPMODE_SLEEP;
  SX1276Write( REG_OPMODE, RF_OPMODE_MODULATIONTYPE_FSK | RF_OPMODE_SLEEP );
  
//  SX1276->RegOpMode = RF_OPMODE_MODULATIONTYPE_FSK | RF_OPMODE_SLEEP;
//  SX1276Write( REG_OPMODE, RF_OPMODE_MODULATIONTYPE_FSK | RF_OPMODE_SLEEP );
  
//  SX1276->RegPaRamp = RF_PARAMP_MODULATIONSHAPING_01;
  SX1276Write( REG_PARAMP, RF_PARAMP_MODULATIONSHAPING_01 );
  
//  SX1276->RegLna = RF_LNA_GAIN_G1;
  SX1276Write( REG_LNA, RF_LNA_GAIN_G1 );
  
 
  //SX1276->RegRxConfig =  RF_RXCONFIG_RXTRIGER_RSSI ;
//  SX1276->RegRxConfig =  RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT ;
  SX1276Write( REG_RXCONFIG, RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT );
//  SX1276->RegRxTimeout1 = RF_RSSI_TIMEOUT1;
  SX1276Write( REG_RXTIMEOUT1, RF_RSSI_TIMEOUT1 );
//  SX1276->RegRxTimeout2 = RF_PREABLE_TIMEOUT2;
  SX1276Write( REG_RXTIMEOUT2, RF_PREABLE_TIMEOUT2 );
//  SX1276->RegRxTimeout3 = 0 ;
  SX1276Write( REG_RXTIMEOUT3, 0 );
//  SX1276->RegRssiThresh = RF_RSSITHRESH_THRESHOLD;
  SX1276Write( REG_RSSITHRESH, RF_RSSITHRESH_THRESHOLD );
  
//  SX1276->RegPreambleLsb = 8;
  SX1276Write( REG_PREAMBLELSB, 8 );
  
//  SX1276->RegPreambleDetect = RF_PREAMBLEDETECT_DETECTOR_ON | RF_PREAMBLEDETECT_DETECTORSIZE_2 |
//	RF_PREAMBLEDETECT_DETECTORTOL_10;
  SX1276Write( REG_PREAMBLEDETECT, RF_PREAMBLEDETECT_DETECTOR_ON | RF_PREAMBLEDETECT_DETECTORSIZE_2 |
	RF_PREAMBLEDETECT_DETECTORTOL_10 );
  
  
  
//  SX1276->RegSyncConfig = RF_SYNCCONFIG_AUTORESTARTRXMODE_WAITPLL_ON | RF_SYNCCONFIG_PREAMBLEPOLARITY_AA |
//	RF_SYNCCONFIG_SYNC_ON |	  RF_SYNCCONFIG_SYNCSIZE_2;
  SX1276Write( REG_SYNCCONFIG, RF_SYNCCONFIG_AUTORESTARTRXMODE_WAITPLL_ON | RF_SYNCCONFIG_PREAMBLEPOLARITY_AA |
	RF_SYNCCONFIG_SYNC_ON |	  RF_SYNCCONFIG_SYNCSIZE_2 );
  
  // A65A9656
//  SX1276->RegSyncValue1 = 0xD3;
  SX1276Write( REG_SYNCVALUE1, 0xD3);
//  SX1276->RegSyncValue2 = 0x91;
  SX1276Write( REG_SYNCVALUE2, 0x91); 

//  SX1276->RegPacketConfig1 = 0;
  SX1276Write( REG_PACKETCONFIG1, 0);
//  SX1276->RegPacketConfig2 = RF_PACKETCONFIG2_DATAMODE_CONTINUOUS;
  SX1276Write( REG_PACKETCONFIG2, RF_PACKETCONFIG2_DATAMODE_CONTINUOUS);
  //                             RSSI_DETECT              DCLK                  DATA
  
  
//  SX1276->RegDioMapping1 = RF_DIOMAPPING1_DIO0_01 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO2_00 | RF_DIOMAPPING1_DIO3_01;
  SX1276Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 | RF_DIOMAPPING1_DIO1_00 |
			  					RF_DIOMAPPING1_DIO2_00 | RF_DIOMAPPING1_DIO3_01 ); 
  //                           TimeOut,                   Data
//  SX1276->RegDioMapping2 = RF_DIOMAPPING2_DIO4_10 | RF_DIOMAPPING2_DIO5_00;
  SX1276Write( REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_10 | RF_DIOMAPPING2_DIO5_00 ); 
  
 // SX1276->RegPayloadLength = FskSettings.PayloadLength;
//  SX1276->RegOcp = 0X0B;  //Disable overload current protection (OCP) for the PA
  SX1276Write( REG_OCP, 0X0B );
  // we can now update the registers with our configuration
//  SX1276WriteBuffer( REG_OPMODE, SX1276Regs + 1, 0x70 - 1 );
  
  //  SX1276FskGetPacketCrcOn( ); // Update CrcOn on FskSettings
  // then we need to set the RF settings 
 // FskSettings.RFFrequency =470000;
  SX1276SetRFFrequency( FskSettings.RFFrequency );
  SX1276FskSetBitrate( FskSettings.Bitrate );
  SX1276FskSetFdev( FskSettings.Fdev );
  SX1276FskSetPa20dBm( true );
  //  SX1276FskSetRFPower(20 );
  SX1276FskSetRFPower( FskSettings.Power );
  // FskSettings.Power = SX1276FskGetRFPower();
  SX1276Read( REG_RXBW, &TempReg );
  SX1276FskSetDccBw( &TempReg, 0, FskSettings.RxBw );
  SX1276Read( REG_AFCBW, &TempReg );
  SX1276FskSetDccBw( &TempReg, 0, FskSettings.RxBwAfc );
  SX1276FskSetRssiOffset( 0 );
//   SX1276->RegRxConfig =RF_RXCONFIG_RXTRIGER_RSSI;
//     
//    SX1276Write( REG_RXCONFIG,SX1276->RegRxConfig); 
  SX1276FskSetOpMode( RF_OPMODE_STANDBY );
  
  
  // Calibrate the HF
  SX1276FskRxCalibrate( );
  
  
  
  SX1276FskSetOpMode(RF_OPMODE_RECEIVER );
  //while(1);
}

//void SX1276FskSetDefaults( void )
//{
//    // REMARK: See SX1276 datasheet for modified default values.
//
//    SX1276Read( REG_VERSION, &SX1276->RegVersion );
//}

void SX1276FskSetOpMode( uint8 opMode )
{
	//RX_EN_OFF();
	//TX_EN_OFF();
	uint8 RegOpMode;
	SX1276Read( REG_OPMODE, &RegOpMode );
	
	RegOpMode = ( RegOpMode & RF_OPMODE_MASK ) | opMode;
	
	SX1276Write( REG_OPMODE, RegOpMode );   
	SX1276Write( REG_OPMODE, RegOpMode ); 
	//   SX1276Read( REG_OPMODE, &SX1276->RegOpMode );   
////	if(opMode==RF_OPMODE_TRANSMITTER)
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
}

uint8_t SX1276FskGetOpMode( void )
{
	uint8 RegOpMode;
    SX1276Read( REG_OPMODE, &RegOpMode );
    
    return RegOpMode & ~RF_OPMODE_MASK;
}


//int32 SX1276FskReadFei( void )
//{
//    SX1276ReadBuffer( REG_FEIMSB, &SX1276->RegFeiMsb, 2 );                          // Reads the FEI value
//
//    return ( int32 )( ( double )( ( ( uint16 )SX1276->RegFeiMsb << 8 ) | ( uint16 )SX1276->RegFeiLsb ) * ( double )FREQ_STEP );
//}

int32 SX1276FskReadAfc( void )
{
	uint8 RegAfc[2];
    SX1276ReadBuffer( REG_AFCMSB, RegAfc, 2 );                            // Reads the AFC value
    return ( int32 )( ( double )( ( ( uint16 )RegAfc[0] << 8 ) | ( uint16 )RegAfc[1] ) * ( double )FREQ_STEP );
}

uint8 SX1276FskReadRxGain( void )
{
	uint8 RegLna;
    SX1276Read( REG_LNA, &RegLna );
    return( RegLna >> 5 ) & 0x07;
}

double SX1276FskReadRssi( void )
{
	uint8 RegRssiValue;
    SX1276Read( REG_RSSIVALUE, &RegRssiValue );                               // Reads the RSSI value

    return ( RegRssiValue / 2.0 );
}

uint8 SX1276FskGetPacketRxGain( void )
{
    return RxGain;
}

double SX1276FskGetPacketRssi( void )
{
    return RxPacketRssiValue;
}

uint32 SX1276FskGetPacketAfc( void )
{
    return RxPacketAfcValue;
}

void SX1276FskSlp(void)
{ 
  SX1276FskSetOpMode(RF_OPMODE_STANDBY);
  // SX1276FskSetOpMode(RF_OPMODE_SLEEP);
    SX1276FskSetOpMode(RF_OPMODE_SLEEP);
}


void SX1276FskTestTx(void)
{
//  SX1276FskSetOpMode( RF_OPMODE_STANDBY  );
//  SX1276Read(REG_PACKETCONFIG2,&SX1276->RegPacketConfig2);
//  SX1276->RegPacketConfig2 &= ~0x40 ;
//  SX1276Write(REG_PACKETCONFIG2,SX1276->RegPacketConfig2);
//  SX1276Read(REG_PACKETCONFIG2,&SX1276->RegPacketConfig2);
//  // Packet DIO mapping setup
//  //                           TX ready              dclk                          data
//  SX1276->RegDioMapping1 = RF_DIOMAPPING1_DIO0_00 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO2_00 | RF_DIOMAPPING1_DIO3_01;
//  //                           LowBat,                   Data
//  SX1276->RegDioMapping2 = RF_DIOMAPPING2_DIO4_00 | RF_DIOMAPPING2_DIO5_00;
//  SX1276WriteBuffer( REG_DIOMAPPING1, &SX1276->RegDioMapping1, 2 );
//  
//  SX1276Read(REG_OOKPEAK,&SX1276->RegOokPeak);
//  SX1276->RegOokPeak |= BIT(5);
//  SX1276Write(REG_OOKPEAK,SX1276->RegOokPeak);
 // SX1276SetRFFrequency(FskSettings.RFFrequency);
  SX1276FskSetBitrate( 9600 );
  SX1276FskSetFdev( 0 );
  SX1276FskSetOpMode(RF_OPMODE_TRANSMITTER );  
 //SX1276ReadBuffer( REG_OPMODE, SX1276Regs + 1, 0x70 - 1 );
  //GDATA_OUT();
  //GDATA_OUT_LOW();
  //LED_TX_ON();
  //LED_RX_OFF();
  //	SX1276->RegOpMode = 0;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //	SX1276->RegOpMode = 1;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //	SX1276->RegOpMode = 2;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //	SX1276->RegOpMode =5;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //   SX1276->RegOpMode =4;
  
//  while(1)
//  {
//	;
//  }
  
}
void SX1276FskTestRx(void)
{
   //uint8 dio;	
  //uint8 dclk= 0;	//记录最近一个跳变沿之前的电平
  
  //static uint16 bit_count;
  //static uint16 bit_error;  
  //  SX1276FskSetOpMode( RF_OPMODE_STANDBY  );
  //  SX1276Read(REG_PACKETCONFIG2,&SX1276->RegPacketConfig2);
  //  SX1276->RegPacketConfig2 &= ~0x40 ;
  //  SX1276Write(REG_PACKETCONFIG2,SX1276->RegPacketConfig2);
  //  SX1276Read(REG_PACKETCONFIG2,&SX1276->RegPacketConfig2);
  //  // Packet DIO mapping setup
  //  //                           TX ready              dclk                          data
  //  SX1276->RegDioMapping1 = RF_DIOMAPPING1_DIO0_00 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO2_00 | RF_DIOMAPPING1_DIO3_01;
  //  //                           LowBat,                   Data
  //  SX1276->RegDioMapping2 = RF_DIOMAPPING2_DIO4_00 | RF_DIOMAPPING2_DIO5_00;
  //  SX1276WriteBuffer( REG_DIOMAPPING1, &SX1276->RegDioMapping1, 2 );
  //  
  //  SX1276Read(REG_OOKPEAK,&SX1276->RegOokPeak);
  //  SX1276->RegOokPeak |= BIT(5);
  //  SX1276Write(REG_OOKPEAK,SX1276->RegOokPeak);
  //SX1276SetRFFrequency( 470800000 );
 // SX1276FskSetBitrate( 9600 );
 // SX1276FskSetFdev( 25000 );
  SX1276FskSetOpMode(RF_OPMODE_RECEIVER );
  //LED_TX_OFF();
  //LED_RX_ON();
  
  //	SX1276->RegOpMode = 0;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //	SX1276->RegOpMode = 1;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //	SX1276->RegOpMode = 2;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //	SX1276->RegOpMode =5;
  //	SX1276Write( REG_OPMODE, SX1276->RegOpMode );   
  //    SX1276Read( REG_OPMODE, &SX1276->RegOpMode ); 
  //   SX1276->RegOpMode =4;
  while(1)
  {

  }
  
}
void SX1276FskTestSleep(void)
{
  //halPara[MODULE_CONSUME]=HIGH_CONSUME;
  //halSleep();
  //while(1);
}
void test(void)
{
////factoryWaitFlg = 1;
//  //SX1276FskTestSleep();
//// SX1276FskTestTx();
//  //SX1276FskTestTx();
//  if(0)//(TEST_TX_READ()==0)&&TEST_RX_READ())
//  {
//     SX1276FskTestTx();
//  }
//  else if(0)//(TEST_RX_READ()==0)&&TEST_TX_READ())
//  {
//     SX1276FskTestRx();
//  }
//   else if(0)//(TEST_RX_READ()==0)&&TEST_TX_READ()==0)
//  {
//   // factoryWaitFlg = 1; 
//	SX1276FskTestSleep();
//  }
  
}
