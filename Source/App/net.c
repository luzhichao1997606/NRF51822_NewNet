/*-------------------------------------------------*/
/*                                                 */
/*            实现网络功能的源文件             */
/*                                                 */
/*-------------------------------------------------*/

#include "hal.h"    //包含需要的头文件
#include "string.h"       //包含需要的头文件
#include "stdio.h"        //包含需要的头文件

#include "net.h"          //包含需要的头文件

#include "wizchip_conf.h" //包含需要的头文件
#include "socket.h"       //包含需要的头文件
#include "dhcp.h"         //包含需要的头文件
#include "dns.h"          //包含需要的头文件

#include "MQTTClient.h"

#include "radio_config.h"

//#include "wizchip_conf.h"
//#include "socket.h"
//#include "w5500.h"
//#include "dhcp.h"         //包含需要的头文件

#include "nrf_nvmc.h"
//#include "SPI1.h"
#include "Uart.h"

#include "cJSON.h"

uint8 Module_Info[] = {"WSN-H04 V1.0 20191202"};

uint8_t Read_ID[16] = {"4001000726123480"}; 
unsigned char  gDATABUF[DATA_BUF_SIZE];    //数据缓冲区
//unsigned char  gDNSBUF[DATA_BUF_SIZE];     //数据缓冲区
char DNS_flag = 0;              //DNS解析标志  0:未成功  1：成功
int  tcp_state;                 //用于保存TCP返回的状态
int  temp_state;                //用于临时保存TCP返回的状态
char my_dhcp_retry = 0;         //DHCP当前共计重试的次数


uint16_t   m_DEVICE_ID = 0x0315;
netpara_t            m_netpara;            //网络参数
para_cfg_t           m_cfgpara;

uint8_t              m_rssi_Calibration;
uint8_t              m_factor;


unsigned char tempBuffer[2048];
unsigned char SendBuffer[3072]= {"{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":3132213B2470E251012401020304}"};



uint8 Work_Flag = 0;	// 开始正常工作标志
uint8 Phy_Link_Flag = 0xff;	// 网线连接状态


uint16 check_time;
uint16 work_time;
uint8 m2m_status = 0;	// M2M模块状态

#define WORK_INIT			0
#define WORK_DHCP			1
#define WORK_TCP           	2
#define WORK_TCP_FATAL     	3
#define WORK_M2M_nPHY		4
#define WORK_M2M_nDHCP		5
#define WORK_M2M_nTCP       	6

uint16 PHY_time = 0xFFFF;
uint16 DHCP_time = 0xFFFF;
uint16 TCP_time = 0xFFFF;
uint16 MQTT_time = 0xFFFF;

uint8_t m_data_sendbuf[1356];

wiz_NetInfo gWIZNETINFO =       //MAC地址自己写一个，不要和路由器下其他的设备一样即可
{        
0x00, 0x08, 0xdc,0x00, 0xab, 0xcd, 
}; 
unsigned char dns2[4]={114,114,114,114};  //DNS解析服务器IP


unsigned char ServerIP[4];                                   //存放服务器IP
unsigned char ServerName[128] = {"mqtt.dawen.ltd"};                               //存放服务器域名
//int  ServerPort = 7611;                                             //存放服务器的端口号

MQTTClient c;
MQTTMessage mqtt_msg;
char SubTopic[120] = "/WSN-LW/xxxxxxxx/service/xxxxx";
char SubTopic0[40] = "/WSN-LW/xxxxxxxx/service/xxxxx";
char SubTopic1[40] = "/WSN-LW/xxxxxxxx/service/xxxxx";
char SubTopic2[40] = "/WSN-LW/xxxxxxxx/service/xxxxx";

char PubTopic[40] = "/WSN-LW/xxxxxxxx/event/xxxxx";
MQTT_Param_t		MQTT_Param;
FLASH_Param_t		FLASH_Param;

void my_ip_assign(void);
void my_ip_conflict(void);
void network_init(void);

void Rf_Working(void);
void send_heartpacket(void);
void send_datapacket(void);

void NetData_processing(uint8_t *rcvdata,uint16_t len);

void setkeepalive(SOCKET s);

void MQTT_Send_task(void);
void MQTT_Package_msg(uint8 *payload, uint8 StartNum, uint16 Num);
void MQTT_Package_Heartbeat(uint16 sn, uint8 resetFlag);

/*默认网络参数*/
const  netpara_t default_netpara =
{
	.server_ip[0]=101,                //服务器默认IP
	.server_ip[1]=37,
	.server_ip[2]=13,
	.server_ip[3]=31,

	.server_port=1883,						         //服务器默认端口号
	
//	.server_ip[0]=121,                //服务器默认IP
//	.server_ip[1]=43,
//	.server_ip[2]=164,
//	.server_ip[3]=164,

//	.server_port=7211,						         //服务器默认端口号
	
	.local_Port=4120,					           //客户端默认端口号
  	.local_IP[0]=192,          //客户端默认IP
	.local_IP[1]=168,
	.local_IP[2]=3,
	.local_IP[3]=168,
	
  	.local_sunNet[0]=255,         //默认子网淹没
	.local_sunNet[1]=255,
	.local_sunNet[2]=255,
	.local_sunNet[3]=0,
	
	.mac[0] = 0x02,
	.mac[1] = 0x17,
	.mac[2] = 0x9a,
	.mac[3] = 0x68,
	.mac[4] = 0x00,
	.mac[5] = 0x01,
	
  	.local_GW[0]=192,              //默认网关
	.local_GW[1]=168,
	.local_GW[2]=3,
	.local_GW[3]=1,
	
	.dns[0]=192,              //默认网关
	.dns[1]=168,
	.dns[2]=3,
	.dns[3]=1,
};


////装载网络参数
////说明: 网关、子网掩码、物理地址、本机IP地址、本机端口号、目的IP地址、目的端口号、端口工作模式
void Load_Net_Parameters(void)
{
	gWIZNETINFO.mac[0]=m_netpara.mac[0];//加载物理地址
	gWIZNETINFO.mac[1]=m_netpara.mac[1];
	gWIZNETINFO.mac[2]=m_netpara.mac[2];
	gWIZNETINFO.mac[3]=m_netpara.mac[3];
	gWIZNETINFO.mac[4]=m_netpara.mac[4];
	gWIZNETINFO.mac[5]=m_netpara.mac[5];
	
	gWIZNETINFO.ip[0]=m_netpara.local_IP[0];//加载本机IP地址
	gWIZNETINFO.ip[1]=m_netpara.local_IP[1];
	gWIZNETINFO.ip[2]=m_netpara.local_IP[2];
	gWIZNETINFO.ip[3]=m_netpara.local_IP[3];
	
	gWIZNETINFO.sn[0]=m_netpara.local_sunNet[0];//加载子网掩码
	gWIZNETINFO.sn[1]=m_netpara.local_sunNet[1];
	gWIZNETINFO.sn[2]=m_netpara.local_sunNet[2];
	gWIZNETINFO.sn[3]=m_netpara.local_sunNet[3];
	
	gWIZNETINFO.gw[0] = m_netpara.local_GW[0];//加载网关参数
	gWIZNETINFO.gw[1] = m_netpara.local_GW[1];
	gWIZNETINFO.gw[2] = m_netpara.local_GW[2];
	gWIZNETINFO.gw[3] = m_netpara.local_GW[3];
	
	gWIZNETINFO.dns[0]=m_netpara.dns[0];//加载本机IP地址
	gWIZNETINFO.dns[1]=m_netpara.dns[1];
	gWIZNETINFO.dns[2]=m_netpara.dns[2];
	gWIZNETINFO.dns[3]=m_netpara.dns[3];
	
	ServerIP[0] = m_netpara.server_ip[0];
	ServerIP[1] = m_netpara.server_ip[1];
	ServerIP[2] = m_netpara.server_ip[2];
	ServerIP[3] = m_netpara.server_ip[3];
//	ServerPort	= m_netpara.server_port; 
	
	gWIZNETINFO.dhcp = NETINFO_STATIC;
}

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
	if(ppara->magic == STORE_MAGIC)
		 return;
	else
	{
		memcpy((uint8_t*)&(ppara->net_cfg),(uint8_t*)&default_netpara,sizeof(para_cfg_t));
		ppara->net_cfg.mac[2] = (NRF_FICR->DEVICEID[0]>>24)&0xFF;
		ppara->net_cfg.mac[3] = (NRF_FICR->DEVICEID[0]>>16)&0xFF;
		ppara->net_cfg.mac[4] = (NRF_FICR->DEVICEID[0]>>8)&0xFF;
		ppara->net_cfg.mac[5] = (NRF_FICR->DEVICEID[0])&0xFF;
		ppara->magic          = STORE_MAGIC;
		ppara->rssi_Calibration = 63;	// rssi 1m标定值
		ppara->factor	       = 32;	// 环境因子
		
		store_clear(STORE_PAGE);
		para_store(STORE_PAGE,*ppara);
	}
	return;
}

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


/*-------------------------------------------------*/
/*函数名：初始化W5500                              */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void W5500_init(void)
{
	//W5500收发内存分区，收发缓冲区各自总的空间是16K，（0-7）每个端口的收发缓冲区我们分配 2K
	char memsize[2][8] = {{2,4,2,2,2,2,2,0},{2,4,2,2,2,2,2,0}}; 
	char tmp;
	uint8_t old_flag = Work_Flag;
	W5500_Reset();   
	//W5500复位
	wiz_NetInfo netinfoqq;

	ctlnetwork(CN_GET_NETINFO, (void*)&netinfoqq);	//读取网络参数
	
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	//注册临界区函数
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  //注册SPI片选信号函数
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	//注册读写函数
	
	Load_Net_Parameters();
	
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){   //如果if成立，表示收发内存分区失败
		UART_Printf("初始化收发分区失败,准备重启\r\n");      //提示信息
		NVIC_SystemReset();                                //重启
	}	
	PHY_time = 10000;//10S
	do{                                                     //检查连接状态
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){ //如果if成立，表示未知错误		 
			UART_Printf("未知错误，准备重启\r\n");            //提示信息
			NVIC_SystemReset();                             //重启
		}
		if(tmp == PHY_LINK_OFF){
			UART_Printf("网线未连接\r\n");//如果检测到，网线没连接，提示连接网线
			delay_ms(2000);              //延时2s
		}
		nrf_drv_wdt_feed();
	}while((tmp == PHY_LINK_OFF) && (Work_Flag == old_flag));                            //循环执行，直到连接上网线
	if(tmp == PHY_LINK_ON)
	{
		Work_Flag = WORK_DHCP;
		PHY_time = 0xFFFF;
		DHCP_time = 30000;	//30s
	}
	setSHAR(gWIZNETINFO.mac);                                   //设置MAC地址
	DHCP_init(SOCK_DHCP, gDATABUF);                             //初始化DHCP
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);//注册DHCP回调函数 
	DNS_flag = 0;                                               //DNS解析标志=0 
	my_dhcp_retry = 0;	                                        //DHCP重试次数=0
	tcp_state = 0;                                              //TCP状态=0
	temp_state = -1;                                            //上一次TCP状态=-1
	
	setkeepalive(SOCK_TCPC);
//	disconnect(SOCK_TCPC);
}

void setkeepalive(SOCKET s)
{
  //IINCHIP_WRITE( Sn_CR(0) , Sn_CR_SEND_KEEP );  
  IINCHIP_WRITE(Sn_KPALVTR(s),0x01);
}

void send_ka(SOCKET s)
{
  IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND_KEEP);
  return;
}
/*-------------------------------------------------*/
/*函数名：W5500发送数据                            */
/*参  数：data:数据                                */
/*返回值：无                                       */
/*-------------------------------------------------*/
void W5500_TxData(unsigned char *data)
{
	send(SOCK_TCPC,&data[1],data[0]);  //W5500发送数据
}

/*-------------------------------------------------*/
/*函数名：DNS域名解析                              */
/*参  数：DomainName:域名                          */
/*返回值：无                                       */
/*-------------------------------------------------*/
void W5500_DNS(unsigned char *DomainName)
{
	char ret;
	
	UART_Printf("开始解析域名\r\n");                         //串口输出信息
//	DNS_init(SOCK_DNS, gDNSBUF);                         //DNS初始化
	DNS_init(SOCK_DNS, gDATABUF);                         //DNS初始化
	ret = DNS_run(dns2,DomainName,m_netpara.server_ip);               //解析域名
	UART_Printf("DNS解析返回值：%d\r\n",ret);                //串口输出数据
	if(ret==1){                                            //返回值等于1，表示解析正确
		UART_Printf("DNS解析成功：%d.%d.%d.%d\r\n",m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3]);//串口输出信息
	    DNS_flag = 1;                                      //DNS解析成功，标志=1
		close(SOCK_DNS);                                 //关闭DNS端口
	}
}

/*-------------------------------------------------*/
/*函数名：获取到IP时的回调函数                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void my_ip_assign(void)
{
	getIPfromDHCP(gWIZNETINFO.ip);     //把获取到的ip参数，记录到机构体中
	getGWfromDHCP(gWIZNETINFO.gw);     //把获取到的网关参数，记录到机构体中
	getSNfromDHCP(gWIZNETINFO.sn);     //把获取到的子网掩码参数，记录到机构体中
	getDNSfromDHCP(gWIZNETINFO.dns);   //把获取到的DNS服务器参数，记录到机构体中
	gWIZNETINFO.dhcp = NETINFO_DHCP;   //标记使用的是DHCP方式
	network_init();                    //初始化网络  
	UART_Printf("DHCP租期 : %d 秒\r\n", getDHCPLeasetime());
}
/*-------------------------------------------------*/
/*函数名：获取IP的失败函数                         */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void my_ip_conflict(void)
{
	UART_Printf("获取IP失败，准备重启\r\n");   //提示获取IP失败
	NVIC_SystemReset();                      //重启
}
/*-------------------------------------------------*/
/*函数名：初始化网络函数                           */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void network_init(void)
{
	char tmpstr[6] = {0};
	wiz_NetInfo netinfo;
	
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);//设置网络参数
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);	//读取网络参数
	ctlwizchip(CW_GET_ID,(void*)tmpstr);	        //读取芯片ID	"W5500"
	
	//打印网络参数
	if(netinfo.dhcp == NETINFO_DHCP) 
		UART_Printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
	else 
		UART_Printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);	
	UART_Printf("===========================\r\n");
	UART_Printf("MAC地址: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);			
	UART_Printf("IP地址: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	UART_Printf("网关地址: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	UART_Printf("子网掩码: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	UART_Printf("DNS服务器: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	UART_Printf("===========================\r\n");
}


/*******************************************************************************
 * Function Name: Net_Time_Work
 * Decription	: 定时器网络任务
 * Calls		: 
 * Called By	: TIMER2_IRQHandler()1ms定时器
 * Arguments	:   
 * Returns		: 
 * Others		: 注意事项
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
volatile uint16 heartTicks;
//uint8_t M2M_Send_Flag = 0;
uint8_t TCP_Send_Flag = 0;
uint8_t Connect_flag = 0;
uint16 delay_time;

uint16 SendTime;

//uint8 TCP_Connect = 0;

// LED状态
uint8_t LED_Flag = 0;
uint16 LED_Time;

uint8 SOCK_NOK_NUM = 0;	// socket失败重连次数

void Net_Time_Work(void)
{
//	char tmp;
	/*--------------------------------------------------------------------*/
	/*   Connect_flag=1同服务器建立了连接,我们可以发布数据和接收推送了    */
	/*--------------------------------------------------------------------*/
	if(Connect_flag==1)
	{
		heartTicks++;
		if(Work_Flag == WORK_TCP)
		{
			if(TCP_Send_Flag == 0)
			{
				TCP_Send_Flag = 1;
				LED_Flag = 1;	// LED闪灯标志
				SendTime = 0;	// 发送超时清零
				SOCK_NOK_NUM = 0;	
				TCP_time = 10000;
				send_datapacket();	//调用上报数据函数	
//				send_ka(SOCK_TCPC);
			}
			else
			{
				if(heartTicks >= 1000)	// 心跳计时到达心跳周期
				{
					disconnect(SOCK_TCPC);
					Connect_flag = 0;
					heartTicks = 0;		//清零心跳计时
				}
			}
		}
	}

	/******LED 灯闪烁 ********/
	if(LED_Flag == 1)	// 点灯标志
	{
		LED_Time++;
		if(LED_Time < 200)	// 点灯时间200ms
			nrf_gpio_pin_clear(DATA_LED);	// 点亮LED灯
		else
		{
			LED_Time = 0;
			LED_Flag = 0;
			nrf_gpio_pin_set(DATA_LED);		// 关闭LED灯
		}
		
	}
	/************************/
	
	
//	/******TCP连接超时检测 ********/
//	if(TCP_Connect == 1)
//	{
//		TCP_Time++;
//		if(TCP_Time > 5000)
//		{
//			TCP_Time = 0;
//			Work_Flag = 3;
//		}
//	}
//	/************************/
	
	// 初始化网线连接超时检测
	if((PHY_time != 0xFFFF)&& (PHY_time > 0))
	{
		PHY_time--;
		if(PHY_time == 0)
		{
			UART_Printf("WORK_M2M_nPHY\r\n");
			Work_Flag = WORK_M2M_nPHY;
		}
	}
	
	// DHCP超时检测
	if((DHCP_time != 0xFFFF)&& (DHCP_time > 0))
	{
		DHCP_time--;
		if(DHCP_time == 0)
		{
			UART_Printf("WORK_M2M_nDHCP\r\n");
			Work_Flag = WORK_M2M_nDHCP;
		}
	}
	
	// TCP超时检测
//	if((TCP_time != 0xFFFF)&& (TCP_time > 0))
//	{
//		TCP_time--;
//		if(TCP_time == 0)
//		{
//			UART_Printf("WORK_TCP_FATAL\r\n");
//			Work_Flag = WORK_TCP_FATAL;
//		}
//	}
	
	if((MQTT_time != 0xFFFF)&& (MQTT_time > 0))
	{
		MQTT_time--;
//		if(MQTT_time == 0)
//		{
//			UART_Printf("WORK_TCP_FATAL\r\n");
//			Work_Flag = WORK_TCP_FATAL;
//		}
	}
	
	/******TCP连接异常 等待重连60s ********/
	if(Work_Flag == WORK_TCP_FATAL)
	{
		delay_time++;
		if(delay_time <= 500)
		{
			delay_time = 501;
			UART_Printf("关闭所有连接\r\n");
			DHCP_stop();
			close(SOCK_TCPC);
		}
		else if(delay_time > 60000) //60s
		{
			SOCK_NOK_NUM++;
			if(SOCK_NOK_NUM > 5)
			{
				SOCK_NOK_NUM = 0;
				Work_Flag = WORK_M2M_nTCP;
			}
			else
				Work_Flag = WORK_INIT;
			delay_time = 0;
			UART_Printf("延时重新连接\r\n");
			
		}
		
	}
	/************************/
	
	/******MQTT 定时任务 ********/
	// todo 连接判断
//	MQTT_Send_task();
	/************************/
}


void MQTT_Send_task(void)
{
	MQTT_Param.time++;
	if(MQTT_Param.time > 60000) // 1分钟
	{
		uint8 i;
		MQTT_Param.time = 0;
		
		if( MQTT_Param.SerialNumber >= 65535 )
			MQTT_Param.SerialNumber = 20;
		
		MQTT_Param.SensorAll = 2;	// 1：全部上线
		
		for(i=0; i<FLASH_Param.SensorNum; i++)	// 传感器数据检测
		{
			SensorTime[i] += 1;	//传感器数据超时计时+1
			if(SensorTime[i] > FLASH_Param.SensorCycle)	//传感器数据超时清零判断，大于两个传感器上报周期，数据清零
			{
				SensorTime[i] = 0;
				if(comMEMCMPByte(SensorData[i], 0, 5)==1)	//存在非零数据
				{
					memset(SensorData[i], 0, 5);
					MQTT_Param.OfflineFlag = 1;	//设置离线状态
				}
			}
		}
		MQTT_Param.SerialNumber += 1;	// 心跳包序号+1	
		
		if( (MQTT_Param.SerialNumber%FLASH_Param.Cycle != 0 || MQTT_Param.SerialNumber <= 20) && MQTT_Param.OfflineFlag == 0 )		// 发送心跳数据包
		{
//		msis:send_Heartbeat_data(SerialNumber,ResetFlag)
//		MQTT_Param.ResetFlag = 0; //清除复位状态	
			MQTT_Package_Heartbeat(MQTT_Param.SerialNumber, 1);
			FLASH_Param.ResetFlag = 0;	//清除复位状态	
		}
		else if( (MQTT_Param.SerialNumber%FLASH_Param.Cycle == 0 && MQTT_Param.SerialNumber > 20) || MQTT_Param.OfflineFlag == 1 )		// 发送数据包，上电前20分钟不上报数据
		{
//		StartSend()
			MQTT_Package_msg(SensorData[0],1,20);
			MQTT_Param.OfflineFlag = 0;	//清除离线状态
			
		}
	
	}
}

void SendTimeOut(void)
{
	SendTime++;
	if(SendTime > 360)//6分钟没有发送数据
	{
		UART_Printf("发送超时，准备重启\r\n");     //串口输出信息
		NVIC_SystemReset();	                       //重启
	}
		
}

uint16 Check_NUM = 0;
uint8 Phy_Link_Flag_st = 0xff;
void Phy_Link_Check(void)
{
	char tmp;
	if(Work_Flag >= WORK_M2M_nPHY)
	{
		Check_NUM++;
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
		{ //如果if成立，表示未知错误		 
			UART_Printf("未知错误，准备重启\r\n");            //提示信息
			NVIC_SystemReset();                             //重启
		}
		if(tmp == PHY_LINK_OFF)
		{
			if(Phy_Link_Flag_st != tmp)
			{
				Check_NUM = 0;
				Phy_Link_Flag_st = PHY_LINK_OFF;
			}
			if(Check_NUM > 5)
			{
//				UART_Printf("网线未连接\r\n");//如果检测到，网线没连接，提示连接网线
				Phy_Link_Flag = PHY_LINK_OFF;	// 网线未连接
			}
			
			
		}
		else if(tmp == PHY_LINK_ON)
		{
			if(Phy_Link_Flag_st != tmp)
			{
				Check_NUM = 0;
				if(Phy_Link_Flag_st == PHY_LINK_OFF)
					Work_Flag = WORK_INIT;	// 进入w5500初始化
				Phy_Link_Flag_st = PHY_LINK_ON;
			}
			if(Check_NUM > 5)
			{
//				UART_Printf("网线连接\r\n");//如果检测到，网线没连接，提示连接网线
				Phy_Link_Flag = PHY_LINK_ON;	// 网线连接
				if((Check_NUM > 300) && (Work_Flag >= WORK_M2M_nDHCP)) // 5分钟重新检测一次网线连接
				{
					UART_Printf("重新检测一次网线连接\r\n");
					Check_NUM = 0;
					Work_Flag = WORK_INIT;	// 进入w5500初始化
				}
			}
		}
	}
}

/*******************************************************************************
 * Function Name: RadioData_NetSend
 * Decription	: 数据报发送
 * Calls		: 
 * Called By	: 
 * Arguments	:   
 * Returns		: 
 * Others		: 注意事项
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
void RadioData_NetSend(void)
{
	return;
}


/*******************************************************************************
 * Function Name: send_heartpacket
 * Decription	: 心跳包发送
 * Calls		: 
 * Called By	: 定时器任务
 * Arguments	:   
 * Returns		: 
 * Others		: 注意事项
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
void send_heartpacket(void)
{
////	uint8_t  heart_data[80];
////	uint16_t check_sum = 0;
////	static uint8_t sn = 0;
////	UART_Printf("发送数据\r\n");
//////	memcpy(heart_data,"$R,P,H,460,00,573E,6FE4,0,0,460044479509898,3001000726123450",60);
////	memcpy(heart_data,"$R,I,D,460,00,1806,3201,0,0,460040883305431,990140123456700501000",65);
////	send(SOCK_TCPC,heart_data,65);
//	
//	
//	uint8_t  heart_data[80];
//	memcpy(heart_data,"$R,P,H,0,0,0,0,0,0,0,",21);
////	hex2strid(&heart_data[21], id, 16);
//	memcpy(&heart_data[21],Read_ID,16);
//	memcpy(&heart_data[37],"\r\n",2);
//	send(SOCK_TCPC,heart_data,39);
	
}

/*******************************************************************************
 * Function Name: send_datapacket
 * Decription	: 数据包发送
 * Calls		: 
 * Called By	: 定时器任务
 * Arguments	:   
 * Returns		: 
 * Others		: 注意事项
 ****************************** Revision History *******************************
 * Version		Date		Modified By		Description               
 * V1.0			2019/10/31				
 ******************************************************************************/
void send_datapacket(void)
{
//	DOUBLE_LINK_NODE *pNode;
//	uint32_t cnt       = 0;
//	uint8_t tag_cnt    = 0;
//	uint8_t i = 0;
//	
//	
//	tag_cnt = count_number_in_double_link(&RADIO_DATA_LIST_HEAD);
//	if(tag_cnt ==0)
//	{
//		UART_Printf("发送心跳\r\n");
//		memcpy(m_data_sendbuf,"$R,P,H,0,0,0,0,0,0,0,",21);
//		//	hex2strid(&heart_data[21], id, 16);
//		memcpy(&m_data_sendbuf[21],Read_ID,16);
//		memcpy(&m_data_sendbuf[37],"\r\n",2);
//		send(SOCK_TCPC,m_data_sendbuf,39);
//		return;
//	}
//	if(tag_cnt >= 70) 
//		tag_cnt = 70;
////	xTimerStart(m_heart_timerout_handle,2);  //启动心跳超时定时器
//	heartTicks = 0;		//清零心跳计时
////	memset(m_data_sendbuf,0,sizeof(m_data_sendbuf));
//	UART_Printf("发送数据\r\n");
//	memcpy(m_data_sendbuf,"$R,P,D,0,0,0,0,0,0,0,",21);
////	hex2strid(&m_data_sendbuf[21], id, 16);
//	memcpy(&m_data_sendbuf[21],Read_ID,16);
//	m_data_sendbuf[37] = ',';
//	cnt = 38;
//	DISABLE_GLOBAL_INTERRUPT();
//	pNode = (&RADIO_DATA_LIST_HEAD)->next;
//	while(i++ < tag_cnt)
//	{	
//		hex2strid((char *)&m_data_sendbuf[cnt],pNode->data,(NET_RADIO_PACKET_LEN-2)<<1);
//		cnt += (NET_RADIO_PACKET_LEN-2)<<1; 
//		if(pNode->data[6] == 0)
//		{
//			memcpy(&m_data_sendbuf[cnt],"00",2);
//			cnt += 2;
//		}
//		else
//		{
//			memcpy(&m_data_sendbuf[cnt],"10",2);
//			cnt += 2;
//		}
//		m_data_sendbuf[cnt++] = pNode->data[7]/100 + 0x30;
//		m_data_sendbuf[cnt++] = pNode->data[7]%100/10 + 0x30;
//		m_data_sendbuf[cnt++] = pNode->data[7]%10 + 0x30;
//		
//		list_del(pNode);
//		pNode = (&RADIO_DATA_LIST_HEAD)->next;
//		if(i < tag_cnt)
//		{
//			m_data_sendbuf[cnt] = '#';
//			cnt += 1;  
//		}
//	}
//	ENABLE_GLOBAL_INTERRUPT();
//	
//	memcpy(&m_data_sendbuf[cnt],"\r\n",2);
//	cnt += 2;
//	send(SOCK_TCPC,m_data_sendbuf,cnt);
	
}



/*-------------------------------------------------*/
/*函数名：网络任务函数		                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
//void Rf_Working(void)
//{
//	if((IRQ_Flag0 == 1)||(IRQ0_READ()==1))//( xSemaphoreTake( BinarySemaphore_0, portMAX_DELAY ) == pdTRUE )//
//	{
//		RF_NUM = 0;
//		rfIRQIntHandler();
//		IRQ_Flag0 = 0;
//	}
//	if((IRQ_Flag1 == 1)||(IRQ1_READ()==1))//( xSemaphoreTake( BinarySemaphore_1, portMAX_DELAY ) == pdTRUE )//
//	{
//		RF_NUM = 1;
//		rfIRQIntHandler();
//		IRQ_Flag1 = 0;
//	}
//	if((IRQ_Flag2 == 1)||(IRQ2_READ()==1))//( xSemaphoreTake( BinarySemaphore_2, portMAX_DELAY ) == pdTRUE )//
//	{
//		RF_NUM = 2;
//		rfIRQIntHandler();
//		IRQ_Flag2 = 0;
//	}
//	if(check_time > 500)
//	{
//		check_time = 0;
//		/*********433检测**********/
//		Rf_Check();
//		/*********433检测**********/
//	}
//}

/*-------------------------------------------------*/
/*函数名：网络任务函数		                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
uint8 SYNSENT_NUM = 0;

//void TCP_Working(void)
//{
//	int  ret;                       //用于保存函数返回值
//	uint8 buff[200];
//	
//	tcp_state=getSn_SR(SOCK_TCPC);                //获取TCP链接端口的状态
//	if(tcp_state!=temp_state)
//	{                    //如果和上次的状态不一样
//		temp_state =  tcp_state;                  //保存本次状态
//		UART_Printf("状态编码:0x%x\r\n",tcp_state); //串口输出信息	
//	}	
//	if(ctlwizchip(CW_GET_PHYLINK, (void*)&ret) == -1)
//	{  //如果if成立，表示未知错误		 
//		UART_Printf("未知错误，准备重启\r\n");             //提示信息
//		NVIC_SystemReset();                              //重启
//	}
//	if(ret == PHY_LINK_OFF)
//	{         //判断网线是否断开
//		UART_Printf("网线断开\r\n");   //如果检测到，网线断开，串口提示信息	
//		delay_ms(1000);              //延时1s
//		TCP_time = 0xFFFF;			// 停止TCP超时检测
//		Work_Flag = WORK_INIT;		// 网线断开，重新初始化w5500
//	}
//	
//	switch(tcp_state)
//	{ //switch语句，判断当前TCP链接的状态   	
//		
//	  case SOCK_SYNSENT: 
//		UART_Printf("SOCK_SYNSENT\r\n");   //如果检测到，网线断开，串口提示信息	
//		delay_ms(1000);              //延时1s
//		break;
//	  case SOCK_INIT:          
//		if(Connect_flag==0)
//		{                                //如果还没有链接服务器，进入if
//			UART_Printf("准备连接服务器\r\n");                //串口输出信息
//			ret = connect(SOCK_TCPC,m_netpara.server_ip,m_netpara.server_port);   //链接服务器
//			UART_Printf("IP地址: %d.%d.%d.%d,%d\r\n", m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3],m_netpara.server_port);
//			UART_Printf("连接服务器返回码：%d\r\n",ret);      //串口输出信息
//			if(ret != SOCK_OK)
////			while(ret != SOCK_OK)	
//			{                         //服务器未开启进入if
//				UART_Printf("SOCKET 连接等待...\r\n",ret);      //串口输出信息
//				delay_ms(300);                             //延时3s
//				
////			disconnect(SOCK_TCPC);
////				ret = connect(SOCK_TCPC,m_netpara.server_ip,m_netpara.server_port);   //链接服务器
//			}
//			delay_ms(300);   
//		}
//		break;	                                       //跳出
//		
//	  case SOCK_ESTABLISHED:   
//		if((Connect_flag==0)&&(getSn_IR(SOCK_TCPC)==Sn_IR_CON))
//		{  //判断链接是否建立													
//			UART_Printf("连接已建立\r\n");                          //串口输出信息
//			Connect_flag = 1;                                     //链接标志=1
//			setSn_IR(SOCK_TCPC, Sn_IR_CON);						/*清除接收中断标志位*/
//		}
//		if(getSn_IR(SOCK_TCPC) & Sn_IR_CON)
//		{
//			setSn_IR(SOCK_TCPC, Sn_IR_CON);								          /*清除接收中断标志位*/
//		}
//		heartTicks = 0;		//清零心跳计时
////		send_datapacket();
//		ret = getSn_RX_RSR(SOCK_TCPC);	     /*定义len为已接收数据的长度*/
//		if(ret > 0)
//		{					                    //如果ret大于0，表示有数据来		 
//			recv(SOCK_TCPC,buff,ret);								              	/*接收来自Client的数据*/
//			UART_Printf("%s\r\n",buff);
////			NetData_processing(buff, ret);	// 接收数据处理
//		}		
//		break; //跳出
//		
//	  case SOCK_CLOSE_WAIT:      
//		UART_Printf("阅读器ID：");
//		Uart_SendData(Read_ID,sizeof(Read_ID));
//		UART_Printf("\r\n");
//		UART_Printf("等待关闭连接\r\n");                   //串口输出信息
//		if((ret=disconnect(SOCK_TCPC)) != SOCK_OK)       //端口关闭
//		{
//			UART_Printf("连接关闭失败，准备重启\r\n");      //串口输出信息
////			NVIC_SystemReset();	                       //重启
//		}
//		UART_Printf("连接关闭成功\r\n");                    //串口输出信息
//		TCP_Send_Flag = 0;
//		Connect_flag = 0;                                 //链接标志=0
//		break;
//		
//	  case SOCK_CLOSED:          
//		UART_Printf("准备打开W5500端口\r\n");               //串口输出信息
//		close(SOCK_TCPC);
//		TCP_Send_Flag = 0;
//		Connect_flag = 0;                                 //链接标志=0
//		ret = socket(SOCK_TCPC,Sn_MR_TCP,m_netpara.local_Port,Sn_MR_ND);  //打开W5500的端口，用于建立TCP链接，本地TCP端口5050
//		if(ret != SOCK_TCPC)
//		{	                           //如果打开失败，进入if																 
//			UART_Printf("端口打开错误，准备重启\r\n");     //串口输出信息
//			NVIC_SystemReset();	                       //重启
//		}
//		else 
//			UART_Printf("打开端口成功\r\n");	       //串口输出信息
//		break;                                            //跳出
//	  default:
//		  break;
//	}
//}

void MQTT_Package_msg(uint8 *payload, uint8 StartNum, uint16 Num)
{
	char data[6] = "";
	uint16 u16len = 0;
	
	memcpy(PubTopic, "/WSN-LW/",8);
	memcpy(PubTopic+8, Read_ID,8);
	memcpy(PubTopic+16, "/event/Data",sizeof("/event/Data"));
	UART_Printf("Pubscribing to %s\r\n", PubTopic);
	
	u16len = strlen("{\"SensorNum\":");
	memcpy(tempBuffer, "{\"SensorNum\":",u16len);
	sprintf(data, "%d", Num); 
	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += strlen(data);
	
	
	memcpy(tempBuffer+u16len, ",\"SensorStart\":",strlen(",\"SensorStart\":"));
	u16len += strlen(",\"SensorStart\":");
	memset(data,0,6);
	sprintf(data, "%d", StartNum); 
	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += strlen(data);
	
	memcpy(tempBuffer+u16len, ",\"SensorData\":\"",strlen(",\"SensorData\":\""));
	u16len += strlen(",\"SensorData\":\"");
	hex2strid(tempBuffer+u16len, payload, 10*Num);
//	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += 10*Num;
	memcpy(tempBuffer+u16len, "\"}",2);
	u16len += 2;
	memcpy(tempBuffer+u16len, 0,1);
	
	mqtt_msg.payload = tempBuffer;
	mqtt_msg.payloadlen = u16len;
//	MQTTPublish(&c, PubTopic, &mqtt_msg);
}

void MQTT_Package_Heartbeat(uint16 sn, uint8 resetFlag)
{
	char data[6] = "";
	uint16 u16len = 0;
	
	memcpy(PubTopic, "/WSN-LW/",8);
	memcpy(PubTopic+8, Read_ID,8);
	memcpy(PubTopic+16, "/event/Heartbeat",sizeof("/event/Heartbeat"));
	UART_Printf("Pubscribing to %s\r\n", PubTopic);
	
	u16len = strlen("{\"Heartbeat\":");
	memcpy(tempBuffer, "{\"Heartbeat\":",u16len);
	sprintf(data, "%d", sn); 
	memcpy(tempBuffer+u16len, data,strlen(data));
	u16len += strlen(data);
	
	memcpy(tempBuffer+u16len, "}",1);
	u16len += 1;
	memcpy(tempBuffer+u16len, 0,1);
}

// @brief messageArrived callback function
void messageArrived(MessageData* md)
{
//	unsigned char testbuffer[100];
//	MQTTMessage* message = md->message;
//    
//    UART_Printf("message received.\r\n");
//    memcpy(testbuffer,(char*)message->payload,(int)message->payloadlen);
//    *(testbuffer + (int)message->payloadlen + 1) = '\n';
//    UART_Printf("%s\r\n",testbuffer);
//	nrf_drv_wdt_feed();
	
	MQTTMessage* message = md->message;

	for (uint8_t i = 0; i < md->topicName->lenstring.len; i++)
		putchar(*(md->topicName->lenstring.data + i));

	UART_Printf(" %*s\r\n",
		(int32_t) message->payloadlen,
		(char*) message->payload);
}

void MQTT_Init(void)
{
	Network n;
    int rc = 0;
	NewNetwork(&n, SOCK_TCPC);
	UART_Printf("准备连接服务器\r\n");                //串口输出信息
	UART_Printf("MQTT Init\r\n");       	         //串口输出信息
	
	ConnectNetwork(&n, m_netpara.server_ip, m_netpara.server_port);
	UART_Printf("IP地址: %d.%d.%d.%d,%d\r\n", m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3],m_netpara.server_port);		
	MQTTClientInit(&c,&n,1000,SendBuffer,3072,tempBuffer,2048);
	
	// 订阅MQTT经纪人
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
//	data.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	data.clientID.cstring = "w5500-client";    //opts.clientid;
	data.username.cstring = ""; //opts.username;
	data.password.cstring = ""; //opts.password;
	
	// 设置会话心跳时间 单位为秒 服务器会每隔1.5*20秒的时间向客户端发送个消息判断客户端是否在线，但这个方法并没有重连的机制
	data.keepAliveInterval = 60;
	// 设置是否清空session,这里如果设置为false表示服务器会保留客户端的连接记录，这里设置为true表示每次连接到服务器都以新的身份连接
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
	UART_Printf("Connected %d\r\n", rc);

	// 订阅主题
	memcpy(SubTopic, "/WSN-LW/",8);
	memcpy(SubTopic+8, Read_ID,8);
	memcpy(SubTopic+16, "/service/#",sizeof("/service/#"));// 订阅参数更新
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
	UART_Printf("Subscribed %d\r\n", rc);
	
//	memcpy(SubTopic, "/WSN-LW/",8);
//	memcpy(SubTopic+8, Read_ID,8);
//	memcpy(SubTopic+16, "/service/Update",sizeof("/service/Update"));// 订阅参数更新
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
//	UART_Printf("Subscribed %d\r\n", rc);
//	
//	memcpy(SubTopic+16, "/service/Read_data",sizeof("/service/Read_data"));// 订阅读取数据
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
//	UART_Printf("Subscribed %d\r\n", rc);
//
//	memcpy(SubTopic+16, "/service/Alarm",sizeof("/service/Alarm"));	// 订阅报警信息
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
//	UART_Printf("Subscribed %d\r\n", rc);
	
	
//    MQTTMessage mqtt_msg;
    mqtt_msg.qos = QOS0;
    mqtt_msg.retained = 0;
    mqtt_msg.dup = 0;
//	memcpy(tempBuffer, "{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":3132213B2470E25101240102} ", 70);
    mqtt_msg.payload = SendBuffer;//"{\"Heartbeat\":8888,\"CSQ\":24,\"IMSI\":460045926209888}";
    mqtt_msg.payloadlen = strlen(SendBuffer);
	
}
void MQTT_Working(void)
{
//	char Topic[3][40]={"","",""};
	Network n;
//	MQTTClient c;
    int rc = 0;
//    unsigned char buf[1000];
	
	NewNetwork(&n, SOCK_TCPC);
	
	UART_Printf("准备连接服务器\r\n");                //串口输出信息
	UART_Printf("MQTT Work\r\n");       	         //串口输出信息
	
//	m_netpara.server_ip[0] = 118;
//	m_netpara.server_ip[1] = 31;
//	m_netpara.server_ip[2] = 227;
//	m_netpara.server_ip[3] = 219;

//	m_netpara.server_ip[0] = 192;
//	m_netpara.server_ip[1] = 168;
//	m_netpara.server_ip[2] = 1;
//	m_netpara.server_ip[3] = 103;
	ConnectNetwork(&n, m_netpara.server_ip, m_netpara.server_port);
	UART_Printf("IP地址: %d.%d.%d.%d,%d\r\n", m_netpara.server_ip[0],m_netpara.server_ip[1],m_netpara.server_ip[2],m_netpara.server_ip[3],m_netpara.server_port);		
	MQTTClientInit(&c,&n,1000,SendBuffer,3072,tempBuffer,2048);
	
	// 订阅MQTT经纪人
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
//	data.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	data.clientID.cstring = "w5500-client";    //opts.clientid;
	data.username.cstring = ""; //opts.username;
	data.password.cstring = ""; //opts.password;

	data.keepAliveInterval = 60;
	data.cleansession = 1;
	
	rc = MQTTConnect(&c, &data);
	UART_Printf("Connected %d\r\n", rc);

	// 订阅主题
	memcpy(SubTopic, "/WSN-LW/",8);
	memcpy(SubTopic+8, Read_ID,8);
	memcpy(SubTopic+16, "/service/+",sizeof("/service/+"));// 订阅参数更新
	UART_Printf("Subscribing to %s\r\n", SubTopic);
	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
	UART_Printf("Subscribed %d\r\n", rc);
	
////	memcpy(SubTopic0, "/WSN-LW/",8);
////	memcpy(SubTopic0+8, Read_ID,8);
////	memcpy(SubTopic0+16, "/service/Update",sizeof("/service/Update"));// 订阅参数更新
////	
////	memcpy(SubTopic1, "/WSN-LW/",8);
////	memcpy(SubTopic1+8, Read_ID,8);
////	memcpy(SubTopic1+16, "/service/Read_data",sizeof("/service/Read_data"));// 订阅参数更新
////	
////	memcpy(SubTopic2, "/WSN-LW/",8);
////	memcpy(SubTopic2+8, Read_ID,8);
////	memcpy(SubTopic2+16, "/service/Alarm",sizeof("/service/Alarm"));// 订阅参数更新
//	
////	memcpy(Topic[0], SubTopic0, strlen(SubTopic0));
////	memcpy(Topic[1], SubTopic1, strlen(SubTopic1));
////	memcpy(Topic[2], SubTopic2, strlen(SubTopic2));
////	//{SubTopic0, SubTopic1, SubTopic2};
////	UART_Printf("Subscribing to %s\r\n", Topic);
////	rc = MQTTSubscribe(&c, Topic, QOS0, messageArrived);// /# 订阅一切
////	UART_Printf("Subscribed %d\r\n", rc);
	
//	memcpy(SubTopic, "/WSN-LW/",8);
//	memcpy(SubTopic+8, Read_ID,8);
//	memcpy(SubTopic+16, "/service/Update",sizeof("/service/Update"));// 订阅参数更新
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
//	UART_Printf("Subscribed %d\r\n", rc);
//	
//	memcpy(SubTopic+16, "/service/Read_data",sizeof("/service/Read_data"));// 订阅读取数据
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
//	UART_Printf("Subscribed %d\r\n", rc);
//
//	memcpy(SubTopic+16, "/service/Alarm",sizeof("/service/Alarm"));	// 订阅报警信息
//	UART_Printf("Subscribing to %s\r\n", SubTopic);
//	rc = MQTTSubscribe(&c, SubTopic, QOS0, messageArrived);// /# 订阅一切
//	UART_Printf("Subscribed %d\r\n", rc);
	
	
//    MQTTMessage mqtt_msg;
    mqtt_msg.qos = QOS0;
    mqtt_msg.retained = 0;
    mqtt_msg.dup = 0;
//	memcpy(tempBuffer, "{\"SensorNum\":3,\"SensorStart\":1,\"SensorData\":3132213B2470E25101240102} ", 70);
    mqtt_msg.payload = SendBuffer;//"{\"Heartbeat\":8888,\"CSQ\":24,\"IMSI\":460045926209888}";
    mqtt_msg.payloadlen = 70;
	
	while(1)
    {
		nrf_drv_wdt_feed();
		if(MQTT_time == 0)
		{ // MQTT定期提交主题
			MQTT_Package_msg(SensorData[0],1,20);
			MQTT_time = 1000;
//			mqtt_msg.payload = tempBuffer;
//			mqtt_msg.payloadlen = u16len;
			rc = MQTTPublish(&c, PubTopic, &mqtt_msg);
			UART_Printf("Publish %d\r\n", rc);
//			UART_Printf("Publish \r\n");
//			MQTTPublish(&c, "/WSN-LW/20010333/event/Heartbeat", &mqtt_msg);
		}
//        delay_ms(1000);
		// 接收主题并与经纪人保持联系
    	MQTTYield(&c, 1000);
    }
}

void DHCP_Working(void)
{
		
	switch(DHCP_run())           //判断DHCP执行在哪个过程
	{
	  case DHCP_IP_ASSIGN:     //该状态表示，路由器分配给开发板ip了
	  case DHCP_IP_CHANGED:    //该状态表示，路由器改变了分配给开发板的ip
		my_ip_assign();   //调用IP参数获取函数，记录各个参数
		break;            //跳出
	  case DHCP_IP_LEASED:     //该状态表示，路由器分配给的开发板ip，正式租用了，表示可以联网通信了
		if(DNS_flag==0)									//如果域名还未解析，进入if
		{                                  
			W5500_DNS(ServerName);						//解析服务器域名
		}
		else
		{                                            //反之，如果已经解析到了IP，进入else分支链接服务器
			if(Work_Flag == WORK_DHCP)
			{
				Work_Flag = WORK_TCP;
				DHCP_time = 0xFFFF;
//				TCP_time = 10000;//10S
				MQTT_time = 1000;//1S
			}
//			TCP_Working();
			MQTT_Working();
		}
		break;  //跳出							
		
	  case DHCP_FAILED:       //该状态表示DHCP获取IP失败     									 
		my_dhcp_retry++;                        //失败次数+1
		if(my_dhcp_retry > MY_MAX_DHCP_RETRY)
		{  //如果失败次数大于最大次数，进入if							
			UART_Printf("DHCP失败，进入M2M模式\r\n");//串口提示信息
			my_dhcp_retry = 0;
			//				NVIC_SystemReset();		            //重启
			Work_Flag = WORK_M2M_nDHCP;
		}
		break;    
	}
}

void M2M_Working(void)
{
	
	if(m2m_status != 0xFF)
		m2m_status = m2m_init();
	else if(m2m_status == 0xFF)
	{// 搜网、连接服务器成功	
		Connect_flag = 1; 
//		if(m2m_TCP_Send() == M2M_CMD_SUCCESS)  	  // send to the cloud
//		{   // 数据发送成功
//			
//		}
		
//		if(M2M_Send_Flag == 1)
		{
//			M2M_Send_Flag = 0;
			m2m_tcpconnect();
			if(m2m_TCP_Send() != M2M_CMD_SUCCESS)  	  // send to the cloud
			{   // 数据发送成功
				m2m_status = 0;
			}
			else
			{
				LED_Flag = 1;
				SendTime = 0;	// 发送超时清零
//				heartTicks = 0;		//清零心跳计时
				m2m_closegprs();
			}
		}
	}
}




/*-------------------------------------------------*/
/*函数名：网络任务函数		                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TCP_Net_Working(void)
{

	while(1)
	{
//		if((Phy_Link_Flag == PHY_LINK_ON) && (TCP_Flag == 1))
//		{
//			if( Work_Flag == 0)
//			{
//				W5500_init();
////				setkeepalive(SOCK_TCPC);
//				Work_Flag = 1;
//			}
//			else if( Work_Flag == 1)
//				DHCP_Working();
//		}
//		else if((Phy_Link_Flag == PHY_LINK_OFF) || (TCP_Flag == 0))
//		{
//			M2M_Working();
//		}
		
		if( Work_Flag == WORK_INIT)
		{
			W5500_init();
//			Work_Flag = WORK_DHCP;
		}
		else if( Work_Flag <= WORK_TCP)
		{
			DHCP_Working();
		}
		else if(Work_Flag >= WORK_M2M_nPHY)
		{
			M2M_Working();
		}
		
//		MQTT_Working();
		
		nrf_drv_wdt_feed();
	}
}

