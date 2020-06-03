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
uint8_t rx_buf[RF_LEN_PAYLOAD];  //接收缓冲区
const uint8_t dstAddr[6] = {0x57,0x53,0x4E,0x52,0x52,0x52};
pun_ModuleParam  punModuleParam = (un_ModuleParam *)ADDR_PARAM_START;

st_Rf stRf;
uint8_t ADDR_Save_Data = 1 ;
/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
// RF初始化
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
    nRF24L01_CE_HIGH();	// 使能24L01
    if(FALSE == nRF24L01_Check())
    {
      u8RF_Init_Num++;
      if(u8RF_Init_Num > 10)
        {
          NVIC_SystemReset();
        }
		nrf_delay_ms(50);
		nRF24L01_Init();//24L01校验失败，复位模块
	}  
	
	halSpiWriteByte(SETUP_RETR, 0x10);          // 自动重发延时500us + 86us, 自动重发计数10次
	halSpiWriteByte(SETUP_AW, RF_LEN_ADDR);     // 设置发送/接收地址长度
	halSpiWriteByte(RX_PW_P0, RF_LEN_PAYLOAD);  // 设置通道0数据包长度    
  	halSpiWriteByte(EN_RXADDR, 0x01);           // 接收通道0使能
	halSpiWriteByte(EN_AA, 0x00);   			//关闭接收自动应答            // 使能通道0接收自动应答
	halSpiWriteByte(FEATURE, 0x01);             // 使能W_TX_PAYLOAD_NOACK命令
	halSpiWriteByte(RF_CH, u8channel);         	// 选择射频工作频道0(0-127)  
    halSpiWriteByte(RF_SETUP, 0x27);            // 0db, 250Kbps
//	u8temp = halSpiReadByte(RX_PW_P0);

	UART_Printf("NRF MQTT_Resv_Channel -------------: %d",ADDR_Save_Data);
    nRF24L01_EnterRxMode();	
}

// RF初始化
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
	nRF24L01_CE_HIGH();	// 使能24L01
    if(FALSE == nRF24L01_2_Check())
	{
		u8RF_2_Init_Num++;
		if(u8RF_2_Init_Num > 10)
		{
			NVIC_SystemReset();
		}
		nrf_delay_ms(50);
		nRF24L01_2_Init();//24L01校验失败，复位模块
	}
	 

    halSpiWriteByte_2(SETUP_RETR, 0x10);          // 自动重发延时500us + 86us, 自动重发计数10次
    halSpiWriteByte_2(SETUP_AW, RF_LEN_ADDR);     // 设置发送/接收地址长度
    halSpiWriteByte_2(RX_PW_P0, RF_LEN_PAYLOAD);  // 设置通道0数据包长度    
  	halSpiWriteByte_2(EN_RXADDR, 0x01);           // 接收通道0使能
	halSpiWriteByte_2(EN_AA, 0x00);   	//关闭接收自动应答            // 使能通道0接收自动应答
	halSpiWriteByte_2(FEATURE, 0x01);             // 使能W_TX_PAYLOAD_NOACK命令
	halSpiWriteByte_2(RF_CH, u8channel);           // 选择射频工作频道0(0-127)  
    halSpiWriteByte_2(RF_SETUP, 0x27);            // 0db, 250Kbps
//	u8temp = halSpiReadByte(RX_PW_P0);
    nRF24L01_2_EnterRxMode();	
}

//检测24L01是否存在
//返回值:TRUE，成功; FALSE，失败	
uint8_t nRF24L01_Check(void)
{
	uint8_t i;
	uint8_t buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};	

	halSpiWriteBuf(TX_ADDR,buf, 5); //写入5个字节的地址.	
	buf[2] = 0XAA;
	halSpiReadBuf(TX_ADDR, buf, 5); //读出写入的地址  
	for(i = 0; i < 5; i++)
	{
		if(buf[i] != 0XA5) 
			return FALSE;
	}
	
	return TRUE;  // 检测到24L01
}

//检测24L01是否存在
//返回值:TRUE，成功; FALSE，失败	
uint8_t nRF24L01_2_Check(void)
{
	uint8_t i;
	uint8_t buf[5] = {0XA5,0XA5,0XA5,0XA5,0XA5};	

	halSpiWriteBuf_2(TX_ADDR,buf, 5); //写入5个字节的地址.	
	buf[2] = 0XAA;
	halSpiReadBuf_2(TX_ADDR, buf, 5); //读出写入的地址  
	for(i = 0; i < 5; i++)
	{
		if(buf[i] != 0XA5) 
			return FALSE;
	}
	
	return TRUE;  // 检测到24L01
}

//  进入接收模式
void nRF24L01_EnterRxMode(void)
{
	uint8_t u8channel;

	UART_Printf("\r\nNRF24L01 ONE --- ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);
	
	u8channel =  (((ADDR_Save_Data-1)%2==0)?
				(2*(ADDR_Save_Data-1)):(2*(ADDR_Save_Data-1)-1));
	nRF24L01_CE_LOW();
	halSpiWriteByte(RF_CH, u8channel);         // 选择射频工作频道0(0-127)  
	halSpiWriteBuf(RX_ADDR_P0, dstAddr, RF_LEN_ADDR);  // 设置通道0接收地址
	halSpiWriteByte(FLUSH_TX, 0);   // 清除TX FIFO寄存器
	halSpiWriteByte(FLUSH_RX, 0);   // 清除RX FIFO寄存器	
	halSpiWriteByte(STATUS, 0x70);  // 清中断标志
  	halSpiWriteByte(CONFIG, 0x0F);  // 使能接收模式  	
	nRF24L01_CE_HIGH();   
}

//  进入接收模式
void nRF24L01_2_EnterRxMode(void)
{
	uint8_t u8channel;

	UART_Printf("\r\nNRF24L01 TWO --- ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);

	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)+2):(2*(ADDR_Save_Data-1)+1));
	nRF24L01_CE_LOW();
	halSpiWriteByte_2(RF_CH, u8channel);         // 选择射频工作频道0(0-127)  
	halSpiWriteBuf_2(RX_ADDR_P0, dstAddr, RF_LEN_ADDR);  // 设置通道0接收地址
	halSpiWriteByte_2(FLUSH_TX, 0);   // 清除TX FIFO寄存器
	halSpiWriteByte_2(FLUSH_RX, 0);   // 清除RX FIFO寄存器	
	halSpiWriteByte_2(STATUS, 0x70);  // 清中断标志
  	halSpiWriteByte_2(CONFIG, 0x0F);  // 使能接收模式  	
	nRF24L01_CE_HIGH();   
}

// nRF24L01+发送接口
void nRF24L01_Tx(const uint8_t *dstAddr, const uint8_t *pbuf, uint8_t u8Len)
{  
	uint8_t u8channel;
	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)):(2*(ADDR_Save_Data-1)-1));
	nRF24L01_CE_LOW();
	halSpiWriteByte(RF_CH, u8channel);           // 选择射频工作频道0(0-127)  
	halSpiWriteByte(FLUSH_TX, 0);   // 清除TX FIFO寄存器
	halSpiWriteByte(FLUSH_RX, 0);   // 清除RX FIFO寄存器
	halSpiWriteBuf(TX_ADDR,    dstAddr, RF_LEN_ADDR); // 设置目标地址
	halSpiWriteBuf(RX_ADDR_P0, dstAddr, RF_LEN_ADDR); // 设置接收ACk地址
    
	//halSpiWriteBuf(WR_TX_PLOAD, pbuf, RF_LEN_PAYLOAD); // 需要ACK
    halSpiWriteBuf(WR_NAC_TX_PLOAD, pbuf, RF_LEN_PAYLOAD);  // 不需要ACK
	
	halSpiWriteByte(STATUS, 0x70);  // 清中断标志
	halSpiWriteByte(CONFIG, 0x0E);  // 进入发射模式
    nRF24L01_CE_HIGH();              // 至少保持10us以上，将数据发送出去
}

// nRF24L01+发送接口
void nRF24L01_2_Tx(const uint8_t *dstAddr, const uint8_t *pbuf, uint8_t u8Len)
{  
	uint8_t u8channel;
	u8channel = (((ADDR_Save_Data-1)%2==0)?
				 (2*(ADDR_Save_Data-1)+2):(2*(ADDR_Save_Data-1)+1));
	nRF24L01_CE_LOW();
	halSpiWriteByte_2(RF_CH, u8channel);	// 选择射频工作频道0(0-127)  
	halSpiWriteByte_2(FLUSH_TX, 0);			// 清除TX FIFO寄存器
	halSpiWriteByte_2(FLUSH_RX, 0);			// 清除RX FIFO寄存器
	halSpiWriteBuf_2(TX_ADDR,    dstAddr, RF_LEN_ADDR); // 设置目标地址
	halSpiWriteBuf_2(RX_ADDR_P0, dstAddr, RF_LEN_ADDR); // 设置接收ACk地址
    
	//halSpiWriteBuf(WR_TX_PLOAD, pbuf, RF_LEN_PAYLOAD); 	 // 需要ACK
    halSpiWriteBuf_2(WR_NAC_TX_PLOAD, pbuf, RF_LEN_PAYLOAD);  // 不需要ACK
	
	halSpiWriteByte_2(STATUS, 0x70);  // 清中断标志
	halSpiWriteByte_2(CONFIG, 0x0E);  // 进入发射模式
    nRF24L01_CE_HIGH();              // 至少保持10us以上，将数据发送出去
}
	
void NRF_LowPower_Mode(void)
{
	uint8_t u8temp;     
	u8temp = halSpiReadByte(CONFIG);	
	halSpiWriteByte(CONFIG, u8temp & ( ~( 1<<1 ) ));  //配置工作模式:掉电模式 
	u8temp = halSpiReadByte(CONFIG);
	nRF24L01_CE_LOW();         
}
//发完数据之后，执行NRF_LowPower_Mode，在下次发送之前，重新配置为发送模式
	
uint16 u16Temp_old1,u16Temp_new1,u16Temp_old2,u16Temp_new2;	// 存储位置可用标志
// RF芯片中断事件
void nRF24L01_IRQ(void)
{
	uint8_t u8State;
//	uint8_t TempReg;
	ADDR_Save_Data = MQTT_Resv_Channel ;
	
	u8State = halSpiReadByte(STATUS);	
	UART_Printf("NRF24L01 ONE Resv Running ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);

	if(u8State & 0x40)  				   
	{	// 接收成功
		halSpiReadBuf(RD_RX_PLOAD, stRf.Buf, RF_LEN_PAYLOAD);
//		stRf.u8Len = RF_LEN_PAYLOAD;
		
		if(((stRf.Buf[0]&0x3f) == 1)
		   &&(stRf.Buf[RF_LEN_PAYLOAD-1] == comCalCRC(&stRf.Buf[0],RF_LEN_PAYLOAD-1)))
		{
			/************无线接收数据存储*************/
			if((stRf.Buf[1]>=1) && (stRf.Buf[1]<= TAG_NUM_MAX) /*&& (stRf.Buf[1]%2 == 1)*/)
			{
				stRf.RfState |= 0x01; 
				
			}
			
		}
		/************无线接收数据存储*************/
	}
	else if(u8State & 0x10) 
	{	//发射达到最大复发次数								 			
	}
	else if(u8State & 0x20)
	{	// 发送成功	
	}

	for (int i = 0; i < 8; i++)
	{
		UART_Printf("NRF24L01 ONE ResvData is -- stRf.Buf[%d] : %d \r\n" , i , stRf.Buf[i] ); 
	}  
 
    nRF24L01_EnterRxMode(); 				 // 进入接收模式
}


// RF芯片中断事件
void nRF24L01_2_IRQ(void)
{
	uint8_t u8State;
//	uint8_t TempReg;
	
	ADDR_Save_Data = MQTT_Resv_Channel ;
	
	u8State = halSpiReadByte_2(STATUS);	 
	UART_Printf("NRF24L01 TWO Resv Running ADDR_Save_Data IS : %d\r\n",ADDR_Save_Data);
	if(u8State & 0x40)  				   
	{	// 接收成功
		halSpiReadBuf_2(RD_RX_PLOAD, &stRf.Buf[10], RF_LEN_PAYLOAD);
//		stRf.u8Len = RF_LEN_PAYLOAD;
		
		if(((stRf.Buf[10]&0x3f) == ADDR_Save_Data)
			&&(stRf.Buf[10+RF_LEN_PAYLOAD-1] == comCalCRC(&stRf.Buf[10],RF_LEN_PAYLOAD-1)))
		{
			/************无线接收数据存储*************/
			if((stRf.Buf[11]>=1) && (stRf.Buf[11]<= TAG_NUM_MAX) /*&& (stRf.Buf[11]%2 == 0)*/)
			{
				stRf.RfState |= 0x02;
				//			TempReg = (stRf.Buf[11]-1); 
				
				//stSensor.update_buf[TAG_NUM_MAX+1] = 0;	// 2.4G接收正常计时清零
				
			}
		}

	/************无线接收数据存储*************/
	}
	else if(u8State & 0x10) 
	{	//发射达到最大复发次数								 			
	}
	else if(u8State & 0x20)
	{	// 发送成功	
	}

	for (int i = 10; i < 18; i++)
	{
		UART_Printf("NRF24L01 TWO ResvData is -- stRf.Buf[%d] : %d \r\n" , i , stRf.Buf[i] ); 
	}   

    nRF24L01_2_EnterRxMode(); // 进入接收模式
}


/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

