#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


//系统调用将会调用的函数
int syscall_get_ticks(void){
	return ticks;
}

//进程调度函数
//将当前进程切换至剩余时间最多的进程,
//若所有进程的ticks都为0,则还原为priority
void schedule(void){
	int greatest_ticks = 0;

	while(!greatest_ticks){
		//在当前进程表中寻找剩余时间最大的进程
		for(PROCESS *p = proc_table;p < proc_table+NUMBER_TASKS+NUMBER_USER_PROCS;p++){
			if(p->ticks > greatest_ticks){
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}
		//若剩余时间皆为0,则还原每个进程的剩余时间为其priority
		if(!greatest_ticks){
			for(PROCESS *p = proc_table;p < proc_table+NUMBER_TASKS+NUMBER_USER_PROCS;p++){
				p->ticks = p->priority;
			}
		}
	}

}
