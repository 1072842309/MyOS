

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "keyboard.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"


#define TTY_FIRST (tty_table)
#define TTY_END (tty_table+NUMBER_CONSOLES)

//私有函数声明
//初始化TTY
PRIVATE void ITTY(TTY *p_tty);
//该TTY试图从键盘缓冲区读入一个字符
PRIVATE void TTYRead(TTY *p_tty);
//该TTY试图向所在控制台输出一个字符
PRIVATE void TTYWrite(TTY *p_tty);
//向TTY对应的缓冲区中压入一个字符
PRIVATE void PushWord(TTY *p_tty,u32 word);

//终端任务
PUBLIC void TaskTTY(void){
	//初始化键盘I/O
	IKeyboard();

	//初始化TTY
	for(TTY *p_tty = TTY_FIRST;p_tty < TTY_END;p_tty++){
		ITTY(p_tty);
	}

	//当前控制台设置为0号
	ChangeConsole(0);

	//轮询所有的tty
	while(1){

		for(TTY *p_tty = TTY_FIRST;p_tty < TTY_END;p_tty++){
			TTYRead(p_tty);
			TTYWrite(p_tty);
		}
	}

}



//再加工键盘传来的按键,随后传给指定的tty
PUBLIC void InputProcess(TTY *p_tty,u32 word){
	char output[2] = {0};

	//不打印控制字符(控制字符都内嵌了FLAG_EXT)
	if(!(word & FLAG_EXT)){

		//当指定tty缓冲区未满时,将该字符传至该缓冲区
		if(p_tty->input_buf_count < TTY_IN_BUF_SIZE){
			*(p_tty->p_input_buf_head) = word;
			p_tty->p_input_buf_head++;
			if(p_tty->p_input_buf_head >= p_tty->input_buf + TTY_IN_BUF_SIZE){
				p_tty->p_input_buf_head = p_tty->input_buf;
			}
			p_tty->input_buf_count++;

		}
	}

	else{
		//卸下修饰位
		int raw_word = word & MASK_RAW;
		switch(raw_word){
		case UP:	
			//SHIFT+上,滚动屏幕
			if((word & FLAG_SHIFT_L) || (word & FLAG_SHIFT_R)){
				ScrollScreen(p_tty->p_console,SCR_UP);
			}
			break;

		case DOWN:
			//SHIFT+下,滚动屏幕
			if((word & FLAG_SHIFT_L) || (word & FLAG_SHIFT_R)){
				ScrollScreen(p_tty->p_console,SCR_DOWN);
			}
			break;

		case ENTER:
			PushWord(p_tty,'\n');
			break;

		case BACKSPACE:
			PushWord(p_tty,'\b');
			break;

		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			//SHIFT+Fn,切换控制台
			if(word & FLAG_SHIFT_L){
				ChangeConsole(raw_word - F1);
			}
			break;
		
		default:
			break;
		}
	}
}

//向TTY对应的缓冲区中压入一个字符
PRIVATE void PushWord(TTY *p_tty,u32 word){
	//判断该tty的缓冲区是否有足够空间
	if(p_tty->input_buf_count < TTY_IN_BUF_SIZE){
		//取出该字符
		*(p_tty->p_input_buf_head) = word;
		p_tty->p_input_buf_head++;
		if(p_tty->p_input_buf_head >= p_tty->input_buf+TTY_IN_BUF_SIZE){
			p_tty->p_input_buf_head = p_tty->input_buf;
		}
		p_tty->input_buf_count++;
	}
}

//向指定tty中打印一个字符串
PUBLIC void tty_write(char *buf,int len,TTY *p_tty){
	while(len){
		OutChar(p_tty->p_console,*buf);
		buf++;
		len--;
	}
}	

//系统发生int中断时调用的函数
PUBLIC int syscall_write(char *buf,int len,PROCESS *p_proc){
	tty_write(buf,len,&tty_table[p_proc->number_tty]);
	return 0;
}

//初始化TTY
PRIVATE void ITTY(TTY *p_tty){
	//初始化对应的缓冲区
	p_tty->input_buf_count = 0;
	p_tty->p_input_buf_head = p_tty->p_input_buf_tail = p_tty->input_buf;

	//初始化对应的控制台
	IConsole(p_tty);

}


//该TTY试图从键盘缓冲区读入一个字符
PRIVATE void TTYRead(TTY *p_tty){
	//判断是否是正在使用的tty
	if(IsCurrentConsole(p_tty->p_console)){
		//以该tty的名义,从键盘接受一个字符
		KeyboardRead(p_tty);

	}
}

//该TTY试图向所在控制台输出一个字符
PRIVATE void TTYWrite(TTY *p_tty){
	//判断该tty的缓冲区是否有未打印字符
	if(p_tty->input_buf_count > 0){
		//取出该字符
		char ch = *(p_tty->p_input_buf_tail);
		p_tty->p_input_buf_tail++;
		if(p_tty->p_input_buf_tail >= p_tty->input_buf+TTY_IN_BUF_SIZE){
			p_tty->p_input_buf_tail = p_tty->input_buf;
		}
		p_tty->input_buf_count--;

		//将该字符打印
		OutChar(p_tty->p_console,ch);
	}

}
