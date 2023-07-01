/**
 * @file main.c
 * @brief ������
 * @details �������
 * @version v1.0
 * @date 2023-7-1
 * @author Mr.wang
 * @E-mail: 32284879@qq.com
 * @note ��������Ҫʵ���˵������Ĺ��ܣ�����ͼƬ����ʾ�����ֵĲ��ţ���������Ľ���
 * @todo δ���İ汾������MQTT���� ����Զ�̿���
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

// MQTTAsync client; // ����MQTT�ͻ���  ��Ҫ����MQTTAsync.hͷ�ļ� ��
// MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
// ����ȫ�ֱ���
pthread_mutex_t my_lock2 = PTHREAD_MUTEX_INITIALIZER; // �������������̺߳�ͼƬ�̶߳�Ҫ��ȡ��Ŀ¼
sem_t cmd_sem;										  // �źţ����ڲ��������ַ�ģ��Ͷ�ȡ�ַ�ģ�飬Э�������߳�
char cmd = '0';										  // �����ַ�����Ϊȫ�ֱ������ڸ��߳��ж�Ҫ����
pthread_t pic_tid, cmd_tid, mqtt_tid;				  // ͼƬ�̺߳������̵߳��߳�ID
int err;											  // �����ж��̴߳����Ƿ�ɹ�
char *dirname = "/home/wp/Desktop/testpic";			  // �ļ�����Ŀ¼

char *fname_array_song[100];
int now_i_song = 0;
int count_all_song = 0;
/**
 * @brief ������
 * @details ͨ�������ӽ���
 * @param[in] argc ��������
 * @param[in] argv ��������
 * @return ����0
 */
int main(int argc, char *argv[])
{
	system("setterm -cursor off"); // �رչ��
	// ��ʼ��������
	err = pthread_mutex_init(&my_lock2, NULL);
	if (err)
	{
		perror("err: init mutex failed\n");
		exit(EXIT_FAILURE);
	}

	// ��ʼ���ź���
	err = sem_init(&cmd_sem, 0, 0);
	if (err)
	{
		perror("err: init semphore failed\n");
		exit(EXIT_FAILURE);
	}

	// ������ȡ������������߳�
	err = pthread_create(&cmd_tid, NULL, (void *)cmd_create, NULL);
	if (err != 0)
	{
		perror("err: create cmd_tid failed\n");
		exit(EXIT_FAILURE);
	}

	// // ������ȡmqtt�����߳�
	// err = pthread_create(&mqtt_tid, NULL, (void *)mqttInit, NULL);
	// if (err != 0)
	// {
	// 	perror("err: create cmd_tid failed\n");
	// 	exit(EXIT_FAILURE);
	// }

	// ��������ͼƬ�����߳�
	err = pthread_create(&pic_tid, NULL, (void *)func_pic, (void *)dirname);
	if (err != 0)
	{
		perror("err: can't create pic_thread\n");
		exit(EXIT_FAILURE);
	}

	// ��������
	func_mus(dirname);

	// ���߳�һֱ�������ȴ�pic_tid�ķ��� Ҳ����ͼƬ�߳̽���
	err = pthread_join(pic_tid, NULL);
	if (err != 0)
	{
		printf("can't join with pic_thread\n");
	}

	// ���߳�һֱ�������ȴ�cmd_tid�ķ��� Ҳ���������߳̽���
	err = pthread_join(cmd_tid, NULL);
	if (err != 0)
	{
		printf("can't join with cmd_thread\n");
	}

	// // ���߳�һֱ�������ȴ�mqtt_tid�ķ��� Ҳ���������߳̽���
	// err = pthread_join(mqtt_tid, NULL);
	// if (err != 0)
	// {
	// 	printf("can't join with cmd_thread\n");
	// }

	system("setterm -cursor on"); // �򿪹��
	system("clear");			  // ����
	// �ر�����
	system("sudo killall -9 madplay");

	exit(EXIT_SUCCESS); // �������˳�
	return 0;
}
