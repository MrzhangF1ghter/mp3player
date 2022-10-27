# mp3player

[中文页](README_ZH.md) | English

## 1. Introduction

**mp3player** is a simple mp3 format music player that provides functions for playing mp3 files,decode id3v1,id3v2 tag, supporting functions such as play, stop, pause, resume, seek,and volume adjustment.

### 1.1. File structure

| Folder | Description |
| ---- | ---- |
| src | Core source code, which mainly implements mp3 playback and tag decode, and export Finsh command line |
| inc | Header file directory |

### 1.2 License

The mp3player package complies with the Apache 2.0 license, see the `LICENSE` file for details.

### 1.3 Dependency

- RT-Thread 4.0+
- RT-Thread Audio driver framework
- optparse command line parameter parsing package
- helix mp3 decoder

### 1.4 Configuration Macro Description


```shell
 --- mp3 player: Minimal music player for mp3 file play.   
 [*]   Enable mp3 player                                   
 (sound0) The play device name                                       
 (2048) mp3 input buffer size                                   
 (4608) mp3 output buffer size   
 (50)  mp3 player default volume                                 
       Version (v1.0.0)  --->  
```

**The play device name**: Specify the sound card device used for playback, default `sound0`

## 2. Use

Common functions of mp3player have been exported to Finsh command line for developers to test and use.

**The functions provided by the play command are as follows**

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

### 2.1 Play function

- Start playing

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
- seek play
```shell
jump to 100 seconds
msh />mp3play -j 100
```

- Stop play

```shell
msh />mp3play -t
[I/mp3 player] play end
```

- Pause playback

```shell
msh />mp3play -p
```

- Resume playback

```shell
msh />mp3play -r
```

- Set volume

```shell
msh />mp3play -v 50
```

```shell
msh />mp3play -d

- dump info
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
## 3. Matters needing attention

- 

## 4. Contact

- Maintenance: MrzhangF1ghter
- Homepage: https://github.com/MrzhangF1ghter/mp3player
