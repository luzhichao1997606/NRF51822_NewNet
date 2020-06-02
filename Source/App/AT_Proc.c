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

#include <string.h>
#include "mystring.h"

#include "NetHandler.h"

static uint8_t CurrentRty;                                                 //�ط�����
static teATCmdNum ATRecCmdNum;                                             //����״̬
static tsTimeType TimeNB;
teNB_TaskStatus NB_TaskStatus;                                             //ATָ��ģʽ
teATCmdNum ATNextCmdNum;                                                   //����ָ��
teATCmdNum ATCurrentCmdNum;                                                //��ǰָ��

char NB_SendDataBuff[350];
char CSQ_buffer[3];
char topic_msg[44];
char IMEI_buf[22];
uint8_t IMEI_flag =0;
uint8_t find_string_flag =0;
uint32_t First_open_flag = 0;
uint16_t ADC_val =0;
uint8_t NB_reset_time = 5;                                                 //��λ����
uint8_t AT_error_time = 50;                                                //������ʱ�䳢�Դ���
uint32_t RTC_Time = 30*60;                                                 //RTC����ʱ��
uint8_t my_sleep = 0;                                                      //���߿��أ�0Ϊ�رգ�*��ѧ�߲�Ҫ�޸�*��

tsATCmds ATCmds[] =                                                        //ATָ���б�
{
	{"AT\r\n","OK",200,NO_REC,100},                                     	   //�رջ��Բ���
	{"ATE0\r\n","OK",200,NO_REC,100},                                   	   //�رջ��Բ���
	{"AT+CIPHEAD=0\r\n", "OK",200,NO_REC,2},                              	   //��������ʱ�Ƿ����� IP ͷ��ʾ 
	{"AT+CPIN?\r\n", "READY",100,NO_REC,10}, 									//��ѯPIN����״̬
	
	{"AT+CREG=0\r\n", "OK",100,NO_REC,10},			//����ע����Ϣ
	{"AT+CREG?\r\n", "+CREG:",100,NO_REC,255},	 //�ȴ�פ�������Դ������ݵ����źŲ�ͬ

	{"AT+COPS=3,2\r\n", "OK",100,NO_REC,10},
	{"AT+COPS?\r\n", "+COPS:",100,NO_REC,10},	
	
	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //��ѯ�ź�
	{"AT+CGATT?\r\n","+CGATT: 1",200,NO_REC,255},  
	
	{"AT+CIPSHUT\r\n","OK",200,NO_REC,1},							//�ر�GPRS(PDP������ȥ����)
	{"AT+CSTT=\"cmnet\"\r\n","OK",200,NO_REC,1},   //����APN
	{"AT+CIICR\r\n","OK",200,NO_REC,1},							//����GPRS����
	{"AT+CIFSR\r\n",".",200,NO_REC,1},						// ��ȡ������ַ
//	{"AT+CIPCSGP?\r\n","+CIPCSGP: 1,\"cmnet\"",200,NO_REC,1},
	{"AT+CIPSTART=\"TCP\",\"www.cortp.com\",\"7211\"\r\n","CONNECT OK",3000,NO_REC,1},
	
	{"AT+CIPSEND=","CLOSED",6000,NO_REC,1},

	/***********/
	
////	{"AT\r\n","OK",200,NO_REC,100},                                     	   //�رջ��Բ���
////	{"ATE0\r\n","OK",200,NO_REC,100},                                   	   //�رջ��Բ���
////	{"AT+GMR\r\n","ME3616",200,NO_REC,3},                                 	 //��ѯ�汾
////	{"ATI\r\n","OK",200,NO_REC,3},                                        	 //��ѯģ����ϢIMEI
////	{"AT+ZCONTLED=1\r\n","OK",1000,NO_REC,2},                             	 //���ơ����ߵĻ���Ҫ�ص�
////	
////	{"AT+ZSLR?\r\n","+ZSLR:1",2000,NO_REC,2},                            	  //�Ƿ�����������ģʽ
////	{"AT+ZSLR=1\r\n","OK",1000,NO_REC,2},                               	   //��������ģʽ
////	{"AT+CPSMS?\r\n","+CPSM",2000,NO_REC,2},                             	  //�Ƿ���������ʱ��
////	{"AT+CPSMS=1,,,\"00011000\",\"00001010\"\r\n","OK",2000,NO_REC,2},   	  //��������ʱ�� 20S
////	
////	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //��ѯ�ź�
////	{"AT+CEREG?\r\n","+CEREG: 0,1",200,NO_REC,255},                        //�ȴ�פ�������Դ������ݵ����źŲ�ͬ
////	
////	{"AT+ECOAPNEW=106.13.150.28,5683,1\r\n","+ECOAPNEW:",3000,NO_REC,3},    //�½�COAP����
////	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //��ѯ�ź�ֵ
////	{"AT+CCLK?\r\n","GMT",200,NO_REC,2},                                   //��ѯ����ʱ��
////	{"AT+ECOAPSEND=1,","COAPNMI:",10000,NO_REC,5},                         //����COAP���ݰ�  ��������ʱ���ź��й�
////	{"AT+ECOAPDEL=1\r\n","OK",2000,NO_REC,3},                              //�Ͽ�COAP����
////	{"AT+ZRST","OK",2000,NO_REC,3},                                        //��λģ�飨������Ӳ����λ��
};

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
    uint16_t msg_len = 0;
	
	clearUart();			//��մ�����Ϣ
	ATCmds[ATCmdNum].ATStatus = NO_REC;   //����Ϊ δ����
    ATRecCmdNum = ATCmdNum;
	
	if(ATCmdNum == AT_TCPSEND)
    {
		// memset(hex_msg,0x00,sizeof(hex_msg));
        // memset(MID,0x00,sizeof(MID));
        // sprintf(MID,"%d",rand()%90+10);  //�������ΪMIDʹ��  ֻ������λ����������Ϊ 10-99
        // memset(MID_msg,0x00,sizeof(MID_msg));
        // str2hex(MID,MID_msg);
        // memset(topic_msg,0x00,sizeof(topic_msg));
        // str2hex(IMEI_buf+9,topic_msg);
        // sprintf(msg,"{\"value\":%d,\"CSQ\":%s}",ADC_val,CSQ_buffer);
        // UART_Printf("##:SEND_buf:%s,%s\r\n",msg,topic_msg);
        // str2hex(msg,hex_msg);
        // memset(msg_body,0x00,sizeof(msg_body));
        // sprintf(msg_body,"4203%s5562b46d71747409575a2f%s49633d636c69656e743205753d746f6d08703d736563726574ff%s",MID_msg,topic_msg,hex_msg);
        // msg_len = strlen(msg_body)/2;
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff));
//        sprintf(NB_SendDataBuff,"%s%d,%s\r\n",ATCmds[ATCmdNum].ATSendStr,msg_len,msg_body);
        sprintf(NB_SendDataBuff,"%s%d\r\n",ATCmds[ATCmdNum].ATSendStr,67);
		// Uart_SendStr((const uint8_t *)ATCmds[ATCmdNum].ATSendStr);
        Uart_SendStr((const uint8_t *)NB_SendDataBuff);
        Uart_SendStr("$R,I,D,460,00,1806,3201,0,0,460040883305431,990140123456700301010\r\n");
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //����
    }
	else	//������ͨAT����
	{
		Uart_SendStr((const uint8_t *)ATCmds[ATCmdNum].ATSendStr);
	}
	//�򿪳�ʱ��ʱ��
    GetTime(&TimeNB,ATCmds[ATCmdNum].TimeOut);
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
//        Usart2type.UsartRecFlag = 0;  //��ս��ձ�־λ
//        Usart2type.UsartRecLen = 0;   //��ս��ճ���
		
//		clearUart();			//��մ�����Ϣ
    }
}


void NB_POWER_ON(void)
{
//    HAL_GPIO_WritePin( NB_PWK_GPIO_Port, NB_PWK_Pin, GPIO_PIN_RESET);
//    HAL_Delay(100);
//    HAL_GPIO_WritePin( NB_PWK_GPIO_Port, NB_PWK_Pin, GPIO_PIN_SET);
//    HAL_Delay(2500);
//    HAL_GPIO_WritePin( NB_PWK_GPIO_Port, NB_PWK_Pin, GPIO_PIN_RESET);
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
//    HAL_GPIO_WritePin(NB_RST_GPIO_Port, NB_RST_Pin, GPIO_PIN_SET);
//    HAL_Delay(2500);
//    HAL_GPIO_WritePin(NB_RST_GPIO_Port, NB_RST_Pin, GPIO_PIN_RESET);
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
//    NB_WAKE_UP();                      //������״̬�»���
//    printf("@ wakeup NB ...\r\n");
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
////			case AT_I:
////				if(IMEI_flag==0)  //��δʶ��IMEI
////				{
////				 memset(IMEI_buf, 0, sizeof(IMEI_buf));     //��ѯ�ź�ֵ
////////			     find_string_flag= Find_string((char *)Usart2type.Usart2RecBuffer,"IMEI: ","\r\n",(char *)IMEI_buf); //ȡ���ź�ֵ
////				if(find_string_flag) IMEI_flag =1;
////				}
////				ATCurrentCmdNum += 1;
////                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);     //����ݽ�
////               NB_TaskStatus = NB_SEND;
////				break;
////        case AT_ZCONTLED:                 //��ƻ��߹ص����
////            if(my_sleep ==1 )             //ʹ��PSM����
////            {
////                ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                NB_TaskStatus = NB_SEND;
////            } else                       //δʹ��PSM���ܣ������������ã�
////            {
////                ATCurrentCmdNum = AT_CEREG;
////                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                NB_TaskStatus = NB_SEND;
////            }
////            break;
////        case AT_ZSLR:                         //����ģʽ�Ѿ�����
////            ATCurrentCmdNum = AT_CEREG;       //��ʼפ��
////            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
////            NB_TaskStatus = NB_SEND;
////            break;
////        case AT_CPSMS_1:
////            ATCurrentCmdNum = AT_ZRST;       //��λģ�飨�ɻ���Ӳ����λ��
////            ATNextCmdNum = AT;               //��ͷ��ʼ����
////            NB_TaskStatus = NB_SEND;
////            break;
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
            } else // �ɹ�ȡ��CSQ
            {
////                if(Cat_Time.flag == 1)  //�������Ա���ɣ����ٶԱ�
////                {
////                    ATCurrentCmdNum += 2;
////                    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                } 
////				else 
				{                //�����δ�Ա����ͶԱ�����
                    ATCurrentCmdNum += 1;
                    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
                }
            }
            NB_TaskStatus = NB_SEND;
            break;
//		case AT_CIFSR:
////            printf("@@CEREG--SUCCESS...\r\n");
//            ATCurrentCmdNum += 1;
//            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //����ݽ�
//            NB_TaskStatus = NB_SEND;
//            break;
        case AT_TCPSEND:
            CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //�ɹ�һ�ξ͸�ֵ
            NB_reset_time =5;                              //��λ������ֵ
//            ATCurrentCmdNum += 1;
			ATCurrentCmdNum = AT_CIPSTART;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);     //����ݽ�
            NB_TaskStatus = NB_SEND;
            break;
        case AT_TCPDEL:                                  //�Ͽ�COAP����
////            memset(CSQ_buffer,0x00,sizeof(CSQ_buffer));    //����ź�����buf
////            work_flag =0;                                  //��ն�ʱ����ʶ
            NB_TaskStatus = NB_IDIE;                       //����Ϊ����̬���ȴ�NB���ߣ�
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
//        printf("@@REV--TimeOut:%s\r\n",stUart.Buf);
        switch(ATCurrentCmdNum)  //������ʱ���������
        {
////        case AT_ZSLR:
////            ATCurrentCmdNum  = AT_ZSLR_1;
////            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////            NB_TaskStatus = NB_SEND;
////            break;
        default:
            break;
        }
        ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
        if(CurrentRty > 0)  //�ط�
        {
            CurrentRty--;
//            printf("@@ now:%d,trytime:%d\r\n",ATCurrentCmdNum,CurrentRty);
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

