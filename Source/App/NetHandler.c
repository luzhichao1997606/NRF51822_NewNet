/**
 * NetHander.c �����ʼ������
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#include "NetHandler.h"
#include "math.h"
#include "hal.h"    //������Ҫ��ͷ�ļ�
#include "app.h"
#include "mystring.h"
#include <string.h>
#include "cJSON.h"
#include "nrf24l01+.h"

#include "socket.h"//Just include one header for WIZCHIP
#include "dhcp.h"
#include "dns.h"
#include "MQTTClient.h"
#include "ble_flash.h"
/*************************protobuf-c*****************************/

#include "UserInformation.pb-c.h"


#define 	CardNum				10
#define		STORE_MAGIC		0x12345679
#define   	STORE_PAGE			127
#define		STATIC_PAGE			126

#define		Debug_TCP 			0
/*Ĭ������*/
/**MQTT**/
//Alarm ���Ľ���
uint8_t MQTT_Resv_Alarm = 0;
uint8_t *MQTT_Resv_AlarmData ;
//Read_Data ���Ľ���
uint8_t MQTT_Resv_Read_data = 	0;
//Updata ���Ľ���
uint8_t MQTT_Resv_Cycle 	  =	5;  // �ϱ����ڣ���λ���ӣ�1~255
uint8_t MQTT_Resv_AlarmTime   =	5; // ��������ʱ�䣬��λ���ӣ�0~255��0��������255�������� 
uint8_t MQTT_Resv_Channel     =	60; // �����ŵ������ε�NRF24L01��
uint8_t MQTT_Resv_SensorNum   =	120;// 1-240 �������豸����Ĳɼ�ģ������(��������)
uint8_t MQTT_Resv_SensorCycle =	10; // �������ϱ����ڣ���λ���ӣ�10~255�����10����(�����������) 

/**TCP**/
uint8_t		SIP_SaveData[4] = {192,168,3,128};	//��̬IP
uint8_t		GW_SaveData[4]	= {192,168,3,1};	//Ĭ������
uint8_t		YIP_SaveData[4] = {192,168,3,145};	//������IP
uint8_t		SN_SaveData[4]  = {255,255,255,0};	//��������

uint16_t	SPORT_SaveData = 8080;				//��̬�˿�
uint16_t	YPORT_SaveData = 1883;				//�������˿�

typedef struct
{
	//MQTT
	uint8_t		MQTT_Resv_Cycle; 			// �ϱ����ڣ���λ���ӣ�1~255
	uint8_t		MQTT_Resv_AlarmTime; 		// ��������ʱ�䣬��λ���ӣ�0~255��0��������255�������� 
	uint8_t		MQTT_Resv_Channel; 			// �����ŵ������ε�NRF24L01��
	uint8_t		MQTT_Resv_SensorNum ;		// 1-240 �������豸����Ĳɼ�ģ������(��������)
	uint8_t 	MQTT_Resv_SensorCycle ;		// �������ϱ����ڣ���λ���ӣ�10~255�����10����(�����������) 
	uint8_t 	POWER_ON_COUNT  ;
	//TCP
	uint8_t		SIP_SaveData[4];			//��̬IP
	uint8_t		GW_SaveData[4];				//Ĭ������
	uint8_t		YIP_SaveData[4];			//������IP
	uint8_t		SN_SaveData[4];				//��������

	uint16_t	SPORT_SaveData;				//��̬�˿�
	uint16_t	YPORT_SaveData;				//�������˿�
	
	
}para_cfg_t;

extern uint8_t		Read_ID[16] ;
extern uint8_t 		DataToSendBuffer[2400] ;

uint8 SensorNum		= 120;
uint8 SensorStart	= 1; 
UserInformation userInfo = USER_INFORMATION__INIT;
Heartbeat heartbeat = HEARTBEAT__INIT;

uint8_t NET_INIT_EVENT = 0;
/*******************����IP**************************/

//�˴������趨MQTT��������ӵ�IP�Լ��˿�

//2Gģ������ߵ�IP���Ǵ˴������趨��

//����Ǳ��ط���2G�����޷����ʣ���ʱ����������ߵĻ��Ϳ��������Ľ���ͨѶ

uint8_t IP_Data[4] 	= {192 , 168  , 3 , 145};
uint16_t Port_Data 	= 1883;
 
//CardList usercard = CARD_LIST__INIT;

Card cardInfo[CardNum];
Card* cardInfo_p[CardNum];
uint8 strIDbuf[CardNum][6][5]={0};	//����ID�ַ���
//	
/*************************protobuf-c*****************************/

uint8_t domain_ip[4]={0};
 	
uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}}; /* WIZCHIP SOCKET Buffer initialize */ 
uint8_t domain_name[]="emqx.iricbing.cn";//"mqtt.yun-ran.com";//"yeelink.net";/*����*/

unsigned char tempBuffer[10];
//MQTT ��������
unsigned char SendBuffer[1200];

unsigned char Buffer[2]= {0x29,0x55};

unsigned char W5500_NOPHY_TryGPRS_Flag = 0 ;
unsigned char W5500_DHCP_Flag = 0 ;

NRF24L01_Data_Set NRF_Data_Poll_1; 
 
//unsigned char ReciveBuffer[500];
typedef union Recive_package
{
    struct {
        uint8_t		is_process_data;  /* serial number array */
		uint8_t		topic[10];
        uint32_t	len;    /* typically, 0xff */
		uint8_t		databuf[500];
    }t;
    uint8_t buf[515];
}Recive_pkg_t;

Recive_pkg_t RecivePkg;

char Topic[50] = "/WSN-LW/";
char SubTopic[50] = "/WSN-LW/";

MQTTClient mqttclient;
MQTTMessage mqtt_msg;
Network network;
para_cfg_t MQTT_Save_Data ;

para_cfg_t TCP_Save_Data ;

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
	//��һ���ϵ磬flashû�д洢����(ȫ��0xff)
	if(		ppara->MQTT_Resv_Cycle 			== 0xff 
		&& 	ppara->MQTT_Resv_AlarmTime 		== 0xff 
		&& 	ppara->MQTT_Resv_Channel 		== 0xff 
		&& 	ppara->MQTT_Resv_SensorNum 		== 0xff 
		&& 	ppara->MQTT_Resv_SensorCycle 	== 0xff 
		&&  ppara->POWER_ON_COUNT			== 0xff 
		) 
	{
		//��ֵĬ����ֵ
		ppara->MQTT_Resv_Cycle 			= 	5;  // �ϱ����ڣ���λ���ӣ�1~255
		ppara->MQTT_Resv_AlarmTime 		=	10; // ��������ʱ�䣬��λ���ӣ�0~255��0��������255�������� 
		ppara->MQTT_Resv_Channel   		=	60; // �����ŵ������ε�NRF24L01��
		ppara->MQTT_Resv_SensorNum		=	120;// 1-240 �������豸����Ĳɼ�ģ������(��������)
		ppara->MQTT_Resv_SensorCycle 	=	10; // �������ϱ����ڣ���λ���ӣ�10~255�����10����(�����������) 

		
		memcpy(ppara->SIP_SaveData,SIP_SaveData,sizeof(SIP_SaveData));  //��̬IP��ֵ
		memcpy(ppara->GW_SaveData,GW_SaveData,sizeof(GW_SaveData)); 	//Ĭ�����ظ�ֵ
		memcpy(ppara->YIP_SaveData,YIP_SaveData,sizeof(YIP_SaveData));	//������IP��ֵ
		memcpy(ppara->SN_SaveData,SN_SaveData,sizeof(SN_SaveData));		//��������

		ppara->SPORT_SaveData	=	SPORT_SaveData	;	//��̬�˿ڸ�ֵ
		ppara->YPORT_SaveData	=	YPORT_SaveData	;	//�������˿ڸ�ֵ 
	}
	//���֮ǰ��������˵���洢�����ݣ����Խ����ݸ�ֵ��ȫ�ֲ�����
	else
	{
		MQTT_Resv_Cycle 		= 	ppara->MQTT_Resv_Cycle 	;	
		MQTT_Resv_AlarmTime 	= 	ppara->MQTT_Resv_AlarmTime 	;
		MQTT_Resv_Channel 		= 	ppara->MQTT_Resv_Channel   	;
		MQTT_Resv_SensorNum 	= 	ppara->MQTT_Resv_SensorNum	;
		MQTT_Resv_SensorCycle 	= 	ppara->MQTT_Resv_SensorCycle ;

		memcpy(SIP_SaveData,ppara->SIP_SaveData,sizeof(SIP_SaveData));  //��̬IP��ֵ
		memcpy(GW_SaveData,ppara->GW_SaveData,sizeof(GW_SaveData)); 	//Ĭ�����ظ�ֵ
		memcpy(YIP_SaveData,ppara->YIP_SaveData,sizeof(YIP_SaveData));	//������IP��ֵ
		memcpy(SN_SaveData,ppara->SN_SaveData,sizeof(SN_SaveData));		//��������

		SPORT_SaveData	=	ppara->SPORT_SaveData	;	//��̬�˿ڸ�ֵ
		YPORT_SaveData	=	ppara->YPORT_SaveData	;	//�������˿ڸ�ֵ 
	}
	
 
	return;
}
/* Private macro -------------------------------------------------------------*/
uint8_t gDATABUF[DATA_BUF_SIZE];//��ȡ���ݵĻ�������2048
typedef struct Info_Static_t
{ 
	uint8_t ip[4];   ///< Source IP Address 
	uint8_t gw[4];   ///< Gateway IP Address 
	uint8_t yip[4];
	uint8_t sn[4];
	
	uint16_t yport;
	uint16_t sport;
	
	dhcp_mode dhcp;  ///< 1 - Static, 2 - DHCP 
}Info_Static;
//��ֵĬ������(�м�����ֻ������������ʵ������)
Info_Static  Info_Static_Get= {	.ip={192, 168, 3, 127},
								.gw={192, 168, 3, 1},
								.yip={192, 168,3, 145},
								.sn={255, 255,255, 0},
								.yport = 1883,
								.sport = 8080,
								.dhcp = NETINFO_STATIC 
};

/*Ĭ������IP��ַ����*/
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},//MAC��ַ
                            .ip = {192, 168, 3, 127},                  //IP��ַ
                            .sn = {255,255,255,0},                     //��������
                            .gw = {192, 168, 3, 1},                    //Ĭ������
                            .dns = {114,114,114,114},                   //DNS������
                            .dhcp = NETINFO_DHCP  						// NETINFO_STATIC
};

//����һ��TCP��socket����(�˴��޸ĵ���TCP��������IP�Ͷ˿ڣ�ǰ��λ�澲̬IP�Ĳ�ͬ����ͬ�����һλ������Ҫ�Լ��޸�)
TCP_Network_Info TCP_network = {  	.My_TCPSocket_Num 	= 3,
									.My_TCP_Connect_IP 	= {192,168,3,140},
									.My_TCP_Connect_Port= 8080

};
//����Ĭ������IP����
void Read_Flash_TCP_Data(void)
{
	para_read(STATIC_PAGE,&TCP_Save_Data);

	for (uint8_t i = 0; i < 4; i++)
	{
		/* code */
		gWIZNETINFO.ip[i] = TCP_Save_Data.SIP_SaveData[i];
		gWIZNETINFO.sn[i] = TCP_Save_Data.SN_SaveData[i];
		gWIZNETINFO.gw[i] = TCP_Save_Data.GW_SaveData[i];
		IP_Data[i]		  = TCP_Save_Data.YIP_SaveData[i];

		Port_Data		  = TCP_Save_Data.YPORT_SaveData ; 
		#if Debug_TCP
			TCP_network.My_TCP_Connect_Port	= 8080;
		#else
		
			TCP_network.My_TCP_Connect_Port	= TCP_Save_Data.SPORT_SaveData;
		#endif
		UART_Printf("gWIZNETINFO.ip[%d] is %d \r\n",i,gWIZNETINFO.ip[i]);
		UART_Printf("gWIZNETINFO.GW[%d] is %d \r\n",i,gWIZNETINFO.gw[i]);
	}

	//�ڶ�ȡ��ʱ���TCP��Socket����Ҳ����Ϊ���õ�����
	for (uint8_t i = 0; i < 3; i++)
	{
		if ( TCP_Save_Data.SIP_SaveData[i] != 0xff)
		{
			TCP_network.My_TCP_Connect_IP[i] = TCP_Save_Data.SIP_SaveData[i];
		} 
	}
	//store_clear(STORE_PAGE); //������
	//store_clear(STATIC_PAGE);//������
	
}
//��̬�����ݵĻ�ȡ�Լ������Ĵ���(flash�洢)
//  IP GW DHCP 
void Static_IP_Get(Info_Static Info)
{ 
	//IP��ַ��ת�� + Ĭ������
	for (uint8_t j = 0; j < 4; j++)
	{
		gWIZNETINFO.ip[j] = Info.ip[j];
		gWIZNETINFO.gw[j] = Info.gw[j]; 
	}
	gWIZNETINFO.dhcp = Info.dhcp ; 
	store_clear(STATIC_PAGE);
	//flash�洢
	memcpy(TCP_Save_Data.SIP_SaveData,Info.ip,sizeof(Info.ip));
	memcpy(TCP_Save_Data.YIP_SaveData,Info.yip,sizeof(Info.yip));
	memcpy(TCP_Save_Data.SN_SaveData,Info.sn,sizeof(Info.sn));
	memcpy(TCP_Save_Data.GW_SaveData,Info.gw,sizeof(Info.gw));

	TCP_Save_Data.SPORT_SaveData = Info.sport;
	TCP_Save_Data.YPORT_SaveData = Info.yport;  
	//
	UART_Printf("д��flash���ݣ� \r\n");
	//for (uint8_t i = 0; i < 4; i++)
	//{
	//	UART_Printf("TCP_Save_Data.SIP_SaveData[%d] is %d \r\n",i,TCP_Save_Data.SIP_SaveData[i]);
	//	UART_Printf("TCP_Save_Data.YIP_SaveData[%d] is %d \r\n",i,TCP_Save_Data.YIP_SaveData[i]);
	//	
	//}
	
	para_store(STATIC_PAGE,TCP_Save_Data); 		
} 
//����һ��TCP��soket����(״̬��)
uint8_t Connect_flag = 0;
void TCP_sTATE_loop(TCP_Network_Info* n)
{ 
	int ret,tcp_state;
 
	tcp_state=getSn_SR(n->My_TCPSocket_Num);  

	switch(tcp_state)
	{ //switch��䣬�жϵ�ǰTCP���ӵ�״̬   										
	case SOCK_INIT:          
		if(Connect_flag==0)
		{                                //�����û�����ӷ�����������if
			UART_Printf("׼�����ӷ�����\r\n");                //���������Ϣ 

			#if Debug_TCP
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , 8080 ); 
			#else
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , n->My_TCP_Connect_Port ); 
			#endif
			
			UART_Printf("���ӷ����������룺%d\r\n",ret);      //���������Ϣ 
			if (ret != SOCK_OK)
			{
				delay_ms(1000);
			} 
		}
		//����
		break;	                                       
		
	case SOCK_ESTABLISHED:   
		if((Connect_flag==0)&&(getSn_IR(n->My_TCPSocket_Num)==Sn_IR_CON))
		{  //�ж������Ƿ���													
			UART_Printf("�����ѽ���\r\n");                          //���������Ϣ 
			Connect_flag = 1;                                     //���ӱ�־=1
			setSn_IR(SOCK_TCPS, Sn_IR_CON);							/*��������жϱ�־λ*/ 
		}
		if(getSn_IR(n->My_TCPSocket_Num) & Sn_IR_CON)
		{
			setSn_IR(SOCK_TCPS, Sn_IR_CON);							/*��������жϱ�־λ*/ 
		}  
		break; //����
		
	case SOCK_CLOSE_WAIT:      
		UART_Printf("�ȴ��ر�����\r\n");                   		//���������Ϣ
		if((ret=disconnect(n->My_TCPSocket_Num)) != SOCK_OK)	//�˿ڹر�
		{
			UART_Printf("���ӹر�ʧ�ܣ�׼������\r\n");				//���������Ϣ
			NVIC_SystemReset();										//����
		}
		UART_Printf("���ӹرճɹ�\r\n");                    //���������Ϣ
		Connect_flag = 0;                                 //���ӱ�־=0
		close(n->My_TCPSocket_Num);
		break;
		
	case SOCK_CLOSED:          
		UART_Printf("׼����W5500�˿�\r\n");               //���������Ϣ
		Connect_flag = 0;                                 //���ӱ�־=0 
		#if Debug_TCP
			ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,8080,Sn_MR_ND);  //��W5500�Ķ˿ڣ����ڽ���TCP���ӣ�����TCP�˿�5050 
		#else
			ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,n->My_TCP_Connect_Port,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
		#endif
		//ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,8080,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
		
		if(ret != n->My_TCPSocket_Num)
		{	                           //�����ʧ�ܣ�����if																 
			UART_Printf("�˿ڴ򿪴���׼������\r\n");     //���������Ϣ
			//NVIC_SystemReset();	                       //����
		}
		else UART_Printf("�򿪶˿ڳɹ�\r\n");	       //���������Ϣ
		break;                                            //����
	}  
}
//IP �ַ��� ת ����
void IP_Str2Int(char * ip,char *IP_Int)
{ 
	char Temp,Count = 0;  
	for (uint8_t i = 0; i < 16; i++)
	{
		if (ip[i] == '.' || ip[i] > '9' || ip[i] < '0' )
		{
			Temp = i;
			for (uint8_t j = 1; j < 4; j++)
			{	
				if (	ip[Temp - j] == '.'  ){break;}  
				if (ip[Temp - j]!='.' && (Temp - j) >= 0)
				{	
					IP_Int[Count] += (ip[Temp-j] - '0')  *  pow(10,(j-1)); 
					//UART_Printf("IP_Int[%d] is %d \r\n" ,Count,IP_Int[Count]);//����
				} 
			}  
			Count ++;
		}
		if (Count >= 4)
		{break;} 
	}  

}

// �˿ں� ת ����

uint16_t COM_Str2Int(char * COM)
{  
	
	uint16_t Result_TCP = 0 ;
	uint8_t Set = 0;
	//�ҵ�0x0d
	for (uint8_t j = 0; j < strlen(COM); j++)
	{
		if(COM[j] == 0x0d)
		{
			Set = j;
			break;
		}
	} 
	for (int i = Set; i >= 0; i--)
	{	
		if ( COM[i] <= '9' && COM[i] >= '0' )
		{  
			if ( COM[i] == 0x0d  ){break;}  

			if ( COM[i]!= 0x0d )
			{	
				Result_TCP += (COM[i] - '0')  *  pow(10,(Set-i-1)); 
			} 
		}
	}
	return Result_TCP;    
}
//TCP���ݵĴ���	
uint8_t SIP_Data_Str[20] 	= {0};
uint8_t GW_Data_Str[20] 	= {0};
uint8_t YIP_Data_Str[20] 	= {0};
uint8_t YPORT_Data_Str[20] 	= {0}; 
uint8_t SN_Data_Str[20] 	= {0}; 
uint8_t SPORT_Data_Str[10] 	= {0};  

void Data_Poll_Buffer(char * Buffer)
{
	uint8_t j;
	char Temp_Data_IP;
	char Data[4] = {0};
	for (uint8_t i = 0; i < strlen(Buffer); i++)
	{
		//SIP
		//��̬IP
		if (Buffer[i] == 'S'&& Buffer[i+1] == 'I' && Buffer[i+2] == 'P')//����ð��
		{
			UART_Printf("���뾲̬IP�洢���� \r\n");
			for ( j = 0; j < sizeof(SIP_Data_Str); j++)
			{
				if (Buffer[(i+4)+j] == 'G')	{	break; 	}  
				SIP_Data_Str[j] = Buffer[(i+4)+j]; 
			}
		}

		//GW
		//Ĭ������
		if (Buffer[i] == 'G'&& Buffer[i+1] == 'W' )//����ð��
		{
			UART_Printf("����Ĭ�����ش洢����\r\n"); 
			for (  j = 0; j < sizeof(GW_Data_Str); j++)
			{
				if (Buffer[(i+3)+j] == 'Y' && Buffer[(i+4)+j] == 'I' )	{	break; 	}  
				GW_Data_Str[j] = Buffer[(i+3)+j];
			} 
		}

		//YIP
		//������IP
		if (Buffer[i] == 'Y' && Buffer[i+1] == 'I' && Buffer[i+2] == 'P')//����ð��
		{	
			UART_Printf("���������IP�洢����\r\n"); 
			for (  j = 0; j < sizeof(YIP_Data_Str); j++)
			{
				if (Buffer[(i+4)+j] == 'Y' && Buffer[(i+5)+j] == 'P' )	{	break; 	}  
				YIP_Data_Str[j] = Buffer[(i+4)+j]; 
			} 
		}

		//YPORT
		//�������˿� 
		if (Buffer[i] == 'Y' && Buffer[i+1] == 'P' && Buffer[i+2] == 'O' 
							&& Buffer[i+3] == 'R' && Buffer[i+4] == 'T' )//����ð��
		{
			UART_Printf("����������˿ڴ洢����\r\n");
			
			for (  j = 0; j < sizeof(YPORT_Data_Str); j++)
			{	
				if (Buffer[(i+6)+j] == 'S' && Buffer[(i+7)+j] == 'N' )	{	break; 	}  
				YPORT_Data_Str[j] = Buffer[(i+6)+j]; 
			}
		}

		//SN
		//��������
		if ( Buffer[i] == 'S' && Buffer[i+1] == 'N' )//����ð��
		{
			UART_Printf("������������洢����\r\n");
			
			for (  j = 0; j < sizeof(SN_Data_Str); j++)
			{	
				if (Buffer[(i+3)+j] == 'S' && Buffer[(i+4)+j] == 'P' )	{	break; 	}  
				SN_Data_Str[j] = Buffer[(i+3)+j]; 
			} 
		}	

		//SPORT
		//��̬�˿�
		if ( Buffer[i] == 'S' && Buffer[i+1] == 'P'  
			&& Buffer[i+2] == 'O' && Buffer[i+3] == 'R' && Buffer[i+4] == 'T')//����ð��
		{
			UART_Printf("���뾲̬�˿ڴ洢����\r\n");
			
			for (  j = 0; j < sizeof(SPORT_Data_Str); j++)
			{	
				if (Buffer[(i+6)+j] == 'E' && Buffer[(i+7)+j] == 'N' )	{	break; 	}  
				SPORT_Data_Str[j] = Buffer[(i+6)+j]; 
			} 
		}		

	}
	
/*******************************IP*****************************************/
	//����̬IP���ݸ�ֵ
	memset(Data,0,sizeof(Data));
	UART_Printf("��̬IP������ %s  \r\n",SIP_Data_Str );  
	IP_Str2Int(SIP_Data_Str,Data); 
	UART_Printf("SIP_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("SIP_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("SIP_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("SIP_Data_StrData [3] %d \r\n",Data[3]); 
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.ip[i] = Data[i]; }  

	//��Ĭ���������ݸ�ֵ
	memset(Data,0,sizeof(Data));
	UART_Printf("Ĭ������������ %s \r\n",GW_Data_Str); 
	IP_Str2Int(GW_Data_Str,Data); 
	UART_Printf("GW_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("GW_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("GW_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("GW_Data_StrData [3] %d \r\n",Data[3]); 
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.gw[i] = Data[i]; }

	//��������IP���ݸ�ֵ
	memset(Data,0,sizeof(Data));
	UART_Printf("������IP������ %s \r\n",YIP_Data_Str); 
	IP_Str2Int(YIP_Data_Str,Data); 
	UART_Printf("YIP_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("YIP_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("YIP_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("YIP_Data_StrData [3] %d \r\n",Data[3]);   
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.yip[i] = Data[i]; }

	//�������������ݸ�ֵ
	memset(Data,0,sizeof(Data));
	UART_Printf("�������������� %s \r\n",SN_Data_Str); 
	IP_Str2Int(SN_Data_Str,Data); 
	UART_Printf("SN_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("SN_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("SN_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("SN_Data_StrData [3] %d \r\n",Data[3]); 
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.sn[i] = Data[i]; }

/*******************************�˿�*****************************************/

	//UART_Printf("YPORT_Data_StrData   is %d\r\n",Info_Static_Get.yport);
	//UART_Printf("YPORT_Data_StrData   is %d\r\n",Info_Static_Get.sport);
 
	Info_Static_Get.yport =  COM_Str2Int(YPORT_Data_Str) ; 
	UART_Printf("�������˿������� %d \r\n",Info_Static_Get.yport);
 
	Info_Static_Get.sport =  COM_Str2Int(SPORT_Data_Str) ; 
	UART_Printf("Ŀ�꾲̬�˿������� %d \r\n",Info_Static_Get.sport);

	//����Static_IP_Get���������ݴ��ڽṹ����
	//����flash�Ĵ洢�������д洢
	Static_IP_Get(Info_Static_Get);
 
	//TCP�����������

	//��ʱ30s
	// delay_ms(30000);
	// NVIC_SystemReset();	                       //����
}
//����һ��TCP��soket����
void Creat_TCP_Client(TCP_Network_Info* n )
{
	int ret,tcp_state; 
	char buf[] = "START";
	char buffer[100] = {0}; 
	char Resv_Data_Over = 0; 
	char Resv_Data_Port_Close = 0; 
	 
	//����TCP���ӣ� 
	__disable_interrupt(); 
	#if	Debug_TCP
		ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,8080,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
	#else
		ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,n->My_TCP_Connect_Port,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
	#endif
	
	if (ret ==  n->My_TCPSocket_Num)
	{
		tcp_state=getSn_SR(n->My_TCPSocket_Num);   

		#if Debug_TCP
			if (NET_INIT_EVENT)
			{
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , 8080 ); 
				NET_INIT_EVENT = 0;
			} 
		#else
			if (NET_INIT_EVENT)
			{
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , n->My_TCP_Connect_Port ); 
				NET_INIT_EVENT = 0;
			} 
		#endif
		
		//û�����Ӳ������ӷ�������busy����ok ���� ����DHCP��
		if((Connect_flag == 0)  && (ret == 0 || ret == 1) && ( !W5500_DHCP_Flag )) 
		{ 	
			//UART_Printf(" ret is %d \r\n",ret);
			delay_ms(500);
			tcp_state=getSn_SR(n->My_TCPSocket_Num);  
			if ((getSn_IR(n->My_TCPSocket_Num)==Sn_IR_CON) && tcp_state == SOCK_ESTABLISHED)
			{
				UART_Printf("���ӳɹ��ˣ�������\r\n");			
				while(!Connect_flag)
				{
					TCP_sTATE_loop(n);
				}
			}
			else
			{
				//�ȴ��´����� 	
				UART_Printf("�ȴ��ر�����\r\n");							//���������Ϣ
				if((ret=disconnect(n->My_TCPSocket_Num)) != SOCK_OK)		//�˿ڹر�
				{
					UART_Printf("���ӹرճɹ�\r\n");                    		//���������Ϣ
				} 
				Connect_flag = 0;                                 			//���ӱ�־=0
				close(n->My_TCPSocket_Num); 
			} 
		} 
	} 
	if (Connect_flag)
	{
		UART_Printf("����TCP�������ɹ� \r\n"); 
		
		//�˴����ж�flash�Ƿ�洢����

		//���н�������
		while (!Resv_Data_Port_Close)
		{ 
			memset(buffer,0,sizeof(buffer)); 
			//���з���
			ret = send(n->My_TCPSocket_Num,buf,strlen(buf)); 
			while(! Resv_Data_Over)
			{
				ret = getSn_RX_RSR(n->My_TCPSocket_Num);	     /*����lenΪ�ѽ������ݵĳ���*/				                    		
				//���ret����0����ʾ�������� 
				recv(n->My_TCPSocket_Num,buffer,ret);	 
				UART_Printf("TCP Data is %s \r\n",buffer)	;	/*��������Client������*/  
				delay_ms(500);
				for (uint8_t i = 0; i < strlen(buffer); i++)
				{ 		
					if (buffer[i] == 'E' && buffer[i+1] == 'N' && buffer[i+2] == 'D' )
					{
						UART_Printf(" \r\n �������ݳɹ����� \r\n !! ");
						//��־λ��λ
						Resv_Data_Over = 1;
					}
				}  
			} 

			if (buffer[0] == 'W')
			{
				//���պ������н���֮����ȡ����
				Data_Poll_Buffer(buffer); 
				Resv_Data_Over = 0;	
			}
			else if(buffer[0] == 'R')
			{
				//���ͺ��� 
				Resv_Data_Over = 0;
			}
			//5�����������
			else if(buffer[0] == 'C')
			{	
				Resv_Data_Port_Close = 1;
				delay_ms(5000);		
				NVIC_SystemReset();	//����
			}
		}
		//�ȴ��´����� 	
		UART_Printf("�ȴ��ر�����\r\n");							//���������Ϣ
		if((ret=disconnect(n->My_TCPSocket_Num)) != SOCK_OK)		//�˿ڹر�
		{
			UART_Printf("���ӹرճɹ�\r\n");                    		//���������Ϣ
		} 
		Connect_flag = 0;                                 			//���ӱ�־=0
		close(n->My_TCPSocket_Num); 
	}

	__enable_interrupt();
}
/* Private functions ---------------------------------------------------------*/
static void RegisterSPItoW5500(void);/*��SPI�ӿں���ע�ᵽW5500��socket����*/
static void InitW5500SocketBuf(void);/*��ʼ��W5500����оƬ*/
static void PhyLinkStatusCheck(void);/* PHY��·״̬���*/
static void DhcpInitHandler(void);   /*DHCP��ʼ��*/
static void my_ip_assign(void);      /*��̬����IP*/          
static void my_ip_conflict(void);    /*IP��ַ��ͻ�ļ򵥻ص�����*/
void DNS_Analyse(void);/*DNS����*/



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


/* IP��ַ��ͻ�ļ򵥻ص����� */
static void my_ip_conflict(void)
{
    UART_Printf("CONFLICT IP from DHCP\r\n");
    
    //halt or reset or any...
    while(1); // this example is halt.
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

/*******************************************************
 * @ brief Call back for ip assing & ip update from DHCP
 * ��̬����IP����Ϣ
 *******************************************************/
static void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);  //IP��ַ
   getGWfromDHCP(gWIZNETINFO.gw);  //Ĭ������
   getSNfromDHCP(gWIZNETINFO.sn);  //��������
   getDNSfromDHCP(gWIZNETINFO.dns);//DNS������
   gWIZNETINFO.dhcp = NETINFO_DHCP;
   
   /* Network initialization */
   network_init();//Ӧ��DHCP����������ַ���������ʼ��
   UART_Printf("DHCP LEASED TIME : %d Sec.\r\n", getDHCPLeasetime());//��ӡ��õ�DHCP�����ַʱ��

   DNS_Analyse();//��������
}


/******************************************************************************
 * @brief  Network Init
 * Intialize the network information to be used in WIZCHIP
 *****************************************************************************/
void network_init(void)
{
    uint8_t tmpstr[6] = {0};
    wiz_NetInfo netinfo;

    /*����gWIZNETINFO�ṹ������������Ϣ*/
    //DNS������gWIZNETINFO�����б���������ȷ����������޷���ȷ����������
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

    /*��ȡ���ú��������Ϣ����netinfo�ṹ��*/
    ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

    /* ���ڴ�ӡ������Ϣ */
    ctlwizchip(CW_GET_ID,(void*)tmpstr);
    if(netinfo.dhcp == NETINFO_DHCP) 
	{
		UART_Printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
		W5500_DHCP_Flag = 1;
	}
    else 
	{
		UART_Printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);
		W5500_DHCP_Flag = 0;
	}
    UART_Printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
                    netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
    UART_Printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
    UART_Printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
    UART_Printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
    UART_Printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
    UART_Printf("===========================\r\n");
	 
}

/*��SPI�ӿں���ע�ᵽW5500��socket����*/
static void RegisterSPItoW5500(void)
{
  /* 1.ע���ٽ������� */
  reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);
  
  /* 2.ע��SPIƬѡ�źź��� */

  reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
      
  /* 3.ע���д���� */
  reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);
}

/*��ʼ��W5500����оƬ:ֱ�ӵ��ùٷ��ṩ�ĳ�ʼ����*/
static void InitW5500SocketBuf(void)
{
  /* WIZCHIP SOCKET Buffer initialize */
  if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){
      UART_Printf("WIZCHIP Initialized fail.\r\n");
//      while(1);
	  NVIC_SystemReset();                                //����
  }
}

/* PHY��·״̬���*/
static void PhyLinkStatusCheck(void)
{
	uint8_t tmp;
	uint8_t Count_Over = 0;
	__disable_interrupt();

	do{
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){
			UART_Printf("Unknown PHY Link stauts.\r\n");
		}
		if(tmp == PHY_LINK_OFF){
			UART_Printf("PHY Link OFF!!!\r\n");//�����⵽������û���ӣ���ʾ��������
			delay_ms(2000);              //��ʱ2s
			Count_Over++;
			//30s û������PHY��˵����ת����GPRS
			if (Count_Over >= 10)
			{
				W5500_NOPHY_TryGPRS_Flag = 1;
				UART_Printf("\r\n NET Change to GPRS \r\n"); 
				para_read(STORE_PAGE,&MQTT_Save_Data);
				UART_Printf("\r\n MQTT ������ȡ�� \r\n"); 
				break;
			} 
		}
	}while(tmp == PHY_LINK_OFF);

	__enable_interrupt();
	if (!W5500_NOPHY_TryGPRS_Flag)
	{
		UART_Printf(" PHY Link Success.\r\n");
	} 
}


/*DHCP��ʼ��*/
static void DhcpInitHandler(void)
{
   /* >> DHCP Client 				  */
  /************************************************/
  
  //must be set the default mac before DHCP started.
  setSHAR(gWIZNETINFO.mac);
		
  DHCP_init(SOCK_DHCP, gDATABUF);//gDATABUF��ȡ���ݵĻ�����
  
  // if you want defiffent action instead defalut ip assign,update, conflict,
  // if cbfunc == 0, act as default.
  //����:�����IP�����µ�IP����ͻ��IP
  reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
}

/*��̬����IP��ַ*/
uint8_t DHCP_proc(void)
{
  static uint8_t my_dhcp_retry = 0;
  uint8_t dhcp_res = DHCP_run();
  switch (dhcp_res)
  {
  case DHCP_IP_ASSIGN:
  case DHCP_IP_CHANGED:
  case DHCP_IP_LEASED:
    getIPfromDHCP(gWIZNETINFO.ip);
    getGWfromDHCP(gWIZNETINFO.gw);
    getSNfromDHCP(gWIZNETINFO.sn);
    getDNSfromDHCP(gWIZNETINFO.dns);
    gWIZNETINFO.dhcp = NETINFO_DHCP;
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
    // For debug
    // printf("\r\n>> DHCP IP Leased Time : %ld Sec\r\n", getDHCPLeasetime());
    break;
  case DHCP_FAILED:
//    UART_Printf(">> DHCP Failed\r\n");
//	gWIZNETINFO.dhcp = NETINFO_STATIC;
	my_dhcp_retry++;
	UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
	if(my_dhcp_retry > MY_MAX_DHCP_RETRY)//��̬IP����ʧ�ܴ���ʮ�Σ���ʹ�þ�̬��IP
	{
//		UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
		gWIZNETINFO.dhcp = NETINFO_STATIC;
		my_dhcp_retry = 0;
		DHCP_stop();// if restart, recall DHCP_init()
		network_init();// apply the default static network and print out netinfo to serial
		//DNS_Analyse();//��������	
	}
    
    break;
  }
  return dhcp_res;
}


/*��̬����IP��ַ*/
void DhcpRunInLoop(void)
{
  static uint8_t my_dhcp_retry = 0;
//  char value[16]={0};
  int32_t t=0;
  
  
  switch(DHCP_run())
  {
    case DHCP_IP_ASSIGN://DHCP IP����
    case DHCP_IP_CHANGED://DHCP IP����
        /* If this block empty, act with default_ip_assign & default_ip_update */
        // This example calls my_ip_assign in the two case.
        break;
    
    case DHCP_IP_LEASED://TO DO YOUR NETWORK APPs.
        //����õ�DHCP�����ַʱ����ִ������App
        //yeelink_get("19610","34409",value);//���騰aD����aDT??3����??oyeelink������?��?ID--LED1
////        yeelink_get("19657","34484",value);//���騰aD����aDT??3����??oyeelink������?��?ID--LED2
////        UART_Printf("%s\n\r",value);	//char value[16]={0};
////        UART_Printf("\n\r");
	  	MQTT_Init();
//        for(t=0;t<11;t++){
//          delay_ms(1000);
//        } 
        break;
    
    case DHCP_FAILED://��̬IP����ʧ��
        /* ===== Example pseudo code =====  */
        // The below code can be replaced your code or omitted.
        // if omitted, retry to process DHCP
        my_dhcp_retry++;
        if(my_dhcp_retry > MY_MAX_DHCP_RETRY)//��̬IP����ʧ�ܴ���ʮ�Σ���ʹ�þ�̬��IP
        {
            UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
            my_dhcp_retry = 0;
            DHCP_stop();// if restart, recall DHCP_init()
            network_init();// apply the default static network and print out netinfo to serial
            DNS_Analyse();//��������
        }
        break;
        
    default:
        break;
  }  
}

/*DNS����*/
void DNS_Analyse(void)
{
    int32_t ret = 0;
  
    /*��ʼ��DNS�������򣬲�ͨ��������غ�����ȡ��machtalk����������ʵIP��ַ*/
    /* DNS client initialization */
    DNS_init(SOCK_DNS, gDATABUF);
//    Timer_Start();/*����Timer3*/
    /* DNS procssing */
    if ((ret = DNS_run(gWIZNETINFO.dns, domain_name, domain_ip)) > 0){ // try to 1st DNS
        UART_Printf("> 1st DNS Reponsed\r\n");
    }else if(ret == -1){
        UART_Printf("> MAX_DOMAIN_NAME is too small. Should be redefined it.\r\n");
//        Timer_Stop();/*�ر�Timer3*/
        while(1);
    }else{
        UART_Printf("> DNS Failed\r\n");
//        Timer_Stop();/*�ر�Timer3*/
        while(1);
    }

    //DNS�����ɹ���IP��ַ�洢��domain_ip�����У������socket��̻��õ��������ֵ��
    if(ret > 0){
        UART_Printf("> Translated %s to %d.%d.%d.%d\r\n",domain_name,domain_ip[0],domain_ip[1],domain_ip[2],domain_ip[3]);
    }
//    Timer_Stop();
}


/*����W5500����*/
int NetworkInitHandler(void)
{
	int rc = 0;
	//��������MAC��ַ
	gWIZNETINFO.mac[0] = (NRF_FICR->DEVICEID[1] >> 8)& 0XFF; 
	gWIZNETINFO.mac[1] = (NRF_FICR->DEVICEID[1] 	)& 0XFF;
	
	gWIZNETINFO.mac[2] = (NRF_FICR->DEVICEID[0] >>24)& 0XFF;
	gWIZNETINFO.mac[3] = (NRF_FICR->DEVICEID[0] >>16)& 0XFF;
	gWIZNETINFO.mac[4] = (NRF_FICR->DEVICEID[0] >> 8)& 0XFF;
	gWIZNETINFO.mac[5] = (NRF_FICR->DEVICEID[0]		)& 0XFF;

	if (gWIZNETINFO.mac[0] % 2 != 0)
	{
		gWIZNETINFO.mac[0] += 1;
	} 
	//��ȡflash�����ݣ�TCP��
	Read_Flash_TCP_Data();
	W5500_Reset();
    RegisterSPItoW5500();/*��SPI�ӿں���ע�ᵽW5500��socket����*/
    InitW5500SocketBuf();/*��ʼ��W5500����оƬ:ֱ�ӵ��ùٷ��ṩ�ĳ�ʼ����*/
    PhyLinkStatusCheck();/* PHY��·״̬���*/
	if (!W5500_NOPHY_TryGPRS_Flag)
	{
		DhcpInitHandler();   /*DHCP��ʼ��*/
	}
	else
	{
		//��ʼ��GPRS
		UART_Printf("GPRS INIT \r\n");
	}
//	rc = PhyLinkStatusCheck();/* PHY��·״̬���*/
	NET_INIT_EVENT = 1;
	return rc;
}
 

/**
 * @name: Unpack_json_MQTT_ResvData
 * @test: test font
 * @msg:  �⹹JSON���ݣ��ѹؼ���Ϣ��ȡ����	
 * @param {type} 
 * @return: 
 */
int Unpack_json_MQTT_ResvData(uint8 * ResvData)
{
	cJSON *json;
	cJSON *json_value, *json_value_1 , *json_value_2 , *json_value_3 , *json_value_4;
	 
	uint8 TempBuffer_Json[5] ;
	int Str_len ;
    json = cJSON_Parse(ResvData);
    if(NULL == json)
    {
        UART_Printf("Error before: %s\n", cJSON_GetErrorPtr());
        return -1;
    }
	//��ȡ���ݳ���
	Str_len = strlen(ResvData);
	UART_Printf("Str_len of ResvData : %d\r\n" , Str_len);
	//��ȡǰ��λ������
	strncpy(TempBuffer_Json,ResvData,5); 

	//Alarm ���Ľ���
	if( strchr(TempBuffer_Json,'A') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Alarm");

		if(json_value->type == cJSON_Number)
		{
			MQTT_Resv_Alarm = json_value->valueint ;
			UART_Printf("Alarm: %d\n", MQTT_Resv_Alarm);
		}
		
		json_value_1 = cJSON_GetObjectItem(json, "AlarmData");

		if(json_value_1->type == cJSON_String)
		{
			MQTT_Resv_AlarmData = json_value_1->valuestring ;
			UART_Printf("AlarmData ��%s\n", MQTT_Resv_AlarmData);//json_value_1->valuestring);

		}  
	}

	//Read_Data ���Ľ���(OK)
	if( strchr(TempBuffer_Json,'R') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Read_data");

		if(json_value->type == cJSON_Number)
		{
			MQTT_Resv_Read_data = json_value->valueint;
			UART_Printf("Read_data: %d\n", MQTT_Resv_Read_data); 
		}  
		 
	}	 
	//Updata ���Ľ���
	if( strchr(TempBuffer_Json,'C') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Cycle"); 
		store_clear(STORE_PAGE);
		if(json_value->type == cJSON_Number)
		{
			 // �ϱ����ڣ���λ���ӣ�1~255(OK)
			MQTT_Resv_Cycle = json_value->valueint;
			MQTT_Save_Data.MQTT_Resv_Cycle = MQTT_Resv_Cycle; 
			UART_Printf("Cycle: %d\n", MQTT_Resv_Cycle);
		} 

		json_value_1 = cJSON_GetObjectItem(json, "AlarmTime"); 
		if(json_value_1->type == cJSON_Number)
		{	
			// ��������ʱ�䣬��λ���ӣ�0~255��0��������255�������� 
			MQTT_Resv_AlarmTime = json_value_1->valueint;
			MQTT_Save_Data.MQTT_Resv_AlarmTime = MQTT_Resv_AlarmTime; 
			UART_Printf("AlarmTime: %d\n", MQTT_Resv_AlarmTime);
		}  

		json_value_2 = cJSON_GetObjectItem(json,"Channel");
		if(json_value_2->type == cJSON_Number)
		{
			// �����ŵ�
			MQTT_Resv_Channel = json_value_2->valueint;
			MQTT_Save_Data.MQTT_Resv_Channel = MQTT_Resv_Channel; 
			UART_Printf("Channel ��%d\n", MQTT_Resv_Channel);
		}   

		json_value_3 = cJSON_GetObjectItem(json,"SensorNum");
		if(json_value_3->type == cJSON_Number)
		{
			// 1-240 �������豸����Ĳɼ�ģ������(OK?)
			MQTT_Resv_SensorNum = json_value_3->valueint;
			 MQTT_Save_Data.MQTT_Resv_SensorNum = MQTT_Resv_SensorNum; 
			UART_Printf("SensorNum ��%d\n", MQTT_Resv_SensorNum);
		}
		
		json_value_4 = cJSON_GetObjectItem(json,"SensorCycle");
		if(json_value_4->type == cJSON_Number)
		{
			// �������ϱ����ڣ���λ���ӣ�10~255�����10���� 
			// �����ݱ��ر��棬��ֹ����û�յ�ʱ���ݶ�ʧ
			MQTT_Resv_SensorCycle = json_value_4->valueint;
			MQTT_Save_Data.MQTT_Resv_SensorCycle = MQTT_Resv_SensorCycle; 
			UART_Printf("SensorCycle ��%d\n", MQTT_Resv_SensorCycle);
		}	
		//�洢
		para_store(STORE_PAGE,MQTT_Save_Data); 		

		MQTT_Save_Data.MQTT_Resv_Cycle 			= 0;
		MQTT_Save_Data.MQTT_Resv_AlarmTime 		= 0;
		MQTT_Save_Data.MQTT_Resv_Channel 		= 0;
		MQTT_Save_Data.MQTT_Resv_SensorNum 		= 0;
		MQTT_Save_Data.MQTT_Resv_SensorCycle 	= 0;
		MQTT_Save_Data.POWER_ON_COUNT			= 1;
	
	}

    cJSON_Delete(json); 
	 
    return 0;
}

// MQTT ����  ȫ���ص�����
// @brief messageArrived callback function 
void messageArrived(MessageData* md)
{
  
	uint32_t i=0; 
	char* data; 
	MQTTMessage* message = md->message;
	
	memset(RecivePkg.buf,0,sizeof(RecivePkg)); 

	for (i = 0; i < md->topicName->lenstring.len; i++)
		IR_SendByte(*(md->topicName->lenstring.data + i));
 
	data = (char*)message->payload;
	RecivePkg.t.len = (int32_t)message->payloadlen;
	memcpy(RecivePkg.t.databuf,data,RecivePkg.t.len);

	UART_Printf(" Message: %s\r\n", RecivePkg.t.databuf); 
	
	//JSON���ݽ����ֵ
	Unpack_json_MQTT_ResvData(RecivePkg.t.databuf);

	RecivePkg.t.is_process_data = true; 
 
}
 
 
 
//�������
void process_message_control(void)
{
	if(RecivePkg.t.is_process_data == true)
	{
		//UART_Printf("Working.......................\r\n");
		RecivePkg.t.is_process_data = false;
		 
	}
	
}

int MQTT_Init(void)
{
	int rc = 0;
	
	NewNetwork(&network, SOCK_TCPS);
	
	UART_Printf("׼�����ӷ�����\r\n");                //���������Ϣ
	__disable_interrupt();
	
	
	//domain_ip[0] = 47;
	//domain_ip[1] = 98;
	//domain_ip[2] = 136;
	//domain_ip[3] = 66; 
	
	ConnectNetwork(&network, IP_Data, Port_Data);
	UART_Printf("IP��ַ: %d.%d.%d.%d,%d\r\n", IP_Data[0],IP_Data[1],IP_Data[2],IP_Data[3],Port_Data);

	MQTTClientInit(&mqttclient,&network,1000,SendBuffer,1200,tempBuffer,10);
	
	delay_ms(500);
	
	// ����MQTT������
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	connectData.willFlag = 0;
	connectData.MQTTVersion = 3;
//	connectData.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	connectData.clientID.cstring = "w5500-client";    //opts.clientid;
	connectData.username.cstring = ""; //opts.username;admin
	connectData.password.cstring = ""; //opts.password; public

	connectData.keepAliveInterval = 60;
	connectData.cleansession = 1;
	
	if ((rc = MQTTConnect(&mqttclient, &connectData)) != 0)
		UART_Printf("Return code from MQTT connect is %dr\n", rc);
	else
		UART_Printf("MQTT Connected\r\n", rc);

	// �������� Updata Alarm Read_data
	sprintf(SubTopic,"/WSN-LW/");
	strcat(SubTopic, Read_ID);
	strcat(SubTopic, "/service/+");// ���Ĳ�������
	
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&mqttclient, SubTopic, QOS0, messageArrived);// /# ����һ�� 
	UART_Printf("Subscribed %d\r\n", rc);
 
	
	rc = MQTT_HeartBeat();	// ����������

	UART_Printf("MQTT Heartbeat Resulu %d \r\n",rc);       	         //���������Ϣ
	para_read(STORE_PAGE,&MQTT_Save_Data);
	//���NRF24L01�����е�����
	Clear_ALL_nrf24l01_TempData(); 
	__enable_interrupt();
	return rc;
}

/**
 * @name: Creat_json_MQTT_SendData
 * @test: test font
 * @msg: ����һ��json��ʽ�������ϴ������ݸ�ʽ
 * @param {Pub_State}  0 : SendDate 1 : Heartbeat
 * @return: char * json����
 */
size_t HeartBeat_lenght = 0;
size_t SendData_lenght = 0; 
uint16_t Pack_Num_Last = 0;		//����������40
uint8_t DataRiver[(PacksSensorNum * 10 )+ 1]  ;  
char * Creat_json_MQTT_SendData(uint8_t Pub_State,uint8_t Pack_NUM)
{
	cJSON * usr; 
	uint8 * out ;
	uint32_t NewTime = 0;
	usr = cJSON_CreateObject(); //���������ݶ���
	uint8_t DataRiver_TempForLess[(PacksSensorNum * 10 )+ 1]  ; 
	switch (Pub_State)
	{
		 //MQTT�����ϱ�-����������40��
	case MQTT_Publish_Type_SendData:
		/* code */
		//���������������󣬸�һ��Ĭ��ֵ
		if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;} 

		memset(DataRiver,0x00, 401 );
		memcpy(DataRiver,DataToSendBuffer + ((PacksSensorNum * 10) * (Pack_NUM-1)) ,(PacksSensorNum * 10)); 
		//�ַ�����ֹ'/0'
		DataRiver[(PacksSensorNum * 10 )] = '\0' ;

    	cJSON_AddItemToObject(usr, "SensorNum", cJSON_CreateNumber(PacksSensorNum));
		cJSON_AddItemToObject(usr, "SensorStart", cJSON_CreateNumber(((Pack_NUM -1)*PacksSensorNum )+ 1));
		cJSON_AddItemToObject(usr, "SensorData", cJSON_CreateString(DataRiver)); 

		//out = cJSON_Print(usr); //��json��ʽ��ӡ�������ַ�����ʽ
		out = cJSON_PrintUnformatted(usr); //��json��ʽ��ӡ�������ַ�����ʽ
		
		UART_Printf("json Data : %s\r\n",out);
		
		UART_Printf("\r\nDataToSendBuffer : %s \r\n",DataRiver)  ;
		
		SendData_lenght = strlen(out)  ; 
 
		UART_Printf("\r\n SendData_lenght : %d \r\n" , SendData_lenght);

		// �ͷ��ڴ� 
		cJSON_Delete(usr); 
		free(out);

		return out;  
	//MQTT���������ϱ�
	case MQTT_Publish_Type_HeartBeat: 
		
		NewTime = GetSystemNowtime();
		 
		cJSON_AddItemToObject(usr,"Heartbeat",cJSON_CreateNumber(NewTime));

		out = cJSON_PrintUnformatted(usr); //��json��ʽ��ӡ�������ַ�����ʽ

		UART_Printf("json Data : %s\n",out);
		// �ͷ��ڴ�
		HeartBeat_lenght = strlen(out);
		UART_Printf("HeartBeat_lenght : %d \r\n" , HeartBeat_lenght);

		cJSON_Delete(usr); 
		free(out);
		
		return out;  
	//��������������40
	case MQTT_Publish_Type_CountLess40 :
		//���������������󣬸�һ��Ĭ��ֵ
		UART_Printf("PacksSensorNum Less than 40 \r\n ");
		if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;}

		memset(DataRiver_TempForLess,0x00,Pack_Num_Last*10);
		memcpy(DataRiver_TempForLess,DataToSendBuffer + ((Pack_Num_Last * 10) * (Pack_NUM-1)) ,(Pack_Num_Last * 10)); 
		//�ַ�����ֹ'/0'
		DataRiver_TempForLess[(Pack_Num_Last * 10 )] = 0x00 ;

    	cJSON_AddItemToObject(usr, "SensorNum", cJSON_CreateNumber(Pack_Num_Last));
		cJSON_AddItemToObject(usr, "SensorStart", cJSON_CreateNumber(((Pack_NUM -1)*Pack_Num_Last )+ 1));
		cJSON_AddItemToObject(usr, "SensorData", cJSON_CreateString(DataRiver_TempForLess)); 

		out = cJSON_PrintUnformatted(usr); //��json��ʽ��ӡ�������ַ�����ʽ
		
		UART_Printf("json Data : %s\r\n",out);
		
		UART_Printf("\r\n DataRiver_TempForLess : %s \r\n",DataRiver_TempForLess)  ;
		
		SendData_lenght = strlen(out) ; 
		 
		UART_Printf("\r\nSendData_lenght : %d \r\n" , SendData_lenght);

		// �ͷ��ڴ� 
		cJSON_Delete(usr); 
		free(out); 
		return out;
	default:

		break;
	}  
}

//������������
int MQTT_HeartBeat(void)
{
	int rc = 0;
	
	static uint16_t mes_id = 0;
	
	sprintf(Topic,"/WSN-LW/");
	strcat(Topic, Read_ID);
	strcat(Topic, "/event/Heartbeat");
	
	
	heartbeat.protocoltype = "w5500";
	heartbeat.heartbeat = mes_id;
	heartbeat.producttype = "WSN-LW";
	heartbeat.softversion = "1.0.1";
	heartbeat.configtag = 1;
	  
	mqtt_msg.qos = QOS0;
	mqtt_msg.retained = 0;
    mqtt_msg.id = mes_id++;
    mqtt_msg.dup = 0;
	mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat,0);
	mqtt_msg.payloadlen = HeartBeat_lenght;
	
	rc = MQTTPublish(&mqttclient, Topic, &mqtt_msg);
	UART_Printf("Publish %s\r\n", Topic);
	//IR_SendData(	Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat)
	//				,HeartBeat_lenght);
	UART_Printf("\r\nPublish %d\r\n", rc);
	UART_Printf( "HeartBeating............" ); 
	return rc;

}


//���ݰ����ʹ�����
int MQTT_SendData(void)
{
	int rc = 0; 
	static uint16_t mes_id = 0; 
	DOUBLE_LINK_NODE *pNode;
	uint8_t tag_cnt	 = 0;
	uint8_t Pack_Num = 0;  
	//Ĭ��120���ӻ�
	if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;}
	//�������
	if (MQTT_Resv_SensorNum >= 40)
	{
		/* code */	
		Pack_Num = MQTT_Resv_SensorNum / PacksSensorNum;
		if (MQTT_Resv_SensorNum % PacksSensorNum )
		{
			Pack_Num ++;
		}
		UART_Printf("Pack_Num is ---------------: %d\r\n" ,Pack_Num);
	}
	else if (MQTT_Resv_SensorNum < 40)
	{
		Pack_Num = 1; 
		Pack_Num_Last = MQTT_Resv_SensorNum;
	}
	DISABLE_GLOBAL_INTERRUPT();
	//�ְ�����
	for (uint8_t i = 1; i <= Pack_Num; i++)
	{	
		sprintf(Topic,"/WSN-LW/");
		strcat(Topic, Read_ID);
		strcat(Topic, "/event/Data");
		
		// init_user(&userInfo);
		userInfo.protocoltype = "w5500";
		userInfo.n_cardlist = tag_cnt;
		userInfo.cardlist = cardInfo_p;
		
		pNode = (&RADIO_DATA_LIST_HEAD)->next;  
		mqtt_msg.qos = QOS0;
		mqtt_msg.retained = 0;
		mqtt_msg.id = mes_id++;
		mqtt_msg.dup = 0;	 

		if (MQTT_Resv_SensorNum >= 40)
		{  
			mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,i) ;   
		}
		else
		{
			mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_CountLess40,i) ; 
		}

		mqtt_msg.payloadlen = SendData_lenght ;//lenght;

		rc = MQTTPublish(&mqttclient, Topic, &mqtt_msg);

		UART_Printf("Publish %s\r\n", Topic);
		//IR_SendData(Buffer,SendData_lenght);

		UART_Printf("\r\nPublish %d\r\n", rc);

		UART_Printf("Sending.........");  

		nrf_delay_ms(1500);
	}  

	ENABLE_GLOBAL_INTERRUPT();
	return rc;
}

int MQTT_Working(void)
{
	process_message_control();
	return MQTTYield(&mqttclient, 60);
    
	
}
/* ѭ����ȡ��������ֵ��������ȡ���Ľ���ύ���������ڴ�ӡ�������ȻҲ���Ը������ֵ��
 * ����LED�ƣ�ѭ����ȡ���ʱ��������10s�������yeelink���Ƶġ�
 */
uint8_t yeelink_get(const char *device_id,const char *sensors_id,char *value)
{
////    int ret;
////    char* presult;
//////    char remote_server[] = "api.yeelink.net";
////	char remote_server[] = "server.yun-ran.com";
////    char str_tmp[128] = {0};
////
////    // ���󻺳�������Ӧ������
////    static char http_request[DATA_BUF_SIZE] = {0};	//����Ϊ��̬��������ֹ��ջ���
////    static char http_response[DATA_BUF_SIZE] = {0};	//����Ϊ��̬��������ֹ��ջ���
//////    sprintf(str_tmp,"/v1.0/device/%s/sensor/%s/datapoints",device_id,sensors_id);
////	sprintf( str_tmp , "/siot/centers/password/%s","12346666");
//////	sprintf( str_tmp , "/1.txt");
////    // ȷ�� HTTP�����ײ�
////    // ����POST /v1.0/device/98d19569e0474e9abf6f075b8b5876b9/1/1/datapoints/add HTTP/1.1\r\n
////    sprintf( http_request , "GET %s HTTP/1.1\r\n",str_tmp);
////
////
////    // �������� ���� Host: api.machtalk.net\r\n
////    sprintf( str_tmp , "Host:%s\r\n" , remote_server);
////    strcat( http_request , str_tmp);
////
////    // �������� ���� APIKey: d8a605daa5f4c8a3ad086151686dce64
////    //sprintf( str_tmp , "U-ApiKey:%s\r\n" , "d8a605daa5f4c8a3ad086151686dce64");//��Ҫ�滻Ϊ�Լ���APIKey
////    sprintf( str_tmp , "U-ApiKey:%s\r\n" , "e5da11d13d2e5f540ef1a99b3506e081");//APIKey--qinfei
////    strcat( http_request , str_tmp);
////    //
////    strcat( http_request , "Accept: */*\r\n");
////    // ���ӱ������ʽ Content-Type:application/x-www-form-urlencoded\r\n
////    strcat( http_request , "Content-Type: application/x-www-form-urlencoded\r\n");
////    strcat( http_request , "Connection: keep-alive\r\n");
////    // HTTP�ײ���HTTP���� �ָ�����
////    strcat( http_request , "\r\n");
////
////    //������ͨ��TCP���ͳ�ȥ
////    //�½�һ��Socket���󶨱��ض˿�5000
////    ret = socket(SOCK_TCPS,Sn_MR_TCP,5000,0x00);
////    if(ret != SOCK_TCPS){
////        UART_Printf("%d:Socket Error\r\n",SOCK_TCPS);
////        while(1);
////    }
//////	domain_ip[0] = 47;
//////	domain_ip[1] = 92;
//////	domain_ip[2] = 31;
//////	domain_ip[3] = 46;
////		
////    //����TCP������
////    ret = connect(SOCK_TCPS,domain_ip,80);
////    if(ret != SOCK_OK){
////        UART_Printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
////        while(1);
////    }	
//////	sprintf( http_request , "GET https://server.yun-ran.com/siot/centers/password/%s HTTP/1.1\r\n","12346666");
////    //��������
////    ret = send(SOCK_TCPS,(unsigned char *)http_request,strlen(http_request));
////    if(ret != strlen(http_request)){
////        UART_Printf("%d:Socket Send Error\r\n",SOCK_TCPS);
////        while(1);
////    }
////
////    // �����Ӧ
////    ret = recv(SOCK_TCPS,(unsigned char *)http_response,DATA_BUF_SIZE);
////    if(ret <= 0){
////        UART_Printf("%d:Socket Get Error\r\n",SOCK_TCPS);
////        while(1);
////    }
////    http_response[ret] = '\0';
////	
////	/*****************/
////	//��֧��https����ֱ����http���󷵻ؽ������
////	/*****************/
////	//	HTTP/1.1 301 Moved Permanently
////	/*****Header*****/
//////////	Server: nginx/1.14.0 (Ubuntu)
//////////	Date: Tue, 28 Apr 2020 05:56:27 GMT
//////////	Content-Type: text/html
//////////	Content-Length: 194
//////////	Connection: keep-alive
//////////	Location: https://server.yun-ran.com/siot/center/password/12346666
////	/*****Body*****/
//////////	<html>
//////////	<head><title>301 Moved Permanently</title></head>
//////////	<body bgcolor="white">
//////////	<center><h1>301 Moved Permanently</h1></center>
//////////	<hr><center>nginx/1.14.0 (Ubuntu)</center>
//////////	</body>
//////////	</html>
////	
////	//�ж��Ƿ��յ�HTTP OK
////    presult = strstr( (const char *)http_response , (const char *)"200 OK\r\n");
////    if( presult != NULL ){
////        static char strTmp[DATA_BUF_SIZE]={0};//����Ϊ��̬��������ֹ��ջ���
////        sscanf(http_response,"%*[^{]{%[^}]",strTmp);
////        //��ȡ������Ϣ
////        char timestamp[64]={0};
////        char timestampTmp[64]={0};
////        char valueTmp[64]={0};
////        sscanf(strTmp,"%[^,],%[^,]",timestampTmp,valueTmp);
////        strncpy(timestamp,strstr(timestampTmp,":")+2,strlen(strstr(timestampTmp,":"))-3);
////        strncpy(value,strstr(valueTmp,":")+1,strlen(strstr(valueTmp,":"))-1);
////    }else{
////        UART_Printf("Http Response Error\r\n");
////        UART_Printf("%s",http_response);
////    }
////    close(SOCK_TCPS);
////    return 0;
}
/*********************************END OF FILE**********************************/

