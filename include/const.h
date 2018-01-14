//全局会用到的宏定义

#ifndef _CONST_H_
#define _CONST_H_

#define EXTERN extern

//类型定义
//函数类型
#define PUBLIC 		//自定义函数类型,与PRIVATE相对,链接文件间共享
#define PRIVATE static	//自定义函数类型,仅在所在文件内可见

//宏常量
//描述符个数
#define GDT_SIZE 128 	//在内核中创建新的GDT表时,默认的表大小
#define IDT_SIZE 256	//中断向量表的大小
//权限
#define PRIVILEGE_KRNL	0
#define PRIVILEGE_TASK	1
#define PRIVILEGE_USER	3
//RPL 
#define	RPL_KRNL SA_RPL0
#define	RPL_TASK SA_RPL1
#define	RPL_USER SA_RPL3

//颜色相关
#define DEFAULT_COLOR 0x0F

//----8259中断相关
//8259主从片对应的端口号
#define INTERUPT_MASTER_PORT0	0x20	//主片端口0x20
#define INTERUPT_MASTER_PORT1	0x21	//主片端口0x21
#define INTERUPT_SLAVE_PORT0	0xA0	//从片端口0xA0
#define INTERUPT_SLAVE_PORT1	0xA1	//从片端口0xA1
//8259中断反馈句柄个数
#define NUMBER_IRQ 16	
//8259中断对应序号
#define	CLOCK_IRQ	0
#define	KEYBOARD_IRQ	1
#define	CASCADE_IRQ	2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	5	/* xt winchester */
#define	FLOPPY_IRQ	6	/* floppy disk */
#define	PRINTER_IRQ	7
#define	AT_WINI_IRQ	14	/* at winchester */

//----8253计数器相关
#define TIMER0		0x40	//端口counter0
#define TIMER_MODE	0x43	//8253模式控制寄存器
#define RATE_GENERATOR	0x34	//输出至端口counter0
#define TIMER_FREQ	1193182L//
#define HZ		100	//每1/HZ秒,发生一次时钟中断,ticks加1	

//----系统调用相关
#define NUMBER_SYS_CALL	2	//系统调用个数

//-----CRT相关(光标)
#define CRT_ADDR_REG	0x3D4	//指定修改数据的索引
#define CRT_DATA_REG	0x3D5	//提供用于修改数据的值
#define CURSOR_H	0xE	//光标坐标的高8位
#define CURSOR_L	0xF	//光标位置的低8位
#define START_ADDR_H	0xC	//显示区开始行地址的高8位
#define START_ADDR_L 	0xD	//显示区开始行地址的低8位
#define VIDEO_MEM_BASE	0xB8000	//显存的基地址
#define VIDEO_MEMORY_SIZE	0x8000	//显存分配的总大小
//-----TTY相关
#define NUMBER_CONSOLES	3	//支持的终端数量






#endif	//_CONST_H_
