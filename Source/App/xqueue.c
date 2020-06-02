#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "xqueue.h"
#include <string.h>

#define assert(exper)   

DOUBLE_LINK_NODE  RADIO_DATA_LIST_HEAD;

/*比较两数组数据，如果相同返回true，不等返回false*/
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
/*创建链表头*/
void create_list_head(DOUBLE_LINK_NODE *head)
{
	 INIT_LIST_HEAD(head);
	 memset(head->data,0,NET_RADIO_PACKET_LEN);
}

//创建链表节点
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

//删除双向链表
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
//在双向链表中查找数据
DOUBLE_LINK_NODE* find_data_in_double_link(const DOUBLE_LINK_NODE* pDLinkNodehead, uint8_t  *pdata,uint8_t len)  
{  	
    DOUBLE_LINK_NODE* pNode = NULL;  
    if(NULL == pDLinkNodehead)  
        return NULL;  
  
    pNode = pDLinkNodehead->next;  
    while(pDLinkNodehead != pNode)
		{  
       if(compare(&pdata[0],&(pNode->data[0]),len) == true)    //查找到相同数据，返回当前节点
			   return pNode;
       pNode = pNode ->next;  
    }     
    return NULL;  
}  
//在双向链表中插入数据
bool insert_data_into_double_link(DOUBLE_LINK_NODE* pDLinkNodehead, uint8_t  *pdata,uint8_t len)  
{  
    DOUBLE_LINK_NODE* pNode;  
    //DOUBLE_LINK_NODE* pIndex;  
  
    if(NULL == pDLinkNodehead)  
        return false;  
  	if(count_number_in_double_link(pDLinkNodehead) > 300)
		return false; 
	
    pNode = find_data_in_double_link(pDLinkNodehead, pdata,2);    //只比较6个mac地址
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

//在双向链表中删除数据
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
//统计双向链表中中的数据个数
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

//输出链表中的数据
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
 * Decription   : 判断内存区域buf中是否全是值u8Val
 * Calls        : 
 * Called By    : 
 * Input        :
 * Output       : None
 * Return Value : 0: buf中是全u8Val
 *                1: buf中不是全u8Val
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
 * Decription   : 计算CRC16值
 * Calls        : 
 * Called By    : 
 * Input        : u16Poly : CRC16多项式
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
		
// 字符串形式的数字转换成16进制的数组  字符串“123456” 转换成  0x123456
// strid：指向字符串的buf指针
// phex：指向16进制数组
// strsz：字符串的长度，等于16进制数组长度*2，必须是偶数
void strid2hex(char *strid, uint8_t *phex, uint16_t strsz)
{
	for(uint16_t i = 0;i < strsz;i+=2)
	{
		*phex = ((*(strid+ i) - 0x30) << 4) + (*(strid+i+1) -0x30);
		phex++;
	}
}

// 将16进制的数组转换成字符串形式的数字 比如0x123456   转换成 字符串“123456”
// strid：指向保存字符串的buf指针
// phex：指向16进制数组
// strsz：转换后字符串的长度，等于16进制数组长度*2，必须是偶数
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

