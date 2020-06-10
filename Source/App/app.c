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
#include "hal.h"    //������Ҫ��ͷ�ļ�
#include "string.h"       //������Ҫ��ͷ�ļ�
#include "stdio.h"        //������Ҫ��ͷ�ļ�
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


/*ѭ���¼�ִ�б�־*/
uint16  LoopEvents=0x0000;

/*ѭ���¼�ִ�б�־�ݴ�*/
uint16  LoopEventstmp=0x0000;//���жϴ������лᱻ�޸�

/*��ʱѭ���¼�ִ�б�־*/
static uint16  LoopTimeEvents=0x0000;

/*��ʱѭ���¼�ִ�б�־�ݴ�*/
static uint16  LoopTimeEventstmp=0x0000;

//MQTT�ϱ���ʱ
int MQTT_Count_SendTimes = 0;

//�̵�����ʱ��־
uint8_t MQTT_Relay_AlarmCount_flag = 0;

//�̵�����ʱ
uint32_t MQTT_Relay_AlarmCount = 0;

//�����ϵ�15�����ϱ�һ��

//�����ϵ��־λ
bool First_Power_ON_Flag  ;

//�����ϵ�15�����ϱ�һ��
#define First_Power_ON_TimeCount 15
/****************************************************************
* Function Name: appInit
* Decription   : app���ʼ������
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
* Decription   : app��������
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
  * @brief  ʱ���
  * @param  t1
  * @param  t2
  * @retval time difference
  */
uint32 TimeDifference(uint32 newtime,uint32 oldtime)
{
    if(newtime > oldtime)
        return newtime - oldtime;
    else
        return newtime + (0xFFFFFFFF - oldtime); //ʱ��������
}


/*����������*/
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

/*���ݷ��ʹ�����*/
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

//*********************************����ʵ����**********************************
/*
 * ��������ϵͳ�����ʼ��
 * ����  ������LED�õ���I/O��
 * ����  ����
 * ���  ����
 */
void SoftWareInit(void)
{	
    UART_Printf("\r\nHandler SoftWareInit function ! \r\n");
    
//    /*����W5500����*/
    NetworkInitHandler();
    
    /*������ѯ�����������ݰ��¼���־ */  
   // LoopEventSetBit(APP_EVENT_GATEWAY_NETINPUT);
    
//    /*��������Э�������¼�*/
    //LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);
	
	/*��������Э�������¼�*/
    if (!W5500_NOPHY_TryGPRS_Flag)
    {
        LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);//ʹ��W5500ͨѶ
    }
    else
    {
        LoopEventSetBit(APP_EVENT_GATEWAY_GPRSINT);//ʹ��GPRSͨѶ
    } 
//	LoopEventSetBit(APP_EVENT_GATEWAY_GPRSINT);//ʹ��GPRSͨѶ
}

/**
  * @fun    LoopEventSetBit
  * @brief  ѭ���¼���־λ��λ
  * @param  None
  * @retval None
  */
void LoopEventSetBit(EventType event)
{
    LoopEventstmp|=event;
}

/**
  * @fun    LoopTimeEventSetBit
  * @brief  ��ʱѭ���¼���־λ��λ
  * @param  None
  * @retval None
  */
void LoopTimeEventSetBit(EventType event)
{
    LoopTimeEventstmp|=event;
}


/**
  * @fun    SoftWareEventInLoop
  * @brief  �¼��ڻ�ִ��
  * @param  None
  * @retval None
  */     
void SoftWareEventInLoop(void)
{
    uint8  hdlstate=0;  /*����״̬*/
    uint8_t GPRS_Status = 0;
    static uint32_t dhcp_ret = DHCP_STOPPED;
	
	//----------------------------------/*����ڻ�--��ʼ��W5500�¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_NETINT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_NETINT!\r\n");
        /*����W5500����*/
    	NetworkInitHandler();
		dhcp_ret = DHCP_STOPPED;
        LoopEventSetBit(APP_EVENT_GATEWAY_DHCP);/*��λ�����ն�Э��ת���¼�*/  
        hdlstate=1;
        if(hdlstate){
            /*����-��ʼ��W5500�¼�*/  
            LoopEvents ^= APP_EVENT_GATEWAY_NETINT; 
            /*��λ�����ն�Э��ת���¼�*/  
            //LoopEvents |= APP_EVENT_GATEWAY_DHCP;   
            hdlstate ^= hdlstate; 
        } 
    }
	
	//----------------------------------/*����ڻ�--W5500 DHCP�����¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_DHCP ){
        UART_Printf("\r\n APP_EVENT_GATEWAY_DHCP!\r\n");
        while( !((dhcp_ret == DHCP_IP_ASSIGN) || (dhcp_ret == DHCP_IP_CHANGED) 
			  || (dhcp_ret == DHCP_FAILED && gWIZNETINFO.dhcp == NETINFO_STATIC) 
              || (dhcp_ret == DHCP_IP_LEASED)))
		{
			dhcp_ret = DHCP_proc();      
		}
		DHCP_stop();// if restart, recall DHCP_init() 
        LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT);/*��λ�����ն�Э��ת���¼�*/
        hdlstate=1;
        if(hdlstate){
             /*����DHCP�����¼�*/ 
            LoopEvents ^= APP_EVENT_GATEWAY_DHCP;  
            /*��λ��ʼ��W5500 Mqtt�¼�*/  
            //LoopEvents |= APP_EVENT_GATEWAY_CONNECT;  
            hdlstate ^= hdlstate;
        }
    }
	
	//----------------------------------/*����ڻ�--��ʼ��W5500 Mqtt�¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_CONNECT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_CONNECT!\r\n");
        if (!W5500_NOPHY_TryGPRS_Flag)
        { 
            if(MQTT_Init() != 0  )
            {
			    LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);/*��λ�����ն�Э��ת���¼�*/
                /*��λ��ʼ��W5500  �¼�*/  
                //LoopEvents |= APP_EVENT_GATEWAY_NETINT;  
            }
		    else  
            {   
                LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);/*��λ�����ն�Э��ת���¼�*/
                /*��λ��ʼ��W5500 Mqtt�����¼�*/  
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
	
	//----------------------------------/*����ڻ�--W5500 mqtt�����¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_RECVNET){
//        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVE!\r\n");
        if ( !W5500_NOPHY_TryGPRS_Flag )
        { 
            if(MQTT_Working()!=0   )
		    {
			    LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);/*��λ�����ն�Э��ת���¼�*/
                /*��λ��ʼ��W5500  �¼�*/  
                //LoopEvents |= APP_EVENT_GATEWAY_NETINT;  
			    hdlstate=1;
		    }
        }
//        LoopEventSetBit();/*�������������¼�*/
//        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_RECVNET;
            hdlstate ^= hdlstate;
        }
    }
	
    //----------------------------------/*����ڻ�--��ʼ��GPRS�¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_GPRSINT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_GPRSINT!\r\n");
        /*��λ����ʼ��GPRSָ�����*/  
        NB_Init();
        LoopEventSetBit(APP_EVENT_GATEWAY_GPRSDO);/*��λ�����ն�Э��ת���¼�*/
        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_GPRSINT;
            hdlstate ^= hdlstate;
        }
    }
    //----------------------------------/*����ڻ�--GPRSָ����¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_GPRSDO){
      
//        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVE!\r\n");
        NB_Work();  
        if (GPRS_InitOver_Flag)
        {
           LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);/*��λ�����ն�Э��ת���¼�*/
           hdlstate=1;
        } 
        // LoopEventSetBit();/*�������������¼�*/
        //LoopEventSetBit(APP_EVENT_GATEWAY_RECVNET);/*��λ�����ն�Э��ת���¼�*/
        
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_GPRSDO;
            hdlstate ^= hdlstate;
        }
    }

	//*******************���ض�ʱ**************************************************/     
    //----------------------------------/*����ڻ����ض�ʱ�¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_TIMEING){
        UART_Printf("\r\n Gateway timed events in SoftWareEventInLoop!\r\n");
        UART_Printf("\r\n APP_EVENT_GATEWAY_TIMEING!\r\n");
        
        UART_Printf("\r\n Handler SoftTimeingLoopEvent Function!\r\n");
        hdlstate=SoftTimeingLoopEvent();/*��ʱѭ���¼�ִ�к���*/
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_TIMEING;
            hdlstate ^= hdlstate;
        }
    }   
//*****************************************************************************/      
    LoopEvents |= LoopEventstmp;
    LoopEventstmp ^= LoopEventstmp;
    
//*****************************************************************************/         
    //LwIP_Periodic_Handle();//��ѯ�Ƿ���յ�����
}


/**
  * @fun    SoftWareTimeingEventInLoop
  * @brief  ��ʱ�¼��ڻ�ִ��
  * @param  None
  * @retval None
  */

void SoftWareTimeingEventInLoop(void)
{
	uint8 tag_cnt;
    static uint32 HeartBeatLt=0; /*���������¼���һ��״̬��ʱ�� */
 	static uint32 SendDataLt=0; /*���������¼���һ��״̬��ʱ�� */
    static uint32 GPRSSendDataLt=0; /*����gprs�¼���һ��״̬��ʱ�� */
      
    static uint8  HeartBeatLtNowtimeStr[10];//��ӡ������ǰʱ��
    uint32  newTime=0;//��Χ��0-4294967295(4 Bytes)
    newTime=GetSystemNowtime();/*��õ�ǰ����ʱ��*/  
    if ( LoopEvents & APP_EVENT_GATEWAY_RECVNET)
    {  
    //------------------------------------------------------------------------
    //********�����������¼�***************************************************
    if(TimeDifference(newTime,HeartBeatLt)>=HEART_BEAT_TIME){
        HeartBeatLt=newTime;
        {//�����������ǰ�������ڵ�ʱ�䡣
          LongIntToStr(HeartBeatLt,HeartBeatLtNowtimeStr);//��������ת���ַ���
          UART_Printf("\r\n HeartBeatLt Nowtime=");   
          UART_Printf(HeartBeatLtNowtimeStr);//�����ǰʱ��
          UART_Printf("\r\n");//�س�����     
        }
        UART_Printf("\r\n Gateway heartbeat time arrived !\r\n");
       
        LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*��λ����ڻ����ض�ʱ�¼�*/
        UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
        
        LoopTimeEventSetBit(APP_T_EVENT_HEARTBEAT);/*��λ�������¼�*/
        UART_Printf("\r\n Set Gateway heartbeat timed events into SoftTimeingLoopEvent!\r\n");
    }
	
	//********�������ݰ��¼�***************************************************
    //2s����һ��
    if(TimeDifference(newTime,SendDataLt)>=(SEND_DATA_TIME))
    {
        if ( First_Power_ON_Flag )
        {
            //����15����
            if (newTime >= (First_Power_ON_TimeCount * 60  ))
            {
                First_Power_ON_Flag = false;
            } 
        }
        if ( !First_Power_ON_Flag)
        { 
        MQTT_Count_SendTimes ++;
        tag_cnt = 16;    
        UART_Printf(" MQTT_Count_SendTimes N ��%d ", MQTT_Count_SendTimes)  ;
        SendDataLt = newTime;
        UART_Printf("MQTT_Resv_Cycle : %d" , MQTT_Resv_Cycle );
        //����˫�������е�����
		tag_cnt = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
        //�ϱ�ʱ����㣨*30��
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
        
        {//�����������ǰ�������ڵ�ʱ�䡣
            LongIntToStr(SendDataLt,HeartBeatLtNowtimeStr);//��������ת���ַ���
            UART_Printf("\r\n HeartBeatLt Nowtime=");   
            UART_Printf(HeartBeatLtNowtimeStr);//�����ǰʱ��
            UART_Printf("\r\n");//�س�����   
           
        }
        
        UART_Printf("\r\n Gateway send data time arrived !\r\n");
    
        LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*��λ����ڻ����ض�ʱ�¼�*/
        UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
        
        LoopTimeEventSetBit(APP_T_EVENT_SENDDATA);/*��λ���ݷ����¼�*/
        UART_Printf("\r\n Set Gateway send data timed events into SoftTimeingLoopEvent!\r\n"); 
        }
    }

		if(MQTT_Resv_Read_data || Clear_Flag)
		{
			MQTT_Resv_Read_data = 0;
			//MQTT_SendData();
            Clear_Flag = 0 ;
            UART_Printf("\r\n ReadData�¼������д����������¼����� �� �����ϱ�״̬ !\r\n");
			LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*��λ����ڻ����ض�ʱ�¼�*/
    		UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
    
    		LoopTimeEventSetBit(APP_T_EVENT_SENDDATA);/*��λ���ݷ����¼�*/
    		UART_Printf("\r\n Set Gateway send data timed events into SoftTimeingLoopEvent!\r\n");
		}

        if ( MQTT_Resv_Alarm && !MQTT_Relay_AlarmCount_flag )
        { 
           nrf_gpio_pin_set(Relay_PIN);
           UART_Printf("\r\n RELAY ALARM !!!!!!!!!!!!!!!!!!\r\n");  
           MQTT_Relay_AlarmCount_flag = 1;
           MQTT_Relay_AlarmCount = newTime;
        } 
        Clear_Buffer_TimeOutTask();  //���Buffer����
		NRF_ALLReflash_Channel();	 //NRFˢ��ͨ�� 
        
    //********GPRS�¼��������ݰ��¼�***************************************************
        if (W5500_NOPHY_TryGPRS_Flag && TimeDifference(newTime,GPRSSendDataLt)>=(GPRS_Rscv_DATA_TIME))
        {
            GPRSSendDataLt = newTime;
            //���ͽ����¼�������
            MQTT_GPRS_SendRscvDataCMD();
        }
        
    }

    LoopTimeEvents|=LoopTimeEventstmp;
    LoopTimeEventstmp^=LoopTimeEventstmp;
}


/**
  * @fun    SoftTimeingLoopEvent
  * @brief  ��ʱѭ���¼�ִ�к���
  * @param  None
  * @retval state of handle event
  */
uint8 SoftTimeingLoopEvent(void)
{
    uint8 state=0;
    uint8 hdlstate=0;
    
    //------------------------------------------------------------------------
    //********�����������¼�***************************************************
    if(LoopTimeEvents & APP_T_EVENT_HEARTBEAT){
        UART_Printf("\r\n APP_T_EVENT_HEARTBEAT!\r\n");
        if(HeartBeatHandler() != 0)
      		LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT); /*��λ����ڻ����ض�ʱ�¼�*/
        hdlstate=1;
        if(hdlstate){
            LoopTimeEvents ^= APP_T_EVENT_HEARTBEAT;
            hdlstate ^= hdlstate;
        }
    }
	
	//********�������ݰ��¼�***************************************************
    if(LoopTimeEvents & APP_T_EVENT_SENDDATA){
        UART_Printf("\r\n APP_T_EVENT_SENDDATA!\r\n");
      	if(SendDataHandler() != 0)
      		LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT); /*��λ����ڻ����ض�ʱ�¼�*/
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
* Decription   : ����Ӳ����ʱ����ȷ��ʱu16Delay ms
* Calls        : 
* Called By    :
* Arguments    : u16Delay: ��ʱʱ��(ms)
* Returns      : None
* Others       : ע������
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
