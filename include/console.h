

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

typedef struct s_console{
	unsigned int original_address;		//对应显存的初位置(用于辨认不同控制台)
	unsigned int display_address;		//在屏幕上显示部分的初位置
	unsigned int video_memory_limit;		//对应显存的最大值
	unsigned int cursor;			//光标的位置
}CONSOLE;

//传递参数
#define SCR_UP 8
#define SCR_DOWN 2

#define SCREEN_WIDTH 80				//屏幕一行的长度(以字符为单位)
#define SCREEN_HIDTH 25				//屏幕一列的长度(以字符为单位)
#define SCREEN_SIZE (SCREEN_WIDTH*SCREEN_HIDTH)	//屏幕一面的大小(以字符为单位)





#endif
