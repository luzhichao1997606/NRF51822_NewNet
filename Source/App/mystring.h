/**
 * mystring.h file for STM32F103.
 * Describtion: �ַ�����ز���������
 * Author: qinfei 2015.04.02
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */


#ifndef _MY_STRING_H_
#define _MY_STRING_H_
#include "hal.h"

/*�ַ������Ժ���*/
extern void StringTest(void);

/*��src�ַ������ӵ�dest�ַ�����ǰ��*/
extern void strcat_oppo(uint8 *dest,const uint8 *src);

/*�ַ������*/
extern void clcstr(uint8 *src,uint16 length);

/*�޷�������ת��Ϊ�ַ���*/
extern uint8 *IntToStr(uint16 num,uint8 *str);

/*�޷��ų�����ת��Ϊ�ַ���*/
extern uint8 *LongIntToStr(uint32 num,uint8 *str);

/*�޷�������ת��Ϊ16���Ʊ�ʾ���ַ���*/
extern uint8 *IntToHexStr(uint8 num,uint8 *str);

/*�޷���16λ����ת��Ϊ16���Ʊ�ʾ���ַ���*/
extern uint8 *u16IntToHexStr(uint16 num,uint8 *str);

/*�ַ���������������漰�ַ���������־*/
extern void StrInvertSeq(uint16 strnum,uint8 *str);

/*ʵ��itoa������Դ��*/
extern char *comItoa(int num, char *str, int radix);

/*�������ƴ�ת��Ϊ16���Ƶ��ַ���*/
extern uint8 *BinToHex_Str(uint8 *dest,const uint8 *src,uint16 length);

/*��λ�ַ����������ֵ�ָ���ַ�*/
extern char *_strrchr(const char *str,int ch);

/*����num���ַ�����dest*/
extern uint8 *_strncpy(uint8 *dest,const uint8 *src,uint16 length);

/*�ַ���ת16����*/
extern void str2hex(char* str, char* hex);

/*Ѱ���ض��ַ���*/
extern int Find_string(char *pcBuf,char *left,char *right, char *pcRes);

#endif
