/*
 * @Descripttion: 
 * @version: 
 * @Author: lzc
 * @Date: 2020-05-29 15:25:53
 * @LastEditors: lzc
 * @LastEditTime: 2020-06-16 14:00:16
 */ 
/**
 * NetHander.h �����ʼ������
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#ifndef __NET_HANDLER_H__
#define	__NET_HANDLER_H__
//#include "stm32f10x_it.h"
#include "wizchip_conf.h" //������Ҫ��ͷ�ļ�
#include "dhcp.h"
/* Private define ------------------------------------------------------------*/
#define SOCK_TCPS        	0
#define SOCK_DHCP			0
#define SOCK_DNS	        1
#define MY_MAX_DHCP_RETRY	0   /*DHCP����������*/
#define DATA_BUF_SIZE           2048 /*��ȡ���ݵ�����ֽ���*/
#define MQTT_Publish_Type_SendData	  0
#define MQTT_Publish_Type_HeartBeat   1
#define MQTT_Publish_Type_CountLess40 2

#define	PacksSensorNum				  40
//Alarm ���Ľ���
extern uint8_t MQTT_Resv_Alarm ;
extern uint8_t *MQTT_Resv_AlarmData ;
//Read_Data ���Ľ���
extern uint8_t MQTT_Resv_Read_data ;
//Updata ���Ľ���
extern uint8_t MQTT_Resv_Cycle ;
extern uint8_t MQTT_Resv_AlarmTime ;
extern uint8_t MQTT_Resv_Channel ;
extern uint8_t MQTT_Resv_SensorNum ;
extern uint8_t MQTT_Resv_SensorCycle ; 
//IP�Լ��˿ں�
extern uint8_t IP_Data[4] 	 ;
extern uint16_t Port_Data    ;
//GPRS��־λ
extern uint8_t W5500_NOPHY_TryGPRS_Flag  ;
typedef struct   //ATָ��ṹ��  5���ֶ�
{
	uint8_t cardid[2];
	uint8_t cardrssi;
	uint8_t cardpower;
	uint8_t nodeid_1[2];
	uint8_t noderssi_1;
    uint8_t nodeid_2[2];
	uint8_t noderssi_2;
    uint8_t nodeid_3[2];
	uint8_t noderssi_3;
    uint8_t nodeid_4[2];
	uint8_t noderssi_4;
    uint8_t nodeid_5[2];
	uint8_t noderssi_5;
}card_pkg_t,*card_pkg;

extern uint8_t Read_ID[16];
extern uint8_t SensorNum;

extern uint16_t Pack_Num_Last ;		//����������40
extern wiz_NetInfo gWIZNETINFO;

typedef struct TCP_Network
{ 
	uint8_t My_TCPSocket_Num ;
	uint8_t My_TCP_Connect_IP[4] ;
	uint16_t My_TCP_Connect_Port ; 

}TCP_Network_Info; 

extern TCP_Network_Info TCP_network;
/* Private functions ---------------------------------------------------------*/
void network_init(void);      /*�����ʼ��*/
int NetworkInitHandler(void);/*����W5500����*/
void DhcpRunInLoop(void);     /*DHCP running*/

uint8_t DHCP_proc(void);

int MQTT_Init(void);

int MQTT_Working(void);
int MQTT_HeartBeat(void);
int MQTT_SendData(void);
char * Creat_json_MQTT_SendData(uint8_t Pub_State,uint8_t Pack_NUM);
int Unpack_json_MQTT_ResvData(uint8_t * ResvData);
uint8_t yeelink_get(const char *device_id,const char *sensors_id,char *value); 
//����TCP����
void Creat_TCP_Client(TCP_Network_Info* n );
#endif /* __NET_HANDLER_H__ */