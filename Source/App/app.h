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
/*定义事件类型数据类型*/
typedef uint16 EventType;

//--------------------定义软件在环执行事件--------------------------------------
////#define APP_EVENT_GATEWAY_RECVNET         0x0001  /*软件在环--接收网络协议数据事件*/
////#define APP_EVENT_GATEWAY_NETTOEDV        0x0002  /*软件在环--网络终端协议转换事件*/
////#define APP_EVENT_GATEWAY_SENDEDV         0x0004  /*软件在环--发送终端协议数据事件*/
////#define APP_EVENT_GATEWAY_RECVEDV         0x0008  /*软件在环--接收终端协议数据事件*/
////#define APP_EVENT_GATEWAY_EDVTONET        0x0010  /*软件在环--终端网络协议转换事件*/
////#define APP_EVENT_GATEWAY_SENDNET         0x0020  /*软件在环--发送网络协议数据事件*/
////#define APP_EVENT_GATEWAY_TIMEING         0x0040  /*软件在环--网关定时事件*/
////#define APP_EVENT_GATEWAY_TEST            0x0080  /*软件在环--网关测试事件*/

////#define APP_EVENT_GATEWAY_NETINPUT        0x0100  /*软件在环--轮询接收网络数据包事件*/

#define APP_EVENT_GATEWAY_NETINT			0x0001  /*软件在环--w5500初始化事件*/
#define APP_EVENT_GATEWAY_DHCP			0x0002  /*软件在环--w5500 DHCP事件*/
#define APP_EVENT_GATEWAY_CONNECT		0x0004  /*软件在环--w5500连接事件*/
#define APP_EVENT_GATEWAY_RECVNET		0x0008  /*软件在环--w5500接收事件*/

#define APP_EVENT_GATEWAY_GPRSINT		0x0010  /*软件在环--2G/4G模块初始化事件*/
#define APP_EVENT_GATEWAY_GPRSDO		0x0020  /*软件在环--2G/4G模块指令事件*/

#define APP_EVENT_GATEWAY_TIMEING         0x8000  /*软件在环--网关定时事件*/

//-----------------------定时事件执行时间--------------------------------------
#define HEART_BEAT_TIME         1000   /*心跳事件时间：3000*10ms每30s心跳一次*/
#define SEND_DATA_TIME          200   /*数据包事件时间：200*10ms每2s检查一次*/
#define GPRS_Rscv_DATA_TIME     150   /*数据包事件时间：100*10ms每1.5s检查一次*/

#define APP_INPUT_TMR_INTERVAL  1
#define APP_TCP_TMR_INTERVAL    25     /*TCP 的调度周期是 250 mstcp_tmr(); */
#define APP_ARP_TMR_INTERVAL    500    /*ARP 的调度周期是 5s  etharp_tmr();*/


//--------------------------定时事件-------------------------------------------
#define APP_T_EVENT_HEARTBEAT	    0x0001   /*心跳包事件       */
#define APP_T_EVENT_SENDDATA		0x0002   /*数据包事件       */


//------------------------函数声明区-------------------------------------------
/*系统软件初始化*/
extern void SoftWareInit(void);

/*循环事件标志位置位*/
extern void LoopEventSetBit(EventType event);

/*定时循环事件标志位置位*/
extern void LoopTimeEventSetBit(EventType event);

/*软件在环执行*/
extern void SoftWareEventInLoop(void);

/*任务在环执行*/
//extern void SoftWareTaskInLoop(void);

/*定时事件在环执行*/
extern void SoftWareTimeingEventInLoop(void);

/*定时循环事件执行函数*/
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

