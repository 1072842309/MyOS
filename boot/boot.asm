;%define 	_BOOT_FOR_DEBUG_	;调试用

%ifdef	_BOOT_FOR_DEBUG_
	org	0100h
%else	
	org	7c00h
%endif


;------------------------------------------------------------------------------
;常量与宏定义
%ifdef _BOOT_FOR_DEBUG_
BaseOfStack		equ	0100h			;调试时的栈基地址
%else
BaseOfStack		equ	7c00h			;栈基地址
%endif

BaseOfLoader		equ	9000h			;读扇区函数的缓冲区,段地址
OffOfLoader		equ	0100h			;读扇区函数的缓冲区,偏移地址
;常量与宏定义结束

%include "fat12.inc"



;扫描扇区时的变量定义
wordRootDirSizeForLoop		dw	RootDirSectorsSize	;等待检查的扇区数
wordSectorNo			dw	0		;正在检查的扇区序号
byteOdd				db	0		;奇数or偶数?

LoaderFileName			db	"LOADER  BIN",0	;目标字符串
;DisplayStr会用到的字符串
MsgLength			equ	9		;单个字符串长度皆为9
BootMsg:			db	"Booting  "	
Msg1:				db	"Ready.   "
Msg2:				db	"No Loader"
;变量定义结束
;------------------------------------------------------------------------------


LABEL_START:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,BaseOfStack

	;清屏
	mov ax,0600h
	mov bx,0700h
	mov cx,0
	mov dx,184Fh
	int 10h

	;显示字符串
	mov dh,0
	call DisplayStr

	;软驱复位
	xor ah,ah
	xor dl,dl
	int 13h

	;在根目录中寻找LOADER.bin
	mov word [wordSectorNo],SectorNumberOfRootDir		;从首个根目录扇区号开始查找
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp word [wordRootDirSizeForLoop],0		;判断根目录是否已读完
	jz LABEL_NOT_FOUND				;若已读完,说明未找到LOADER.bin
	dec word [wordRootDirSizeForLoop]
	;开始对新扇区的检查
	mov ax,BaseOfLoader
	mov es,ax
	mov bx,OffOfLoader				;es:bx指向读扇区时写入数据用的缓冲区
	mov ax,[wordSectorNo]				;得到新的即将检查的扇区的序号
	mov cl,1					;设置检查扇区数为1
	call ReadSector					;将当前扇区的数据写入缓冲区BaseOfLoader:OffOfLoader

	mov si,LoaderFileName				;ds:si指向目标字符串
	mov di,OffOfLoader				;es:di指向缓冲区中即将被检查的字符串
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
	jz LABEL_FILENAME_FOUND				;如果11个字符全都匹配,说明找到了LOADER.bin
	dec cx
	lodsb						;ds:si的字符写入al,si自动加一
	cmp al,byte [es:di]				;将al与待查字符串中的相应字符进行比较
	jz LABEL_GO_ON					;若相同
	jmp LABEL_DIFFERENT				;若不同

LABEL_GO_ON:
	inc di						;手动增加di,指向待查字符串中的下一个字符
	jmp LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and di,0FFE0h					;,复原di,由于一个文件名最大11字节,因此最多涉及4位,仅需将前四位归位
	add di,20h					;准备检查根目录文件中的下一个文件
	mov si,LoaderFileName				;si重置为指向目标字符串首
	jmp LABEL_SEARCH_FOR_LOEADER

LABEL_GOTO_NEXT_SECTOR:					;准备去往下一个扇区检查
	add word [wordSectorNo],1				;增加当前检查扇区号的序数
	jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NOT_FOUND:					;所有扇区中皆未找到
	mov dh,2					;打印函数的标识符设为2
	call DisplayStr 					;打印错误信息
%ifdef	_BOOT_FOR_DEBUG_
	mov ax,4c00h
	int 21h						;若处在调试模式,返回DOS
%else	
	jmp $						;否则,无限循环
%endif

LABEL_FILENAME_FOUND:					;找到了目标字符串
	and di,0FFE0h					;复原di,由于一个文件名最大11字节,因此最多涉及4位,仅需将前四位归位
	add di,1Ah					;es:di指向loader.bin的首个FAT项(保存在32字节的根目录文件中
	;保存此FAT项
	mov cx,word [es:di]
	push cx
	;计算首个扇区号
	mov ax,RootDirSectorsSize
	add cx,ax
	add cx,DeltaSectorNo
	;
	mov ax,BaseOfLoader
	mov es,ax
	mov bx,OffOfLoader				;es:bx指向缓冲区
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
	mov dh,1
	call DisplayStr
	;loader.bin已完全加载至缓冲区,可以执行缓冲区的指令了!
	jmp BaseOfLoader:OffOfLoader
	









;------------------------------------------------------------------------------
;函数
;
;
;------------------------------------------------------------------------------


;函数DisPStr
;根据标识符dh,打印一个字符串
DisplayStr:
	mov ax,MsgLength
	mul dh
	add ax,BootMsg
	mov bp,ax					;es:bp指向目标字符串
	mov ax,ds
	mov es,ax
	mov cx,MsgLength				;cx为字符串长度
	mov ax,1301h
	mov bx,0007h
	mov dl,0
	int 10h						
	ret



;函数ReadSector
;从第ax个扇区开始,将cl个扇区写入到es:bx中
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


;函数GotFATEntry
;读取序号为ax的扇区的FAT项值,用ax返回
GotFATEntry:
	;备份
	push es
	push bx
	push ax
	;为避免影响到缓冲区内的指令,在缓冲区之前辟出一片区域存放FAT
	mov ax,BaseOfLoader
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

times 510-($-$$)		db 	0	;填充空间
dw	0xaa55
