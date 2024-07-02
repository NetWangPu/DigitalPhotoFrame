# 电子相框代码

### 实现功能

自动搜索指定文件的资源

音乐播放控制(E开始播放 D是结束播放 R是上一首 F切换到下一首)

图片随机特效播放控制(W上一张 S下一张)

图片特效 :

1.从上往下刷新

2.从下网上刷新

3.棋盘特效

4.随机棋盘特效

5.旋转放大

6.亮白特效

7.九宫格特效

8.从中间展示特效

等

### 项目部署

##### 运行环境

Distributor ID:	Ubuntu
Description:	Ubuntu 16.04.7 LTS
Release:	16.04
Codename:	xenial

##### 设置frameBuffer

800 * 600 24bit

##### 安装库

安装libjpeg62-dev

```shell
sudo apt-get install libjpeg62-dev
```

安装madplay

```shell
apt-get install madplay
```

安装libncurses5-dev

```shell
sudo apt install libncurses5-dev
```

##### 编译指令

```shell
 gcc main.c sound.c  pic.c cmd.c -ljpeg -lpthread -lncurses -lm
```

#### 项目写的一般，有很多不足之处，欢迎大家指正，项目还需要继续完善优化 时间有限，后续会继续完善

#### 联系方式

邮箱：321284879@qq.com
