//全局变量声明/定义
//当前头文件巧妙地利用了宏标签,使得该文件内的变量在global.c中被定义,在其余包含该头文件的文件中被声明(extern)

#ifndef _GLOBAL_H_
#define _GLOBAL_H_


#ifdef _GLOBAL_C_	//该标签仅在global.c中存在,
#undef EXTERN		//取消了EXTERN的原有功效,使得在global.c中,本应作为声明变量的语句,
#define EXTERN		//变成了定义变量的语句
#endif


EXTERN int ticks;	//当前的时钟中断次数
EXTERN int disp_pos;	//在显存中打印字符串时,会根据该值选择打印位置,并在打印结束后更新该值
EXTERN u8 gdt_ptr[6];	//转移至内核后的GDTR
EXTERN DESCRIPTOR gdt[GDT_SIZE];	//转移至内核后的GDT表

EXTERN u8 idt_ptr[6];	//IDTR
EXTERN GATE idt[IDT_SIZE];	//IDT,中断向量表

EXTERN TSS tss;		//任务状态段
EXTERN PROCESS *p_proc_ready;	//当前可执行的进程

EXTERN u32 interupt_times;	//记录当前中断的层数

EXTERN int number_crt_console;	//当前正在运行的控制台

extern PROCESS proc_table[];		//PCB
extern TASK task_table[];		//任务表
extern TASK usr_proc_table[];		//用户进程表
extern unsigned char task_stack[];	//分配给进程的堆栈

extern irq_handler irq_table[];		//irq中断反馈表

extern system_call syscall_table[];	//系统调用表

//tty与console表
extern TTY tty_table[NUMBER_CONSOLES];
extern CONSOLE console_table[NUMBER_CONSOLES];

#endif	//_GLOBAL_H_
