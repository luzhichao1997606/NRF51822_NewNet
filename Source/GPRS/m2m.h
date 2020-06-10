#ifndef M2M_H
#define M2M_H

#include "hal.h"

typedef enum 
{
  M2M_CMD_SUCCESS,
  M2M_CMD_ERROR,
  M2M_CMD_TIMEOUT,
}en_M2MState;

//#define SERVER_IP_ADDR        	"www.cortp.com"
//#define SERVER_PORT           	"7211"

/*gprs information */
typedef struct
{
  char lac[5];
  char cid[5];
  char mcc[5];
  char mic[5];
  char imsi[16];
  char imei[16];
  char csq[10];
}gprs_info_t;



extern gprs_info_t m_gprs_info;

void m2m_pwron(unsigned char NewState);

en_M2MState m2m_gprscheck(void);
en_M2MState m2m_findsim(void);

en_M2MState m2m_findcreg(void);

en_M2MState m2m_getcsq(void);

en_M2MState m2m_findcgatt(void);

en_M2MState m2m_setpdp(void);

en_M2MState m2m_getconstate(void);

en_M2MState m2m_CregSetForLocation(void);

en_M2MState m2m_setapn(void);

en_M2MState m2m_actgprs(void);

en_M2MState m2m_closegprs(void);

en_M2MState m2m_tcpconnect(void);

en_M2MState 	m2m_TCP_Send(void);

en_M2MState     m2m_tcp_close(void);

en_M2MState  	m2m_manualget(void);

en_M2MState     m2m_GetIMEI(void);

en_M2MState     m2m_GetIMSI(void);

en_M2MState     m2m_getlocalip(void);


en_M2MState     m2m_Mconfig(void);	// Mqtt 配置
en_M2MState     m2m_Mipstart(void);	// Mqtt IP地址或域名地址，以及端口号
en_M2MState   	m2m_Mconnect(void);	// 建立mqtt会话
en_M2MState   	m2m_Mmsgset(void);	// 消息上报模式:缓存模式,用 AT+MQTTMSGGET 来读消息
en_M2MState   	m2m_Mmsgget(void);  // 查询缓存中的主题订阅消息
en_M2MState   	m2m_Msub(void);		// 订阅	
en_M2MState     m2m_Mpub(const char *msg);		// 发送消息

uint32_t 		m2m_getrcvdata(uint8_t *pdata);

uint8_t  		m2m_init(void);
void     		m2m_echoclose(void);

en_M2MState m2m_Get_NetInfo(void);
en_M2MState Check_Response(const char * Response, unsigned int Response_Time_Out);
en_M2MState SendAT_CheckResponse(const char* AT_Command, const char * Response, unsigned int Response_Time_Out, uint8 ReSendCount);






 

#endif
