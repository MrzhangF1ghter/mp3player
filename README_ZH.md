# mp3player

中文页 | [English](README.md)

## 1. 简介

**mp3player** 是一个简易的 mp3 格式的音乐播放器，提供播放mp3 文件的功能，支持获取MP3 标签信息、播放、跳转、停止、暂停、恢复，以及音量调节等功能。

### 1.1. 文件结构

| 文件夹 | 说明 |
| ---- | ---- |
| src  | 核心源码，主要实现 MP3 播放和MP3 标签解析，以及导出 Finsh 命令行 |
| inc  | 头文件目录 |

### 1.2 许可证

mp3player package 遵循 Apache 2.0 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 4.0+
- RT-Thread Audio 驱动框架
- optparse 命令行参数解析软件包
- helix MP3解码软件包

### 1.4 配置宏说明

```shell
 --- mp3 player: Minimal music player for mp3 file play.   
 [*]   Enable mp3 player                                   
 (sound0) The play device name                                       
 (2048) mp3 input buffer size                                   
 (4608) mp3 output buffer size   
 (50)  mp3 player default volume                                 
       Version (v1.0.0)  --->  
```

**The play device name**：指定播放使用的声卡设备，默认`sound0`  

## 2. 使用

mp3player 的常用功能已经导出到 Finsh 命令行，以便开发者测试和使用。

**播放命令提供的功能如下 **

```shell
msh />mp3play -help
usage: mp3play [option] [target] ...

usage options:
  -h,     --help                     Print defined help message.
  -s URI, --start=URI                Play mp3 music with URI(local files).
  -t,     --stop                     Stop playing music.
  -p,     --pause                    Pause the music.
  -r,     --resume                   Resume the music.
  -v lvl, --volume=lvl               Change the volume(0~99).
  -d,     --dump                     Dump play relevant information.
  -j      --jump                     Jump to seconds that given.
```

### 2.1 播放功能

- 开始播放

```shell
msh />mp3play -s bryan_adams_-_here_i_am.mp3
[I/mp3 player]: play start, uri=bryan_adams_-_here_i_am.mp3
msh />------------MP3 INFO------------
Title:Here I Am
Artist:Bryan Adams
Year:2002
Comment:Spirit: Stallion Of The Cimarr
Genre:Blues
Length:04:45
Bitrate:320 kbit/s
Frequency:44100 Hz
--------------------------------
```

- 跳转播放
```shell
跳转至100秒
msh />mp3play -j 100
```

- 停止播放

```shell
msh />mp3play -t
[I/mp3 player] play end
```

- 暂停播放

```shell
msh />mp3play -p
```

- 恢复播放

```shell
msh />mp3play -r
```

- 设置音量

```shell
msh />mp3play -v 50
```

- 打印播放信息
```shell
msh />mp3play -d

mp3_player status:
uri     - bryan_adams_-_here_i_am.mp3
status  - PLAYING
volume  - 10
00:03 / 04:45
------------MP3 INFO------------
Title:Here I Am
Artist:Bryan Adams
Year:2002
Comment:Spirit: Stallion Of The Cimarr
Genre:Blues
Length:04:45
Bitrate:320 kbit/s
Frequency:44100 Hz
--------------------------------
```

## 3. 注意事项

- 待补充

## 4. 联系方式

- 维护：MrzhangF1ghter
- 主页：https://github.com/MrzhangF1ghter/mp3player
