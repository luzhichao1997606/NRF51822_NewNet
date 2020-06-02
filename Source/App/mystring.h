/**
 * mystring.h file for STM32F103.
 * Describtion: 字符串相关操作函数集
 * Author: qinfei 2015.04.02
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */


#ifndef _MY_STRING_H_
#define _MY_STRING_H_
#include "hal.h"

/*字符串测试函数*/
extern void StringTest(void);

/*将src字符串连接到dest字符串的前面*/
extern void strcat_oppo(uint8 *dest,const uint8 *src);

/*字符串清空*/
extern void clcstr(uint8 *src,uint16 length);

/*无符号整数转换为字符串*/
extern uint8 *IntToStr(uint16 num,uint8 *str);

/*无符号长整数转换为字符串*/
extern uint8 *LongIntToStr(uint32 num,uint8 *str);

/*无符号整数转换为16进制表示的字符串*/
extern uint8 *IntToHexStr(uint8 num,uint8 *str);

/*无符号16位整数转换为16进制表示的字符串*/
extern uint8 *u16IntToHexStr(uint16 num,uint8 *str);

/*字符串逆序调整，不涉及字符串结束标志*/
extern void StrInvertSeq(uint16 strnum,uint8 *str);

/*实现itoa函数的源码*/
extern char *comItoa(int num, char *str, int radix);

/*将二进制串转换为16进制的字符串*/
extern uint8 *BinToHex_Str(uint8 *dest,const uint8 *src,uint16 length);

/*定位字符串中最后出现的指定字符*/
extern char *_strrchr(const char *str,int ch);

/*复制num个字符串到dest*/
extern uint8 *_strncpy(uint8 *dest,const uint8 *src,uint16 length);

/*字符串转16进制*/
extern void str2hex(char* str, char* hex);

/*寻找特定字符串*/
extern int Find_string(char *pcBuf,char *left,char *right, char *pcRes);

#endif
