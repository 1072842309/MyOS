#使用工具选项(工具名+参数)
LD 			= ld
LDFLAGS			= -melf_i386 -Ttext $(KERNELENTRY) 
CC			= gcc
CFLAGS			= -m32 -c -I include/ -fno-stack-protector
ASM			= nasm
ASMKFLAGS		= -f elf -I include/
ASMBFLAGS		= -I boot/inc/

#BOOT
BOOT			= boot.bin

#LOADER
LOADER			= loader.bin

#KERNEL
OBJ			= kernel/kernel.o kernel/start.o lib/kernel_lib_asm.o lib/kernel_lib_c.o lib/string.o kernel/global.o kernel/interupt_8259.o kernel/protect.o kernel/main.o kernel/clock.o kernel/proc.o kernel/syscall.o kernel/keyboard.o kernel/tty.o kernel/keymap.o kernel/console.o kernel/printf.o 
KERNELFILE 		= kernel.bin

#加载kernel.bin时的入口地址
KERNELENTRY		= 0x30400


#可执行行为列表
.PHONY:	image clean_all


#更新a.img
image:			$(BOOT) $(LOADER) $(KERNELFILE)
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount a.img floppy
	sudo cp $(BOOT) $(LOADER) $(KERNELFILE) floppy
	sudo umount floppy

#删除除源码外的一切文件
clean_all:		
	rm $(BOOT) $(LOADER) $(KERNELFILE) $(OBJ)

#生成loader.bin
$(LOADER):		boot/loader.asm boot/inc/load.inc boot/inc/fat12.inc boot/inc/pm.inc
	$(ASM) $(ASMBFLAGS) $< -o $@

#生成boot.bin
$(BOOT):		boot/boot.asm boot/inc/fat12.inc
	$(ASM) $(ASMBFLAGS) $< -o $@

#清理生成kernel.bin时的.o文件
#clean:
#	rm -f $(OBJ)

#生成kernel.bin
$(KERNELFILE):		$(OBJ) 
	$(LD) $(LDFLAGS) $(OBJ) -o $@

# kernel/
kernel/kernel.o:	kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) $< -o $@

kernel/start.o:		kernel/start.c include/const.h include/type.h include/string.h include/prototype.h include/protect.h include/proc.h include/global.h
	$(CC) $(CFLAGS) $< -o $@

kernel/global.o:	kernel/global.c include/type.h  include/const.h include/protect.h include/prototype.h include/proc.h include/global.h	
	$(CC) $(CFLAGS) $< -o $@

kernel/interupt_8259.o:	kernel/interupt_8259.c include/type.h include/const.h include/protect.h include/prototype.h
	$(CC) $(CFLAGS) $< -o $@

kernel/protect.o:	kernel/protect.c include/type.h include/const.h include/protect.h include/prototype.h include/proc.h include/string.h include/global.h
	$(CC) $(CFLAGS) $< -o $@

kernel/main.o:		kernel/main.c include/type.h include/const.h include/string.h include/prototype.h include/protect.h include/proc.h include/global.h
	$(CC) $(CFLAGS) $< -o $@

kernel/clock.o:		kernel/clock.c include/const.h include/string.h
	$(CC) $(CFLAGS) $< -o $@

kernel/syscall.o:	kernel/syscall.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) $< -o $@  

kernel/proc.o:		kernel/proc.c include/type.h include/const.h include/prototype.h 
	$(CC) $(CFLAGS) $< -o $@

kernel/kerboard.o:	kernel/keyboard.c include/type.h include/const.h include/prototype.h include/keyboard.h
	$(CC) $(CFLAGS) $< -o $@

kernel/tty.o:		kernel/tty.c include/type.h include/const.h include/protect.h include/prototype.h include/proc.h include/keyboard.h include/console.h include/tty.h include/global.h
	$(CC) $(CFLAGS) $< -o $@

kernel/keymap.o:	kernel/keymap.c include/type.h include/keyboard.h
	$(CC) $(CFLAGS) $< -o $@

kernel/console.o:	kernel/console.c include/type.h include/const.h include/console.h include/tty.h
	$(CC) $(CFLAGS) $< -o $@

kernel/printf.o:	kernel/printf.c include/type.h

# lib/
lib/kernel_lib_asm.o:	lib/kernel_lib_asm.asm
	$(ASM) $(ASMKFLAGS) $< -o $@  

lib/kernel_lib_c.o:	lib/kernel_lib_c.c include/type.h include/const.h include/protect.h include/prototype.h include/string.h include/proc.h include/global.h
	$(CC) $(CFLAGS) $< -o $@  

lib/string.o:		lib/string.asm
	$(ASM) $(ASMKFLAGS) $< -o $@  

