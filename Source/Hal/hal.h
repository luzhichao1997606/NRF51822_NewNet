/******************************************************************************
* File        : hal.h
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date         Modified By   Description               
* 1     V1.0      2011-05-01   Lihao         Original Version
*******************************************************************************/
#ifndef HAL_H
#define HAL_H

#if defined __cplusplus
extern "C" {
#endif

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
#include "nrf51.h"
#include "nrf_gpio.h"
//#include "spi_master.h"
#include "nrf_it.h"
#include <stdio.h>
#include <stdarg.h>

#include "string.h"       //包含需要的头文件
	
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_spi.h"
	
#include "xqueue.h"
	
#include "nrf_drv_wdt.h"

#include "nrf_delay.h"
 
	
/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/
#define WATCHDOG_EN()      //WWDG_Init(0x7F, 0x7F)  // 49.152ms
#define WATCHDOG_FEED()    //WWDG_SetCounter(0x7F)

#define GPIO_LED1          6

#define DATA_LED            19
#define TX_LED              18
	
#define PWRKEY				2

//#define RF_SPI_SDN_PIN	   24
//#define RF_SPI_IRQ_PIN	   25
//#define RF_SPI_SEL_PIN 	   26

#define  W5500_SPI_MISO_PIN      28
#define  W5500_SPI_MOSI_PIN      27
#define  W5500_SPI_SCK_PIN       29
#define  W5500_SPI_CS_PIN        24
#define  W5500_RST_PIN           26
	
//#define rfPowerOn() 			nrf_gpio_pin_clear(RF_SPI_SDN_PIN)	//′ò?aRFD???￡?à-μínSDNòy??
//#define rfPowerOff() 			nrf_gpio_pin_set(RF_SPI_SDN_PIN)	
//
//#define rfGetIRQState()			nrf_gpio_pin_read(RF_SPI_IRQ_PIN)
//
//#define RF_SPI_SEL_LOW()        nrf_gpio_pin_clear(RF_SPI_SEL_PIN)
//#define RF_SPI_SEL_HIGH()       nrf_gpio_pin_set(RF_SPI_SEL_PIN)	
	
//#define RF_SPI             SPI0
//#define RF_SPI_BASE        NRF_SPI0
	

#define ENABLE_GLOBAL_INTERRUPT()      __enable_interrupt()
#define DISABLE_GLOBAL_INTERRUPT()     __disable_interrupt()

#define ENABLE_MAC_INTERRUPTS()        __enable_interrupt() //{ENABLE_MAC_EXT_INTERRUPT();}

#define DISABLE_MAC_INTERRUPTS()       __disable_interrupt() //{DISABLE_MAC_EXT_INTERRUPT();}

#define ENABLE_MAC_TIMER_INTERRUPT()   TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE)
#define DISABLE_MAC_TIMER_INTERRUPT()  TIM2_ITConfig(TIM2_IT_UPDATE, DISABLE)
#define CLEAR_MAC_TIMER_INTERRUPT()    TIM2_ClearITPendingBit(TIM2_IT_UPDATE)
#define START_MAC_TIMER()              TIM2_Cmd(ENABLE)
#define STOP_MAC_TIMER()               TIM2_Cmd(DISABLE)

#define ENABLE_MAC_EXT_INTERRUPT()     //GPIO_Init(RF_IRQ_PORT, (GPIO_Pin_TypeDef)RF_IRQ_PIN, GPIO_MODE_IN_PU_IT)
#define DISABLE_MAC_EXT_INTERRUPT()    //GPIO_Init(RF_IRQ_PORT, (GPIO_Pin_TypeDef)RF_IRQ_PIN, GPIO_MODE_IN_PU_NO_IT)
#define CLEAR_MAC_EXT_INTERRUPT()

#define UART_PIN_RX      0
#define UART_PIN_TX      1
#define UART_PIN_CTS     18
#define UART_PIN_RTS     19
#define UART_BAUDRATE    UART_BAUDRATE_BAUDRATE_Baud115200
#define UART_HWFC        false

	
typedef uint8_t 	uint8;
typedef uint16_t 	uint16;	
typedef uint32_t 	uint32;
//typedef uint8_t 	SOCKET;
typedef uint8_t		u_char;

typedef char 		int8;
typedef int 		int16;
typedef int32_t		int32;
typedef uint32_t	u_long;
typedef uint16		u_short;
typedef uint16		u_int;



extern uint32 SystemNowtime;//系统当前时间单位10ms
extern uint32 uip_timer;//uip计时器，每10ms增加1.
/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/
extern volatile uint16 delayTicks;  // 用于精确延时
extern volatile uint16 delay_us_Ticks;
/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
void halInit(void);
//uint8_t halSpiReadByte(uint8_t addr);
//void halSpiWriteByte(uint8_t addr, uint8_t data);

uint8_t comCRC8(uint8_t* pbuf, uint16_t u16Len, uint8_t u8Poly);

uint8_t SPI_ReadWriteByte(uint8_t byte);

void SPI_WriteByte(unsigned char TxData);
unsigned char SPI_ReadByte(void);


int fputc(int ch, FILE *f);
int putchar(int ch);
void Uart_SendData(const uint8_t *pbuf, uint8_t u8Len);
void Uart_SendStr(const uint8_t *pbuf);

void delay_ms(uint16 u16Delay);
void UART_Printf(const char* fmt, ...);

void IR_SendByte(uint8_t Byte);
void IR_SendData(const uint8_t *pbuf, uint8_t u8Len);

uint32 GetSystemNowtime(void);

void watchdog_init(void);
/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* HAL_H */
/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

