#ifndef __SPI1_H
#define __SPI1_H
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_spi.h"

#include "nrf_delay.h"

#include "radio.h"
#include "sx1276_hal.h"
#include "sx1276_lora.h"
#include "sx1276_fsk.h"
#include "sx1276.h"	

#include "socket.h"
#include "xqueue.h"
//#include "net.h"


#define  Rf_SPI_MISO_PIN      11
#define  Rf_SPI_MOSI_PIN      12
#define  Rf_SPI_SCK_PIN       13
#define  Rf_SPI_CS0_PIN       14
#define	 RF_SPI_IRQ0_PIN		15

#define  Rf_SPI_CS1_PIN       8
#define	 RF_SPI_IRQ1_PIN		9

#define  Rf_SPI_CS2_PIN       3
#define	 RF_SPI_IRQ2_PIN		4
//#define  Rf_SDN_PIN           24

#define RF_RST0_PIN			16
#define RF_RST0_OUT()			nrf_gpio_cfg_output(RF_RST0_PIN);
#define RF_RST0_LOW()			nrf_gpio_pin_clear(RF_RST0_PIN)
#define RF_RST0_HIGH()		nrf_gpio_pin_set(RF_RST0_PIN)

#define RF_RST1_PIN			10
#define RF_RST1_OUT()			nrf_gpio_cfg_output(RF_RST1_PIN);
#define RF_RST1_LOW()			nrf_gpio_pin_clear(RF_RST1_PIN)
#define RF_RST1_HIGH()		nrf_gpio_pin_set(RF_RST1_PIN)

#define RF_RST2_PIN			5
#define RF_RST2_OUT()			nrf_gpio_cfg_output(RF_RST2_PIN);
#define RF_RST2_LOW()			nrf_gpio_pin_clear(RF_RST2_PIN)
#define RF_RST2_HIGH()		nrf_gpio_pin_set(RF_RST2_PIN)

#define SPI_MOSI_HIGH    nrf_gpio_pin_set(Rf_SPI_MOSI_PIN)
#define SPI_MOSI_LOW     nrf_gpio_pin_clear(Rf_SPI_MOSI_PIN)
 
#define SPI_CLK_HIGH     nrf_gpio_pin_set(Rf_SPI_SCK_PIN)
#define SPI_CLK_LOW      nrf_gpio_pin_clear(Rf_SPI_SCK_PIN)
 
#define RF_SPI_SEL0_LOW()   nrf_gpio_pin_clear(Rf_SPI_CS0_PIN);nrf_gpio_pin_set(Rf_SPI_CS1_PIN);nrf_gpio_pin_set(Rf_SPI_CS2_PIN)
#define RF_SPI_SEL0_HIGH()    nrf_gpio_pin_set(Rf_SPI_CS0_PIN);nrf_gpio_pin_set(Rf_SPI_CS1_PIN);nrf_gpio_pin_set(Rf_SPI_CS2_PIN)

#define RF_SPI_SEL1_LOW()   nrf_gpio_pin_clear(Rf_SPI_CS1_PIN);nrf_gpio_pin_set(Rf_SPI_CS0_PIN);nrf_gpio_pin_set(Rf_SPI_CS2_PIN)
#define RF_SPI_SEL1_HIGH()    nrf_gpio_pin_set(Rf_SPI_CS1_PIN);nrf_gpio_pin_set(Rf_SPI_CS0_PIN);nrf_gpio_pin_set(Rf_SPI_CS2_PIN)

#define RF_SPI_SEL2_LOW()   nrf_gpio_pin_clear(Rf_SPI_CS2_PIN);nrf_gpio_pin_set(Rf_SPI_CS1_PIN);nrf_gpio_pin_set(Rf_SPI_CS0_PIN)
#define RF_SPI_SEL2_HIGH()    nrf_gpio_pin_set(Rf_SPI_CS2_PIN);nrf_gpio_pin_set(Rf_SPI_CS1_PIN);nrf_gpio_pin_set(Rf_SPI_CS0_PIN)

//#define rfPowerOff()	nrf_gpio_pin_set(Rf_SDN_PIN)
//#define rfPowerOn()		nrf_gpio_pin_clear(Rf_SDN_PIN)
 
#define SPI_DATA_GET     nrf_gpio_pin_read(Rf_SPI_MISO_PIN)


#define IRQ0_READ()		nrf_gpio_pin_read(RF_SPI_IRQ0_PIN)
#define IRQ1_READ()		nrf_gpio_pin_read(RF_SPI_IRQ1_PIN)
#define IRQ2_READ()		nrf_gpio_pin_read(RF_SPI_IRQ2_PIN)

#define SEGMENT_VARIABLE(name, vartype, locsegment) locsegment vartype name
#define VARIABLE_SEGMENT_POINTER(name, vartype, targsegment) targsegment vartype * name
#define SEGMENT_VARIABLE_SEGMENT_POINTER(name, vartype, targsegment, locsegment) targsegment vartype * locsegment name
#define LOCATED_VARIABLE(name, vartype, locsegment, addr, init) locsegment vartype name @ addr

# define SEG_GENERIC     //SEG_GENERIC only applies to pointers in Raisonance, not variables.
# define SEG_FAR   
# define SEG_DATA  
# define SEG_NEAR  
# define SEG_IDATA 
# define SEG_XDATA 
# define SEG_PDATA 
# define SEG_CODE  
# define SEG_BDATA 

extern uint8 IRQ_Flag0,IRQ_Flag1,IRQ_Flag2;
extern uint8 RF_RX0,RF_RX1,RF_RX2;
//void SpiSent_1Byte(unsigned char dat);
//unsigned char SpiRead_1Byte(void);
//void SpiSentByte(uint8_t *write,uint8_t len);
//void SpiReadByte(uint8_t *read, uint8_t len);
uint8 halSpiReadByte(uint8 addr);
void halSpiWriteByte(uint8 addr, uint8 data);
uint8_t halSpiReadWriteByte( uint8_t TxByte );
void GPIO_Spi_init(void);

		
#endif
