//操作内存的相关函数

#ifndef _STRING_H_
#define _STRING_H_

//c函数memcpy的山寨版,将src处的size个字节拷贝至dst处
PUBLIC void* MemCpy(void *dst,void *src,int size);

PUBLIC void* MemSet(void *dst,char ch,int size);

PUBLIC void* StrCpy(void *dst,void *src);

PUBLIC int StrLen(char *p_str);

#endif //_STRING_H_
