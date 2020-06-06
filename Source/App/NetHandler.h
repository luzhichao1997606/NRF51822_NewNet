/*
 * @Descripttion: 
 * @version: 
 * @Author: lzc
 * @Date: 2020-05-29 15:25:53
 * @LastEditors: lzc
 * @LastEditTime: 2020-06-06 12:17:37
 */ 
/**
 * NetHander.h 网络初始化函数
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#ifndef __NET_HANDLER_H__
#define	__NET_HANDLER_H__
//#include "stm32f10x_it.h"
#include "wizchip_conf.h" //包含需要的头文件
#include "dhcp.h"
/* Private define ------------------------------------------------------------*/
#define SOCK_TCPS        	0
#define SOCK_DHCP		0
#define SOCK_DNS	        1
#define MY_MAX_DHCP_RETRY	10   /*DHCP重连最大次数*/
#define DATA_BUF_SIZE           2048 /*获取数据的最大字节数*/
#define MQTT_Publish_Type_SendData	  0
#define MQTT_Publish_Type_HeartBeat   1
#define MQTT_Publish_Type_CountLess40 2
//Alarm 订阅解析
extern uint8_t MQTT_Resv_Alarm ;
extern uint8_t *MQTT_Resv_AlarmData ;
//Read_Data 订阅解析
extern uint8_t MQTT_Resv_Read_data ;
//Updata 订阅解析
extern uint8_t MQTT_Resv_Cycle ;
extern uint8_t MQTT_Resv_AlarmTime ;
extern uint8_t MQTT_Resv_Channel ;
extern uint8_t MQTT_Resv_SensorNum ;
extern uint8_t MQTT_Resv_SensorCycle ; 

typedef struct   //AT指令结构体  5个字段
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
extern wiz_NetInfo gWIZNETINFO;

/* Private functions ---------------------------------------------------------*/
void network_init(void);      /*网络初始化*/
int NetworkInitHandler(void);/*配置W5500网络*/
void DhcpRunInLoop(void);     /*DHCP running*/

uint8_t DHCP_proc(void);

int MQTT_Init(void);

int MQTT_Working(void);
int MQTT_HeartBeat(void);
int MQTT_SendData(void);

uint8_t yeelink_get(const char *device_id,const char *sensors_id,char *value);

#endif /* __NET_HANDLER_H__ */