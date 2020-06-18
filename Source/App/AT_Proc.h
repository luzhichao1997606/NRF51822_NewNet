/**
 * NetHander.h �����ʼ������
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#ifndef __AT_PROC_H__
#define	__AT_PROC_H__
//#include "stm32f10x_it.h"
//������Ҫ��ͷ�ļ�
#include <stdint.h>
/* Private define ------------------------------------------------------------*/
#define SENDDATATIME 10*1000

typedef enum    
{
	SUCCESS_REC = 0,
	TIME_OUT,
	NO_REC
}teATStatus;
typedef enum   //ATָ��״̬��
{
	NB_IDIE = 0,
	NB_SEND,
	NB_WAIT,
	NB_ACCESS
}teNB_TaskStatus;
extern teNB_TaskStatus NB_TaskStatus;      //ATָ��ģʽ

typedef struct   //ATָ��ṹ��  5���ֶ�
{
	char *ATSendStr;
	char *ATRecStr;
	uint16_t TimeOut; //��ʱ
	teATStatus ATStatus;  //ָ��״̬
	uint8_t RtyNum;  //�ط�����
}tsATCmds;

//ATָ��ö�٣���ǰ��ATָ���Ӧ
//typedef enum
//{
//	AT = 0,
//	ATE0,
//	AT_GMR,       //��ѯ�汾��
//	AT_I,         //��ѯģ����ϢIMEI
//	AT_ZCONTLED,  //�ص�
//	AT_ZSLR,      //�Ƿ�������
//	AT_ZSLR_1,    //��������
//	AT_CPSMS, 	  //�Ƿ���������ʱ��
//	AT_CPSMS_1,   //��������ʱ��
//	AT_CSQ,       //�����ź�
//	AT_CEREG,     //����פ��
//	AT_ECOAPNEW,  //����COAP
//	AT_CSQ_2,     //��ȡ�ź�
//	AT_CCLK,      //��ȡ����ʱ��
//	AT_ECOAPSEND, //����COAP��Ϣ
//	AT_ECOAPDEL,  //ȡ��COAP����
//	AT_ZRST,
//	//AT_IDIE,
//}teATCmdNum;

typedef enum
{
	AT = 0,
	ATE0,
	AT_CIPHEAD,		//��������ʱ�Ƿ����� IP ͷ��ʾ 
	AT_CPIN,		//��ѯPIN����״̬
	AT_CREG,		//����ע����Ϣ
	AT_CREG_1,		//��ѯ����ע����Ϣ
	AT_COPS,		//������Ӫ��
	AT_COPS_1,      //��ѯ��Ӫ��
	AT_CSQ,      	//��ѯ�ź�
	AT_CGATT,		//�Ƿ���GPRS
	AT_CIPSHUT,    	//�ر�GPRS(PDP������ȥ����)
	AT_CSTT, 	  	//����APN
	AT_CIICR,   	//����GPRS����
	AT_CIFSR,       //��ȡ������ַ 
	AT_MCONFIG,		//MQTT���ò���
	AT_MIPSTART,	//��������ʱ�ĵ�ַ�Ͷ˿ں�
	AT_MCONNECT,	//��ʼ����MQTT����
	AT_MQTTMSGSET,	//����MQTT�Ĳ���
	AT_MPUB,		//MQTT�ķ���  

}teATCmdNum;
extern teATCmdNum ATNextCmdNum;                                        //����ָ��
extern teATCmdNum ATCurrentCmdNum;                                     //��ǰָ��
extern uint8_t GPRS_InitOver_Flag ;
void NB_Work(void);

void ATRec(void);
void MQTT_GPRS_ResvData(void);
void NB_Init(void);
void NB_Task(void);
void Rec_WaitAT(void);
void COAPSendData(uint16_t Flag);
void MQTT_GPRS_SendRscvDataCMD(void);
/*��ʱ�����*/
typedef struct
{
	uint32_t TimeStart;
	uint32_t TimeInter;
}tsTimeType;
void GetTime(tsTimeType *TimeType,uint32_t TimeInter);
uint8_t  CompareTime(tsTimeType *TimeType);
/* Private functions ---------------------------------------------------------*/
extern void MQTT_GPRS_SendData();
extern void MQTT_GPRS_Heartbeat();

extern char IP_Config_Data[18]    ;
extern char Port_Config_Data[8]   ;

#endif /* __NET_HANDLER_H__ */