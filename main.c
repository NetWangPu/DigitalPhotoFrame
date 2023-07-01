/**
 * @file main.c
 * @brief 主函数
 * @details 电子相框
 * @version v1.0
 * @date 2023-7-1
 * @author Mr.wang
 * @E-mail: 32284879@qq.com
 * @note 本程序主要实现了电子相框的功能，包括图片的显示，音乐的播放，控制命令的接收
 * @todo 未来的版本将加入MQTT功能 用于远程控制
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <jpeglib.h>
#include <jerror.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <semaphore.h>
// #include <MQTTAsync.h>

#include "sound.h"
#include "pic.h"
#include "cmd.h"
// #include "netmqtt.h"

// MQTTAsync client; // 定义MQTT客户端  需要导入MQTTAsync.h头文件 需
// MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
// 定义全局变量
pthread_mutex_t my_lock2 = PTHREAD_MUTEX_INITIALIZER; // 互斥锁，音乐线程和图片线程都要读取此目录
sem_t cmd_sem;										  // 信号，用在产生键控字符模块和读取字符模块，协调两个线程
char cmd = '0';										  // 命令字符，设为全局变量，在各线程中都要访问
pthread_t pic_tid, cmd_tid, mqtt_tid;				  // 图片线程和命令线程的线程ID
int err;											  // 用于判断线程创建是否成功
char *dirname = "/home/wp/Desktop/testpic";			  // 文件所在目录

char *fname_array_song[100];
int now_i_song = 0;
int count_all_song = 0;
/**
 * @brief 主函数
 * @details 通过创建子进程
 * @param[in] argc 参数个数
 * @param[in] argv 参数数组
 * @return 返回0
 */
int main(int argc, char *argv[])
{
	system("setterm -cursor off"); // 关闭光标
	// 初始化互斥锁
	err = pthread_mutex_init(&my_lock2, NULL);
	if (err)
	{
		perror("err: init mutex failed\n");
		exit(EXIT_FAILURE);
	}

	// 初始化信号量
	err = sem_init(&cmd_sem, 0, 0);
	if (err)
	{
		perror("err: init semphore failed\n");
		exit(EXIT_FAILURE);
	}

	// 创建获取控制命令的子线程
	err = pthread_create(&cmd_tid, NULL, (void *)cmd_create, NULL);
	if (err != 0)
	{
		perror("err: create cmd_tid failed\n");
		exit(EXIT_FAILURE);
	}

	// // 创建获取mqtt的子线程
	// err = pthread_create(&mqtt_tid, NULL, (void *)mqttInit, NULL);
	// if (err != 0)
	// {
	// 	perror("err: create cmd_tid failed\n");
	// 	exit(EXIT_FAILURE);
	// }

	// 创建播放图片的子线程
	err = pthread_create(&pic_tid, NULL, (void *)func_pic, (void *)dirname);
	if (err != 0)
	{
		perror("err: can't create pic_thread\n");
		exit(EXIT_FAILURE);
	}

	// 播放音乐
	func_mus(dirname);

	// 主线程一直阻塞，等待pic_tid的返回 也就是图片线程结束
	err = pthread_join(pic_tid, NULL);
	if (err != 0)
	{
		printf("can't join with pic_thread\n");
	}

	// 主线程一直阻塞，等待cmd_tid的返回 也就是命令线程结束
	err = pthread_join(cmd_tid, NULL);
	if (err != 0)
	{
		printf("can't join with cmd_thread\n");
	}

	// // 主线程一直阻塞，等待mqtt_tid的返回 也就是命令线程结束
	// err = pthread_join(mqtt_tid, NULL);
	// if (err != 0)
	// {
	// 	printf("can't join with cmd_thread\n");
	// }

	system("setterm -cursor on"); // 打开光标
	system("clear");			  // 清屏
	// 关闭音乐
	system("sudo killall -9 madplay");

	exit(EXIT_SUCCESS); // 父进程退出
	return 0;
}
