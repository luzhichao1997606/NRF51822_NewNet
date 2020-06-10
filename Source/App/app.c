/******************************************************************************
* File        : app.c
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2011-05-01    Lihao         Original Version
*******************************************************************************/

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
#include "hal.h"    //包含需要的头文件
#include "string.h"       //包含需要的头文件
#include "stdio.h"        //包含需要的头文件
#include "app.h"
#include "mystring.h"
#include "NetHandler.h"
#include "AT_Proc.h" 
#include "nrf24l01+.h"
/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/

/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/
#define  DEBUG  1
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


/*循环事件执行标志*/
uint16  LoopEvents=0x0000;

/*循环事件执行标志暂存*/
uint16  LoopEventstmp=0x0000;//在中断处理函数中会被修改

/*定时循环事件执行标志*/
static uint16  LoopTimeEvents=0x0000;

/*定时循环事件执行标志暂存*/
static uint16  LoopTimeEventstmp=0x0000;

//MQTT上报计时
int MQTT_Count_SendTimes = 0;

//继电器计时标志
uint8_t MQTT_Relay_AlarmCount_flag = 0;

//继电器计时
uint32_t MQTT_Relay_AlarmCount = 0;

//初次上电15分钟上报一次

//初次上电标志位
bool First_Power_ON_Flag  ;

//初次上电15分钟上报一次
#define First_Power_ON_TimeCount 15
/****************************************************************
* Function Name: appInit
* Decription   : app层初始化函数
* Calls        : 
* Called By    : None
* Input        : None
* Output       : None
* Return Value : None
* Others       : 
*------------------------- Revision History -------------------
* No. Version Date      Revised by   Item      Description     
* 1   V1.0    2011-2-18 LiHao   SRWF-4E68 Original Version 
*****************************************************************/
void appInit(void)
{
	
}

/****************************************************************
* Function Name: appTask
* Decription   : app层主函数
* Calls        : 
* Called By    : None
* Input        : None
* Output       : None
* Return Value : None
* Others       : 
*------------------------- Revision History -------------------
* No. Version Date      Revised by   Item      Description     
* 1   V1.0    2011-2-18 LiHao   SRWF-4E68 Original Version 
*****************************************************************/
void appTask(void)
{
}


/**
  * @fun    TimeDifference
  * @brief  时间差
  * @param  t1
  * @param  t2
  * @retval time difference
  */
uint32 TimeDifference(uint32 newtime,uint32 oldtime)
{
    if(newtime > oldtime)
        return newtime - oldtime;
    else
        return newtime + (0xFFFFFFFF - oldtime); //时间计数溢出
}


/*心跳处理函数*/
int HeartBeatHandler(void)
{
    UART_Printf("\r\nRun into HeartBeatHandler function !\r\n");
    if (!W5500_NOPHY_TryGPRS_Flag)
    {
        /* code */
        return MQTT_HeartBeat();
    }
    else
    {
        UART_Printf("HeartBeat!!!!!!!!!!!!! \r\n");
        MQTT_GPRS_Heartbeat(); 
    }
    
	
}

/*数据发送处理函数*/
int SendDataHandler(void)
{
    UART_Printf("\r\nRun into SendDataHandler function !\r\n");
    if (!W5500_NOPHY_TryGPRS_Flag)
    {
        /* code */
	    return MQTT_SendData();
    }
    else
    {
        UART_Printf("SendData!!!!!!! \r\n");
        MQTT_GPRS_SendData(); 
    }
    
    
}

//*********************************函数实体区**********************************
/*
 * 函数名：系统软件初始化
 * 描述  ：配置LED用到的I/O口
 * 输入  ：无
 * 输出  ：无
 */
void SoftWareInit(void)
{	
    UART_Printf("\r\nHandler SoftWareInit function ! \r\n");
    
//    /*配置W5500网络*/
    NetworkInitHandler();
    
    /*设置轮询接收网络数据包事件标志 */  
   // LoopEventSetBit(APP_EVENT_GATEWAY_NETINPUT);
    
//    /*接收网络协议数据事件*/
    //LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);
	
	/*接收网络协议数据事件*/
    if (!W5500_NOPHY_TryGPRS_Flag)
    {
        LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);//使用W5500通讯
    }
    else
    {
        LoopEventSetBit(APP_EVENT_GATEWAY_GPRSINT);//使用GPRS通讯
    } 
//	LoopEventSetBit(APP_EVENT_GATEWAY_GPRSINT);//使用GPRS通讯
}

/**
  * @fun    LoopEventSetBit
  * @brief  循环事件标志位置位
  * @param  None
  * @retval None
  */
void LoopEventSetBit(EventType event)
{
    LoopEventstmp|=event;
}

/**
  * @fun    LoopTimeEventSetBit
  * @brief  定时循环事件标志位置位
  * @param  None
  * @retval None
  */
void LoopTimeEventSetBit(EventType event)
{
    LoopTimeEventstmp|=event;
}


/**
  * @fun    SoftWareEventInLoop
  * @brief  事件在环执行
  * @param  None
  * @retval None
  */     
void SoftWareEventInLoop(void)
{
    uint8  hdlstate=0;  /*处理状态*/
    uint8_t GPRS_Status = 0;
    static uint32_t dhcp_ret = DHCP_STOPPED;
	
	//----------------------------------/*软件在环--初始化W5500事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_NETINT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_NETINT!\r\n");
        /*配置W5500网络*/
    	NetworkInitHandler();
		dhcp_ret = DHCP_STOPPED;
        LoopEventSetBit(APP_EVENT_GATEWAY_DHCP);/*置位网络终端协议转换事件*/  
        hdlstate=1;
        if(hdlstate){
            /*置零-初始化W5500事件*/  
            LoopEvents ^= APP_EVENT_GATEWAY_NETINT; 
            /*置位网络终端协议转换事件*/  
            //LoopEvents |= APP_EVENT_GATEWAY_DHCP;   
            hdlstate ^= hdlstate; 
        } 
    }
	
	//----------------------------------/*软件在环--W5500 DHCP配置事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_DHCP ){
        UART_Printf("\r\n APP_EVENT_GATEWAY_DHCP!\r\n");
        while( !((dhcp_ret == DHCP_IP_ASSIGN) || (dhcp_ret == DHCP_IP_CHANGED) 
			  || (dhcp_ret == DHCP_FAILED && gWIZNETINFO.dhcp == NETINFO_STATIC) 
              || (dhcp_ret == DHCP_IP_LEASED)))
		{
			dhcp_ret = DHCP_proc();      
		}
		DHCP_stop();// if restart, recall DHCP_init() 
        LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT);/*置位网络终端协议转换事件*/
        hdlstate=1;
        if(hdlstate){
             /*置零DHCP配置事件*/ 
            LoopEvents ^= APP_EVENT_GATEWAY_DHCP;  
            /*置位初始化W5500 Mqtt事件*/  
            //LoopEvents |= APP_EVENT_GATEWAY_CONNECT;  
            hdlstate ^= hdlstate;
        }
    }
	
	//----------------------------------/*软件在环--初始化W5500 Mqtt事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_CONNECT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_CONNECT!\r\n");
        if (!W5500_NOPHY_TryGPRS_Flag)
        { 
            if(MQTT_Init() != 0  )
            {
			    LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);/*置位网络终端协议转换事件*/
                /*置位初始化W5500  事件*/  
                //LoopEvents |= APP_EVENT_GATEWAY_NETINT;  
            }
		    else  
            {   
                LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);/*置位网络终端协议转换事件*/
                /*置位初始化W5500 Mqtt接收事件*/  
                //LoopEvents |= APP_EVENT_GATEWAY_RECVNET; 
            }
        }
        hdlstate=1;
        if(hdlstate)
        {
            LoopEvents ^= APP_EVENT_GATEWAY_CONNECT;
            hdlstate ^= hdlstate;
        }
    }
	
	//----------------------------------/*软件在环--W5500 mqtt接收事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_RECVNET){
//        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVE!\r\n");
        if ( !W5500_NOPHY_TryGPRS_Flag )
        { 
            if(MQTT_Working()!=0   )
		    {
			    LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);/*置位网络终端协议转换事件*/
                /*置位初始化W5500  事件*/  
                //LoopEvents |= APP_EVENT_GATEWAY_NETINT;  
			    hdlstate=1;
		    }
        }
//        LoopEventSetBit();/*不会引起其他事件*/
//        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_RECVNET;
            hdlstate ^= hdlstate;
        }
    }
	
    //----------------------------------/*软件在环--初始化GPRS事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_GPRSINT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_GPRSINT!\r\n");
        /*复位、初始化GPRS指令参数*/  
        NB_Init();
        LoopEventSetBit(APP_EVENT_GATEWAY_GPRSDO);/*置位网络终端协议转换事件*/
        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_GPRSINT;
            hdlstate ^= hdlstate;
        }
    }
    //----------------------------------/*软件在环--GPRS指令处理事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_GPRSDO){
      
//        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVE!\r\n");
        NB_Work();  
        if (GPRS_InitOver_Flag)
        {
           LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);/*置位网络终端协议转换事件*/
           hdlstate=1;
        } 
        // LoopEventSetBit();/*不会引起其他事件*/
        //LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);/*置位网络终端协议转换事件*/
        
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_GPRSDO;
            hdlstate ^= hdlstate;
        }
    }

	//*******************网关定时**************************************************/     
    //----------------------------------/*软件在环网关定时事件*/
    if(LoopEvents & APP_EVENT_GATEWAY_TIMEING){
        UART_Printf("\r\n Gateway timed events in SoftWareEventInLoop!\r\n");
        UART_Printf("\r\n APP_EVENT_GATEWAY_TIMEING!\r\n");
        
        UART_Printf("\r\n Handler SoftTimeingLoopEvent Function!\r\n");
        hdlstate=SoftTimeingLoopEvent();/*定时循环事件执行函数*/
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_TIMEING;
            hdlstate ^= hdlstate;
        }
    }   
//*****************************************************************************/      
    LoopEvents |= LoopEventstmp;
    LoopEventstmp ^= LoopEventstmp;
    
//*****************************************************************************/         
    //LwIP_Periodic_Handle();//轮询是否接收到数据
}


/**
  * @fun    SoftWareTimeingEventInLoop
  * @brief  定时事件在环执行
  * @param  None
  * @retval None
  */

void SoftWareTimeingEventInLoop(void)
{
	uint8 tag_cnt;
    static uint32 HeartBeatLt=0; /*定义心跳事件上一次状态的时间 */
 	static uint32 SendDataLt=0; /*定义心跳事件上一次状态的时间 */
    static uint32 GPRSSendDataLt=0; /*定义gprs事件上一次状态的时间 */
      
    static uint8  HeartBeatLtNowtimeStr[10];//打印心跳当前时间
    uint32  newTime=0;//范围：0-4294967295(4 Bytes)
    newTime=GetSystemNowtime();/*获得当前运行时间*/  
    if ( LoopEvents & APP_EVENT_GATEWAY_RECVNET)
    {  
    //------------------------------------------------------------------------
    //********网关心跳包事件***************************************************
    if(TimeDifference(newTime,HeartBeatLt)>=HEART_BEAT_TIME){
        HeartBeatLt=newTime;
        {//调试输出：当前心跳处于的时间。
          LongIntToStr(HeartBeatLt,HeartBeatLtNowtimeStr);//长整形数转成字符串
          UART_Printf("\r\n HeartBeatLt Nowtime=");   
          UART_Printf(HeartBeatLtNowtimeStr);//输出当前时间
          UART_Printf("\r\n");//回车换行     
        }
        UART_Printf("\r\n Gateway heartbeat time arrived !\r\n");
       
        LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*置位软件在环网关定时事件*/
        UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
        
        LoopTimeEventSetBit(APP_T_EVENT_HEARTBEAT);/*置位心跳包事件*/
        UART_Printf("\r\n Set Gateway heartbeat timed events into SoftTimeingLoopEvent!\r\n");
    }
	
	//********网关数据包事件***************************************************
    //2s进入一次
    if(TimeDifference(newTime,SendDataLt)>=(SEND_DATA_TIME))
    {
        if ( First_Power_ON_Flag )
        {
            //大于15分钟
            if (newTime >= (First_Power_ON_TimeCount * 60  ))
            {
                First_Power_ON_Flag = false;
            } 
        }
        if ( !First_Power_ON_Flag)
        { 
        MQTT_Count_SendTimes ++;
        tag_cnt = 16;    
        UART_Printf(" MQTT_Count_SendTimes N ：%d ", MQTT_Count_SendTimes)  ;
        SendDataLt = newTime;
        UART_Printf("MQTT_Resv_Cycle : %d" , MQTT_Resv_Cycle );
        //计算双向链表中的数据
		tag_cnt = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
        //上报时间计算（*30）
        if (MQTT_Count_SendTimes < (MQTT_Resv_Cycle * 30) || MQTT_Count_SendTimes == 0)
        {
            return; 
        }
        else if(MQTT_Count_SendTimes >= (MQTT_Resv_Cycle * 30))
        {
             MQTT_Count_SendTimes = 0;
        } 
		//if(tag_cnt == 0)
		//	return;

        HeartBeatLt=newTime;
        // if(tag_cnt >= 70) 
        // 	tag_cnt = 70;
        
        {//调试输出：当前心跳处于的时间。
            LongIntToStr(SendDataLt,HeartBeatLtNowtimeStr);//长整形数转成字符串
            UART_Printf("\r\n HeartBeatLt Nowtime=");   
            UART_Printf(HeartBeatLtNowtimeStr);//输出当前时间
            UART_Printf("\r\n");//回车换行   
           
        }
        
        UART_Printf("\r\n Gateway send data time arrived !\r\n");
    
        LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*置位软件在环网关定时事件*/
        UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
        
        LoopTimeEventSetBit(APP_T_EVENT_SENDDATA);/*置位数据发送事件*/
        UART_Printf("\r\n Set Gateway send data timed events into SoftTimeingLoopEvent!\r\n"); 
        }
    }

		if(MQTT_Resv_Read_data || Clear_Flag)
		{
			MQTT_Resv_Read_data = 0;
			//MQTT_SendData();
            Clear_Flag = 0 ;
            UART_Printf("\r\n ReadData事件或者有传感器离线事件发生 ， 进入上报状态 !\r\n");
			LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*置位软件在环网关定时事件*/
    		UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
    
    		LoopTimeEventSetBit(APP_T_EVENT_SENDDATA);/*置位数据发送事件*/
    		UART_Printf("\r\n Set Gateway send data timed events into SoftTimeingLoopEvent!\r\n");
		}

        if ( MQTT_Resv_Alarm && !MQTT_Relay_AlarmCount_flag )
        { 
           nrf_gpio_pin_set(Relay_PIN);
           UART_Printf("\r\n RELAY ALARM !!!!!!!!!!!!!!!!!!\r\n");  
           MQTT_Relay_AlarmCount_flag = 1;
           MQTT_Relay_AlarmCount = newTime;
        } 
        Clear_Buffer_TimeOutTask();  //清除Buffer数据
		NRF_ALLReflash_Channel();	 //NRF刷新通道 
        
    //********GPRS事件网关数据包事件***************************************************
        if (W5500_NOPHY_TryGPRS_Flag && TimeDifference(newTime,GPRSSendDataLt)>=(GPRS_Rscv_DATA_TIME))
        {
            GPRSSendDataLt = newTime;
            //发送接收事件的命令
            MQTT_GPRS_SendRscvDataCMD();
        }
        
    }

    LoopTimeEvents|=LoopTimeEventstmp;
    LoopTimeEventstmp^=LoopTimeEventstmp;
}


/**
  * @fun    SoftTimeingLoopEvent
  * @brief  定时循环事件执行函数
  * @param  None
  * @retval state of handle event
  */
uint8 SoftTimeingLoopEvent(void)
{
    uint8 state=0;
    uint8 hdlstate=0;
    
    //------------------------------------------------------------------------
    //********网关心跳包事件***************************************************
    if(LoopTimeEvents & APP_T_EVENT_HEARTBEAT){
        UART_Printf("\r\n APP_T_EVENT_HEARTBEAT!\r\n");
        if(HeartBeatHandler() != 0)
      		LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT); /*置位软件在环网关定时事件*/
        hdlstate=1;
        if(hdlstate){
            LoopTimeEvents ^= APP_T_EVENT_HEARTBEAT;
            hdlstate ^= hdlstate;
        }
    }
	
	//********网关数据包事件***************************************************
    if(LoopTimeEvents & APP_T_EVENT_SENDDATA){
        UART_Printf("\r\n APP_T_EVENT_SENDDATA!\r\n");
      	if(SendDataHandler() != 0)
      		LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT); /*置位软件在环网关定时事件*/
        hdlstate=1;
        if(hdlstate){
            LoopTimeEvents ^= APP_T_EVENT_SENDDATA;
            hdlstate ^= hdlstate;
        }
    }
    
   
    if(!LoopTimeEvents) state=1;
    return state;
}


/*******************************************************************************
* Function Name: tmrDelay
* Decription   : 利用硬件定时器精确延时u16Delay ms
* Calls        : 
* Called By    :
* Arguments    : u16Delay: 延时时间(ms)
* Returns      : None
* Others       : 注意事项
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2011-05-01    Lihao         Original Version
*******************************************************************************/
#if 0
void tmrDelay(uint16 u16Delay)
{
    delayTicks = u16Delay;
    while(delayTicks > 0)
    {
        u16Delay = u16Delay;
    }
}
#endif

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/
