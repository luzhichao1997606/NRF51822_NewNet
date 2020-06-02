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
#include "radio_config.h"
#include "nrf_delay.h"
#include "string.h"
#include "xqueue.h"
//#include "net.h"
#include "NetHandler.h"

#include "hal.h"
	 
/* These are set to zero as Shockburst packets don't have corresponding fields. */
#define PACKET0_S1_SIZE                  (0UL)  //!< ���������ǲ����
#define PACKET0_S0_SIZE                  (0UL)  //!< ���������ǲ����
#define PACKET0_PAYLOAD_SIZE             (0UL)  //!< ���������ǲ����
#define PACKET1_BASE_ADDRESS_LENGTH      (2UL)  //!< base address�����趨
#define PACKET1_STATIC_LENGTH            (18UL)  //!< ÿ�η��͹̶�����Ϊ4���ֽڵ����ݰ�
#define PACKET1_PAYLOAD_SIZE             (18UL)  //!< ����������ݳ��ȸ�STALENһ��
/**
 * @brief Function for swapping/mirroring bits in a byte.
 * 
 *@verbatim
 * output_bit_7 = input_bit_0
 * output_bit_6 = input_bit_1
 *           :
 * output_bit_0 = input_bit_7
 *@endverbatim
 *
 * @param[in] inp is the input byte to be swapped.
 *
 * @return
 * Returns the swapped/mirrored input byte.
 */
static uint32_t swap_bits(uint32_t inp);

/**
 * @brief Function for swapping bits in a 32 bit word for each byte individually.
 * 
 * The bits are swapped as follows:
 * @verbatim
 * output[31:24] = input[24:31] 
 * output[23:16] = input[16:23]
 * output[15:8]  = input[8:15]
 * output[7:0]   = input[0:7]
 * @endverbatim
 * @param[in] input is the input word to be swapped.
 *
 * @return
 * Returns the swapped input byte.
 */
static uint32_t bytewise_bitswap(uint32_t inp);

static uint32_t swap_bits(uint32_t inp)
{
    uint32_t i;
    uint32_t retval = 0;
    
    inp = (inp & 0x000000FFUL);
    
    for(i = 0; i < 8; i++)
    {
        retval |= ((inp >> i) & 0x01) << (7 - i);     
    }
    
    return retval;    
}


static uint32_t bytewise_bitswap(uint32_t inp)
{
      return (swap_bits(inp >> 24) << 24)
           | (swap_bits(inp >> 16) << 16)
           | (swap_bits(inp >> 8) << 8)
           | (swap_bits(inp));
}

uint8_t packet[18];  ///< ����BUFFER
uint8_t trigger_packet[8];

uint8_t m_rssi_threshold;

Radio_states_t m_radio_states = RADIO_STATE_READER;

/** 
 * @brief Function for configuring the radio to operate in Shockburst compatible mode.
 * 
 * To configure the application running on nRF24L series devices:
 *
 * @verbatim
 * uint8_t tx_address[5] = { 0xC0, 0x01, 0x23, 0x45, 0x67 };
 * hal_nrf_set_rf_channel(7);
 * hal_nrf_set_address_width(HAL_NRF_AW_5BYTES); 
 * hal_nrf_set_address(HAL_NRF_TX, tx_address);
 * hal_nrf_set_address(HAL_NRF_PIPE0, tx_address); 
 * hal_nrf_open_pipe(0, false);
 * hal_nrf_set_datarate(HAL_NRF_1MBPS);
 * hal_nrf_set_crc_mode(HAL_NRF_CRC_16BIT);
 * hal_nrf_setup_dynamic_payload(0xFF);
 * hal_nrf_enable_dynamic_payload(false);
 * @endverbatim
 *
 * When transmitting packets with hal_nrf_write_tx_payload(const uint8_t *tx_pload, uint8_t length),
 * match the length with PACKET_STATIC_LENGTH.
 * hal_nrf_write_tx_payload(payload, PACKET_STATIC_LENGTH);
 * 
*/

//���߹���4�ֱ���2440MHz��ͨ��0��ַ0x99999999C0��ͨ��1~7��ַ0x66666666XX
//����2MHz�������ֽ�3�ֽ�QQXXCC(QQ��ʼ��CC������xx������char)
void radio_configure()//�������ã�׼����nrf24L01ͨѶ
{
  //���߹���04��+4�ֱ���0��0�ֱ���FC��-4�ֱ���F8��-8�ֱ�
	//    F4��-12�ֱ���F0��-16�ֱ���EC��-20�ֱ���D8��-30�ֱ�
	NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);  //�趨���书�ʣ�������4DBm
  
	NRF_RADIO->FREQUENCY = 13UL;//����Ƶ��12MHz+2400MHz=2412MHz

	//�������ʣ�00��1Mbit��01��2Mbit��02��250Kbit��03��1Mbit��������
	NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);//����250Kbit

	// ���ߵ�ַ����
//  NRF_RADIO->PREFIX0 = 0xC3C2C18AUL;  // ͨ��3 �� 0 �ĵ�1�ֽ�
//  NRF_RADIO->PREFIX1 = 0xC7C6C5C4UL;  // ͨ��7 �� 4 �ĵ�1�ֽ�
  // Radio address config
    NRF_RADIO->PREFIX0 = 
        ((uint32_t)swap_bits(0xC3) << 24) // Prefix byte of address 3 converted to nRF24L series format
      | ((uint32_t)swap_bits(0xC2) << 16) // Prefix byte of address 2 converted to nRF24L series format
      | ((uint32_t)swap_bits(0xC1) << 8)  // Prefix byte of address 1 converted to nRF24L series format
      | ((uint32_t)swap_bits(0x8A) << 0); // Prefix byte of address 0 converted to nRF24L series format
    // | ((uint32_t)swap_bits(0xe7) << 0); // Prefix byte of address 0 converted to nRF24L series format

    NRF_RADIO->PREFIX1 = 
        ((uint32_t)swap_bits(0xC7) << 24) // Prefix byte of address 7 converted to nRF24L series format
      | ((uint32_t)swap_bits(0xC6) << 16) // Prefix byte of address 6 converted to nRF24L series format
      | ((uint32_t)swap_bits(0xC4) << 0); // Prefix byte of address 4 converted to nRF24L series format

  NRF_RADIO->BASE0   = bytewise_bitswap(0x63994567UL);  // ͨ��0�ĸ��ֽ�
  NRF_RADIO->BASE1   = bytewise_bitswap(0x89ABCDEFUL);  // ͨ��1-7�ĸ��ֽ�
  NRF_RADIO->TXADDRESS = 0x00UL;      // ����ʹ�õ�ͨ���ţ�0ͨ��
  NRF_RADIO->RXADDRESSES = 0x01UL;    // ���յ�ͨ���ţ�1ͨ��

  // ���ð�0������
  NRF_RADIO->PCNF0 = (PACKET0_S1_SIZE<<RADIO_PCNF0_S1LEN_Pos)| //S1����ĳ���
                     (PACKET0_S0_SIZE << RADIO_PCNF0_S0LEN_Pos) | //S0���ĳ���
                     (PACKET0_PAYLOAD_SIZE << RADIO_PCNF0_LFLEN_Pos);  //�����ֶ��еı�����

  // ���ð�1������
   NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos)    | //Ч��λ��0�أ�1����
                      (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)           | //���ݴ�С�ˣ��ߵ��ֽ��ĸ��ȷ� 0���ֽڣ�1���ֽڣ�
                      (PACKET1_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos)       | //ͨ��1~7���ֽڳ��ȣ� nrf24�ߵ��ֽ�5�ֽڣ�4����+1���ͣ�
                      (PACKET1_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos)           | //�����ֽڳ��ȣ�255~1��3�ֽ�QxC
                      (PACKET1_PAYLOAD_SIZE << RADIO_PCNF1_MAXLEN_Pos);   //Ӳ�������ֽڳ��ȣ�255~1��3�ֽ�QxC

  // CRC У�鳤������
  NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // У�鳤�� 2��char
  if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk)== (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos) )
  {
    NRF_RADIO->CRCINIT = 0xFFFFUL;      // У���ʼֵ
    NRF_RADIO->CRCPOLY = 0x11021UL;     // CRC poly: x^16+x^12^x^5+1
  }
  else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
  {
    NRF_RADIO->CRCINIT = 0xFFUL;        // У���ʼֵ
    NRF_RADIO->CRCPOLY = 0x107UL;       // CRC poly: x^8+x^2^x^1+1
  }
	
  m_radio_states = RADIO_STATE_READER;
//���ռĴ�����  NRF_RADIO->PACKETPTR	
}

/*����׼��*/
void reader_Radio_Rx_Rdy(void)
{  
	NRF_RADIO->PACKETPTR = (uint32_t)packet;			// DMAָ��ָ�����BUFFER

	NRF_RADIO->EVENTS_DISABLED = 0U;         // ���߹رգ���־λ
	NRF_RADIO->TASKS_DISABLE   = 1U;         // �ر������豸
	while (NRF_RADIO->EVENTS_DISABLED == 0U) // �ȴ��豸�ر�
	{
	}		
	NRF_RADIO->EVENTS_READY = 0U; 				 // �¼�׼�� �շ�ģʽת�����  ��־λ    
	NRF_RADIO->TASKS_RXEN   = 1U;          // ʹ�ܽ���
	NRF_RADIO->TASKS_TXEN   = 0;
	while(NRF_RADIO->EVENTS_READY == 0U)   // �ȴ�����׼����
	{
	}
	NRF_RADIO->EVENTS_END  = 0U;  					// �����¼�			
	NRF_RADIO->TASKS_START = 1U;           // ��ʼ
	/*�ж�ʹ��*/
	NRF_RADIO->INTENSET  = 0x8A;
	NVIC_EnableIRQ(RADIO_IRQn);
}

/*���˱�ǩ���ݣ�ͨ������true����ͨ������false*/
bool tag_filter(uint8_t rssi)
{
	switch(m_rssi_threshold)
	{
		case 0:
		return true;
		case 1:
		if(rssi<95)
			return true;
		break;
		case 2:
		if(rssi<90)
			return true;
		break;
		case 3:
		if(rssi<85)
			return true;
		break;
		case 4:
		if(rssi<80)
			return true;
		break;
		case 5:
		if(rssi<75)
			return true;
		break;
		case 6:
		if(rssi<70)
			return true;
		break;
		case 7:
		if(rssi<65)
			return true;
		break;
		case 8:
		if(rssi<60)
			return true;
		break;
		default:
		break;
	}
	return false;
}

/*radio�����Д�*/
//QUEUE_DECLEAR(Q_TAG);
//#include "queue.h"
uint8_t num= 0;
void RADIO_IRQHandler(void)
{  
	uint8_t TempReg, u8Rssi, u8dat;
	uint8_t i,j=0xf0;
	uint8_t packet_data[9];
	uint8_t crc8_val;
//	uint16_t dest_devid;
//	static uint8_t rssi;
//	uint8_t con1,con2;
//    st_Tag stTag;
	
	if(NRF_RADIO->EVENTS_ADDRESS)
	{
		NRF_RADIO->TASKS_RSSISTART = 0x01;
		NRF_RADIO->EVENTS_ADDRESS  = 0x00;
	}
	if(NRF_RADIO->EVENTS_RSSIEND)
	{
		NRF_RADIO->TASKS_RSSISTOP = 0x01;
//		stTag.u8RSSI = NRF_RADIO->RSSISAMPLE;
		//NRF_RADIO->TASKS_RSSISTOP = 0x01;
		NRF_RADIO->EVENTS_RSSIEND = 0x00;
	}
	if(NRF_RADIO->EVENTS_END)
	{
		
		if(NRF_RADIO->CRCSTATUS == 0x01)
		{	
			crc8_val = comCRC8(((card_pkg)packet)->nodeid_1,15,0x69);
			crc8_val ^= ((card_pkg)packet)->cardid[0];
			crc8_val ^= ((card_pkg)packet)->cardid[1];
			if(((card_pkg)packet)->cardrssi == crc8_val)
			{
				DISABLE_GLOBAL_INTERRUPT();
				((card_pkg)packet)->cardrssi = NRF_RADIO->RSSISAMPLE;
				insert_data_into_double_link(&RADIO_DATA_LIST_HEAD,packet,18);
				num = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
				ENABLE_GLOBAL_INTERRUPT();
			}

//			RFWorkTime = 0;
////			nrf_gpio_pin_set(GPIO_SOS_LED);
//			packet[2] = ((packet[2]&0xC0)>>6);
//			
//			if(packet[2] == 0)
//				u8Rssi = NRF_RADIO->RSSISAMPLE;
//			else if(packet[2] == 1)
//				u8Rssi = NRF_RADIO->RSSISAMPLE-8;
////			else
////				u8Rssi = NRF_RADIO->RSSISAMPLE-16;
//			else
//				u8Rssi = NRF_RADIO->RSSISAMPLE+16;
//			packet[2] = 0;
//			for(TempReg = 0;TempReg < u8TagNum; TempReg++)
//			{   // �����洢���飬�����Ƿ����ظ��Ĵӽڵ��¼���еĻ��������ݸ���
//				if((comMEMCMP(stTag[TempReg].ID, &packet[0], 2) == 0) )
//				{
//					comMEMCPY(stTag[TempReg].ID, &packet[0], 2);
//					if(packet[2] >= stTag[TempReg].u8Flag)
//					{
//						stTag[TempReg].u8Flag = packet[2];
//						stTag[TempReg].u8RSSI = (stTag[TempReg].u8RSSI + u8Rssi)/2;
////						stTag[TempReg].u8Flag = stTag[TempReg].u8Flag|0x80;
//					}
//					break;
//				}
//			}
//			
//			if(TempReg == u8TagNum)
//			{   
//				// û���ҵ��ظ��Ĵӽڵ��¼������û��������׷��
//				if(u8TagNum < 50)
//				{//����û��������׷��
//					comMEMCPY(stTag[TempReg].ID, &packet[0], 2);
//					stTag[TempReg].u8Flag = packet[2];
//					stTag[TempReg].u8RSSI = u8Rssi;
////					stTag[TempReg].u8Flag = (stTag[TempReg].u8Flag|0x40);
//					u8TagNum++;
//				}
//				else
//				{//�����������Ƚ��źű�־����ǿֵ�����������ݴ����������ݸ���	
//					u8dat = packet[2];
//					for(i = 0; i < 50; i++)
//					{
//						if( (u8dat > stTag[i].u8Flag) 
//						   ||( (u8dat == stTag[i].u8Flag) 
//							  &&(stTag[i].u8RSSI > NRF_RADIO->RSSISAMPLE)) )
//						{
//							u8dat = stTag[i].u8Flag;
//							j = i;
////							comMEMCPY(stTag[i].ID, &packet[0], 2);
////							stTag[i].u8Flag = 3;//packet[2];
////							stTag[i].u8RSSI = u8Rssi;
////							stTag[i].u8Flag |=0x20;
////							break;
//						}
//					}
//					if(j < 50)	// �ҵ��ȵ�ǰ�����źŸ��������
//					{
//						if( (packet[2] > stTag[j].u8Flag) 
//						   ||( (packet[2] == stTag[j].u8Flag) 
//							  &&(stTag[j].u8RSSI > NRF_RADIO->RSSISAMPLE)) )
//						{
//							comMEMCPY(stTag[j].ID, &packet[0], 2);
//							stTag[j].u8Flag = packet[2];
//						}
//					}
//				}
//			}
			
		}
		NRF_RADIO->EVENTS_END  = 0U;  				
		NRF_RADIO->TASKS_START = 1U;
	}     	
}

/** 
 * @}
 */
