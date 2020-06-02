/**
 * mystring.c file for STM32F103.
 * Describtion: 字符串相关操作函数集
 * Author: qinfei 2015.04.02
 * Version: GatewayV1.0
 * Support:qf.200806@163.com
 */

#include "mystring.h"
#include "hal.h"
#include <string.h>
#include <stdio.h>

static uint8 dest[100]="adajlngijnlfahogjlhs";
static uint8 src[20]="1234567891234657891";

/*字符串测试函数*/
void StringTest(void)
{
	strcat_oppo(dest,src);
	UART_Printf("%s\n",dest);
}


/**
  * @fun    strcat_oppo
  * @brief  将src字符串连接到dest字符串的前面
  * @param  dest:指向目的字符串的指针
  * @param  src	:指向源字符串的指针
  * @retval None
  */
void strcat_oppo(uint8 *dest,const uint8 *src)
{
	uint16	dlength,slength;
	dlength=(uint16)strlen((const char*)dest);
	slength=(uint16)strlen((const char*)src);
	StrInvertSeq(dlength,dest);
	StrInvertSeq(slength,(uint8 *)src);
	strcat((char*)dest,(const char*)src);
	StrInvertSeq(dlength+slength,dest);
}

/**
  * @fun     clcstr
  * @brief  字符串清空
  * @param  src:指向源字符串的指针
  * @param  length:字符串长度
  * @retval None
  */
void clcstr(uint8 *src,uint16 length)
{
	for(;length>0;length--)
		src[length-1]='\0';
}

/**
  * @fun     IntToStr
  * @brief  无符号整数转换为字符串
  * @param  num	:字符串长度
  * @param  str	:指向字符串的指针
  * @retval str	:返回字符串的指针
  */
uint8 *IntToStr(uint16 num,uint8 *str)
{
	uint8 i=0;
	while(num){
		str[i]=num%10+'0';
		num/=10;
		i++;
	}
	str[i]='\0';
	StrInvertSeq(i,str);
	return str;
}


/**
  * @fun    LongIntToStr
  * @brief  无符号长整数转换为字符串
  * @param  num	:字符串长度
  * @param  str	:指向字符串的指针
  * @retval str	:返回字符串的指针
  */
uint8 *LongIntToStr(uint32 num,uint8 *str)
{
	uint16 i=0;
	while(num){
		str[i]=num%10+'0';
		num/=10;
		i++;
	}
	str[i]='\0';
	StrInvertSeq(i,str);
	return str;
}

/**
  * @fun    IntToHexStr
  * @brief  无符号整数转换为16进制表示的字符串
  * @param  num	:要进行转换的数值变量
  * @param  str	:指向2字节字符串的指针
  * @retval str	:返回2字节字符串的指针
  *	@date	2014/6/6
  */
uint8 *IntToHexStr(uint8 num,uint8 *str)
{
	uint8	i;
	uint8	tdu[2];
	for(i=0,tdu[0]=num/0x10,tdu[1]=num%0x10;i<2;i++)
		str[i]=tdu[i]/10?tdu[i]%10+'A':tdu[i]%10+'0';
	return str;
}

/**
  * @fun    u16IntToHexStr
  * @brief  无符号16位整数转换为16进制表示的字符串
  * @param  num	:要进行转换的数值变量
  * @param  str	:指向4字节字符串的指针
  * @retval str	:返回4字节字符串的指针
  *	@date	2014/6/6
  */
uint8 *u16IntToHexStr(uint16 num,uint8 *str)
{
	uint8	i;
	uint8	tdu[4];
	for(i=0,tdu[0]=num/0x1000,tdu[1]=(num%0x1000)/0x100,tdu[2]=(num%0x100)/0x10,tdu[3]=num%0x10;i<4;i++)
		str[i]=tdu[i]/10?tdu[i]%10+'A':tdu[i]%10+'0';
	str[i] = '\0';
	return str;
}
/**
  * @fun     StrInvertSeq
  * @brief  字符串逆序调整，不涉及字符串结束标志
  * @param  strnum:字符串长度
  * @param  str	:指向字符串的指针
  * @retval None
  */
void StrInvertSeq(uint16 strnum,uint8 *str)
{
	uint8  c;
	uint16 i=0;
	uint16 num=strnum/2;
	for(strnum--;i<num;i++,strnum--){
		c=str[i];
		str[i]=str[strnum];
		str[strnum]=c;
	}
}


/**
  * @fun     comItoa
  * @brief  实现itoa函数的源码
  * @param  num	:要进行转换的数值变量 -123
  * @param  str	:指向字符串的指针 
  * @param  radix	:转移数字时所用的基数，转换基数为10：十进制；2：二进制...
  * @retval str	:返回字符串的指针 "-123"
  */
char *comItoa(int num,char *str,int radix) 
{  
	/* 索引表 */ 
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
	unsigned unum; /* 中间变量 */ 
	int i=0,j,k; 
	/* 确定unum的值 */ 
	if(radix==10&&num<0) /* 十进制负数 */ 
	{ 
		unum=(unsigned)-num; 
		str[i++]='-'; 
	} 
	else unum=(unsigned)num; /* 其它情况 */ 
	/* 逆序 */ 
	do  
	{ 
		str[i++]=index[unum%(unsigned)radix]; 
		unum/=radix; 
	}while(unum); 
	str[i]='\0'; 
	/* 转换 */ 
	if(str[0]=='-') 
	{
		k=1; /* 十进制负数 */ 
		i++;
	}
	else k=0; 
	/* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */ 
	char temp; 
	for(j=k;j<=(i-k-1)/2.0;j++) 
	{ 
		temp=str[j]; 
		str[j]=str[i-j-1]; 
		str[i-j-1]=temp; 
	} 
	return str; 
} 

/**
  * @fun    BinToHex_Str
  * @brief  将二进制串转换为16进制的字符串
  * @param  dest:指向装载字符串的指针
  * @param  src	:指向二进制串的指针
  * @param  length:二进制串的长度
  * @retval dest:返回字符串的指针
  *	@date	2014/6/6
  */
uint8 *BinToHex_Str(uint8 *dest,const uint8 *src,uint16 length)
{
	uint16	i;
	for(i=0;i<length;i++)
		IntToHexStr(src[i],&dest[i*2]);
	return dest;
}

/**
  * @fun     strrchr
  * @brief  定位字符串中最后出现的指定字符
  * @param  str	:指向字符串的指针
  * @param  ch	:字符
  * @retval uint16:返回值
  */
char *_strrchr(const char *str,int ch)
{
	char *start=(char *)str;
	while(*str++); /*get the end of the string*/
	while(--str != start && *str != (char)ch); /**/
	if(*str == (char)ch)
		return((char *)str);
	return NULL;
}

/**
  * @fun    strncpy
  * @brief  复制num个字符串到dest
  * @param  dest:指向装载字符串的指针
  * @param  src	:指向源字符串的指针
  * @param  length:字符串的长度
  * @retval dest:返回字符串的指针
  *	@date	2014/7/19
  */
uint8 *_strncpy(uint8 *dest,const uint8 *src,uint16 length)
{
	uint16	i;
	for(i=0;i<length;i++)	dest[i]=src[i];
	return dest;
}


/*字符转换相关*/
char Value2Hex(const int value)
{
    char Hex = NULL;
    if (value>=0 && value<=9)
    {
        Hex = (char)(value+'0');
    }
    else if (value>9 && value<16)
    {
        Hex = (char)(value-10+'A');
    }
    return Hex;
}
void str2hex(char* str, char* hex)
{

    const char* cHex = "0123456789ABCDEF";
    int i=0;
    for(int j =0; j < strlen(str); j++)
    {
        unsigned int a =  (unsigned int) str[j];
        hex[i++] = cHex[(a & 0xf0) >> 4];
        hex[i++] = cHex[(a & 0x0f)];
    }
    hex[i] = '\0';
}
//int Str2Hex(char *str,char *hex)
//{
//    int high = 0;
//    int low = 0;
//    int temp = 0;
//    if (NULL==str || NULL==hex) {
//        printf("the str or hex is wrong\n");
//        return -1;
//    }
//    if (0==strlen(str)) {
//        printf("the input str is wrong\n");
//        return -2;
//    }
//    while(*str)
//    {
//        temp = (int)(*str);
//        high = temp>>4;
//        low = temp & 15;
//        *hex = Value2Hex(high);
//        hex++;
//        *hex = Value2Hex(low);
//        hex++;
//        str++;
//    }
//    *hex = '\0';
//    return 0;
//}


/***********************************************************
  函数名称：Find_string(char *pcBuf,char*left,char*right, char *pcRes)
  函数功能：寻找特定字符串
  入口参数：
           char *pcBuf 为传入的字符串
           char*left   为搜索字符的左边标识符  例如："["
           char*right  为搜索字符的右边标识符  例如："]"
					 char *pcRes 为输出转存的字符串数组
  返回值：用来校验是否成功，无所谓的。
  备注： left字符需要唯一，right字符从left后面开始唯一即可
 服务器下发命令举例：+MQTTPUBLISH: 0,0,0,0,/device/NB/zx99999999999999_back,6,[reastrobot]
***********************************************************/
int Find_string(char *pcBuf,char *left,char *right, char *pcRes)
{
    char *pcBegin = NULL;
    char *pcEnd = NULL;
    pcBegin = strstr(pcBuf, left);//取出左边数据
    pcEnd = strstr(pcBegin+strlen(left), right);//扫描右边标识
    if(pcBegin == NULL || pcEnd == NULL || pcBegin > pcEnd)
    {
        UART_Printf("string name not found!\n");
        return 0;
    }
    else
    {
        pcBegin += strlen(left);
        memcpy(pcRes, pcBegin, pcEnd-pcBegin);
        return 1;
    }
}

