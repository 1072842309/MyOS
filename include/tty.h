
#ifndef _TTY_H_
#define _TTY_H_

#define TTY_IN_BUF_SIZE 256


typedef struct s_tty{
	u32 input_buf[TTY_IN_BUF_SIZE];		//输入缓冲区
	u32 *p_input_buf_head;			//缓冲区的下一个空闲位置
	u32 *p_input_buf_tail;			//缓冲区中应被首先处理的位置
	int input_buf_count;			//缓冲区已占用空间大小

	struct s_console *p_console;		//当前tty对应的console
}TTY;









#endif
