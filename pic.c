#include "pic.h"

extern sem_t cmd_sem; // 信号，用在产生键控字符模块和读取字符模块，协调两个线程
extern char cmd;      // 命令字符，设为全局变量，在各线程中都要访问
/**
 * @brief 遍历挂载U盘的目录，把所有图像文件放在数组中
 * @details 接收目录名作为参数，从此目录中读取图像文件名，全部放在数组中，然后循环调用图像显示函数，显示图像
 * @param[in] dirname 目录名
 * @return 无
 */
void img_search(char *dirname)
{
    char *fname = NULL;                                  // 指向文件名的指针
    char *dirname2 = (char *)malloc(sizeof(char) * 128); // 如果分配太小，很可能放不下文件名（含路径）
    DIR *dp;                                             // 目录指针
    struct dirent *entry;                                // 目录项结构体
    struct stat statbuf;                                 // 文件属性结构体
    // 打开的目录为当前目录路下的picture目录
    if ((dp = opendir(dirname)) == NULL)
    {
        perror("err: failed to opendir\n"); // 打开失败，返回
        return;
    }
    chdir(dirname);                       // 切换到当前目录
    while ((entry = readdir(dp)) != NULL) // 读取指定文件夹下图像文件名（字符串）放到内存 用指针fname指向这个字符串
    {
        lstat(entry->d_name, &statbuf); // 获取文件属性
        if (S_ISDIR(statbuf.st_mode))   // 如果是目录，递归调用img_search函数
        {
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) // 跳过.和..目录
            {
                continue;
            }
            sprintf(dirname2, "%s/%s", dirname, entry->d_name); // 连接字符串，构造一个完整的文件名
            img_search(dirname2);                               // 递归调用
        }
        else
        {
            // 判断是否为图像文件,如果是，则把文件名放到fname_array[i]指向的空间中去，如果不是，继续readdir
            int len = strlen(entry->d_name);                                                          // 获取文件名长度
            len -= 4;                                                                                 // 去掉后缀名
            if (strcmp(entry->d_name + len, ".jpg") != 0 && strcmp(entry->d_name + len, ".JPG") != 0) // 判断是否为图像文件 .jpg或.JPG结尾 4个字符
            {
                continue;
            }
            else
            {
                fname = entry->d_name;                                      // 指向文件名的指针
                fname_array[img_count] = (char *)malloc(sizeof(char) * 64); // 为每个指针分配空间
                sprintf(fname_array[img_count], "%s/%s", dirname, fname);   // 连接字符串，构造一个完整的文件名
                // printf("%s\n", fname_array[img_count]);
                img_count++; // 图片数量加1
            }
        }
    }
    chdir("..");  // 切换到上一级目录
    closedir(dp); // 关闭目录
    return;
}

/**
 * @brief 播放图片的线程函数
 * @details 接收目录名作为参数，从此目录中读取图像文件名，全部放在数组中，然后循环调用图像显示函数，显示图像
 * @param[in] dirname 目录名
 * @return 无
 */
void func_pic(char *dirname)
{
    int j = 0;           // 用来索引每一幅图片
    img_search(dirname); // 调用函数，把所有图像文件名放在数组中
    // sem_wait(&cmd_sem);  // 等待信号量，信号量为0时，阻塞
    while (1)
    {
        // 播放完一幅图片后，等待信号量，信号量为0时，阻塞
        sem_wait(&cmd_sem); // 等待信号量，信号量为0时，阻塞
        // printf("cmd = %c\n", cmd);
        //
        switch (cmd)
        {
        case 's':
            j++; // 播放下一幅图片
            break;
        case 'w':
            // printw("Down arrow key\n");
            j--; // 播放上一幅图片
            break;
        default:
            break;
        }
        // 播放下一幅图片
        if (j == img_count)
        {
            j = 0;
        }
        else if (j < 0)
        {
            j = img_count - 1;
        }
        frame_prcs(fname_array[j]);
    }
}

/**
 * @brief 处理一幅图像的函数，要被循环调用
 * @details 接收图像文件名作为参数，把图像解压，然后把图像数据放在缓冲区，最后把缓冲区内容写到屏幕
 * @param[in] fname 图像文件名
 * @return 无
 */
void frame_prcs(char *fname)
{
    system("clear"); // 清屏
    // system("setterm -cursor off"); // 关闭光标
    // 打开framebuffer设备
    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1)
    {
        perror("Error opening framebuffer device");
        return -1;
    }

    // 获取 FrameBuffer 变量和屏幕信息
    struct fb_var_screeninfo vinfo;
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable screen info");
        close(fbfd);
        return -1;
    }

    struct fb_fix_screeninfo finfo;
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed screen info");
        close(fbfd);
        return -1;
    }
    // #ifdef DEBUG
    //     // 输出屏幕信息
    //     printf("Framebuffer Information:\n");
    //     printf("Device: %s\n", "/dev/fb0");
    //     printf("Resolution: %dx%d\n", vinfo.xres, vinfo.yres);
    //     printf("Virtual Resolution: %dx%d\n", vinfo.xres_virtual, vinfo.yres_virtual);
    //     printf("Bits Per Pixel: %d\n", vinfo.bits_per_pixel);
    //     printf("Bytes Per Pixel: %d\n", vinfo.bits_per_pixel / 8);
    //     printf("Line Length: %d\n", finfo.line_length);
    //     printf("Buffer Size: %ld bytes\n", vinfo.yres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8);
    // #endif
    long screensize = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;

    // 映射 FrameBuffer 到内存
    char *framebuffer = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (framebuffer == MAP_FAILED)
    {
        perror("Error mapping framebuffer to memory");
        close(fbfd);
        return -1;
    }
    struct jpeg_decompress_struct cinfo; // 解压对象
    struct jpeg_error_mgr jerr;          // 错误处理对象
    FILE *infile;                        // 文件指针

    // 打开图像文件
    if ((infile = fopen(fname, "rb")) == NULL)
    {
        perror("err: open image failed\n");
        exit(-1);
    }
    cinfo.err = jpeg_std_error(&jerr); // 初始化错误处理对象
    jpeg_create_decompress(&cinfo);    // 创建解压对象
    jpeg_stdio_src(&cinfo, infile);    // 指定解压对象的数据源
    jpeg_read_header(&cinfo, TRUE);    // 读取图像文件的头信息
    jpeg_start_decompress(&cinfo);     // 开始解压

    // 获取图像的每一行的数据，放到buffer中
    unsigned char *buffer[cinfo.output_height];
    int i;
    for (i = 0; i < cinfo.output_height; i++)
    {
        buffer[i] = (unsigned char *)malloc(cinfo.output_width * cinfo.output_components);
    }
    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *row_pointer = buffer[cinfo.output_scanline];
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
    }

    // 获取屏幕的虚拟分辨率
    int screen_width = vinfo.xres_virtual;
    int screen_height = vinfo.yres_virtual;

    // 获取img的宽度和高度
    int img_width = cinfo.output_width;
    int img_height = cinfo.output_height;
    // #ifdef DEBUG
    //     printf("img_width = %d\n\t", img_width);
    //     printf("img_height = %d\n\t", img_height);
    // #endif
    // 计算xy的偏移量
    int x_offset = (vinfo.xres - img_width);  // 水平居中
    int y_offset = (vinfo.yres - img_height); // 垂直居中

    // 检查偏移量是否超出屏幕边界，如果超出则调整偏移量
    if (x_offset < 0)
    {
        x_offset = 0;
    }
    if (y_offset < 0)
    {
        y_offset = 0;
    }

    int j;
    // 生成一个1-8的随机数
    srand((unsigned)time(NULL));
    int num = rand() % 8 + 1;
    // 根据特效选择，对图像进行处理
    switch (num)
    {
    case 1: // 1从上到下
    {
        for (i = 0; i < cinfo.output_height && screen_height > img_height; i++)
        {
            for (j = 0; j < cinfo.output_width && screen_width > img_width; j++)
            {
                // 计算偏移地址offset
                // int offset = (j + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (i + vinfo.yoffset) * finfo.line_length;
                int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                // 将buffer像素写入到framebuffer中
                *(framebuffer + offset) = buffer[i][j * 3 + 2];
                *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                *(framebuffer + offset + 2) = buffer[i][j * 3];
            }
            if (i % 2 == 0)
            {
                usleep(1500); // 休眠1毫秒
            }
        }
    }
    break;
    case 2: // 2. 从下到上
    {
        // 从下往上刷新屏幕
        for (i = cinfo.output_height - 1; i >= 0 && screen_height > img_height; i--)
        {
            for (j = cinfo.output_width - 1; j >= 0 && screen_width > img_width; j--)
            {
                // 计算偏移地址offset
                // int offset = (j + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (i + vinfo.yoffset) * finfo.line_length;
                int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                // 将buffer像素写入到framebuffer中
                *(framebuffer + offset) = buffer[i][j * 3 + 2];
                *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                *(framebuffer + offset + 2) = buffer[i][j * 3];
            }
            if (i % 2 == 0)
            {
                usleep(1500); // 休眠1毫秒
            }
        }
    }
    break;
    case 3: // 从中心往外
    {
        // 从中心往外刷新屏幕
        int max_radius = (img_width > img_height ? img_width : img_height) / 2;
        for (int radius = 0; radius <= max_radius; radius++)
        {
            // 计算刷新区域的边界
            int x_min = img_width / 2 - radius;
            int x_max = img_width / 2 + radius;
            int y_min = img_height / 2 - radius;
            int y_max = img_height / 2 + radius;
            // 防止越界
            if (x_min < 0)
            {
                x_min = 0;
            }
            if (x_max > img_width - 1)
            {
                x_max = img_width - 1;
            }
            if (y_min < 0)
            {
                y_min = 0;
            }
            if (y_max > img_height - 1)
            {
                y_max = img_height - 1;
            }

            int x, y;

            // 刷新指定区域的像素
            for (i = y_min; i <= y_max; i++)
            {
                for (j = x_min; j <= x_max; j++)
                {
                    // 计算偏移地址offset
                    int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                    // 将buffer像素写入到framebuffer中
                    *(framebuffer + offset) = buffer[i][j * 3 + 2];
                    *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                    *(framebuffer + offset + 2) = buffer[i][j * 3];
                }
            }
            usleep(1500); // 休眠1毫秒
        }
    }
    break;
    case 4:
    {
        //   计算每个九宫格的宽度和高度
        int grid_width = img_width / 3;
        int grid_height = img_height / 3;

        // 九宫格切换效果
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                // 计算当前九宫格的位置
                int grid_x = col * grid_width;
                int grid_y = row * grid_height;

                // 刷新当前九宫格的像素
                for (int i = grid_y; i < grid_y + grid_height; i++)
                {
                    for (int j = grid_x; j < grid_x + grid_width; j++)
                    {
                        // 计算偏移地址offset
                        int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                        // 将buffer像素写入到framebuffer中
                        *(framebuffer + offset) = buffer[i][j * 3 + 2];
                        *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                        *(framebuffer + offset + 2) = buffer[i][j * 3];
                    }
                }

                usleep(100000); // 休眠100毫秒
            }
        }
    }
    break;
    case 5:
    {
        // 间隔刷新
        int grid_width = img_width / 9;
        int grid_height = img_height / 9;
        for (j = 0; j < 2; j++)
        {
            for (i = 0; i < 81; i++)
            {
                // 如果是偶数且j为0则
                if (i % 2 == 0 && j == 0)
                {
                    // 取余计算出行列
                    int row = (i) / 9;
                    int col = (i) % 9;
                    // 计算当前九宫格的位置
                    int grid_x = col * grid_width;
                    int grid_y = row * grid_height;

                    // 刷新当前九宫格的像素
                    for (int i = grid_y; i < grid_y + grid_height; i++)
                    {
                        for (int j = grid_x; j < grid_x + grid_width; j++)
                        {
                            // 计算偏移地址offset
                            int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                            // 将buffer像素写入到framebuffer中
                            *(framebuffer + offset) = buffer[i][j * 3 + 2];
                            *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                            *(framebuffer + offset + 2) = buffer[i][j * 3];
                        }
                    }
                    usleep(10000); // 休眠100毫秒
                }
                else if (i % 2 == 1 && j == 1)
                {
                    // 取余计算出行列
                    int row = (i) / 9;
                    int col = (i) % 9;
                    // 计算当前九宫格的位置
                    int grid_x = col * grid_width;
                    int grid_y = row * grid_height;

                    // 刷新当前九宫格的像素
                    for (int i = grid_y; i < grid_y + grid_height; i++)
                    {
                        for (int j = grid_x; j < grid_x + grid_width; j++)
                        {
                            // 计算偏移地址offset
                            int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                            // 将buffer像素写入到framebuffer中
                            *(framebuffer + offset) = buffer[i][j * 3 + 2];
                            *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                            *(framebuffer + offset + 2) = buffer[i][j * 3];
                        }
                    }
                    usleep(1000); // 休眠100毫秒
                }
            }
        }
    }
    break;
    case 7:
    {
        // 定义初始扰动幅度和最小扰动幅度
        int initialDisturbance = 200;

        // 外部循环控制刷新次数
        for (int refreshCount = 0; refreshCount < 10; refreshCount++)
        {
            // 内部循环刷新图片
            for (i = 0; i < cinfo.output_height && screen_height > img_height; i++)
            {
                for (j = 0; j < cinfo.output_width && screen_width > img_width; j++)
                {
                    // 计算偏移地址 offset
                    int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;

                    // 计算当前扰动量
                    int disturbance = initialDisturbance - refreshCount * initialDisturbance / 10;

                    // 为每个像素的颜色添加扰动
                    int r = buffer[i][j * 3 + 2] + disturbance; // 扰动红色分量
                    int g = buffer[i][j * 3 + 1] + disturbance; // 扰动绿色分量
                    int b = buffer[i][j * 3] + disturbance;     // 扰动蓝色分量

                    // 将处理后的颜色写入 framebuffer
                    *(framebuffer + offset) = (unsigned char)(r < 0 ? 0 : (r > 255 ? 255 : r));
                    *(framebuffer + offset + 1) = (unsigned char)(g < 0 ? 0 : (g > 255 ? 255 : g));
                    *(framebuffer + offset + 2) = (unsigned char)(b < 0 ? 0 : (b > 255 ? 255 : b));
                }
            }
            usleep(100000); // 休眠100毫秒
        }
    }
    break;
    case 8:
    {
        // 定义旋转角度和放大倍数
        float rotationAngle = 0.0; // 旋转角度
        float scaleFactor = 4.6;   // 放大倍数
        // 获取背景颜色
        unsigned char backgroundR = 0; // 背景红色分量
        unsigned char backgroundG = 0; // 背景绿色分量
        unsigned char backgroundB = 0; // 背景蓝色分量

        // 清除屏幕为背景色
        for (int y = 0; y < img_height; y++)
        {
            for (int x = 0; x < img_width; x++)
            {
                // 计算偏移地址offset
                int offset = (x + x_offset) * (vinfo.bits_per_pixel / 8) + (y + y_offset) * finfo.line_length;
                // 填充背景色
                *(framebuffer + offset) = backgroundR;
                *(framebuffer + offset + 1) = backgroundG;
                *(framebuffer + offset + 2) = backgroundB;
            }
        }
        // 从最小值旋转两圈刚好填满屏幕
        float max_rotationAngle = 360.0; // 最大旋转角度
        float angle_step = 10;           // 旋转角度步长

        while (rotationAngle <= max_rotationAngle)
        {
            scaleFactor -= 0.1;
            // 计算当前旋转角度对应的弧度值
            float radian = rotationAngle * M_PI / 180.0;

            // 刷新指定区域的像素
            for (int y = 0; y < img_height; y++)
            {
                for (int x = 0; x < img_width; x++)
                {
                    // 计算相对于中心点的偏移坐标
                    int dx = x - img_width / 2;
                    int dy = y - img_height / 2;

                    // 计算旋转后的位置
                    float rotatedX = dx * cos(radian) - dy * sin(radian);
                    float rotatedY = dx * sin(radian) + dy * cos(radian);

                    // 计算放大后的位置
                    int scaledX = (int)(rotatedX * scaleFactor) + img_width / 2;
                    int scaledY = (int)(rotatedY * scaleFactor) + img_height / 2;

                    // 防止越界
                    if (scaledX >= 0 && scaledX < img_width && scaledY >= 0 && scaledY < img_height)
                    {
                        // 计算偏移地址offset
                        int offset = (x + x_offset) * (vinfo.bits_per_pixel / 8) + (y + y_offset) * finfo.line_length;

                        // 将buffer像素写入到framebuffer中
                        *(framebuffer + offset) = buffer[scaledY][scaledX * 3 + 2];
                        *(framebuffer + offset + 1) = buffer[scaledY][scaledX * 3 + 1];
                        *(framebuffer + offset + 2) = buffer[scaledY][scaledX * 3];
                    }
                }
            }
            usleep(25000); // 休眠15毫秒
            // 清除屏幕为背景色
            if (rotationAngle != 360.0)
            {
                // 清除屏幕为背景色 但是最后一次不清除
                for (int y = 0; y < img_height; y++)
                {
                    for (int x = 0; x < img_width; x++)
                    {
                        // 计算偏移地址offset
                        int offset = (x + x_offset) * (vinfo.bits_per_pixel / 8) + (y + y_offset) * finfo.line_length;
                        // 填充背景色
                        *(framebuffer + offset) = backgroundR;
                        *(framebuffer + offset + 1) = backgroundG;
                        *(framebuffer + offset + 2) = backgroundB;
                    }
                }
            }
            // 更新旋转角度
            rotationAngle += angle_step;
        }
    }
    break;
    default:
    {
        // 随机特效
        int grid_width = img_width / 9;
        int grid_height = img_height / 9;
        // 九宫格切换效果
        // 生成1-16的随机数并存入数组
        int arr[81];
        for (i = 0; i < 81; i++)
        {
            arr[i] = i + 1;
        }
        // 打乱数组
        srand(time(NULL));
        for (i = 0; i < 81; i++)
        {
            int index = rand() % 81;
            int temp = arr[i];
            arr[i] = arr[index];
            arr[index] = temp;
        }
        // for循环将数组中的元素打印出来
        for (i = 0; i < 81; i++)
        {
            // 取余计算出行列
            int row = (arr[i] - 1) / 9;
            int col = (arr[i] - 1) % 9;
            // 计算当前九宫格的位置
            int grid_x = col * grid_width;
            int grid_y = row * grid_height;
            // 刷新当前九宫格的像素
            for (int i = grid_y; i < grid_y + grid_height; i++)
            {
                for (int j = grid_x; j < grid_x + grid_width; j++)
                {
                    // 计算偏移地址offset
                    int offset = (j + x_offset) * (vinfo.bits_per_pixel / 8) + (i + y_offset) * finfo.line_length;
                    // 将buffer像素写入到framebuffer中
                    *(framebuffer + offset) = buffer[i][j * 3 + 2];
                    *(framebuffer + offset + 1) = buffer[i][j * 3 + 1];
                    *(framebuffer + offset + 2) = buffer[i][j * 3];
                }
            }
            usleep(10000);
        }
    }
    break;
    }
    // 释放缓冲区内存
    for (i = 0; i < cinfo.output_height; i++)
    {
        free(buffer[i]);
    }

    // 完成解压，销毁加压对象
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    munmap(framebuffer, screensize);

    // 关闭 FrameBuffer 设备文件
    close(fbfd);

    return;
}
