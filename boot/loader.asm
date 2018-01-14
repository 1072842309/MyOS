org	0100h

	jmp	LABEL_START

;包含的文件
%include "load.inc"	;加载loader和内核需要的信息
%include "fat12.inc"	;软驱信息
%include "pm.inc"	;定义保护模式描述符需要用的宏定义


;GDT
LABEL_GDT:		Descriptor	0,	0,	0	
LABEL_DESC_FLAT_C:	Descriptor	0,	0fffffh,DA_CR|DA_32|DA_LIMIT_4K	
LABEL_DESC_FLAT_RW:	Descriptor	0,	0fffffh,DA_DRW|DA_32|DA_LIMIT_4K
LABEL_DESC_VIDEO:	Descriptor	0b8000h,0ffffh,	DA_DRW|DA_DPL3

GdtLen		equ	$-LABEL_GDT	;段长度
GdtPtr		dw	GdtLen-1	;段界限
		dd	BaseOfLoaderPhysicalAddress+LABEL_GDT	;段基址

;选择子
SelectorFlatC		equ	LABEL_DESC_FLAT_C 	-LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	-LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	-LABEL_GDT + SA_RPL3

;宏声明
BaseOfStack		equ	0100h	;栈顶指针设在ip之前

;PageDirectoryBase		equ	100000h	;页目录开始地址
;PageTableBase		equ	101000h	;页表开始地址


LABEL_START:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,BaseOfStack

	mov dh,0
	call DisplayStrReal


	;得到内存容量
	mov di,_MemChkBuf
	mov ebx,0
.loop_check_mem:
	mov eax,0E820h	;指定int 15将会调用的
	mov ecx,20	;将在缓冲区填充的字节数
	mov edx,0534D4150h;	;校验信息
	int 15h
	jc .LABEL_CHECK_MEM_FAIL	;若CF非0,说明发生错误
	add di,20
	inc dword [_dwMCRNumber]	
	cmp ebx,0
	jne .loop_check_mem	;若ebx非0,说明仍有后续待读内容
	jmp .LABEL_CHECK_MEM_OK
.LABEL_CHECK_MEM_FAIL:
	mov dword[_dwMCRNumber],0
.LABEL_CHECK_MEM_OK:
	

	;仿照boot.asm中的方法,在根目录中寻找kernal.bin
	;软驱复位
	xor ah,ah
	xor dl,dl
	int 13h
	mov word [wordSectorNo],SectorNumberOfRootDir		;从首个根目录扇区号开始查找
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp word [wordRootDirSizeForLoop],0		;判断根目录是否已读完
	jz LABEL_NOT_FOUND				;若已读完,说明未找到KERNAL.bin
	dec word [wordRootDirSizeForLoop]
	;开始对新扇区的检查
	mov ax,BaseOfKernel
	mov es,ax
	mov bx,OffOfKernel				;es:bx指向读扇区时写入数据用的缓冲区
	mov ax,[wordSectorNo]				;得到新的即将检查的扇区的序号
	mov cl,1					;设置检查扇区数为1
	call ReadSector					;将当前扇区的数据写入缓冲区BaseOfKernel:OffOfKernel

	mov si,KernelFileName				;ds:si指向目标字符串
	mov di,OffOfKernel				;es:di指向缓冲区中即将被检查的字符串
	cld						;设置方向标志DF为正向增加
	mov dx,10h					;每个扇区中的最大文件数
							;16(每个扇区中的最大文件数)*32(每个文件在根目录区中所占空间大小)=512(一个扇区的大小)
	
	;开始检查写入缓冲区的16个文件
LABEL_SEARCH_FOR_LOEADER:
	cmp dx,0
	jz LABEL_GOTO_NEXT_SECTOR			;如果当前扇区的16个文件皆不匹配,去往下一个扇区
	dec dx
	mov cx,11					;对当前文件的11个字符进行检查

LABEL_CMP_FILENAME:
	cmp cx,0
	jz LABEL_FILENAME_FOUND				;如果11个字符全都匹配,说明找到了KERNAL.bin
	dec cx
	lodsb						;ds:si的字符写入al,si自动加一
	cmp al,byte [es:di]				;将al与待查字符串中的相应字符进行比较
	jz LABEL_GO_ON					;若相同
	jmp LABEL_DIFFERENT				;若不同

LABEL_GO_ON:
	inc di						;手动增加di,指向待查字符串中的下一个字符
	jmp LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and di,0FFE0h					;复原di,由于一个文件名最大11字节,因此最多涉及4位,仅需将前四位归位
	add di,20h					;准备检查根目录文件中的下一个文件
	mov si,KernelFileName				;si重置为指向目标字符串首
	jmp LABEL_SEARCH_FOR_LOEADER

LABEL_GOTO_NEXT_SECTOR:					;准备去往下一个扇区检查
	add word [wordSectorNo],1				;增加当前检查扇区号的序数
	jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NOT_FOUND:					;所有扇区中皆未找到
	mov dh,2					;打印函数的标识符设为2
	call DisplayStrReal 					;打印错误信息
%ifdef	_BOOT_DEBUG_
	mov ax,4c00h
	int 21h						;若处在调试模式,返回DOS
%else	
	jmp $						;否则,无限循环
%endif

LABEL_FILENAME_FOUND:					;找到了目标字符串
	and di,0FFE0h					;复原di,由于一个文件名最大11字节,因此最多涉及4位,仅需将前四位归位
	add di,1Ah					;es:di指向KERNAL.bin的首个FAT项(保存在32字节的根目录文件中
	;保存此FAT项
	mov cx,word [es:di]
	push cx
	;计算首个扇区号
	mov ax,RootDirSectorsSize
	add cx,ax
	add cx,DeltaSectorNo
	;
	mov ax,BaseOfKernel
	mov es,ax
	mov bx,OffOfKernel				;es:bx指向缓冲区
	mov ax,cx					;ax保存首个扇区号

LABEL_LOADING_FILE:
	;为了增加可读性,在读扇区时打'.'
	;备份
	push ax
	push bx	
	;打'.'
	mov ah,0Eh
	mov al,'.'
	mov bl,0Fh
	int 10h
	;还原	
	pop bx
	pop ax

	;将从指定扇区开始的一个扇区内的内容写入缓冲区
	mov cl,1
	call ReadSector
	;根据当前的FAT项,判断是否存在后继项
	pop ax						;取出之前保存的FAT项
	call GotFATEntry
	cmp ax,0FFFh					;当无后继项时,将表示为0xFFF
	jz LABEL_LOADED					;若无后继项,表明所有内容皆已写入缓冲区
	;若有后继项,则根据后继项计算新扇区的位置
	push ax						;保存此FAT项
	;计算新扇区号
	add ax,RootDirSectorsSize
	add ax,DeltaSectorNo
	;更新缓冲区写入位置
	add bx,[BPB_BytsPerSec]
	jmp LABEL_LOADING_FILE

LABEL_LOADED:

	;call KillMotor		;关闭软驱马达

	mov dh,1
	call DisplayStrReal

	;准备进入保护模式
	;加载GDTR
	lgdt [GdtPtr]
	;关中断
	cli
	;打开A20
	in al,92h
	or al,0000_0010b
	out 92h,al
	;打开cr0开关
	mov eax,cr0
	or eax,1
	mov cr0,eax

	;跳转至保护模式代码段
	jmp dword SelectorFlatC:(BaseOfLoaderPhysicalAddress+LABEL_PM_START)

	jmp $
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
;变量定义1
;------------------------------------------------------------------------------
wordRootDirSizeForLoop	dw	RootDirSectorsSize	;仍然在等待被读取的文件的数量
wordSectorNo		dw	0		;当前读取的扇区号
byteOdd			db	0		;ReadSector使用的变量

;------------------------------------------------------------------------------
;字符串
;------------------------------------------------------------------------------
KernelFileName		db	"KERNEL  BIN", 0	; KERNEL.BIN 之文件名
; 为简化代码, 下面每个字符串的长度均为 MsgLength
MsgLength		equ	9
LoadMessage:		db	"Loading  "
Message1		db	"Ready.   "
Message2		db	"No KERNEL"

_szMemChkTitle:	db "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:	db "RAM size:", 0
_szReturn:	db 0Ah, 0

;------------------------------------------------------------------------------
;变量定义2
;由于将被保护模式借用,因此单独放出来
;------------------------------------------------------------------------------
_dwMCRNumber:	dd 0			; 保存可供使用的内存需要的页表数,向上取整
_doublewordDispPos:	dd (80 * 6 + 0) * 2	; 屏幕第 6 行, 第 0 列
_dwMemSize:	dd 0			; 可供使用的内存总和
_ARDStruct:	; Address Range Descriptor Structure
  _dwBaseAddrLow:		dd	0
  _dwBaseAddrHigh:		dd	0
  _dwLengthLow:			dd	0
  _dwLengthHigh:		dd	0
  _dwType:			dd	0
_MemChkBuf:	times	256	db	0
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
;函数DisplayStrReal
;------------------------------------------------------------------------------
;根据标识符dh,打印一个字符串
DisplayStrReal:
	mov ax,MsgLength
	mul dh
	add ax,LoadMessage
	mov bp,ax					;es:bp指向目标字符串
	mov ax,ds
	mov es,ax
	mov cx,MsgLength				;cx为字符串长度
	mov ax,1301h
	mov bx,0007h
	mov dl,0
	add dh,3
	int 10h						
	ret
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
;函数ReadSector
;从第ax个扇区开始,将cl个扇区写入到es:bx中
;------------------------------------------------------------------------------
ReadSector:
	push bp
	mov bp,sp
	sub esp,2					;辟出两个字节的保留空间

	mov byte [bp-2],cl				;将扇区数暂存在保留空间
	push bx						;备份bx
	mov bl,[BPB_SecPerTrk]				;每磁道扇区数作为除数
	div bl
	;起始扇区号
	inc ah						;余数+1为起始扇区号
	mov cl,ah					;cl保存起始扇区号
	;磁头号
	mov dh,al
	shr al,1
	mov ch,al
	and dh,1
	;and dh,1					
	;磁道号
	;mov ch,al
	;mov c
	;shr ch,1
	;还原bx
	pop bx
	;驱动器号设为0(A盘)
	mov dl,[BS_DrvNum]
	
	;系统调用,读扇区
RetryReading:
	;将读扇区数
	mov al,byte [bp-2]
	;
	mov ah,2
	int 13h
	jc RetryReading

	add esp,2
	pop bp

	ret
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
;函数GotFATEntry
;读取序号为ax的扇区的FAT项值,用ax返回
;------------------------------------------------------------------------------
GotFATEntry:
	;备份
	push es
	push bx
	push ax
	;为避免影响到缓冲区内的指令,在缓冲区之前辟出一片区域存放FAT
	mov ax,BaseOfKernel
	sub ax,0100h
	mov es,ax
	;还原ax
	pop ax
	mov byte [byteOdd],0
	mov bx,3
	mul bx
	mov bx,2
	div bx
	cmp dx,0
	jz LABEL_EVEN
	mov byte [byteOdd],1
LABEL_EVEN:
	;ax保存当前FAT项在FAT中的偏移量
	xor dx,dx
	mov bx,[BPB_BytsPerSec]
	div bx							;ax保存当前FAT项所在扇区对应FAT首扇区的扇区号
								;dx保存当前FAT项在所属扇区中的偏移量
	push dx							;备份dx
	mov bx,0
	add ax,SectorNumberOfFAT1					;ax加上FAT首扇区的扇区号,等于当前FAT项所在的扇区
	mov cl,2
	call ReadSector						;从ax指定的扇区开始,读取两个扇区的内容至缓冲区内

	pop dx							;还原dx
	add bx,dx						;bx指向当前FAT项的第一个字节处
	mov ax,[es:bx]						;将从bx开始的一个字写入ax
	cmp byte [byteOdd],1
	jnz LABEL_EVEN_2
	shr ax,4
LABEL_EVEN_2:
	and ax,0FFFh

	;得到了FAT项值,可以返回了
	pop bx
	pop es
	ret
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
;以下的代码已进入保护模式
[section .s32]
ALIGN 32
[bits 32]
LABEL_PM_START:
	;初始化段寄存器
	mov ax,SelectorVideo
	mov gs,ax

	mov ax,SelectorFlatRW
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov ss,ax
	mov esp,TopOfStack

	;打印内存信息
	call DispMemInfo
	;开启分页
	call SetupPaging

	mov ah,0Fh
	mov al,'Z'
	mov [gs:((5*80+39)*2)],ax	

	call IKernel
	
	;进入内核
	jmp SelectorFlatC:KernelEntryPhysicalAddress

	jmp $
;------------------------------------------------------------------------------

;数据段
[section .data1]
ALIGN 32
LABEL_DATA:
;以16位的2号变量组为基础
szMemChkTitle		equ	BaseOfLoaderPhysicalAddress + _szMemChkTitle
szRAMSize		equ	BaseOfLoaderPhysicalAddress + _szRAMSize
szReturn		equ	BaseOfLoaderPhysicalAddress + _szReturn
doublewordDispPos		equ	BaseOfLoaderPhysicalAddress + _doublewordDispPos
dwMemSize		equ	BaseOfLoaderPhysicalAddress + _dwMemSize
dwMCRNumber		equ	BaseOfLoaderPhysicalAddress + _dwMCRNumber
ARDStruct		equ	BaseOfLoaderPhysicalAddress + _ARDStruct
	dwBaseAddrLow	equ	BaseOfLoaderPhysicalAddress + _dwBaseAddrLow
	dwBaseAddrHigh	equ	BaseOfLoaderPhysicalAddress + _dwBaseAddrHigh
	dwLengthLow	equ	BaseOfLoaderPhysicalAddress + _dwLengthLow
	dwLengthHigh	equ	BaseOfLoaderPhysicalAddress + _dwLengthHigh
	dwType		equ	BaseOfLoaderPhysicalAddress + _dwType
MemChkBuf		equ	BaseOfLoaderPhysicalAddress + _MemChkBuf
;在数据段末尾定义堆栈
times	1024	db	0
TopOfStack	equ	BaseOfLoaderPhysicalAddress + $ 	;栈顶指针

;外援:已封装的打印函数
%include "lib.inc"


;------------------------------------------------------------------------------
; 将内核导入内存
;------------------------------------------------------------------------------
IKernel:
	;备份寄存器
	push eax
	push ecx
	push esi

	mov cx,[BaseOfKernelPhyAddr+2ch]	
	movzx ecx,cx				;Program Header的数量	
	mov esi,[BaseOfKernelPhyAddr+1ch]	
	add esi,BaseOfKernelPhyAddr		;esi指向首个Program Header的物理地址
NEXT_PH:
	cmp dword [esi+0x00],0	;判断当前PH是否有效
	je CUR_PH_END		;若不存在,则跳过当前PH
	;将三个参数压栈
	push dword [esi+10h]	;压栈当前PH管理的字节数
	mov eax,[esi+04h]	
	add eax,BaseOfKernelPhyAddr	
	push eax		;压栈当前PH的源地址
	push dword [esi+08h]	;压栈当前PH的目标地址
	;调用MemCpy
	call MemCpy
	add esp,12		;自行处理栈顶指针
CUR_PH_END:	
	add esi,20h		;指向下一个PH的物理地址
	loop NEXT_PH

	;还原寄存器
	pop esi
	pop ecx
	pop eax

	ret
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; 显示内存信息
;------------------------------------------------------------------------------
DispMemInfo:
	;备份寄存器
	push	esi
	push	edi
	push	ecx

	mov	esi, MemChkBuf			;检测内存时得到的信息
	mov	ecx, [dwMCRNumber]		;for(int i=0;i<[MCRNumber];i++)//每次得到一个ARDS
.loop:				  		;{
	;将当前正在检测内存的信息依次打印,同时存入ARDStruct
	mov	edx, 5		  		;  for(int j=0;j<5;j++)//每次得到一个ARDS中的成员
	mov	edi, ARDStruct	  		;  {//依次显示:BaseAddrLow,BaseAddrHigh,LengthLow,LengthHigh,Type
.1:
	;打印当前获得的信息
	push	dword [esi]	  		;
	call	DispInt		  		;    DispInt(MemChkBuf[j*4]); // 显示一个成员
	;将当前信息存入ARDStruct
	pop	eax		  		;
	stosd			  		;    ARDStruct[j*4] = MemChkBuf[j*4];
	;五元素中的当前信息已处理完毕,准备处理下一个
	add	esi, 4		  		;
	dec	edx		  		;
	cmp	edx, 0		  		;
	jnz	.1		  		;  }
	;当前检测内存的五元素全部处理完毕,
	;检测它是否可被使用
	call	DispReturn	  		;  printf("\n");
	cmp	dword [dwType], 1 		;  if(Type == AddressRangeMemory) //当前检测的内存是否可被使用
	jne	.2		  		;  {
	mov	eax, [dwBaseAddrLow]		;
	add	eax, [dwLengthLow]		;
	cmp	eax, [dwMemSize]  		;    if(BaseAddrLow + LengthLow > MemSize)
	jb	.2		  		;
	mov	[dwMemSize], eax  		;    MemSize = BaseAddrLow + LengthLow;
.2:				  		;  }
	loop	.loop		  		;}
	;内存信息处理完毕,汇总并打印可使用内存
	call	DispReturn	  		
	push	szRAMSize	  		
	;打印提示信息"RAM Size:"	  		
	call	DisplayStr			
	add	esp, 4		  		
	;打印可使用内存总量			  		
	push	dword [dwMemSize] 		
	call	DispInt		  		
	add	esp, 4		  		
	;复原寄存器
	pop	ecx
	pop	edi
	pop	esi
	ret
; ---------------------------------------------------------------------------

;------------------------------------------------------------------------------
; 启动分页机制
; 为简化处理, 所有线性地址对应相等的物理地址. 并且不考虑内存空洞.
; 实际应用中,页表由于经常删减,其排列是不连续的
;------------------------------------------------------------------------------
SetupPaging:
	;备份寄存器
	push eax
	push ebx
	push ecx
	push edi

	;获得所需页表数量
	mov ecx,[dwMCRNumber]

	;首先初始化页目录
	push ecx		;保留,初始化页表时复用
	mov ax,SelectorFlatRW
	mov es,ax

	mov edi,PageDirectoryBase
	mov eax,PageTableBase|PG_P|PG_USU|PG_RWW	;页目录中的页表项值
NEXT_DIR_ENTRY:
	stosd
	add eax,0x1000		;1024(每张页表中的物理页个数)*4(页表中每项所占字节数) = 4K(一个页表所占空间字节数)
	loop NEXT_DIR_ENTRY

	;初始化页表
	pop eax
	mov ebx,1024		;每张页表含有1024个物理页
	mul ebx
	mov ecx,eax
	mov edi,PageTableBase
	mov eax,PG_P|PG_USU|PG_RWW
NEXT_TBL_ENTRY:
	stosd
	add eax,0x1000
	loop NEXT_TBL_ENTRY
	
	;设置cr3指向页目录首地址
	mov eax,PageDirectoryBase
	mov cr3,eax

	;调整cr0寄存器,正式开启分页机制
	mov eax,cr0
	or eax,80000000h
	mov cr0,eax

	;还原寄存器
	pop edi
	pop ecx
	pop ebx
	pop eax

	ret
; ------------------------------------------------------------------------
