#include <string.h>
//#include <stdlib.h>
#include "m2m.h"

#define M2M_TASK_NUM  12

void m2m_echoclose(void);
typedef en_M2MState (*m2m_task_function)(void);
gprs_info_t m_gprs_info;

const static m2m_task_function m2m_task_list[]=
{
	m2m_gprscheck,
	m2m_findsim,
	m2m_GetIMSI,
	m2m_CregSetForLocation,
	m2m_findcreg,
	m2m_Get_NetInfo,
	m2m_getcsq,
	m2m_findcgatt,
	m2m_closegprs,
	m2m_setapn,
	m2m_actgprs,
	m2m_getlocalip,
//	m2m_tcpconnect,
};

//void tmrDelay(uint16_t u16Delay) // ͨ����ʱ��ʵ�ֵľ�ȷms��ʱ
//{
//	uint16_t i,delayTicks = 2100;
//	while(delayTicks > 0)
//	{
//		for(i=0; i<u16Delay;i++);
//		delayTicks--;
//		nrf_drv_wdt_feed();
//	}
//}
/****************************************************************************
*func: send M2M command
*para: the command that will be execute
****************************************************************************/
static void Send_AT_Command(char* AT_Command)
{
	
	clearUart();	// ��մ�����Ϣ
	
	Uart_SendStr((const uint8_t *)AT_Command);
	Uart_SendStr((const uint8_t *)"\r\n");
	delay_ms(100);
	
	#if AT_Debug
	//		if(m_mode == WORK_MODE)
	//			printf_debug("%s",AT_Command);
	if(macPIB.u8NodeType != TEST_MODE)
	{
		USART1TxRxPortA_Status(ENABLE);	// ʹ�ܴ���PAӳ�䣬�ر�PC�˿�����	
		Uart_SendStr(AT_Command);
		Uart_SendStr("\r\n");
		USART1TxRxPortA_Status(DISABLE);	// �رմ���PAӳ�䣬ʹ��PC�˿�����
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
				USART1TxRxPortA_Status(ENABLE);	// ʹ�ܴ���PAӳ�䣬�ر�PC�˿�����	
				Uart_SendStr(stUart.Buf);
				Uart_SendStr("\r\n");
				USART1TxRxPortA_Status(DISABLE);	// �رմ���PAӳ�䣬ʹ��PC�˿�����
			}
			#endif
			return M2M_CMD_SUCCESS;
		}
		delay_ms(100);
	}
	#if AT_Debug
	if(macPIB.u8NodeType != TEST_MODE)
	{
		USART1TxRxPortA_Status(ENABLE);	// ʹ�ܴ���PAӳ�䣬�ر�PC�˿�����	
		Uart_SendStr(stUart.Buf);
		Uart_SendStr("\r\n");
		USART1TxRxPortA_Status(DISABLE);	// �رմ���PAӳ�䣬ʹ��PC�˿�����	
	}
	#endif
	
	return M2M_CMD_TIMEOUT;
}

// ����ATָ������Ӧ�𣬿�ָ���ط�����
// AT_Command:��Ҫ���͵�ATָ��
// Response:�ȴ���Ӧ������
// Response_Time_Out:�ȴ�Ӧ��ĳ�ʱʱ�䣬��λ100ms
// ReSendCount:�ȵ�Ӧ��ʱ֮���ط�AT����Ĵ���
en_M2MState SendAT_CheckResponse(char* AT_Command, const char * Response, unsigned int Response_Time_Out, uint8_t ReSendCount)
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
			delay_ms(100);
		}
		
	}
	
	return M2M_CMD_TIMEOUT;
}

/**************************************************************************
*����M2M��Դ����
*para: ENABLE:�򿪵�Դ��DISABLE���رյ�Դ
**************************************************************************/
void  m2m_pwron(unsigned char NewState)
{
//	if(NewState == ENABLE)
//	{
//		//USART1TxRxPortA_Status(ENABLE);	// ʹ�ܴ���PAӳ�䣬�ر�PC�˿�����	
//		//Uart_SendStr("POWER UP\r\n");
//		GPRSPowerOn(); // GPRS�ϵ�
//		//USART1TxRxPortA_Status(DISABLE);	// �رմ���PAӳ�䣬ʹ��PC�˿�����	
//	}
//	else
//	{
//		//USART1TxRxPortA_Status(ENABLE);	// ʹ�ܴ���PAӳ�䣬�ر�PC�˿�����	
//		//Uart_SendStr("POWER DOWN\r\n");
//		GPRSPowerOff(); // GPRS�ϵ�
//	}
}

// ���GPRSģ���Ƿ񿪻�
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

// ���SIM���Ƿ����
en_M2MState m2m_findsim(void)
{
	return SendAT_CheckResponse("AT+CPIN?", "READY", 10, 10);	// ���SIM���Ƿ�׼����
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
	Send_AT_Command("AT+CREG?");	//����Ƿ�ע�ᵽGSM
	if(Check_Response("OK",15) != M2M_CMD_SUCCESS)
	{
		delay_ms(1000);
		retry2++;
		if(retry2 >= 5) 
			return M2M_CMD_ERROR;
		else
			goto retry;
	}
	else
	{
		retry2= 0;
		delay_ms(1000);
		ptr = strchr((char*)stUart.Buf,',');
		if(ptr)
		{
			ptr++;
			if((*ptr == '1') ||(*ptr == '5'))
			{
				ptr = strchr((const char*)ptr,'"');  		//�õ���վλ����Ϣ
				ptr++;
				memcpy(m_gprs_info.lac,ptr,4);
				ptr = strchr((const char*)ptr,',');
				ptr++;
				ptr++;
				memcpy(m_gprs_info.cid,ptr,4);
				return M2M_CMD_SUCCESS;
			}
		}
		retry_cnt++;
		if(retry_cnt>=30)
			return M2M_CMD_ERROR;
		goto retry;
	}
}

// �õ������ƶ�����Ϣ
en_M2MState m2m_Get_NetInfo(void)
{
	char *p;
//	uint8_t retry_cnt = 0;
	char netinfo[6];
	int i;
	SendAT_CheckResponse("AT+COPS=3,2", "OK", 20, 10);
	SendAT_CheckResponse("AT+COPS?", "OK", 20, 10);	//��ѯ��Ӫ��
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
	memset(m_gprs_info.mcc,0,sizeof(m_gprs_info.mcc));
	memset(m_gprs_info.mic,0,sizeof(m_gprs_info.mic));
	memset(netinfo,0,sizeof(netinfo));
	p = strchr((const char *)stUart.Buf,'\"');
	if(p == NULL)
		return M2M_CMD_ERROR;
	p++;
	for(i = 0;*p != '\"';i++)
	{
		netinfo[i] = *p;
		p++;
	}
	memcpy(m_gprs_info.mcc,netinfo,3);
	memcpy(m_gprs_info.mic,(char*)(netinfo+3),strlen(netinfo)-3);
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
	Send_AT_Command("AT+CSQ");			  //���GSM�ź�����
	if( Check_Response("OK",20) == M2M_CMD_SUCCESS)
	{
		p = strchr((const char *)stUart.Buf,':');
		p++;
		memset(m_gprs_info.csq,0,sizeof(m_gprs_info.csq));
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
	int i = 0;
	int j = 0;
	Send_AT_Command("AT+GSN");
	if(Check_Response("OK", 10) != M2M_CMD_SUCCESS)
		return M2M_CMD_ERROR;
	
	memset(m_gprs_info.imei, 0, sizeof(m_gprs_info.imei));
	for(; i < strlen((const char *)stUart.Buf); i++)
	{
		if((stUart.Buf[i] >= '0') &&(stUart.Buf[i] <= '9'))
		{
			m_gprs_info.imei[j++] = stUart.Buf[i];
		}
	}
	return M2M_CMD_SUCCESS;
}
/*
@bief:�õ�SIM����IMSI
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
	memset(m_gprs_info.imsi,0,sizeof(m_gprs_info.imsi));
	cstart = strchr((const char *)stUart.Buf,'\n');
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
	return SendAT_CheckResponse("AT+CGATT?", "+CGATT: 1", 20, 20);	//��ѯ�Ƿ����ӵ�GPRS
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
	return SendAT_CheckResponse("AT+CIICR", "OK", 200, 1);	//����GPRS����
	
}

// ��ѯ�����IP��ַ
en_M2MState  m2m_getlocalip(void)
{
	return SendAT_CheckResponse("AT+CIFSR", ".", 10, 1);	// ��ȡ������ַ
}
/**********************************************************
*shut gprs pdp
***********************************************************/
en_M2MState m2m_closegprs(void)
{
	return SendAT_CheckResponse("AT+CIPSHUT", "OK", 100, 5);	//�ر�GPRS(PDP������ȥ����)
}

/***********************************************************
*tcp connect
************************************************************/
en_M2MState m2m_tcpconnect(void)
{
	return SendAT_CheckResponse("AT+CIPSTART=\"TCP\",\"gateway.cortp.com\",\"7611\"",
								"CONNECT OK",
								100,
								5);	//����TCP����
}
/************************************************************
*get tcp connection state
*************************************************************/
en_M2MState m2m_getconstate(void)
{
	return SendAT_CheckResponse("AT+CIPSTATUS", "STATE: CONNECT OK", 10, 1);// ��ѯTCP����״̬
}

////st_AES stAES; 
////static void AES_Send(uint8_t *pbuf,uint8_t u8Len)
////{
////	if((stAES.u8Count + u8Len) >= 40)
////		return;
////	
////	comMEMCPY(&stAES.Buf[stAES.u8Count],pbuf,u8Len);
////	stAES.u8Count += u8Len;
////	while(stAES.u8Count >= 16)
////	{
////		/*��һ�����*/
////		AES_init_ctx(&m_aes, m_randkey);
////		AES_ECB_encrypt(&m_aes,stAES.Buf);  
////		
////		/*�ڶ������*/
////		AES_init_ctx(&m_aes, m_key);
////		AES_ECB_encrypt(&m_aes,stAES.Buf);
////		
////		// ���ͼ��ܺ������ 16�ֽ�
////		Uart_SendData(stAES.Buf,16);
////		
////		// ���ͺ�����Ѽ��ܷ�������
////		stAES.u8Count -= 16;
////		comMEMCPY(stAES.Buf,(const uint8_t *)&stAES.Buf[16],stAES.u8Count);
////		
////	}
////	
////	WATCHDOG_FEED();
////}

/***********************************************************
* Function name:       MC20_TCP_Send
* Descriptions:        TCP��������
* input parameters:
* output parameters:
* Returned value:      ��
*************************************************************/
extern char RF_ResetNum;
extern uint8_t SensorWakeUpFlag;
extern uint8_t Sendnum;
en_M2MState m2m_TCP_Send(void)
{
	uint8_t  tmpbuf[22],i=0;
	uint16_t u16len;
	uint8_t  u8addlen;
	
	uint8_t tag_cnt    = 0;
	DOUBLE_LINK_NODE *pNode;
	uint32_t cnt       = 0;
	
	clearUart();	// ��մ�����Ϣ
	
	tag_cnt = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
	if(tag_cnt >= 70) 
		tag_cnt = 70;
	u16len = (62 + tag_cnt*18);
	
	tmpbuf[0] = u16len/1000 + 0x30;
	tmpbuf[1] = u16len%1000/100 + 0x30;
	tmpbuf[2] = u16len%100/10 + 0x30;
	tmpbuf[3] = u16len%10 + 0x30;
	
	Uart_SendStr((const uint8_t *)"AT+CIPSEND=");	//���뷢��״̬
	Uart_SendData(tmpbuf,4);
	Uart_SendStr((const uint8_t *)"\r\n");
	
	if(Check_Response(">",10) != M2M_CMD_SUCCESS)
		return M2M_CMD_ERROR;	
	
	if(tag_cnt != 0) 
		Uart_SendStr((const uint8_t *)"$R,P,D,");
	else
		Uart_SendStr((const uint8_t *)"$R,P,H,");
	
	Uart_SendStr((const uint8_t *)m_gprs_info.mcc);
	Uart_SendStr((const uint8_t *)",");
	Uart_SendStr((const uint8_t *)m_gprs_info.mic);
	Uart_SendStr((const uint8_t *)",");
	Uart_SendStr((const uint8_t *)m_gprs_info.lac);	// m_gprs_info.lac
	Uart_SendStr((const uint8_t *)",");
	Uart_SendStr((const uint8_t *)m_gprs_info.cid);	// m_gprs_info.cid
	Uart_SendStr((const uint8_t *)",0,0,");
	Uart_SendStr((const uint8_t *)m_gprs_info.imsi);
	Uart_SendStr((const uint8_t *)",");
	Uart_SendData((const uint8_t *)Read_ID,16);
	
//	Uart_SendStr("$R,I,D,460,00,1806,3201,0,0,460040883305431,990140123456700301000\r\n");
	DISABLE_GLOBAL_INTERRUPT();
	pNode = (&RADIO_DATA_LIST_HEAD)->next;
	while(i++ < tag_cnt)
	{	
		hex2strid((char *)tmpbuf,pNode->data,(NET_RADIO_PACKET_LEN-2)<<1);
		cnt = (NET_RADIO_PACKET_LEN-2)<<1;
		
		if(i == 1)
			Uart_SendStr((const uint8_t *)",");
		else
			Uart_SendStr((const uint8_t *)"#");
		if(pNode->data[6] == 0)
		{
			memcpy(&tmpbuf[cnt],"00",2);
			cnt += 2;
		}
		else
		{
			memcpy(&tmpbuf[cnt],"10",2);
			cnt += 2;
		}
		tmpbuf[cnt++] = pNode->data[7]/100 + 0x30;
		tmpbuf[cnt++] = pNode->data[7]%100/10 + 0x30;
		tmpbuf[cnt++] = pNode->data[7]%10 + 0x30;
		
		list_del(pNode);
		pNode = (&RADIO_DATA_LIST_HEAD)->next;
		
		Uart_SendData(tmpbuf,17);
	}
	ENABLE_GLOBAL_INTERRUPT();
	Uart_SendData((const uint8_t *)"\r\n",2);
////	//comMEMSET(cmd_buf,0,sizeof(cmd_buf));
////	// �������ݳ����ַ�
////	if(u8TagNum > TAG_NUM_MAX)
////		u8TagNum = 0; 
////	if(Sendnum == 1)
////	{
////		u16len = (67 + u8TagNum*22);
////	}
////	else
////	{
////		u16len = (67 + (u8TagNum-50)*22);
////	}
//////	//	u16len = (84 + u8TagNum*22);
//////	u16len = (67 + u8TagNum*22);
////	if(u16len%16 != 0)
////		u8addlen = (16-u16len%16);
////	else
////		u8addlen = 0;
////	u16len += u8addlen;	// ��ȫ��������Ϊ16��������
////	u16len += 16;	// ������Կ
////	tmpbuf[0] = u16len/1000 + 0x30;
////	tmpbuf[1] = u16len%1000/100 + 0x30;
////	tmpbuf[2] = u16len%100/10 + 0x30;
////	tmpbuf[3] = u16len%10 + 0x30;
////	
////	clearUart();	// ��մ�����Ϣ
////	
////	Uart_SendStr("AT+CIPSEND=");
////	Uart_SendData(tmpbuf,4);
////	Uart_SendStr("\r\n");
////	if(Check_Response(">",10) != M2M_CMD_SUCCESS)
////		return M2M_CMD_ERROR;	
////	
////	
////	/*��̬��Կ����*/
////	for(i =0; i < sizeof(m_randkey); i++)
////	{
////		m_randkey[i] = comRand(0xFF);
////	}
////	
////	stAES.u8Count = 0;
////	if(macPIB.u8PanDianFlag == 1)	// �����̵����ݰ�
////	{
////		//		Uart_SendStr("$R,I,V,460,00,1806,3201,30.751163,120.647502,460040883305431,990140123456700501000");
////		//		Uart_SendStr("$R,I,V,");
////		AES_Send("$R,I,V,",strlen("$R,I,V,"));
////		
////		//		sprintf(cur_point,"$R,I,V,%s,%s,%s,%s,%s,%s,%s,",m_gprs_info.mcc,m_gprs_info.mic,m_gprs_info.lac,m_gprs_info.cid,"0","0",m_gprs_info.imsi);
////	}
////	else
////	{
////		//		Uart_SendStr("$R,I,D,460,00,1806,3201,30.751163,120.647502,460040883305431,990140123456700501000");
////		//		Uart_SendStr("$R,I,D,");
////		AES_Send("$R,I,D,",strlen("$R,I,D,"));
////	}
////	AES_Send(m_gprs_info.mcc,strlen(m_gprs_info.mcc));
////	AES_Send(",",1);
////	AES_Send(m_gprs_info.mic,strlen(m_gprs_info.mic));
////	AES_Send(",",1);
////	AES_Send(m_gprs_info.lac,strlen(m_gprs_info.lac));	// m_gprs_info.lac
////	AES_Send(",",1);
////	AES_Send(m_gprs_info.cid,strlen(m_gprs_info.cid));	// m_gprs_info.cid
////	AES_Send(",0,0,",5);
////	AES_Send(m_gprs_info.imsi,strlen(m_gprs_info.imsi));
////	AES_Send(",",1);
////	AES_Send(punStoreInfo->id,DEVICE_ID_LEN);
////	if (PWR_GetFlagStatus(PWR_FLAG_PVDOF) != RESET)	// ���Ƿѹ
////	{
////		AES_Send("1",1);
////	}
////	else
////	{
////		AES_Send("0",1);
////	}
//////	if(GetSensorState() == 0)	// �ɻɹܶϿ������ӿ�
//////	{
//////		AES_Send("100",3);
//////	}
//////	else
//////	{
//////		AES_Send("000",3);
//////	}
//////	if(GetSensorState() == 0)	// �ɻɹܶϿ������ӿ�
//////	{
//////		AES_Send("10",2);
//////	}
//////	else
//////	{
//////		AES_Send("00",2);
//////	}
////	/********TEST******/
////	if(GetSensorState() == 0)	// �ɻɹܶϿ������ӿ�
////	{
////		AES_Send("1",1);
////	}
////	else
////	{
////		AES_Send("0",1);
////	}
////	if(SensorWakeUpFlag == 1)	// �ɻɹܴ���
////	{
////		AES_Send("1",1);
////	}
////	else
////	{
////		AES_Send("0",1);
////	}
////	/********TEST******/
////	AES_Send(&RF_ResetNum,1);// ��ƵоƬ��λ����
////	AES_Send(&punStoreInfo->reset_num,1);// ��λ����

////	for(i =50*(Sendnum-1); i < u8TagNum; i++)
////	{
////		hex2strid(tmpbuf, stTag[i].ID, 20);
////		AES_Send("#",1);	
////		AES_Send(tmpbuf,20);
////		AES_Send("0",1);
////	}
////	AES_Send("\r\n",2);	
////	if(u8addlen != 0)
////	{
////		for(i=0; i < u8addlen; i++)
////			AES_Send("0",1);
////	}
////	/*�ڶ������*/
////	AES_init_ctx(&m_aes, m_key);
////	AES_ECB_encrypt(&m_aes,m_randkey);	//��һ����Կ���м��ܲ�����
////	
////	// ���ͼ��ܺ������ 16�ֽ�
////	Uart_SendData(m_randkey,16);
////	if(Sendnum == 1)
////	{
////		// ���ݷ��ͺ�
////		macPIB.u8PanDianFlag = 0; 	// ����̵�״̬
////		u8TagNum = 0; 	// ��մӽڵ���Ϣ
////	}
////	else
////	{
////		u8TagNum = 50;
////	}
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
*�ֶ���ȡ������������
****************************************************************/
//en_M2MState  m2m_manualget(void)
//{
//	return SendAT_CheckResponse("AT+CIPRXGET=2, 1460", "OK", 10, 1);
//	
//	//	Send_AT_Command("AT+CIPRXGET=2, 1460");	// ��ѯTCP����״̬
//	//	return Check_Response("OK",10);
//}

/*************************************************************
*m2m��ʼ��
**************************************************************/
uint8_t m2m_init(void)
{
	uint8_t i;
	
//	GPRSPowerOn(); // GPRS�ϵ�
//	
	m2m_reset();
	nrf_gpio_pin_set(PWRKEY);
	delay_ms(500); 
	nrf_gpio_pin_clear(PWRKEY);
	delay_ms(5000);
	memset((uint8_t*)&m_gprs_info, 0, sizeof(gprs_info_t));
	//init_fun_num = sizeof(m2m_init_list)/sizeof(m2m_init_list[0]);
	
	for(i = 0; i < M2M_TASK_NUM; i++)
	{
		if(m2m_task_list[i]() != M2M_CMD_SUCCESS)
			return i;
	}
	return 0xFF;
}

// �رջ���
void m2m_echoclose(void)
{
	Send_AT_Command("ATE0&W\r\n");
	//Send_AT_Command("AT\r\n");
	Check_Response("OK",20);
}

// ��λ2Gģ��
void  m2m_reset(void)
{
	SendAT_CheckResponse("AT+CFUN=1,1", "OK", 20, 1);	//ģ�鸴λ
}

















