//函数原型

#ifndef _PROTO_H_
#define _PROTO_H

// lib/kernel_lib_asm.asm
PUBLIC void OutputByte(u16 port,u8 value);	//向端口port中写出value
PUBLIC u8 InputByte(u16 port);			//从端口port中读入值,并返回
PUBLIC void DisplayStr(char *info);		//在disp_pos指定的位置打出info,并更新disp_pos
PUBLIC void DispInt(int input);
PUBLIC void DispColorStr(char *info,int color);//与disp_str唯一的不同是可以制定打印颜色
PUBLIC void DisableIrq(int irq);		//关闭对应的8259中断
PUBLIC void EnableIrq(int irq);			//开启对应的8259中断
PUBLIC void DisableInterupt(void);		//关闭中断响应
PUBLIC void EnableInterupt(void);		//开启中断响应

// kernel/protect.c
PUBLIC void I8259A();			//将8259的16个边角的中断输出注册到idt中的中断向量上去
PUBLIC void IProt();				//进行与保护模式相关的初始化工作,
						//1.为中断向量表中的门描述符赋值 2.设置GDT描述符TSS 3.设置GDT描述符LDT
PUBLIC u32 Seg2Phy(u16 seg);			//32位GDT描述符 -> 指定段的绝对地址

// lib/kernel_lib_c.c
PUBLIC void Delay(int time);
PUBLIC char* Int2Str(char *str,int num);

// kernel/kernel.asm
PUBLIC void Restart(void);
PUBLIC void SysCall(void);

// kernel/main.c
void TestA();			//首个测试进程B
void TestB();			//测试进程B
void TestC();			//测试进程C

// kernel/interupt_8259.c
PUBLIC void PutIrqHandler(int irq, irq_handler handler);
PUBLIC void SpuriousIrq(int irq);

// kernel/clock.c 
PUBLIC void IClock(void);		//初始化8259时钟中断
PUBLIC void ClockHandler(int irq);	//8259时钟中断反馈句柄
PUBLIC void MilliDelay(int milli_sec);	//延迟sec毫秒

//proc.c
PUBLIC int syscall_get_ticks(void);	//
PUBLIC void schedule(void);		//进程调度

//syscall.asm
PUBLIC int get_ticks(void);	
PUBLIC void write(char *buf,int len);

// kernel/keyboard.c
PUBLIC void IKeyboard(void);		//初始化8259键盘中断
PUBLIC void KeyboardRead(TTY *p_tty);		//从键盘缓冲区读出一个字节
PUBLIC void KeyboardHandler(int irq);	//8259键盘中断反馈句柄

// kernel/tty.c
PUBLIC void TaskTTY(void);		//终端
PUBLIC void InputProcess(TTY *p_tty,u32 word);	//后续处理键盘输入
PUBLIC int syscall_write(char *buf,int len,PROCESS *p_proc);	//

// kernel/console.c
PUBLIC void IConsole(TTY *p_tty);	//为tty装配控制台
PUBLIC void ChangeConsole(int new_console_index);	//更换控制台
PUBLIC void OutChar(CONSOLE *p_console,char ch);	//向控制台打印一个字符
PUBLIC void SetCursor(unsigned int pos);	//充值光标
PUBLIC int IsCurrentConsole(CONSOLE *p_console);	
PUBLIC void ScrollScreen(CONSOLE *p_con,int dir);	//滚动屏幕

// kernel/printf.c
int SPrintf(char *buf,const char *format,va_list arg);
int Printf(const char *format,...);





#endif
