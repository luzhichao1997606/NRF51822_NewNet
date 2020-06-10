#include <string.h>
#include "m2m.h"
#include "common.h"

#define M2M_TASK_NUM  15

void m2m_echoclose(void);
void m2m_reset(void);
typedef en_M2MState (*m2m_task_function)(void);
gprs_info_t m_gprs_info;

const static m2m_task_function m2m_task_list[]=
{
	m2m_gprscheck,
	m2m_findsim,
	m2m_GetIMEI,
	m2m_GetIMSI,
	m2m_CregSetForLocation,
	m2m_findcreg,
	m2m_Get_NetInfo,
	m2m_getcsq,
	m2m_findcgatt,
	m2m_closegprs,
	m2m_Mconfig,	// Mqtt 配置
	m2m_Mipstart,	// Mqtt IP地址或域名地址，以及端口号
	m2m_Mconnect,	// 建立mqtt会话
	m2m_Mmsgset,	// 消息上报模式:缓存模式,用 AT+MQTTMSGGET 来读消息
	m2m_Msub,		// 订阅	
	// m2m_Mpub,		// 发送消息

	// m2m_setapn,
	// m2m_actgprs,
	// m2m_getlocalip,
	// m2m_tcpconnect,
};

/****************************************************************************
*func: send M2M command
*para: the command that will be execute
****************************************************************************/
static void Send_AT_Command(const char* AT_Command)
{
	
	clearUart();	// 清空串口信息
	
	Uart_SendStr((uint8 const *)AT_Command);
	Uart_SendStr("\r\n");
	tmrDelay(100);
	
	#if AT_Debug
	//		if(m_mode == WORK_MODE)
	//			printf_debug("%s",AT_Command);
	if(macPIB.u8NodeType != TEST_MODE)
	{
		USART1TxRxPortA_Status(ENABLE);	// 使能串口PA映射，关闭PC端口上拉	
		Uart_SendStr(AT_Command);
		Uart_SendStr("\r\n");
		USART1TxRxPortA_Status(DISABLE);	// 关闭串口PA映射，使能PC端口上拉
	}
	#endif
	
}

/******************************************************************************
* Function name:       Check_Response
* Descriptions:        
* input parameters:    Response:                wanted response
Resonse_Time _out :  the uint is 10 ticks(100ms) 
*
*Returned value:     result that execute the cmd
******************************************************************************/
en_M2MState Check_Response(const char * Response, unsigned int Response_Time_Out)
{
	while(Response_Time_Out--)
	{
		
		if(strstr((char*)stUart.Buf, Response) != NULL)
		{
			#if AT_Debug
			if(macPIB.u8NodeType != TEST_MODE)
			{
				USART1TxRxPortA_Status(ENABLE);	// 使能串口PA映射，关闭PC端口上拉	
				Uart_SendStr(stUart.Buf);
				Uart_SendStr("\r\n");
				USART1TxRxPortA_Status(DISABLE);	// 关闭串口PA映射，使能PC端口上拉
			}
			#endif
			return M2M_CMD_SUCCESS;
		}
		tmrDelay(100);
	}
	#if AT_Debug
	if(macPIB.u8NodeType != TEST_MODE)
	{
		USART1TxRxPortA_Status(ENABLE);	// 使能串口PA映射，关闭PC端口上拉	
		Uart_SendStr(stUart.Buf);
		Uart_SendStr("\r\n");
		USART1TxRxPortA_Status(DISABLE);	// 关闭串口PA映射，使能PC端口上拉	
	}
	#endif
	
	return M2M_CMD_TIMEOUT;
}

// 发送AT指令，并检测应答，可指定重发次数
// AT_Command:需要发送的AT指令
// Response:等待的应答内容
// Response_Time_Out:等待应答的超时时间，单位100ms
// ReSendCount:等到应答超时之后，重发AT命令的次数
en_M2MState SendAT_CheckResponse(const char* AT_Command, const char * Response, unsigned int Response_Time_Out, uint8 ReSendCount)
{
	unsigned int temp;
	
	while(ReSendCount--)
	{
		Send_AT_Command(AT_Command);
		
		for(temp = 0; temp < Response_Time_Out; temp++)
		{
			if(strstr((char*)stUart.Buf, Response) != NULL)
			{
				return M2M_CMD_SUCCESS;
			}
			tmrDelay(100);
		}
		
	}
	
	return M2M_CMD_TIMEOUT;
}

/**************************************************************************
*控制M2M电源开关
*para: ENABLE:打开电源；DISABLE：关闭电源
**************************************************************************/
void  m2m_pwron(unsigned char NewState)
{
	if(NewState == ENABLE)
	{ 
		nrf_gpio_pin_set(PWRKEY);
		delay_ms(500); 
		nrf_gpio_pin_clear(PWRKEY);
		delay_ms(5000); 
	}
	else
	{
//		USART1TxRxPortA_Status(ENABLE);	// 使能串口PA映射，关闭PC端口上拉	
//		Uart_SendStr("POWER DOWN\r\n");
		nrf_gpio_pin_set(PWRKEY);
		delay_ms(500); 
	}
}

// 检测GPRS模块是否开机
en_M2MState m2m_gprscheck(void)
{
	if(SendAT_CheckResponse("AT", "OK", 10, 5) != M2M_CMD_SUCCESS)
	{
		return M2M_CMD_ERROR;
	}
	
	if(SendAT_CheckResponse("ATE0", "OK", 10, 2) != M2M_CMD_SUCCESS)
	{
		return M2M_CMD_ERROR;
	}
	
	if(SendAT_CheckResponse("AT+CIPHEAD=1", "OK", 10, 1) != M2M_CMD_SUCCESS)
	{
		return M2M_CMD_ERROR;
	}
	return M2M_CMD_SUCCESS;
}

// 检测SIM卡是否存在
en_M2MState m2m_findsim(void)
{
	return SendAT_CheckResponse("AT+CPIN?", "READY", 10, 10);
}
/*
*fun : set creg for location information
*para:null
*/
en_M2MState m2m_CregSetForLocation(void)
{	
	return SendAT_CheckResponse("AT+CREG=2", "OK", 10, 10);
}

/*************************************************************************
*fun: search the network
*para:NULL
*return: success or error
*************************************************************************/
en_M2MState m2m_findcreg(void)
{
	uint8_t retry_cnt = 0;
	uint8_t retry2 = 0;
	char *ptr;
	
  retry:
	Send_AT_Command("AT+CREG?");
	if(Check_Response("OK",15) != M2M_CMD_SUCCESS)
	{
		tmrDelay(1000);
		retry2++;
		if(retry2 >= 5) 
			return M2M_CMD_ERROR;
		else
			goto retry;
	}
	else
	{
		retry2= 0;
		tmrDelay(1000);
		ptr = strchr((char*)stUart.Buf,',');
		if(ptr)
		{
			ptr++;
			if((*ptr == '1') ||(*ptr == '5'))
			{
				ptr = strchr((const char*)ptr,'"');  		//得到基站位置信息
				ptr++;
				comMEMCPY((uint8 *)m_gprs_info.lac,(uint8 const *)ptr,4);
				ptr = strchr((const char*)ptr,',');
				ptr++;
				ptr++;
				comMEMCPY((uint8 *)m_gprs_info.cid,(uint8 const *)ptr,4);
				return M2M_CMD_SUCCESS;
			}
		}
		retry_cnt++;
		if(retry_cnt>=30)
			return M2M_CMD_ERROR;
		goto retry;
	}
}

// 得到国家移动码信息
en_M2MState m2m_Get_NetInfo(void)
{
	char *p;
//	uint8_t retry_cnt = 0;
	char netinfo[6];
	int i;
	SendAT_CheckResponse("AT+COPS=3,2", "OK", 20, 10);
	SendAT_CheckResponse("AT+COPS?", "OK", 20, 10);
//  res:
//	Send_AT_Command("AT+COPS=3,2\r");
//	if(Check_Response("OK", 20) != M2M_CMD_SUCCESS)
//	{
//		retry_cnt++;
//		if(retry_cnt>=10)
//			return M2M_CMD_ERROR;
//		else
//			goto res;
//	}
//  cops_retry:
//	Send_AT_Command("AT+COPS?\r");
//	if(Check_Response("OK", 20) != M2M_CMD_SUCCESS)
//	{	
//		retry_cnt++;
//		if(retry_cnt>=10)
//			return M2M_CMD_ERROR;
//		else
//			goto cops_retry;
//	}
	comMEMSET((uint8 *)m_gprs_info.mcc,0,sizeof(m_gprs_info.mcc));
	comMEMSET((uint8 *)m_gprs_info.mic,0,sizeof(m_gprs_info.mic));
	comMEMSET((uint8 *)netinfo,0,sizeof(netinfo));
	p = strchr((char const *)stUart.Buf,'\"');
	if(p == NULL)
		return M2M_CMD_ERROR;
	p++;
	for(i = 0;*p != '\"';i++)
	{
		netinfo[i] = *p;
		p++;
	}
	comMEMCPY((uint8 *)m_gprs_info.mcc,(uint8 const *)netinfo,3);
	comMEMCPY((uint8 *)m_gprs_info.mic,(uint8 const *)(netinfo+3),strlen(netinfo)-3);
	return  M2M_CMD_SUCCESS;
}

/**********************************************************************
*fun:get csq
*para:null
*return:success or error
**********************************************************************/
en_M2MState m2m_getcsq(void)
{
	char *p;
	Send_AT_Command("AT+CSQ");				//配置 PDP context
	if( Check_Response("OK",20) == M2M_CMD_SUCCESS)
	{
		p = strstr((char const *)stUart.Buf,": ");
		p++;
		comMEMSET((uint8 *)m_gprs_info.csq,0,sizeof(m_gprs_info.csq));
		for(int i = 0;*p != ',';p++,i++)
			m_gprs_info.csq[i] = *p;
		return M2M_CMD_SUCCESS;
	}
	else
		return M2M_CMD_ERROR;
}
/*
@brief:get the IMEI of the sim card
*/
en_M2MState m2m_GetIMEI(void)
{
	char  *cstart,i =0;	
	SendAT_CheckResponse("AT+CGSN", "OK", 10, 10);
	
	comMEMSET((uint8 *)m_gprs_info.imei,0,sizeof(m_gprs_info.imei));
	cstart = strchr((char const *)stUart.Buf,'\n');
	if(cstart)
	{
		++cstart;
		while((*cstart >= '0') &&(*cstart <='9'))
			m_gprs_info.imei[i++] = *cstart++;
	}
	else
		return   M2M_CMD_ERROR;
	return  M2M_CMD_SUCCESS;
	
//	Send_AT_Command("AT+CGSN");
//	if(Check_Response("OK", 10) != M2M_CMD_SUCCESS)
//		return M2M_CMD_ERROR;
//	
//	comMEMSET((uint8 *)m_gprs_info.imei, 0, sizeof(m_gprs_info.imei));
//	for(; i < strlen((char const *)stUart.Buf); i++)
//	{
//		if((stUart.Buf[i] >= '0') &&(stUart.Buf[i] <= '9'))
//		{
//			m_gprs_info.imei[j++] = stUart.Buf[i];
//		}
//	}
//	return M2M_CMD_SUCCESS;
}
/*
@bief:得到SIM卡的IMSI
*/
en_M2MState m2m_GetIMSI(void)
{
//	uint16_t retry=0;
	char  *cstart,i =0;	
	SendAT_CheckResponse("AT+CIMI", "OK", 10, 10);
//  x:
//	Send_AT_Command("AT+CIMI\r");
//	if(Check_Response("OK", 10) != M2M_CMD_SUCCESS)
//	{
//		retry++;
//		if(retry >= 10)
//			return M2M_CMD_ERROR;
//		else
//			goto x;
//	}
	comMEMSET((uint8 *)m_gprs_info.imsi,0,sizeof(m_gprs_info.imsi));
	cstart = strchr((char const *)stUart.Buf,'\n');
	if(cstart)
	{
		++cstart;
		while((*cstart >= '0') &&(*cstart <='9'))
			m_gprs_info.imsi[i++] = *cstart++;
	}
	else
		return   M2M_CMD_ERROR;
	return  M2M_CMD_SUCCESS;
}



/********************************************************
*func:waiting for GPRS attacted
*para:null
*return:success or error
*********************************************************/
en_M2MState m2m_findcgatt(void)
{
	return SendAT_CheckResponse("AT+CGATT?", "+CGATT: 1", 20, 20);
}

/*********************************************************
*func: set pdp
*para: pdpnum
*return:success or error
**********************************************************/
en_M2MState  m2m_setpdp(void)
{
	return SendAT_CheckResponse("AT+CGDCONT=0", "OK", 20, 1);
}
/*********************************************************
*func:set apn,china mobile
**********************************************************/
en_M2MState  m2m_setapn(void)
{
	return SendAT_CheckResponse("AT+CSTT=\"cmnet\"", "OK", 20, 1);
}

/**********************************************************
*active gprs connection
***********************************************************/
en_M2MState  m2m_actgprs(void)
{
	return SendAT_CheckResponse("AT+CIICR", "OK", 200, 1);
	
}

// 查询分配的IP地址
en_M2MState  m2m_getlocalip(void)
{
	return SendAT_CheckResponse("AT+CIFSR", ".", 10, 1);
}
/**********************************************************
*shut gprs pdp
***********************************************************/
en_M2MState m2m_closegprs(void)
{
	return SendAT_CheckResponse("AT+CIPSHUT", "OK", 200, 1);
}

/***********************************************************
*tcp connect
************************************************************/
en_M2MState m2m_tcpconnect(void)
{
	return SendAT_CheckResponse("AT+CIPSTART=\"TCP\",\"www.cortp.com\",\"7211\"",
								"CONNECT OK",
								300,
								1);
}
/************************************************************
*get tcp connection state
*************************************************************/
en_M2MState m2m_getconstate(void)
{
	return SendAT_CheckResponse("AT+CIPSTATUS", "STATE: CONNECT OK", 10, 1);// 查询TCP连接状态
}

/************************************************************
*mqtt config
*AT+MCONFIG=<clientid>,XXXX,$$$$
* 注意：
* XXXX 是用户名
* $$$$ 是密码
* 请开发者写真实的<clientid>、用户名和密码，不要
* 照抄，这三个参数加不加双引号都可以。如果用户名和
* 密码为空，则可以写成：
* AT+MCONFIG=<clientid>,””,””
*************************************************************/
en_M2MState m2m_Mconfig(void)
{
	char ATcmd[60] = {0};
	sprintf(ATcmd,"AT+MCONFIG=\"");
	strcat(ATcmd,m_gprs_info.imei);
	strcat(ATcmd,"\",\"20050001\",\"xj0ccb17\"");
	return SendAT_CheckResponse(ATcmd, "OK", 10, 1);// 查询TCP连接状态
}

/************************************************************
*mqtt ip port
* AT+MIPSTART=”ip 或域名”,”port”	
* 这里，请填上用户自己的mqtt服务器的IP地址或域名
* 地址，以及端口号
*************************************************************/
en_M2MState m2m_Mipstart(void)
{
	// return SendAT_CheckResponse("AT+MIPSTART=\"mqtt.yun-ran.com\",\"1883\"",
	// 							"CONNECT",
	// 							100,
	// 							2);
	return SendAT_CheckResponse("AT+MIPSTART=\"www.touchzhili.com\",\"6379\"",
								"CONNECT",
								100,
								2);
								
}

/************************************************************
*mqtt connect
* 建立mqtt会话
* 注：在MIPSTART 返回CONNECT OK 后才能发
* MCONNECT 命令，而且要立即发，否则会被服务器踢掉。
* 收到CONNACK OK后才能发布消息
*************************************************************/
en_M2MState m2m_Mconnect(void)
{
	return SendAT_CheckResponse("AT+MCONNECT=1,60",
								"CONNACK OK",
								300,
								1);
}

/************************************************************
*mqtt MQTT MSG SET
* 消息上报模式
* 0:主动上报到串口。有新订阅消息时，上报的 URC 为：
* +MSUB：<topic>,<len>,<message>
* 1:缓存模式。有新订阅消息时，上报的 URC 为：
* +MSUB：<store_addr>
* 然后用 AT+MQTTMSGGET 来读消息
*************************************************************/
en_M2MState	m2m_Mmsgset(void)	// 消息上报模式:缓存模式,用 AT+MQTTMSGGET 来读消息
{
	return SendAT_CheckResponse("AT+MQTTMSGSET=1", "OK", 10, 1);// 消息上报模式:0主动上报到串口,1缓存模式
}

//static char *mystrstr(char *srcstr, int strclen, char *substr, int sublen)
//{
//    char *p = 0;   
//    for (p = srcstr; p <= srcstr + strclen - sublen; p++)
//    {
//        if (memcmp (p, substr, sublen) == 0)
//            return (p);
//    }   
//    return (0);
//}

/************************************************************
*mqtt MQTT MSG GET
* 打印收到的所有的订阅消息
* 0:主动上报到串口。有新订阅消息时，上报的 URC 为：
* +MSUB：<topic>,<len>,<message>
* 1:缓存模式。有新订阅消息时，上报的 URC 为：
* +MSUB：<store_addr>
* 然后用 AT+MQTTMSGGET 来读消息
*************************************************************/
char *ptr;
char *subptr;
en_M2MState	m2m_Mmsgget(void)  // 查询缓存中的主题订阅消息
{
	uint8 stbuf[10]={0};
	uint8 i = 0;
	//uint8_t retry_cnt = 0;
	uint8_t retry2 = 0;
	en_M2MState status = M2M_CMD_ERROR;
	

GetMsg:
  	tmrDelay(1000);
	Send_AT_Command("AT+MQTTMSGGET");				//配置 PDP context
	
	if( Check_Response("OK",20) == M2M_CMD_SUCCESS)
	{
		subptr = strstr((const char *)(stUart.Buf), "+MSUB:");
//		subptr = mystrstr((char *)stUart.Buf,300,"+MSUB:",strlen("+MSUB:")-1);
		if(subptr != NULL)
		{
			if(strstr(subptr, "/service/AlarmEnable") != NULL)
			{
				ptr = strstr((const char *)subptr,"byte,");
				subptr = ptr;
				if(ptr != NULL)
				{
					ptr = strstr((const char *)subptr, "\"AlarmEnable\":");
					if(ptr != NULL)
					{
//						p++;
						ptr = ptr + strlen("\"AlarmEnable\":");
						// AlarmEnable = *p;
						if(*ptr == '0')	// 撤防命令
						{
							if(GetKGState() != 0)	// 读取限位开关（井盖）状态，只有当井盖打开状态才能开锁
							{
								LockOpen();
								macPIB.u16LockOpenTime = macPIB.u16LockOpenSet;	// 电子锁持续打开60s
							}
							
						}
						else
						{
							LockClose();
							macPIB.u16LockOpenTime = 0;	// 电子锁持续打开时间
						}
						
					}
					status = M2M_CMD_SUCCESS;
				}
			}
			if(strstr(subptr, "/service/Config") != NULL)
			{
				ptr = strstr((char const *)subptr,"byte,");
				subptr = ptr;
				if(ptr != NULL)
				{
					ptr = strstr((char const *)subptr, "\"HeartPeriod\":");
					if(ptr != NULL)
					{
					
//						ptr++;
						ptr = ptr + strlen("\"HeartPeriod\":");
						i = 0;
						while(*ptr != ',')
						{
							stbuf[i++] = *ptr++;
						}
						// HeartPeriod = *p;
						macPIB.u8HeartPeriod = comAtoi(stbuf);
					}
					ptr = strstr((char const *)subptr, "\"TempMax\":");
					if(ptr != NULL)
					{
//						ptr++;
						ptr = ptr + strlen("\"TempMax\":");
						// TempMax = *p;
						i = 0;
						while(*ptr != ',')
						{
							stbuf[i++] = *ptr++;
						}
						macPIB.u8TempMax = comAtoi(stbuf);
					}
					ptr = strstr((char const *)subptr, "\"OpenTime\":");
					if(ptr != NULL)
					{
//						ptr++;
						ptr = ptr + strlen("\"OpenTime\":");
						// TempMax = *p;
						i = 0;
						while(*ptr != '}')
						{
							stbuf[i++] = *ptr++;
						}
						macPIB.u16LockOpenSet = comAtoi(stbuf);
					}
					status = M2M_CMD_SUCCESS;
				}
			}
			if(status == M2M_CMD_SUCCESS)
				return M2M_CMD_SUCCESS;	
		}
		retry2= 0;
		
//		retry_cnt++;
//		if(retry_cnt>=5)
			return M2M_CMD_ERROR;
		goto GetMsg;
	}
	else
	{
//		tmrDelay(1000);
		retry2++;
		if(retry2 >= 5) 
			return M2M_CMD_ERROR;
		else
			goto GetMsg;
	}
}
/************************************************************
*mqtt Subscribe
* mqtt订阅主题
*************************************************************/
en_M2MState m2m_Msub(void)
{


	// char ATcmd[60] = {0};
	// sprintf(ATcmd,"AT+MSUB=\"/WSN_BJ09/");
	// strcat(ATcmd,punStoreInfo->id);
	// //+ MQTT通配符
	// strcat(ATcmd,"/service/+\",0");
	// return SendAT_CheckResponse(ATcmd,//"AT+MSUB=\"/WSN_BJ09/12346666/service/+\",0",
	// 							"SUBACK",
	// 							10,
	// 							1);
	unsigned char rtstatus=M2M_CMD_ERROR;
	//主题Config
	char ATcmd[60] = {0};
	sprintf(ATcmd,"AT+MSUB=\"/WSN_BJ09/");
	strcat(ATcmd,punStoreInfo->id);
	//+ MQTT通配符
	strcat(ATcmd,"/service/Config\",0");
	rtstatus= SendAT_CheckResponse(ATcmd,//"AT+MSUB=\"/WSN_BJ09/12346666/service/+\",0",
								"SUBACK",
								10,
								1);
  //主题AlarmEnable
	sprintf(ATcmd,"AT+MSUB=\"/WSN_BJ09/");
	strcat(ATcmd,punStoreInfo->id);
	//+ MQTT通配符
	strcat(ATcmd,"/service/AlarmEnable\",0");
	rtstatus= SendAT_CheckResponse(ATcmd,//"AT+MSUB=\"/WSN_BJ09/12346666/service/+\",0",
								"SUBACK",
								10,
								1);
	return rtstatus;
}

/************************************************************
*mqtt Publish
* mqtt发布主题
*************************************************************/
en_M2MState m2m_Mpub(const char *msg)
{
	return SendAT_CheckResponse(msg,//"AT+MPUB=\"/WSN_BJ/12346666/event/AlarmReport\",0,0,\"safafaf\"",
								"OK",
								10,
								1);
}

//__no_init st_AES stAES; 
//static void AES_Send(uint8 *pbuf,uint8 u8Len)
//{
//	if((stAES.u8Count + u8Len) >= 40)
//		return;
//	
//	comMEMCPY(&stAES.Buf[stAES.u8Count],pbuf,u8Len);
//	stAES.u8Count += u8Len;
//	while(stAES.u8Count >= 16)
//	{
//		/*第一层加密*/
//		AES_init_ctx(&m_aes, m_randkey);
//		AES_ECB_encrypt(&m_aes,stAES.Buf);  
//		
//		/*第二层加密*/
//		AES_init_ctx(&m_aes, m_key);
//		AES_ECB_encrypt(&m_aes,stAES.Buf);
//		
//		// 发送加密后的数据 16字节
//		Uart_SendData(stAES.Buf,16);
//		
//		// 发送后清除已加密发送数据
//		stAES.u8Count -= 16;
//		comMEMCPY(stAES.Buf,(const uint8 *)&stAES.Buf[16],stAES.u8Count);
//		
//	}
//	
//	WATCHDOG_FEED();
//}

/***********************************************************
* Function name:       MC20_TCP_Send
* Descriptions:        TCP发送数据
* input parameters:
* output parameters:
* Returned value:      无
*************************************************************/
//extern char RF_ResetNum;
//extern uint8 SensorWakeUpFlag;
//extern uint8 Sendnum;
//extern uint8 u8SensorState;  // 触发中断后立即保存干簧管的状态，用于延时去抖
en_M2MState m2m_TCP_Send(void)
{
	uint8  tmpbuf[22],i;
	uint16 u16len;
	uint8  u8addlen;
	
	return Check_Response("SEND OK",300);
}
/*************************************************************
*function : close gprs connection
**************************************************************/
//en_M2MState m2m_tcp_close(void)
//{
//	Send_AT_Command("AT+CIPCLOSE");
//	
//	return M2M_CMD_SUCCESS;
//		
//	//return SendAT_CheckResponse("AT+CIPCLOSE", "CLOSE OK", 100, 1);
//	
//	//	Send_AT_Command("AT+CIPCLOSE");
//	//	return Check_Response("CLOSE OK",100);
//}

/***************************************************************
*手动获取接收数据设置
****************************************************************/
//en_M2MState  m2m_manualget(void)
//{
//	return SendAT_CheckResponse("AT+CIPRXGET=2, 1460", "OK", 10, 1);
//	
//	//	Send_AT_Command("AT+CIPRXGET=2, 1460");	// 查询TCP连接状态
//	//	return Check_Response("OK",10);
//}

/*************************************************************
*m2m初始化
**************************************************************/
uint8_t m2m_init(void)
{
	uint8 i;
	
	m2m_pwron(ENABLE);   //开机
//	GPRSPowerOn(); // GPRS上电
	m2m_reset();
	tmrDelay(5000);

	comMEMSET((uint8_t*)&m_gprs_info, 0, sizeof(gprs_info_t));
	//init_fun_num = sizeof(m2m_init_list)/sizeof(m2m_init_list[0]);
	
	for(i = 0; i < M2M_TASK_NUM; i++)
	{
		if(m2m_task_list[i]() != M2M_CMD_SUCCESS)
			return i;
	}
	return 0xFF;
}

// 关闭回显
void m2m_echoclose(void)
{
	Send_AT_Command("ATE0&W\r\n");
	//Send_AT_Command("AT\r\n");
	Check_Response("OK",20);
}

// 关闭回显
void m2m_reset(void)
{
	SendAT_CheckResponse("AT+RESET", "OK", 10, 5);
//	Send_AT_Command("AT+RESET\r\n");
//	//Send_AT_Command("AT\r\n");
//	Check_Response("OK",20);
}
















