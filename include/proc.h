
typedef struct s_stackframe {
	u32	gs;		/* \                                    */
	u32	fs;		/* |                                    */
	u32	es;		/* |                                    */
	u32	ds;		/* |                                    */
	u32	edi;		/* |                                    */
	u32	esi;		/* | pushed by save()                   */
	u32	ebp;		/* |                                    */
	u32	kernel_esp;	/* <- 'popad' will ignore it            */
	u32	ebx;		/* |                                    */
	u32	edx;		/* |                                    */
	u32	ecx;		/* |                                    */
	u32	eax;		/* /                                    */
	u32	retaddr;	/* return addr for kernel.asm::save()   */
	u32	eip;		/* \                                    */
	u32	cs;		/* |                                    */
	u32	eflags;		/* | pushed by CPU during interrupt     */
	u32	esp;		/* |                                    */
	u32	ss;		/* /                                    */
}STACK_FRAME;


typedef struct s_proc {
	STACK_FRAME regs;		//保存当前进程的寄存器  

	u16 ldt_sel;			//当前进程的LDT选择子
	DESCRIPTOR ldts[LDT_SIZE];	//当前进程的LDT

	int ticks;			//当前进程剩余可执行次数
	int priority;			//当前进程优先级

	int number_tty;			//对应的tty

	u32 pid;			//当前进程的id
	char p_name[16];		//当前进程的名字
}PROCESS;

typedef struct s_task{
	task_f init_eip;	//该进程对应函数的入口地址
	int stack_size;		//该进程所属堆栈的大小
	char name[32];		//该进程的名字(id)
}TASK;

//进程数
#define NUMBER_TASKS	1	//(系统调用)任务数
#define NUMBER_USER_PROCS	3	//用户进程数

//分配给进程TESTA的堆栈的大小
#define STACK_SIZE_TESTA	0x8000
//分配给进程TESTB的堆栈的大小
#define STACK_SIZE_TESTB	0x8000
//分配给进程TESTC的堆栈的大小
#define STACK_SIZE_TESTC	0x8000
//分配给进程tty的堆栈的大小
#define STACK_SIZE_TTY		0x8000

//分配给所有进程的堆栈的总大小
#define STACK_SIZE_TOTAL	STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC + STACK_SIZE_TTY
