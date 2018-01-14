%include "sconst.inc"


;导入函数
extern cstart			;来自start.c	
extern exception_handler	;来自protect.c
extern SpuriousIrq		;来自i8259.c
extern kernel_main		;来自main.c
extern Delay			;来自kernel_lib_c.c
extern DispInt			;来自kernel_lib_c.c
extern DisplayStr			;来自kernel_lib_asm.asm
extern ClockHandler		;来自clock.c

;导入全局变量
extern gdt_ptr			;来自global.c,用于传递GDTR
extern idt_ptr			;来自global.c,用于传递IDTR
extern disp_pos			;来自global.c,记录打印的位置
extern p_proc_ready;		;来自global.c,指定当前已准备好的进程
extern tss			;来自global.c,储存任务状态段
extern interupt_times		;来自global.c,记录当前中断层数
extern irq_table 		;来自global.c,中断反馈表
extern syscall_table		;来自global.c,记录可用的系统调用



[section .bss]	;静态数据段
StackSpace	resb 	2*1024		;分配2K的堆栈空间
StackTop:	;栈顶指针位置	


[section .text]	

;导出中断/异常的处理例程
;异常
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
;中断
global  hwint00
global  hwint01
global  hwint02
global  hwint03
global  hwint04
global  hwint05
global  hwint06
global  hwint07
global  hwint08
global  hwint09
global  hwint10
global  hwint11
global  hwint12
global  hwint13
global  hwint14
global  hwint15
;系统调用
global SysCall

global Restart	;导出Restart 

global _start	; 导出 _start

_start:	
	mov esp,StackTop	;调整栈顶指针,由loader挪至kernel
	mov dword [disp_pos],0	;置屏幕上的打印位置为0

	sgdt [gdt_ptr]	;保存当前的GDTR,以便cstart更改
	call cstart 	;更新GDT和IDT
			;装配TSS和LDT

	lgdt [gdt_ptr]	;加载新的GDTR
	lidt [idt_ptr]	;加载IDTR

	;跳转
	jmp SELECTOR_KERNEL_CS:init

init:
	;加载测试进程A的TSS
	xor eax,eax
	mov ax,SELECTOR_TSS
	ltr ax

	;
	call kernel_main


;======================================================================
; save
; 发生中断时首先调用该函数,备份寄存器,切入内核栈(如果必要的话),并处理重入
;======================================================================
save:
	;依次备份通用寄存器和段寄存器
	pushad
	push ds
	push es
	push fs
	push gs
	;至此,PCB表项中的备份已完成
	;重置段寄存器
	mov dx,ss
	mov ds,dx
	mov es,dx

	mov esi,esp

	;增加中断嵌套层数
	inc dword [interupt_times]
	cmp dword [interupt_times],0
	;若当前为重入中断,已经在内核栈中了,不需要再切入内核栈
	jne .reenter_push  

	;若当前为非重入中断,则切入内核栈
	mov esp,StackTop
	push Restart	;为中断结束准备返回地址
	jmp [esi+RETADR-P_STACKBASE]	;之前在堆栈中压入了call save后的地址,跳转至该地址继续执行

.reenter_push:
	push Reenter	;为中断结束准备返回地址
	jmp [esi+RETADR-P_STACKBASE]	;之前在堆栈中压入了call save后的地址,跳转至该地址继续执行


;======================================================================
;定义硬件中断的对应例程(handler)
;======================================================================
; ---------------------------------
%macro	hwint_master	1
	;当中断反馈执行到此步时,cpu已自动压入了PCB表项中的ss,esp,eflags,cs和eip,此时指向retaddr的开头
	call save

	;在调用当前中断对应的处理句柄之前,先关闭该中断
	in al, INTERUPT_MASTER_PORT1
	or al, (1 << %1)		
	out INTERUPT_MASTER_PORT1, al

	;可以接手其他8259中断
	mov al, EOI			
	out INTERUPT_MASTER_PORT0, al
	
	;调用该中断对应的处理句柄
	sti	
	push %1
	call [irq_table + 4 * %1]	
	add esp,4
	cli

	;当前中断对应的处理句柄调用完毕,开启该中断
	in al, INTERUPT_MASTER_PORT1
	and al, ~(1 << %1)	
	out INTERUPT_MASTER_PORT1, al

	;跳转至在call save中push的地址
	ret
%endmacro
; ---------------------------------
ALIGN   16
hwint00:                ; Interrupt routine for irq 0 (the clock).
	hwint_master	0	
ALIGN   16
hwint01:                ; Interrupt routine for irq 1 (keyboard)
        hwint_master    1

ALIGN   16
hwint02:                ; Interrupt routine for irq 2 (cascade!)
        hwint_master    2

ALIGN   16
hwint03:                ; Interrupt routine for irq 3 (second serial)
        hwint_master    3

ALIGN   16
hwint04:                ; Interrupt routine for irq 4 (first serial)
        hwint_master    4

ALIGN   16
hwint05:                ; Interrupt routine for irq 5 (XT winchester)
        hwint_master    5

ALIGN   16
hwint06:                ; Interrupt routine for irq 6 (floppy)
        hwint_master    6

ALIGN   16
hwint07:                ; Interrupt routine for irq 7 (printer)
        hwint_master    7

; ---------------------------------
;8259从片
;调用函数,打出当前中断的向量号 
%macro  hwint_slave     1
        push    %1
        call    SpuriousIrq
        add     esp, 4
        hlt
%endmacro
; ---------------------------------

ALIGN   16
hwint08:                ; Interrupt routine for irq 8 (realtime clock).
        hwint_slave     8

ALIGN   16
hwint09:                ; Interrupt routine for irq 9 (irq 2 redirected)
        hwint_slave     9

ALIGN   16
hwint10:                ; Interrupt routine for irq 10
        hwint_slave     10

ALIGN   16
hwint11:                ; Interrupt routine for irq 11
        hwint_slave     11

ALIGN   16
hwint12:                ; Interrupt routine for irq 12
        hwint_slave     12

ALIGN   16
hwint13:                ; Interrupt routine for irq 13 (FPU exception)
        hwint_slave     13

ALIGN   16
hwint14:                ; Interrupt routine for irq 14 (AT winchester)
        hwint_slave     14

ALIGN   16
hwint15:                ; Interrupt routine for irq 15
        hwint_slave     15


;======================================================================
;定义处理器规定的异常的对应例程(handler)
;======================================================================
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call	exception_handler
	add	esp, 8		; 弹出向量号和错误码,让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

;系统调用(实现原理与8259中断反馈基本相同)
;eax,指定调用的函数号
SysCall:
	call save
	push dword [p_proc_ready]

	sti
	push ecx
	push ebx
	;根据eax调用syscall_table中的对应函数
	call [syscall_table + eax * 4]
	add esp,4*3
	;将系统调用后的返回值eax放入进程表中eax的对应位置,以便特权切换时,eax可以顺利传递给原低特权进程
	;esi承接自call save
	mov [esi+EAXREG-P_STACKBASE],eax
	cli
	
	;根据不同情况(是否重入)跳至Restart或Reenter
	ret

;======================================================================
; Restart
; 加载首个进程
;======================================================================
 Restart:	;非重入中断会跳到此处
 	;取出当前需要执行的进程
 	mov esp,[p_proc_ready]
	;加载该进程的ldt
	lldt [esp+P_LDT_SEL]
	;为TSS模拟切换环境
	lea eax,[esp+P_STACKTOP]
	mov dword [tss+TSS3_S_SP0],eax

Reenter:	;重入中断会直接跳到此处
	dec dword [interupt_times]
	;加载该进程的段寄存器
	pop gs
	pop fs
	pop es
	pop ds
	;加载通用寄存器
	popad

	;跳过retaddr
	add esp,4

	iretd
