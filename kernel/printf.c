#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "string.h"
#include "prototype.h"

int SPrintf(char *buf,const char *format,va_list arg){
	char *p;
	char temp[256];
	
	va_list next_arg = arg;

	for(p = buf;*format;format++){
		if(*format != '%'){
			*p++ = *format;
			continue;
		}
		format++;
		//匹配特殊字符
		switch(*format){
		case 'x':
			Int2Str(temp,*((int*)next_arg));
			StrCpy(p,temp);
			next_arg += 4;
			p += StrLen(temp);
			break;
		default:
			break;
		}
	}
	return p - buf;
}


int Printf(const char *format,...){
	char buf[256];	//存放转换后的字符串
	va_list arg = ((char*)&format + 4);
	int i = SPrintf(buf,format,arg);
	write(buf,i);

	return i;
}


