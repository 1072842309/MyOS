;------------------------------------------------------------------------------

	;FAT12磁盘的头
	jmp short LABEL_START
	nop
	
	;磁盘头
	BS_OEMName		db	'*PEACE**'	;厂商名,8字节
	BPB_BytsPerSec		dw	512		;每扇区字节数
	BPB_SecPerClus		db	1		;没簇扇区数
	BPB_RsvdSecCnt		dw	1		;boot记录占用扇区数(引导扇区)
	BPB_NumFATs		db	2		;FAT数
	BPB_RootEntCnt		dw	224		;根目录文件最大数
	BPB_TotSec16		dw	2880		;扇区总数
	BPB_Media		db	0xF0		;介质描述符
	BPB_FATSz16		dw	9		;每FAT扇区数
	BPB_SecPerTrk		dw	18		;每磁道扇区数
	BPB_NumHeads		dw	2		;磁头数
	BPB_HiddSec		dd	0		;隐藏扇区数
	BPB_TotSec32		dd	0
	BS_DrvNum		db	0		;中断13的驱动器号
	BS_Reserved1		db	0		;未使用
	BS_BootSig		db	29h		;扩展引导标记
	BS_VolID		dd	0		;卷序列号
	BS_VolLab		db	'*PEACE*LOVE'	;卷标,11字节
	BS_FileSysType		db	'FAT12   '	;文件系统类型,8字节


;------------------------------------------------------------------------------

; -------------------------------------------------------------------------
; 基于 FAT12 头的一些常量定义，如果头信息改变，下面的常量可能也要做相应改变
; -------------------------------------------------------------------------
; BPB_FATSz16
FATSz			equ	9

; 根目录占用空间:
RootDirSectorsSize	equ	14

; Root Directory 的第一个扇区号	= BPB_RsvdSecCnt + (BPB_NumFATs * FATSz)
SectorNumberOfRootDir	equ	19

; FAT1 的第一个扇区号
SectorNumberOfFAT1	equ	1

; 文件的开始Sector号
DeltaSectorNo		equ	17
