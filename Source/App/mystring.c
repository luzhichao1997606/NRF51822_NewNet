/**
 * mystring.c file for STM32F103.
 * Describtion: �ַ�����ز���������
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

/*�ַ������Ժ���*/
void StringTest(void)
{
	strcat_oppo(dest,src);
	UART_Printf("%s\n",dest);
}


/**
  * @fun    strcat_oppo
  * @brief  ��src�ַ������ӵ�dest�ַ�����ǰ��
  * @param  dest:ָ��Ŀ���ַ�����ָ��
  * @param  src	:ָ��Դ�ַ�����ָ��
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
  * @brief  �ַ������
  * @param  src:ָ��Դ�ַ�����ָ��
  * @param  length:�ַ�������
  * @retval None
  */
void clcstr(uint8 *src,uint16 length)
{
	for(;length>0;length--)
		src[length-1]='\0';
}

/**
  * @fun     IntToStr
  * @brief  �޷�������ת��Ϊ�ַ���
  * @param  num	:�ַ�������
  * @param  str	:ָ���ַ�����ָ��
  * @retval str	:�����ַ�����ָ��
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
  * @brief  �޷��ų�����ת��Ϊ�ַ���
  * @param  num	:�ַ�������
  * @param  str	:ָ���ַ�����ָ��
  * @retval str	:�����ַ�����ָ��
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
  * @brief  �޷�������ת��Ϊ16���Ʊ�ʾ���ַ���
  * @param  num	:Ҫ����ת������ֵ����
  * @param  str	:ָ��2�ֽ��ַ�����ָ��
  * @retval str	:����2�ֽ��ַ�����ָ��
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
  * @brief  �޷���16λ����ת��Ϊ16���Ʊ�ʾ���ַ���
  * @param  num	:Ҫ����ת������ֵ����
  * @param  str	:ָ��4�ֽ��ַ�����ָ��
  * @retval str	:����4�ֽ��ַ�����ָ��
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
  * @brief  �ַ���������������漰�ַ���������־
  * @param  strnum:�ַ�������
  * @param  str	:ָ���ַ�����ָ��
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
  * @brief  ʵ��itoa������Դ��
  * @param  num	:Ҫ����ת������ֵ���� -123
  * @param  str	:ָ���ַ�����ָ�� 
  * @param  radix	:ת������ʱ���õĻ�����ת������Ϊ10��ʮ���ƣ�2��������...
  * @retval str	:�����ַ�����ָ�� "-123"
  */
char *comItoa(int num,char *str,int radix) 
{  
	/* ������ */ 
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
	unsigned unum; /* �м���� */ 
	int i=0,j,k; 
	/* ȷ��unum��ֵ */ 
	if(radix==10&&num<0) /* ʮ���Ƹ��� */ 
	{ 
		unum=(unsigned)-num; 
		str[i++]='-'; 
	} 
	else unum=(unsigned)num; /* ������� */ 
	/* ���� */ 
	do  
	{ 
		str[i++]=index[unum%(unsigned)radix]; 
		unum/=radix; 
	}while(unum); 
	str[i]='\0'; 
	/* ת�� */ 
	if(str[0]=='-') 
	{
		k=1; /* ʮ���Ƹ��� */ 
		i++;
	}
	else k=0; 
	/* ��ԭ���ġ�/2����Ϊ��/2.0������֤��num��16~255֮�䣬radix����16ʱ��Ҳ�ܵõ���ȷ��� */ 
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
  * @brief  �������ƴ�ת��Ϊ16���Ƶ��ַ���
  * @param  dest:ָ��װ���ַ�����ָ��
  * @param  src	:ָ������ƴ���ָ��
  * @param  length:�����ƴ��ĳ���
  * @retval dest:�����ַ�����ָ��
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
  * @brief  ��λ�ַ����������ֵ�ָ���ַ�
  * @param  str	:ָ���ַ�����ָ��
  * @param  ch	:�ַ�
  * @retval uint16:����ֵ
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
  * @brief  ����num���ַ�����dest
  * @param  dest:ָ��װ���ַ�����ָ��
  * @param  src	:ָ��Դ�ַ�����ָ��
  * @param  length:�ַ����ĳ���
  * @retval dest:�����ַ�����ָ��
  *	@date	2014/7/19
  */
uint8 *_strncpy(uint8 *dest,const uint8 *src,uint16 length)
{
	uint16	i;
	for(i=0;i<length;i++)	dest[i]=src[i];
	return dest;
}


/*�ַ�ת�����*/
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
  �������ƣ�Find_string(char *pcBuf,char*left,char*right, char *pcRes)
  �������ܣ�Ѱ���ض��ַ���
  ��ڲ�����
           char *pcBuf Ϊ������ַ���
           char*left   Ϊ�����ַ�����߱�ʶ��  ���磺"["
           char*right  Ϊ�����ַ����ұ߱�ʶ��  ���磺"]"
					 char *pcRes Ϊ���ת����ַ�������
  ����ֵ������У���Ƿ�ɹ�������ν�ġ�
  ��ע�� left�ַ���ҪΨһ��right�ַ���left���濪ʼΨһ����
 �������·����������+MQTTPUBLISH: 0,0,0,0,/device/NB/zx99999999999999_back,6,[reastrobot]
***********************************************************/
int Find_string(char *pcBuf,char *left,char *right, char *pcRes)
{
    char *pcBegin = NULL;
    char *pcEnd = NULL;
    pcBegin = strstr(pcBuf, left);//ȡ���������
    pcEnd = strstr(pcBegin+strlen(left), right);//ɨ���ұ߱�ʶ
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

