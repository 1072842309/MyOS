
#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


//初始化8259时钟中断
PUBLIC void IClock(void){
	//设定8253
	OutputByte(TIMER_MODE,RATE_GENERATOR);
	OutputByte(TIMER0,(u8)(TIMER_FREQ/HZ));
	OutputByte(TIMER0,(u8)((TIMER_FREQ/HZ)>>8));

	//设定时钟的8259中断反馈
	PutIrqHandler(CLOCK_IRQ,ClockHandler);
	EnableIrq(CLOCK_IRQ);
}

//8259时钟中断反馈句柄
PUBLIC void ClockHandler(int irq){
	//时钟记时+1
	ticks++;
	p_proc_ready->ticks--;
	if(interupt_times != 0){
	//	DisplayStr("!");
		return;
	}
	//DisplayStr("#");
	schedule();

	return;
}

//延迟sec毫秒
PUBLIC void MilliDelay(int milli_sec){
	int old = get_ticks();
	while(( (get_ticks() - old) * 1000 / HZ ) < milli_sec);
}
