#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "xqueue.h"
#include <string.h>

#define assert(exper)   

DOUBLE_LINK_NODE  RADIO_DATA_LIST_HEAD;

/*�Ƚ����������ݣ������ͬ����true�����ȷ���false*/
static bool compare(uint8_t *s,uint8_t *p,uint32_t len)
{
	while(len--)
	{
		if(*s!=*p)
			return false;
		s++;
		p++;
  }
  return true;
}

static inline void INIT_LIST_HEAD(DOUBLE_LINK_NODE *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add( DOUBLE_LINK_NODE *new,
                DOUBLE_LINK_NODE *prev,
                DOUBLE_LINK_NODE *next)
{
		next->prev = new;
		new->next = next;
		new->prev = prev;
		prev->next = new;
}

static inline void list_add(DOUBLE_LINK_NODE *new, DOUBLE_LINK_NODE *head)
{
  __list_add(new, head, head->next);
}

static inline void list_add_tail(DOUBLE_LINK_NODE *new, DOUBLE_LINK_NODE *head)
{
  __list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(DOUBLE_LINK_NODE * prev, DOUBLE_LINK_NODE * next)
{
	next->prev = prev;
	prev->next = next;
}

void list_del(DOUBLE_LINK_NODE *entry)
{
	__list_del(entry->prev, entry->next);
//	vPortFree(entry);
	free(entry);
}
/*��������ͷ*/
void create_list_head(DOUBLE_LINK_NODE *head)
{
	 INIT_LIST_HEAD(head);
	 memset(head->data,0,NET_RADIO_PACKET_LEN);
}

//��������ڵ�
DOUBLE_LINK_NODE* create_double_link_node(uint8_t *pdata,uint8_t len)  
{  
    //int i;
	
    DOUBLE_LINK_NODE* pDLinkNode = NULL;  
    pDLinkNode = (DOUBLE_LINK_NODE*)malloc(sizeof(DOUBLE_LINK_NODE));  
    if(pDLinkNode == NULL)
			return NULL;
    memset(pDLinkNode, 0, sizeof(DOUBLE_LINK_NODE));
	  if((pdata!=NULL) &&(len <= NET_RADIO_PACKET_LEN))
			memcpy(pDLinkNode->data,pdata,len);
    return pDLinkNode;  
}  

//ɾ��˫������
void delete_all_double_link_node(DOUBLE_LINK_NODE* pDLinkNodeHead)  
{  
    DOUBLE_LINK_NODE* pNode;  
    if(NULL == pDLinkNodeHead)  
        return ;  
		
		pNode = pDLinkNodeHead->next; 
    while(pNode != pDLinkNodeHead)
	  {
   		list_del(pNode); 
  		pNode = pDLinkNodeHead->next;
	  }
} 
//��˫�������в�������
DOUBLE_LINK_NODE* find_data_in_double_link(const DOUBLE_LINK_NODE* pDLinkNodehead, uint8_t  *pdata,uint8_t len)  
{  	
    DOUBLE_LINK_NODE* pNode = NULL;  
    if(NULL == pDLinkNodehead)  
        return NULL;  
  
    pNode = pDLinkNodehead->next;  
    while(pDLinkNodehead != pNode)
		{  
       if(compare(&pdata[0],&(pNode->data[0]),len) == true)    //���ҵ���ͬ���ݣ����ص�ǰ�ڵ�
			   return pNode;
       pNode = pNode ->next;  
    }     
    return NULL;  
}  
//��˫�������в�������
bool insert_data_into_double_link(DOUBLE_LINK_NODE* pDLinkNodehead, uint8_t  *pdata,uint8_t len)  
{  
    DOUBLE_LINK_NODE* pNode;  
    //DOUBLE_LINK_NODE* pIndex;  
  
    if(NULL == pDLinkNodehead)  
        return false;  
  	if(count_number_in_double_link(pDLinkNodehead) > 300)
		return false; 
	
    pNode = find_data_in_double_link(pDLinkNodehead, pdata,2);    //ֻ�Ƚ�6��mac��ַ
    if(pNode != NULL)
		{
			memcpy(pNode->data,pdata,len);  
      return false; 
		}
    pNode  = create_double_link_node(pdata,len);  
    if(pNode != NULL)
		{
		  list_add(pNode,pDLinkNodehead);
		}
			
    return true;  
}

//��˫��������ɾ������
bool delete_data_from_double_link(DOUBLE_LINK_NODE* pDLinkNodehead, uint8_t  *pdata,uint8_t len)  
{  
    DOUBLE_LINK_NODE* pNode;  
    if(NULL == pDLinkNodehead /*|| NULL == pDLinkNodehead*/)  
        return false;  
  	
    pNode = find_data_in_double_link(pDLinkNodehead, pdata,NET_RADIO_PACKET_LEN); 
    if(pNode != NULL)
     	list_del(pNode);	 
      
    return true;  
}
//ͳ��˫���������е����ݸ���
int count_number_in_double_link(const DOUBLE_LINK_NODE* pDLinkNodehead)  
{  
    int count = 0;  
    DOUBLE_LINK_NODE* pNode = (DOUBLE_LINK_NODE*)pDLinkNodehead;

      
    pNode = pDLinkNodehead->next;
    while(pDLinkNodehead != pNode){  
        count ++;  
        pNode = pNode->next;  
    }  
    return count;  
}

//��������е�����
void print_double_link_node( DOUBLE_LINK_NODE* pDLinkNodehead,uint8_t *pout)  
{  
    DOUBLE_LINK_NODE* pNode = pDLinkNodehead->next;  
  
    while(pDLinkNodehead != pNode)
		{		
			  memcpy(pout,pNode->data,NET_RADIO_PACKET_LEN);
        pNode = pNode ->next;  
    }  
} 		

/*******************************************************************************
 * Function Name: comMEMCMPByte
 * Decription   : �ж��ڴ�����buf���Ƿ�ȫ��ֵu8Val
 * Calls        : 
 * Called By    : 
 * Input        :
 * Output       : None
 * Return Value : 0: buf����ȫu8Val
 *                1: buf�в���ȫu8Val
 * Others       : None
 *------------------------------ Revision History ------------------------------
 * No. Version Date       Revised by   Item       Description     
 *          
 ******************************************************************************/
unsigned char comMEMCMPByte(const unsigned char *buf, unsigned char u8Val, unsigned char u8Len)
{
    while(u8Len--)
	{
		if(buf[u8Len] != u8Val)
		{
			return 1;
		}
	}
	return 0;
}

/*******************************************************************************
 * Function Name: comCalCRC16
 * Decription   : ����CRC16ֵ
 * Calls        : 
 * Called By    : 
 * Input        : u16Poly : CRC16����ʽ
 * Output       : None
 * Return Value :
 * Others       : None
 *------------------------------ Revision History ------------------------------
 * No. Version Date       Revised by   Item       Description     
 *          
 ******************************************************************************/
uint16_t comCalCRC16(uint8_t* pbuf, uint16_t u16Len, uint16_t u16Poly)
{
    unsigned char i;
    unsigned short u16CRC = 0;

    while(u16Len--)
    {
        u16CRC ^= *pbuf++;
        for(i = 0; i < 8; ++i)
        {
            if (u16CRC & 0x0001)
            {
                u16CRC  >>= 1;
                u16CRC  ^=  u16Poly;
            }
            else 
            {
                u16CRC >>= 1;
            }
        }
    } 

    return u16CRC;
}
		
// �ַ�����ʽ������ת����16���Ƶ�����  �ַ�����123456�� ת����  0x123456
// strid��ָ���ַ�����bufָ��
// phex��ָ��16��������
// strsz���ַ����ĳ��ȣ�����16�������鳤��*2��������ż��
void strid2hex(char *strid, uint8_t *phex, uint16_t strsz)
{
	for(uint16_t i = 0;i < strsz;i+=2)
	{
		*phex = ((*(strid+ i) - 0x30) << 4) + (*(strid+i+1) -0x30);
		phex++;
	}
}

// ��16���Ƶ�����ת�����ַ�����ʽ������ ����0x123456   ת���� �ַ�����123456��
// strid��ָ�򱣴��ַ�����bufָ��
// phex��ָ��16��������
// strsz��ת�����ַ����ĳ��ȣ�����16�������鳤��*2��������ż��
void hex2strid(char *strid, uint8_t *phex, uint16_t strsz)
{
	for(uint16_t i = 0;i < strsz;i+=2)
	{
		if(((*phex) >> 4) >=0 && ((*phex) >> 4) <= 9)
			*(strid+i)   = ((*phex) >> 4) + 0x30;
		else if(((*phex) >> 4) >=0x0A && ((*phex) >> 4) <= 0x0F)
			*(strid+i)   = ((*phex) >> 4) + 0x37;
		
		if((*phex & 0x0F) >=0 && (*phex & 0x0F) <= 9)
			*(strid+i+1) = (*phex & 0x0F) +0x30;
		else if((*phex & 0x0F) >=0x0A && (*phex & 0x0F) <= 0x0F)
			*(strid+i+1) = (*phex & 0x0F) +0x37;
		phex++;
	}
}

