#include "type.h"	
#include "const.h"
#include "string.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


PUBLIC void cstart(void){
	DisplayStr("\n\n\n\n\n\n\n\n\n\n\n\n\n\ncstart\n");
	//将gdt转移至新表中
	MemCpy(&gdt,				//新GDT表基址
	       (void*)(*((u32*)(&gdt_ptr[2]))),	//旧GDT表基址
	       *((u16*)(&gdt_ptr[0])) + 1);	//GDT表的大小
	
	//根据新表设置新的GDTR
	u16 *pgdt_limit = (u16*)(&gdt_ptr[0]);
	*pgdt_limit = GDT_SIZE * sizeof(DESCRIPTOR)-1; //GDT表的界限值
	u32 *pgdt_base = (u32*)(&gdt_ptr[2]);
	*pgdt_base = (u32)&gdt;	//GDT表的基址


	//根据全局变量和宏设置IDTR
	u16* pidt_limit = (u16*)(&idt_ptr[0]);
	u32* pidt_base  = (u32*)(&idt_ptr[2]);
	*pidt_limit = IDT_SIZE * sizeof(GATE) - 1;
	*pidt_base  = (u32)&idt;

	//设置中断向量表IDT
	IProt();

	DisplayStr("------cstart ends---------\n");

}
