/*-------------------------------------------------*/
/*                                                 */
/*            ʵ�����繦�ܵ�Դ�ļ�             */
/*                                                 */
/*-------------------------------------------------*/

#include "hal.h"    //������Ҫ��ͷ�ļ�
#include "string.h"       //������Ҫ��ͷ�ļ�
#include "stdio.h"        //������Ҫ��ͷ�ļ�

#include "net.h"          //������Ҫ��ͷ�ļ�

#include "wizchip_conf.h" //������Ҫ��ͷ�ļ�
#include "socket.h"       //������Ҫ��ͷ�ļ�
#include "dhcp.h"         //������Ҫ��ͷ�ļ�
#include "dns.h"          //������Ҫ��ͷ�ļ�

#include "MQTTClient.h"

#include "radio_config.h"

//#include "wizchip_conf.h"
//#include "socket.h"
//#include "w5500.h"
//#include "dhcp.h"         //������Ҫ��ͷ�ļ�

#include "nrf_nvmc.h"
//#include "SPI1.h"
#include "Uart.h"

#include "cJSON.h"

uint8 Module_Info[] = {"WSN-H04 V1.0 20191202"};

uint8_t Read_ID[16] = {"4001000726123480"}; 
unsigned char  gDATABUF[DATA_BUF_SIZE];    //���ݻ�����
//unsigned char  gDNSBUF[DATA_BUF_SIZE];     //���ݻ�����
char DNS_flag = 0;              //DNS������־  0:δ�ɹ�  1���ɹ�
int  tcp_state;                 //���ڱ���TCP���ص�״̬
int  temp_state;                //������ʱ����TCP���ص�״̬
char my_dhcp_retry = 0;         //DHCP��ǰ�������ԵĴ���


uint16_t   m_DEVICE_ID = 0x0315;
netpara_t            m_netpara;            //�������
para_cfg_t           m_cfgpara;

uint8_t              m_rssi_Calibration;
uint8_t              m_factor;


unsigned char tempBuffer[2048];
unsigned char SendBuffer[3072]= {"{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":3132213B2470E251012401020304}"};



uint8 Work_Flag = 0;	// ��ʼ����������־
uint8 Phy_Link_Flag = 0xff;	// ��������״̬


uint16 check_time;
uint16 work_time;
uint8 m2m_status = 0;	// M2Mģ��״̬

#define WORK_INIT			0
#define WORK_DHCP			1
#define WORK_TCP           	2
#define WORK_TCP_FATAL     	3
#define WORK_M2M_nPHY		4
#define WORK_M2M_nDHCP		5
#define WORK_M2M_nTCP       	6

uint16 PHY_time = 0xFFFF;
uint16 DHCP_time = 0xFFFF;
uint16 TCP_time = 0xFFFF;
uint16 MQTT_time = 0xFFFF;

uint8_t m_data_sendbuf[1356];

wiz_NetInfo gWIZNETINFO =       //MAC��ַ�Լ�дһ������Ҫ��·�������������豸һ������
{        
0x00, 0x08, 0xdc,0x00, 0xab, 0xcd, 
}; 
unsigned char dns2[4]={114,114,114,114};  //DNS����������IP


unsigned char ServerIP[4];                                   //��ŷ�����IP
unsigned char ServerName[128] = {"mqtt.dawen.ltd"};                               //��ŷ���������
//int  ServerPort = 7611;                                             //��ŷ������Ķ˿ں�

MQTTClient c;
MQTTMessage mqtt_msg;
char SubTopic[120] = "/WSN-LW/xxxxxxxx/service/xxxxx";
char SubTopic0[40] = "/WSN-LW/xxxxxxxx/service/xxxxx";
char SubTopic1[40] = "/WSN-LW/xxxxxxxx/service/xxxxx";
char SubTopic2[40] = "/WSN-LW/xxxxxxxx/service/xxxxx";

char PubTopic[40] = "/WSN-LW/xxxxxxxx/event/xxxxx";
MQTT_Param_t		MQTT_Param;
FLASH_Param_t		FLASH_Param;

void my_ip_assign(void);
void my_ip_conflict(void);
void network_init(void);

void Rf_Working(void);
void send_heartpacket(void);
void send_datapacket(void);

void NetData_processing(uint8_t *rcvdata,uint16_t len);

void setkeepalive(SOCKET s);

void MQTT_Send_task(void);
void MQTT_Package_msg(uint8 *payload, uint8 StartNum, uint16 Num);
void MQTT_Package_Heartbeat(uint16 sn, uint8 resetFlag);

/*Ĭ���������*/
const  netpara_t default_netpara =
{
	.server_ip[0]=101,                //������Ĭ��IP
	.server_ip[1]=37,
	.server_ip[2]=13,
	.server_ip[3]=31,

	.server_port=1883,						         //������Ĭ�϶˿ں�
	
//	.server_ip[0]=121,                //������Ĭ��IP
//	.server_ip[1]=43,
//	.server_ip[2]=164,
//	.server_ip[3]=164,

//	.server_port=7211,						         //������Ĭ�϶˿ں�
	
	.local_Port=4120,					           //�ͻ���Ĭ�϶˿ں�
  	.local_IP[0]=192,          //�ͻ���Ĭ��IP
	.local_IP[1]=168,
	.local_IP[2]=3,
	.local_IP[3]=168,
	
  	.local_sunNet[0]=255,         //Ĭ��������û
	.local_sunNet[1]=255,
	.local_sunNet[2]=255,
	.local_sunNet[3]=0,
	
	.mac[0] = 0x02,
	.mac[1] = 0x17,
	.mac[2] = 0x9a,
	.mac[3] = 0x68,
	.mac[4] = 0x00,
	.mac[5] = 0x01,
	
  	.local_GW[0]=192,              //Ĭ������
	.local_GW[1]=168,
	.local_GW[2]=3,
	.local_GW[3]=1,
	
	.dns[0]=192,              //Ĭ������
	.dns[1]=168,
	.dns[2]=3,
	.dns[3]=1,
};


////װ���������
////˵��: ���ء��������롢�����ַ������IP��ַ�������˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
void Load_Net_Parameters(void)
{
	gWIZNETINFO.mac[0]=m_netpara.mac[0];//���������ַ
	gWIZNETINFO.mac[1]=m_netpara.mac[1];
	gWIZNETINFO.mac[2]=m_netpara.mac[2];
	gWIZNETINFO.mac[3]=m_netpara.mac[3];
	gWIZNETINFO.mac[4]=m_netpara.mac[4];
	gWIZNETINFO.mac[5]=m_netpara.mac[5];
	
	gWIZNETINFO.ip[0]=m_netpara.local_IP[0];//���ر���IP��ַ
	gWIZNETINFO.ip[1]=m_netpara.local_IP[1];
	gWIZNETINFO.ip[2]=m_netpara.local_IP[2];
	gWIZNETINFO.ip[3]=m_netpara.local_IP[3];
	
	gWIZNETINFO.sn[0]=m_netpara.local_sunNet[0];//������������
	gWIZNETINFO.sn[1]=m_netpara.local_sunNet[1];
	gWIZNETINFO.sn[2]=m_netpara.local_sunNet[2];
	gWIZNETINFO.sn[3]=m_netpara.local_sunNet[3];
	
	gWIZNETINFO.gw[0] = m_netpara.local_GW[0];//�������ز���
	gWIZNETINFO.gw[1] = m_netpara.local_GW[1];
	gWIZNETINFO.gw[2] = m_netpara.local_GW[2];
	gWIZNETINFO.gw[3] = m_netpara.local_GW[3];
	
	gWIZNETINFO.dns[0]=m_netpara.dns[0];//���ر���IP��ַ
	gWIZNETINFO.dns[1]=m_netpara.dns[1];
	gWIZNETINFO.dns[2]=m_netpara.dns[2];
	gWIZNETINFO.dns[3]=m_netpara.dns[3];
	
	ServerIP[0] = m_netpara.server_ip[0];
	ServerIP[1] = m_netpara.server_ip[1];
	ServerIP[2] = m_netpara.server_ip[2];
	ServerIP[3] = m_netpara.server_ip[3];
//	ServerPort	= m_netpara.server_port; 
	
	gWIZNETINFO.dhcp = NETINFO_STATIC;
}

/*�洢���*/
void  store_clear(uint32_t page)
{
	 nrf_nvmc_page_erase(page*1024ul);
}
/*�����洢*/
void  para_store(uint32_t page,para_cfg_t para)
{
	nrf_nvmc_write_bytes(page*1024ul,(const uint8_t*)&para,sizeof(para_cfg_t));
}
/*������ȡ*/
void  para_read(uint32_t page,para_cfg_t *ppara)
{
	volatile uint8_t *p;
	
	p =(volatile uint8_t*)(page*1024ul);
	
	memcpy((uint8_t*)ppara,(uint8_t*)p,sizeof(para_cfg_t));
	if(ppara->magic == STORE_MAGIC)
		 return;
	else
	{
		memcpy((uint8_t*)&(ppara->net_cfg),(uint8_t*)&default_netpara,sizeof(para_cfg_t));
		ppara->net_cfg.mac[2] = (NRF_FICR->DEVICEID[0]>>24)&0xFF;
		ppara->net_cfg.mac[3] = (NRF_FICR->DEVICEID[0]>>16)&0xFF;
		ppara->net_cfg.mac[4] = (NRF_FICR->DEVICEID[0]>>8)&0xFF;
		ppara->net_cfg.mac[5] = (NRF_FICR->DEVICEID[0])&0xFF;
		ppara->magic          = STORE_MAGIC;
		ppara->rssi_Calibration = 63;	// rssi 1m�궨ֵ
		ppara->factor	       = 32;	// ��������
		
		store_clear(STORE_PAGE);
		para_store(STORE_PAGE,*ppara);
	}
	return;
}

/*-------------------------------------------------*/
/*�������������ٽ���                               */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);    //��ֹȫ���ж�
}
/*-------------------------------------------------*/
/*���������˳��ٽ���                               */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);   //��ȫ���ж�
}

/*-------------------------------------------------*/
/*�������� Ƭѡ�ź�����͵�ƽ                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CS_Select(void)
{
	nrf_gpio_pin_clear(W5500_SPI_CS_PIN);
}

/*-------------------------------------------------*/
/*�������� Ƭѡ�ź�����ߵ�ƽ                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CS_Deselect(void)
{
	nrf_gpio_pin_set(W5500_SPI_CS_PIN); 
}
/*-------------------------------------------------*/
/*�������� Ƭѡ�ź�����ߵ�ƽ                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_Reset(void)
{
	nrf_gpio_pin_clear(W5500_RST_PIN);
	delay_ms(500);
	nrf_gpio_pin_set(W5500_RST_PIN);
	delay_ms(2000);
}


/*-------------------------------------------------*/
/*����������ʼ��W5500                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_init(void)
{
	//W5500�շ��ڴ�������շ������������ܵĿռ���16K����0-7��ÿ���˿ڵ��շ����������Ƿ��� 2K
	char memsize[2][8] = {{2,4,2,2,2,2,2,0},{2,4,2,2,2,2,2,0}}; 
	char tmp;
	uint8_t old_flag = Work_Flag;
	W5500_Reset();   
	//W5500��λ
	wiz_NetInfo netinfoqq;

	ctlnetwork(CN_GET_NETINFO, (void*)&netinfoqq);	//��ȡ�������
	
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	//ע���ٽ�������
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  //ע��SPIƬѡ�źź���
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	//ע���д����
	
	Load_Net_Parameters();
	
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){   //���if��������ʾ�շ��ڴ����ʧ��
		UART_Printf("��ʼ���շ�����ʧ��,׼������\r\n");      //��ʾ��Ϣ
		NVIC_SystemReset();                                //����
	}	
	PHY_time = 10000;//10S
	do{                                                     //�������״̬
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){ //���if��������ʾδ֪����		 
			UART_Printf("δ֪����׼������\r\n");            //��ʾ��Ϣ
			NVIC_SystemReset();                             //����
		}
		if(tmp == PHY_LINK_OFF){
			UART_Printf("����δ����\r\n");//�����⵽������û���ӣ���ʾ��������
			delay_ms(2000);              //��ʱ2s
		}
		nrf_drv_wdt_feed();
	}while((tmp == PHY_LINK_OFF) && (Work_Flag == old_flag));                            //ѭ��ִ�У�ֱ������������
	if(tmp == PHY_LINK_ON)
	{
		Work_Flag = WORK_DHCP;
		PHY_time = 0xFFFF;
		DHCP_time = 30000;	//30s
	}
	setSHAR(gWIZNETINFO.mac);                                   //����MAC��ַ
	DHCP_init(SOCK_DHCP, gDATABUF);                             //��ʼ��DHCP
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);//ע��DHCP�ص����� 
	DNS_flag = 0;                                               //DNS������־=0 
	my_dhcp_retry = 0;	                                        //DHCP���Դ���=0
	tcp_state = 0;                                              //TCP״̬=0
	temp_state = -1;                                            //��һ��TCP״̬=-1
	
	setkeepalive(SOCK_TCPC);
//	disconnect(SOCK_TCPC);
}

void setkeepalive(SOCKET s)
{
  //IINCHIP_WRITE( Sn_CR(0) , Sn_CR_SEND_KEEP );  
  IINCHIP_WRITE(Sn_KPALVTR(s),0x01);
}

void send_ka(SOCKET s)
{
  IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND_KEEP);
  return;
}
/*-------------------------------------------------*/
/*��������W5500��������                            */
/*��  ����data:����                                */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_TxData(unsigned char *data)
{
	send(SOCK_TCPC,&data[1],data[0]);  //W5500��������
}

/*-------------------------------------------------*/
/*��������DNS��������                              */
/*��  ����DomainName:����                          */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_DNS(unsigned char *DomainName)
{
	char ret;
	
	UART_Printf("��ʼ��������\r\n");                         //���������Ϣ
//	DNS_init(SOCK_DNS, gDNSBUF);                         //DNS��ʼ��
	DNS_init(SOCK_DNS, gDATABUF);                         //DNS��ʼ��
	ret = DNS_run(dns2,DomainName,m_netpara.server_ip);               //��������
	UART_Printf("DNS��������ֵ��%d\r\n",ret);                //�����������
	if(ret==1){                                            //����ֵ����1����ʾ������ȷ
		UART_Printf("DNS�����ɹ���%d.%d.%d.%d\r\n",m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3]);//���������Ϣ
	    DNS_flag = 1;                                      //DNS�����ɹ�����־=1
		close(SOCK_DNS);                                 //�ر�DNS�˿�
	}
}

/*-------------------------------------------------*/
/*����������ȡ��IPʱ�Ļص�����                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void my_ip_assign(void)
{
	getIPfromDHCP(gWIZNETINFO.ip);     //�ѻ�ȡ����ip��������¼����������
	getGWfromDHCP(gWIZNETINFO.gw);     //�ѻ�ȡ�������ز�������¼����������
	getSNfromDHCP(gWIZNETINFO.sn);     //�ѻ�ȡ�������������������¼����������
	getDNSfromDHCP(gWIZNETINFO.dns);   //�ѻ�ȡ����DNS��������������¼����������
	gWIZNETINFO.dhcp = NETINFO_DHCP;   //���ʹ�õ���DHCP��ʽ
	network_init();                    //��ʼ������  
	UART_Printf("DHCP���� : %d ��\r\n", getDHCPLeasetime());
}
/*-------------------------------------------------*/
/*����������ȡIP��ʧ�ܺ���                         */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void my_ip_conflict(void)
{
	UART_Printf("��ȡIPʧ�ܣ�׼������\r\n");   //��ʾ��ȡIPʧ��
	NVIC_SystemReset();                      //����
}
/*-------------------------------------------------*/
/*����������ʼ�����纯��                           */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void network_init(void)
{
	char tmpstr[6] = {0};
	wiz_NetInfo netinfo;
	
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);//�����������
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);	//��ȡ�������
	ctlwizchip(CW_GET_ID,(void*)tmpstr);	        //��ȡоƬID	"W5500"
	
	//��ӡ�������
	if(netinfo.dhcp == NETINFO_DHCP) 
		UART_Printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
	else 
		UART_Printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);	
	UART_Printf("===========================\r\n");
	UART_Printf("MAC��ַ: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);			
	UART_Printf("IP��ַ: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	UART_Printf("���ص�ַ: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	UART_Printf("��������: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	UART_Printf("DNS������: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	UART_Printf("===========================\r\n");
}


/*******************************************************************************
 * Function Name: Net_Time_Work
 * Decription	: ��ʱ����������
 * Calls		: 
 * Called By	: TIMER2_IRQHandler()1ms��ʱ��
 * Arguments	:   
 * Returns		: 
 * Others		: ע������
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
volatile uint16 heartTicks;
//uint8_t M2M_Send_Flag = 0;
uint8_t TCP_Send_Flag = 0;
uint8_t Connect_flag = 0;
uint16 delay_time;

uint16 SendTime;

//uint8 TCP_Connect = 0;

// LED״̬
uint8_t LED_Flag = 0;
uint16 LED_Time;

uint8 SOCK_NOK_NUM = 0;	// socketʧ����������

void Net_Time_Work(void)
{
//	char tmp;
	/*--------------------------------------------------------------------*/
	/*   Connect_flag=1ͬ����������������,���ǿ��Է������ݺͽ���������    */
	/*--------------------------------------------------------------------*/
	if(Connect_flag==1)
	{
		heartTicks++;
		if(Work_Flag == WORK_TCP)
		{
			if(TCP_Send_Flag == 0)
			{
				TCP_Send_Flag = 1;
				LED_Flag = 1;	// LED���Ʊ�־
				SendTime = 0;	// ���ͳ�ʱ����
				SOCK_NOK_NUM = 0;	
				TCP_time = 10000;
				send_datapacket();	//�����ϱ����ݺ���	
//				send_ka(SOCK_TCPC);
			}
			else
			{
				if(heartTicks >= 1000)	// ������ʱ������������
				{
					disconnect(SOCK_TCPC);
					Connect_flag = 0;
					heartTicks = 0;		//����������ʱ
				}
			}
		}
	}

	/******LED ����˸ ********/
	if(LED_Flag == 1)	// ��Ʊ�־
	{
		LED_Time++;
		if(LED_Time < 200)	// ���ʱ��200ms
			nrf_gpio_pin_clear(DATA_LED);	// ����LED��
		else
		{
			LED_Time = 0;
			LED_Flag = 0;
			nrf_gpio_pin_set(DATA_LED);		// �ر�LED��
		}
		
	}
	/************************/
	
	
//	/******TCP���ӳ�ʱ��� ********/
//	if(TCP_Connect == 1)
//	{
//		TCP_Time++;
//		if(TCP_Time > 5000)
//		{
//			TCP_Time = 0;
//			Work_Flag = 3;
//		}
//	}
//	/************************/
	
	// ��ʼ���������ӳ�ʱ���
	if((PHY_time != 0xFFFF)&& (PHY_time > 0))
	{
		PHY_time--;
		if(PHY_time == 0)
		{
			UART_Printf("WORK_M2M_nPHY\r\n");
			Work_Flag = WORK_M2M_nPHY;
		}
	}
	
	// DHCP��ʱ���
	if((DHCP_time != 0xFFFF)&& (DHCP_time > 0))
	{
		DHCP_time--;
		if(DHCP_time == 0)
		{
			UART_Printf("WORK_M2M_nDHCP\r\n");
			Work_Flag = WORK_M2M_nDHCP;
		}
	}
	
	// TCP��ʱ���
//	if((TCP_time != 0xFFFF)&& (TCP_time > 0))
//	{
//		TCP_time--;
//		if(TCP_time == 0)
//		{
//			UART_Printf("WORK_TCP_FATAL\r\n");
//			Work_Flag = WORK_TCP_FATAL;
//		}
//	}
	
	if((MQTT_time != 0xFFFF)&& (MQTT_time > 0))
	{
		MQTT_time--;
//		if(MQTT_time == 0)
//		{
//			UART_Printf("WORK_TCP_FATAL\r\n");
//			Work_Flag = WORK_TCP_FATAL;
//		}
	}
	
	/******TCP�����쳣 �ȴ�����60s ********/
	if(Work_Flag == WORK_TCP_FATAL)
	{
		delay_time++;
		if(delay_time <= 500)
		{
			delay_time = 501;
			UART_Printf("�ر���������\r\n");
			DHCP_stop();
			close(SOCK_TCPC);
		}
		else if(delay_time > 60000) //60s
		{
			SOCK_NOK_NUM++;
			if(SOCK_NOK_NUM > 5)
			{
				SOCK_NOK_NUM = 0;
				Work_Flag = WORK_M2M_nTCP;
			}
			else
				Work_Flag = WORK_INIT;
			delay_time = 0;
			UART_Printf("��ʱ��������\r\n");
			
		}
		
	}
	/************************/
	
	/******MQTT ��ʱ���� ********/
	// todo �����ж�
//	MQTT_Send_task();
	/************************/
}


void MQTT_Send_task(void)
{
	MQTT_Param.time++;
	if(MQTT_Param.time > 60000) // 1����
	{
		uint8 i;
		MQTT_Param.time = 0;
		
		if( MQTT_Param.SerialNumber >= 65535 )
			MQTT_Param.SerialNumber = 20;
		
		MQTT_Param.SensorAll = 2;	// 1��ȫ������
		
		for(i=0; i<FLASH_Param.SensorNum; i++)	// ���������ݼ��
		{
			SensorTime[i] += 1;	//���������ݳ�ʱ��ʱ+1
			if(SensorTime[i] > FLASH_Param.SensorCycle)	//���������ݳ�ʱ�����жϣ����������������ϱ����ڣ���������
			{
				SensorTime[i] = 0;
				if(comMEMCMPByte(SensorData[i], 0, 5)==1)	//���ڷ�������
				{
					memset(SensorData[i], 0, 5);
					MQTT_Param.OfflineFlag = 1;	//��������״̬
				}
			}
		}
		MQTT_Param.SerialNumber += 1;	// ���������+1	
		
		if( (MQTT_Param.SerialNumber%FLASH_Param.Cycle != 0 || MQTT_Param.SerialNumber <= 20) && MQTT_Param.OfflineFlag == 0 )		// �����������ݰ�
		{
//		msis:send_Heartbeat_data(SerialNumber,ResetFlag)
//		MQTT_Param.ResetFlag = 0; //�����λ״̬	
			MQTT_Package_Heartbeat(MQTT_Param.SerialNumber, 1);
			FLASH_Param.ResetFlag = 0;	//�����λ״̬	
		}
		else if( (MQTT_Param.SerialNumber%FLASH_Param.Cycle == 0 && MQTT_Param.SerialNumber > 20) || MQTT_Param.OfflineFlag == 1 )		// �������ݰ����ϵ�ǰ20���Ӳ��ϱ�����
		{
//		StartSend()
			MQTT_Package_msg(SensorData[0],1,20);
			MQTT_Param.OfflineFlag = 0;	//�������״̬
			
		}
	
	}
}

void SendTimeOut(void)
{
	SendTime++;
	if(SendTime > 360)//6����û�з�������
	{
		UART_Printf("���ͳ�ʱ��׼������\r\n");     //���������Ϣ
		NVIC_SystemReset();	                       //����
	}
		
}

uint16 Check_NUM = 0;
uint8 Phy_Link_Flag_st = 0xff;
void Phy_Link_Check(void)
{
	char tmp;
	if(Work_Flag >= WORK_M2M_nPHY)
	{
		Check_NUM++;
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
		{ //���if��������ʾδ֪����		 
			UART_Printf("δ֪����׼������\r\n");            //��ʾ��Ϣ
			NVIC_SystemReset();                             //����
		}
		if(tmp == PHY_LINK_OFF)
		{
			if(Phy_Link_Flag_st != tmp)
			{
				Check_NUM = 0;
				Phy_Link_Flag_st = PHY_LINK_OFF;
			}
			if(Check_NUM > 5)
			{
//				UART_Printf("����δ����\r\n");//�����⵽������û���ӣ���ʾ��������
				Phy_Link_Flag = PHY_LINK_OFF;	// ����δ����
			}
			
			
		}
		else if(tmp == PHY_LINK_ON)
		{
			if(Phy_Link_Flag_st != tmp)
			{
				Check_NUM = 0;
				if(Phy_Link_Flag_st == PHY_LINK_OFF)
					Work_Flag = WORK_INIT;	// ����w5500��ʼ��
				Phy_Link_Flag_st = PHY_LINK_ON;
			}
			if(Check_NUM > 5)
			{
//				UART_Printf("��������\r\n");//�����⵽������û���ӣ���ʾ��������
				Phy_Link_Flag = PHY_LINK_ON;	// ��������
				if((Check_NUM > 300) && (Work_Flag >= WORK_M2M_nDHCP)) // 5�������¼��һ����������
				{
					UART_Printf("���¼��һ����������\r\n");
					Check_NUM = 0;
					Work_Flag = WORK_INIT;	// ����w5500��ʼ��
				}
			}
		}
	}
}

/*******************************************************************************
 * Function Name: RadioData_NetSend
 * Decription	: ���ݱ�����
 * Calls		: 
 * Called By	: 
 * Arguments	:   
 * Returns		: 
 * Others		: ע������
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
void RadioData_NetSend(void)
{
	return;
}


/*******************************************************************************
 * Function Name: send_heartpacket
 * Decription	: ����������
 * Calls		: 
 * Called By	: ��ʱ������
 * Arguments	:   
 * Returns		: 
 * Others		: ע������
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
void send_heartpacket(void)
{
////	uint8_t  heart_data[80];
////	uint16_t check_sum = 0;
////	static uint8_t sn = 0;
////	UART_Printf("��������\r\n");
//////	memcpy(heart_data,"$R,P,H,460,00,573E,6FE4,0,0,460044479509898,3001000726123450",60);
////	memcpy(heart_data,"$R,I,D,460,00,1806,3201,0,0,460040883305431,990140123456700501000",65);
////	send(SOCK_TCPC,heart_data,65);
//	
//	
//	uint8_t  heart_data[80];
//	memcpy(heart_data,"$R,P,H,0,0,0,0,0,0,0,",21);
////	hex2strid(&heart_data[21], id, 16);
//	memcpy(&heart_data[21],Read_ID,16);
//	memcpy(&heart_data[37],"\r\n",2);
//	send(SOCK_TCPC,heart_data,39);
	
}

/*******************************************************************************
 * Function Name: send_datapacket
 * Decription	: ���ݰ�����
 * Calls		: 
 * Called By	: ��ʱ������
 * Arguments	:   
 * Returns		: 
 * Others		: ע������
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
void send_datapacket(void)
{
//	DOUBLE_LINK_NODE *pNode;
//	uint32_t cnt       = 0;
//	uint8_t tag_cnt    = 0;
//	uint8_t i = 0;
//	
//	
//	tag_cnt = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
//	if(tag_cnt ==0)
//	{
//		UART_Printf("��������\r\n");
//		memcpy(m_data_sendbuf,"$R,P,H,0,0,0,0,0,0,0,",21);
//		//	hex2strid(&heart_data[21], id, 16);
//		memcpy(&m_data_sendbuf[21],Read_ID,16);
//		memcpy(&m_data_sendbuf[37],"\r\n",2);
//		send(SOCK_TCPC,m_data_sendbuf,39);
//		return;
//	}
//	if(tag_cnt >= 70) 
//		tag_cnt = 70;
////	xTimerStart(m_heart_timerout_handle,2);  //����������ʱ��ʱ��
//	heartTicks = 0;		//����������ʱ
////	memset(m_data_sendbuf,0,sizeof(m_data_sendbuf));
//	UART_Printf("��������\r\n");
//	memcpy(m_data_sendbuf,"$R,P,D,0,0,0,0,0,0,0,",21);
////	hex2strid(&m_data_sendbuf[21], id, 16);
//	memcpy(&m_data_sendbuf[21],Read_ID,16);
//	m_data_sendbuf[37] = ',';
//	cnt = 38;
//	DISABLE_GLOBAL_INTERRUPT();
//	pNode = (&RADIO_DATA_LIST_HEAD)->next;
//	while(i++ < tag_cnt)
//	{	
//		hex2strid((char *)&m_data_sendbuf[cnt],pNode->data,(NET_RADIO_PACKET_LEN-2)<<1);
//		cnt += (NET_RADIO_PACKET_LEN-2)<<1; 
//		if(pNode->data[6] == 0)
//		{
//			memcpy(&m_data_sendbuf[cnt],"00",2);
//			cnt += 2;
//		}
//		else
//		{
//			memcpy(&m_data_sendbuf[cnt],"10",2);
//			cnt += 2;
//		}
//		m_data_sendbuf[cnt++] = pNode->data[7]/100 + 0x30;
//		m_data_sendbuf[cnt++] = pNode->data[7]%100/10 + 0x30;
//		m_data_sendbuf[cnt++] = pNode->data[7]%10 + 0x30;
//		
//		list_del(pNode);
//		pNode = (&RADIO_DATA_LIST_HEAD)->next;
//		if(i < tag_cnt)
//		{
//			m_data_sendbuf[cnt] = '#';
//			cnt += 1;  
//		}
//	}
//	ENABLE_GLOBAL_INTERRUPT();
//	
//	memcpy(&m_data_sendbuf[cnt],"\r\n",2);
//	cnt += 2;
//	send(SOCK_TCPC,m_data_sendbuf,cnt);
	
}



/*-------------------------------------------------*/
/*������������������		                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
//void Rf_Working(void)
//{
//	if((IRQ_Flag0 == 1)||(IRQ0_READ()==1))//( xSemaphoreTake( BinarySemaphore_0, portMAX_DELAY ) == pdTRUE )//
//	{
//		RF_NUM = 0;
//		rfIRQIntHandler();
//		IRQ_Flag0 = 0;
//	}
//	if((IRQ_Flag1 == 1)||(IRQ1_READ()==1))//( xSemaphoreTake( BinarySemaphore_1, portMAX_DELAY ) == pdTRUE )//
//	{
//		RF_NUM = 1;
//		rfIRQIntHandler();
//		IRQ_Flag1 = 0;
//	}
//	if((IRQ_Flag2 == 1)||(IRQ2_READ()==1))//( xSemaphoreTake( BinarySemaphore_2, portMAX_DELAY ) == pdTRUE )//
//	{
//		RF_NUM = 2;
//		rfIRQIntHandler();
//		IRQ_Flag2 = 0;
//	}
//	if(check_time > 500)
//	{
//		check_time = 0;
//		/*********433���**********/
//		Rf_Check();
//		/*********433���**********/
//	}
//}

/*-------------------------------------------------*/
/*������������������		                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
uint8 SYNSENT_NUM = 0;

//void TCP_Working(void)
//{
//	int  ret;                       //���ڱ��溯������ֵ
//	uint8 buff[200];
//	
//	tcp_state=getSn_SR(SOCK_TCPC);                //��ȡTCP���Ӷ˿ڵ�״̬
//	if(tcp_state!=temp_state)
//	{                    //������ϴε�״̬��һ��
//		temp_state =  tcp_state;                  //���汾��״̬
//		UART_Printf("״̬����:0x%x\r\n",tcp_state); //���������Ϣ	
//	}	
//	if(ctlwizchip(CW_GET_PHYLINK, (void*)&ret) == -1)
//	{  //���if��������ʾδ֪����		 
//		UART_Printf("δ֪����׼������\r\n");             //��ʾ��Ϣ
//		NVIC_SystemReset();                              //����
//	}
//	if(ret == PHY_LINK_OFF)
//	{         //�ж������Ƿ�Ͽ�
//		UART_Printf("���߶Ͽ�\r\n");   //�����⵽�����߶Ͽ���������ʾ��Ϣ	
//		delay_ms(1000);              //��ʱ1s
//		TCP_time = 0xFFFF;			// ֹͣTCP��ʱ���
//		Work_Flag = WORK_INIT;		// ���߶Ͽ������³�ʼ��w5500
//	}
//	
//	switch(tcp_state)
//	{ //switch��䣬�жϵ�ǰTCP���ӵ�״̬   	
//		
//	  case SOCK_SYNSENT: 
//		UART_Printf("SOCK_SYNSENT\r\n");   //�����⵽�����߶Ͽ���������ʾ��Ϣ	
//		delay_ms(1000);              //��ʱ1s
//		break;
//	  case SOCK_INIT:          
//		if(Connect_flag==0)
//		{                                //�����û�����ӷ�����������if
//			UART_Printf("׼�����ӷ�����\r\n");                //���������Ϣ
//			ret = connect(SOCK_TCPC,m_netpara.server_ip,m_netpara.server_port);   //���ӷ�����
//			UART_Printf("IP��ַ: %d.%d.%d.%d,%d\r\n", m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3],m_netpara.server_port);
//			UART_Printf("���ӷ����������룺%d\r\n",ret);      //���������Ϣ
//			if(ret != SOCK_OK)
////			while(ret != SOCK_OK)	
//			{                         //������δ��������if
//				UART_Printf("SOCKET ���ӵȴ�...\r\n",ret);      //���������Ϣ
//				delay_ms(300);                             //��ʱ3s
//				
////			disconnect(SOCK_TCPC);
////				ret = connect(SOCK_TCPC,m_netpara.server_ip,m_netpara.server_port);   //���ӷ�����
//			}
//			delay_ms(300);   
//		}
//		break;	                                       //����
//		
//	  case SOCK_ESTABLISHED:   
//		if((Connect_flag==0)&&(getSn_IR(SOCK_TCPC)==Sn_IR_CON))
//		{  //�ж������Ƿ���													
//			UART_Printf("�����ѽ���\r\n");                          //���������Ϣ
//			Connect_flag = 1;                                     //���ӱ�־=1
//			setSn_IR(SOCK_TCPC, Sn_IR_CON);						/*��������жϱ�־λ*/
//		}
//		if(getSn_IR(SOCK_TCPC) & Sn_IR_CON)
//		{
//			setSn_IR(SOCK_TCPC, Sn_IR_CON);								          /*��������жϱ�־λ*/
//		}
//		heartTicks = 0;		//����������ʱ
////		send_datapacket();
//		ret = getSn_RX_RSR(SOCK_TCPC);	     /*����lenΪ�ѽ������ݵĳ���*/
//		if(ret > 0)
//		{					                    //���ret����0����ʾ��������		 
//			recv(SOCK_TCPC,buff,ret);								              	/*��������Client������*/
//			UART_Printf("%s\r\n",buff);
////			NetData_processing(buff, ret);	// �������ݴ���
//		}		
//		break; //����
//		
//	  case SOCK_CLOSE_WAIT:      
//		UART_Printf("�Ķ���ID��");
//		Uart_SendData(Read_ID,sizeof(Read_ID));
//		UART_Printf("\r\n");
//		UART_Printf("�ȴ��ر�����\r\n");                   //���������Ϣ
//		if((ret=disconnect(SOCK_TCPC)) != SOCK_OK)       //�˿ڹر�
//		{
//			UART_Printf("���ӹر�ʧ�ܣ�׼������\r\n");      //���������Ϣ
////			NVIC_SystemReset();	                       //����
//		}
//		UART_Printf("���ӹرճɹ�\r\n");                    //���������Ϣ
//		TCP_Send_Flag = 0;
//		Connect_flag = 0;                                 //���ӱ�־=0
//		break;
//		
//	  case SOCK_CLOSED:          
//		UART_Printf("׼����W5500�˿�\r\n");               //���������Ϣ
//		close(SOCK_TCPC);
//		TCP_Send_Flag = 0;
//		Connect_flag = 0;                                 //���ӱ�־=0
//		ret = socket(SOCK_TCPC,Sn_MR_TCP,m_netpara.local_Port,Sn_MR_ND);  //��W5500�Ķ˿ڣ����ڽ���TCP���ӣ�����TCP�˿�5050
//		if(ret != SOCK_TCPC)
//		{	                           //�����ʧ�ܣ�����if																 
//			UART_Printf("�˿ڴ򿪴���׼������\r\n");     //���������Ϣ
//			NVIC_SystemReset();	                       //����
//		}
//		else 
//			UART_Printf("�򿪶˿ڳɹ�\r\n");	       //���������Ϣ
//		break;                                            //����
//	  default:
//		  break;
//	}
//}

void MQTT_Package_msg(uint8 *payload, uint8 StartNum, uint16 Num)
{
	char data[6] = "";
	uint16 u16len = 0;
	
	memcpy(PubTopic, "/WSN-LW/",8);
	memcpy(PubTopic+8, Read_ID,8);
	memcpy(PubTopic+16, "/event/Data",sizeof("/event/Data"));
	UART_Printf("Pubscribing to %s\r\n", PubTopic);
	
	u16len = strlen("{\"SensorNum\":");
	memcpy(tempBuffer, "{\"SensorNum\":",u16len);
	sprintf(data, "%d", Num); 
	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += strlen(data);
	
	
	memcpy(tempBuffer+u16len, ",\"SensorStart\":",strlen(",\"SensorStart\":"));
	u16len += strlen(",\"SensorStart\":");
	memset(data,0,6);
	sprintf(data, "%d", StartNum); 
	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += strlen(data);
	
	memcpy(tempBuffer+u16len, ",\"SensorData\":\"",strlen(",\"SensorData\":\""));
	u16len += strlen(",\"SensorData\":\"");
	hex2strid(tempBuffer+u16len, payload, 10*Num);
//	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += 10*Num;
	memcpy(tempBuffer+u16len, "\"}",2);
	u16len += 2;
	memcpy(tempBuffer+u16len, 0,1);
	
	mqtt_msg.payload = tempBuffer;
	mqtt_msg.payloadlen = u16len;
//	MQTTPublish(&c, PubTopic, &mqtt_msg);
}

void MQTT_Package_Heartbeat(uint16 sn, uint8 resetFlag)
{
	char data[6] = "";
	uint16 u16len = 0;
	
	memcpy(PubTopic, "/WSN-LW/",8);
	memcpy(PubTopic+8, Read_ID,8);
	memcpy(PubTopic+16, "/event/Heartbeat",sizeof("/event/Heartbeat"));
	UART_Printf("Pubscribing to %s\r\n", PubTopic);
	
	u16len = strlen("{\"Heartbeat\":");
	memcpy(tempBuffer, "{\"Heartbeat\":",u16len);
	sprintf(data, "%d", sn); 
	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += strlen(data);
	
	memcpy(tempBuffer+u16len, "}",1);
	u16len += 1;
	memcpy(tempBuffer+u16len, 0,1);
}

// @brief messageArrived callback function
void messageArrived(MessageData* md)
{
//	unsigned char testbuffer[100];
//	MQTTMessage* message = md->message;
//    
//    UART_Printf("message received.\r\n");
//    memcpy(testbuffer,(char*)message->payload,(int)message->payloadlen);
//    *(testbuffer + (int)message->payloadlen + 1) = '\n';
//    UART_Printf("%s\r\n",testbuffer);
//	nrf_drv_wdt_feed();
	
	MQTTMessage* message = md->message;

	for (uint8_t i = 0; i < md->topicName->lenstring.len; i++)
		putchar(*(md->topicName->lenstring.data + i));

	UART_Printf(" %*s\r\n",
		(int32_t) message->payloadlen,
		(char*) message->payload);
}

void MQTT_Init(void)
{
	Network n;
    int rc = 0;
	NewNetwork(&n, SOCK_TCPC);
	UART_Printf("׼�����ӷ�����\r\n");                //���������Ϣ
	UART_Printf("MQTT Init\r\n");       	         //���������Ϣ
	
	ConnectNetwork(&n, m_netpara.server_ip, m_netpara.server_port);
	UART_Printf("IP��ַ: %d.%d.%d.%d,%d\r\n", m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3],m_netpara.server_port);		
	MQTTClientInit(&c,&n,1000,SendBuffer,3072,tempBuffer,2048);
	
	// ����MQTT������
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
//	data.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	data.clientID.cstring = "w5500-client";    //opts.clientid;
	data.username.cstring = ""; //opts.username;
	data.password.cstring = ""; //opts.password;
	
	// ���ûỰ����ʱ�� ��λΪ�� ��������ÿ��1.5*20���ʱ����ͻ��˷��͸���Ϣ�жϿͻ����Ƿ����ߣ������������û�������Ļ���
	data.keepAliveInterval = 60;
	// �����Ƿ����session,�����������Ϊfalse��ʾ�������ᱣ���ͻ��˵����Ӽ�¼����������Ϊtrue��ʾÿ�����ӵ������������µ��������
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
	UART_Printf("Connected %d\r\n", rc);

	// ��������
	memcpy(SubTopic, "/WSN-LW/",8);
	memcpy(SubTopic+8, Read_ID,8);
	memcpy(SubTopic+16, "/service/#",sizeof("/service/#"));// ���Ĳ�������
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
	UART_Printf("Subscribed %d\r\n", rc);
	
//	memcpy(SubTopic, "/WSN-LW/",8);
//	memcpy(SubTopic+8, Read_ID,8);
//	memcpy(SubTopic+16, "/service/Update",sizeof("/service/Update"));// ���Ĳ�������
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
//	UART_Printf("Subscribed %d\r\n", rc);
//	
//	memcpy(SubTopic+16, "/service/Read_data",sizeof("/service/Read_data"));// ���Ķ�ȡ����
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
//	UART_Printf("Subscribed %d\r\n", rc);
//
//	memcpy(SubTopic+16, "/service/Alarm",sizeof("/service/Alarm"));	// ���ı�����Ϣ
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
//	UART_Printf("Subscribed %d\r\n", rc);
	
	
//    MQTTMessage mqtt_msg;
    mqtt_msg.qos = QOS0;
    mqtt_msg.retained = 0;
    mqtt_msg.dup = 0;
//	memcpy(tempBuffer, "{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":3132213B2470E25101240102} ", 70);
    mqtt_msg.payload = SendBuffer;//"{\"Heartbeat\":8888,\"CSQ\":24,\"IMSI\":460045926209888}";
    mqtt_msg.payloadlen = strlen(SendBuffer);
	
}
void MQTT_Working(void)
{
//	char Topic[3][40]={"","",""};
	Network n;
//	MQTTClient c;
    int rc = 0;
//    unsigned char buf[1000];
	
	NewNetwork(&n, SOCK_TCPC);
	
	UART_Printf("׼�����ӷ�����\r\n");                //���������Ϣ
	UART_Printf("MQTT Work\r\n");       	         //���������Ϣ
	
//	m_netpara.server_ip[0] = 118;
//	m_netpara.server_ip[1] = 31;
//	m_netpara.server_ip[2] = 227;
//	m_netpara.server_ip[3] = 219;

//	m_netpara.server_ip[0] = 192;
//	m_netpara.server_ip[1] = 168;
//	m_netpara.server_ip[2] = 1;
//	m_netpara.server_ip[3] = 103;
	ConnectNetwork(&n, m_netpara.server_ip, m_netpara.server_port);
	UART_Printf("IP��ַ: %d.%d.%d.%d,%d\r\n", m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3],m_netpara.server_port);		
	MQTTClientInit(&c,&n,1000,SendBuffer,3072,tempBuffer,2048);
	
	// ����MQTT������
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
//	data.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	data.clientID.cstring = "w5500-client";    //opts.clientid;
	data.username.cstring = ""; //opts.username;
	data.password.cstring = ""; //opts.password;

	data.keepAliveInterval = 60;
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
	UART_Printf("Connected %d\r\n", rc);

	// ��������
	memcpy(SubTopic, "/WSN-LW/",8);
	memcpy(SubTopic+8, Read_ID,8);
	memcpy(SubTopic+16, "/service/+",sizeof("/service/+"));// ���Ĳ�������
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
	UART_Printf("Subscribed %d\r\n", rc);
	
////	memcpy(SubTopic0, "/WSN-LW/",8);
////	memcpy(SubTopic0+8, Read_ID,8);
////	memcpy(SubTopic0+16, "/service/Update",sizeof("/service/Update"));// ���Ĳ�������
////	
////	memcpy(SubTopic1, "/WSN-LW/",8);
////	memcpy(SubTopic1+8, Read_ID,8);
////	memcpy(SubTopic1+16, "/service/Read_data",sizeof("/service/Read_data"));// ���Ĳ�������
////	
////	memcpy(SubTopic2, "/WSN-LW/",8);
////	memcpy(SubTopic2+8, Read_ID,8);
////	memcpy(SubTopic2+16, "/service/Alarm",sizeof("/service/Alarm"));// ���Ĳ�������
//	
////	memcpy(Topic[0], SubTopic0, strlen(SubTopic0));
////	memcpy(Topic[1], SubTopic1, strlen(SubTopic1));
////	memcpy(Topic[2], SubTopic2, strlen(SubTopic2));
////	//{SubTopic0, SubTopic1, SubTopic2};
////	UART_Printf("Subscribing to %s\r\n", Topic);
////	rc = MQTTSubscribe(&c, Topic, QOS0, messageArrived);// /# ����һ��
////	UART_Printf("Subscribed %d\r\n", rc);
	
//	memcpy(SubTopic, "/WSN-LW/",8);
//	memcpy(SubTopic+8, Read_ID,8);
//	memcpy(SubTopic+16, "/service/Update",sizeof("/service/Update"));// ���Ĳ�������
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
//	UART_Printf("Subscribed %d\r\n", rc);
//	
//	memcpy(SubTopic+16, "/service/Read_data",sizeof("/service/Read_data"));// ���Ķ�ȡ����
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
//	UART_Printf("Subscribed %d\r\n", rc);
//
//	memcpy(SubTopic+16, "/service/Alarm",sizeof("/service/Alarm"));	// ���ı�����Ϣ
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# ����һ��
//	UART_Printf("Subscribed %d\r\n", rc);
	
	
//    MQTTMessage mqtt_msg;
    mqtt_msg.qos = QOS0;
    mqtt_msg.retained = 0;
    mqtt_msg.dup = 0;
//	memcpy(tempBuffer, "{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":3132213B2470E25101240102} ", 70);
    mqtt_msg.payload = SendBuffer;//"{\"Heartbeat\":8888,\"CSQ\":24,\"IMSI\":460045926209888}";
    mqtt_msg.payloadlen = 70;
	
	while(1)
    {
		nrf_drv_wdt_feed();
		if(MQTT_time == 0)
		{ // MQTT�����ύ����
			MQTT_Package_msg(SensorData[0],1,20);
			MQTT_time = 1000;
//			mqtt_msg.payload = tempBuffer;
//			mqtt_msg.payloadlen = u16len;
			rc = MQTTPublish(&c, PubTopic, &mqtt_msg);
			UART_Printf("Publish %d\r\n", rc);
//			UART_Printf("Publish \r\n");
//			MQTTPublish(&c, "/WSN-LW/20010333/event/Heartbeat", &mqtt_msg);
		}
//        delay_ms(1000);
		// �������Ⲣ�뾭���˱�����ϵ
    	MQTTYield(&c, 1000);
    }
}

void DHCP_Working(void)
{
		
	switch(DHCP_run())           //�ж�DHCPִ�����ĸ�����
	{
	  case DHCP_IP_ASSIGN:     //��״̬��ʾ��·���������������ip��
	  case DHCP_IP_CHANGED:    //��״̬��ʾ��·�����ı��˷�����������ip
		my_ip_assign();   //����IP������ȡ��������¼��������
		break;            //����
	  case DHCP_IP_LEASED:     //��״̬��ʾ��·����������Ŀ�����ip����ʽ�����ˣ���ʾ��������ͨ����
		if(DNS_flag==0)									//���������δ����������if
		{                                  
			W5500_DNS(ServerName);						//��������������
		}
		else
		{                                            //��֮������Ѿ���������IP������else��֧���ӷ�����
			if(Work_Flag == WORK_DHCP)
			{
				Work_Flag = WORK_TCP;
				DHCP_time = 0xFFFF;
//				TCP_time = 10000;//10S
				MQTT_time = 1000;//1S
			}
//			TCP_Working();
			MQTT_Working();
		}
		break;  //����							
		
	  case DHCP_FAILED:       //��״̬��ʾDHCP��ȡIPʧ��     									 
		my_dhcp_retry++;                        //ʧ�ܴ���+1
		if(my_dhcp_retry > MY_MAX_DHCP_RETRY)
		{  //���ʧ�ܴ�������������������if							
			UART_Printf("DHCPʧ�ܣ�����M2Mģʽ\r\n");//������ʾ��Ϣ
			my_dhcp_retry = 0;
			//				NVIC_SystemReset();		            //����
			Work_Flag = WORK_M2M_nDHCP;
		}
		break;    
	}
}

void M2M_Working(void)
{
	
	if(m2m_status != 0xFF)
		m2m_status = m2m_init();
	else if(m2m_status == 0xFF)
	{// ���������ӷ������ɹ�	
		Connect_flag = 1; 
//		if(m2m_TCP_Send() == M2M_CMD_SUCCESS)  	  // send to the cloud
//		{   // ���ݷ��ͳɹ�
//			
//		}
		
//		if(M2M_Send_Flag == 1)
		{
//			M2M_Send_Flag = 0;
			m2m_tcpconnect();
			if(m2m_TCP_Send() != M2M_CMD_SUCCESS)  	  // send to the cloud
			{   // ���ݷ��ͳɹ�
				m2m_status = 0;
			}
			else
			{
				LED_Flag = 1;
				SendTime = 0;	// ���ͳ�ʱ����
//				heartTicks = 0;		//����������ʱ
				m2m_closegprs();
			}
		}
	}
}




/*-------------------------------------------------*/
/*������������������		                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TCP_Net_Working(void)
{

	while(1)
	{
//		if((Phy_Link_Flag == PHY_LINK_ON) && (TCP_Flag == 1))
//		{
//			if( Work_Flag == 0)
//			{
//				W5500_init();
////				setkeepalive(SOCK_TCPC);
//				Work_Flag = 1;
//			}
//			else if( Work_Flag == 1)
//				DHCP_Working();
//		}
//		else if((Phy_Link_Flag == PHY_LINK_OFF) || (TCP_Flag == 0))
//		{
//			M2M_Working();
//		}
		
		if( Work_Flag == WORK_INIT)
		{
			W5500_init();
//			Work_Flag = WORK_DHCP;
		}
		else if( Work_Flag <= WORK_TCP)
		{
			DHCP_Working();
		}
		else if(Work_Flag >= WORK_M2M_nPHY)
		{
			M2M_Working();
		}
		
//		MQTT_Working();
		
		nrf_drv_wdt_feed();
	}
}

