/**
 * AT_Proc.c ATָ�����
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#include "AT_Proc.h"
#include "hal.h"    //������Ҫ��ͷ�ļ�
#include "Uart.h"	
#include "cJSON.h"
#include <string.h>
#include "mystring.h"
#include "app.h"
#include "NetHandler.h"

static uint8_t CurrentRty;                                                 //�ط�����
static teATCmdNum ATRecCmdNum;                                             //����״̬
static tsTimeType TimeNB;
teNB_TaskStatus NB_TaskStatus;                                             //ATָ��ģʽ
teATCmdNum ATNextCmdNum;                                                   //����ָ��
teATCmdNum ATCurrentCmdNum;                                                //��ǰָ��

char NB_SendDataBuff[500];
char CSQ_buffer[3];
char topic_msg[44];
char IMEI_buf[22];
uint8_t IMEI_flag =0;                                                  //
uint8_t find_string_flag =0;
uint32_t First_open_flag = 0;
uint16_t ADC_val =0;
uint8_t NB_reset_time = 5;                                                 //��λ����
uint8_t AT_error_time = 50;                                                //������ʱ�䳢�Դ���
uint32_t RTC_Time = 30*60;                                                 //RTC����ʱ��
uint8_t my_sleep = 0;                                                      //���߿��أ�0Ϊ�رգ�*��ѧ�߲�Ҫ�޸�*��
uint8_t GPRS_InitOver_Flag = 0;
tsATCmds ATCmds[] =                                                        //ATָ���б�
{
	{"AT\r\n","OK",200,NO_REC,100},                                         //�رջ��Բ���
	{"ATE0\r\n","OK",200,NO_REC,100},                                       //�رջ��Բ���
	{"AT+CIPHEAD=0\r\n", "OK",200,NO_REC,2},                                //��������ʱ�Ƿ����� IP ͷ��ʾ 
	{"AT+CPIN?\r\n", "READY",100,NO_REC,10},                                //��ѯPIN����״̬
	
	{"AT+CREG=0\r\n", "OK",100,NO_REC,10},			                        //����ע����Ϣ
	{"AT+CREG?\r\n", "+CREG:",100,NO_REC,255},                              //�ȴ�פ�������Դ������ݵ����źŲ�ͬ

	{"AT+COPS=3,2\r\n", "OK",100,NO_REC,10},
	{"AT+COPS?\r\n", "+COPS:",100,NO_REC,10},	
	
	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //��ѯ�ź�
	{"AT+CGATT?\r\n","+CGATT: 1",200,NO_REC,255},  
	
	  
    {"AT+CIPSHUT\r\n","OK",200,NO_REC,2},                                   //�ر�GPRS(PDP������ȥ����)
    {"AT+CSTT=\"cmnet\"\r\n","OK",200,NO_REC,1},                            //����APN
    {"AT+CIICR\r\n","OK",200,NO_REC,10},                                    //����GPRS����
    {"AT+CIFSR\r\n",".",200,NO_REC,10},                                     // ��ȡ������ַ

    {"AT+MCONFIG=\"cellid\",\"\",\"\"\r\n","OK",200,NO_REC,1},  

    {"AT+MIPSTART=\"121.89.170.53\",\"1884\"\r\n","CONNECT OK",1000,NO_REC,2}, 

    {"AT+MCONNECT=1,60\r\n","CONNACK OK",300,NO_REC,255}, 
    {"AT+MQTTMSGSET=1\r\n","OK",10,NO_REC,1},  

    {"AT+MPUB=\"/WSN-LW/20010333/event/Data\",0,0,\"9999\"\r\n","OK",200,NO_REC,35}, 


	
 };

void str_replace(char * cp, int n, char * str)
{
	int lenofstr;
	int i;
	char * tmp;
	lenofstr = strlen(str); 
	//str3��str2�̣���ǰ�ƶ� 
	if(lenofstr < n)  
	{
		tmp = cp+n;
		while(*tmp)
		{
			*(tmp-(n-lenofstr)) = *tmp; //n-lenofstr���ƶ��ľ��� 
			tmp++;
		}
		*(tmp-(n-lenofstr)) = *tmp; //move '\0'	
	}
	else
	        //str3��str2���������ƶ�
		if(lenofstr > n)
		{
			tmp = cp;
			while(*tmp) tmp++;
			while(tmp>=cp+n)
			{
				*(tmp+(lenofstr-n)) = *tmp;
				tmp--;
			}   
		}
	strncpy(cp,str,lenofstr);
}
//str1Ϊ�������ݣ�str2Ϊ��Ҫ�滻���ַ���,str3Ϊ�ƻ��滻���ַ���,
char * jsondata_change_toGPRS(char * str1,char * str2,char *str3)
{
    int i,len,count=0;
    char c;
   	char *p;  	
	//��ʼ�����ַ���str2 
   	p = strstr(str1,str2);
   	while(p)
	{
		count++;
		//ÿ�ҵ�һ��str2������str3���滻 
		str_replace(p,strlen(str2),str3);
		p = p+strlen(str3);
		p = strstr(p,str2);
	}   	
    UART_Printf("�滻��ɣ��Ѿ��� %s �滻Ϊ %s��һ�� %d �� \r\n",str2,str3,count);
    return str1;
}
/*-------------------------------------------------*/
/*�������� ����ATָ��                      */
/*��  ����ATCmdNum��Ӧ��ATָ��                     */
/*����ֵ����                                       */    

void ATSend(teATCmdNum ATCmdNum)
{
	char MID[5];
    char MID_msg[5];       //
    char msg[100];         //�ַ�����Ϣ
    char hex_msg[200];     //hexת���Ժ����Ϣ
    char msg_body[300];    //�������ݰ���Ϣ
    char str2[5]="\"";
    char str3[5]="\\22"; 
    uint16_t msg_len = 0;
	
	clearUart();			//��մ�����Ϣ
	ATCmds[ATCmdNum].ATStatus = NO_REC;   //����Ϊ δ����
    ATRecCmdNum = ATCmdNum;
	
	if(ATCmdNum == AT_MPUB)
    {   
        //����AT��������֮����ж���
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff)); 
        strcat(NB_SendDataBuff,"AT+MSUB=\"/WSN-LW/");
        strcat(NB_SendDataBuff,Read_ID);
        strcat(NB_SendDataBuff,"/service/+\",0");
        //sprintf(NB_SendDataBuff,"%s\r\n",Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1)); 
        //strcat(NB_SendDataBuff,jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1),str2,str3));
        strcat(NB_SendDataBuff,"\r\n");  
       // UART_Printf("TestData : %s \r\n",jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1),str2,str3));
        Uart_SendStr(NB_SendDataBuff);
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //����
        ATRec();

    }
	else	//������ͨAT����
	{
		Uart_SendStr((const uint8_t *)ATCmds[ATCmdNum].ATSendStr);
	}
	//�򿪳�ʱ��ʱ��
    GetTime(&TimeNB,ATCmds[ATCmdNum].TimeOut);
}
//GPRS��MQTT���ݷ��ͺ���
void MQTT_GPRS_SendData()
{       
    char str2[5]="\""; 
    char str3[5]="\\22"; 
    uint8_t  Pack_Num = 0;  
	//Ĭ��120���ӻ�
	if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;}
	//�������
	if (MQTT_Resv_SensorNum >= 40)
	{
		/* code */	
		Pack_Num = MQTT_Resv_SensorNum / PacksSensorNum;
		if (MQTT_Resv_SensorNum % PacksSensorNum )
		{
			Pack_Num ++;
		}
		UART_Printf("Pack_Num is ---------------: %d\r\n" ,Pack_Num);
	}
	else if (MQTT_Resv_SensorNum < 40)
	{
		Pack_Num = 1; 
		Pack_Num_Last = MQTT_Resv_SensorNum;
	}
    for (uint8_t i = 1; i <= Pack_Num; i++)
    {	
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff)); 
        strcat(NB_SendDataBuff,"AT+MPUB=\"/WSN-LW/");
        strcat(NB_SendDataBuff,Read_ID);
        strcat(NB_SendDataBuff,"/event/Data\",0,0,\"");
        //sprintf(NB_SendDataBuff,"%s\r\n",Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1)); 
        if (MQTT_Resv_SensorNum >= 40)
        { 
            strcat(NB_SendDataBuff,jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,i),str2,str3));
        }
        else
        {
           strcat(NB_SendDataBuff,jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_CountLess40,i),str2,str3));
        } 
        strcat(NB_SendDataBuff,"\"\r\n");  
       // UART_Printf("TestData : %s \r\n",jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1),str2,str3));
        Uart_SendStr(NB_SendDataBuff);
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //����
    }
        
}
//GPRS��MQTT���ݷ��ͺ���
void MQTT_GPRS_Heartbeat()
{
        char str2[5]="\""; 
        char str3[5]="\\22"; 
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff)); 
        strcat(NB_SendDataBuff,"AT+MPUB=\"/WSN-LW/");
        strcat(NB_SendDataBuff,Read_ID);
        strcat(NB_SendDataBuff,"/event/Heartbeat\",0,0,\"");
        //sprintf(NB_SendDataBuff,"%s\r\n",Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1)); 
        strcat(NB_SendDataBuff,jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat,0),str2,str3));
        strcat(NB_SendDataBuff,"\"\r\n");  
       // UART_Printf("TestData : %s \r\n",jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1),str2,str3));
        Uart_SendStr(NB_SendDataBuff);
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //����
}

void ATRec(void)
{
    if(stUart.RxOK)  //�����յ�����
    {
		stUart.RxOK = 0;
		stUart.u8Count = 0;
		stUart.u8Timer = 0x7F;
        if(strstr((const char*)stUart.Buf,ATCmds[ATRecCmdNum].ATRecStr) != NULL) //���������ӦУ���
        {
            ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC;  //����Ϊ�ɹ�����
        }
        UART_Printf("@@Rev:%s\r\n",stUart.Buf); 
    }
}
//MQTT���ͽ������ݵ�����
void MQTT_GPRS_SendRscvDataCMD()
{
        //����AT��������֮����ж���
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff)); 
        strcat(NB_SendDataBuff,"AT+MQTTMSGGET\r\n");  
        Uart_SendStr(NB_SendDataBuff);
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //����
        MQTT_GPRS_ResvData();
}
//MQTT��������
void MQTT_GPRS_ResvData()
{ 
    uint8_t Count_Error = 0;
    uint8_t Topic_Num[60]; 
    uint8_t JsonData[200];
    if(stUart.RxOK)  //�����յ�����
    {
		stUart.RxOK = 0;
		stUart.u8Count = 0;
		stUart.u8Timer = 0x7F; 
        UART_Printf("@@Rev:%s\r\n",stUart.Buf); 
        //��������
        for (uint8_t i = 0; i < 180; i++)
        { 
            //Topic ���д�ӡ
            if (stUart.Buf[i] == '+' && stUart.Buf[i+1] == 'M' 
                && stUart.Buf[i+2] == 'S' && stUart.Buf[i+3] == 'U' 
                && stUart.Buf[i+4] == 'B' && stUart.Buf[i+5] == ':'
                && stUart.Buf[i+6] == ' ' && stUart.Buf[i+7] == '/'
                && stUart.Buf[i+8] == 'W' && stUart.Buf[i+9] == 'S'
                )
            {
                UART_Printf("Catch!!!!!!!!!!!!!!!!!!!!!!!!\r\n"); 
                for (uint8_t j = 0; j < 30; j++)
                {
                    Topic_Num[j] = stUart.Buf[j+(i+7)];
                } 
                //Topic ���д�ӡ
                UART_Printf("Topic : %s\r\n",Topic_Num); 
            }   
              //��ȡjson����    
            if (stUart.Buf[i] == '{' )
            {
                for (uint8_t k = 0; k < 100; k++)
                {
                    JsonData[k] = stUart.Buf[i+k];
                } 
                 //json���� ���д�ӡ
                UART_Printf("JsonData : %s\r\n",JsonData); 
                Unpack_json_MQTT_ResvData(JsonData);
            }
            //�������֮����ִ�����ô�ͻ����½������ݵĴ���������µĳ�ʼ��
            if ( (stUart.Buf[i] == 'E'&& stUart.Buf[i+1] == 'R'&& stUart.Buf[i+2] == 'R'&& stUart.Buf[i+3] == 'O'&& stUart.Buf[i+4] == 'R')
                ||(stUart.Buf[i] == 'E'&& stUart.Buf[i+1] == 'r'&& stUart.Buf[i+2] == 'r'&& stUart.Buf[i+3] == 'o'&& stUart.Buf[i+4] == 'r'))
            {
                Count_Error ++;
                UART_Printf("Count_Error is %d Now ! \r\n", Count_Error);
                if (Count_Error >= 5)
                {
                   Count_Error = 0;
                   NB_Init();
                   NB_Work();
                } 
            }    
        }  
        clearUart();  
    } 
}

void NB_POWER_ON(void)
{ 
	nrf_gpio_pin_set(PWRKEY);
	delay_ms(500); 
	nrf_gpio_pin_clear(PWRKEY);
	delay_ms(5000);
}
void NB_WAKE_UP(void)
{
//    HAL_GPIO_WritePin( NB_WIN_GPIO_Port, NB_WIN_Pin, GPIO_PIN_SET);
//    HAL_Delay(2500);
//    HAL_GPIO_WritePin( NB_WIN_GPIO_Port, NB_WIN_Pin, GPIO_PIN_RESET);
}

void NB_RESET(void)
{ 
	UART_Printf("@@ NB_RESET...\r\n");
	Uart_SendStr("AT+CFUN=1,1\r\n");
	NB_POWER_ON();
}

void NB_Init(void)
{
    UART_Printf("@@ Will Enable NB...\r\n");
	
    NB_RESET();
//    NB_POWER_ON();                     //����
    NB_TaskStatus = NB_SEND;           //���뷢��ģʽ
    ATCurrentCmdNum = AT;              //��ǰָ��
    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);        //��һ��ָ��
}

void COAPSendData(uint16_t Flag)
{
    ADC_val = Flag; 
    ATCurrentCmdNum = AT_CREG;
    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);        //��һ��ָ��
    NB_TaskStatus = NB_SEND;
}

//NB����״̬��
void NB_Work(void)
{
    switch(NB_TaskStatus)
    {
    case NB_IDIE:                                 //����״̬
        //printf("...\r\n");
        return;
    case NB_SEND:                                 //����״̬
        if(ATCurrentCmdNum != ATNextCmdNum)       //���ط�״̬
        {
            CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //�ط�������ֵ
        }
        ATSend(ATCurrentCmdNum);                  //��������
        NB_TaskStatus = NB_WAIT;
        return;
    case NB_WAIT:                                 //�ȴ�����״̬
        ATRec();                                   //��������
        Rec_WaitAT();                              //����ɹ�������ʧ������
        return;
    case NB_ACCESS:

        break;
    default:
        return;
    }
}

//NB����״̬��
//main����while(1)�е���
void NB_Task(void)
{
    while(1)
    {
        switch(NB_TaskStatus)
        {
        case NB_IDIE:                                 //����״̬
            //printf("...\r\n");
            return;
        case NB_SEND:                                 //����״̬
            if(ATCurrentCmdNum != ATNextCmdNum)       //���ط�״̬
            {
                CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //�ط�������ֵ
            }
            ATSend(ATCurrentCmdNum);                  //��������
            NB_TaskStatus = NB_WAIT;
            return;
        case NB_WAIT:                                 //�ȴ�����״̬
            ATRec();                                   //��������
            Rec_WaitAT();                              //����ɹ�������ʧ������
            return;
        case NB_ACCESS:

            break;
        default:
            return;
        }
    }
} 
void Rec_WaitAT(void)
{
    if(ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC) //�ɹ����ݰ�
    {
        switch(ATCurrentCmdNum)
        {
 
        case AT_CREG_1:
//            printf("@@CEREG--SUCCESS...\r\n");
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;
            break;
		case AT_COPS_1:
//            printf("@@CEREG--SUCCESS...\r\n");
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;
            break;
		case AT_CSQ:
            memset(CSQ_buffer, 0, sizeof(CSQ_buffer));     //��ѯ�ź�ֵ
            find_string_flag= Find_string((char *)stUart.Buf,": ",",",(char *)CSQ_buffer); //ȡ���ź�ֵ
            if(find_string_flag==0)                        //��ѯ�ź�ʧ��
            {
                ATCurrentCmdNum  = AT_CSQ;
                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            } 
            else // �ɹ�ȡ��CSQ
            {
 
				{   //�����δ�Ա����ͶԱ�����
                    ATCurrentCmdNum += 1;
                    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
                }
            }
            NB_TaskStatus = NB_SEND;
            break;  
       case AT_MCONFIG:
            
            UART_Printf("\r\n ����MQTT���������˺� %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;

            break;

       case AT_MIPSTART:
            
            UART_Printf("\r\n ��������ʱ�ĵ�ַ�Ͷ˿ں�  %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;

            break;

       case AT_MCONNECT:
            
            UART_Printf("\r\n ��ʼ����MQTT���� %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;

            break;  

       case AT_MQTTMSGSET:
            
            UART_Printf("\r\n ����MQTT���ϱ���Ϣģʽ %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;

            break;   
       case AT_MPUB:
            
            UART_Printf("\r\n MQTT�ϱ���ϢPUB %d\r\n",ATCurrentCmdNum);
            //�������˴�˵��GPRS��ʼ����ɡ�
            GPRS_InitOver_Flag = 1; 
            ATCurrentCmdNum  = AT_MQTTMSGSET;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
            NB_TaskStatus = NB_SEND;

            break;  

        default:
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            NB_TaskStatus = NB_SEND;
            break;
        }
    }
    else if(CompareTime(&TimeNB))           //��û�յ�Ԥ������  ���ҳ�ʱ
    { 
        switch(ATCurrentCmdNum)  //������ʱ���������
        {
 
        default:
            break;
        }
        ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
        if(CurrentRty > 0)  //�ط�
        {
            CurrentRty--; 
            ATNextCmdNum = ATCurrentCmdNum;  //��һ�� ���ǵ�ǰ����  ʵ���ط�Ч��
            NB_TaskStatus = NB_SEND;
        }
        else  //�ط���������
        {
            NB_RESET();
            ATCurrentCmdNum = AT;
            NB_TaskStatus = NB_SEND;
            return;
        }
    }
}

/*����ʱ�䶨ʱ��*/
void GetTime(tsTimeType *TimeType,uint32_t TimeInter)
{
    TimeType->TimeStart = GetSystemNowtime();    //��ȡϵͳ����ʱ��
    TimeType->TimeInter = TimeInter;      //���ʱ��
}
uint8_t  CompareTime(tsTimeType *TimeType)  //�Ƚ�ʱ��
{
    uint32_t nowtime = GetSystemNowtime();
    if(nowtime >= TimeType->TimeStart)  
        return ((nowtime-TimeType->TimeStart) >= TimeType->TimeInter);  //����1����ʱ
    else
        return ((nowtime+(0xFFFFFFFF-TimeType->TimeStart)) >= TimeType->TimeInter);  //����1����ʱ
    
    // return ((GetSystemNowtime()-TimeType->TimeStart) >= TimeType->TimeInter);  //����1����ʱ
}
/*********************************END OF FILE**********************************/

