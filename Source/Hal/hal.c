/******************************************************************************
* File        : hal.c
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2017-12-06    Lihao         Original Version
*******************************************************************************/

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
#include "hal.h"     

/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/
uint32 SystemNowtime;//系统当前时间单位10ms
uint32 uip_timer=0;//uip计时器，每10ms增加1.
/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/
static void halGpioInit(void);
static void halUartInit(void);
static void halSpiInit(void);
static void halTimInit(void);

/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/
void Uart_SendData(const uint8_t *pbuf, uint8_t u8Len);
/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
/*******************************************************************************
 * Function Name: halInit
 * Decription   : 硬件初始化，进入配置模式前的初始化
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void halInit(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;  //清除外部高速时钟准备好标记 
    NRF_CLOCK->TASKS_HFCLKSTART = 1;     //启动外部高速时钟

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)  //外部高速时钟准备好
    {
    }
	
    halGpioInit();
    halSpiInit();
    halUartInit();    
    halTimInit();

}
/*******************************************************************************
 * Function Name: delay_ms
 * Decription	: 毫秒延时函数
 * Calls		: 
 * Called By	: 
 * Arguments	: time_ms:要延时毫秒时间数  
 * Returns		: 
 * Others		: 调用该函数时，全局中断和1ms定时器中断必须使能，
 *                不能在其他中断函数中调用此函数
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/29				
 ******************************************************************************/
volatile uint16 delayTicks;
void delay_ms(uint16 u16Delay)
{
	delayTicks = u16Delay;
	while(delayTicks > 0)
	{
		u16Delay = u16Delay;
		if(delayTicks%100==0)
			nrf_drv_wdt_feed();
//		WATCHDOG_FEED();
	}
}


///************************************************************** 
//* 函数名  : fputc()
//* 描述    : 重定义putc函数，这样可以使用printf函数从串口1打印输出
//* 输入    : None
//* 输出    : None
//* 返回    : None
//* 作者    : 青风
//* 创建日期: 2014.1.1
//* 版本    : V1.00
//*************************************************************/
//int fputc(int ch, FILE *f)
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
///************************************************************** 
//* 函数名  : fputc()
//* 描述    : 重定义putc函数，这样可以使用printf函数从串口1获取数据
//* 输入    : None
//* 输出    : None
//* 返回    : None
//* 作者    : 青风
//* 创建日期: 2014.1.1
//* 版本    : V1.00
//*************************************************************/
////int fgetc(FILE *f)
////{
////		/* 等待串口1输入数据 */
////		/* 返回接收到的数据 */
//////		return (int)simple_uart_get();
////}


/************************************************************** 
* 函数名  : UART_Printf()
* 描述    : 重定义printf函数，这样可以使用UART_Printf函数实现
*			printf从串口1打印输出
* 输入    : None
* 输出    : None
* 返回    : None
* 作者    : 需要调用串口发送函数Uart_SendData()
* 创建日期: 2014.1.1 
* 版本    : V1.00
*************************************************************/
void UART_Printf(const char* fmt, ...) 
{
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
//    Uart_SendData((uint8_t*)buff, strlen(buff));
	IR_SendData((uint8_t*)buff, strlen(buff));
    va_end(args);
}

int putchar(int ch)
{
	NRF_UART0->TXD = (uint8_t)ch;
	
	while (NRF_UART0->EVENTS_TXDRDY!=1)
	{
		////等待发送完
	}
	
	NRF_UART0->EVENTS_TXDRDY=0;
	
	return 0;
}
/*******************************************************************************
 * Function Name: Uart_SendData
 * Decription   :
 * Calls        :
 * Called By    :
 * Arguments    : pbuf  : 指向待发送数据
 *                u8Len : 待发送字节数
 * Returns      : None
 * Others       : None
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Uart_SendData(const uint8_t *pbuf, uint8_t u8Len)
{
    uint8_t u8Loop;
    
    for(u8Loop = 0; u8Loop < u8Len; u8Loop++)
    {
        putchar(pbuf[u8Loop]);
    }
}

/*******************************************************************************
 * Function Name: Uart_SendStr
 * Decription   :
 * Calls        :
 * Called By    :
 * Arguments    : pbuf  : 指向待发送数据
 * Returns      : None
 * Others       : None
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void Uart_SendStr(const uint8_t *pbuf)
{
    if(pbuf == NULL)
    {
        return;
    }
    
    while(*pbuf)
    {
        putchar(*pbuf);
        pbuf++;
    }
}

/*******************************************************************************
 * Function Name: halSpiReadByte
 * Decription   : 通过SPI接口读写指定地址的值
 * Calls        : 
 * Called By    :
 * Arguments    : 
 *                addr : 要读取的地址值 
 * Returns      : 
 *                指定地址的值
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
//uint8_t halSpiReadByte(uint8_t addr)
//{
//    uint8_t       ret;
//    unsigned long temp;
//      
//    temp = __get_interrupt_state();  
//    disableInterrupts();
//	
//	RF_SPI_SEL_LOW();
//    halSpiReadWriteByte(addr);
//    ret = halSpiReadWriteByte(0x00);
//    RF_SPI_SEL_HIGH();
//    
//    __set_interrupt_state(temp);
//    return ret;
//	return 0;
//}

/*******************************************************************************
 * Function Name: halSpiWriteByte
 * Decription   : 通过SPI接口向指定地址的写入一个值
 * Calls        : 
 * Called By    :
 * Arguments    : 
 *                addr : 要写入的地址
 *                data : 要写入的值
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
//void halSpiWriteByte(uint8_t addr, uint8_t data)
//{
//    unsigned long temp;
//      
//    temp = __get_interrupt_state();  
//    disableInterrupts();
//	
//    RF_SPI_SEL_LOW();
//    halSpiReadWriteByte(addr|0x80);
//    halSpiReadWriteByte(data);
//    RF_SPI_SEL_HIGH();
//	
//    __set_interrupt_state(temp);  
//}

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/
/*******************************************************************************
 * Function Name: halGpioInit
 * Decription   : 初始化定时器2
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
static void halGpioInit(void)
{
	nrf_gpio_cfg_output(GPIO_LED1);
	nrf_gpio_cfg_output(TX_LED);
	
	nrf_gpio_cfg_output(DATA_LED);
	nrf_gpio_pin_set(DATA_LED);
	
	nrf_gpio_cfg_output(PWRKEY);
	nrf_gpio_pin_set(PWRKEY);
	
	nrf_gpio_cfg_output(W5500_SPI_CS_PIN);
	nrf_gpio_pin_clear(W5500_SPI_CS_PIN);
	nrf_gpio_cfg_output(W5500_RST_PIN);
	nrf_gpio_pin_set(W5500_RST_PIN);
//	nrf_gpio_cfg_output(RF_SPI_SDN_PIN);
//	nrf_gpio_cfg_output(RF_SPI_SEL_PIN);
//	
//	nrf_gpio_cfg_input(RF_SPI_IRQ_PIN, NRF_GPIO_PIN_PULLUP);
//	//NVIC_EnableIRQ(GPIOTE_IRQn);
//	NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos)
//		                     | (RF_SPI_IRQ_PIN << GPIOTE_CONFIG_PSEL_Pos)  
//			                 | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);
//	NRF_GPIOTE->INTENSET  = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
	
}

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
static void halUartInit(void)
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

/*******************************************************************************
 * Function Name: halSpiInit
 * Decription   : 初始化SPI口
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
static const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);
static void halSpiInit(void)
{
//    spi_master_init(RF_SPI, SPI_MODE0, false);
//	nrf_drv_spi_init(&m_spi_master_0, &config ,NULL);	
	uint32_t err_code = NRF_SUCCESS;

    nrf_drv_spi_config_t config =
    {
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        .irq_priority = 3,
        .orc          = 0xFF,
        .frequency    = NRF_DRV_SPI_FREQ_4M,
        .mode         = NRF_DRV_SPI_MODE_0,
        .bit_order    = (NRF_DRV_SPI_BIT_ORDER_MSB_FIRST),
		.sck_pin  = W5500_SPI_SCK_PIN,
		.mosi_pin = W5500_SPI_MOSI_PIN,
		.miso_pin = W5500_SPI_MISO_PIN,
    };
	
	err_code = nrf_drv_spi_init(&m_spi_master_0, &config ,NULL);
	
	APP_ERROR_CHECK(err_code);
	
	nrf_gpio_pin_dir_set(W5500_SPI_CS_PIN,NRF_GPIO_PIN_DIR_OUTPUT);
	nrf_gpio_pin_set(W5500_SPI_CS_PIN);
	
	nrf_gpio_pin_dir_set(W5500_RST_PIN,NRF_GPIO_PIN_DIR_OUTPUT);
	nrf_gpio_pin_set(W5500_RST_PIN);
}

/*
 * 函数名：GetSystemNowtime
 * 描述  ：获取系统当前时间
 * 输入  ：无
 * 输出  ：系统当前时间
 * 调用  ：在 SysTick 中断函数 SysTick_Handler()调用
 */  
uint32 GetSystemNowtime(void)
{  
    return SystemNowtime;//返回系统当前时间
}
	
/*******************************************************************************
 * Function Name: halTimInit
 * Decription   : 初始化定时器2
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
static void halTimInit(void)
{
    /*
   TIM1_TimeBaseInit(TIM2_PRESCALER_16, TIM1_COUNTERMODE_UP, 999, 0);
   TIM1_OC3Init(TIM1_OCMODE_PWM2, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_ENABLE,
                500, TIM1_OCPOLARITY_LOW, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_SET,
                TIM1_OCNIDLESTATE_RESET);
   TIM1_OC4Init(TIM1_OCMODE_PWM2, TIM1_OUTPUTSTATE_ENABLE, 500, TIM1_OCPOLARITY_LOW,
                TIM1_OCIDLESTATE_SET);
   TIM1_Cmd(ENABLE);
   TIM1_CtrlPWMOutputs(ENABLE);
   */
//   TIM2_TimeBaseInit(TIM2_PRESCALER_16, 999);  // 1ms中断一次
//   TIM2_PrescalerConfig(TIM2_PRESCALER_16, TIM2_PSCRELOADMODE_IMMEDIATE);
//   TIM2_ARRPreloadConfig(ENABLE);
//   TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
//   TIM2_Cmd(ENABLE);
   
   //TIM3_TimeBaseInit(TIM3_PRESCALER_16, 999);  // 1ms中断一次
   //TIM3_PrescalerConfig(TIM3_PRESCALER_16, TIM3_PSCRELOADMODE_IMMEDIATE);
   //TIM3_ARRPreloadConfig(ENABLE);
   //TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
   //TIM3_Cmd(ENABLE);
	// Timer2
	NRF_TIMER2->MODE        = TIMER_MODE_MODE_Timer;
	NRF_TIMER2->PRESCALER   = 7;  //Ftimer  = 125kHz   =8us

	NRF_TIMER2->CC[2]       = (125U);  //=8us*125=1ms
	NRF_TIMER2->INTENSET    = TIMER_INTENSET_COMPARE2_Enabled << TIMER_INTENSET_COMPARE2_Pos;


	NRF_TIMER2->SHORTS      = (TIMER_SHORTS_COMPARE1_CLEAR_Enabled << TIMER_SHORTS_COMPARE1_CLEAR_Pos);

	NVIC_ClearPendingIRQ(TIMER2_IRQn);
	NVIC_SetPriority(TIMER2_IRQn,3);
	NVIC_EnableIRQ(TIMER2_IRQn);  
	NRF_TIMER2->TASKS_START = 1; //开启定时器

	// Timer1
	NRF_TIMER1->MODE        = TIMER_MODE_MODE_Timer;
	NRF_TIMER1->PRESCALER   = 9;  // 16MHz/2^9 = Ftimer  = 31250 Hz   =32us

	NRF_TIMER1->CC[2]       = (31250);  //=32us*31250=1s
	NRF_TIMER1->INTENSET    = TIMER_INTENSET_COMPARE2_Enabled << TIMER_INTENSET_COMPARE2_Pos;


	NRF_TIMER1->SHORTS      = (TIMER_SHORTS_COMPARE1_CLEAR_Enabled << TIMER_SHORTS_COMPARE1_CLEAR_Pos);

	NVIC_ClearPendingIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn,2);
	NVIC_EnableIRQ(TIMER1_IRQn);  
	NRF_TIMER1->TASKS_START = 1; //开启定时器
	
//	// Timer0
//	NRF_TIMER0->MODE        = TIMER_MODE_MODE_Timer;
//	NRF_TIMER0->PRESCALER   = 3;  // 16MHz/2^4 = Ftimer  = 1000 Hz   =1us
//
//	NRF_TIMER0->CC[2]       = (30U);  //=32us*31250=1s
//	NRF_TIMER0->INTENSET    = TIMER_INTENSET_COMPARE2_Enabled << TIMER_INTENSET_COMPARE2_Pos;
//
//
//	NRF_TIMER0->SHORTS      = (TIMER_SHORTS_COMPARE1_CLEAR_Enabled << TIMER_SHORTS_COMPARE1_CLEAR_Pos);
//
//	NVIC_ClearPendingIRQ(TIMER0_IRQn);
//	NVIC_SetPriority(TIMER0_IRQn,2);
//	NVIC_EnableIRQ(TIMER0_IRQn);  
//	NRF_TIMER0->TASKS_START = 1; //开启定时器
}

void nrf_timer_delay_ms(uint_fast16_t volatile number_of_ms)
{

    NRF_TIMER0->MODE           = TIMER_MODE_MODE_Timer;        // 设置为定时器模式
    NRF_TIMER0->PRESCALER      = 3;                            // Prescaler 9 produces 31250 Hz timer frequency => 1 tick = 32 us.
    NRF_TIMER0->BITMODE        = TIMER_BITMODE_BITMODE_16Bit;  // 16 bit 模式.
    NRF_TIMER0->TASKS_CLEAR    = 1;                            // 清定时器.
    
    // With 32 us ticks, we need to multiply by 31.25 to get milliseconds.
    NRF_TIMER0->CC[0]          = number_of_ms * 2;
//    NRF_TIMER0->CC[0]         += number_of_ms / 4; 
    NRF_TIMER0->TASKS_START    = 1;                    // Start timer.

    while (NRF_TIMER0->EVENTS_COMPARE[0] == 0)
    {
        // Do nothing.
    }

    NRF_TIMER0->EVENTS_COMPARE[0]  = 0;
    NRF_TIMER0->TASKS_STOP         = 1;                // Stop timer.
}

/*******************************************************************************
* Function Name: tmrDelay
* Decription   : 利用硬件定时器精确延时u16Delay ms
* Calls        : 
* Called By    :
* Arguments    : u16Delay: 延时时间(ms)
* Returns      : None
* Others       : 注意事项
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2011-05-01    Lihao         Original Version
*******************************************************************************/
volatile uint16 delay_us_Ticks;
void delay_us(uint16 u16Delay)
{
    delay_us_Ticks = u16Delay;
    while(delay_us_Ticks > 0)
    {
        u16Delay = u16Delay;
    }
}

void IR_SendByte(uint8_t Byte)
{
	uint8_t i=8;
	nrf_gpio_pin_clear(TX_LED); //发送起始位
	nrf_timer_delay_ms(98);
	//发送8位数据
	for(i=0;i<8;i++)
	{
		if(Byte & 0x01)
		{
			nrf_gpio_pin_set(TX_LED);//1
		}
		else
		{
			nrf_gpio_pin_clear(TX_LED);//0			
		}	
		nrf_timer_delay_ms(98);	
		Byte = Byte>>1;//低位在前
	}
//	while(i--)
//	{  
//	  MNUSART_TXD = (Byte&0x01);     //先传低位
//	  delay_us(104);	  
//	  Byte = Byte>>1;
//	  //无校验位
//	  MNUSART_TXD=1;//发送结束位
//	  delay_us(104);
	
//	}
	 nrf_gpio_pin_set(TX_LED);
	 nrf_timer_delay_ms(98);

}

void IR_SendData(const uint8_t *pbuf, uint8_t u8Len)	//发送数组最好用这个，也可发送字符串
{
  uint16_t i;
  for(i=0;i<u8Len;i++)
  {
  	 IR_SendByte(pbuf[i]);
  
  }

}
/*******************************************************************************
 * Function Name: halSpiReadWriteByte
 * Decription   : 通过SPI接口读写一个字节
 * Calls        : 
 * Called By    :
 * Arguments    : None                
 * Returns      : None
 * Others       : 注意事项
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
uint8_t SPI_ReadWriteByte(uint8_t byte)
{
//	uint8_t temp;
	uint32_t err_code;
	uint8_t rxbuf[1];
	
	err_code = nrf_drv_spi_transfer(&m_spi_master_0,&byte,1,NULL,0);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_spi_transfer(&m_spi_master_0,NULL,0,rxbuf,1);
	APP_ERROR_CHECK(err_code);
//	spi_master_tx_rx((uint32_t *)RF_SPI_BASE, 1, &byte, &temp);
	return rxbuf[0];
}

/*-------------------------------------------------*/
/*函数名：写1字节数据到SPI总线                     */
/*参  数：TxData:写到总线的数据                    */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_WriteByte(unsigned char TxData)
{				 
	uint32_t err_code;
	
	err_code = nrf_drv_spi_transfer(&m_spi_master_0,&TxData,1,NULL,0);
	APP_ERROR_CHECK(err_code);		
}

/*-------------------------------------------------*/
/*函数名：从SPI总线读取1字节数据                   */
/*参  数：无                                       */
/*返回值：读到的数据                               */
/*-------------------------------------------------*/
unsigned char SPI_ReadByte(void)
{			 
	uint8_t rxbuf[1];
	uint32_t err_code;
	
	err_code = nrf_drv_spi_transfer(&m_spi_master_0,NULL,0,rxbuf,1);
	APP_ERROR_CHECK(err_code);
	
	return rxbuf[0];		    
}

uint8_t comCRC8(uint8_t* pbuf, uint16_t u16Len, uint8_t u8Poly)
{
    unsigned char i;
    unsigned char u8CRC = u8Poly;

    while(u16Len--)
    {
        u8CRC ^= *pbuf++;        
    } 
    return u8CRC;
}

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

void watchdog_init(void)
{
//	bool      erase_bonds;
	nrf_drv_wdt_config_t  wdt_cfg = NRF_DRV_WDT_DEAFULT_CONFIG;
	nrf_drv_wdt_channel_id  WDT_ID;
//	uint8_t reset_cnt = 0;
	
	/*看门狗初始化*/	
	nrf_drv_wdt_init(&wdt_cfg,NULL);
	nrf_drv_wdt_channel_alloc(&WDT_ID);
	nrf_drv_wdt_enable();
	nrf_drv_wdt_feed();
}
