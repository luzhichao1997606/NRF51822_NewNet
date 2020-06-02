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
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    1.0.B2 ( PRELIMINARY )
 * \date       Nov 21 2012
 * \author     Miguel Luis
 */

//#include <stdbool.h>
   //#include "intrinsics.h"
#include "SPI1.h"	

uint8 SpiInOut( uint8 byte );



/*void SX1276SetReset( uint8 state )
{
    ;
}*/

void SX1276Write(uint8 addr, uint8 u8data)
{
    halSpiWriteByte(addr, u8data);
}

void SX1276Read(uint8 addr, uint8 *u8data)
{
    *u8data = halSpiReadByte(addr);
}

        
   

void SX1276WriteBuffer( uint8 addr, uint8 *buffer, uint8 u8size )
{
	uint8_t i;

    if(RF_NUM == 0)
	{
		RF_SPI_SEL0_LOW();
	}
	else if(RF_NUM == 1)
	{
		RF_SPI_SEL1_LOW();
	}
	else if(RF_NUM == 2)
	{
		RF_SPI_SEL2_LOW();
	}

    halSpiReadWriteByte( addr | 0x80 );
    for( i = 0; i < u8size; i++ )
    {
        halSpiReadWriteByte( buffer[i] );
    }

    if(RF_NUM == 0)
   {
		RF_SPI_SEL0_HIGH();
   }
    else if(RF_NUM == 1)
	{
		RF_SPI_SEL1_HIGH();
	}
	else if(RF_NUM == 2)
	{
		RF_SPI_SEL2_HIGH();
	}
}

void SX1276ReadBuffer( uint8 addr, uint8 *buffer, uint8 u8size )
{
    uint8_t i;
	
    if(RF_NUM == 0)
	{
		RF_SPI_SEL0_LOW();
	}
	else if(RF_NUM == 1)
	{
		RF_SPI_SEL1_LOW();
	}
	else if(RF_NUM == 2)
	{
		RF_SPI_SEL2_LOW();
	}

    halSpiReadWriteByte( addr & 0x7F );

    for( i = 0; i < u8size; i++ )
    {
        buffer[i] = halSpiReadWriteByte( 0 );
    }

    if(RF_NUM == 0)
   {
		RF_SPI_SEL0_HIGH();
   }
    else if(RF_NUM == 1)
	{
		RF_SPI_SEL1_HIGH();
	}
	else if(RF_NUM == 2)
	{
		RF_SPI_SEL2_HIGH();
	}
}

void SX1276WriteFifo( uint8 *buffer, uint8 u8size )
{
    SX1276WriteBuffer( 0, buffer, u8size );
}

void SX1276ReadFifo( uint8 *buffer, uint8 u8size )
{
    SX1276ReadBuffer( 0, buffer, u8size );
}

uint8 SX1276ReadDio0( void )
{
    return IRQ0_READ();
}

uint8 SX1276ReadDio1( void )
{
    return 0;//DIO1_READ();
}

uint8 SX1276ReadDio2( void )
{
    return 0;//DIO2_READ();
}

uint8 SX1276ReadDio3( void )
{
    return 0;//DIO3_READ();
}





