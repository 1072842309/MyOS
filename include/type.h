//全局会用到的类型定义
#ifndef _TYPE_H_
#define _TYPE_H_

//自定义类型,便于理解
typedef unsigned char u8;	//8位
typedef unsigned short u16;	//16位
typedef unsigned int u32;	//32位

typedef char* va_list;		//可变函参包


//注册调用门时,被调用函数的类型
typedef void (*interupt_handler)();

//进程函数的句柄
typedef void (*task_f)();

//中断反馈句柄
typedef void (*irq_handler)(int irq);

//系统调用句柄
typedef void *system_call;



#endif	//_TYPE_H_
