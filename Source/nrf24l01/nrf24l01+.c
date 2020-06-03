/******************************************************************************
* File        : nrf24l01+.c
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2011-05-01    Lihao         Original Version
*******************************************************************************/

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
#include "common.h"
#include "hal.h"
#include "nrf24l01+.h"
#include "app_api.h"
#include "SPI1.h"

#include "NetHandler.h"
/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/

/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/
uint8_t rx_buf[RF_LEN_PAYLOAD];  //���ջ�����
const uint8_t dstAddr[6] = {0x57,0x53,0x4E,0x52,0x52,0x52};
pun_ModuleParam  punModuleParam = (un_ModuleParam *)ADDR_PARAM_START;

st_Rf stRf;
uint8_t ADDR_Save_Data = 1 ;
/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
// RF��ʼ��
uint8_t u8RF_Init_Num=0;
void nRF24L01_Init(void)
{
//	uint8_t u8temp;
	uint8_t u8channel;
	if (MQTT_Resv_Channel != 0)
	{
		ADDR_Save_Data = MQTT_Resv_Channel ;
	}
	
    u8channel = (((ADDR_Save_Data-1)%2==0)?
				(2*(ADDR_Save_Data-1)):(2*(ADDR_Save_Data-1)-1)); 
    nRF24L01_CE_HIGH();	// ʹ��24L01
    if(FALSE == nRF24L01_Check())
    {
      u8RF_Init_Num++;
      if(u8RF_Init_Num > 10)
        {
          NVIC_SystemReset();
        }
		nrf_delay_ms(50);
		nRF24L01_Init();//24L01У��ʧ�ܣ���λģ��
	}  
	
	halSpiWriteByte(SETUP_RETR, 0x10);          // �Զ��ط���ʱ500us + 86us, �Զ��ط�����10��
	halSpiWriteByte(SETUP_AW, RF_LEN_ADDR);     // ���÷���/���յ�ַ����
	halSpiWriteByte(RX_PW_P0, RF_LEN_PAYLOAD);  // ����ͨ��0���ݰ�����    
  	halSpiWriteByte(EN_RXADDR, 0x01);           // ����ͨ��0ʹ��
	halSpiWriteByte(EN_AA, 0x00);   			//�رս����Զ�Ӧ��            // ʹ��ͨ��0�����Զ�Ӧ��
	halSpiWriteByte(FEATURE, 0x01);             // ʹ��W_TX_PAYLOAD_NOACK����
	halSpiWriteByte(RF_CH, u8channel);         	// ѡ����Ƶ����Ƶ��0(0-127)  
    halSpiWriteByte(RF_SETUP, 0x27);            // 0db, 250Kbps
//	u8temp = halSpiReadByte(RX_PW_P0);

	UART_Printf("NRF MQTT_Resv_Channel -------------: %d",ADDR_Save_Data);
    nRF24L01_EnterRxMode();	
}

// RF��ʼ��
uint8_t u8RF_2_Init_Num=0;
void nRF24L01_2_Init(void)
{
//	uint8_t u8temp;
	uint8_t u8channel;
	if (MQTT_Resv_Channel != 0)
	{
		ADDR_Save_Data = MQTT_Resv_Channel ;
	}
	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)+2):(2*(ADDR_Save_Data-1)+1));
	nRF24L01_CE_HIGH();	// ʹ��24L01
    if(FALSE == nRF24L01_2_Check())
	{
		u8RF_2_Init_Num++;
		if(u8RF_2_Init_Num > 10)
		{
			NVIC_SystemReset();
		}
		nrf_delay_ms(50);
		nRF24L01_2_Init();//24L01У��ʧ�ܣ���λģ��
	}
	 

    halSpiWriteByte_2(SETUP_RETR, 0x10);          // �Զ��ط���ʱ500us + 86us, �Զ��ط�����10��
    halSpiWriteByte_2(SETUP_AW, RF_LEN_ADDR);     // ���÷���/���յ�ַ����
    halSpiWriteByte_2(RX_PW_P0, RF_LEN_PAYLOAD);  // ����ͨ��0���ݰ�����    
  	halSpiWriteByte_2(EN_RXADDR, 0x01);           // ����ͨ��0ʹ��
	halSpiWriteByte_2(EN_AA, 0x00);   	//�رս����Զ�Ӧ��            // ʹ��ͨ��0�����Զ�Ӧ��
	halSpiWriteByte_2(FEATURE, 0x01);             // ʹ��W_TX_PAYLOAD_NOACK����
	halSpiWriteByte_2(RF_CH, u8channel);           // ѡ����Ƶ����Ƶ��0(0-127)  
    halSpiWriteByte_2(RF_SETUP, 0x27);            // 0db, 250Kbps
//	u8temp = halSpiReadByte(RX_PW_P0);
    nRF24L01_2_EnterRxMode();	
}

//���24L01�Ƿ����
//����ֵ:TRUE���ɹ�; FALSE��ʧ��	
uint8_t nRF24L01_Check(void)
{
	uint8_t i;
	uint8_t buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};	

	halSpiWriteBuf(TX_ADDR,buf, 5); //д��5���ֽڵĵ�ַ.	
	buf[2] = 0XAA;
	halSpiReadBuf(TX_ADDR, buf, 5); //����д��ĵ�ַ  
	for(i = 0; i < 5; i++)
	{
		if(buf[i] != 0XA5) 
			return FALSE;
	}
	
	return TRUE;  // ��⵽24L01
}

//���24L01�Ƿ����
//����ֵ:TRUE���ɹ�; FALSE��ʧ��	
uint8_t nRF24L01_2_Check(void)
{
	uint8_t i;
	uint8_t buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};	

	halSpiWriteBuf_2(TX_ADDR,buf, 5); //д��5���ֽڵĵ�ַ.	
	buf[2] = 0XAA;
	halSpiReadBuf_2(TX_ADDR, buf, 5); //����д��ĵ�ַ  
	for(i = 0; i < 5; i++)
	{
		if(buf[i] != 0XA5) 
			return FALSE;
	}
	
	return TRUE;  // ��⵽24L01
}

//  �������ģʽ
void nRF24L01_EnterRxMode(void)
{
	uint8_t u8channel;

	UART_Printf("\r\nNRF24L01 ONE --- ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);
	
	u8channel =  (((ADDR_Save_Data-1)%2==0)?
				(2*(ADDR_Save_Data-1)):(2*(ADDR_Save_Data-1)-1));
	nRF24L01_CE_LOW();
	halSpiWriteByte(RF_CH, u8channel);         // ѡ����Ƶ����Ƶ��0(0-127)  
	halSpiWriteBuf(RX_ADDR_P0, dstAddr, RF_LEN_ADDR);  // ����ͨ��0���յ�ַ
	halSpiWriteByte(FLUSH_TX, 0);   // ���TX FIFO�Ĵ���
	halSpiWriteByte(FLUSH_RX, 0);   // ���RX FIFO�Ĵ���	
	halSpiWriteByte(STATUS, 0x70);  // ���жϱ�־
  	halSpiWriteByte(CONFIG, 0x0F);  // ʹ�ܽ���ģʽ  	
	nRF24L01_CE_HIGH();   
}

//  �������ģʽ
void nRF24L01_2_EnterRxMode(void)
{
	uint8_t u8channel;

	UART_Printf("\r\nNRF24L01 TWO --- ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);

	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)+2):(2*(ADDR_Save_Data-1)+1));
	nRF24L01_CE_LOW();
	halSpiWriteByte_2(RF_CH, u8channel);         // ѡ����Ƶ����Ƶ��0(0-127)  
	halSpiWriteBuf_2(RX_ADDR_P0, dstAddr, RF_LEN_ADDR);  // ����ͨ��0���յ�ַ
	halSpiWriteByte_2(FLUSH_TX, 0);   // ���TX FIFO�Ĵ���
	halSpiWriteByte_2(FLUSH_RX, 0);   // ���RX FIFO�Ĵ���	
	halSpiWriteByte_2(STATUS, 0x70);  // ���жϱ�־
  	halSpiWriteByte_2(CONFIG, 0x0F);  // ʹ�ܽ���ģʽ  	
	nRF24L01_CE_HIGH();   
}

// nRF24L01+���ͽӿ�
void nRF24L01_Tx(const uint8_t *dstAddr, const uint8_t *pbuf, uint8_t u8Len)
{  
	uint8_t u8channel;
	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)):(2*(ADDR_Save_Data-1)-1));
	nRF24L01_CE_LOW();
	halSpiWriteByte(RF_CH, u8channel);           // ѡ����Ƶ����Ƶ��0(0-127)  
	halSpiWriteByte(FLUSH_TX, 0);   // ���TX FIFO�Ĵ���
	halSpiWriteByte(FLUSH_RX, 0);   // ���RX FIFO�Ĵ���
	halSpiWriteBuf(TX_ADDR,    dstAddr, RF_LEN_ADDR); // ����Ŀ���ַ
	halSpiWriteBuf(RX_ADDR_P0, dstAddr, RF_LEN_ADDR); // ���ý���ACk��ַ
    
	//halSpiWriteBuf(WR_TX_PLOAD, pbuf, RF_LEN_PAYLOAD); // ��ҪACK
    halSpiWriteBuf(WR_NAC_TX_PLOAD, pbuf, RF_LEN_PAYLOAD);  // ����ҪACK
	
	halSpiWriteByte(STATUS, 0x70);  // ���жϱ�־
	halSpiWriteByte(CONFIG, 0x0E);  // ���뷢��ģʽ
    nRF24L01_CE_HIGH();              // ���ٱ���10us���ϣ������ݷ��ͳ�ȥ
}

// nRF24L01+���ͽӿ�
void nRF24L01_2_Tx(const uint8_t *dstAddr, const uint8_t *pbuf, uint8_t u8Len)
{  
	uint8_t u8channel;
	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)+2):(2*(ADDR_Save_Data-1)+1));
	nRF24L01_CE_LOW();
	halSpiWriteByte_2(RF_CH, u8channel);	// ѡ����Ƶ����Ƶ��0(0-127)  
	halSpiWriteByte_2(FLUSH_TX, 0);			// ���TX FIFO�Ĵ���
	halSpiWriteByte_2(FLUSH_RX, 0);			// ���RX FIFO�Ĵ���
	halSpiWriteBuf_2(TX_ADDR,    dstAddr, RF_LEN_ADDR); // ����Ŀ���ַ
	halSpiWriteBuf_2(RX_ADDR_P0, dstAddr, RF_LEN_ADDR); // ���ý���ACk��ַ
    
	//halSpiWriteBuf(WR_TX_PLOAD, pbuf, RF_LEN_PAYLOAD); 	 // ��ҪACK
    halSpiWriteBuf_2(WR_NAC_TX_PLOAD, pbuf, RF_LEN_PAYLOAD);  // ����ҪACK
	
	halSpiWriteByte_2(STATUS, 0x70);  // ���жϱ�־
	halSpiWriteByte_2(CONFIG, 0x0E);  // ���뷢��ģʽ
    nRF24L01_CE_HIGH();              // ���ٱ���10us���ϣ������ݷ��ͳ�ȥ
}
	
void NRF_LowPower_Mode(void)
{
	uint8_t u8temp;     
	u8temp = halSpiReadByte(CONFIG);	
	halSpiWriteByte(CONFIG, u8temp & ( ~( 1<<1 ) ));  //���ù���ģʽ:����ģʽ 
	u8temp = halSpiReadByte(CONFIG);
	nRF24L01_CE_LOW();         
}
//��������֮��ִ��NRF_LowPower_Mode�����´η���֮ǰ����������Ϊ����ģʽ
	
uint16 u16Temp_old1,u16Temp_new1,u16Temp_old2,u16Temp_new2;	// �洢λ�ÿ��ñ�־
// RFоƬ�ж��¼�
void nRF24L01_IRQ(void)
{
	uint8_t u8State;
//	uint8_t TempReg;
	ADDR_Save_Data = MQTT_Resv_Channel ;
	
	u8State = halSpiReadByte(STATUS);	
	UART_Printf("NRF24L01 ONE Resv Running ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);

	if(u8State & 0x40)  				   
	{	// ���ճɹ�
		halSpiReadBuf(RD_RX_PLOAD, stRf.Buf, RF_LEN_PAYLOAD);
//		stRf.u8Len = RF_LEN_PAYLOAD;
		
		if(((stRf.Buf[0]&0x3f) == 1)
		   &&(stRf.Buf[RF_LEN_PAYLOAD-1] == comCalCRC(&stRf.Buf[0],RF_LEN_PAYLOAD-1)))
		{
			/************���߽������ݴ洢*************/
			if((stRf.Buf[1]>=1) && (stRf.Buf[1]<= TAG_NUM_MAX) /*&& (stRf.Buf[1]%2 == 1)*/)
			{
				stRf.RfState |= 0x01; 
				
			}
			
		}
		/************���߽������ݴ洢*************/
	}
	else if(u8State & 0x10) 
	{	//����ﵽ��󸴷�����								 			
	}
	else if(u8State & 0x20)
	{	// ���ͳɹ�	
	}

	for (int i = 0; i < 8; i++)
	{
		UART_Printf("NRF24L01 ONE ResvData is -- stRf.Buf[%d] : %d \r\n" , i , stRf.Buf[i] ); 
	}  
 
    nRF24L01_EnterRxMode(); 				 // �������ģʽ
}


// RFоƬ�ж��¼�
void nRF24L01_2_IRQ(void)
{
	uint8_t u8State;
//	uint8_t TempReg;
	
	ADDR_Save_Data = MQTT_Resv_Channel ;
	
	u8State = halSpiReadByte_2(STATUS);	 
	UART_Printf("NRF24L01 TWO Resv Running ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);
	if(u8State & 0x40)  				   
	{	// ���ճɹ�
		halSpiReadBuf_2(RD_RX_PLOAD, &stRf.Buf[10], RF_LEN_PAYLOAD);
//		stRf.u8Len = RF_LEN_PAYLOAD;
		
		if(((stRf.Buf[10]&0x3f) == ADDR_Save_Data)
			&&(stRf.Buf[10+RF_LEN_PAYLOAD-1] == comCalCRC(&stRf.Buf[10],RF_LEN_PAYLOAD-1)))
		{
			/************���߽������ݴ洢*************/
			if((stRf.Buf[11]>=1) && (stRf.Buf[11]<= TAG_NUM_MAX) /*&& (stRf.Buf[11]%2 == 0)*/)
			{
				stRf.RfState |= 0x02;
				//			TempReg = (stRf.Buf[11]-1); 
				
				//stSensor.update_buf[TAG_NUM_MAX+1] = 0;	// 2.4G����������ʱ����
				
			}
		}

	/************���߽������ݴ洢*************/
	}
	else if(u8State & 0x10) 
	{	//����ﵽ��󸴷�����								 			
	}
	else if(u8State & 0x20)
	{	// ���ͳɹ�	
	}

	for (int i = 10; i < 18; i++)
	{
		UART_Printf("NRF24L01 TWO ResvData is -- stRf.Buf[%d] : %d \r\n" , i , stRf.Buf[i] ); 
	}   

    nRF24L01_2_EnterRxMode(); // �������ģʽ
}


/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

