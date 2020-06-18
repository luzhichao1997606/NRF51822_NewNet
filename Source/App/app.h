/******************************************************************************
* File        : app.h
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date         Modified By   Description               
* 1     V1.0      2011-05-01   Lihao         Original Version
*******************************************************************************/
#ifndef APP_H
#define APP_H

#if defined __cplusplus
extern "C" {
#endif

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/

/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/

/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/
/*�����¼�������������*/
typedef uint16 EventType;

//--------------------��������ڻ�ִ���¼�--------------------------------------
////#define APP_EVENT_GATEWAY_RECVNET         0x0001  /*����ڻ�--��������Э�������¼�*/
////#define APP_EVENT_GATEWAY_NETTOEDV        0x0002  /*����ڻ�--�����ն�Э��ת���¼�*/
////#define APP_EVENT_GATEWAY_SENDEDV         0x0004  /*����ڻ�--�����ն�Э�������¼�*/
////#define APP_EVENT_GATEWAY_RECVEDV         0x0008  /*����ڻ�--�����ն�Э�������¼�*/
////#define APP_EVENT_GATEWAY_EDVTONET        0x0010  /*����ڻ�--�ն�����Э��ת���¼�*/
////#define APP_EVENT_GATEWAY_SENDNET         0x0020  /*����ڻ�--��������Э�������¼�*/
////#define APP_EVENT_GATEWAY_TIMEING         0x0040  /*����ڻ�--���ض�ʱ�¼�*/
////#define APP_EVENT_GATEWAY_TEST            0x0080  /*����ڻ�--���ز����¼�*/

////#define APP_EVENT_GATEWAY_NETINPUT        0x0100  /*����ڻ�--��ѯ�����������ݰ��¼�*/

#define APP_EVENT_GATEWAY_NETINT			0x0001  /*����ڻ�--w5500��ʼ���¼�*/
#define APP_EVENT_GATEWAY_DHCP			0x0002  /*����ڻ�--w5500 DHCP�¼�*/
#define APP_EVENT_GATEWAY_CONNECT		0x0004  /*����ڻ�--w5500�����¼�*/
#define APP_EVENT_GATEWAY_RECVNET		0x0008  /*����ڻ�--w5500�����¼�*/

#define APP_EVENT_GATEWAY_GPRSINT		0x0010  /*����ڻ�--2G/4Gģ���ʼ���¼�*/
#define APP_EVENT_GATEWAY_GPRSDO		0x0020  /*����ڻ�--2G/4Gģ��ָ���¼�*/

#define APP_EVENT_GATEWAY_TIMEING         0x8000  /*����ڻ�--���ض�ʱ�¼�*/

//-----------------------��ʱ�¼�ִ��ʱ��--------------------------------------
#define HEART_BEAT_TIME         1000   /*�����¼�ʱ�䣺3000*10msÿ30s����һ��*/
#define SEND_DATA_TIME          200   /*���ݰ��¼�ʱ�䣺200*10msÿ2s���һ��*/
#define GPRS_Rscv_DATA_TIME     150   /*���ݰ��¼�ʱ�䣺100*10msÿ1.5s���һ��*/

#define APP_INPUT_TMR_INTERVAL  1
#define APP_TCP_TMR_INTERVAL    25     /*TCP �ĵ��������� 250 mstcp_tmr(); */
#define APP_ARP_TMR_INTERVAL    500    /*ARP �ĵ��������� 5s  etharp_tmr();*/


//--------------------------��ʱ�¼�-------------------------------------------
#define APP_T_EVENT_HEARTBEAT	    0x0001   /*�������¼�       */
#define APP_T_EVENT_SENDDATA		0x0002   /*���ݰ��¼�       */


//------------------------����������-------------------------------------------
/*ϵͳ�����ʼ��*/
extern void SoftWareInit(void);

/*ѭ���¼���־λ��λ*/
extern void LoopEventSetBit(EventType event);

/*��ʱѭ���¼���־λ��λ*/
extern void LoopTimeEventSetBit(EventType event);

/*����ڻ�ִ��*/
extern void SoftWareEventInLoop(void);

/*�����ڻ�ִ��*/
//extern void SoftWareTaskInLoop(void);

/*��ʱ�¼��ڻ�ִ��*/
extern void SoftWareTimeingEventInLoop(void);

/*��ʱѭ���¼�ִ�к���*/
extern uint8 SoftTimeingLoopEvent(void);

extern uint8_t MQTT_Relay_AlarmCount_flag ;

extern uint32_t MQTT_Relay_AlarmCount ;

extern bool First_Power_ON_Flag;

extern uint8_t Step_Flag;
#if defined __cplusplus
}
#endif

#endif /* APP_H */
/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

