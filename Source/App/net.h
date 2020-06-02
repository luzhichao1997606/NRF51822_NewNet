/*-------------------------------------------------*/
/*                                                 */
/*            ʵ�����繦�ܵ�ͷ�ļ�             */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __NET_H
#define __NET_H

#define   STORE_MAGIC           0x12345679
#define   STORE_PAGE            127

/*Socket �˿�ѡ�񣬿ɰ��Լ���ϰ�߶���*/
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
////#define SOCK_DHCP		    0      //DHCP�����õĶ˿ڣ�W5500��8����0-7����
////#define SOCK_TCPS	        1      //TCP�����õĶ˿ڣ� W5500��8����0-7����
////#define SOCKET_DNS          2      //DNS�����õĶ˿ڣ� W5500��8����0-7����
#define MY_MAX_DHCP_RETRY	3      //DHCP���Դ���
#define DATA_BUF_SIZE       2048   //��������С

#define HeartTime	5000	//��������5s
#define DataTime		300	//���ݰ�����1s
#define SetTime		30000	//Ĭ�ϲ�������ʱ��
#define M2MDataTime	5000	//M2M�ϱ�����

/*cmd define*/

#define   CMD_UPLOADDATA         0x01
#define   CMD_HEART              0x02
#define   CMD_READCFG            0x04
#define   CMD_WRITECFG           0x03  //д�������
//#define   CMD_SETCTRL            0x05  //���ÿ���ָ��
#define   CMD_FACTORY_RESET      0xF0  //�ָ���������
#define   CMD_READ_VER      0xF1  //������汾
#define   CMD_HAL_CFG        0x05  //���ò���
#define   CMD_HAL_READ       0x06  //��ȡ����



typedef struct
{
	uint8_t  local_IP[4];	          //�ͻ���Ĭ��IP
	uint32_t local_Port;					 //�ͻ���Ĭ�϶˿ں�
	uint8_t  server_ip[4];         //������Ĭ��IP
	uint32_t server_port;					 //������Ĭ�϶˿ں�
	uint8_t  local_sunNet[4];       //Ĭ��������û
	uint8_t  local_GW[4];           //Ĭ������
	uint8_t  mac[6];
	uint8_t dns[4];				// DNS ��������ַ
}netpara_t;

typedef struct
{
	uint32_t   magic;
	netpara_t  net_cfg;
	uint8_t    rssi_Calibration;//rssi_threshold; //rssi��ֵ
	uint8_t    factor;//tag_type;       //��ǩ����
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


/*���������ò��������ʽ*/
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
	uint16_t SerialNumber;	// �����
	uint8_t OfflineFlag;	// ����״̬
	uint8_t SensorAll;		// �Ƿ�ȫ������
}MQTT_Param_t;

typedef struct
{
	uint8_t Cycle;	// MQTT�ϱ���������
	uint8_t AlarmTime;	// ��������ʱ��
	uint8_t Channel;	// �����ŵ�
	uint8_t SensorNum;		// ����������
	uint8_t SensorCycle;	// ���������ݸ������ڣ���ʱ����ʱ��
	uint8_t ResetFlag;		// ��λ״̬λ
}FLASH_Param_t;

extern MQTT_Param_t		MQTT_Param;
extern FLASH_Param_t	FLASH_Param;

extern const  netpara_t default_netpara;
extern uint8_t Read_ID[16];

extern uint16_t check_time;
extern uint16_t work_time;
extern uint16_t   m_DEVICE_ID;
extern netpara_t            m_netpara;            //�������
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
