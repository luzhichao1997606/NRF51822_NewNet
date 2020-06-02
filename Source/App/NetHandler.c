/**
 * NetHander.c �����ʼ������
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#include "NetHandler.h"
#include "hal.h"    //������Ҫ��ͷ�ļ�
#include "mystring.h"
#include <string.h>
#include "cJSON.h"

#include "socket.h"//Just include one header for WIZCHIP
#include "dhcp.h"
#include "dns.h"
#include "MQTTClient.h"

/*************************protobuf-c*****************************/

#include "UserInformation.pb-c.h"

#define CardNum		10

//typedef union node_package
//{
//    struct {
//        uint8_t id[4];  /* serial number array */
//        uint8_t rssi;    /* typically, 0xff */
//    }t;
//    uint8_t buf[5];
//}node_pkg_t;

extern uint8_t		Read_ID[16] ;
uint8 SensorNum		= 120;
uint8 SensorStart	= 1;
//uint8_t message_pack_buffer[1024];
UserInformation userInfo = USER_INFORMATION__INIT;
Heartbeat heartbeat = HEARTBEAT__INIT;

//CardList usercard = CARD_LIST__INIT;

Card cardInfo[CardNum];
Card* cardInfo_p[CardNum];
uint8 strIDbuf[CardNum][6][5]={0};	//����ID�ַ���
//	
/*************************protobuf-c*****************************/

	
uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}}; /* WIZCHIP SOCKET Buffer initialize */

uint8_t domain_ip[4]={0};/*����IP*/
uint8_t domain_name[]="emqx.iricbing.cn";//"mqtt.yun-ran.com";//"yeelink.net";/*����*/

unsigned char tempBuffer[2048];
//MQTT ��������
unsigned char SendBuffer[3072]= {"3132213B2470E251012401020304"};

unsigned char Buffer[2]= {0x29,0x55};

//unsigned char ReciveBuffer[500];
typedef union Recive_package
{
    struct {
        uint8_t		is_process_data;  /* serial number array */
		uint8_t		topic[10];
        uint32_t	len;    /* typically, 0xff */
		uint8_t		databuf[500];
    }t;
    uint8_t buf[515];
}Recive_pkg_t;

Recive_pkg_t RecivePkg;

char Topic[50] = "/WSN_LW/";
char SubTopic[50] = "/WSN_LW/";

MQTTClient mqttclient;
MQTTMessage mqtt_msg;
Network network;

/* Private macro -------------------------------------------------------------*/
uint8_t gDATABUF[DATA_BUF_SIZE];//��ȡ���ݵĻ�������2048

/*Ĭ������IP��ַ����*/
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},//MAC��ַ
                            .ip = {192, 168, 3, 127},                  //IP��ַ
                            .sn = {255,255,255,0},                     //��������
                            .gw = {192, 168, 3, 1},                    //Ĭ������
                            .dns = {114,114,114,114},                   //DNS������
                            .dhcp = NETINFO_DHCP  // NETINFO_STATIC
};


/* Private functions ---------------------------------------------------------*/
static void RegisterSPItoW5500(void);/*��SPI�ӿں���ע�ᵽW5500��socket����*/
static void InitW5500SocketBuf(void);/*��ʼ��W5500����оƬ*/
static void PhyLinkStatusCheck(void);/* PHY��·״̬���*/
static void DhcpInitHandler(void);   /*DHCP��ʼ��*/
static void my_ip_assign(void);      /*��̬����IP*/          
static void my_ip_conflict(void);    /*IP��ַ��ͻ�ļ򵥻ص�����*/
void DNS_Analyse(void);/*DNS����*/



/*-------------------------------------------------*/
/*�������������ٽ���                               */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);    //��ֹȫ���ж�
}
/*-------------------------------------------------*/
/*���������˳��ٽ���                               */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);   //��ȫ���ж�
}

/*-------------------------------------------------*/
/*�������� Ƭѡ�ź�����͵�ƽ                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CS_Select(void)
{
	nrf_gpio_pin_clear(W5500_SPI_CS_PIN);
}

/*-------------------------------------------------*/
/*�������� Ƭѡ�ź�����ߵ�ƽ                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SPI_CS_Deselect(void)
{
	nrf_gpio_pin_set(W5500_SPI_CS_PIN); 
}


/* IP��ַ��ͻ�ļ򵥻ص����� */
static void my_ip_conflict(void)
{
    UART_Printf("CONFLICT IP from DHCP\r\n");
    
    //halt or reset or any...
    while(1); // this example is halt.
}

/*-------------------------------------------------*/
/*�������� Ƭѡ�ź�����ߵ�ƽ                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void W5500_Reset(void)
{
	nrf_gpio_pin_clear(W5500_RST_PIN);
	delay_ms(500);
	nrf_gpio_pin_set(W5500_RST_PIN);
	delay_ms(2000);
}

/*******************************************************
 * @ brief Call back for ip assing & ip update from DHCP
 * ��̬����IP����Ϣ
 *******************************************************/
static void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);  //IP��ַ
   getGWfromDHCP(gWIZNETINFO.gw);  //Ĭ������
   getSNfromDHCP(gWIZNETINFO.sn);  //��������
   getDNSfromDHCP(gWIZNETINFO.dns);//DNS������
   gWIZNETINFO.dhcp = NETINFO_DHCP;
   
   /* Network initialization */
   network_init();//Ӧ��DHCP����������ַ���������ʼ��
   UART_Printf("DHCP LEASED TIME : %d Sec.\r\n", getDHCPLeasetime());//��ӡ��õ�DHCP�����ַʱ��

   DNS_Analyse();//��������
}


/******************************************************************************
 * @brief  Network Init
 * Intialize the network information to be used in WIZCHIP
 *****************************************************************************/
void network_init(void)
{
    uint8_t tmpstr[6] = {0};
    wiz_NetInfo netinfo;

    /*����gWIZNETINFO�ṹ������������Ϣ*/
    //DNS������gWIZNETINFO�����б���������ȷ����������޷���ȷ����������
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

    /*��ȡ���ú��������Ϣ����netinfo�ṹ��*/
    ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

    /* ���ڴ�ӡ������Ϣ */
    ctlwizchip(CW_GET_ID,(void*)tmpstr);
    if(netinfo.dhcp == NETINFO_DHCP) 
      UART_Printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
    else 
      UART_Printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);

    UART_Printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
                    netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
    UART_Printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
    UART_Printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
    UART_Printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
    UART_Printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
    UART_Printf("===========================\r\n");
}

/*��SPI�ӿں���ע�ᵽW5500��socket����*/
static void RegisterSPItoW5500(void)
{
  /* 1.ע���ٽ������� */
  reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);
  
  /* 2.ע��SPIƬѡ�źź��� */

  reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
      
  /* 3.ע���д���� */
  reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);
}

/*��ʼ��W5500����оƬ:ֱ�ӵ��ùٷ��ṩ�ĳ�ʼ����*/
static void InitW5500SocketBuf(void)
{
  /* WIZCHIP SOCKET Buffer initialize */
  if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){
      UART_Printf("WIZCHIP Initialized fail.\r\n");
//      while(1);
	  NVIC_SystemReset();                                //����
  }
}

/* PHY��·״̬���*/
static void PhyLinkStatusCheck(void)
{
	uint8_t tmp;
	do{
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){
			UART_Printf("Unknown PHY Link stauts.\r\n");
		}
		if(tmp == PHY_LINK_OFF){
			UART_Printf("PHY Link OFF!!!\r\n");//�����⵽������û���ӣ���ʾ��������
			delay_ms(2000);              //��ʱ2s
		}
	}while(tmp == PHY_LINK_OFF);
	UART_Printf(" PHY Link Success.\r\n");
}


/*DHCP��ʼ��*/
static void DhcpInitHandler(void)
{
   /* >> DHCP Client 				  */
  /************************************************/
  
  //must be set the default mac before DHCP started.
  setSHAR(gWIZNETINFO.mac);
		
  DHCP_init(SOCK_DHCP, gDATABUF);//gDATABUF��ȡ���ݵĻ�����
  
  // if you want defiffent action instead defalut ip assign,update, conflict,
  // if cbfunc == 0, act as default.
  //����:�����IP�����µ�IP����ͻ��IP
  reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
}

/*��̬����IP��ַ*/
uint8_t DHCP_proc(void)
{
  static uint8_t my_dhcp_retry = 0;
  uint8_t dhcp_res = DHCP_run();
  switch (dhcp_res)
  {
  case DHCP_IP_ASSIGN:
  case DHCP_IP_CHANGED:
  case DHCP_IP_LEASED:
    getIPfromDHCP(gWIZNETINFO.ip);
    getGWfromDHCP(gWIZNETINFO.gw);
    getSNfromDHCP(gWIZNETINFO.sn);
    getDNSfromDHCP(gWIZNETINFO.dns);
    gWIZNETINFO.dhcp = NETINFO_DHCP;
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
    // For debug
    // printf("\r\n>> DHCP IP Leased Time : %ld Sec\r\n", getDHCPLeasetime());
    break;
  case DHCP_FAILED:
//    UART_Printf(">> DHCP Failed\r\n");
//	gWIZNETINFO.dhcp = NETINFO_STATIC;
	my_dhcp_retry++;
	UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
	if(my_dhcp_retry > MY_MAX_DHCP_RETRY)//��̬IP����ʧ�ܴ���ʮ�Σ���ʹ�þ�̬��IP
	{
//		UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
		gWIZNETINFO.dhcp = NETINFO_STATIC;
		my_dhcp_retry = 0;
		DHCP_stop();// if restart, recall DHCP_init()
		network_init();// apply the default static network and print out netinfo to serial
		DNS_Analyse();//��������	
	}
    
    break;
  }
  return dhcp_res;
}


/*��̬����IP��ַ*/
void DhcpRunInLoop(void)
{
  static uint8_t my_dhcp_retry = 0;
//  char value[16]={0};
  int32_t t=0;
  
  
  switch(DHCP_run())
  {
    case DHCP_IP_ASSIGN://DHCP IP����
    case DHCP_IP_CHANGED://DHCP IP����
        /* If this block empty, act with default_ip_assign & default_ip_update */
        // This example calls my_ip_assign in the two case.
        break;
    
    case DHCP_IP_LEASED://TO DO YOUR NETWORK APPs.
        //����õ�DHCP�����ַʱ����ִ������App
        //yeelink_get("19610","34409",value);//���騰aD����aDT??3����??oyeelink������?��?ID--LED1
////        yeelink_get("19657","34484",value);//���騰aD����aDT??3����??oyeelink������?��?ID--LED2
////        UART_Printf("%s\n\r",value);	//char value[16]={0};
////        UART_Printf("\n\r");
	  	MQTT_Init();
//        for(t=0;t<11;t++){
//          delay_ms(1000);
//        } 
        break;
    
    case DHCP_FAILED://��̬IP����ʧ��
        /* ===== Example pseudo code =====  */
        // The below code can be replaced your code or omitted.
        // if omitted, retry to process DHCP
        my_dhcp_retry++;
        if(my_dhcp_retry > MY_MAX_DHCP_RETRY)//��̬IP����ʧ�ܴ���ʮ�Σ���ʹ�þ�̬��IP
        {
            UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
            my_dhcp_retry = 0;
            DHCP_stop();// if restart, recall DHCP_init()
            network_init();// apply the default static network and print out netinfo to serial
            DNS_Analyse();//��������
        }
        break;
        
    default:
        break;
  }  
}

/*DNS����*/
void DNS_Analyse(void)
{
    int32_t ret = 0;
  
    /*��ʼ��DNS�������򣬲�ͨ��������غ�����ȡ��machtalk����������ʵIP��ַ*/
    /* DNS client initialization */
    DNS_init(SOCK_DNS, gDATABUF);
//    Timer_Start();/*����Timer3*/
    /* DNS procssing */
    if ((ret = DNS_run(gWIZNETINFO.dns, domain_name, domain_ip)) > 0){ // try to 1st DNS
        UART_Printf("> 1st DNS Reponsed\r\n");
    }else if(ret == -1){
        UART_Printf("> MAX_DOMAIN_NAME is too small. Should be redefined it.\r\n");
//        Timer_Stop();/*�ر�Timer3*/
        while(1);
    }else{
        UART_Printf("> DNS Failed\r\n");
//        Timer_Stop();/*�ر�Timer3*/
        while(1);
    }

    //DNS�����ɹ���IP��ַ�洢��domain_ip�����У������socket��̻��õ��������ֵ��
    if(ret > 0){
        UART_Printf("> Translated %s to %d.%d.%d.%d\r\n",domain_name,domain_ip[0],domain_ip[1],domain_ip[2],domain_ip[3]);
    }
//    Timer_Stop();
}


/*����W5500����*/
int NetworkInitHandler(void)
{
	int rc = 0;
	//��������MAC��ַ
	gWIZNETINFO.mac[0] = (NRF_FICR->DEVICEID[1] >> 8)& 0XFF; 
	gWIZNETINFO.mac[1] = (NRF_FICR->DEVICEID[1] 	)& 0XFF;
	
	gWIZNETINFO.mac[2] = (NRF_FICR->DEVICEID[0] >>24)& 0XFF;
	gWIZNETINFO.mac[3] = (NRF_FICR->DEVICEID[0] >>16)& 0XFF;
	gWIZNETINFO.mac[4] = (NRF_FICR->DEVICEID[0] >> 8)& 0XFF;
	gWIZNETINFO.mac[5] = (NRF_FICR->DEVICEID[0]		)& 0XFF;

	if (gWIZNETINFO.mac[0] % 2 != 0)
	{
		gWIZNETINFO.mac[0] += 1;
	} 

	W5500_Reset();
    RegisterSPItoW5500();/*��SPI�ӿں���ע�ᵽW5500��socket����*/
    InitW5500SocketBuf();/*��ʼ��W5500����оƬ:ֱ�ӵ��ùٷ��ṩ�ĳ�ʼ����*/
    PhyLinkStatusCheck();/* PHY��·״̬���*/
    DhcpInitHandler();   /*DHCP��ʼ��*/
	
//	rc = PhyLinkStatusCheck();/* PHY��·״̬���*/
	
	return rc;
}
 

/**
 * @name: Unpack_json_MQTT_ResvData
 * @test: test font
 * @msg:  �⹹JSON���ݣ��ѹؼ���Ϣ��ȡ����	
 * @param {type} 
 * @return: 
 */
/*Ĭ������*/
//Alarm ���Ľ���
uint8_t MQTT_Resv_Alarm = 0;
uint8_t *MQTT_Resv_AlarmData ;
//Read_Data ���Ľ���
uint8_t MQTT_Resv_Read_data = 	0;
//Updata ���Ľ���
uint8_t MQTT_Resv_Cycle 	= 	1; // �ϱ����ڣ���λ���ӣ�1~255
uint8_t MQTT_Resv_AlarmTime =	10;// ��������ʱ�䣬��λ���ӣ�0~255��0��������255�������� 
uint8_t MQTT_Resv_Channel   =	12;// �����ŵ�
uint8_t MQTT_Resv_SensorNum =	40;// 1-240 �������豸����Ĳɼ�ģ������
uint8_t MQTT_Resv_SensorCycle =	35;// �������ϱ����ڣ���λ���ӣ�10~255�����10����  
int Unpack_json_MQTT_ResvData(uint8 * ResvData)
{
	cJSON *json;
	cJSON *json_value, *json_value_1 , *json_value_2 , *json_value_3 , *json_value_4;
	 
	uint8 TempBuffer_Json[5] ;
	int Str_len ;
    json = cJSON_Parse(ResvData);
    if(NULL == json)
    {
        UART_Printf("Error before: %s\n", cJSON_GetErrorPtr());
        return -1;
    }
	//��ȡ���ݳ���
	Str_len = strlen(ResvData);
	UART_Printf("Str_len of ResvData : %d\r\n" , Str_len);
	//��ȡǰ��λ������
	strncpy(TempBuffer_Json,ResvData,5); 

	//Alarm ���Ľ���
	if( strchr(TempBuffer_Json,'A') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Alarm");

		if(json_value->type == cJSON_Number)
		{
			MQTT_Resv_Alarm = json_value->valueint ;
			UART_Printf("Alarm: %d\n", MQTT_Resv_Alarm);
		}
		
		json_value_1 = cJSON_GetObjectItem(json, "AlarmData");

		if(json_value_1->type == cJSON_String)
		{
			MQTT_Resv_AlarmData = json_value_1->valuestring ;
			UART_Printf("AlarmData ��%s\n", MQTT_Resv_AlarmData);//json_value_1->valuestring);

		} 

		 
	}

	//Read_Data ���Ľ���(OK)
	if( strchr(TempBuffer_Json,'R') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Read_data");

		if(json_value->type == cJSON_Number)
		{
			MQTT_Resv_Read_data = json_value->valueint;
			UART_Printf("Read_data: %d\n", MQTT_Resv_Read_data);
			if(MQTT_Resv_Read_data)
			{
				MQTT_Resv_Read_data = 0;
				MQTT_SendData();
			}
		}  
		 
	}	

	//Updata ���Ľ���
	if( strchr(TempBuffer_Json,'C') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Cycle"); 
		if(json_value->type == cJSON_Number)
		{
			 // �ϱ����ڣ���λ���ӣ�1~255(OK)
			MQTT_Resv_Cycle = json_value->valueint;
			UART_Printf("Cycle: %d\n", MQTT_Resv_Cycle);
		} 

		json_value_1 = cJSON_GetObjectItem(json, "AlarmTime"); 
		if(json_value_1->type == cJSON_Number)
		{	
			// ��������ʱ�䣬��λ���ӣ�0~255��0��������255�������� 
			MQTT_Resv_AlarmTime = json_value_1->valueint;
			UART_Printf("AlarmTime: %d\n", MQTT_Resv_AlarmTime);
		}  

		json_value_2 = cJSON_GetObjectItem(json,"Channel");
		if(json_value_2->type == cJSON_Number)
		{
			// �����ŵ�
			MQTT_Resv_Channel = json_value_2->valueint;
			UART_Printf("Channel ��%d\n", MQTT_Resv_Channel);
		}   

		json_value_3 = cJSON_GetObjectItem(json,"SensorNum");
		if(json_value_3->type == cJSON_Number)
		{
			// 1-240 �������豸����Ĳɼ�ģ������(OK?)
			MQTT_Resv_SensorNum = json_value_3->valueint;
			UART_Printf("SensorNum ��%d\n", MQTT_Resv_SensorNum);
		}
		
		json_value_4 = cJSON_GetObjectItem(json,"SensorCycle");
		if(json_value_4->type == cJSON_Number)
		{
			// �������ϱ����ڣ���λ���ӣ�10~255�����10���� 
			// �����ݱ��ر��棬��ֹ����û�յ�ʱ���ݶ�ʧ
			MQTT_Resv_SensorCycle = json_value_4->valueint;
			UART_Printf("SensorCycle ��%d\n", MQTT_Resv_SensorCycle);
		}
		 
	}

    cJSON_Delete(json); 
	 
    return 0;
}

// MQTT ����  ȫ���ص�����
// @brief messageArrived callback function 
void messageArrived(MessageData* md)
{
  
	uint32_t i=0; 
	char* data; 
	MQTTMessage* message = md->message;
	
	memset(RecivePkg.buf,0,sizeof(RecivePkg)); 

	for (i = 0; i < md->topicName->lenstring.len; i++)
		IR_SendByte(*(md->topicName->lenstring.data + i));
 
	data = (char*)message->payload;
	RecivePkg.t.len = (int32_t)message->payloadlen;
	memcpy(RecivePkg.t.databuf,data,RecivePkg.t.len);

	UART_Printf(" Message: %s\r\n", RecivePkg.t.databuf); 
	
	//JSON���ݽ����ֵ
	Unpack_json_MQTT_ResvData(RecivePkg.t.databuf);

	RecivePkg.t.is_process_data = true; 
 
}
 
 
 
//�������
void process_message_control(void)
{
	if(RecivePkg.t.is_process_data == true)
	{
		//UART_Printf("Working.......................\r\n");
		RecivePkg.t.is_process_data = false;
		 
	}
	
}

int MQTT_Init(void)
{
	int rc = 0;
	
	NewNetwork(&network, SOCK_TCPS);
	
	UART_Printf("׼�����ӷ�����\r\n");                //���������Ϣ
	
	
	
	domain_ip[0] = 47;
	domain_ip[1] = 98;
	domain_ip[2] = 136;
	domain_ip[3] = 66;

	ConnectNetwork(&network, domain_ip, 1883);
	UART_Printf("IP��ַ: %d.%d.%d.%d,%d\r\n", domain_ip[0],domain_ip[1],domain_ip[2],domain_ip[3],1883);
	MQTTClientInit(&mqttclient,&network,1000,SendBuffer,3072,tempBuffer,2048);
	
	delay_ms(500);
	
	// ����MQTT������
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	connectData.willFlag = 0;
	connectData.MQTTVersion = 3;
//	connectData.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	connectData.clientID.cstring = "w5500-client";    //opts.clientid;
	connectData.username.cstring = "admin"; //opts.username;
	connectData.password.cstring = "public"; //opts.password; 

	connectData.keepAliveInterval = 60;
	connectData.cleansession = 1;
	
	if ((rc = MQTTConnect(&mqttclient, &connectData)) != 0)
		UART_Printf("Return code from MQTT connect is %dr\n", rc);
	else
		UART_Printf("MQTT Connected\r\n", rc);

	// �������� Updata Alarm Read_data
	sprintf(SubTopic,"/WSN_LW/");
	strcat(SubTopic, Read_ID);
	strcat(SubTopic, "/service/+");// ���Ĳ�������
	
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&mqttclient, SubTopic, QOS0, messageArrived);// /# ����һ�� 
	UART_Printf("Subscribed %d\r\n", rc);
 
	
	rc = MQTT_HeartBeat();	// ����������

	UART_Printf("MQTT HeartBeat Resulu %d \r\n",rc);       	         //���������Ϣ

	return rc;
}

/**
 * @name: Creat_json_MQTT_SendData
 * @test: test font
 * @msg: ����һ��json��ʽ�������ϴ������ݸ�ʽ
 * @param {Pub_State}  0 : SendDate 1 : HeartBeat
 * @return: char * json����
 */
size_t HeartBeat_lenght = 0;
size_t SendData_lenght = 0; 
char * Creat_json_MQTT_SendData(uint8 Pub_State)
{
	cJSON * usr; 
	uint8 *out; 
	usr = cJSON_CreateObject(); //���������ݶ���

	switch (Pub_State)
	{
	case MQTT_Publish_Type_SendData:
		/* code */
		//���������������󣬸�һ��Ĭ��ֵ
		if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;}

    	cJSON_AddItemToObject(usr, "SensorNum", cJSON_CreateNumber(MQTT_Resv_SensorNum));
		cJSON_AddItemToObject(usr, "SensorStart", cJSON_CreateNumber(SensorStart));
		cJSON_AddItemToObject(usr, "SensorData", cJSON_CreateString(SendBuffer));
 
		out = cJSON_Print(usr); //��json��ʽ��ӡ�������ַ�����ʽ
		
		UART_Printf("json Data : %s\n",out);
		
		SendData_lenght = strlen(out); 

		UART_Printf("SendData_lenght : %d \r\n" , SendData_lenght);

		// �ͷ��ڴ� 
		cJSON_Delete(usr); 
		free(out);

		return out;  
	case MQTT_Publish_Type_HeartBeat: 
		 
		cJSON_AddItemToObject(usr,"HeartBeat",cJSON_CreateNumber(999));

		out = cJSON_Print(usr); //��json��ʽ��ӡ�������ַ�����ʽ

		UART_Printf("json Data : %s\n",out);
		// �ͷ��ڴ�
		HeartBeat_lenght = strlen(out);
		UART_Printf("HeartBeat_lenght : %d \r\n" , HeartBeat_lenght);

		cJSON_Delete(usr); 
		free(out);
		
		return out;  

	default:

		break;
	}  
}

//��������������
int MQTT_HeartBeat(void)
{
	int rc = 0;
	
	static uint16_t mes_id = 0;
	
	sprintf(Topic,"/WSN_LW/");
	strcat(Topic, Read_ID);
	strcat(Topic, "/event/Heartbeat");
	
	
	heartbeat.protocoltype = "w5500";
	heartbeat.heartbeat = mes_id;
	heartbeat.producttype = "WSN-LW";
	heartbeat.softversion = "1.0.1";
	heartbeat.configtag = 1;
	 
//	unpack_heartbeat_data(tempBuffer, lenght);
	mqtt_msg.qos = QOS0;
	mqtt_msg.retained = 0;
    mqtt_msg.id = mes_id++;
    mqtt_msg.dup = 0;
	mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat);
	mqtt_msg.payloadlen = HeartBeat_lenght;
	
	rc = MQTTPublish(&mqttclient, Topic, &mqtt_msg);
	UART_Printf("Publish %s\r\n", Topic);
	//IR_SendData(	Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat)
	//				,HeartBeat_lenght);
	UART_Printf("\r\nPublish %d\r\n", rc);
	UART_Printf( "HeartBeating............" );
	
	return rc;

}


//���ݰ����ʹ�������
int MQTT_SendData(void)
{
	int rc = 0; 
	static uint16_t mes_id = 0; 
	DOUBLE_LINK_NODE *pNode;
	uint8_t tag_cnt    = 0;

	tag_cnt = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
//	if(tag_cnt == 0)
//		return 0;

	sprintf(Topic,"/WSN_LW/");
	strcat(Topic, Read_ID);
	strcat(Topic, "/event/Data");
	
	if(tag_cnt >= 10) 
        tag_cnt = 10;  
	// init_user(&userInfo);
	userInfo.protocoltype = "w5500";
	userInfo.n_cardlist = tag_cnt;
	userInfo.cardlist = cardInfo_p;
	DISABLE_GLOBAL_INTERRUPT();
	pNode = (&RADIO_DATA_LIST_HEAD)->next; 

	ENABLE_GLOBAL_INTERRUPT();
	//lenght = user_information__pack(&userInfo, SendBuffer);//user_information__get_packed_size(&userInfo);

	mqtt_msg.qos = QOS0;
	mqtt_msg.retained = 0;
	mqtt_msg.id = mes_id++;
	mqtt_msg.dup = 0;
	mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData) ; //SendBuffer;
	mqtt_msg.payloadlen = SendData_lenght ;//lenght;
	
	rc = MQTTPublish(&mqttclient, Topic, &mqtt_msg);
	UART_Printf("Publish %s\r\n", Topic);
	//IR_SendData(Buffer,SendData_lenght);

	UART_Printf("\r\nPublish %d\r\n", rc);

	UART_Printf("Sending.........");  
	 
	return rc;
}

int MQTT_Working(void)
{
	process_message_control();
	return MQTTYield(&mqttclient, 60);
    
	
}
/* ѭ����ȡ��������ֵ��������ȡ���Ľ���ύ���������ڴ�ӡ�������ȻҲ���Ը������ֵ��
 * ����LED�ƣ�ѭ����ȡ���ʱ��������10s�������yeelink���Ƶġ�
 */
uint8_t yeelink_get(const char *device_id,const char *sensors_id,char *value)
{
////    int ret;
////    char* presult;
//////    char remote_server[] = "api.yeelink.net";
////	char remote_server[] = "server.yun-ran.com";
////    char str_tmp[128] = {0};
////
////    // ���󻺳�������Ӧ������
////    static char http_request[DATA_BUF_SIZE] = {0};	//����Ϊ��̬��������ֹ��ջ���
////    static char http_response[DATA_BUF_SIZE] = {0};	//����Ϊ��̬��������ֹ��ջ���
//////    sprintf(str_tmp,"/v1.0/device/%s/sensor/%s/datapoints",device_id,sensors_id);
////	sprintf( str_tmp , "/siot/centers/password/%s","12346666");
//////	sprintf( str_tmp , "/1.txt");
////    // ȷ�� HTTP�����ײ�
////    // ����POST /v1.0/device/98d19569e0474e9abf6f075b8b5876b9/1/1/datapoints/add HTTP/1.1\r\n
////    sprintf( http_request , "GET %s HTTP/1.1\r\n",str_tmp);
////
////
////    // �������� ���� Host: api.machtalk.net\r\n
////    sprintf( str_tmp , "Host:%s\r\n" , remote_server);
////    strcat( http_request , str_tmp);
////
////    // �������� ���� APIKey: d8a605daa5f4c8a3ad086151686dce64
////    //sprintf( str_tmp , "U-ApiKey:%s\r\n" , "d8a605daa5f4c8a3ad086151686dce64");//��Ҫ�滻Ϊ�Լ���APIKey
////    sprintf( str_tmp , "U-ApiKey:%s\r\n" , "e5da11d13d2e5f540ef1a99b3506e081");//APIKey--qinfei
////    strcat( http_request , str_tmp);
////    //
////    strcat( http_request , "Accept: */*\r\n");
////    // ���ӱ��������ʽ Content-Type:application/x-www-form-urlencoded\r\n
////    strcat( http_request , "Content-Type: application/x-www-form-urlencoded\r\n");
////    strcat( http_request , "Connection: keep-alive\r\n");
////    // HTTP�ײ���HTTP���� �ָ�����
////    strcat( http_request , "\r\n");
////
////    //������ͨ��TCP���ͳ�ȥ
////    //�½�һ��Socket���󶨱��ض˿�5000
////    ret = socket(SOCK_TCPS,Sn_MR_TCP,5000,0x00);
////    if(ret != SOCK_TCPS){
////        UART_Printf("%d:Socket Error\r\n",SOCK_TCPS);
////        while(1);
////    }
//////	domain_ip[0] = 47;
//////	domain_ip[1] = 92;
//////	domain_ip[2] = 31;
//////	domain_ip[3] = 46;
////		
////    //����TCP������
////    ret = connect(SOCK_TCPS,domain_ip,80);
////    if(ret != SOCK_OK){
////        UART_Printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
////        while(1);
////    }	
//////	sprintf( http_request , "GET https://server.yun-ran.com/siot/centers/password/%s HTTP/1.1\r\n","12346666");
////    //��������
////    ret = send(SOCK_TCPS,(unsigned char *)http_request,strlen(http_request));
////    if(ret != strlen(http_request)){
////        UART_Printf("%d:Socket Send Error\r\n",SOCK_TCPS);
////        while(1);
////    }
////
////    // �����Ӧ
////    ret = recv(SOCK_TCPS,(unsigned char *)http_response,DATA_BUF_SIZE);
////    if(ret <= 0){
////        UART_Printf("%d:Socket Get Error\r\n",SOCK_TCPS);
////        while(1);
////    }
////    http_response[ret] = '\0';
////	
////	/*****************/
////	//��֧��https����ֱ����http���󷵻ؽ������
////	/*****************/
////	//	HTTP/1.1 301 Moved Permanently
////	/*****Header*****/
//////////	Server: nginx/1.14.0 (Ubuntu)
//////////	Date: Tue, 28 Apr 2020 05:56:27 GMT
//////////	Content-Type: text/html
//////////	Content-Length: 194
//////////	Connection: keep-alive
//////////	Location: https://server.yun-ran.com/siot/center/password/12346666
////	/*****Body*****/
//////////	<html>
//////////	<head><title>301 Moved Permanently</title></head>
//////////	<body bgcolor="white">
//////////	<center><h1>301 Moved Permanently</h1></center>
//////////	<hr><center>nginx/1.14.0 (Ubuntu)</center>
//////////	</body>
//////////	</html>
////	
////	//�ж��Ƿ��յ�HTTP OK
////    presult = strstr( (const char *)http_response , (const char *)"200 OK\r\n");
////    if( presult != NULL ){
////        static char strTmp[DATA_BUF_SIZE]={0};//����Ϊ��̬��������ֹ��ջ���
////        sscanf(http_response,"%*[^{]{%[^}]",strTmp);
////        //��ȡ������Ϣ
////        char timestamp[64]={0};
////        char timestampTmp[64]={0};
////        char valueTmp[64]={0};
////        sscanf(strTmp,"%[^,],%[^,]",timestampTmp,valueTmp);
////        strncpy(timestamp,strstr(timestampTmp,":")+2,strlen(strstr(timestampTmp,":"))-3);
////        strncpy(value,strstr(valueTmp,":")+1,strlen(strstr(valueTmp,":"))-1);
////    }else{
////        UART_Printf("Http Response Error\r\n");
////        UART_Printf("%s",http_response);
////    }
////    close(SOCK_TCPS);
////    return 0;
}
/*********************************END OF FILE**********************************/
