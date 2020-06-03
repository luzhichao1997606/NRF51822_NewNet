/******************************************************************************
 * File        : app.h
 * Author      : Lihao
 * DESCRIPTION :
 *
 ****************************** Revision History *******************************
 * No.   Version   Date         Modified By   Description               
 * 1     V1.0      2011-05-01   Lihao         Original Version
 ******************************************************************************/
#ifndef APP_H
#define APP_H

#if defined __cplusplus
extern "C" {
#endif

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
#include "hidefs.h"
#include "common.h"

/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/
//#define TEST_MODE
#define APP_FRM_HDR_LEN    5  // APP帧头长度
#define APP_BUF_SIZE       170

#define UART_RX_OVER_TIME  10    // 判断串口接收完成的超时时间(ms)
	
#define UART_BUF_SIZE      255
#define Q_UART_SIZE        130
//#define Q_APP_SIZE         170
#define RF_BUF_LEN		   20
#define APP_BUF_LEN		   170	
                                                                
#define LEN_PARAM          9         // 有效配置参数长度
#define ALARM_PARAM_START   ((uint32)0x08003800)	
#define ADDR_PARAM_START   	((uint32)0x08003C00)	// 存储配置参数的起始地址,FLASH最后一页
#define ADDR_PARAM_RF_CH	ADDR_PARAM_START+4		// 存储2.4G信道的起始地址
#define ADDR_PARAM_ID		ADDR_PARAM_RF_CH+1		// 存储id的起始地址
	
#define Music_Deng		121		// 等。。。第121个音频文件
#define Music_Router	122		// 中继设备异常；第122个音频文件
#define Music_EndDevice	123		// 采集设备异常；第123个音频文件

#define Reset_RF_TIME	15		// 中继接收2.4G超时复位时间；20分钟未收到2.4G数据，复位2.4G模块
#define Clear_Data_TIME	3		// 存储节点数据清零时间；35分钟未收到该节点数据，认为该节点异常，数据清零

#define ALARMTIME	15000	// 报警循环时间 15s
#define ALARMNUM		4	// 1分钟报警次数 4次
/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/
typedef struct
{
    uint8  RxOK:1;       // 接收完成标志位   0:正在接收    1:接收完毕
    uint8  u8Timer:7;    // 接收时间间隔计数 0x7F:接收超时 其他:正在接收
	uint8  u8TimeOver;
	uint8  u8State;		 // 0:RF空闲    1:正在接收或发送
} st_Uart;


typedef struct
{
    uint16  u16Wait;    // 模块进入休眠状态的剩余时间
    uint16  u16Timer;   // 接收或发送超时计数器
	uint8   RfState;    // 0:RF空闲    1:正在接收或发送
	
	uint8	u8Count;	// 已发送或已接收的字节数
	uint16	u16PreambleLength;
	uint8	u8Len;		// 数据包的总长度
	uint8   Buf[RF_BUF_LEN];
} st_Rf;

typedef struct
{
	uint8  RxOK; 
	uint8	u8Len;
	uint8   Buf[APP_BUF_LEN];
} st_APP;

/* 串口及手持机命令 */
typedef enum
{
	CMD_ACK           = 0x00,  // 确认帧
	CMD_NACK          = 0x01,  // 否认帧
	CMD_READ_TP_REQ   = 0x02,  // 查询透传模块参数
	CMD_READ_TP_REP   = 0x03,  // 查询透传模块参数应答
	CMD_SET_TP_REQ    = 0x04,  // 配置透传模块参数
	
	CMD_READ_VER	  = 0xF0,  // 读取模块版本信息
	
	CMD_ACTIVE_REQ    = 0x05,  // 激活无线模块
//	CMD_ACTIVE_STA    = 0x39,  // 查询模块激活状态
} en_Cmd;

/* 433串口命令 */
typedef enum
{
	CMD_GET_DATA      = 0x01,  // 轮询命令
	
	CMD_SEND_DATA     = 0x81,  // 轮询应答
	
	CMD_ALARM_ON     	  = 0x02,  // 报警开
	
	CMD_ALARM_ON_BK      = 0x82,  // 报警开应答
	
	CMD_ROUTER_ON     	  = 0x03,  // 报警开
	
	CMD_ROUTER_ON_BK      = 0x83,  // 报警开应答
	
	CMD_ALARM_OFF     	  = 0x04,  // 报警关
	
	CMD_ALARM_OFF_BK      = 0x84,  // 报警关应答
	
	CMD_ERROR		  = 0xF0,  // 错误命令
	
//	CMD_ACTIVE_STA    = 0x39,  // 查询模块激活状态
} en_433Cmd;

/* GD5800音频芯片串口命令 */
typedef enum
{
	CMD_PLAY		= 0x01,  // 播放当前曲目
		
	CMD_STOP		= 0x0E,  // 停止播放
	
	CMD_VOLUME		= 0x31,  // 音量设置
	
	CMD_LOOP		= 0x33,  // 循环播放 00：全盘循环，01：文件夹循环，02：单曲循环，03：随机，04：单曲播放（默认单曲播放）
	
	CMD_COMBINED	= 0x47,  // 组合播放
	
	CMD_COMBINED_IN	= 0x48,  // 组合插播
	
//	CMD_ACTIVE_STA    = 0x39,  // 查询模块激活状态
} en_GD5800Cmd;

/* 确认帧 */
typedef struct
{
    uint8 u8Hdr[2];           //0 包头0x5A 0xA5
	uint8 u8len;
    uint8 u8Cmd;           //2 命令字
} st_FrmAck, *pst_FrmAck;


/* 工作参数 */
typedef struct
{
	uint8 u8Hdr[2];			//0 包头0x5A 0xA5
	uint8 u8len;			//2 长度
    uint8 u8Cmd;            //3 命令字
	uint8 u8ReportCycle;	//4 上报周期，用来判断2.4G接收超时
	uint8 u8AlarmTime;		//5 报警持续时间
	uint8 u8NrfChannel;     //6 无线信道    1-200	433~465MHz
	uint8 u8Crc;			//7
	
}st_WorkParam;

typedef union
{
	st_WorkParam stWorkParam;
//	st_WorkModeStruct3_4 stWorkModeStruct3_4;
//	st_WorkModeStruct5 stWorkModeStruct5;
}un_ModuleParam, *pun_ModuleParam;

// 接收包数据帧结构
typedef struct
{
	uint8 u8Hdr[2];           //0 包头0x55 0xAA
    uint8 u8Len; 	// 长度
	uint8 u8Cmd; 	// 命令字
	uint8 u8Num; 	// 传感器数量
	uint8 u8Channel; // 433信道
	uint8 u8Crc;	// 累加和校验
} st_Protocol, *pst_Protocol;

// 发送包数据帧结构
typedef struct
{
	uint8 u8Hdr[2];           //0 包头0x55 0xAA
    uint8 u8Len; 	// 长度
	uint8 u8Cmd; 	// 命令字
//	uint8 u8R_Vol; 	// 中继电压
	uint8 u8Start;	// 起始编号
	uint8 u8Num;	// 传感器数量
//	uint8 u8data[1];	// 传感器数据
//	uint8 u8Crc;	// 累加和校验
} st_Protocol_bk, *pst_Protocol_bk;

/* 设置appTask工作事件标志 */
typedef struct
{
    uint8     RfIRQFlag_1;
	uint8     RfIRQFlag_2;
    uint8	  UartSendFlag;
	uint8	  AlarmSendFlag;
} app_Event;

typedef struct
{
//	uint8 ID[8];
	uint8 u8temp1_H;// 1#传感器温度
	uint8 u8temp1_L;
	uint8 u8temp2_H;// 2#传感器温度
	uint8 u8temp2_L;
	uint8 u8Bat_vol;	// 传感器电压
//	uint8 u8Used;  // 数组中该元素是否为空 0：空  1：数据有效
} st_Tag;

#define TAG_NUM_MAX 240

typedef struct
{
    uint8	u8TagNum;	// 上报传感器数量
	uint8	u8ClearID;	// 被清空数据ID
    uint8	update_buf[TAG_NUM_MAX+2];	// 数据上报超时计数
//	st_Tag	stTag[TAG_NUM_MAX];	// 传感器数据
} st_Sensor, *pst_Sensor;


// GD8500语音模块数据帧结构
typedef struct
{
	uint8 u8Hdr;           //0 包头0x7E
	uint8 u8Len; 	// 长度
	uint8 u8Cmd; 	// 命令字
	uint8 u8data[1];
//	uint8 u8data[1];	// 传感器数据
//	uint8 u8Crc;	// 累加和校验
} st_GD8500, *pst_GD8500;

/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/
//extern const uint32 PortRate[8];
//extern const UART1_Parity_TypeDef ParityBit[3];
//extern const UART1_StopBits_TypeDef StopBit[3];
extern volatile uint16 delayTicks;  // 用于精确延时
extern st_Uart stUart;
extern st_APP stAPP;
extern st_Rf stRf;
//QUEUE_DECLEAR(Q_UART);
//QUEUE_DECLEAR(Q_APP);



extern uint8 AlarmNum;	// 报警次数	20次
extern uint16 AlarmTime;	// 报警间隔时间	30s


/* 采集数据应答帧结构 */
//extern st_Tag stTag[TAG_NUM_MAX];
//extern uint8 update_buf[TAG_NUM_MAX+1];
extern st_Sensor	stSensor;
extern uint8 Rf_CheckTime;		// 433异常复位时间
extern pun_ModuleParam punModuleParam;
extern pst_Protocol pstProtocol;
extern pst_Protocol_bk pstProtocol_bk;
extern pst_GD8500 pstGD8500;

extern app_Event App_Event;
extern const uint8 ModuleAddr[6];
extern const uint8 dstAddr[6];
/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/
void appInit(void);
void appTask(void);
void tmrDelay(uint16 u16Delay);

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* APP_H */
/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/

