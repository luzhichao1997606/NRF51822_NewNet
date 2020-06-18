/**
 * NetHander.c 网络初始化函数
 * Describtion:
 * Author: qinfei 2015.04.09
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */
#include "NetHandler.h"
#include "math.h"
#include "hal.h"    //包含需要的头文件
#include "app.h"
#include "mystring.h"
#include <string.h>
#include "cJSON.h"
#include "nrf24l01+.h"

#include "socket.h"//Just include one header for WIZCHIP
#include "dhcp.h"
#include "dns.h"
#include "MQTTClient.h"
#include "ble_flash.h"
/*************************protobuf-c*****************************/

#include "UserInformation.pb-c.h"


#define 	CardNum				10
#define		STORE_MAGIC		0x12345679
#define   	STORE_PAGE			127
#define		STATIC_PAGE			126

#define		Debug_TCP 			0
/*默认数据*/
/**MQTT**/
//Alarm 订阅解析
uint8_t MQTT_Resv_Alarm = 0;
uint8_t *MQTT_Resv_AlarmData ;
//Read_Data 订阅解析
uint8_t MQTT_Resv_Read_data = 	0;
//Updata 订阅解析
uint8_t MQTT_Resv_Cycle 	  =	5;  // 上报周期，单位分钟，1~255
uint8_t MQTT_Resv_AlarmTime   =	5; // 报警持续时间，单位分钟，0~255，0不报警，255持续报警 
uint8_t MQTT_Resv_Channel     =	60; // 工作信道（传参到NRF24L01）
uint8_t MQTT_Resv_SensorNum   =	120;// 1-240 该数传设备下面的采集模块数量(配置数组)
uint8_t MQTT_Resv_SensorCycle =	10; // 传感器上报周期，单位分钟，10~255，最低10分钟(清除数组数据) 

/**TCP**/
uint8_t		SIP_SaveData[4] = {192,168,3,128};	//静态IP
uint8_t		GW_SaveData[4]	= {192,168,3,1};	//默认网关
uint8_t		YIP_SaveData[4] = {192,168,3,145};	//服务器IP
uint8_t		SN_SaveData[4]  = {255,255,255,0};	//子网掩码

uint16_t	SPORT_SaveData = 8080;				//静态端口
uint16_t	YPORT_SaveData = 1883;				//服务器端口

typedef struct
{
	//MQTT
	uint8_t		MQTT_Resv_Cycle; 			// 上报周期，单位分钟，1~255
	uint8_t		MQTT_Resv_AlarmTime; 		// 报警持续时间，单位分钟，0~255，0不报警，255持续报警 
	uint8_t		MQTT_Resv_Channel; 			// 工作信道（传参到NRF24L01）
	uint8_t		MQTT_Resv_SensorNum ;		// 1-240 该数传设备下面的采集模块数量(配置数组)
	uint8_t 	MQTT_Resv_SensorCycle ;		// 传感器上报周期，单位分钟，10~255，最低10分钟(清除数组数据) 
	uint8_t 	POWER_ON_COUNT  ;
	//TCP
	uint8_t		SIP_SaveData[4];			//静态IP
	uint8_t		GW_SaveData[4];				//默认网关
	uint8_t		YIP_SaveData[4];			//服务器IP
	uint8_t		SN_SaveData[4];				//子网掩码

	uint16_t	SPORT_SaveData;				//静态端口
	uint16_t	YPORT_SaveData;				//服务器端口
	
	
}para_cfg_t;

extern uint8_t		Read_ID[16] ;
extern uint8_t 		DataToSendBuffer[2400] ;

uint8 SensorNum		= 120;
uint8 SensorStart	= 1; 
UserInformation userInfo = USER_INFORMATION__INIT;
Heartbeat heartbeat = HEARTBEAT__INIT;

uint8_t NET_INIT_EVENT = 0;
/*******************域名IP**************************/

//此处可以设定MQTT服务的连接的IP以及端口

//2G模块和网线的IP都是此处进行设定的

//如果是本地服务，2G可能无法访问，此时如果接入网线的话就可以正常的进行通讯

uint8_t IP_Data[4] 	= {192 , 168  , 3 , 145};
uint16_t Port_Data 	= 1883;
 
//CardList usercard = CARD_LIST__INIT;

Card cardInfo[CardNum];
Card* cardInfo_p[CardNum];
uint8 strIDbuf[CardNum][6][5]={0};	//缓存ID字符串
//	
/*************************protobuf-c*****************************/

uint8_t domain_ip[4]={0};
 	
uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}}; /* WIZCHIP SOCKET Buffer initialize */ 
uint8_t domain_name[]="emqx.iricbing.cn";//"mqtt.yun-ran.com";//"yeelink.net";/*域名*/

unsigned char tempBuffer[10];
//MQTT 发送数据
unsigned char SendBuffer[1200];

unsigned char Buffer[2]= {0x29,0x55};

unsigned char W5500_NOPHY_TryGPRS_Flag = 0 ;
unsigned char W5500_DHCP_Flag = 0 ;

NRF24L01_Data_Set NRF_Data_Poll_1; 
 
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

char Topic[50] = "/WSN-LW/";
char SubTopic[50] = "/WSN-LW/";

MQTTClient mqttclient;
MQTTMessage mqtt_msg;
Network network;
para_cfg_t MQTT_Save_Data ;

para_cfg_t TCP_Save_Data ;

/*存储清除*/
void  store_clear(uint32_t page)
{
	 nrf_nvmc_page_erase(page*1024ul);
}
/*参数存储*/
void  para_store(uint32_t page,para_cfg_t para)
{
	nrf_nvmc_write_bytes(page*1024ul,(const uint8_t*)&para,sizeof(para_cfg_t));
}
/*参数读取*/
void  para_read(uint32_t page,para_cfg_t *ppara)
{
	volatile uint8_t *p;
	
	p =(volatile uint8_t*)(page*1024ul);
	
	memcpy((uint8_t*)ppara,(uint8_t*)p,sizeof(para_cfg_t));
	//第一次上电，flash没有存储数据(全是0xff)
	if(		ppara->MQTT_Resv_Cycle 			== 0xff 
		&& 	ppara->MQTT_Resv_AlarmTime 		== 0xff 
		&& 	ppara->MQTT_Resv_Channel 		== 0xff 
		&& 	ppara->MQTT_Resv_SensorNum 		== 0xff 
		&& 	ppara->MQTT_Resv_SensorCycle 	== 0xff 
		&&  ppara->POWER_ON_COUNT			== 0xff 
		) 
	{
		//赋值默认数值
		ppara->MQTT_Resv_Cycle 			= 	5;  // 上报周期，单位分钟，1~255
		ppara->MQTT_Resv_AlarmTime 		=	10; // 报警持续时间，单位分钟，0~255，0不报警，255持续报警 
		ppara->MQTT_Resv_Channel   		=	60; // 工作信道（传参到NRF24L01）
		ppara->MQTT_Resv_SensorNum		=	120;// 1-240 该数传设备下面的采集模块数量(配置数组)
		ppara->MQTT_Resv_SensorCycle 	=	10; // 传感器上报周期，单位分钟，10~255，最低10分钟(清除数组数据) 

		
		memcpy(ppara->SIP_SaveData,SIP_SaveData,sizeof(SIP_SaveData));  //静态IP赋值
		memcpy(ppara->GW_SaveData,GW_SaveData,sizeof(GW_SaveData)); 	//默认网关赋值
		memcpy(ppara->YIP_SaveData,YIP_SaveData,sizeof(YIP_SaveData));	//服务器IP赋值
		memcpy(ppara->SN_SaveData,SN_SaveData,sizeof(SN_SaveData));		//子网掩码

		ppara->SPORT_SaveData	=	SPORT_SaveData	;	//静态端口赋值
		ppara->YPORT_SaveData	=	YPORT_SaveData	;	//服务器端口赋值 
	}
	//如果之前就有数据说明存储过数据，所以将数据赋值给全局参数。
	else
	{
		MQTT_Resv_Cycle 		= 	ppara->MQTT_Resv_Cycle 	;	
		MQTT_Resv_AlarmTime 	= 	ppara->MQTT_Resv_AlarmTime 	;
		MQTT_Resv_Channel 		= 	ppara->MQTT_Resv_Channel   	;
		MQTT_Resv_SensorNum 	= 	ppara->MQTT_Resv_SensorNum	;
		MQTT_Resv_SensorCycle 	= 	ppara->MQTT_Resv_SensorCycle ;

		memcpy(SIP_SaveData,ppara->SIP_SaveData,sizeof(SIP_SaveData));  //静态IP赋值
		memcpy(GW_SaveData,ppara->GW_SaveData,sizeof(GW_SaveData)); 	//默认网关赋值
		memcpy(YIP_SaveData,ppara->YIP_SaveData,sizeof(YIP_SaveData));	//服务器IP赋值
		memcpy(SN_SaveData,ppara->SN_SaveData,sizeof(SN_SaveData));		//子网掩码

		SPORT_SaveData	=	ppara->SPORT_SaveData	;	//静态端口赋值
		YPORT_SaveData	=	ppara->YPORT_SaveData	;	//服务器端口赋值 
	}
	
 
	return;
}
/* Private macro -------------------------------------------------------------*/
uint8_t gDATABUF[DATA_BUF_SIZE];//获取数据的缓冲区：2048
typedef struct Info_Static_t
{ 
	uint8_t ip[4];   ///< Source IP Address 
	uint8_t gw[4];   ///< Gateway IP Address 
	uint8_t yip[4];
	uint8_t sn[4];
	
	uint16_t yport;
	uint16_t sport;
	
	dhcp_mode dhcp;  ///< 1 - Static, 2 - DHCP 
}Info_Static;
//赋值默认数据(中间数据只做参数传递无实际意义)
Info_Static  Info_Static_Get= {	.ip={192, 168, 3, 127},
								.gw={192, 168, 3, 1},
								.yip={192, 168,3, 145},
								.sn={255, 255,255, 0},
								.yport = 1883,
								.sport = 8080,
								.dhcp = NETINFO_STATIC 
};

/*默认网络IP地址配置*/
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},//MAC地址
                            .ip = {192, 168, 3, 127},                  //IP地址
                            .sn = {255,255,255,0},                     //子网掩码
                            .gw = {192, 168, 3, 1},                    //默认网关
                            .dns = {114,114,114,114},                   //DNS服务器
                            .dhcp = NETINFO_DHCP  						// NETINFO_STATIC
};

//建立一个TCP的socket函数(此处修改的是TCP服务器的IP和端口，前三位随静态IP的不同而不同，最后一位可能需要自己修改)
TCP_Network_Info TCP_network = {  	.My_TCPSocket_Num 	= 3,
									.My_TCP_Connect_IP 	= {192,168,3,140},
									.My_TCP_Connect_Port= 8080

};
//更改默认网络IP配置
void Read_Flash_TCP_Data(void)
{
	para_read(STATIC_PAGE,&TCP_Save_Data);

	for (uint8_t i = 0; i < 4; i++)
	{
		/* code */
		gWIZNETINFO.ip[i] = TCP_Save_Data.SIP_SaveData[i];
		gWIZNETINFO.sn[i] = TCP_Save_Data.SN_SaveData[i];
		gWIZNETINFO.gw[i] = TCP_Save_Data.GW_SaveData[i];
		IP_Data[i]		  = TCP_Save_Data.YIP_SaveData[i];

		Port_Data		  = TCP_Save_Data.YPORT_SaveData ; 
		#if Debug_TCP
			TCP_network.My_TCP_Connect_Port	= 8080;
		#else
		
			TCP_network.My_TCP_Connect_Port	= TCP_Save_Data.SPORT_SaveData;
		#endif
		UART_Printf("gWIZNETINFO.ip[%d] is %d \r\n",i,gWIZNETINFO.ip[i]);
		UART_Printf("gWIZNETINFO.GW[%d] is %d \r\n",i,gWIZNETINFO.gw[i]);
	}

	//在读取的时候把TCP的Socket网段也更改为设置的网段
	for (uint8_t i = 0; i < 3; i++)
	{
		if ( TCP_Save_Data.SIP_SaveData[i] != 0xff)
		{
			TCP_network.My_TCP_Connect_IP[i] = TCP_Save_Data.SIP_SaveData[i];
		} 
	}
	//store_clear(STORE_PAGE); //调试用
	//store_clear(STATIC_PAGE);//调试用
	
}
//静态的数据的获取以及参数的传递(flash存储)
//  IP GW DHCP 
void Static_IP_Get(Info_Static Info)
{ 
	//IP地址的转换 + 默认网关
	for (uint8_t j = 0; j < 4; j++)
	{
		gWIZNETINFO.ip[j] = Info.ip[j];
		gWIZNETINFO.gw[j] = Info.gw[j]; 
	}
	gWIZNETINFO.dhcp = Info.dhcp ; 
	store_clear(STATIC_PAGE);
	//flash存储
	memcpy(TCP_Save_Data.SIP_SaveData,Info.ip,sizeof(Info.ip));
	memcpy(TCP_Save_Data.YIP_SaveData,Info.yip,sizeof(Info.yip));
	memcpy(TCP_Save_Data.SN_SaveData,Info.sn,sizeof(Info.sn));
	memcpy(TCP_Save_Data.GW_SaveData,Info.gw,sizeof(Info.gw));

	TCP_Save_Data.SPORT_SaveData = Info.sport;
	TCP_Save_Data.YPORT_SaveData = Info.yport;  
	//
	UART_Printf("写入flash数据！ \r\n");
	//for (uint8_t i = 0; i < 4; i++)
	//{
	//	UART_Printf("TCP_Save_Data.SIP_SaveData[%d] is %d \r\n",i,TCP_Save_Data.SIP_SaveData[i]);
	//	UART_Printf("TCP_Save_Data.YIP_SaveData[%d] is %d \r\n",i,TCP_Save_Data.YIP_SaveData[i]);
	//	
	//}
	
	para_store(STATIC_PAGE,TCP_Save_Data); 		
} 
//建立一个TCP的soket函数(状态机)
uint8_t Connect_flag = 0;
void TCP_sTATE_loop(TCP_Network_Info* n)
{ 
	int ret,tcp_state;
 
	tcp_state=getSn_SR(n->My_TCPSocket_Num);  

	switch(tcp_state)
	{ //switch语句，判断当前TCP链接的状态   										
	case SOCK_INIT:          
		if(Connect_flag==0)
		{                                //如果还没有链接服务器，进入if
			UART_Printf("准备连接服务器\r\n");                //串口输出信息 

			#if Debug_TCP
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , 8080 ); 
			#else
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , n->My_TCP_Connect_Port ); 
			#endif
			
			UART_Printf("连接服务器返回码：%d\r\n",ret);      //串口输出信息 
			if (ret != SOCK_OK)
			{
				delay_ms(1000);
			} 
		}
		//跳出
		break;	                                       
		
	case SOCK_ESTABLISHED:   
		if((Connect_flag==0)&&(getSn_IR(n->My_TCPSocket_Num)==Sn_IR_CON))
		{  //判断链接是否建立													
			UART_Printf("连接已建立\r\n");                          //串口输出信息 
			Connect_flag = 1;                                     //链接标志=1
			setSn_IR(SOCK_TCPS, Sn_IR_CON);							/*清除接收中断标志位*/ 
		}
		if(getSn_IR(n->My_TCPSocket_Num) & Sn_IR_CON)
		{
			setSn_IR(SOCK_TCPS, Sn_IR_CON);							/*清除接收中断标志位*/ 
		}  
		break; //跳出
		
	case SOCK_CLOSE_WAIT:      
		UART_Printf("等待关闭连接\r\n");                   		//串口输出信息
		if((ret=disconnect(n->My_TCPSocket_Num)) != SOCK_OK)	//端口关闭
		{
			UART_Printf("连接关闭失败，准备重启\r\n");				//串口输出信息
			NVIC_SystemReset();										//重启
		}
		UART_Printf("连接关闭成功\r\n");                    //串口输出信息
		Connect_flag = 0;                                 //链接标志=0
		close(n->My_TCPSocket_Num);
		break;
		
	case SOCK_CLOSED:          
		UART_Printf("准备打开W5500端口\r\n");               //串口输出信息
		Connect_flag = 0;                                 //链接标志=0 
		#if Debug_TCP
			ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,8080,Sn_MR_ND);  //打开W5500的端口，用于建立TCP链接，本地TCP端口5050 
		#else
			ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,n->My_TCP_Connect_Port,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
		#endif
		//ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,8080,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
		
		if(ret != n->My_TCPSocket_Num)
		{	                           //如果打开失败，进入if																 
			UART_Printf("端口打开错误，准备重启\r\n");     //串口输出信息
			//NVIC_SystemReset();	                       //重启
		}
		else UART_Printf("打开端口成功\r\n");	       //串口输出信息
		break;                                            //跳出
	}  
}
//IP 字符串 转 整形
void IP_Str2Int(char * ip,char *IP_Int)
{ 
	char Temp,Count = 0;  
	for (uint8_t i = 0; i < 16; i++)
	{
		if (ip[i] == '.' || ip[i] > '9' || ip[i] < '0' )
		{
			Temp = i;
			for (uint8_t j = 1; j < 4; j++)
			{	
				if (	ip[Temp - j] == '.'  ){break;}  
				if (ip[Temp - j]!='.' && (Temp - j) >= 0)
				{	
					IP_Int[Count] += (ip[Temp-j] - '0')  *  pow(10,(j-1)); 
					//UART_Printf("IP_Int[%d] is %d \r\n" ,Count,IP_Int[Count]);//调试
				} 
			}  
			Count ++;
		}
		if (Count >= 4)
		{break;} 
	}  

}

// 端口号 转 整形

uint16_t COM_Str2Int(char * COM)
{  
	
	uint16_t Result_TCP = 0 ;
	uint8_t Set = 0;
	//找到0x0d
	for (uint8_t j = 0; j < strlen(COM); j++)
	{
		if(COM[j] == 0x0d)
		{
			Set = j;
			break;
		}
	} 
	for (int i = Set; i >= 0; i--)
	{	
		if ( COM[i] <= '9' && COM[i] >= '0' )
		{  
			if ( COM[i] == 0x0d  ){break;}  

			if ( COM[i]!= 0x0d )
			{	
				Result_TCP += (COM[i] - '0')  *  pow(10,(Set-i-1)); 
			} 
		}
	}
	return Result_TCP;    
}
//TCP数据的处理	
uint8_t SIP_Data_Str[20] 	= {0};
uint8_t GW_Data_Str[20] 	= {0};
uint8_t YIP_Data_Str[20] 	= {0};
uint8_t YPORT_Data_Str[20] 	= {0}; 
uint8_t SN_Data_Str[20] 	= {0}; 
uint8_t SPORT_Data_Str[10] 	= {0};  

void Data_Poll_Buffer(char * Buffer)
{
	uint8_t j;
	char Temp_Data_IP;
	char Data[4] = {0};
	for (uint8_t i = 0; i < strlen(Buffer); i++)
	{
		//SIP
		//静态IP
		if (Buffer[i] == 'S'&& Buffer[i+1] == 'I' && Buffer[i+2] == 'P')//还有冒号
		{
			UART_Printf("进入静态IP存储部分 \r\n");
			for ( j = 0; j < sizeof(SIP_Data_Str); j++)
			{
				if (Buffer[(i+4)+j] == 'G')	{	break; 	}  
				SIP_Data_Str[j] = Buffer[(i+4)+j]; 
			}
		}

		//GW
		//默认网关
		if (Buffer[i] == 'G'&& Buffer[i+1] == 'W' )//还有冒号
		{
			UART_Printf("进入默认网关存储部分\r\n"); 
			for (  j = 0; j < sizeof(GW_Data_Str); j++)
			{
				if (Buffer[(i+3)+j] == 'Y' && Buffer[(i+4)+j] == 'I' )	{	break; 	}  
				GW_Data_Str[j] = Buffer[(i+3)+j];
			} 
		}

		//YIP
		//服务器IP
		if (Buffer[i] == 'Y' && Buffer[i+1] == 'I' && Buffer[i+2] == 'P')//还有冒号
		{	
			UART_Printf("进入服务器IP存储部分\r\n"); 
			for (  j = 0; j < sizeof(YIP_Data_Str); j++)
			{
				if (Buffer[(i+4)+j] == 'Y' && Buffer[(i+5)+j] == 'P' )	{	break; 	}  
				YIP_Data_Str[j] = Buffer[(i+4)+j]; 
			} 
		}

		//YPORT
		//服务器端口 
		if (Buffer[i] == 'Y' && Buffer[i+1] == 'P' && Buffer[i+2] == 'O' 
							&& Buffer[i+3] == 'R' && Buffer[i+4] == 'T' )//还有冒号
		{
			UART_Printf("进入服务器端口存储部分\r\n");
			
			for (  j = 0; j < sizeof(YPORT_Data_Str); j++)
			{	
				if (Buffer[(i+6)+j] == 'S' && Buffer[(i+7)+j] == 'N' )	{	break; 	}  
				YPORT_Data_Str[j] = Buffer[(i+6)+j]; 
			}
		}

		//SN
		//子网掩码
		if ( Buffer[i] == 'S' && Buffer[i+1] == 'N' )//还有冒号
		{
			UART_Printf("进入子网掩码存储部分\r\n");
			
			for (  j = 0; j < sizeof(SN_Data_Str); j++)
			{	
				if (Buffer[(i+3)+j] == 'S' && Buffer[(i+4)+j] == 'P' )	{	break; 	}  
				SN_Data_Str[j] = Buffer[(i+3)+j]; 
			} 
		}	

		//SPORT
		//静态端口
		if ( Buffer[i] == 'S' && Buffer[i+1] == 'P'  
			&& Buffer[i+2] == 'O' && Buffer[i+3] == 'R' && Buffer[i+4] == 'T')//还有冒号
		{
			UART_Printf("进入静态端口存储部分\r\n");
			
			for (  j = 0; j < sizeof(SPORT_Data_Str); j++)
			{	
				if (Buffer[(i+6)+j] == 'E' && Buffer[(i+7)+j] == 'N' )	{	break; 	}  
				SPORT_Data_Str[j] = Buffer[(i+6)+j]; 
			} 
		}		

	}
	
/*******************************IP*****************************************/
	//将静态IP数据赋值
	memset(Data,0,sizeof(Data));
	UART_Printf("静态IP数据是 %s  \r\n",SIP_Data_Str );  
	IP_Str2Int(SIP_Data_Str,Data); 
	UART_Printf("SIP_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("SIP_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("SIP_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("SIP_Data_StrData [3] %d \r\n",Data[3]); 
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.ip[i] = Data[i]; }  

	//将默认网关数据赋值
	memset(Data,0,sizeof(Data));
	UART_Printf("默认网关数据是 %s \r\n",GW_Data_Str); 
	IP_Str2Int(GW_Data_Str,Data); 
	UART_Printf("GW_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("GW_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("GW_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("GW_Data_StrData [3] %d \r\n",Data[3]); 
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.gw[i] = Data[i]; }

	//将服务器IP数据赋值
	memset(Data,0,sizeof(Data));
	UART_Printf("服务器IP数据是 %s \r\n",YIP_Data_Str); 
	IP_Str2Int(YIP_Data_Str,Data); 
	UART_Printf("YIP_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("YIP_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("YIP_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("YIP_Data_StrData [3] %d \r\n",Data[3]);   
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.yip[i] = Data[i]; }

	//将子网掩码数据赋值
	memset(Data,0,sizeof(Data));
	UART_Printf("子网掩码数据是 %s \r\n",SN_Data_Str); 
	IP_Str2Int(SN_Data_Str,Data); 
	UART_Printf("SN_Data_StrData [0] %d \r\n",Data[0]);
	UART_Printf("SN_Data_StrData [1] %d \r\n",Data[1]);
	UART_Printf("SN_Data_StrData [2] %d \r\n",Data[2]);
	UART_Printf("SN_Data_StrData [3] %d \r\n",Data[3]); 
	for (uint8_t i = 0; i < 4; i++)	{ Info_Static_Get.sn[i] = Data[i]; }

/*******************************端口*****************************************/

	//UART_Printf("YPORT_Data_StrData   is %d\r\n",Info_Static_Get.yport);
	//UART_Printf("YPORT_Data_StrData   is %d\r\n",Info_Static_Get.sport);
 
	Info_Static_Get.yport =  COM_Str2Int(YPORT_Data_Str) ; 
	UART_Printf("服务器端口数据是 %d \r\n",Info_Static_Get.yport);
 
	Info_Static_Get.sport =  COM_Str2Int(SPORT_Data_Str) ; 
	UART_Printf("目标静态端口数据是 %d \r\n",Info_Static_Get.sport);

	//调用Static_IP_Get函数将数据存在结构体中
	//调用flash的存储函数进行存储
	Static_IP_Get(Info_Static_Get);
 
	//TCP完成重新启动

	//延时30s
	// delay_ms(30000);
	// NVIC_SystemReset();	                       //重启
}
//建立一个TCP的soket函数
void Creat_TCP_Client(TCP_Network_Info* n )
{
	int ret,tcp_state; 
	char buf[] = "START";
	char buffer[100] = {0}; 
	char Resv_Data_Over = 0; 
	char Resv_Data_Port_Close = 0; 
	 
	//建立TCP连接！ 
	__disable_interrupt(); 
	#if	Debug_TCP
		ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,8080,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
	#else
		ret = socket(n->My_TCPSocket_Num,Sn_MR_TCP,n->My_TCP_Connect_Port,SF_TCP_NODELAY | SF_IO_NONBLOCK); 
	#endif
	
	if (ret ==  n->My_TCPSocket_Num)
	{
		tcp_state=getSn_SR(n->My_TCPSocket_Num);   

		#if Debug_TCP
			if (NET_INIT_EVENT)
			{
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , 8080 ); 
				NET_INIT_EVENT = 0;
			} 
		#else
			if (NET_INIT_EVENT)
			{
				ret = connect(n->My_TCPSocket_Num , n->My_TCP_Connect_IP , n->My_TCP_Connect_Port ); 
				NET_INIT_EVENT = 0;
			} 
		#endif
		
		//没有连接并且连接反馈都是busy或者ok 并且 不是DHCP的
		if((Connect_flag == 0)  && (ret == 0 || ret == 1) && ( !W5500_DHCP_Flag )) 
		{ 	
			//UART_Printf(" ret is %d \r\n",ret);
			delay_ms(500);
			tcp_state=getSn_SR(n->My_TCPSocket_Num);  
			if ((getSn_IR(n->My_TCPSocket_Num)==Sn_IR_CON) && tcp_state == SOCK_ESTABLISHED)
			{
				UART_Printf("连接成功了！！！！\r\n");			
				while(!Connect_flag)
				{
					TCP_sTATE_loop(n);
				}
			}
			else
			{
				//等待下次连接 	
				UART_Printf("等待关闭连接\r\n");							//串口输出信息
				if((ret=disconnect(n->My_TCPSocket_Num)) != SOCK_OK)		//端口关闭
				{
					UART_Printf("连接关闭成功\r\n");                    		//串口输出信息
				} 
				Connect_flag = 0;                                 			//链接标志=0
				close(n->My_TCPSocket_Num); 
			} 
		} 
	} 
	if (Connect_flag)
	{
		UART_Printf("连接TCP服务器成功 \r\n"); 
		
		//此处需判断flash是否存储数据

		//进行接收数据
		while (!Resv_Data_Port_Close)
		{ 
			memset(buffer,0,sizeof(buffer)); 
			//进行发送
			ret = send(n->My_TCPSocket_Num,buf,strlen(buf)); 
			while(! Resv_Data_Over)
			{
				ret = getSn_RX_RSR(n->My_TCPSocket_Num);	     /*定义len为已接收数据的长度*/				                    		
				//如果ret大于0，表示有数据来 
				recv(n->My_TCPSocket_Num,buffer,ret);	 
				UART_Printf("TCP Data is %s \r\n",buffer)	;	/*接收来自Client的数据*/  
				delay_ms(500);
				for (uint8_t i = 0; i < strlen(buffer); i++)
				{ 		
					if (buffer[i] == 'E' && buffer[i+1] == 'N' && buffer[i+2] == 'D' )
					{
						UART_Printf(" \r\n 接收数据成功！！ \r\n !! ");
						//标志位置位
						Resv_Data_Over = 1;
					}
				}  
			} 

			if (buffer[0] == 'W')
			{
				//接收函数进行接收之后，提取数据
				Data_Poll_Buffer(buffer); 
				Resv_Data_Over = 0;	
			}
			else if(buffer[0] == 'R')
			{
				//发送函数 
				Resv_Data_Over = 0;
			}
			//5秒后重新启动
			else if(buffer[0] == 'C')
			{	
				Resv_Data_Port_Close = 1;
				delay_ms(5000);		
				NVIC_SystemReset();	//重启
			}
		}
		//等待下次连接 	
		UART_Printf("等待关闭连接\r\n");							//串口输出信息
		if((ret=disconnect(n->My_TCPSocket_Num)) != SOCK_OK)		//端口关闭
		{
			UART_Printf("连接关闭成功\r\n");                    		//串口输出信息
		} 
		Connect_flag = 0;                                 			//链接标志=0
		close(n->My_TCPSocket_Num); 
	}

	__enable_interrupt();
}
/* Private functions ---------------------------------------------------------*/
static void RegisterSPItoW5500(void);/*将SPI接口函数注册到W5500的socket库中*/
static void InitW5500SocketBuf(void);/*初始化W5500网络芯片*/
static void PhyLinkStatusCheck(void);/* PHY链路状态检查*/
static void DhcpInitHandler(void);   /*DHCP初始化*/
static void my_ip_assign(void);      /*动态分配IP*/          
static void my_ip_conflict(void);    /*IP地址冲突的简单回调函数*/
void DNS_Analyse(void);/*DNS解析*/



/*-------------------------------------------------*/
/*函数名：进入临界区                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);    //禁止全局中断
}
/*-------------------------------------------------*/
/*函数名：退出临界区                               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);   //开全局中断
}

/*-------------------------------------------------*/
/*函数名： 片选信号输出低电平                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CS_Select(void)
{
	nrf_gpio_pin_clear(W5500_SPI_CS_PIN);
}

/*-------------------------------------------------*/
/*函数名： 片选信号输出高电平                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SPI_CS_Deselect(void)
{
	nrf_gpio_pin_set(W5500_SPI_CS_PIN); 
}


/* IP地址冲突的简单回调函数 */
static void my_ip_conflict(void)
{
    UART_Printf("CONFLICT IP from DHCP\r\n");
    
    //halt or reset or any...
    while(1); // this example is halt.
}

/*-------------------------------------------------*/
/*函数名： 片选信号输出高电平                      */
/*参  数：无                                       */
/*返回值：无                                       */
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
 * 动态分配IP等信息
 *******************************************************/
static void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);  //IP地址
   getGWfromDHCP(gWIZNETINFO.gw);  //默认网关
   getSNfromDHCP(gWIZNETINFO.sn);  //子网掩码
   getDNSfromDHCP(gWIZNETINFO.dns);//DNS服务器
   gWIZNETINFO.dhcp = NETINFO_DHCP;
   
   /* Network initialization */
   network_init();//应用DHCP分配的网络地址进行网络初始化
   UART_Printf("DHCP LEASED TIME : %d Sec.\r\n", getDHCPLeasetime());//打印租得的DHCP网络地址时间

   DNS_Analyse();//域名解析
}


/******************************************************************************
 * @brief  Network Init
 * Intialize the network information to be used in WIZCHIP
 *****************************************************************************/
void network_init(void)
{
    uint8_t tmpstr[6] = {0};
    wiz_NetInfo netinfo;

    /*根据gWIZNETINFO结构体设置网络信息*/
    //DNS服务在gWIZNETINFO变量中必须设置正确，否则可能无法正确分析域名。
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

    /*获取设置后的网络信息放入netinfo结构体*/
    ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

    /* 串口打印网络信息 */
    ctlwizchip(CW_GET_ID,(void*)tmpstr);
    if(netinfo.dhcp == NETINFO_DHCP) 
	{
		UART_Printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
		W5500_DHCP_Flag = 1;
	}
    else 
	{
		UART_Printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);
		W5500_DHCP_Flag = 0;
	}
    UART_Printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
                    netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
    UART_Printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
    UART_Printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
    UART_Printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
    UART_Printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
    UART_Printf("===========================\r\n");
	 
}

/*将SPI接口函数注册到W5500的socket库中*/
static void RegisterSPItoW5500(void)
{
  /* 1.注册临界区函数 */
  reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);
  
  /* 2.注册SPI片选信号函数 */

  reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
      
  /* 3.注册读写函数 */
  reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);
}

/*初始化W5500网络芯片:直接调用官方提供的初始化库*/
static void InitW5500SocketBuf(void)
{
  /* WIZCHIP SOCKET Buffer initialize */
  if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){
      UART_Printf("WIZCHIP Initialized fail.\r\n");
//      while(1);
	  NVIC_SystemReset();                                //重启
  }
}

/* PHY链路状态检查*/
static void PhyLinkStatusCheck(void)
{
	uint8_t tmp;
	uint8_t Count_Over = 0;
	__disable_interrupt();

	do{
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){
			UART_Printf("Unknown PHY Link stauts.\r\n");
		}
		if(tmp == PHY_LINK_OFF){
			UART_Printf("PHY Link OFF!!!\r\n");//如果检测到，网线没连接，提示连接网线
			delay_ms(2000);              //延时2s
			Count_Over++;
			//30s 没有连接PHY则说明是转到了GPRS
			if (Count_Over >= 10)
			{
				W5500_NOPHY_TryGPRS_Flag = 1;
				UART_Printf("\r\n NET Change to GPRS \r\n"); 
				para_read(STORE_PAGE,&MQTT_Save_Data);
				UART_Printf("\r\n MQTT 参数获取！ \r\n"); 
				break;
			} 
		}
	}while(tmp == PHY_LINK_OFF);

	__enable_interrupt();
	if (!W5500_NOPHY_TryGPRS_Flag)
	{
		UART_Printf(" PHY Link Success.\r\n");
	} 
}


/*DHCP初始化*/
static void DhcpInitHandler(void)
{
   /* >> DHCP Client 				  */
  /************************************************/
  
  //must be set the default mac before DHCP started.
  setSHAR(gWIZNETINFO.mac);
		
  DHCP_init(SOCK_DHCP, gDATABUF);//gDATABUF获取数据的缓冲区
  
  // if you want defiffent action instead defalut ip assign,update, conflict,
  // if cbfunc == 0, act as default.
  //参数:分配的IP、更新的IP、冲突的IP
  reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
}

/*动态分配IP地址*/
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
	if(my_dhcp_retry > MY_MAX_DHCP_RETRY)//动态IP分配失败大于十次，将使用静态的IP
	{
//		UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
		gWIZNETINFO.dhcp = NETINFO_STATIC;
		my_dhcp_retry = 0;
		DHCP_stop();// if restart, recall DHCP_init()
		network_init();// apply the default static network and print out netinfo to serial
		//DNS_Analyse();//域名解析	
	}
    
    break;
  }
  return dhcp_res;
}


/*动态分配IP地址*/
void DhcpRunInLoop(void)
{
  static uint8_t my_dhcp_retry = 0;
//  char value[16]={0};
  int32_t t=0;
  
  
  switch(DHCP_run())
  {
    case DHCP_IP_ASSIGN://DHCP IP分配
    case DHCP_IP_CHANGED://DHCP IP更新
        /* If this block empty, act with default_ip_assign & default_ip_update */
        // This example calls my_ip_assign in the two case.
        break;
    
    case DHCP_IP_LEASED://TO DO YOUR NETWORK APPs.
        //在租得的DHCP网络地址时间内执行网络App
        //yeelink_get("19610","34409",value);//×￠òaDèòaDT??3é×??oyeelinkéè±?μ?ID--LED1
////        yeelink_get("19657","34484",value);//×￠òaDèòaDT??3é×??oyeelinkéè±?μ?ID--LED2
////        UART_Printf("%s\n\r",value);	//char value[16]={0};
////        UART_Printf("\n\r");
	  	MQTT_Init();
//        for(t=0;t<11;t++){
//          delay_ms(1000);
//        } 
        break;
    
    case DHCP_FAILED://动态IP分配失败
        /* ===== Example pseudo code =====  */
        // The below code can be replaced your code or omitted.
        // if omitted, retry to process DHCP
        my_dhcp_retry++;
        if(my_dhcp_retry > MY_MAX_DHCP_RETRY)//动态IP分配失败大于十次，将使用静态的IP
        {
            UART_Printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
            my_dhcp_retry = 0;
            DHCP_stop();// if restart, recall DHCP_init()
            network_init();// apply the default static network and print out netinfo to serial
            DNS_Analyse();//域名解析
        }
        break;
        
    default:
        break;
  }  
}

/*DNS解析*/
void DNS_Analyse(void)
{
    int32_t ret = 0;
  
    /*初始化DNS解析程序，并通过调用相关函数获取到machtalk服务器的真实IP地址*/
    /* DNS client initialization */
    DNS_init(SOCK_DNS, gDATABUF);
//    Timer_Start();/*启动Timer3*/
    /* DNS procssing */
    if ((ret = DNS_run(gWIZNETINFO.dns, domain_name, domain_ip)) > 0){ // try to 1st DNS
        UART_Printf("> 1st DNS Reponsed\r\n");
    }else if(ret == -1){
        UART_Printf("> MAX_DOMAIN_NAME is too small. Should be redefined it.\r\n");
//        Timer_Stop();/*关闭Timer3*/
        while(1);
    }else{
        UART_Printf("> DNS Failed\r\n");
//        Timer_Stop();/*关闭Timer3*/
        while(1);
    }

    //DNS解析成功：IP地址存储在domain_ip变量中，后面的socket编程会用到这个变量值。
    if(ret > 0){
        UART_Printf("> Translated %s to %d.%d.%d.%d\r\n",domain_name,domain_ip[0],domain_ip[1],domain_ip[2],domain_ip[3]);
    }
//    Timer_Stop();
}


/*配置W5500网络*/
int NetworkInitHandler(void)
{
	int rc = 0;
	//重新设置MAC地址
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
	//读取flash的数据（TCP）
	Read_Flash_TCP_Data();
	W5500_Reset();
    RegisterSPItoW5500();/*将SPI接口函数注册到W5500的socket库中*/
    InitW5500SocketBuf();/*初始化W5500网络芯片:直接调用官方提供的初始化库*/
    PhyLinkStatusCheck();/* PHY链路状态检查*/
	if (!W5500_NOPHY_TryGPRS_Flag)
	{
		DhcpInitHandler();   /*DHCP初始化*/
	}
	else
	{
		//初始化GPRS
		UART_Printf("GPRS INIT \r\n");
	}
//	rc = PhyLinkStatusCheck();/* PHY链路状态检查*/
	NET_INIT_EVENT = 1;
	return rc;
}
 

/**
 * @name: Unpack_json_MQTT_ResvData
 * @test: test font
 * @msg:  解构JSON数据，把关键信息提取出来	
 * @param {type} 
 * @return: 
 */
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
	//获取数据长度
	Str_len = strlen(ResvData);
	UART_Printf("Str_len of ResvData : %d\r\n" , Str_len);
	//获取前几位的数据
	strncpy(TempBuffer_Json,ResvData,5); 

	//Alarm 订阅解析
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
			UART_Printf("AlarmData ：%s\n", MQTT_Resv_AlarmData);//json_value_1->valuestring);

		}  
	}

	//Read_Data 订阅解析(OK)
	if( strchr(TempBuffer_Json,'R') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Read_data");

		if(json_value->type == cJSON_Number)
		{
			MQTT_Resv_Read_data = json_value->valueint;
			UART_Printf("Read_data: %d\n", MQTT_Resv_Read_data); 
		}  
		 
	}	 
	//Updata 订阅解析
	if( strchr(TempBuffer_Json,'C') != NULL)
	{
		json_value = cJSON_GetObjectItem(json, "Cycle"); 
		store_clear(STORE_PAGE);
		if(json_value->type == cJSON_Number)
		{
			 // 上报周期，单位分钟，1~255(OK)
			MQTT_Resv_Cycle = json_value->valueint;
			MQTT_Save_Data.MQTT_Resv_Cycle = MQTT_Resv_Cycle; 
			UART_Printf("Cycle: %d\n", MQTT_Resv_Cycle);
		} 

		json_value_1 = cJSON_GetObjectItem(json, "AlarmTime"); 
		if(json_value_1->type == cJSON_Number)
		{	
			// 报警持续时间，单位分钟，0~255，0不报警，255持续报警 
			MQTT_Resv_AlarmTime = json_value_1->valueint;
			MQTT_Save_Data.MQTT_Resv_AlarmTime = MQTT_Resv_AlarmTime; 
			UART_Printf("AlarmTime: %d\n", MQTT_Resv_AlarmTime);
		}  

		json_value_2 = cJSON_GetObjectItem(json,"Channel");
		if(json_value_2->type == cJSON_Number)
		{
			// 工作信道
			MQTT_Resv_Channel = json_value_2->valueint;
			MQTT_Save_Data.MQTT_Resv_Channel = MQTT_Resv_Channel; 
			UART_Printf("Channel ：%d\n", MQTT_Resv_Channel);
		}   

		json_value_3 = cJSON_GetObjectItem(json,"SensorNum");
		if(json_value_3->type == cJSON_Number)
		{
			// 1-240 该数传设备下面的采集模块数量(OK?)
			MQTT_Resv_SensorNum = json_value_3->valueint;
			 MQTT_Save_Data.MQTT_Resv_SensorNum = MQTT_Resv_SensorNum; 
			UART_Printf("SensorNum ：%d\n", MQTT_Resv_SensorNum);
		}
		
		json_value_4 = cJSON_GetObjectItem(json,"SensorCycle");
		if(json_value_4->type == cJSON_Number)
		{
			// 传感器上报周期，单位分钟，10~255，最低10分钟 
			// 将数据本地保存，防止无线没收到时数据丢失
			MQTT_Resv_SensorCycle = json_value_4->valueint;
			MQTT_Save_Data.MQTT_Resv_SensorCycle = MQTT_Resv_SensorCycle; 
			UART_Printf("SensorCycle ：%d\n", MQTT_Resv_SensorCycle);
		}	
		//存储
		para_store(STORE_PAGE,MQTT_Save_Data); 		

		MQTT_Save_Data.MQTT_Resv_Cycle 			= 0;
		MQTT_Save_Data.MQTT_Resv_AlarmTime 		= 0;
		MQTT_Save_Data.MQTT_Resv_Channel 		= 0;
		MQTT_Save_Data.MQTT_Resv_SensorNum 		= 0;
		MQTT_Save_Data.MQTT_Resv_SensorCycle 	= 0;
		MQTT_Save_Data.POWER_ON_COUNT			= 1;
	
	}

    cJSON_Delete(json); 
	 
    return 0;
}

// MQTT 订阅  全部回调函数
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
	
	//JSON数据解包赋值
	Unpack_json_MQTT_ResvData(RecivePkg.t.databuf);

	RecivePkg.t.is_process_data = true; 
 
}
 
 
 
//解包函数
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
	
	UART_Printf("准备连接服务器\r\n");                //串口输出信息
	__disable_interrupt();
	
	
	//domain_ip[0] = 47;
	//domain_ip[1] = 98;
	//domain_ip[2] = 136;
	//domain_ip[3] = 66; 
	
	ConnectNetwork(&network, IP_Data, Port_Data);
	UART_Printf("IP地址: %d.%d.%d.%d,%d\r\n", IP_Data[0],IP_Data[1],IP_Data[2],IP_Data[3],Port_Data);

	MQTTClientInit(&mqttclient,&network,1000,SendBuffer,1200,tempBuffer,10);
	
	delay_ms(500);
	
	// 订阅MQTT经纪人
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	connectData.willFlag = 0;
	connectData.MQTTVersion = 3;
//	connectData.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	connectData.clientID.cstring = "w5500-client";    //opts.clientid;
	connectData.username.cstring = ""; //opts.username;admin
	connectData.password.cstring = ""; //opts.password; public

	connectData.keepAliveInterval = 60;
	connectData.cleansession = 1;
	
	if ((rc = MQTTConnect(&mqttclient, &connectData)) != 0)
		UART_Printf("Return code from MQTT connect is %dr\n", rc);
	else
		UART_Printf("MQTT Connected\r\n", rc);

	// 订阅主题 Updata Alarm Read_data
	sprintf(SubTopic,"/WSN-LW/");
	strcat(SubTopic, Read_ID);
	strcat(SubTopic, "/service/+");// 订阅参数更新
	
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&mqttclient, SubTopic, QOS0, messageArrived);// /# 订阅一切 
	UART_Printf("Subscribed %d\r\n", rc);
 
	
	rc = MQTT_HeartBeat();	// 发送心跳包

	UART_Printf("MQTT Heartbeat Resulu %d \r\n",rc);       	         //串口输出信息
	para_read(STORE_PAGE,&MQTT_Save_Data);
	//清除NRF24L01数组中的数据
	Clear_ALL_nrf24l01_TempData(); 
	__enable_interrupt();
	return rc;
}

/**
 * @name: Creat_json_MQTT_SendData
 * @test: test font
 * @msg: 创建一个json格式的数据上传的数据格式
 * @param {Pub_State}  0 : SendDate 1 : Heartbeat
 * @return: char * json数据
 */
size_t HeartBeat_lenght = 0;
size_t SendData_lenght = 0; 
uint16_t Pack_Num_Last = 0;		//传感器少于40
uint8_t DataRiver[(PacksSensorNum * 10 )+ 1]  ;  
char * Creat_json_MQTT_SendData(uint8_t Pub_State,uint8_t Pack_NUM)
{
	cJSON * usr; 
	uint8 * out ;
	uint32_t NewTime = 0;
	usr = cJSON_CreateObject(); //创建根数据对象
	uint8_t DataRiver_TempForLess[(PacksSensorNum * 10 )+ 1]  ; 
	switch (Pub_State)
	{
		 //MQTT数据上报-传感器大于40个
	case MQTT_Publish_Type_SendData:
		/* code */
		//若传感器数量错误，给一个默认值
		if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;} 

		memset(DataRiver,0x00, 401 );
		memcpy(DataRiver,DataToSendBuffer + ((PacksSensorNum * 10) * (Pack_NUM-1)) ,(PacksSensorNum * 10)); 
		//字符串休止'/0'
		DataRiver[(PacksSensorNum * 10 )] = '\0' ;

    	cJSON_AddItemToObject(usr, "SensorNum", cJSON_CreateNumber(PacksSensorNum));
		cJSON_AddItemToObject(usr, "SensorStart", cJSON_CreateNumber(((Pack_NUM -1)*PacksSensorNum )+ 1));
		cJSON_AddItemToObject(usr, "SensorData", cJSON_CreateString(DataRiver)); 

		//out = cJSON_Print(usr); //将json形式打印成正常字符串形式
		out = cJSON_PrintUnformatted(usr); //将json形式打印成正常字符串形式
		
		UART_Printf("json Data : %s\r\n",out);
		
		UART_Printf("\r\nDataToSendBuffer : %s \r\n",DataRiver)  ;
		
		SendData_lenght = strlen(out)  ; 
 
		UART_Printf("\r\n SendData_lenght : %d \r\n" , SendData_lenght);

		// 释放内存 
		cJSON_Delete(usr); 
		free(out);

		return out;  
	//MQTT心跳数据上报
	case MQTT_Publish_Type_HeartBeat: 
		
		NewTime = GetSystemNowtime();
		 
		cJSON_AddItemToObject(usr,"Heartbeat",cJSON_CreateNumber(NewTime));

		out = cJSON_PrintUnformatted(usr); //将json形式打印成正常字符串形式

		UART_Printf("json Data : %s\n",out);
		// 释放内存
		HeartBeat_lenght = strlen(out);
		UART_Printf("HeartBeat_lenght : %d \r\n" , HeartBeat_lenght);

		cJSON_Delete(usr); 
		free(out);
		
		return out;  
	//传感器数量少于40
	case MQTT_Publish_Type_CountLess40 :
		//若传感器数量错误，给一个默认值
		UART_Printf("PacksSensorNum Less than 40 \r\n ");
		if(MQTT_Resv_SensorNum == 0){MQTT_Resv_SensorNum = SensorNum;}

		memset(DataRiver_TempForLess,0x00,Pack_Num_Last*10);
		memcpy(DataRiver_TempForLess,DataToSendBuffer + ((Pack_Num_Last * 10) * (Pack_NUM-1)) ,(Pack_Num_Last * 10)); 
		//字符串休止'/0'
		DataRiver_TempForLess[(Pack_Num_Last * 10 )] = 0x00 ;

    	cJSON_AddItemToObject(usr, "SensorNum", cJSON_CreateNumber(Pack_Num_Last));
		cJSON_AddItemToObject(usr, "SensorStart", cJSON_CreateNumber(((Pack_NUM -1)*Pack_Num_Last )+ 1));
		cJSON_AddItemToObject(usr, "SensorData", cJSON_CreateString(DataRiver_TempForLess)); 

		out = cJSON_PrintUnformatted(usr); //将json形式打印成正常字符串形式
		
		UART_Printf("json Data : %s\r\n",out);
		
		UART_Printf("\r\n DataRiver_TempForLess : %s \r\n",DataRiver_TempForLess)  ;
		
		SendData_lenght = strlen(out) ; 
		 
		UART_Printf("\r\nSendData_lenght : %d \r\n" , SendData_lenght);

		// 释放内存 
		cJSON_Delete(usr); 
		free(out); 
		return out;
	default:

		break;
	}  
}

//心跳包处理函数
int MQTT_HeartBeat(void)
{
	int rc = 0;
	
	static uint16_t mes_id = 0;
	
	sprintf(Topic,"/WSN-LW/");
	strcat(Topic, Read_ID);
	strcat(Topic, "/event/Heartbeat");
	
	
	heartbeat.protocoltype = "w5500";
	heartbeat.heartbeat = mes_id;
	heartbeat.producttype = "WSN-LW";
	heartbeat.softversion = "1.0.1";
	heartbeat.configtag = 1;
	  
	mqtt_msg.qos = QOS0;
	mqtt_msg.retained = 0;
    mqtt_msg.id = mes_id++;
    mqtt_msg.dup = 0;
	mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat,0);
	mqtt_msg.payloadlen = HeartBeat_lenght;
	
	rc = MQTTPublish(&mqttclient, Topic, &mqtt_msg);
	UART_Printf("Publish %s\r\n", Topic);
	//IR_SendData(	Creat_json_MQTT_SendData(MQTT_Publish_Type_HeartBeat)
	//				,HeartBeat_lenght);
	UART_Printf("\r\nPublish %d\r\n", rc);
	UART_Printf( "HeartBeating............" ); 
	return rc;

}


//数据包发送处理函数
int MQTT_SendData(void)
{
	int rc = 0; 
	static uint16_t mes_id = 0; 
	DOUBLE_LINK_NODE *pNode;
	uint8_t tag_cnt	 = 0;
	uint8_t Pack_Num = 0;  
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
	DISABLE_GLOBAL_INTERRUPT();
	//分包发送
	for (uint8_t i = 1; i <= Pack_Num; i++)
	{	
		sprintf(Topic,"/WSN-LW/");
		strcat(Topic, Read_ID);
		strcat(Topic, "/event/Data");
		
		// init_user(&userInfo);
		userInfo.protocoltype = "w5500";
		userInfo.n_cardlist = tag_cnt;
		userInfo.cardlist = cardInfo_p;
		
		pNode = (&RADIO_DATA_LIST_HEAD)->next;  
		mqtt_msg.qos = QOS0;
		mqtt_msg.retained = 0;
		mqtt_msg.id = mes_id++;
		mqtt_msg.dup = 0;	 

		if (MQTT_Resv_SensorNum >= 40)
		{  
			mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_SendData,i) ;   
		}
		else
		{
			mqtt_msg.payload = Creat_json_MQTT_SendData(MQTT_Publish_Type_CountLess40,i) ; 
		}

		mqtt_msg.payloadlen = SendData_lenght ;//lenght;

		rc = MQTTPublish(&mqttclient, Topic, &mqtt_msg);

		UART_Printf("Publish %s\r\n", Topic);
		//IR_SendData(Buffer,SendData_lenght);

		UART_Printf("\r\nPublish %d\r\n", rc);

		UART_Printf("Sending.........");  

		nrf_delay_ms(1500);
	}  

	ENABLE_GLOBAL_INTERRUPT();
	return rc;
}

int MQTT_Working(void)
{
	process_message_control();
	return MQTTYield(&mqttclient, 60);
    
	
}
/* 循环获取开关量的值，并将获取到的结果提交出来并串口打印输出，当然也可以根据这个值来
 * 控制LED灯，循环获取间隔时间必须大于10s，这个是yeelink限制的。
 */
uint8_t yeelink_get(const char *device_id,const char *sensors_id,char *value)
{
////    int ret;
////    char* presult;
//////    char remote_server[] = "api.yeelink.net";
////	char remote_server[] = "server.yun-ran.com";
////    char str_tmp[128] = {0};
////
////    // 请求缓冲区和响应缓冲区
////    static char http_request[DATA_BUF_SIZE] = {0};	//声明为静态变量，防止堆栈溢出
////    static char http_response[DATA_BUF_SIZE] = {0};	//声明为静态变量，防止堆栈溢出
//////    sprintf(str_tmp,"/v1.0/device/%s/sensor/%s/datapoints",device_id,sensors_id);
////	sprintf( str_tmp , "/siot/centers/password/%s","12346666");
//////	sprintf( str_tmp , "/1.txt");
////    // 确定 HTTP请求首部
////    // 例如POST /v1.0/device/98d19569e0474e9abf6f075b8b5876b9/1/1/datapoints/add HTTP/1.1\r\n
////    sprintf( http_request , "GET %s HTTP/1.1\r\n",str_tmp);
////
////
////    // 增加属性 例如 Host: api.machtalk.net\r\n
////    sprintf( str_tmp , "Host:%s\r\n" , remote_server);
////    strcat( http_request , str_tmp);
////
////    // 增加密码 例如 APIKey: d8a605daa5f4c8a3ad086151686dce64
////    //sprintf( str_tmp , "U-ApiKey:%s\r\n" , "d8a605daa5f4c8a3ad086151686dce64");//需要替换为自己的APIKey
////    sprintf( str_tmp , "U-ApiKey:%s\r\n" , "e5da11d13d2e5f540ef1a99b3506e081");//APIKey--qinfei
////    strcat( http_request , str_tmp);
////    //
////    strcat( http_request , "Accept: */*\r\n");
////    // 增加表单编码格式 Content-Type:application/x-www-form-urlencoded\r\n
////    strcat( http_request , "Content-Type: application/x-www-form-urlencoded\r\n");
////    strcat( http_request , "Connection: keep-alive\r\n");
////    // HTTP首部和HTTP内容 分隔部分
////    strcat( http_request , "\r\n");
////
////    //将数据通过TCP发送出去
////    //新建一个Socket并绑定本地端口5000
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
////    //连接TCP服务器
////    ret = connect(SOCK_TCPS,domain_ip,80);
////    if(ret != SOCK_OK){
////        UART_Printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
////        while(1);
////    }	
//////	sprintf( http_request , "GET https://server.yun-ran.com/siot/centers/password/%s HTTP/1.1\r\n","12346666");
////    //发送请求
////    ret = send(SOCK_TCPS,(unsigned char *)http_request,strlen(http_request));
////    if(ret != strlen(http_request)){
////        UART_Printf("%d:Socket Send Error\r\n",SOCK_TCPS);
////        while(1);
////    }
////
////    // 获得响应
////    ret = recv(SOCK_TCPS,(unsigned char *)http_response,DATA_BUF_SIZE);
////    if(ret <= 0){
////        UART_Printf("%d:Socket Get Error\r\n",SOCK_TCPS);
////        while(1);
////    }
////    http_response[ret] = '\0';
////	
////	/*****************/
////	//不支持https请求，直接用http请求返回结果如下
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
////	//判断是否收到HTTP OK
////    presult = strstr( (const char *)http_response , (const char *)"200 OK\r\n");
////    if( presult != NULL ){
////        static char strTmp[DATA_BUF_SIZE]={0};//声明为静态变量，防止堆栈溢出
////        sscanf(http_response,"%*[^{]{%[^}]",strTmp);
////        //提取返回信息
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

