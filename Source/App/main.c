/******************************************************************************
* File        : main.c
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
#include "hal.h"   

//#include "w5500.h"
//#include "W5500_conf.h"
//#include "socket.h"

//#include "mqtt.h"         //包含需要的头文件
//#include "wizchip_conf.h" //包含需要的头文件
#include "socket.h"       	//包含需要的头文件
//#include "dhcp.h"         //包含需要的头文件
//#include "dns.h"          //包含需要的头文件

//#include "net.h"          //包含需要的头文件


#include "nrf24l01+.h"
#include "w5500.h"
#include "wizchip_conf.h"
#include "socket.h"
/*app函数头文件*/
#include "dhcp.h"


#include "string.h"       //包含需要的头文件
#include "SPI1.h"	

#include "nrf_drv_wdt.h"

#include "radio_config.h"

#include "app.h"
#include "NetHandler.h"	        //APP任务函数
#include "AT_Proc.h"
/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/
//#include "cJSON.h"
//unsigned char SendBuffer1[100]= {"{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":\"3132213B2470E251012401020304\"}"};
/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/



/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/
static void SystemInit(void);
bool W5500_Diver_Test(char * W5500_id);
/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
uint8 stbuff[16] = {0xff};
//判断W5500驱动是否正常
uint8 PHY_Status[6] = {0x57,0x35,0x35,0x30,0x30,0x00};
uint8 stbuff11[16] = {0x55,0xaa,0x34,0x48,0x15,0x64,0x67,0x34,0x48,0x15};
uint16_t	m_DEVICE_ID = 0x0315;
uint8_t		Read_ID[16] ; 
char id[6];
bool Result ;
  
 /**
  * @name: Realy_TimeOutTask
  * @test:  
  * @msg: 	 
  * @param  {NRF_NUM}  : 继电器清除
  * @return: Result
  */
uint32_t NewTime = 0;
 void Realy_TimeOutTask(void)
 {
	 NewTime = GetSystemNowtime();
	 if (MQTT_Relay_AlarmCount_flag && ((NewTime - MQTT_Relay_AlarmCount) >= (MQTT_Resv_AlarmTime * 6000)))
	 {
		 MQTT_Resv_Alarm = 0 ;
		 MQTT_Relay_AlarmCount_flag = 0;
		nrf_gpio_pin_clear(Relay_PIN);
		UART_Printf("\r\n Clear RELAY ALARM !!!!!!!!!!!!!!!!!!\r\n"); 
	 }
	 if (MQTT_Resv_Alarm == 0 )
	 {
		 nrf_gpio_pin_clear(Relay_PIN);
		 MQTT_Relay_AlarmCount_flag = 0; 
	 }
	 
 }
 /**
  * @name: NRF_Init_Result_Num
  * @test:  
  * @msg: 	初始化NRF24l01
  * @param  {NRF_NUM}  : 选择具体的初始化的编号
  * @return: Result
  */
 uint8_t NRF_Init_Result_Num(uint8_t NRF_NUM)
 {
	 uint8_t Result = 0;
	 switch (NRF_NUM)
	 {
	 case 1:
			nRF24L01_Init();
			Result = nRF24L01_Check();
		 break;

	 case 2:
			nRF24L01_2_Init();
			Result = nRF24L01_2_Check();
		 break;

	 default:

		 break;
	 } 

	 return Result;
 }

  
  void main(void)
{
        
	memset(stbuff,0xff,16); 
	SystemInit();  
	//spi引脚初始化
	GPIO_Spi_init(); 
	//初始化NRF24L01
	NRF_Init_Result_Num(1);
	NRF_Init_Result_Num(2); 
	radio_configure();  
	create_list_head(&RADIO_DATA_LIST_HEAD);

	volatile  uint8_t *p_maddr = (volatile uint8_t *)0x17000;
	m_DEVICE_ID = *p_maddr + (*(p_maddr+1) <<8);
        
	if(memcmp((uint8_t*)p_maddr,stbuff,8) == 0)
	{
		memcpy(Read_ID,(uint8_t*)"20010333",8);
		UART_Printf(" Writed ID ERROR!!!"  );
	}
	else
	{
		memcpy(Read_ID,(uint8_t*)p_maddr,8);
		UART_Printf(" Writed ID : %s"  , Read_ID );
	}

	UART_Printf(" Writed ID : %s"  , Read_ID  );
	
	UART_Printf(" \r\n NRF24l01 Check : %d", nRF24L01_Check());
	/*初始化应用软件*/
	SoftWareInit();
	reader_Radio_Rx_Rdy(); 
	W5500_Diver_Test(id);
	//初次上电标志位置位
	First_Power_ON_Flag = true;
	while(1)
	{  
		//UART_Printf("MQTT_Resv_Chanle ： %d" , MQTT_Resv_Channel);
		/*事件在环执行*/
		SoftWareEventInLoop();  
		SoftWareTimeingEventInLoop(); 
		Realy_TimeOutTask();		//继电器超时事件 
	}
    
} 

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/
/*******************************************************************************
* Function Name: SystemInit
* Decription   : 系统初始化
* Calls        : 
* Called By    :
* Arguments    : None
* Returns      : None
* Others       : None
****************************** Revision History *******************************
* No.   Version   Date         Modified By   Description               
* 1     V1.0      2017-12-06   Lihao         Original Version
******************************************************************************/
static void SystemInit(void)
{
	halInit(); 
} 

/**
 * @name: W5500_Diver_Test
 * @test: test font
 * @msg:  
 * @param {uint8} id 
 * @return: Result
 */

bool W5500_Diver_Test(char * W5500_id)
{
	bool Result = true;
	ctlwizchip(CW_GET_ID,(void *)W5500_id);
	for (uint8 i = 0; i < 6; i++)
	{
		if (W5500_id[i] != PHY_Status[i])
		{
			Result = false;
			return Result;
		}  
	}
	return Result;
}
/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/
