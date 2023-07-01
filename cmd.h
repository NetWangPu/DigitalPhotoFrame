#ifndef CMD_H
#define CMD_H

#include <stdio.h>     // 包含标准输入输出库，用于 getchar() 和 fflush() 函数
#include <pthread.h>   // 包含线程相关的库，用于 pthread_create() 函数
#include <semaphore.h> // 包含信号量相关的库，用于 sem_post() 函数
#include <ncurses.h>   // 包含 ncurses 库，用于 getch() 函数
#include <unistd.h>    // 包含 Linux 标准库，用于 sleep() 函数
#include <fcntl.h>
#include "sound.h"
#include <termios.h>

extern pthread_mutex_t mutex;      // 互斥锁，用于线程间的互斥访问
extern pthread_cond_t cond;        // 条件变量，用于线程间的同步
extern unsigned char *buffer[600]; // 用于存放图片的缓冲区

void cmd_create(void *arg); // 产生字符命令的线程函数
void hide_input();          // 隐藏输入字符

#endif