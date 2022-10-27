/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2021-06-02     MrzhangF1ghter    first implementation
 */

#ifndef __MP3_PLAYER_H__
#define __MP3_PLAYER_H__

#include <stdio.h>
#include <rtdevice.h>

#include "mp3dec.h" /* helix include files */

enum MSG_TYPE
{
    MSG_NONE = 0,
    MSG_START = 1,
    MSG_STOP = 2,
    MSG_PAUSE = 3,
    MSG_RESUME = 4,
};

enum PLAYER_EVENT
{
    PLAYER_EVENT_NONE = 0,
    PLAYER_EVENT_PLAY = 1,
    PLAYER_EVENT_STOP = 2,
    PLAYER_EVENT_PAUSE = 3,
    PLAYER_EVENT_RESUME = 4,
};

struct play_msg
{
    int type;
    void *data;
};

#pragma pack(1)
typedef struct
{
    uint8_t *read_ptr;
    int read_offset;
    int bytes_left;
} decode_oper_t;

/* 
 * music basic info structure definition
 */
typedef struct
{
    uint8_t title[30];
    uint8_t artist[30];
    uint8_t year[4];
    uint8_t comment[30];
    uint8_t genre;
} mp3_basic_info_t;

/* 
 * mp3_info structure definition
 */
typedef struct
{
    mp3_basic_info_t mp3_basic_info;
    uint32_t total_seconds;
    uint32_t curent_seconds;

    uint32_t bitrate;
    uint32_t samplerate;
    uint16_t outsamples;
    uint8_t vbr;
    uint32_t data_start;
    long file_size;
} mp3_info_t;

/* 
 * mp3 player main structure definition
 */
struct mp3_player
{
    int state;
    char *uri;
    uint8_t *in_buffer;
    uint16_t *out_buffer;
    rt_device_t audio_device;
    rt_mq_t mq;
    rt_mutex_t lock;
    struct rt_completion ack;
    FILE *fp;

    int volume;

    /* helix decoder */
    HMP3Decoder mp3_decoder;
    MP3FrameInfo mp3_frameinfo;

    mp3_info_t mp3_info;

    decode_oper_t decode_oper;
};
#pragma pack()

/**
 * mp3 player status
 */
enum PLAYER_STATE
{
    PLAYER_STATE_STOPED = 0,
    PLAYER_STATE_PLAYING = 1,
    PLAYER_STATE_PAUSED = 2,
};

/**
 * @brief             Play wav music
 *
 * @param uri         the pointer for file path
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int mp3_player_play(char *uri);

/**
 * @brief             Stop music
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int mp3_player_stop(void);

/**
 * @brief             Pause music
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int mp3_player_pause(void);

/**
 * @brief             Resume music
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int mp3_player_resume(void);

/**
 * @brief             Sev volume
 *
 * @param volume      volume value(0 ~ 99)
 *
 * @return
 *      - 0      Success
 *      - others Failed
 */
int mp3_player_volume_set(int volume);

/**
 * @brief             Get volume
 *
 * @return            volume value(0~00)
 */
int mp3_player_volume_get(void);

/**
 * @brief             Get wav player state
 *
 * @return
 *      - PLAYER_STATE_STOPED   stoped status
 *      - PLAYER_STATE_PLAYING  playing status
 *      - PLAYER_STATE_PAUSED   paused
 */
int mp3_player_state_get(void);

/**
 * @brief             Get the uri that is currently playing
 *
 * @return            uri that is currently playing
 */
char *mp3_player_uri_get(void);

/**
 * @brief             show mp3 info
 */
void mp3_info_show(void);

/**
 * @brief             show mp3 info
 */
void mp3_disp_time(void);

/**
 * @brief             seek to destination seconds
 *
 * @return            the error code,0 on success
 */
rt_err_t mp3_seek(uint32_t seconds);

#endif
