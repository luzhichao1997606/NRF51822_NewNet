/******************************************************************************
* File        : app.c
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2017-12-07    Lihao         Original Version
*******************************************************************************/

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
#include "hal.h"
#include "wizchip_conf.h"
//#include "net.h"
#include "dhcp.h"         //包含需要的头文件
#include "mqtt_interface.h"
#include "nrf24l01+.h"
#include "dns.h"
#include "SPI1.h"	
#include "Uart.h"
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

/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
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
//void UART0_IRQHandler(void)
//{
//  if (NRF_UART0->EVENTS_RXDRDY == 1)
//  {
//	  NRF_UART0->EVENTS_RXDRDY = 0;
//
////	  if(stUart.u8Count < UART_BUF_SIZE)
////	  {
////		  stUart.RxOK    = 0;
////		  stUart.u8Timer = 0;
////		  stUart.Buf[stUart.u8Count++] = (uint8_t)NRF_UART0->RXD;
////	  }
////	  else
////	  {
////		  (uint8_t)NRF_UART0->RXD;
////		  stUart.u8Timer = 0x7F;
////		  stUart.RxOK = 1;
////	  }	  
//  }
//}

// IRQ引脚中断
void GPIOTE_IRQHandler(void)
{
	if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && 
		(NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
	{
		NRF_GPIOTE->EVENTS_IN[0] = 0;			
		if(IRQ0_READ() == 0)
			{
				IRQ_Flag0 = 1;
				nRF24L01_IRQ();
			}
		if(IRQ1_READ() == 0)
			{
				IRQ_Flag1 = 1;
				nRF24L01_2_IRQ();
			}
		if(IRQ2_READ()==1)
			{
				IRQ_Flag2 = 1;
			}
//		DemoApp_Pollhandler();
	}
	if (NRF_GPIOTE->EVENTS_PORT==1)
	{
		NRF_GPIOTE->EVENTS_PORT=0; 
	//rfIRQIntHandler();

	} 
	
}


void TIMER0_IRQHandler(void)  //定时器中断模式服务函数 1s
{

    if ((NRF_TIMER0->EVENTS_COMPARE[2] == 1) && (NRF_TIMER0->INTENSET & TIMER_INTENSET_COMPARE2_Msk))
    {
      NRF_TIMER0->EVENTS_COMPARE[2] = 0;
			
//			nrf_gpio_pin_toggle(TX_LED);
			// 用于精确延时函数
			if(delay_us_Ticks > 0)
			{
				delay_us_Ticks--;
			}

		 	NRF_TIMER0->TASKS_CLEAR = 1; //清楚计算
    }
 
}

void TIMER1_IRQHandler(void)  //定时器中断模式服务函数 1s
{

    if ((NRF_TIMER1->EVENTS_COMPARE[2] == 1) && (NRF_TIMER1->INTENSET & TIMER_INTENSET_COMPARE2_Msk))
    {
      NRF_TIMER1->EVENTS_COMPARE[2] = 0;
			
			nrf_gpio_pin_toggle(GPIO_LED1);
			DHCP_time_handler();
////			Phy_Link_Check();	//网线连接状态检测
//			SendTimeOut();	 //发送超时检测
			DNS_time_handler();
		 	NRF_TIMER1->TASKS_CLEAR = 1; //清楚计算
    }
 
}
#define UART_RX_OVER_TIME  30    // 判断串口接收完成的超时时间(ms)
uint16_t closed_time;
void TIMER2_IRQHandler(void)  //定时器中断模式服务函数 1ms
{

	if ((NRF_TIMER2->EVENTS_COMPARE[2] == 1) && (NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE2_Msk))
	{
		NRF_TIMER2->EVENTS_COMPARE[2] = 0;
		// 用于精确延时函数
		if(delayTicks > 0)
		{
			delayTicks--;
		}
		uip_timer++;
		if(uip_timer>= 10)
		{
			uip_timer = 0;
			SystemNowtime++;
		}
		
////		check_time++;
	    closed_time++;
////		Net_Time_Work();
		MilliTimer_Handler();
//		timer2_isr();
		//			Uart_TimeHandler();
		//			timnum++;
		//			if(timnum >= 1000)
		//			{
		//				timnum = 0;
		////      LED1_Toggle();//电平翻转void LED2_Toggle(void)
		////			nrf_gpio_pin_toggle(LED_0);
		////			simple_uart_put(0x35);
		//				
		//			}
		
		// 判断串口是否接收超时
		if(stUart.u8Timer < UART_RX_OVER_TIME)
		{
			stUart.u8Timer++;
		}
		else if(stUart.u8Timer != 0x7F)
		{
			stUart.u8Timer = 0x7F;
			stUart.RxOK    = 1;
		}
		NRF_TIMER2->TASKS_CLEAR = 1; //清楚计算
	}

}

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

