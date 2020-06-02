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
uint16  LoopEventstmp=0x0000;//���жϴ��������лᱻ�޸�

/*��ʱѭ���¼�ִ�б�־*/
static uint16  LoopTimeEvents=0x0000;

/*��ʱѭ���¼�ִ�б�־�ݴ�*/
static uint16  LoopTimeEventstmp=0x0000;

//MQTT�ϱ���ʱ
int MQTT_Count_SendTimes = 0;
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


/*������������*/
int HeartBeatHandler(void)
{
    UART_Printf("\r\nRun into HeartBeatHandler function !\r\n");
	return MQTT_HeartBeat();
}

/*���ݷ��ʹ�������*/
int SendDataHandler(void)
{
    UART_Printf("\r\nRun into SendDataHandler function !\r\n");
	return MQTT_SendData();
}

//*********************************����ʵ����**********************************
/*
 * ��������ϵͳ������ʼ��
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
    LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);//ʹ��W5500ͨѶ
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
   
    static uint32_t dhcp_ret = DHCP_STOPPED;
	
	//----------------------------------/*�����ڻ�--��ʼ��W5500�¼�*/
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
	 
	//----------------------------------/*�����ڻ�--W5500 DHCP�����¼�*/
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
	
	//----------------------------------/*�����ڻ�--��ʼ��W5500 Mqtt�¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_CONNECT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_CONNECT!\r\n");
        if(MQTT_Init() != 0)
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

        hdlstate=1;
        if(hdlstate)
        {
            LoopEvents ^= APP_EVENT_GATEWAY_CONNECT;
            hdlstate ^= hdlstate;
        }
    }
	
	//----------------------------------/*�����ڻ�--W5500 mqtt�����¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_RECVNET){
//        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVE!\r\n");
        if(MQTT_Working()!=0)
		{
			LoopEventSetBit(APP_EVENT_GATEWAY_NETINT);/*��λ�����ն�Э��ת���¼�*/
            /*��λ��ʼ��W5500  �¼�*/  
            //LoopEvents |= APP_EVENT_GATEWAY_NETINT; 

			hdlstate=1;
		}
//        LoopEventSetBit();/*�������������¼�*/
//        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_RECVNET;
            hdlstate ^= hdlstate;
        }
    }
	
    //----------------------------------/*�����ڻ�--��ʼ��GPRS�¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_GPRSINT){
        UART_Printf("\r\n APP_EVENT_GATEWAY_GPRSINT!\r\n");
        /*��λ����ʼ��GPRSָ�����*/
    	NB_Init();          //��ʼ��NBģ��
		
        LoopEventSetBit(APP_EVENT_GATEWAY_GPRSDO);/*��λ�����ն�Э��ת���¼�*/
        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_GPRSINT;
            hdlstate ^= hdlstate;
        }
    }
    //----------------------------------/*�����ڻ�--GPRSָ����¼�*/
    if(LoopEvents & APP_EVENT_GATEWAY_GPRSDO){
      
//        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVE!\r\n");
          NB_Work();  
//        LoopEventSetBit();/*�������������¼�*/
//        hdlstate=1;
        if(hdlstate){
            LoopEvents ^= APP_EVENT_GATEWAY_GPRSDO;
            hdlstate ^= hdlstate;
        }
    }

	//*******************���ض�ʱ**************************************************/     
    //----------------------------------/*�����ڻ����ض�ʱ�¼�*/
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
	
	
////////*******************����NET->GATEWAY->�ն�*************************************/
//////    //----------------------------------/*�����ڻ�--��������Э�������¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_RECVNET){
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVNET!\r\n");
//////        
//////        LoopEventSetBit(APP_EVENT_GATEWAY_NETTOEDV);/*��λ�����ն�Э��ת���¼�*/
//////        hdlstate=1;
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_RECVNET;
//////            hdlstate ^= hdlstate;
//////        }
//////    }
//////    
//////    //----------------------------------/*�����ڻ�--�����ն�Э��ת���¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_NETTOEDV){
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_NETTOEDV!\r\n");
//////        while( !((dhcp_ret == DHCP_IP_ASSIGN) || (dhcp_ret == DHCP_IP_CHANGED) || (dhcp_ret == DHCP_FAILED) || (dhcp_ret == DHCP_IP_LEASED)))
//////		{
//////			dhcp_ret = DHCP_proc();      
//////		}
//////		DHCP_stop();// if restart, recall DHCP_init()
//////        LoopEventSetBit(APP_EVENT_GATEWAY_SENDEDV);/*��λ���ڴ����¼�*/
//////        hdlstate=1;
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_NETTOEDV;
//////            hdlstate ^= hdlstate;
//////        }
//////    }
//////    
//////    //----------------------------------/*�����ڻ�--�����ն�Э�������¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_SENDEDV){
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_SENDEDV!\r\n");
//////        MQTT_Init(); 
//////		//LoopEventSetBit();/*�������������¼�*/
//////        hdlstate=1;
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_SENDEDV;
//////            hdlstate ^= hdlstate;
//////        } 
//////    }
//////
////////*******************�ն�->GATEWAY->����NET*************************************/    
//////    //----------------------------------/*�����ڻ�--�����ն�Э�������¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_RECVEDV){
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_RECVEDV!\r\n");
//////        
//////        //APP_SerialRecvHandler();/*���ڽ��մ����¼�*/
//////
//////        LoopEventSetBit(APP_EVENT_GATEWAY_EDVTONET);/*��λ�ն�����Э��ת���¼�*/
//////        hdlstate=1;
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_RECVEDV;
//////            hdlstate ^= hdlstate;
//////        }
//////    }
//////    
//////    //----------------------------------/*�����ڻ�--�ն�����Э��ת���¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_EDVTONET){
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_EDVTONET!\r\n");
//////         
//////        LoopEventSetBit(APP_EVENT_GATEWAY_SENDNET);/*��λ�ն�����Э��ת���¼�*/
//////        hdlstate=1;
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_EDVTONET;
//////            hdlstate ^= hdlstate;
//////        }
//////    }
//////    
//////    //----------------------------------/*�����ڻ�--��������Э�������¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_SENDNET){
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_SENDNET!\r\n");
//////        
//////        //LoopEventSetBit();/*�������������¼�*/
//////        hdlstate=1;
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_SENDNET;
//////            hdlstate ^= hdlstate;
//////        }
//////    }
//////    
////////*******************���ض�ʱ**************************************************/     
//////    //----------------------------------/*�����ڻ����ض�ʱ�¼�*/
//////    if(LoopEvents & APP_EVENT_GATEWAY_TIMEING){
//////        UART_Printf("\r\n Gateway timed events in SoftWareEventInLoop!\r\n");
//////        UART_Printf("\r\n APP_EVENT_GATEWAY_TIMEING!\r\n");
//////        
//////        UART_Printf("\r\n Handler SoftTimeingLoopEvent Function!\r\n");
//////        hdlstate=SoftTimeingLoopEvent();/*��ʱѭ���¼�ִ�к���*/
//////        if(hdlstate){
//////            LoopEvents ^= APP_EVENT_GATEWAY_TIMEING;
//////            hdlstate ^= hdlstate;
//////        }
//////    }
    
    
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
       
        LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*��λ�����ڻ����ض�ʱ�¼�*/
        UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
        
        LoopTimeEventSetBit(APP_T_EVENT_HEARTBEAT);/*��λ�������¼�*/
        UART_Printf("\r\n Set Gateway heartbeat timed events into SoftTimeingLoopEvent!\r\n");
    }
	
	//********�������ݰ��¼�***************************************************
    if(TimeDifference(newTime,SendDataLt)>=(SEND_DATA_TIME)){
    
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
    
        LoopEventSetBit(APP_EVENT_GATEWAY_TIMEING); /*��λ�����ڻ����ض�ʱ�¼�*/
        UART_Printf("\r\n Set Gateway timed events into SoftWareEventInLoop!\r\n");
        
        LoopTimeEventSetBit(APP_T_EVENT_SENDDATA);/*��λ���ݷ����¼�*/
        UART_Printf("\r\n Set Gateway send data timed events into SoftTimeingLoopEvent!\r\n"); 
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
      		LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT); /*��λ�����ڻ����ض�ʱ�¼�*/
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
      		LoopEventSetBit(APP_EVENT_GATEWAY_CONNECT); /*��λ�����ڻ����ض�ʱ�¼�*/
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