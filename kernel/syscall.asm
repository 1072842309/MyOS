;操作系统为低特权进程提供的系统调用

%include "sconst.inc"

;导出函数
global get_ticks
global write

_SYSCALL_get_ticks		equ	0	;get_ticks调用
_SYSCALL_write			equ	1	;write调用
INTERUPT_VECTOR_SYS_CALL	equ	0x90	;int号




bits 32
[section .text]
get_ticks:
	mov eax,_SYSCALL_get_ticks
	int INTERUPT_VECTOR_SYS_CALL
	ret

write:
	mov eax,_SYSCALL_write
	mov ebx,[esp+4]		;首参数
	mov ecx,[esp+8]		;第二参数
	int INTERUPT_VECTOR_SYS_CALL
	ret
