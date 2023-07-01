#ifndef pic_h
#define pic_h
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <dirent.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <ncurses.h> // 包含 ncurses 库，用于 getch() 函数

#include <semaphore.h>

#include <math.h>

#define BUFSZ 1024        // 1024张图片
int img_count;            // 统计图片文件数量的变量，应为函数要递归，所以要设为全局的
char *fname_array[BUFSZ]; // 指针数组，每个元素指向一个图像文件的全名
#define DEBUG 0           // 调试开关，1为开，0为关

extern pthread_mutex_t mutex; // 互斥锁，用于线程间的互斥访问
extern pthread_cond_t cond;   // 条件变量，用于线程间的同步

void img_search(char *dirname); // 递归搜索目录，将所有图像文件的全名存入fname_array数组中
void func_pic(char *dirname);   // 图片线程函数

#endif