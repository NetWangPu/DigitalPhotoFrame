#include "cmd.h"

extern char *fname_array_song[100];
extern int now_i_song;
extern int count_all_song;

extern char cmd;      // 命令字符，设为全局变量，在各线程中都要访问
extern sem_t cmd_sem; // 信号，用在产生键控字符模块和读取字符模块，协调两个线程

/**
 * @brief 隐藏输入字符
 * @details 用于隐藏输入字符，使得输入的字符不显示在屏幕上
 * @param[in] 无
 * @return 无
 */
void hide_input()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * @brief 产生字符命令的线程函数
 * @details 产生的字符，放在全局变量cmd中，在图片线程中被使用
 * @param[in] arg 无
 * @return 无
 */
void cmd_create(void *arg)
{
    initscr();            // 初始化 ncurses
    keypad(stdscr, TRUE); // 启用特殊键盘输入
    hide_input();        // 隐藏输入字符
    while ((cmd = getch()) != 'q')
    {
        switch (cmd)
        {
        case 'w':
            // printw("Up arrow key\n");
            sem_post(&cmd_sem); // 发送信号通知线程1有新的命令
            break;
        case 's':
            // printw("Left arrow key\n");
            sem_post(&cmd_sem); // 发送信号通知线程1有新的命令
            break;
        case 'e': // 播放音乐
            play_sound(fname_array_song[now_i_song]);
            break;
        case 'd': // 暂停音乐
            stop_sound();
            break;
        case 'f': // 下一首音乐
            next_sound();
            break;
        case 'r': // 上一首音乐
            last_sound();
            break;
        default:
            // sem_post(&cmd_sem); // 发送信号通知线程1有新的命令
            break;
        }
        refresh();
    }

    endwin(); // 关闭 ncurses

    return NULL;
}
