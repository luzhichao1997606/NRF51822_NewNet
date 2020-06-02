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

//#include "mqtt.h"         //������Ҫ��ͷ�ļ�
//#include "wizchip_conf.h" //������Ҫ��ͷ�ļ�
#include "socket.h"       	//������Ҫ��ͷ�ļ�
//#include "dhcp.h"         //������Ҫ��ͷ�ļ�
//#include "dns.h"          //������Ҫ��ͷ�ļ�

//#include "net.h"          //������Ҫ��ͷ�ļ�


#include "w5500.h"
#include "wizchip_conf.h"
#include "socket.h"
/*app����ͷ�ļ�*/
#include "dhcp.h"


#include "string.h"       //������Ҫ��ͷ�ļ�
//#include "SPI1.h"

#include "nrf_drv_wdt.h"

#include "radio_config.h"

#include "app.h"
#include "NetHandler.h"	        //APP������
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
//�ж�W5500�����Ƿ�����
uint8 PHY_Status[6] = {0x57,0x35,0x35,0x30,0x30,0x00};
uint8 stbuff11[16] = {0x55,0xaa,0x34,0x48,0x15,0x64,0x67,0x34,0x48,0x15};
uint16_t	m_DEVICE_ID = 0x0315;
uint8_t		Read_ID[16] ;

bool Flag = false;
char id[6];
bool Result ;
 void main(void)
{
        
	memset(stbuff,0xff,16);
	SystemInit(); 
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
	
	/*��ʼ��Ӧ�����*/
	SoftWareInit();
	reader_Radio_Rx_Rdy(); 
	W5500_Diver_Test(id);
	while(1)
	{ 
		/*�¼��ڻ�ִ��*/
		SoftWareEventInLoop();  
		SoftWareTimeingEventInLoop(); 
	}
    
} 

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/
/*******************************************************************************
* Function Name: SystemInit
* Decription   : ϵͳ��ʼ��
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
