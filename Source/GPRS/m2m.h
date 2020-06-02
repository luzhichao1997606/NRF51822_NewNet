#ifndef M2M_H
#define M2M_H

//#include "hal.h"

#include "Uart.h"	

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

typedef struct
{
    uint8_t  u8Count;      // 接收buf有效字节数
    uint8_t  Buf[40];  // 接收buf
} st_AES;

extern st_AES stAES;
extern gprs_info_t m_gprs_info;

void  			m2m_pwron(unsigned char NewState);

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

uint32_t 		m2m_getrcvdata(uint8_t *pdata);

uint8_t  		m2m_init(void);
void     		m2m_echoclose(void);

void  m2m_reset(void);
	
en_M2MState m2m_Get_NetInfo(void);
en_M2MState Check_Response(const char * Response, unsigned int Response_Time_Out);
en_M2MState SendAT_CheckResponse(char* AT_Command, const char * Response, unsigned int Response_Time_Out, uint8_t ReSendCount);























#endif
