/**
 * NetHander.h 网络初始化函数
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#ifndef __AT_PROC_H__
#define	__AT_PROC_H__
//#include "stm32f10x_it.h"
//包含需要的头文件
#include <stdint.h>
/* Private define ------------------------------------------------------------*/
#define SENDDATATIME 10*1000

typedef enum    
{
	SUCCESS_REC = 0,
	TIME_OUT,
	NO_REC
}teATStatus;
typedef enum   //AT指令状态机
{
	NB_IDIE = 0,
	NB_SEND,
	NB_WAIT,
	NB_ACCESS
}teNB_TaskStatus;
extern teNB_TaskStatus NB_TaskStatus;      //AT指令模式

typedef struct   //AT指令结构体  5个字段
{
	char *ATSendStr;
	char *ATRecStr;
	uint16_t TimeOut; //超时
	teATStatus ATStatus;  //指令状态
	uint8_t RtyNum;  //重发次数
}tsATCmds;

//AT指令枚举，与前面AT指令对应
//typedef enum
//{
//	AT = 0,
//	ATE0,
//	AT_GMR,       //查询版本号
//	AT_I,         //查询模组信息IMEI
//	AT_ZCONTLED,  //关灯
//	AT_ZSLR,      //是否开启休眠
//	AT_ZSLR_1,    //开启休眠
//	AT_CPSMS, 	  //是否设置休眠时间
//	AT_CPSMS_1,   //设置休眠时间
//	AT_CSQ,       //测试信号
//	AT_CEREG,     //测试驻网
//	AT_ECOAPNEW,  //连接COAP
//	AT_CSQ_2,     //读取信号
//	AT_CCLK,      //获取网络时间
//	AT_ECOAPSEND, //发送COAP消息
//	AT_ECOAPDEL,  //取消COAP连接
//	AT_ZRST,
//	//AT_IDIE,
//}teATCmdNum;

typedef enum
{
	AT = 0,
	ATE0,
	AT_CIPHEAD,		//接收数据时是否增加 IP 头提示 
	AT_CPIN,		//查询PIN码锁状态
	AT_CREG,		//网络注册信息
	AT_CREG_1,		//查询网络注册信息
	AT_COPS,		//设置运营商
	AT_COPS_1,      //查询运营商
	AT_CSQ,      	//查询信号
	AT_CGATT,		//是否附着GPRS
	AT_CIPSHUT,    	//关闭GPRS(PDP上下文去激活)
	AT_CSTT, 	  	//设置APN
	AT_CIICR,   	//激活GPRS连接
	AT_CIFSR,       //获取本机地址 
	AT_MCONFIG,		//MQTT设置参数
	AT_MIPSTART,	//设置连接时的地址和端口号
	AT_MCONNECT,	//开始建立MQTT链接
	AT_MQTTMSGSET,	//设置MQTT的参数
	AT_MPUB,		//MQTT的发送  

}teATCmdNum;
extern teATCmdNum ATNextCmdNum;                                        //下条指令
extern teATCmdNum ATCurrentCmdNum;                                     //当前指令
extern uint8_t GPRS_InitOver_Flag ;
void NB_Work(void);

void ATRec(void);
void MQTT_GPRS_ResvData(void);
void NB_Init(void);
void NB_Task(void);
void Rec_WaitAT(void);
void COAPSendData(uint16_t Flag);
void MQTT_GPRS_SendRscvDataCMD(void);
/*定时器相关*/
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