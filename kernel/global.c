//全局变量



#define _GLOBAL_C_


#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


//PCB表
PUBLIC PROCESS proc_table[NUMBER_TASKS+NUMBER_USER_PROCS];
//任务表(ring0)
PUBLIC TASK task_table[NUMBER_TASKS] = { {TaskTTY,STACK_SIZE_TTY,"TTY"} };
//用户进程表(ring3)
PUBLIC TASK usr_proc_table[NUMBER_USER_PROCS] = { 
					{TestA,STACK_SIZE_TESTA,"TESTA"},
					{TestB,STACK_SIZE_TESTB,"TESTB"},
				     	{TestC,STACK_SIZE_TESTC,"TESTC"} }; 

//分配给所有进程的堆栈总和
PUBLIC unsigned char task_stack[STACK_SIZE_TOTAL];

//8259中断反馈表
PUBLIC irq_handler irq_table[NUMBER_IRQ];

//系统调用表
PUBLIC system_call syscall_table[NUMBER_SYS_CALL] = { syscall_get_ticks,syscall_write };

//tty与console表
PUBLIC TTY tty_table[NUMBER_CONSOLES];
PUBLIC CONSOLE console_table[NUMBER_CONSOLES];
