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

// �ַ�����ʽ������ת����16���Ƶ�����  �ַ�����123456�� ת����  0x123456
// strid��ָ���ַ�����bufָ��
// phex��ָ��16��������
// strsz���ַ����ĳ��ȣ�����16�������鳤��*2��������ż��
void strid2hex(char *strid, uint8_t *phex, uint16_t strsz);
// ��16���Ƶ�����ת�����ַ�����ʽ������ ����0x123456   ת���� �ַ�����123456��
// strid��ָ�򱣴��ַ�����bufָ��
// phex��ָ��16��������
// strsz��ת�����ַ����ĳ��ȣ�����16�������鳤��*2��������ż��
void hex2strid(char *strid, uint8_t *phex, uint16_t strsz);

#endif
