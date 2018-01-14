#include "type.h"
#include "const.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


//重置屏幕打印的开始位置
PRIVATE void SetDisplayAddress(u32 addr);
//刷新屏幕
PRIVATE void Flush(CONSOLE *p_con);


/*======================================================================*
  为指定tty装配控制台
 *======================================================================*/
PUBLIC void IConsole(TTY *p_tty){
	int number_tty = p_tty - tty_table;	//
	p_tty->p_console = console_table + number_tty;

	int total_video_memory_size = VIDEO_MEMORY_SIZE/2;	//以字符为单位
	int single_console_video_memory_size = total_video_memory_size/NUMBER_CONSOLES;	//单个控制台分配的总字符

	//控制台在显存中的位置
	p_tty->p_console->original_address = single_console_video_memory_size * number_tty;
	//控制台在显存中分配的字符大小
	p_tty->p_console->video_memory_limit = single_console_video_memory_size;
	//当前显示位置
	p_tty->p_console->display_address = p_tty->p_console->original_address;

	//光标所指位置
	if(number_tty == 0){
		//如果是首个控制台,则光标保持指向原位置
		p_tty->p_console->cursor = disp_pos/2;
	}
	else{
		//非首控制台,则将光标指向显存头部
		p_tty->p_console->cursor = p_tty->p_console->original_address;
		OutChar(p_tty->p_console,number_tty+'0');
		OutChar(p_tty->p_console,'#');
	}
	//重置光标
	SetCursor(p_tty->p_console->cursor);
}


/*======================================================================*
  滚动屏幕
 *======================================================================*/
PUBLIC void ScrollScreen(CONSOLE *p_con,int dir){

	//向上滚动
	if(dir == SCR_UP){
		//如果未达到顶端
		if(p_con->display_address > p_con->original_address){
			p_con->display_address -= SCREEN_WIDTH;
		}
	}
	//向下滚动
	else if(dir == SCR_DOWN){
		//如果未达到底端
		if(p_con->display_address+SCREEN_WIDTH < p_con->original_address+p_con->video_memory_limit){
			p_con->display_address += SCREEN_WIDTH;
		}
	}
	//重置屏幕打印
	SetDisplayAddress(p_con->display_address);
	//重置光标
	SetCursor(p_con->cursor);
}


/*======================================================================*
  更换正在使用的控制台
 *======================================================================*/
PUBLIC void ChangeConsole(int new_console_index){
	//如果控制台索引非法
	if(new_console_index < 0 || new_console_index >= NUMBER_CONSOLES){
		return;
	}
	number_crt_console = new_console_index;
	SetCursor(console_table[new_console_index].cursor);
	SetDisplayAddress(console_table[new_console_index].display_address);
}


/*======================================================================*
  判断是否正在显示该控制台
 *======================================================================*/
PUBLIC int IsCurrentConsole(CONSOLE *p_console){
	return p_console - console_table == number_crt_console;
}



/*======================================================================*
  向指定控制台打印一个字符
 *======================================================================*/
PUBLIC void OutChar(CONSOLE *p_con,char ch){
	//打印的位置
	u8 *p_video_memory = (u8*)(VIDEO_MEM_BASE + p_con->cursor*2);
	//
	switch(ch){
	//Enter
	case '\n':
		if(p_con->cursor + SCREEN_WIDTH < p_con->original_address + p_con->video_memory_limit){
			p_con->cursor = (p_con->original_address + SCREEN_WIDTH*(((p_con->cursor-p_con->original_address)/SCREEN_WIDTH) + 1) );
		}
		break;
	
	//Backspace
	case '\b':
		if(p_con->cursor > p_con->original_address){
			*(p_video_memory - 2) = ' ';
			*(p_video_memory - 1) = DEFAULT_COLOR;
			p_con->cursor--;
		}
		break;

	default:
		//打印
		*p_video_memory++ = ch;
		*p_video_memory++ = DEFAULT_COLOR;
		//更新光标位置
		p_con->cursor++;
		break;
	}
	//如果当前光标向上移动时离开了屏幕,则自动向上滚动屏幕
	while(p_con->cursor < p_con->display_address){
		ScrollScreen(p_con,SCR_UP);
	}

	//如果当前光标向下移动时离开了屏幕,则自动向下滚动屏幕
	while(p_con->cursor >= p_con->display_address + SCREEN_SIZE){
		ScrollScreen(p_con,SCR_DOWN);
	}

	Flush(p_con);	//刷新屏幕
}


/*======================================================================*
  重置光标
 *======================================================================*/
PUBLIC void SetCursor(unsigned int pos){
	DisableInterupt();
	OutputByte(CRT_ADDR_REG,CURSOR_H);
	OutputByte(CRT_DATA_REG,(pos>>8)&0xFF);
	OutputByte(CRT_ADDR_REG,CURSOR_L);
	OutputByte(CRT_DATA_REG,pos&0xFF);
	EnableInterupt();
}


/*======================================================================*
//刷新屏幕
 *======================================================================*/
PRIVATE void Flush(CONSOLE *p_con){
	//重置光标
	SetCursor(p_con->cursor);
	//重置屏幕打印基址
	SetDisplayAddress(p_con->display_address);
}



/*======================================================================*
  重置屏幕打印的开始位置
 *======================================================================*/
PRIVATE void SetDisplayAddress(u32 addr){
	DisableInterupt();
	OutputByte(CRT_ADDR_REG,START_ADDR_H);
	OutputByte(CRT_DATA_REG,(addr>>8)&0xFF);
	OutputByte(CRT_ADDR_REG,START_ADDR_L);
	OutputByte(CRT_DATA_REG,addr&0xFF);
	EnableInterupt();
}

