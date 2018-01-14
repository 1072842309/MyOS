//与建立8259中断响应(反馈)相关的函数

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


/*======================================================================*
 将8259的16个边角注册到中断向量表的0x20~0x2f 
 *======================================================================*/
PUBLIC void I8259A(void){
	OutputByte(INTERUPT_MASTER_PORT0,0x11); 			// Master 8259, ICW1. 
	OutputByte(INTERUPT_SLAVE_PORT0,0x11); 			// Slave  8259, ICW1. 
	OutputByte(INTERUPT_MASTER_PORT1,INTERUPT_VECTOR_IRQ0); 	// Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20 
	OutputByte(INTERUPT_SLAVE_PORT1,INTERUPT_VECTOR_IRQ8); 	// Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28 
	OutputByte(INTERUPT_MASTER_PORT1,0x4); 			// Master 8259, ICW3. IR2 对应 '从8259'. 
	OutputByte(INTERUPT_SLAVE_PORT1,0x2); 	 		// Slave  8259, ICW3. 对应 '主8259' 的 IR2. 
	OutputByte(INTERUPT_MASTER_PORT1,0x1); 			// Master 8259, ICW4. 
	OutputByte(INTERUPT_SLAVE_PORT1,0x1); 			// Slave  8259, ICW4. 
	OutputByte(INTERUPT_MASTER_PORT1,0xFF); 			// Master 8259, OCW1.  
	OutputByte(INTERUPT_SLAVE_PORT1,0xFF); 			// Slave  8259, OCW1.  

	//先为16个边角注册上默认的中断反馈句柄
	for(int i = 0;i < NUMBER_IRQ;i++){
		irq_table[i] = SpuriousIrq;
	}
}


/*======================================================================*
 当触发8259中断时,默认调用该函数
 *======================================================================*/
PUBLIC void SpuriousIrq(int irq){
	DisplayStr("SpuriousIrq: ");
	DispInt(irq);
	DisplayStr("\n");
}

/*======================================================================*
 PutIrqHandler
 为irq设定中断反馈句柄
 *======================================================================*/
PUBLIC void PutIrqHandler(int irq, irq_handler handler)
{
	DisableIrq(irq);		//设定前先关闭该中断
	//将该句柄注册至对应的8259中断反馈表
	irq_table[irq] = handler;
}
