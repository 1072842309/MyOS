//在保护模式下内核需要用到的一些重要的函数


#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"

//内部函数声明
//初始化IDT(门)描述符
PRIVATE void IIdtDesc(unsigned char vector, u8 desc_type,interupt_handler handler, unsigned char privilege);
//初始化GDT(一般)描述符
PRIVATE void IDesc(DESCRIPTOR * p_desc, u32 base, u32 limit, u16 attribute);

//从kernel.asm导入中断/异常的处理例程,以便为他们注册到对应的向量号上去
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();
void    hwint00();
void    hwint01();
void    hwint02();
void    hwint03();
void    hwint04();
void    hwint05();
void    hwint06();
void    hwint07();
void    hwint08();
void    hwint09();
void    hwint10();
void    hwint11();
void    hwint12();
void    hwint13();
void    hwint14();
void    hwint15();


/*======================================================================*
 将引入的例程初始化成中断门,并注册到对应的中断向量
 *======================================================================*/
PUBLIC void IProt(){
        //将8259的16个边角注册到中断向量表 
	I8259A();

	// 全部初始化成中断门(没有陷阱门)
	IIdtDesc(INTERUPT_VECTOR_DIVIDE,	DA_386IGate,
		    divide_error,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_DEBUG,	DA_386IGate,
		    single_step_exception,	PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_NMI,	DA_386IGate,
		    nmi,			PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_BREAKPOINT,	DA_386IGate,
		    breakpoint_exception,	PRIVILEGE_USER);

	IIdtDesc(INTERUPT_VECTOR_OVERFLOW,	DA_386IGate,
		    overflow,			PRIVILEGE_USER);

	IIdtDesc(INTERUPT_VECTOR_BOUNDS,	DA_386IGate,
		    bounds_check,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_INVAL_OP,	DA_386IGate,
		    inval_opcode,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_COPROC_NOT,	DA_386IGate,
		    copr_not_available,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_DOUBLE_FAULT,DA_386IGate,
		    double_fault,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_COPROC_SEG,	DA_386IGate,
		    copr_seg_overrun,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_INVAL_TSS,	DA_386IGate,
		    inval_tss,			PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_SEG_NOT,	DA_386IGate,
		    segment_not_present,	PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_STACK_FAULT,DA_386IGate,
		    stack_exception,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_PROTECTION,	DA_386IGate,
		    general_protection,		PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_PAGE_FAULT,	DA_386IGate,
		    page_fault,			PRIVILEGE_KRNL);

	IIdtDesc(INTERUPT_VECTOR_COPROC_ERR,	DA_386IGate,
		    copr_error,			PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 0,   DA_386IGate,
                    hwint00,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 1,   DA_386IGate,
                    hwint01,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 2,   DA_386IGate,
                    hwint02,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 3,   DA_386IGate,
                    hwint03,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 4,   DA_386IGate,
                    hwint04,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 5,   DA_386IGate,
                    hwint05,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 6,   DA_386IGate,
                    hwint06,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ0 + 7,   DA_386IGate,
                    hwint07,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 0,   DA_386IGate,
                    hwint08,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 1,   DA_386IGate,
                    hwint09,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 2,   DA_386IGate,
                    hwint10,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 3,   DA_386IGate,
                    hwint11,                  	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 4,   DA_386IGate,
                    hwint12,                  	PRIVILEGE_KRNL);

       IIdtDesc(INTERUPT_VECTOR_IRQ8 + 5, 	DA_386IGate,
                    hwint13,                	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 6,	DA_386IGate,
                    hwint14,            	PRIVILEGE_KRNL);

        IIdtDesc(INTERUPT_VECTOR_IRQ8 + 7,	DA_386IGate,
                    hwint15,                 	PRIVILEGE_KRNL);
	
	//将中断号INT_VECTOR_SYS_CALL注册为系统调用
	IIdtDesc(INTERUPT_VECTOR_SYS_CALL,	DA_386IGate,
		    SysCall,			PRIVILEGE_USER);

	//填充 GDT 中 TSS 这个描述符
	MemSet(&tss, 0, sizeof(tss));
	tss.ss0 = SELECTOR_KERNEL_DS;
	IDesc(&gdt[INDEX_TSS],
			VirToPhys(Seg2Phy(SELECTOR_KERNEL_DS), &tss),
			sizeof(tss) - 1,
			DA_386TSS);
	tss.iobase = sizeof(tss); /* 没有I/O许可位图 */

	//填充GDT中每个进程对应的LDT的描述符
	for(int i = 0;i < NUMBER_TASKS + NUMBER_USER_PROCS;i++){
		IDesc(&gdt[INDEX_LDT_FIRST+i],
			VirToPhys(Seg2Phy(SELECTOR_KERNEL_DS), proc_table[i].ldts),
			LDT_SIZE * sizeof(DESCRIPTOR) - 1,
			DA_LDT);
	}

}



/*======================================================================*
  IIdtDesc
  在IDT中为指定中断向量装配中断门
 *======================================================================*/
PRIVATE void IIdtDesc(unsigned char vector, u8 desc_type,
			  interupt_handler handler, unsigned char privilege)
{
	GATE *	gate	= &idt[vector];
	u32	base	= (u32)handler;
	gate->offset_low	= base & 0xFFFF;
	gate->selector	= SELECTOR_KERNEL_CS;
	gate->param_count	= 0;
	gate->attr		= desc_type | (privilege << 5);
	gate->offset_high	= (base >> 16) & 0xFFFF;
}

/*======================================================================*
  Seg2Phy
  32位GDT描述符 -> 指定段基的绝对地址
 *======================================================================*/
PUBLIC u32 Seg2Phy(u16 seg){

	//摘除低三位的TI和RPL
	DESCRIPTOR *p_dest = &gdt[seg>>3];
	//从对应描述符中取出地址
	return (p_dest->base_high<<24 | p_dest->base_mid<<16 | p_dest->base_low); 
}




/*======================================================================*
  IDesc
  初始化GDT描述符
 *======================================================================*/
PRIVATE void IDesc(DESCRIPTOR * desc, u32 base, u32 limit, u16 attribute){
	desc->limit_low	= limit & 0x0FFFF;
	desc->base_low	= base & 0x0FFFF;
	desc->base_mid	= (base >> 16) & 0x0FF;
	desc->attr_low	= attribute & 0xFF;
	desc->limit_high_attr_high= ((limit>>16) & 0x0F) | (attribute>>8) & 0xF0;
	desc->base_high	= (base >> 24) & 0x0FF;
}

/*======================================================================*
  exception_handler
  针对异常的处理例程 
 *======================================================================*/

PUBLIC void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	int i;
	int text_color = 0x74; /* 灰底红字 */

	char * err_msg[] = {"#DE Divide Error",
			    "#DB RESERVED",
			    "—  NMI Interrupt",
			    "#BP Breakpoint",
			    "#OF Overflow",
			    "#BR BOUND Range Exceeded",
			    "#UD Invalid Opcode (Undefined Opcode)",
			    "#NM Device Not Available (No Math Coprocessor)",
			    "#DF Double Fault",
			    "    Coprocessor Segment Overrun (reserved)",
			    "#TS Invalid TSS",
			    "#NP Segment Not Present",
			    "#SS Stack-Segment Fault",
			    "#GP General Protection",
			    "#PF Page Fault",
			    "—  (Intel reserved. Do not use.)",
			    "#MF x87 FPU Floating-Point Error (Math Fault)",
			    "#AC Alignment Check",
			    "#MC Machine Check",
			    "#XF SIMD Floating-Point Exception"
	};

	/* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
	disp_pos = 0;
	for(i=0;i<80*5;i++){
		DisplayStr(" ");
	}
	disp_pos = 0;

	DispColorStr("Exception! --> ", text_color);
	DispColorStr(err_msg[vec_no], text_color);
	DispColorStr("\n\n", text_color);
	DispColorStr("EFLAGS:", text_color);
	DispInt(eflags);
	DispColorStr("CS:", text_color);
	DispInt(cs);
	DispColorStr("EIP:", text_color);
	DispInt(eip);

	if(err_code != 0xFFFFFFFF){
		DispColorStr("Error code:", text_color);
		DispInt(err_code);
	}
}
