#ifndef SOUND_H
#define SOUND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

extern pthread_mutex_t my_lock2; // 互斥锁，音乐线程和图片线程都要读取此目录

void play_sound(char *fname); // 播放音乐
void func_mus(char *dirname); // 音乐线程函数
void next_sound();            // 播放下一首音乐
void last_sound();            // 播放上一首音乐
void stop_sound();            // 关闭后台播放的音乐

#endif
