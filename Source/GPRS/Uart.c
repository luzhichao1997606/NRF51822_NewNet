#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "Uart.h"	
#include "nrf_drv_spi.h"
#include "nrf_delay.h"


st_Uart stUart;    // 串口接收缓冲结构体
/*******************************************************************************
 * Function Name: halUartInit
 * Decription   : 初始化串口
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void halUartInit(void)
{
	nrf_gpio_cfg_output(UART_PIN_TX);
	nrf_gpio_cfg_input(UART_PIN_RX, NRF_GPIO_PIN_NOPULL);  
	
	NRF_UART0->PSELTXD = UART_PIN_TX;
	NRF_UART0->PSELRXD = UART_PIN_RX;
	if(UART_HWFC)
	{
		nrf_gpio_cfg_output(UART_PIN_RTS);
		nrf_gpio_cfg_input(UART_PIN_CTS, NRF_GPIO_PIN_NOPULL);
		NRF_UART0->PSELCTS = UART_PIN_CTS;
		NRF_UART0->PSELRTS = UART_PIN_RTS;
		NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
	}
	
	NRF_UART0->BAUDRATE         = (UART_BAUDRATE << UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX    = 1;
	NRF_UART0->TASKS_STARTRX    = 1;
	NRF_UART0->EVENTS_RXDRDY    = 0;
	
	// 开串口接收中断
    NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;
    NVIC_SetPriority(UART0_IRQn, 1);
    NVIC_EnableIRQ(UART0_IRQn);	
}

///*******************************************************************************
// * Function Name: putchar
// * Decription   :
// * Calls        :
// * Called By    :
// * Arguments    : ch : 指向待发送字节
// * Returns      : None
// * Others       : None
// ****************************** Revision History *******************************
// * No.   Version   Date         Modified By   Description               
// * 1     V1.0      2011-05-01   Lihao         Original Version
// ******************************************************************************/
//int putchar(int ch)
//{
//	NRF_UART0->TXD = (uint8_t)ch;
//	
//	while (NRF_UART0->EVENTS_TXDRDY!=1)
//	{
//		////等待发送完
//	}
//	
//	NRF_UART0->EVENTS_TXDRDY=0;
//	
//	return 0;
//}
//
///*******************************************************************************
// * Function Name: Uart_SendData
// * Decription   :
// * Calls        :
// * Called By    :
// * Arguments    : pbuf  : 指向待发送数据
// *                u8Len : 待发送字节数
// * Returns      : None
// * Others       : None
// ****************************** Revision History *******************************
// * No.   Version   Date         Modified By   Description               
// * 1     V1.0      2011-05-01   Lihao         Original Version
// ******************************************************************************/
//void Uart_SendData(const uint8_t *pbuf, uint8_t u8Len)
//{
//    uint8_t u8Loop;
//    
//    for(u8Loop = 0; u8Loop < u8Len; u8Loop++)
//    {
//        putchar(pbuf[u8Loop]);
//    }
//}
//
///*******************************************************************************
// * Function Name: Uart_SendStr
// * Decription   :
// * Calls        :
// * Called By    :
// * Arguments    : pbuf  : 指向待发送数据
// * Returns      : None
// * Others       : None
// ****************************** Revision History *******************************
// * No.   Version   Date         Modified By   Description               
// * 1     V1.0      2011-05-01   Lihao         Original Version
// ******************************************************************************/
//void Uart_SendStr(const uint8_t *pbuf)
//{
//    if(pbuf == NULL)
//    {
//        return;
//    }
//    
//    while(*pbuf)
//    {
//        putchar(*pbuf);
//        pbuf++;
//    }
//}

// 清除串口信息
void clearUart(void)
{
	stUart.RxOK = 0;
	stUart.u8Count = 0;
	stUart.u8Timer = 0x7F;
	memset(stUart.Buf, 0, UART_BUF_SIZE);
}

/*******************************************************************************
* Function Name: UART0_IRQHandler
* Decription   : 串口中断处理函数
* Calls        : 
* Called By    :
* Arguments    : void
* Returns      : void
* Others       : 注意事项
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2011-05-01    Lihao         Original Version
*******************************************************************************/
void UART0_IRQHandler(void)
{
  if (NRF_UART0->EVENTS_RXDRDY == 1)
  {
	  NRF_UART0->EVENTS_RXDRDY = 0;

	  if(stUart.u8Count < UART_BUF_SIZE)
	  {
		  stUart.RxOK    = 0;
		  stUart.u8Timer = 0;
		  stUart.Buf[stUart.u8Count++] = (uint8_t)NRF_UART0->RXD;
	  }
	  else
	  {
		  stUart.u8Timer = (uint8_t)NRF_UART0->RXD;
		  stUart.u8Timer = 0x7F;
		  stUart.RxOK = 1;
	  }	  
  }
}

//void Reset_W5500(void)
//{
//	nrf_gpio_pin_dir_set(W5500_RST_PIN,NRF_GPIO_PIN_DIR_OUTPUT);
//  nrf_gpio_pin_clear(W5500_RST_PIN);
//  vTaskDelay(1);  
//  nrf_gpio_pin_set(W5500_RST_PIN);
//  vTaskDelay(1);
//}
//// Connected to Data Flash
//void WIZ_CS(uint8_t val)
//{
//	if (val == LOW) {
//   		nrf_gpio_pin_clear(W5500_SPI_CS_PIN); 
//	}else if (val == HIGH){
//   		nrf_gpio_pin_set(W5500_SPI_CS_PIN); 
//	}
//}
//uint8_t SPI2_SendByte(uint8_t byte)
//{
//	return 0;
//}




