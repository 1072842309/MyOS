//保护模式下用到的类型声明 & 宏定义

#ifndef _PROTECT_MODE_H_
#define _PROTECT_MODE_H_


//自定义段描述符
//与汇编中的宏定义Descriptor相对应
typedef struct{

	u16 limit_low; 		//段界限低16位
	u16 base_low;		//段基址低16位
	u8 base_mid;		//段基址中8位
	u8 attr_low;		//属性低8位
	u8 limit_high_attr_high;//低4位是段界限高4位,高4位是属性高4位
	u8 base_high;		//段基址高8位
}DESCRIPTOR;


//自定义门描述符
typedef struct{
	u16 offset_low;		//段界限低16位
	u16 selector;		//选择子
	u8 param_count;		//仅在调用门中有效
				//使用调用门时的参数数量
	u8 attr;		//P(7位) DPL(6-5位) S(4位) TYPE(3-0位)
	u16 offset_high;	//段界限高16位
}GATE;

//任务状态描述段(TSS)
typedef struct s_tss {
	u32	backlink;
	u32	esp0;	/* stack pointer to use during interrupt */
	u32	ss0;	/*   "   segment  "  "    "        "     */
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
}TSS;

/* 选择子索引 */
#define	INDEX_DUMMY		0	/* \                         */
#define	INDEX_FLAT_C		1	/* | LOADER 里面已经确定了的 */
#define	INDEX_FLAT_RW		2	/* |                         */
#define	INDEX_VIDEO		3	/* /                         */
#define	INDEX_TSS		4
#define	INDEX_LDT_FIRST		5

//选择子(皆已于loader中定义完毕),此处定义宏仅为了方便
#define SELECTOR_DUMMY		0x00
#define SELECTOR_FLAT_C		0x08
#define SELECTOR_FLAT_RW	0x10
#define SELECTOR_VIDEO		0x18+3	//RPL = 3,可以由低特权级任务修改
#define SELECTOR_TSS		0x20 	//TSS
#define SELECTOR_LDT_FIRST	0x28 	//首个进程


//快捷选择子
#define SELECTOR_KERNEL_CS 	SELECTOR_FLAT_C	//快捷选择子,指向代码段
#define SELECTOR_KERNEL_DS	SELECTOR_FLAT_RW//快捷选择子,指向数据段
#define SELECTOR_KERNEL_GS	SELECTOR_VIDEO	//快捷选择子,指向数据段


//单个LDT中的描述符个数
#define LDT_SIZE		2



/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4

/* 定义描述符时会用到的类型(attr) */
// 描述符类型值说明 
#define	DA_32			0x4000	// 32 位段				
#define	DA_LIMIT_4K		0x8000	// 段界限粒度为 4K 字节			
#define	DA_DPL0			0x00	// DPL = 0				
#define	DA_DPL1			0x20	// DPL = 1				
#define	DA_DPL2			0x40	// DPL = 2				
#define	DA_DPL3			0x60	// DPL = 3				
// 存储段描述符类型值说明 
#define	DA_DR			0x90	// 存在的只读数据段类型值		
#define	DA_DRW			0x92	// 存在的可读写数据段属性值		
#define	DA_DRWA			0x93	// 存在的已访问可读写数据段类型值	
#define	DA_C			0x98	// 存在的只执行代码段属性值		
#define	DA_CR			0x9A	// 存在的可执行可读代码段属性值		
#define	DA_CCO			0x9C	// 存在的只执行一致代码段属性值		
#define	DA_CCOR			0x9E	// 存在的可执行可读一致代码段属性值	
// 系统段描述符类型值说明 
#define	DA_LDT			0x82	// 局部描述符表段类型值			
#define	DA_TaskGate		0x85	// 任务门类型值				
#define	DA_386TSS		0x89	// 可用 386 任务状态段类型值		
#define	DA_386CGate		0x8C	// 386 调用门类型值			
#define	DA_386IGate		0x8E	// 386 中断门类型值			
#define	DA_386TGate		0x8F	// 386 陷阱门类型值			



//中断向量
//处理器定义的异常对应的中断向量
#define	INTERUPT_VECTOR_DIVIDE			0x0
#define	INTERUPT_VECTOR_DEBUG			0x1
#define	INTERUPT_VECTOR_NMI			0x2
#define	INTERUPT_VECTOR_BREAKPOINT		0x3
#define	INTERUPT_VECTOR_OVERFLOW		0x4
#define	INTERUPT_VECTOR_BOUNDS			0x5
#define	INTERUPT_VECTOR_INVAL_OP		0x6
#define	INTERUPT_VECTOR_COPROC_NOT		0x7
#define	INTERUPT_VECTOR_DOUBLE_FAULT		0x8
#define	INTERUPT_VECTOR_COPROC_SEG		0x9
#define	INTERUPT_VECTOR_INVAL_TSS		0xA
#define	INTERUPT_VECTOR_SEG_NOT			0xB
#define	INTERUPT_VECTOR_STACK_FAULT		0xC
#define	INTERUPT_VECTOR_PROTECTION		0xD
#define	INTERUPT_VECTOR_PAGE_FAULT		0xE
#define	INTERUPT_VECTOR_COPROC_ERR		0x10
//8259芯片触发的中断对应的中断向量
#define INTERUPT_VECTOR_IRQ0			0x20	//主片的中断向量对应0x20-0x27
#define INTERUPT_VECTOR_IRQ8			0x28	//从片的中断向量对应0x28-0x2f
//系统调用对应中断向量
#define INTERUPT_VECTOR_SYS_CALL		0x90


//宏函数
//线性地址(基地址,偏移量)->物理地址
#define VirToPhys(seg_base,vir) (u32)(((u32)seg_base) + (u32)(vir))


#endif //_PROTECT_MODE_H_
