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
#include "cJSON.h"
#include <string.h>
#include "mystring.h"
#include "app.h"
#include "NetHandler.h"

static uint8_t CurrentRty;                                                 //重发次数
static teATCmdNum ATRecCmdNum;                                             //接收状态
static tsTimeType TimeNB;
teNB_TaskStatus NB_TaskStatus;                                             //AT指令模式
teATCmdNum ATNextCmdNum;                                                   //下条指令
teATCmdNum ATCurrentCmdNum;                                                //当前指令

char NB_SendDataBuff[500];
char CSQ_buffer[3];
char topic_msg[44];
char IMEI_buf[22];
uint8_t IMEI_flag =0;                                                  //
uint8_t find_string_flag =0;
uint32_t First_open_flag = 0;
uint16_t ADC_val =0;
uint8_t NB_reset_time = 5;                                                 //复位次数
uint8_t AT_error_time = 50;                                                //网络上时间尝试次数
uint32_t RTC_Time = 30*60;                                                 //RTC唤醒时间
uint8_t my_sleep = 0;                                                      //休眠开关（0为关闭！*初学者不要修改*）
uint8_t GPRS_InitOver_Flag = 0;
tsATCmds ATCmds[] =                                                        //AT指令列表
{
	{"AT\r\n","OK",200,NO_REC,100},                                         //关闭回显测试
	{"ATE0\r\n","OK",200,NO_REC,100},                                       //关闭回显测试
	{"AT+CIPHEAD=0\r\n", "OK",200,NO_REC,2},                                //接收数据时是否增加 IP 头提示 
	{"AT+CPIN?\r\n", "READY",100,NO_REC,10},                                //查询PIN码锁状态
	
	{"AT+CREG=0\r\n", "OK",100,NO_REC,10},			                        //网络注册信息
	{"AT+CREG?\r\n", "+CREG:",100,NO_REC,255},                              //等待驻网，尝试次数根据当地信号不同

	{"AT+COPS=3,2\r\n", "OK",100,NO_REC,10},
	{"AT+COPS?\r\n", "+COPS:",100,NO_REC,10},	
	
	{"AT+CSQ\r\n","+CSQ:",200,NO_REC,3},                                    //查询信号
	{"AT+CGATT?\r\n","+CGATT: 1",200,NO_REC,255},  
	
	  
    {"AT+CIPSHUT\r\n","OK",200,NO_REC,2},                                   //关闭GPRS(PDP上下文去激活)
    {"AT+CSTT=\"cmnet\"\r\n","OK",200,NO_REC,1},                            //设置APN
    {"AT+CIICR\r\n","OK",200,NO_REC,10},                                    //激活GPRS连接
    {"AT+CIFSR\r\n",".",200,NO_REC,10},                                     // 获取本机地址

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
	//str3比str2短，往前移动 
	if(lenofstr < n)  
	{
		tmp = cp+n;
		while(*tmp)
		{
			*(tmp-(n-lenofstr)) = *tmp; //n-lenofstr是移动的距离 
			tmp++;
		}
		*(tmp-(n-lenofstr)) = *tmp; //move '\0'	
	}
	else
	        //str3比str2长，往后移动
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
//str1为传入数据，str2为需要替换的字符串,str3为计划替换的字符串,
char * jsondata_change_toGPRS(char * str1,char * str2,char *str3)
{
    int i,len,count=0;
    char c;
   	char *p;  	
	//开始查找字符串str2 
   	p = strstr(str1,str2);
   	while(p)
	{
		count++;
		//每找到一个str2，就用str3来替换 
		str_replace(p,strlen(str2),str3);
		p = p+strlen(str3);
		p = strstr(p,str2);
	}   	
    UART_Printf("替换完成，已经将 %s 替换为 %s，一共 %d 个 \r\n",str2,str3,count);
    return str1;
}
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
    char str2[5]="\"";
    char str3[5]="\\22"; 
    uint16_t msg_len = 0;
	
	clearUart();			//清空串口信息
	ATCmds[ATCmdNum].ATStatus = NO_REC;   //设置为 未接收
    ATRecCmdNum = ATCmdNum;
	
	if(ATCmdNum == AT_MPUB)
    {   
        //发送AT命令正常之后进行订阅
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff)); 
        strcat(NB_SendDataBuff,"AT+MSUB=\"/WSN-LW/");
        strcat(NB_SendDataBuff,Read_ID);
        strcat(NB_SendDataBuff,"/service/+\",0");
        //sprintf(NB_SendDataBuff,"%s\r\n",Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1)); 
        //strcat(NB_SendDataBuff,jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1),str2,str3));
        strcat(NB_SendDataBuff,"\r\n");  
       // UART_Printf("TestData : %s \r\n",jsondata_change_toGPRS(Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,1),str2,str3));
        Uart_SendStr(NB_SendDataBuff);
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //调试
        ATRec();

    }
	else	//发送普通AT命令
	{
		Uart_SendStr((const uint8_t *)ATCmds[ATCmdNum].ATSendStr);
	}
	//打开超时定时器
    GetTime(&TimeNB,ATCmds[ATCmdNum].TimeOut);
}
//GPRS的MQTT数据发送函数
void MQTT_GPRS_SendData()
{       
    char str2[5]="\""; 
    char str3[5]="\\22"; 
    uint8_t  Pack_Num = 0;  
	//默认120个从机
	if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;}
	//计算包数
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
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //调试
    }
        
}
//GPRS的MQTT数据发送函数
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
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //调试
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
    }
}
//MQTT发送接受数据的命令
void MQTT_GPRS_SendRscvDataCMD()
{
        //发送AT命令正常之后进行订阅
        memset(NB_SendDataBuff,0,sizeof(NB_SendDataBuff)); 
        strcat(NB_SendDataBuff,"AT+MQTTMSGGET\r\n");  
        Uart_SendStr(NB_SendDataBuff);
        UART_Printf("@@ NB_SendDataBuff:%s\r\n",NB_SendDataBuff);  //调试
        MQTT_GPRS_ResvData();
}
//MQTT接受数据
void MQTT_GPRS_ResvData()
{ 
    uint8_t Count_Error = 0;
    uint8_t Topic_Num[60]; 
    uint8_t JsonData[200];
    if(stUart.RxOK)  //串口收到数据
    {
		stUart.RxOK = 0;
		stUart.u8Count = 0;
		stUart.u8Timer = 0x7F; 
        UART_Printf("@@Rev:%s\r\n",stUart.Buf); 
        //遍历数组
        for (uint8_t i = 0; i < 180; i++)
        { 
            //Topic 进行打印
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
                //Topic 进行打印
                UART_Printf("Topic : %s\r\n",Topic_Num); 
            }   
              //获取json数据    
            if (stUart.Buf[i] == '{' )
            {
                for (uint8_t k = 0; k < 100; k++)
                {
                    JsonData[k] = stUart.Buf[i+k];
                } 
                 //json数据 进行打印
                UART_Printf("JsonData : %s\r\n",JsonData); 
                Unpack_json_MQTT_ResvData(JsonData);
            }
            //如果发送之后出现错误那么就会重新进行数据的处理进行重新的初始化
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
//    NB_POWER_ON();                     //开机
    NB_TaskStatus = NB_SEND;           //进入发送模式
    ATCurrentCmdNum = AT;              //当前指令
    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);        //下一条指令
}

void COAPSendData(uint16_t Flag)
{
    ADC_val = Flag; 
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
            } 
            else // 成功取出CSQ
            {
 
				{   //如果还未对表，发送对表命令
                    ATCurrentCmdNum += 1;
                    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
                }
            }
            NB_TaskStatus = NB_SEND;
            break;  
       case AT_MCONFIG:
            
            UART_Printf("\r\n 设置MQTT参数密码账号 %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;

            break;

       case AT_MIPSTART:
            
            UART_Printf("\r\n 设置连接时的地址和端口号  %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;

            break;

       case AT_MCONNECT:
            
            UART_Printf("\r\n 开始建立MQTT链接 %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;

            break;  

       case AT_MQTTMSGSET:
            
            UART_Printf("\r\n 设置MQTT的上报消息模式 %d\r\n",ATCurrentCmdNum);
            ATCurrentCmdNum += 1;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;

            break;   
       case AT_MPUB:
            
            UART_Printf("\r\n MQTT上报消息PUB %d\r\n",ATCurrentCmdNum);
            //如果进入此处说明GPRS初始化完成。
            GPRS_InitOver_Flag = 1; 
            ATCurrentCmdNum  = AT_MQTTMSGSET;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum +1);   //命令递进
            NB_TaskStatus = NB_SEND;

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
        switch(ATCurrentCmdNum)  //检索超时出错的命令
        {
 
        default:
            break;
        }
        ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
        if(CurrentRty > 0)  //重发
        {
            CurrentRty--; 
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

