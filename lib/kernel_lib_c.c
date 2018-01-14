//内核iernel中用到的C语言工具函数

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"

/*======================================================================*
  Int2Str
  将int转换为16进制格式的字符串返回
  数字前面的0不被显示出来,比如0000B800被显示成 B800
 *======================================================================*/
PUBLIC char* Int2Str(char * str, int num)
{
	char *	p = str;
	char	ch;
	int	i;
	int	flag = 0;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = 1;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}

/*======================================================================*
  DispInt
  在disp_pos处打印16进制格式的input
 *======================================================================*/
PUBLIC void DispInt(int input){
	char output[16];
	Int2Str(output, input); //在output中保存input的0x版本
	DisplayStr(output);
	return;
}


/*======================================================================*
 Delay
 延迟指定时间(粗略)
 *======================================================================*/
PUBLIC void Delay(int time)
{
	for (int i = 0; i < time; i++)
		for(int k = 0;k < 10;k++){
			for (int j = 0; j < 10000; j++);
		}

}










