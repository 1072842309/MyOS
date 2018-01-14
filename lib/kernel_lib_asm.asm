;内核kernel中用到的汇编语言工具函数
%include "sconst.inc"

extern disp_pos	;从全局变量中导入

[SECTION .text]

; 导出函数
global DisplayStr
global DispColorStr 
global OutputByte
global InputByte
global DisableIrq
global EnableIrq
global DisableInterupt
global EnableInterupt

; ========================================================================
; void DisplayStr(char * info);
; 在disp_pos处打印info
; ========================================================================
DisplayStr:
	push ebp
	mov ebp, esp

	mov esi, [ebp + 8] ;info
	mov edi, [disp_pos]
	mov ah, 0Fh ;预设颜色
_1:
	lodsb
	test al, al
	jz _2
	cmp al, 0Ah	; 是回车吗?
	jnz _3
	push eax
	mov eax, edi
	mov bl, 160
	div bl
	and eax, 0FFh
	inc eax
	mov bl, 160
	mul bl
	mov edi, eax
	pop eax
	jmp _1
_3:
	mov [gs:edi], ax
	add edi, 2
	jmp _1

_2:
	mov [disp_pos], edi

	pop	ebp
	ret


; ========================================================================
; void DispColorStr(char * info,int color);
; 在disp_pos处打印info,color指定颜色
; ========================================================================
DispColorStr:
	push ebp
	mov ebp, esp

	mov esi, [ebp + 8]	;info
	mov edi, [disp_pos]
	mov ah, [ebp + 12] ;color
.1:
	lodsb
	test al, al
	jz .2
	cmp al, 0Ah	; 是回车吗?
	jnz .3
	push eax
	mov eax, edi
	mov bl, 160
	div bl
	and eax, 0FFh
	inc eax
	mov bl, 160
	mul bl
	mov edi, eax
	pop eax
	jmp .1
.3:
	mov [gs:edi], ax
	add edi, 2
	jmp .1

.2:
	mov [disp_pos], edi

	pop ebp
	ret

; ========================================================================
; void OutputByte(u16 port, u8 value);
; 向端口port中写出value 
; ========================================================================
OutputByte:
	mov edx, [esp + 4]		; port
	mov al, [esp + 8]	; value
	out dx, al
	nop ;一点延迟
	nop
	ret

; ========================================================================
; u8 InputByte(u16 port);
; 从端口port中读入值,并返回 
; ========================================================================
InputByte:
	mov edx, [esp + 4]		; port
	xor eax, eax
	in al, dx
	nop ; 一点延迟
	nop
	ret
	

; ========================================================================
;                  void DisableIrq(int irq);
; ========================================================================
; 等效代码:
;	if(irq < 8)
;		out_byte(INTERUPT_MASTER_PORT1, in_byte(INTERUPT_MASTER_PORT0MASK) | (1 << irq));
;	else
;		out_byte(INTERUPT_SLAVE_PORT1, in_byte(INTERUPT_SLAVE_PORT0MASK) | (1 << irq));
DisableIrq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, 1
        rol     ah, cl                  ; ah = (1 << (irq % 8))
        cmp     cl, 8
        jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
        in      al, INTERUPT_MASTER_PORT1
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INTERUPT_MASTER_PORT1, al       ; set bit at master 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
disable_8:
        in      al, INTERUPT_SLAVE_PORT1
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INTERUPT_SLAVE_PORT1, al       ; set bit at slave 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
dis_already:
        popf
        xor     eax, eax                ; already disabled
        ret
 
; ========================================================================
;                  void EnableIrq(int irq);
; ========================================================================
; 等效代码:
;       if(irq < 8)
;               out_byte(INTERUPT_MASTER_PORT1, in_byte(INTERUPT_MASTER_PORT0MASK) & ~(1 << irq));
;       else
;               out_byte(INTERUPT_SLAVE_PORT1, in_byte(INTERUPT_SLAVE_PORT0MASK) & ~(1 << irq));
;
EnableIrq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, ~1
        rol     ah, cl                  ; ah = ~(1 << (irq % 8))
        cmp     cl, 8
        jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
        in      al, INTERUPT_MASTER_PORT1
        and     al, ah
        out     INTERUPT_MASTER_PORT1, al       ; clear bit at master 8259
        popf
        ret
enable_8:
        in      al, INTERUPT_SLAVE_PORT1
        and     al, ah
        out     INTERUPT_SLAVE_PORT1, al       ; clear bit at slave 8259
        popf
        ret


; ========================================================================
;  void DisableInterupt(void);
; ========================================================================
DisableInterupt:
	cli
	ret


; ========================================================================
;  void EnableInterupt(void);
; ========================================================================
EnableInterupt:
	sti
	ret
