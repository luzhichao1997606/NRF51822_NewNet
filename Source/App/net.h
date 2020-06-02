/*-------------------------------------------------*/
/*                                                 */
/*            实现网络功能的头文件             */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __NET_H
#define __NET_H

#define   STORE_MAGIC           0x12345679
#define   STORE_PAGE            127

/*Socket 端口选择，可按自己的习惯定义*/
#define SOCK_TCPS			0
#define SOCK_HUMTEM			0
#define SOCK_PING			0
#define SOCK_TCPC			1
#define SOCK_UDPS           	2
#define SOCK_WEIBO			2
#define SOCK_DHCP			3
#define SOCK_HTTPS          	4
#define SOCK_DNS            	5
#define SOCK_SMTP           	6
#define SOCK_NTP            	7
////#define SOCK_DHCP		    0      //DHCP功能用的端口，W5500有8个，0-7都行
////#define SOCK_TCPS	        1      //TCP连接用的端口， W5500有8个，0-7都行
////#define SOCKET_DNS          2      //DNS功能用的端口， W5500有8个，0-7都行
#define MY_MAX_DHCP_RETRY	3      //DHCP重试次数
#define DATA_BUF_SIZE       2048   //缓冲区大小

#define HeartTime	5000	//心跳周期5s
#define DataTime		300	//数据包周期1s
#define SetTime		30000	//默认参数工作时间
#define M2MDataTime	5000	//M2M上报周期

/*cmd define*/

#define   CMD_UPLOADDATA         0x01
#define   CMD_HEART              0x02
#define   CMD_READCFG            0x04
#define   CMD_WRITECFG           0x03  //写网络参数
//#define   CMD_SETCTRL            0x05  //设置控制指令
#define   CMD_FACTORY_RESET      0xF0  //恢复出厂设置
#define   CMD_READ_VER      0xF1  //读软件版本
#define   CMD_HAL_CFG        0x05  //配置参数
#define   CMD_HAL_READ       0x06  //读取参数



typedef struct
{
	uint8_t  local_IP[4];	          //客户端默认IP
	uint32_t local_Port;					 //客户端默认端口号
	uint8_t  server_ip[4];         //服务器默认IP
	uint32_t server_port;					 //服务器默认端口号
	uint8_t  local_sunNet[4];       //默认子网淹没
	uint8_t  local_GW[4];           //默认网关
	uint8_t  mac[6];
	uint8_t dns[4];				// DNS 服务器地址
}netpara_t;

typedef struct
{
	uint32_t   magic;
	netpara_t  net_cfg;
	uint8_t    rssi_Calibration;//rssi_threshold; //rssi阈值
	uint8_t    factor;//tag_type;       //标签类型
}para_cfg_t;

typedef struct
{
	uint8_t head[2];
	uint8_t len[2];
	uint8_t device_ID[2];
	uint8_t cmd;
	uint8_t sn;
	uint8_t localip[4];
	uint8_t localport[2];
	uint8_t gateway[4];
	uint8_t mac[6];
	uint8_t submask[4];
	uint8_t server_ip[4];
	uint8_t server_port[2];
	uint8_t reserv[2];
	uint16_t check;
}writecfg_t;


/*触发器配置参数网络格式*/
typedef struct
{
	uint8_t head[2];
	uint8_t len[2];
	uint8_t device_id[2];
	uint8_t cmd;
	uint8_t sn;
	uint8_t rssi;
	uint8_t factor;
	uint8_t reserved[2];
	uint16_t check;
}NetTriCfg_t;


typedef struct
{
	uint16_t time;	//ms
	uint16_t SerialNumber;	// 包序号
	uint8_t OfflineFlag;	// 离线状态
	uint8_t SensorAll;		// 是否全部上线
}MQTT_Param_t;

typedef struct
{
	uint8_t Cycle;	// MQTT上报数据周期
	uint8_t AlarmTime;	// 报警持续时间
	uint8_t Channel;	// 无线信道
	uint8_t SensorNum;		// 传感器数量
	uint8_t SensorCycle;	// 传感器数据更新周期，超时清零时间
	uint8_t ResetFlag;		// 复位状态位
}FLASH_Param_t;

extern MQTT_Param_t		MQTT_Param;
extern FLASH_Param_t	FLASH_Param;

extern const  netpara_t default_netpara;
extern uint8_t Read_ID[16];

extern uint16_t check_time;
extern uint16_t work_time;
extern uint16_t   m_DEVICE_ID;
extern netpara_t            m_netpara;            //网络参数
extern para_cfg_t           m_cfgpara;
extern uint8_t              m_rssi_Calibration;
extern uint8_t              m_factor;

void  store_clear(uint32_t page);

void  para_store(uint32_t page,para_cfg_t para);

void  para_read(uint32_t page,para_cfg_t *ppara);


void W5500_init(void);
void W5500_TxData(unsigned char *data);
void W5500_DNS(unsigned char *DomainName);

void Net_Time_Work(void);
void Phy_Link_Check(void);
void SendTimeOut(void);

void Net_Working(void);
void TCP_Working(void);
void TCP_Net_Working(void);

#endif
