/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * Copyright (C) SEMTECH S.A.
 */
/*!
 * \file       sx1276-Hal.h
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */

#ifndef __SX1276_HAL_H__
#define __SX1276_HAL_H__

#include "SPI1.h"	
#include "hal.h"

#define  SRWF_2E80A
//#define  VER_A10

#ifdef SRWF_2E80A
#define XTAL_FREQ           32000000
#define FREQ_STEP           61.03515625 

//#define XTAL_FREQ                      30000000//32000000
//#define FREQ_STEP                     	 57.22045898//61.03515625 
#else
#define XTAL_FREQ                         26000000        
#define FREQ_STEP                        49.59106445    
#endif

//DIO2 PB1
#define  DIO2_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_1,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  DIO2_OUT()          {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_1,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  DIO2_OUT_LOW()      {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);}  
#define  DIO2_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);} 
#define  DIO2_READ()         (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1))


//DIO0 PB10
//#define  DIO0_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_10,GPIO_MODE_INPUT,GPIO_PULLUP  );}
//#define  DIO0_OUT()          {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_10,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
//#define  DIO0_OUT_LOW()      {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);}  
//#define  DIO0_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);} 
//#define  DIO0_READ()         (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10))

//DIO1 PB2
//#define  DIO1_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_2,GPIO_MODE_INPUT,GPIO_NOPULL );}
//#define  DIO1_OUT()          {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_2,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
//#define  DIO1_OUT_LOW()      {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);}  
//#define  DIO1_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);} 
//#define  DIO1_READ()         (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))

//DIO3 PB0
//#define  DIO3_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_0,GPIO_MODE_INPUT,GPIO_NOPULL );}
//#define  DIO3_OUT()          {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_0,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
//#define  DIO3_OUT_LOW()      {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);}  
//#define  DIO3_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);} 
//#define  DIO3_READ()         (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0))

//DIO4 PA3
//#define  DIO4_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_3,GPIO_MODE_INPUT,GPIO_NOPULL );}
//#define  DIO4_OUT()          {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_3,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
//#define  DIO4_OUT_LOW()      {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);}  
//#define  DIO4_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);} 
//#define  DIO4_READ()         (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3))

 //RXTX PA1
#define RXTX_IN()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_1,GPIO_MODE_INPUT,GPIO_NOPULL );}
//#define  DIO4_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_3,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  RXTX_OUT()          {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_1,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  RXTX_OUT_LOW()      {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);}  
#define  RXTX_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);} 
#define  RXTX_READ()         (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1))



//SCLK PA5
#define  SPI_SCLK_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_5,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  SPI_SCLK_OUT()          {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_5,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  SPI_SCLK_OUT_LOW()      {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);}  
#define  SPI_SCLK_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);} 
#define  SPI_SCLK_READ()         (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))

//SDI PA7
#define  SPI_SDI_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_7,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  SPI_SDI_OUT()          {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_7,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  SPI_SDI_OUT_LOW()      {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);}  
#define  SPI_SDI_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);} 
#define  SPI_SDI_READ()         (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7))

//SDO PA6
#define  SPI_SDO_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_6,GPIO_MODE_INPUT,GPIO_NOPULL);}
#define  SPI_SDO_OUT()          {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_6,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  SPI_SDO_OUT_LOW()      {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);}  
#define  SPI_SDO_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);} 
#define  SPI_SDO_READ()         (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))

//CS PA4
#define  SPI_SCS_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_4,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  SPI_SCS_OUT()          {HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_4,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  SPI_SCS_OUT_LOW()      {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);}  
#define  SPI_SCS_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);} 
#define  SPI_SCS_READ()         (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))



//RXEN PB12
#define  TXEN_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_12,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  TXEN_OUT()          {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_12,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL );}
#define  TXEN_OUT_LOW()      {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);}  
#define  TXEN_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);} 
#define  TXEN_READ()         (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12))

//TXEN PB13
#define  RXEN_IN_PULLUP()    {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_13,GPIO_MODE_INPUT,GPIO_NOPULL );}
#define  RXEN_OUT()          {HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_13,GPIO_MODE_OUTPUT_PP ,GPIO_NOPULL );}
#define  RXEN_OUT_LOW()      {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);}  
#define  RXEN_OUT_HIGH()     {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);} 
#define  RXEN_READ()         (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13))
   
#define  RF_LNA_ON()              RXEN_OUT_HIGH()
#define  RF_LNA_OFF()             RXEN_OUT_LOW()
#define  RF_PA_ON()               TXEN_OUT_HIGH()
#define  RF_PA_OFF()              TXEN_OUT_LOW()

//DIO1 P24
#define  GCLK_IN_PULLUP()    DIO1_IN_PULLUP()
#define  GCLK_OUT()          DIO1_OUT()
#define  GCLK_OUT_LOW()     DIO1_OUT_LOW()
#define  GCLK_OUT_HIGH()    DIO1_OUT_HIGH()
#define  GCLK_READ()         DIO1_READ()
#define  GP_CLK             GCLK_READ()

#define  GP_DIO1_CLK_ENABLE_EXTI()       {HAL_GPIO_DeInit(GPIOB,GPIO_PIN_2);HAL_GPIO_ConfigPin(GPIOB,GPIO_PIN_2, GPIO_MODE_IT_RISING,GPIO_PULLUP );}
#define  GP_DIO4_TIMEOUT_ENABLE_EXTI()   {HAL_GPIO_DeInit(GPIOA,GPIO_PIN_3);HAL_GPIO_ConfigPin(GPIOA,GPIO_PIN_3, GPIO_MODE_IT_RISING,GPIO_PULLUP );}
#define  GP_DIO0_RSSIDETEC_ENABLE_EXTI() {HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI4_15_IRQn));}

#define  GP_DIO1_CLK_DISABLE_EXTI()       {HAL_GPIO_DeInit(GPIOB,GPIO_PIN_2);DIO1_IN_PULLUP() ;}
#define  GP_DIO4_TIMEOUT_DISABLE_EXTI()   {HAL_GPIO_DeInit(GPIOA,GPIO_PIN_3);DIO4_IN_PULLUP() ;}
#define  GP_DIO0_RSSIDETEC_DISABLE_EXTI() {HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI4_15_IRQn));;}



#define  GDATA_IN_PULLUP()    DIO2_IN_PULLUP()
#define  GDATA_OUT()           DIO2_OUT()
#define  GDATA_OUT_LOW()     DIO2_OUT_LOW()
#define  GDATA_OUT_HIGH()    DIO2_OUT_HIGH()
#define  GDATA_READ()         DIO2_READ()
#define  GDATA               GDATA_READ()

//GPIO2 P20  RX
#define  GRX_DATA_IN_PULLUP()    GDATA_IN_PULLUP()
#define  GRX_DATA_OUT()          GDATA_OUT()
#define  GRX_DATA_OUT_LOW()    GDATA_OUT_LOW()
#define  GRX_DATA_OUT_HIGH()    GDATA_OUT_HIGH()
#define  GRX_DATA_READ()          GDATA_READ()
#define  GP_RX_DAT               GDATA_READ()

//GPIO P17 TX
#define  GTX_DATA_IN_PULLUP()    GDATA_IN_PULLUP()
#define  GTX_DATA_OUT()           GDATA_OUT()
#define  GTX_DATA_OUT_LOW()     GDATA_OUT_LOW()
#define  GTX_DATA_OUT_HIGH()    GDATA_OUT_HIGH()
#define  GTX_DATA_READ()          GDATA_READ()



#define  GP_TX_DAT_1            GTX_DATA_OUT_HIGH()
#define  GP_TX_DAT_0            GTX_DATA_OUT_LOW()



#define  SET_NSS_LOW()  SPI_SCS_OUT_LOW()
#define  SET_NSS_HIGH()  SPI_SCS_OUT_HIGH()


/*!
 * DIO state read functions mapping
 */
#define DIO0                                        SX1276ReadDio0( )
#define DIO1                                        SX1276ReadDio1( )
#define DIO2                                        SX1276ReadDio2( )
#define DIO3                                        SX1276ReadDio3( )
//#define DIO4                                        SX1276ReadDio4( )
//#define DIO5                                        SX1276ReadDio5( )

typedef enum
{
    RADIO_RESET_OFF,
    RADIO_RESET_ON
}tRadioResetState;



/*!
 * \brief Initializes the radio interface I/Os
 */
void SX1276InitIo( void );
void InitSpi( void );
/*!
 * \brief Set the radio reset pin state
 *
 * \param state New reset pin state
 */
void SX1276SetReset( uint8 state );

/*!
 * \brief Writes the radio register at the specified address
 *
 * \param [IN]: addr Register address
 * \param [IN]: u8data New register value
 */
void SX1276Write( uint8 addr, uint8 u8data );

/*!
 * \brief Reads the radio register at the specified address
 *
 * \param [IN]: addr Register address
 * \param [OUT]: u8data Register value
 */
void SX1276Read( uint8 addr, uint8 *u8data );

/*!
 * \brief Writes multiple radio registers starting at address
 *
 * \param [IN] addr   First Radio register address
 * \param [IN] buffer Buffer containing the new register's values
 * \param [IN] u8size   Number of registers to be written
 */
void SX1276WriteBuffer( uint8 addr, uint8 *buffer, uint8 u8size );

/*!
 * \brief Reads multiple radio registers starting at address
 *
 * \param [IN] addr First Radio register address
 * \param [OUT] buffer Buffer where to copy the registers u8data
 * \param [IN] u8size Number of registers to be read
 */
void SX1276ReadBuffer( uint8 addr, uint8 *buffer, uint8 u8size );

/*!
 * \brief Writes the buffer contents to the radio FIFO
 *
 * \param [IN] buffer Buffer containing u8data to be put on the FIFO.
 * \param [IN] u8size Number of bytes to be written to the FIFO
 */
void SX1276WriteFifo( uint8 *buffer, uint8 u8size );

/*!
 * \brief Reads the contents of the radio FIFO
 *
 * \param [OUT] buffer Buffer where to copy the FIFO read u8data.
 * \param [IN] u8size Number of bytes to be read from the FIFO
 */
void SX1276ReadFifo( uint8 *buffer, uint8 u8size );

/*!
 * \brief Gets the SX1276 DIO0 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8 SX1276ReadDio0( void );

/*!
 * \brief Gets the SX1276 DIO1 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8 SX1276ReadDio1( void );

/*!
 * \brief Gets the SX1276 DIO2 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8 SX1276ReadDio2( void );

/*!
 * \brief Gets the SX1276 DIO3 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8 SX1276ReadDio3( void );

/*!
 * \brief Gets the SX1276 DIO4 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8 SX1276ReadDio4( void );

/*!
 * \brief Gets the SX1276 DIO5 hardware pin status
 *
 * \retval status Current hardware pin status [1, 0]
 */
uint8 SX1276ReadDio5( void );

/*!
 * \brief Writes the external RxTx pin value
 *
 * \remark see errata note
 *
 * \param [IN] txEnable [1: Tx, 0: Rx]
 */
void SX1276WriteRxTx( uint8 txEnable );

#endif //__SX1276_HAL_H__

