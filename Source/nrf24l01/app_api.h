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
#define APP_FRM_HDR_LEN    5  // APP֡ͷ����
#define APP_BUF_SIZE       170

#define UART_RX_OVER_TIME  10    // �жϴ��ڽ�����ɵĳ�ʱʱ��(ms)
	
#define UART_BUF_SIZE      255
#define Q_UART_SIZE        130
//#define Q_APP_SIZE         170
#define RF_BUF_LEN		   20
#define APP_BUF_LEN		   170	
                                                                
#define LEN_PARAM          9         // ��Ч���ò�������
#define ALARM_PARAM_START   ((uint32)0x08003800)	
#define ADDR_PARAM_START   	((uint32)0x08003C00)	// �洢���ò�������ʼ��ַ,FLASH���һҳ
#define ADDR_PARAM_RF_CH	ADDR_PARAM_START+4		// �洢2.4G�ŵ�����ʼ��ַ
#define ADDR_PARAM_ID		ADDR_PARAM_RF_CH+1		// �洢id����ʼ��ַ
	
#define Music_Deng		121		// �ȡ�������121����Ƶ�ļ�
#define Music_Router	122		// �м��豸�쳣����122����Ƶ�ļ�
#define Music_EndDevice	123		// �ɼ��豸�쳣����123����Ƶ�ļ�

#define Reset_RF_TIME	15		// �м̽���2.4G��ʱ��λʱ�䣻20����δ�յ�2.4G���ݣ���λ2.4Gģ��
#define Clear_Data_TIME	3		// �洢�ڵ���������ʱ�䣻35����δ�յ��ýڵ����ݣ���Ϊ�ýڵ��쳣����������

#define ALARMTIME	15000	// ����ѭ��ʱ�� 15s
#define ALARMNUM		4	// 1���ӱ������� 4��
/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/
typedef struct
{
    uint8  RxOK:1;       // ������ɱ�־λ   0:���ڽ���    1:�������
    uint8  u8Timer:7;    // ����ʱ�������� 0x7F:���ճ�ʱ ����:���ڽ���
	uint8  u8TimeOver;
	uint8  u8State;		 // 0:RF����    1:���ڽ��ջ���
} st_Uart;


typedef struct
{
    uint16  u16Wait;    // ģ���������״̬��ʣ��ʱ��
    uint16  u16Timer;   // ���ջ��ͳ�ʱ������
	uint8   RfState;    // 0:RF����    1:���ڽ��ջ���
	
	uint8	u8Count;	// �ѷ��ͻ��ѽ��յ��ֽ���
	uint16	u16PreambleLength;
	uint8	u8Len;		// ���ݰ����ܳ���
	uint8   Buf[RF_BUF_LEN];
} st_Rf;

typedef struct
{
	uint8  RxOK; 
	uint8	u8Len;
	uint8   Buf[APP_BUF_LEN];
} st_APP;

/* ���ڼ��ֳֻ����� */
typedef enum
{
	CMD_ACK           = 0x00,  // ȷ��֡
	CMD_NACK          = 0x01,  // ����֡
	CMD_READ_TP_REQ   = 0x02,  // ��ѯ͸��ģ�����
	CMD_READ_TP_REP   = 0x03,  // ��ѯ͸��ģ�����Ӧ��
	CMD_SET_TP_REQ    = 0x04,  // ����͸��ģ�����
	
	CMD_READ_VER	  = 0xF0,  // ��ȡģ��汾��Ϣ
	
	CMD_ACTIVE_REQ    = 0x05,  // ��������ģ��
//	CMD_ACTIVE_STA    = 0x39,  // ��ѯģ�鼤��״̬
} en_Cmd;

/* 433�������� */
typedef enum
{
	CMD_GET_DATA      = 0x01,  // ��ѯ����
	
	CMD_SEND_DATA     = 0x81,  // ��ѯӦ��
	
	CMD_ALARM_ON     	  = 0x02,  // ������
	
	CMD_ALARM_ON_BK      = 0x82,  // ������Ӧ��
	
	CMD_ROUTER_ON     	  = 0x03,  // ������
	
	CMD_ROUTER_ON_BK      = 0x83,  // ������Ӧ��
	
	CMD_ALARM_OFF     	  = 0x04,  // ������
	
	CMD_ALARM_OFF_BK      = 0x84,  // ������Ӧ��
	
	CMD_ERROR		  = 0xF0,  // ��������
	
//	CMD_ACTIVE_STA    = 0x39,  // ��ѯģ�鼤��״̬
} en_433Cmd;

/* GD5800��ƵоƬ�������� */
typedef enum
{
	CMD_PLAY		= 0x01,  // ���ŵ�ǰ��Ŀ
		
	CMD_STOP		= 0x0E,  // ֹͣ����
	
	CMD_VOLUME		= 0x31,  // ��������
	
	CMD_LOOP		= 0x33,  // ѭ������ 00��ȫ��ѭ����01���ļ���ѭ����02������ѭ����03�������04���������ţ�Ĭ�ϵ������ţ�
	
	CMD_COMBINED	= 0x47,  // ��ϲ���
	
	CMD_COMBINED_IN	= 0x48,  // ��ϲ岥
	
//	CMD_ACTIVE_STA    = 0x39,  // ��ѯģ�鼤��״̬
} en_GD5800Cmd;

/* ȷ��֡ */
typedef struct
{
    uint8 u8Hdr[2];           //0 ��ͷ0x5A 0xA5
	uint8 u8len;
    uint8 u8Cmd;           //2 ������
} st_FrmAck, *pst_FrmAck;


/* �������� */
typedef struct
{
	uint8 u8Hdr[2];			//0 ��ͷ0x5A 0xA5
	uint8 u8len;			//2 ����
    uint8 u8Cmd;            //3 ������
	uint8 u8ReportCycle;	//4 �ϱ����ڣ������ж�2.4G���ճ�ʱ
	uint8 u8AlarmTime;		//5 ��������ʱ��
	uint8 u8NrfChannel;     //6 �����ŵ�    1-200	433~465MHz
	uint8 u8Crc;			//7
	
}st_WorkParam;

typedef union
{
	st_WorkParam stWorkParam;
//	st_WorkModeStruct3_4 stWorkModeStruct3_4;
//	st_WorkModeStruct5 stWorkModeStruct5;
}un_ModuleParam, *pun_ModuleParam;

// ���հ�����֡�ṹ
typedef struct
{
	uint8 u8Hdr[2];           //0 ��ͷ0x55 0xAA
    uint8 u8Len; 	// ����
	uint8 u8Cmd; 	// ������
	uint8 u8Num; 	// ����������
	uint8 u8Channel; // 433�ŵ�
	uint8 u8Crc;	// �ۼӺ�У��
} st_Protocol, *pst_Protocol;

// ���Ͱ�����֡�ṹ
typedef struct
{
	uint8 u8Hdr[2];           //0 ��ͷ0x55 0xAA
    uint8 u8Len; 	// ����
	uint8 u8Cmd; 	// ������
//	uint8 u8R_Vol; 	// �м̵�ѹ
	uint8 u8Start;	// ��ʼ���
	uint8 u8Num;	// ����������
//	uint8 u8data[1];	// ����������
//	uint8 u8Crc;	// �ۼӺ�У��
} st_Protocol_bk, *pst_Protocol_bk;

/* ����appTask�����¼���־ */
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
	uint8 u8temp1_H;// 1#�������¶�
	uint8 u8temp1_L;
	uint8 u8temp2_H;// 2#�������¶�
	uint8 u8temp2_L;
	uint8 u8Bat_vol;	// ��������ѹ
//	uint8 u8Used;  // �����и�Ԫ���Ƿ�Ϊ�� 0����  1��������Ч
} st_Tag;

#define TAG_NUM_MAX 240

typedef struct
{
    uint8	u8TagNum;	// �ϱ�����������
	uint8	u8ClearID;	// ���������ID
    uint8	update_buf[TAG_NUM_MAX+2];	// �����ϱ���ʱ����
//	st_Tag	stTag[TAG_NUM_MAX];	// ����������
} st_Sensor, *pst_Sensor;


// GD8500����ģ������֡�ṹ
typedef struct
{
	uint8 u8Hdr;           //0 ��ͷ0x7E
	uint8 u8Len; 	// ����
	uint8 u8Cmd; 	// ������
	uint8 u8data[1];
//	uint8 u8data[1];	// ����������
//	uint8 u8Crc;	// �ۼӺ�У��
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
extern volatile uint16 delayTicks;  // ���ھ�ȷ��ʱ
extern st_Uart stUart;
extern st_APP stAPP;
extern st_Rf stRf;
//QUEUE_DECLEAR(Q_UART);
//QUEUE_DECLEAR(Q_APP);



extern uint8 AlarmNum;	// ��������	20��
extern uint16 AlarmTime;	// �������ʱ��	30s


/* �ɼ�����Ӧ��֡�ṹ */
//extern st_Tag stTag[TAG_NUM_MAX];
//extern uint8 update_buf[TAG_NUM_MAX+1];
extern st_Sensor	stSensor;
extern uint8 Rf_CheckTime;		// 433�쳣��λʱ��
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