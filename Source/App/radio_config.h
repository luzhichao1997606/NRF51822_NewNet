/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef RADIO_CONFIG_H
#define RADIO_CONFIG_H

//#include "app_fifo.h"
//#include "FreeRTOS.h"
//#include "timers.h"

#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_spi.h"

#include "xqueue.h"
#include "socket.h"
#include "hal.h"

#define PACKET_BASE_ADDRESS_LENGTH  (2UL)                   //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH        (8UL)                   //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE      (PACKET_STATIC_LENGTH)  //!< Packet payload maximum size in bytes

void    radio_configure(void);
void    TXMode_init(void);
void	  Start_HFCLK(void);
void    reader_radio_configure_NRF24Lx(void);
void    reader_Radio_Rx_Rdy(void);

void    trigger_Radio_Rx_Rdy(void);
void    trigger_TXMode_txstart(uint8_t *tdata);
void    trigger_radio_configure_NRF24Lx(void);


void reader_radio_configure(void);

typedef enum 
{
	RADIO_STATE_READER,
	RADIO_STATE_TRIGGER,
}Radio_states_t;
typedef struct
{
	uint8_t trigger_id[2];
	uint8_t reader_id[2];
	uint8_t cmd;
	uint8_t power1;
	uint8_t power2;
	uint8_t interval;
}Trigger_ack_msg_t;

extern  uint8_t         packet[18];  ///< ·¢ËÍBUFFER
extern  uint8_t         trigger_packet[8];
extern  Radio_states_t  m_radio_states;


//extern TimerHandle_t         m_heart_timerout_handle;
//extern QueueHandle_t         m_Trigger_msgQueue;
//extern QueueHandle_t         m_trigger_ackmsg;
//extern QueueHandle_t         m_heartpacket_queue;

#endif
