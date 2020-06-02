#ifndef __UART_H
#define __UART_H
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_spi.h"

#include "socket.h"
#include "xqueue.h"
//#include "net.h"

#include "m2m.h"
#include "hal.h"

//#define UART_PIN_RX      1
//#define UART_PIN_TX      0
//#define UART_PIN_CTS     13
//#define UART_PIN_RTS     14
//#define UART_BAUDRATE    UART_BAUDRATE_BAUDRATE_Baud115200
//#define UART_HWFC        false
//	



#define UART_BUF_SIZE      180

typedef struct
{
    uint8_t  RxOK:1;       // 接收完成标志位   0:正在接收    1:接收完毕
    uint8_t  u8Timer:7;    // 接收时间间隔计数 0x7F:接收超时 其他:正在接收
    uint8_t  u8Count;      // 串口接收buf有效字节数
    uint8_t  Buf[UART_BUF_SIZE];  // 串口接收buf
} st_Uart;

extern st_Uart stUart;    // 串口接收缓冲结构体

void halUartInit(void);
//int putchar(int ch);
void Uart_SendData(const uint8_t *pbuf, uint8_t u8Len);
void Uart_SendStr(const uint8_t *pbuf);
void clearUart(void);
		
#endif

