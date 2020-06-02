/**
 * AT_Proc.c AT指令处理函数
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#include "AT_Proc.h"
#include "hal.h"    //包含需要的头文件
#include "Uart.h"	

#include <string.h>
#include "mystring.h"

#include "NetHandler.h"

static uint8_t CurrentRty;                                                 //重发次数
static teATCmdNum ATRecCmdNum;                                             //接收状态
static tsTimeType TimeNB;
teNB_TaskStatus NB_TaskStatus;                                             //AT指令模式
teATCmdNum ATNextCmdNum;                                                   //下条指令
teATCmdNum ATCurrentCmdNum;                                                //当前指令

char NB_SendDataBuff[350];
char CSQ_buffer[3];
char topic_msg[44];
char IMEI_buf[22];
uint8_t IMEI_flag =0;
uint8_t find_string_flag =0;
uint32_t First_open_flag = 0;
uint16_t ADC_val =0;
uint8_t NB_reset_time = 5;                                                 //复位次数
uint8_t AT_error_time = 50;                                                //网络上时间尝试次数
uint32_t RTC_Time = 30*60;                                                 //RTC唤醒时间
uint8_t my_sleep = 0;                                                      //休眠开关（0为关闭！*初学者不要修改*）

tsATCmds ATCmds[] =                                                        //AT指令列表
{
	{"AT\r\n","OK",200,NO_REC,100},                                     	   //关闭回显测试
	{"ATE0\r\n","OK",200,NO_REC,100},                                   	   //关闭回显测试
	{"AT+CIPHEAD=0\r\n", "OK",200,NO_REC,2},                              	   //接收数据时是否增加 IP 头提示 
	{"AT+CPIN?\r\n", "READY",100,NO_REC,10}, 									//查询PIN码锁状态
	
	{"AT+CREG=0\r\n", "OK",100,NO_REC,10},			//网络注册信息
	{"AT+CREG?\r\n", "+CREG:",100,NO_REC,255},	 //等待驻网，尝试次数根据当地信号不同

	{"AT+COPS=3,2\r\n", "OK",100,NO_REC,10},
	{"AT+COPS?\r\n", "+COPS:",100,NO_REC,10},	
	
	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //查询信号
	{"AT+CGATT?\r\n","+CGATT: 1",200,NO_REC,255},  
	
	{"AT+CIPSHUT\r\n","OK",200,NO_REC,1},							//关闭GPRS(PDP上下文去激活)
	{"AT+CSTT=\"cmnet\"\r\n","OK",200,NO_REC,1},   //设置APN
	{"AT+CIICR\r\n","OK",200,NO_REC,1},							//激活GPRS连接
	{"AT+CIFSR\r\n",".",200,NO_REC,1},						// 获取本机地址
//	{"AT+CIPCSGP?\r\n","+CIPCSGP: 1,\"cmnet\"",200,NO_REC,1},
	{"AT+CIPSTART=\"TCP\",\"www.cortp.com\",\"7211\"\r\n","CONNECT OK",3000,NO_REC,1},
	
	{"AT+CIPSEND=","CLOSED",6000,NO_REC,1},

	/***********/
	
////	{"AT\r\n","OK",200,NO_REC,100},                                     	   //关闭回显测试
////	{"ATE0\r\n","OK",200,NO_REC,100},                                   	   //关闭回显测试
////	{"AT+GMR\r\n","ME3616",200,NO_REC,3},                                 	 //问询版本
////	{"ATI\r\n","OK",200,NO_REC,3},                                        	 //问询模组信息IMEI
////	{"AT+ZCONTLED=1\r\n","OK",1000,NO_REC,2},                             	 //开灯、休眠的话需要关灯
////	
////	{"AT+ZSLR?\r\n","+ZSLR:1",2000,NO_REC,2},                            	  //是否已配置休眠模式
////	{"AT+ZSLR=1\r\n","OK",1000,NO_REC,2},                               	   //配置休眠模式
////	{"AT+CPSMS?\r\n","+CPSM",2000,NO_REC,2},                             	  //是否配置休眠时钟
////	{"AT+CPSMS=1,,,\"00011000\",\"00001010\"\r\n","OK",2000,NO_REC,2},   	  //配置休眠时钟 20S
////	
////	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //查询信号
////	{"AT+CEREG?\r\n","+CEREG: 0,1",200,NO_REC,255},                        //等待驻网，尝试次数根据当地信号不同
////	
////	{"AT+ECOAPNEW=106.13.150.28,5683,1\r\n","+ECOAPNEW:",3000,NO_REC,3},    //新建COAP链接
////	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //问询信号值
////	{"AT+CCLK?\r\n","GMT",200,NO_REC,2},                                   //问询网络时间
////	{"AT+ECOAPSEND=1,","COAPNMI:",10000,NO_REC,5},                         //发送COAP数据包  此命令延时与信号有关
////	{"AT+ECOAPDEL=1\r\n","OK",2000,NO_REC,3},                              //断开COAP链接
////	{"AT+ZRST","OK",2000,NO_REC,3},                                        //复位模组（建议用硬件复位）
};

/*-------------------------------------------------*/
/*函数名： 发送AT指令                      */
/*参  数：ATCmdNum对应的AT指令                     */
/*返回值：无                                       */
void ATSend(teATCmdNum ATCmdNum)
{
	char MID[5];
    char MID_msg[5];       //
    char msg[100];         //字符串消息
    char hex_msg[200];     //hex转换以后的消息
    char msg_body[300];    //完整数据包消息
    uint16_t msg_len = 0;
	
	clearUart();			//清空串口信息
	ATCmds[ATCmdNum].ATStatus = NO_REC;   //设置为 未接收
    ATRecCmdNum = ATCmdNum;
	
	if(ATCmdNum == AT_TCPSEND)
    {
		// memset(hex_msg,0x00,sizeof(hex_msg));
        // memset(MID,0x00,sizeof(MID));
        // sprintf(MID,"%d",rand()%90+10);  //随机数作为MID使用  只能是两位数所以设置为 10-99
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
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //调试
    }
	else	//发送普通AT命令
	{
		Uart_SendStr((const uint8_t *)ATCmds[ATCmdNum].ATSendStr);
	}
	//打开超时定时器
    GetTime(&TimeNB,ATCmds[ATCmdNum].TimeOut);
}

void ATRec(void)
{
    if(stUart.RxOK)  //串口收到数据
    {
		stUart.RxOK = 0;
		stUart.u8Count = 0;
		stUart.u8Timer = 0x7F;
        if(strstr((const char*)stUart.Buf,ATCmds[ATRecCmdNum].ATRecStr) != NULL) //如果包含对应校验符
        {
            ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC;  //设置为成功接收
        }
        UART_Printf("@@Rev:%s\r\n",stUart.Buf);
//        Usart2type.UsartRecFlag = 0;  //清空接收标志位
//        Usart2type.UsartRecLen = 0;   //清空接收长度
		
//		clearUart();			//清空串口信息
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
//    NB_POWER_ON();                     //开机
    NB_TaskStatus = NB_SEND;           //进入发送模式
    ATCurrentCmdNum = AT;              //当前指令
    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);        //下一条指令
}

void COAPSendData(uint16_t Flag)
{
    ADC_val = Flag;
//    NB_WAKE_UP();                      //从休眠状态下唤醒
//    printf("@ wakeup NB ...\r\n");
    ATCurrentCmdNum = AT_CREG;
    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);        //下一条指令
    NB_TaskStatus = NB_SEND;
}

//NB工作状态机
void NB_Work(void)
{
    switch(NB_TaskStatus)
    {
    case NB_IDIE:                                 //空闲状态
        //printf("...\r\n");
        return;
    case NB_SEND:                                 //发送状态
        if(ATCurrentCmdNum != ATNextCmdNum)       //非重发状态
        {
            CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //重发次数赋值
        }
        ATSend(ATCurrentCmdNum);                  //发送数据
        NB_TaskStatus = NB_WAIT;
        return;
    case NB_WAIT:                                 //等待数据状态
        ATRec();                                   //接收数据
        Rec_WaitAT();                              //处理成功命令与失败命令
        return;
    case NB_ACCESS:

        break;
    default:
        return;
    }
}

//NB任务状态机
//main函数while(1)中调用
void NB_Task(void)
{
    while(1)
    {
        switch(NB_TaskStatus)
        {
        case NB_IDIE:                                 //空闲状态
            //printf("...\r\n");
            return;
        case NB_SEND:                                 //发送状态
            if(ATCurrentCmdNum != ATNextCmdNum)       //非重发状态
            {
                CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //重发次数赋值
            }
            ATSend(ATCurrentCmdNum);                  //发送数据
            NB_TaskStatus = NB_WAIT;
            return;
        case NB_WAIT:                                 //等待数据状态
            ATRec();                                   //接收数据
            Rec_WaitAT();                              //处理成功命令与失败命令
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
    if(ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC) //成功数据包
    {
        switch(ATCurrentCmdNum)
        {
////			case AT_I:
////				if(IMEI_flag==0)  //还未识别IMEI
////				{
////				 memset(IMEI_buf, 0, sizeof(IMEI_buf));     //查询信号值
////////			     find_string_flag= Find_string((char *)Usart2type.Usart2RecBuffer,"IMEI: ","\r\n",(char *)IMEI_buf); //取出信号值
////				if(find_string_flag) IMEI_flag =1;
////				}
////				ATCurrentCmdNum += 1;
////                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);     //命令递进
////               NB_TaskStatus = NB_SEND;
////				break;
////        case AT_ZCONTLED:                 //点灯或者关灯完毕
////            if(my_sleep ==1 )             //使能PSM功能
////            {
////                ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                NB_TaskStatus = NB_SEND;
////            } else                       //未使能PSM功能（跳过休眠配置）
////            {
////                ATCurrentCmdNum = AT_CEREG;
////                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                NB_TaskStatus = NB_SEND;
////            }
////            break;
////        case AT_ZSLR:                         //休眠模式已经配置
////            ATCurrentCmdNum = AT_CEREG;       //开始驻网
////            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
////            NB_TaskStatus = NB_SEND;
////            break;
////        case AT_CPSMS_1:
////            ATCurrentCmdNum = AT_ZRST;       //复位模组（可换成硬件复位）
////            ATNextCmdNum = AT;               //从头开始运行
////            NB_TaskStatus = NB_SEND;
////            break;
        case AT_CREG_1:
//            printf("@@CEREG--SUCCESS...\r\n");
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;
            break;
		case AT_COPS_1:
//            printf("@@CEREG--SUCCESS...\r\n");
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;
            break;
		case AT_CSQ:
            memset(CSQ_buffer, 0, sizeof(CSQ_buffer));     //查询信号值
            find_string_flag= Find_string((char *)stUart.Buf,": ",",",(char *)CSQ_buffer); //取出信号值
            if(find_string_flag==0)                        //查询信号失败
            {
                ATCurrentCmdNum  = AT_CSQ;
                ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            } else // 成功取出CSQ
            {
////                if(Cat_Time.flag == 1)  //如果网络对表完成，不再对表
////                {
////                    ATCurrentCmdNum += 2;
////                    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
////                } 
////				else 
				{                //如果还未对表，发送对表命令
                    ATCurrentCmdNum += 1;
                    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
                }
            }
            NB_TaskStatus = NB_SEND;
            break;
//		case AT_CIFSR:
////            printf("@@CEREG--SUCCESS...\r\n");
//            ATCurrentCmdNum += 1;
//            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
//            NB_TaskStatus = NB_SEND;
//            break;
        case AT_TCPSEND:
            CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;   //成功一次就赋值
            NB_reset_time =5;                              //复位次数赋值
//            ATCurrentCmdNum += 1;
			ATCurrentCmdNum = AT_CIPSTART;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);     //命令递进
            NB_TaskStatus = NB_SEND;
            break;
        case AT_TCPDEL:                                  //断开COAP连接
////            memset(CSQ_buffer,0x00,sizeof(CSQ_buffer));    //清空信号数据buf
////            work_flag =0;                                  //清空定时器标识
            NB_TaskStatus = NB_IDIE;                       //设置为空闲态（等待NB休眠）
            break;
        
        default:
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            NB_TaskStatus = NB_SEND;
            break;
        }
    }
    else if(CompareTime(&TimeNB))           //还没收到预期数据  并且超时
    {
//        printf("@@REV--TimeOut:%s\r\n",stUart.Buf);
        switch(ATCurrentCmdNum)  //检索超时出错的命令
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
        if(CurrentRty > 0)  //重发
        {
            CurrentRty--;
//            printf("@@ now:%d,trytime:%d\r\n",ATCurrentCmdNum,CurrentRty);
            ATNextCmdNum = ATCurrentCmdNum;  //下一条 还是当前命令  实现重发效果
            NB_TaskStatus = NB_SEND;
        }
        else  //重发次数用完
        {
            NB_RESET();
            ATCurrentCmdNum = AT;
            NB_TaskStatus = NB_SEND;
            return;
        }
    }
}

/*设置时间定时器*/
void GetTime(tsTimeType *TimeType,uint32_t TimeInter)
{
    TimeType->TimeStart = GetSystemNowtime();    //获取系统运行时间
    TimeType->TimeInter = TimeInter;      //间隔时间
}
uint8_t  CompareTime(tsTimeType *TimeType)  //比较时间
{
    uint32_t nowtime = GetSystemNowtime();
    if(nowtime >= TimeType->TimeStart)  
        return ((nowtime-TimeType->TimeStart) >= TimeType->TimeInter);  //返回1代表超时
    else
        return ((nowtime+(0xFFFFFFFF-TimeType->TimeStart)) >= TimeType->TimeInter);  //返回1代表超时
    
    // return ((GetSystemNowtime()-TimeType->TimeStart) >= TimeType->TimeInter);  //返回1代表超时
}
/*********************************END OF FILE**********************************/

