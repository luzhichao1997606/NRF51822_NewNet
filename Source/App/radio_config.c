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
#define PACKET0_S1_SIZE                  (0UL)  //!< 此例程我们不理会
#define PACKET0_S0_SIZE                  (0UL)  //!< 此例程我们不理会
#define PACKET0_PAYLOAD_SIZE             (0UL)  //!< 此例程我们不理会
#define PACKET1_BASE_ADDRESS_LENGTH      (2UL)  //!< base address长度设定
#define PACKET1_STATIC_LENGTH            (18UL)  //!< 每次发送固定长度为4个字节的数据包
#define PACKET1_PAYLOAD_SIZE             (18UL)  //!< 保持最大数据长度跟STALEN一致
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

uint8_t packet[18];  ///< 发送BUFFER
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

//无线功率4分贝，2440MHz，通道0地址0x99999999C0，通道1~7地址0x66666666XX
//速率2MHz，数据字节3字节QQXXCC(QQ开始，CC结束，xx是数据char)
void radio_configure()//无线配置，准备和nrf24L01通讯
{
  //无线功率04：+4分贝，0：0分贝，FC：-4分贝，F8：-8分贝
	//    F4：-12分贝，F0：-16分贝，EC：-20分贝，D8：-30分贝
	NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);  //设定发射功率，这里是4DBm
  
	NRF_RADIO->FREQUENCY = 13UL;//无线频率12MHz+2400MHz=2412MHz

	//无线速率：00：1Mbit，01：2Mbit，02：250Kbit，03：1Mbit（蓝牙）
	NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);//速率250Kbit

	// 无线地址设置
//  NRF_RADIO->PREFIX0 = 0xC3C2C18AUL;  // 通道3 到 0 的低1字节
//  NRF_RADIO->PREFIX1 = 0xC7C6C5C4UL;  // 通道7 到 4 的低1字节
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

  NRF_RADIO->BASE0   = bytewise_bitswap(0x63994567UL);  // 通道0的高字节
  NRF_RADIO->BASE1   = bytewise_bitswap(0x89ABCDEFUL);  // 通道1-7的高字节
  NRF_RADIO->TXADDRESS = 0x00UL;      // 发射使用的通道号：0通道
  NRF_RADIO->RXADDRESSES = 0x01UL;    // 接收的通道号：1通道

  // 配置包0的设置
  NRF_RADIO->PCNF0 = (PACKET0_S1_SIZE<<RADIO_PCNF0_S1LEN_Pos)| //S1领域的长度
                     (PACKET0_S0_SIZE << RADIO_PCNF0_S0LEN_Pos) | //S0场的长度
                     (PACKET0_PAYLOAD_SIZE << RADIO_PCNF0_LFLEN_Pos);  //长度字段中的比特数

  // 配置包1的设置
   NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos)    | //效验位（0关，1开）
                      (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)           | //数据大小端（高低字节哪个先发 0低字节，1高字节）
                      (PACKET1_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos)       | //通道1~7高字节长度（ nrf24高低字节5字节：4个高+1个低）
                      (PACKET1_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos)           | //数据字节长度（255~1）3字节QxC
                      (PACKET1_PAYLOAD_SIZE << RADIO_PCNF1_MAXLEN_Pos);   //硬件传输字节长度（255~1）3字节QxC

  // CRC 校验长度配置
  NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // 校验长度 2个char
  if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk)== (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos) )
  {
    NRF_RADIO->CRCINIT = 0xFFFFUL;      // 校验初始值
    NRF_RADIO->CRCPOLY = 0x11021UL;     // CRC poly: x^16+x^12^x^5+1
  }
  else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
  {
    NRF_RADIO->CRCINIT = 0xFFUL;        // 校验初始值
    NRF_RADIO->CRCPOLY = 0x107UL;       // CRC poly: x^8+x^2^x^1+1
  }
	
  m_radio_states = RADIO_STATE_READER;
//接收寄存器是  NRF_RADIO->PACKETPTR	
}

/*接收准备*/
void reader_Radio_Rx_Rdy(void)
{  
	NRF_RADIO->PACKETPTR = (uint32_t)packet;			// DMA指针指向接收BUFFER

	NRF_RADIO->EVENTS_DISABLED = 0U;         // 无线关闭，标志位
	NRF_RADIO->TASKS_DISABLE   = 1U;         // 关闭无线设备
	while (NRF_RADIO->EVENTS_DISABLED == 0U) // 等待设备关闭
	{
	}		
	NRF_RADIO->EVENTS_READY = 0U; 				 // 事件准备 收发模式转换完成  标志位    
	NRF_RADIO->TASKS_RXEN   = 1U;          // 使能接收
	NRF_RADIO->TASKS_TXEN   = 0;
	while(NRF_RADIO->EVENTS_READY == 0U)   // 等待接收准备好
	{
	}
	NRF_RADIO->EVENTS_END  = 0U;  					// 结束事件			
	NRF_RADIO->TASKS_START = 1U;           // 开始
	/*中断使能*/
	NRF_RADIO->INTENSET  = 0x8A;
	NVIC_EnableIRQ(RADIO_IRQn);
}

/*过滤标签数据，通过返回true，不通过返回false*/
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

/*radio接收中斷*/
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
//			{   // 遍历存储数组，查找是否有重复的从节点记录，有的话用新数据覆盖
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
//				// 没有找到重复的从节点记录，数据没满，往后追加
//				if(u8TagNum < 50)
//				{//数据没满，往后追加
//					comMEMCPY(stTag[TempReg].ID, &packet[0], 2);
//					stTag[TempReg].u8Flag = packet[2];
//					stTag[TempReg].u8RSSI = u8Rssi;
////					stTag[TempReg].u8Flag = (stTag[TempReg].u8Flag|0x40);
//					u8TagNum++;
//				}
//				else
//				{//数据已满，比较信号标志、场强值，比已有数据大，则用新数据覆盖	
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
//					if(j < 50)	// 找到比当前数据信号更差的数据
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
