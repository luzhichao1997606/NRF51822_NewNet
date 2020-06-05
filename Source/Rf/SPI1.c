#include <stdint.h>

#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "SPI1.h"	
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
//#include "FreeRTOS.h"
//#include "task.h"

uint8_t IRQ_Flag0,IRQ_Flag1,IRQ_Flag2;
uint8_t RF_RX0,RF_RX1,RF_RX2;
uint8_t RF_NUM = 0;

void GPIO_Spi_init(void)
{
//	nrf_gpio_pin_dir_set(Rf_SPI_MISO_PIN,NRF_GPIO_PIN_DIR_INPUT);
    nrf_gpio_cfg_input(Rf_SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(Rf_SPI_SCK_PIN);
	
    nrf_gpio_cfg_output(Rf_SPI_CS0_PIN);
	nrf_gpio_cfg_output(Rf_SPI_CS1_PIN);
	nrf_gpio_cfg_output(Rf_SPI_CS2_PIN);
	
    nrf_gpio_cfg_output(Rf_SPI_MOSI_PIN);  
	nrf_gpio_cfg_output(RF_RST0_PIN);  
	nrf_gpio_cfg_output(RF_RST1_PIN);  
	nrf_gpio_cfg_output(RF_RST2_PIN); 
	
	nrf_gpio_cfg_output(nRF24L01_CE_PIN);

    SPI_CLK_LOW;

    RF_SPI_SEL0_HIGH();
	RF_SPI_SEL1_HIGH();
	RF_SPI_SEL2_HIGH();

    SPI_MOSI_LOW;
	  
	NRF_GPIO->PIN_CNF[RF_SPI_IRQ0_PIN]=(GPIO_PIN_CNF_DIR_Input<<GPIO_PIN_CNF_DIR_Pos) 
										| (GPIO_PIN_CNF_INPUT_Connect<<GPIO_PIN_CNF_INPUT_Pos)
										| (GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos) 
										| (GPIO_PIN_CNF_DRIVE_S0S1<<GPIO_PIN_CNF_DRIVE_Pos) 
										| (GPIO_PIN_CNF_SENSE_High<<GPIO_PIN_CNF_SENSE_Pos);

	NRF_GPIO->PIN_CNF[RF_SPI_IRQ1_PIN]=(GPIO_PIN_CNF_DIR_Input<<GPIO_PIN_CNF_DIR_Pos) 
										| (GPIO_PIN_CNF_INPUT_Connect<<GPIO_PIN_CNF_INPUT_Pos)
										| (GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos) 
										| (GPIO_PIN_CNF_DRIVE_S0S1<<GPIO_PIN_CNF_DRIVE_Pos) 
										| (GPIO_PIN_CNF_SENSE_High<<GPIO_PIN_CNF_SENSE_Pos);

	NRF_GPIO->PIN_CNF[RF_SPI_IRQ2_PIN]=(GPIO_PIN_CNF_DIR_Input<<GPIO_PIN_CNF_DIR_Pos) 
										| (GPIO_PIN_CNF_INPUT_Connect<<GPIO_PIN_CNF_INPUT_Pos)
										| (GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos)  
										| (GPIO_PIN_CNF_DRIVE_S0S1<<GPIO_PIN_CNF_DRIVE_Pos) 
										| (GPIO_PIN_CNF_SENSE_High<<GPIO_PIN_CNF_SENSE_Pos);
 
	NRF_GPIOTE->INTENSET  = GPIOTE_INTENSET_PORT_Set << GPIOTE_INTENSET_PORT_Pos;
	
    //NVIC_SetPriority(GPIOTE_IRQn, 1); 
    

	NRF_GPIOTE->CONFIG[0] =	(GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos)
                           	|	(RF_SPI_IRQ0_PIN << GPIOTE_CONFIG_PSEL_Pos)  
                           	|	(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);//�ж�����

	NRF_GPIOTE->CONFIG[0] =	(GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos)
                           	|	(RF_SPI_IRQ1_PIN << GPIOTE_CONFIG_PSEL_Pos)  
                           	|	(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);//�ж�����

	NRF_GPIOTE->INTENSET  = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;//ʹ���ж�����
	 
    NVIC_EnableIRQ(GPIOTE_IRQn);   

	NVIC_SetPriority(GPIOTE_IRQn, 4);      
 }

uint8_t halSpiReadWriteByte( uint8_t TxByte )
{
	uint8_t i = 0, Data = 0;
	SPI_CLK_LOW;
	for( i = 0; i < 8; i++ )			//һ���ֽ�8byte��Ҫѭ��8��
	{
		/** ���� */
		
		if( 0x80 == ( TxByte & 0x80 ))
		{
			SPI_MOSI_HIGH;		//�������Ҫ���͵�λΪ 1 ���ø�IO����
		}
		else
		{
			SPI_MOSI_LOW;		//�������Ҫ���͵�λΪ 0 ���õ�IO����
		}
		TxByte <<= 1;					//��������һλ���ȷ��͵������λ
		
		SPI_CLK_HIGH;			//ʱ�����ø�
//		__nop( );
//		__nop( );
//		for(j=0;j<5;j++);
		/** ���� */
		Data <<= 1;						//������������һλ���Ƚ��յ��������λ
		if(SPI_DATA_GET != 0) 
		{
			Data |= 0x01;				//�������ʱIO����Ϊ������Ϊ���յ� 1
		}
		
		SPI_CLK_LOW;				//ʱ�����õ�
//		__nop( );
//		__nop( );
//		for(j=0;j<5;j++);
		
	}
	return Data;		//���ؽ��յ����ֽ�
}
/*******************************************************************************
 * Function Name: halSpiReadByte
 * Decription   : ͨ��SPI�ӿڶ�дָ����ַ��ֵ
 * Calls        : 
 * Called By    :
 * Arguments    : 
 *                addr : Ҫ��ȡ�ĵ�ֵַ 
 * Returns      : 
 *                ָ����ַ��ֵ
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
uint8_t halSpiReadByte(uint8_t addr)
{
    uint8_t       ret;  
    DISABLE_GLOBAL_INTERRUPT(); 
    RF_SPI_SEL0_LOW();  
	
    halSpiReadWriteByte(addr);
    ret = halSpiReadWriteByte(0x00);  
	RF_SPI_SEL0_HIGH();  

    ENABLE_GLOBAL_INTERRUPT();
    return ret;
}
/*******************************************************************************
* Function Name: halSpiReadBuf
* Decription   : ͨ��SPI�ӿڶ�д������ַ�Ķ��ֵ
* Calls        : 
* Called By    :
* Arguments    : addr : Ҫ��ȡ����ʼ��ֵַ 
* Returns      : ָ����ַ��ֵ
* Others       : ע������
****************************** Revision History *******************************
* No.   Version   Date         Modified By   Description               
* 1     V1.0      2011-05-01   Lihao         Original Version
******************************************************************************/
void halSpiReadBuf(uint8_t addr, uint8_t *pbuf, uint8_t u8Len)
{
	unsigned long temp;
    
	DISABLE_GLOBAL_INTERRUPT(); 
	
	RF_SPI_SEL0_LOW();
	halSpiReadWriteByte(addr);
	while(u8Len--)
	{
	    *pbuf = halSpiReadWriteByte(0x00);
	    pbuf++;
	}  
	RF_SPI_SEL0_HIGH();

	ENABLE_GLOBAL_INTERRUPT();
}

/*******************************************************************************
 * Function Name: halSpiWriteByte
 * Decription   : ͨ��SPI�ӿ���ָ����ַ��д��һ��ֵ
 * Calls        : 
 * Called By    :
 * Arguments    : 
 *                addr : Ҫд��ĵ�ַ
 *                data : Ҫд���ֵ
 * Returns      : None
 * Others       : ע������
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
void halSpiWriteByte(uint8_t addr, uint8_t data)
{
 
    DISABLE_GLOBAL_INTERRUPT();
		
	RF_SPI_SEL0_LOW(); 

	halSpiReadWriteByte(addr|0x20);	// nRF24L01+ SPIд��״̬λΪ0x20
    halSpiReadWriteByte(data);
	
	RF_SPI_SEL0_HIGH(); 

    ENABLE_GLOBAL_INTERRUPT();  
}
/*******************************************************************************
* Function Name: halSpiWriteBuf
* Decription   : ͨ��SPI�ӿ���ָ����ַ��д��һ��ֵ
* Calls        : 
* Called By    :
* Arguments    : addr : Ҫд��ĵ�ַ
*                data : Ҫд���ֵ
* Returns      : ����nRF24L01+״̬�ֽ�
* Others       : ע������
****************************** Revision History *******************************
* No.   Version   Date         Modified By   Description               
* 1     V1.0      2011-05-01   Lihao         Original Version
******************************************************************************/
void halSpiWriteBuf(uint8_t addr,const uint8_t *pbuf, uint8_t u8Len)
{ 
	DISABLE_GLOBAL_INTERRUPT();
	
	RF_SPI_SEL0_LOW();
	halSpiReadWriteByte(addr|0x20);
	while(u8Len--)
	{	
	    halSpiReadWriteByte(*pbuf);
		pbuf++;
	}
	RF_SPI_SEL0_HIGH();
	ENABLE_GLOBAL_INTERRUPT();  
}
/*************************************************************************************************************************/
/**
 * @name: halSpiWriteByte_2
 * @test: ͨ��SPI2�ӿ���ָ����ַ��д��һ��ֵ
 * @msg: 
 * @param {type} 
 * @return: 
 */
void halSpiWriteByte_2(uint8_t addr ,uint8_t data)
{
	unsigned long temp;
    
	temp = __get_interrupt_state();
	__disable_interrupt();
	
	RF_SPI_SEL1_LOW();

	halSpiReadWriteByte(addr|0x20);	// nRF24L01+ SPIд��״̬λΪ0x20
    halSpiReadWriteByte(data);

	RF_SPI_SEL1_HIGH();

	__set_interrupt_state(temp);
} 
/**
 * @name: halSpiWriteBuf_2
 * @test: ͨ��SPI�ӿڶ�д������ַ�Ķ��ֵ
 * @msg: 
 * @param {type} 
 * @return: 
 */ 
void halSpiWriteBuf_2(uint8_t addr,const uint8_t *pbuf, uint8_t u8Len)
{
	unsigned long temp;
    
	temp = __get_interrupt_state();
	__disable_interrupt();
	
	RF_SPI_SEL1_LOW();
	halSpiReadWriteByte(addr|0x20);
	while(u8Len--)
	{	
	    halSpiReadWriteByte(*pbuf);
		pbuf++;
	}
	RF_SPI_SEL1_HIGH();
	__set_interrupt_state(temp);
}
/**
 * @name: halSpiReadByte_2
 * @test: test font
 * @msg: 
 * @param {type} 
 * @return: 
 */
uint8_t halSpiReadByte_2(uint8_t addr)
{
	uint8_t       ret;
	unsigned long temp;
    
	temp = __get_interrupt_state();
	__disable_interrupt();
	
	RF_SPI_SEL1_LOW();
	
	halSpiReadWriteByte(addr);
	ret = halSpiReadWriteByte(0x00);
	
	RF_SPI_SEL1_HIGH();
	__set_interrupt_state(temp);
	return ret;	 
}
/**
 * @name: halSpiReadBuf_2
 * @test:  ͨ��SPI�ӿڶ�д������ַ�Ķ��ֵ
 * @msg: 
 * @param {type} 
 * @return: 
 */
void halSpiReadBuf_2(uint8_t addr, uint8_t *pbuf, uint8_t u8Len)
{
	unsigned long temp;
    
	temp = __get_interrupt_state();
	__disable_interrupt();
	
	RF_SPI_SEL1_LOW();
	halSpiReadWriteByte(addr);
	while(u8Len--)
	{
	    *pbuf = halSpiReadWriteByte(0x00);
	    pbuf++;
	}  
	RF_SPI_SEL1_HIGH();
	__set_interrupt_state(temp);
}

/*******************************************************************************
 * Function Name: comCalCRC
 * Decription   : �ۻ���У��
 * Calls        : 
 * Called By    : 
 * Input        : 
 * Output       : None
 * Return Value :
 * Others       : None
 *------------------------------ Revision History ------------------------------
 * No. Version Date       Revised by   Item       Description     
 *          
 ******************************************************************************/
uint8_t comCalCRC(uint8_t* pbuf, uint16_t u16Len)
{
    unsigned char u8CRC = 0;

    while(u16Len--)
    {
        u8CRC += *pbuf++;
    } 

    return u8CRC;
}
/***********************************************************************************************************************************/
////// IRQ�����ж�t
////void GPIOTE_IRQHandler(void)
////{
////	if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && 
////		(NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
////	{
////		NRF_GPIOTE->EVENTS_IN[0] = 0;
////		rfIRQIntHandler();
////	}
////}
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




