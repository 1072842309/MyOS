#include "type.h"	
#include "const.h"
#include "string.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


/*======================================================================*
 kernel_main
 准备进程
 *======================================================================*/
PUBLIC int kernel_main(void){
	DisplayStr("kernel_main begin\n");

	TASK *p_task = task_table+0;				//当前进程调用函数在任务表中的对应项
	PROCESS *p_proc = proc_table+0; 			//当前进程在进程表中的对应项
	char *p_task_stack = task_stack+STACK_SIZE_TOTAL;	//当前进程对应的堆栈顶
	u16 selector_ldt = SELECTOR_LDT_FIRST;			//当前进程对应的LDT选择子
	
	u8 privilege;		//进程的特权级
	u8 rpl;			//进程的RPL
	int eflags;		//进程的标志位
	//装配所有可运行进程
	for(int i = 0;i < NUMBER_TASKS + NUMBER_USER_PROCS;i++){
		//1.装配特权级
		if(i < NUMBER_TASKS){	//任务
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
		}
		else{	//用户进程
			p_task = usr_proc_table + (i - NUMBER_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
		}
		//2.装配PCB表项
		//初始化当前进程LDT
		p_proc->ldt_sel = selector_ldt;
		//将GDT第1个描述符的改版(调整其DPL为1)赋给LDT中的第0个描述符
		MemCpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr_low = DA_C | privilege << 5;	
		//将GDT第2个描述符的改版(调整其DPL为1)赋给LDT中的第1个描述符
		MemCpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr_low = DA_DRW | privilege << 5;
	
		//段寄存器中,除gs外都指向LDT内的描述符
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;	
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;	
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
	
		//
		p_proc->regs.eip= (u32)p_task->init_eip;	//当前进程对应函数的入口地址
		p_proc->regs.esp= (u32)p_task_stack;		//当前进程对应的堆栈顶
		p_proc->regs.eflags = eflags;			// IF=1, IOPL=1, bit 2 is always 1.

		//进程的标识符
		StrCpy(p_proc->p_name,p_task->name);
		p_proc->pid = i;
		
		//为下一个待更新进程准备路标
		p_task++;
		p_proc++;
		p_task_stack -= p_task->stack_size;
		selector_ldt += 1 << 3;
	}

	//设置每个进程的优先级和剩余时间
	proc_table[0].ticks = proc_table[0].priority = 300;
	proc_table[1].ticks = proc_table[1].priority = 50;
	proc_table[2].ticks = proc_table[2].priority = 30;
	proc_table[3].ticks = proc_table[3].priority = 10;

	//设置每个进程所属的tty
	for(p_proc = proc_table;p_proc < proc_table+NUMBER_TASKS+NUMBER_USER_PROCS;p_proc++){
	       p_proc->number_tty = 0;
	}
	proc_table[1].number_tty = 0;
	proc_table[2].number_tty = 1;
	proc_table[3].number_tty = 1;



	//准备运行PCB表中的首进程
	p_proc_ready = proc_table;

	//初始化嵌套层数
	interupt_times = 0;

	//初始化时钟中断次数
	ticks = 0;

	//初始化8259中断处理
	IClock();
	IKeyboard();

	//执行该进程
	Restart();

	while(1);
}

/*======================================================================*
  TestA
  首个测试进程
 *======================================================================*/
void TestA(){
	while(1){
		//DisplayStr("A");
		//DispInt(get_ticks());
		//DisplayStr(".");
		//MilliDelay(100);
	}
}

/*======================================================================*
  TestB
  测试进程B
 *======================================================================*/
 void TestB(void){
	int i = 0x1000;
	Printf("nihao%x",12);
	while(1){
		DisplayStr("P1");
		//DispInt(i++);
		//DisplayStr(".");
		MilliDelay(40000);
	}
 }

/*======================================================================*
  TestC
  测试进程C
 *======================================================================*/
 void TestC(void){
	int i = 0x2000;
	while(1){
		DisplayStr("P2");
		//DispInt(i++);
		//DisplayStr(".");
		MilliDelay(10000);
	}
 }
