#include "sound.h"

extern char *fname_array_song[100];
extern int now_i_song;
extern int count_all_song;

/**
 * @brief 播放音乐
 * @details 通过madplay命令播放音乐
 * @param[in] fname 音乐的路径
 * @return 无
 */
void play_sound(char *fname)
{
    char *fname2 = (char *)malloc(sizeof(char) * 64);
    // 后台播放 不输出
    sprintf(fname2, "madplay %s -r >/dev/null 2>&1 &", fname); //-r 重复播放 &后台播放 不输出
    system(fname2);
    return;
}

/**
 * @brief 关闭后台播放的音乐
 * @details 通过kill命令关闭后台播放的音乐
 * @param[in] 无
 * @return 无
 */
void stop_sound()
{
    system("pkillmadplay ");
    return;
}

/**
 * @brief 播放下一首音乐
 * @details 通过kill命令关闭后台播放的音乐
 * @param[in] 无
 * @return 无
 */
void next_sound()
{
    system("pkill madplay");
    now_i_song++;
    if (now_i_song >= count_all_song)
        now_i_song = 0;
    play_sound(fname_array_song[now_i_song]);
    return;
}
/**
 * @brief 播放上一首音乐
 * @details 通过kill命令关闭后台播放的音乐
 * @param[in] 无
 * @return 无
 */
void last_sound()
{
    system("pkill madplay");
    now_i_song--;
    if (now_i_song < 0)
        now_i_song = count_all_song - 1;
    play_sound(fname_array_song[now_i_song]);
    return;
}

/**
 * @brief 音乐线程函数
 * @details 通过madplay命令播放音乐
 * @param[in] dirname 音乐所在目录
 * @return 无
 */
void func_mus(char *dirname)
{
    char *fname = NULL;

    DIR *dp;
    struct dirent *entry;

    // 操作U盘目录之前上锁
    pthread_mutex_lock(&my_lock2);

    if ((dp = opendir(dirname)) == NULL) // 打开的目录为当前目录路下的picture目录
    {
        perror("err: fail to opendir\n");
        return;
    }

    char *fname2 = (char *)malloc(sizeof(char) * 64); // 如果分配太小，很可能放不下文件名（含路径）

    while ((entry = readdir(dp)) != NULL) // 读取指定文件夹下图像文件名（字符串）放到内存 用指针fname指向这个字符串
    {
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
            continue;
        // 判断是否为音频文件
        int len = strlen(entry->d_name);
        len -= 4;
        if (strcmp(entry->d_name + len, ".mp3") != 0 && strcmp(entry->d_name + len, ".MP3") != 0)
            continue;
        else
        {
            fname = entry->d_name;
            sprintf(fname2, "%s/%s", dirname, fname); // 连接字符串，构造一个完整的文件名
            fname_array_song[count_all_song] = (char *)malloc(sizeof(char) * 64);
            strcpy(fname_array_song[count_all_song], fname2);
            count_all_song++;
        }
    }
    // U盘目录操作完毕，解锁
    pthread_mutex_unlock(&my_lock2);
    free(fname2);
    play_sound(fname_array_song[now_i_song]);
    pthread_exit((void *)0);
}
