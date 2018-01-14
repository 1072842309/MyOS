//处理与键盘I/O相关的内核函数

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "keyboard.h"
#include "console.h"
#include "tty.h"
#include "prototype.h"
#include "global.h"

//从缓冲区读取一个字节
PRIVATE char GetByteFromBuffer(void);

PRIVATE KEYBOARD_INPUT_BUF keyboard_input_buf;


//下列按键需与其他字符联动发挥作用,
//当按下(按住)他们时,将对应全局变量置1,后续字节会尝试与该字节联动
//当松开这些按键,硬件传递一个BreakCode,程序会自动将对应全局变量置0
PRIVATE int shift_l;		/* l shift state */
PRIVATE int shift_r;		/* r shift state */
PRIVATE int alt_l;		/* l alt state	 */
PRIVATE int alt_r;		/* r left state	 */
PRIVATE int ctrl_l;		/* l ctrl state	 */
PRIVATE int ctrl_r;		/* l ctrl state	 */
PRIVATE int caps_lock;		/* Caps Lock	 */
PRIVATE int num_lock;		/* Num Lock	 */
PRIVATE int scroll_lock;	/* Scroll Lock	 */

//部分按键由两字节(0xE0,0xXX),若接受到一个0xE0,则将该全局变量置1
PRIVATE int code_with_E0 = 0;

//初始化8259键盘中断
PUBLIC void IKeyboard(void){
	//初始化缓冲区
	keyboard_input_buf.p_head = keyboard_input_buf.p_tail = keyboard_input_buf.buf;
	keyboard_input_buf.count = 0;

	//设定键盘的8259中断反馈
	PutIrqHandler(KEYBOARD_IRQ,KeyboardHandler);
	EnableIrq(KEYBOARD_IRQ);
}

//从键盘输入缓冲区中读出一个字节并打印
PUBLIC void KeyboardRead(TTY *p_tty){
	
	char output[2] = {0};
	int make;	//new_code是否为MakeCode?
	code_with_E0 = 0;

	u32 word = 0;	//保存当前按键对应的字符

	//从缓冲区中读取一个字节
	u8 new_code = GetByteFromBuffer();


	//开始解析字符
	//仅PAUSE按键由0xE1开始
	if(new_code == 0xE1){
		u8 pause_code[] = {0xE1, 0x1D, 0x45,
				       0xE1, 0x9D, 0xC5};
		int is_pause = 1;
		for(int i = 1;i < 6;i++){
			if (GetByteFromBuffer() != pause_code[i]) {
				is_pause = 0;
				break;
			}
		}
		if (is_pause) {
			word = PAUSEBREAK;
		}
	}
	//特殊的由0xE0开头的按键
	else if(new_code == 0xE0){
		//获取第二个字节
		new_code = GetByteFromBuffer();
		//PrintScreen被按下
		if(new_code == 0x2A){
			if(GetByteFromBuffer() == 0xE0) {
				if(GetByteFromBuffer() == 0x37) {
					word= PRINTSCREEN;
					make = 1;
				}
			}
		}
		//PrintScreen被释放
		if(new_code == 0xB7){
			if(GetByteFromBuffer() == 0xE0) {
				if(GetByteFromBuffer() == 0xAA) {
					word= PRINTSCREEN;
					make = 0;
				}
			}
		}
		//不是PrintScreen,则当前按键仅由双字节组成
		if(word == 0){
			code_with_E0 = 1;
		}
	}
	//如果该字符不是PRINTSCREEN或PAUSEBREAK
	if((word != PRINTSCREEN) && (word != PAUSEBREAK)){
		//判断当前按键是Make还是Break
		make = (new_code & FLAG_BREAK)?0:1;
		//定位该行
		u32 *select_row = &keymap[(new_code&0x7F) * MAP_COLS];

		//在当前行内将会打印的项序数
		int column = 0;
		//如果当前shift处于按下的状态
		if(shift_l || shift_r){
			column = 1;
		}
		//如果当前当前按键对应两字节字符
		if(code_with_E0){
			column = 2;
			code_with_E0 = 0;
		}

		//取出对应字符
		u32 word = select_row[column];

		switch(word){
			//若该字符为控制字符,则根据make/break设置全局变量
			case SHIFT_L:
				shift_l = make;	
				break;
			case SHIFT_R:
				shift_r = make;	
				break;
			case CTRL_L:
				ctrl_l = make;	
				break;
			case CTRL_R:
				ctrl_r = make;	
				break;
			case ALT_L:
				alt_l = make;	
				break;
			case ALT_R:
				alt_r = make;	
				break;
			//其他情况下,则直接跳过
			default:
				break;
		}
		//舍弃Break Code
		if(make){
			//如果当前字符是通过按键联动得到的,则为其装配一个说明位
			word |= shift_l?FLAG_SHIFT_L:0;
			word |= shift_r?FLAG_SHIFT_R:0;
			word |= ctrl_l?FLAG_CTRL_L:0;
			word |= ctrl_r?FLAG_CTRL_R:0;
			word |= alt_l?FLAG_ALT_L:0;
			word |= alt_r?FLAG_ALT_R:0;

			
			//将当前按键对应的字符传给InputProcess进行后续处理
			InputProcess(p_tty,word);
		}
	}
}

//将"从键盘I/O缓冲区读一个字节"这一行为封装为一个函数
PRIVATE char GetByteFromBuffer(void){
	char new_code;	//保存新字节
	
	while(keyboard_input_buf.count <= 0);	//等待新按键的到来

	DisableInterupt();
	new_code = *(keyboard_input_buf.p_tail);
	keyboard_input_buf.p_tail++;
	//如果指针到达了缓冲区末尾,则将指针移至缓冲区开头
	if(keyboard_input_buf.p_tail == keyboard_input_buf.buf+KEYBOARD_INPUT_SIZE){
		keyboard_input_buf.p_tail = keyboard_input_buf.buf;
	}
	//减少缓冲区中的字节累计数
	keyboard_input_buf.count--;
	EnableInterupt();
	
	return new_code;
}

//8259键盘中断反馈句柄
PUBLIC void KeyboardHandler(int irq){
	u8 new_code = InputByte(0x60); 	//获得一个新扫描码


	//如果缓冲区已满,则直接舍弃
	if(keyboard_input_buf.count < KEYBOARD_INPUT_SIZE){
		*(keyboard_input_buf.p_head) = new_code;
		keyboard_input_buf.p_head++;
		//如果已到达缓冲区末尾,则将指针移至缓冲区开头
		if(keyboard_input_buf.p_head == keyboard_input_buf.buf+KEYBOARD_INPUT_SIZE){
			keyboard_input_buf.p_head = keyboard_input_buf.buf;
		}
		//增大缓冲区中的字节累计个数
		keyboard_input_buf.count++;
	}

	return;
}
