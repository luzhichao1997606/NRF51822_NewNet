#ifndef XQUEUE_H
#define XQUEUE_H


#define   NET_RADIO_PACKET_LEN    18

typedef struct _DOUBLE_LINK_NODE  
{  
    uint8_t data[NET_RADIO_PACKET_LEN];  
    struct _DOUBLE_LINK_NODE* prev;  
    struct _DOUBLE_LINK_NODE* next;  
}DOUBLE_LINK_NODE;

extern DOUBLE_LINK_NODE  RADIO_DATA_LIST_HEAD;


void create_list_head(DOUBLE_LINK_NODE*head);

void list_del(DOUBLE_LINK_NODE *entry);

DOUBLE_LINK_NODE* create_double_link_node(uint8_t *pdata,uint8_t len);

void delete_all_double_link_node(DOUBLE_LINK_NODE* pDLinkNode) ;

DOUBLE_LINK_NODE* find_data_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode, uint8_t  *pdata,uint8_t len); 

void delete_all_double_link_node(DOUBLE_LINK_NODE* pDLinkNode)  ;

DOUBLE_LINK_NODE* find_data_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode, uint8_t  *pdata,uint8_t len)  ;

bool insert_data_into_double_link(DOUBLE_LINK_NODE*ppDLinkNode, uint8_t  *pdata,uint8_t len)  ;

bool delete_data_from_double_link(DOUBLE_LINK_NODE* ppDLinkNode, uint8_t  *pdata,uint8_t len)  ;

int count_number_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode) ;

void print_double_link_node( DOUBLE_LINK_NODE* pDLinkNodehead,uint8_t *pout);

unsigned char comMEMCMPByte(const unsigned char *buf, unsigned char u8Val, unsigned char u8Len);
uint16_t comCalCRC16(uint8_t* pbuf, uint16_t u16Len, uint16_t u16Poly);

// 字符串形式的数字转换成16进制的数组  字符串“123456” 转换成  0x123456
// strid：指向字符串的buf指针
// phex：指向16进制数组
// strsz：字符串的长度，等于16进制数组长度*2，必须是偶数
void strid2hex(char *strid, uint8_t *phex, uint16_t strsz);
// 将16进制的数组转换成字符串形式的数字 比如0x123456   转换成 字符串“123456”
// strid：指向保存字符串的buf指针
// phex：指向16进制数组
// strsz：转换后字符串的长度，等于16进制数组长度*2，必须是偶数
void hex2strid(char *strid, uint8_t *phex, uint16_t strsz);

#endif
