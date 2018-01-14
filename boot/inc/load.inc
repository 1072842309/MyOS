BaseOfLoader		equ	9000h	;LOADER.BIN被加载到的位置的段地址
OffOfLoader		equ	0100h	;LOADER.BIN被加载到的位置的偏移地址
BaseOfLoaderPhysicalAddress	equ	BaseOfLoader*10h	;LOADER.BIN被加载到的位置的物理地址

BaseOfKernel		equ	8000h	;KERNEL.BIN被加载到的位置的段地址
OffOfKernel		equ	0h	;KERNEL.BIN被加载到的位置的偏移地址
BaseOfKernelPhyAddr	equ	BaseOfKernel*10h	;KERNEL.BIN被加载到的物理地址

KernelEntryPhysicalAddress	equ	030400h	;内核的入口地址,应与内核ld时-Ttext的参数值相等

PageDirectoryBase		equ	200000h	;页目录开始地址:2M
PageTableBase		equ	201000h	;页表开始地址:2M+4K
