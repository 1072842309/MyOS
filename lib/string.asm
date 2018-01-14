;操作内存的函数

[SECTION .text]

;导出函数
global MemCpy
global MemSet
global StrCpy
global StrLen


; ------------------------------------------------------------------------
; void* MemCpy(void* es:dst,void* ds:src,int size);
; 将src起的size个字节拷贝至dst处
; P.S.es,ds皆指向0-4GB段,亦即皆为0,不影响函数调用
; ------------------------------------------------------------------------
MemCpy:
	;备份ebp
	push	ebp
	mov	ebp, esp
	;备份寄存器
	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	;dst	
	mov	esi, [ebp + 12]	;src
	mov	ecx, [ebp + 16]	;size
	
	rep	movsb		;根据ecx逐字节拷贝src至dst 

	mov	eax, [ebp + 8]	;返回目标地址dst

	;复原寄存器
	pop	ecx
	pop	edi
	pop	esi
	;复原ebp
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; MemCpy 结束
; ------------------------------------------------------------------------

; ------------------------------------------------------------------------
; void MemSet(void* p_dst, char ch, int size);
; ------------------------------------------------------------------------
MemSet:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	edx, [ebp + 12]	; Char to be putted
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	byte [edi], dl		; ┓
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; ------------------------------------------------------------------------


; ------------------------------------------------------------------------
; char* StrCpy(char* p_dst, char* p_src);
; ------------------------------------------------------------------------
StrCpy:
	push    ebp
	mov     ebp, esp

	mov     esi, [ebp + 12] ; Source
	mov     edi, [ebp + 8]  ; Destination

.1:
	mov     al, [esi]               ; ┓
	inc     esi                     ; ┃
					; ┣ 逐字节移动
	mov     byte [edi], al          ; ┃
	inc     edi                     ; ┛

	cmp     al, 0           ; 是否遇到 '\0'
	jnz     .1              ; 没遇到就继续循环，遇到就结束

	mov     eax, [ebp + 8]  ; 返回值

	pop     ebp
	ret                     ; 函数结束，返回
; StrCpy 结束-------------------------------------------------------------

; ------------------------------------------------------------------------
; int StrLen(char* p_str);
; ------------------------------------------------------------------------
StrLen:
        push    ebp
        mov     ebp, esp

        mov     eax, 0                  ; 字符串长度开始是 0
        mov     esi, [ebp + 8]          ; esi 指向首地址

.1:
        cmp     byte [esi], 0           ; 看 esi 指向的字符是否是 '\0'
        jz      .2                      ; 如果是 '\0'，程序结束
        inc     esi                     ; 如果不是 '\0'，esi 指向下一个字符
        inc     eax                     ;         并且，eax 自加一
        jmp     .1                      ; 如此循环

.2:
        pop     ebp
        ret                             ; 函数结束，返回
; ------------------------------------------------------------------------
