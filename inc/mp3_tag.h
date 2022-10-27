/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2021-06-02     MrzhangF1ghter    first implementation
 */

#ifndef __MP3_TAG_H__
#define __MP3_TAG_H__

#include <stdint.h>
#include <stdio.h>
#include "mp3_player.h"

#define TITLE_LEN_MAX 30
#define ARTIST_LEN_MAX 30
#define ALBUM_LEN_MAX 30

/* 
 *  ID3V1 TAG
 */
typedef struct
{
    uint8_t id[3];
    uint8_t title[30];
    uint8_t artist[30];
    uint8_t year[4];
    uint8_t comment[30];
    uint8_t genre;
} ID3V1_Tag_t;

/* 
 *  ID3V2 TAG header
 */
typedef struct
{
    uint8_t id[3];
    uint8_t mversion;
    uint8_t sversion;
    uint8_t flags;
    uint8_t size[4];
} ID3V2_TagHead_t;

/* 
 *  ID3V2.3 TAG header
 */
typedef struct
{
    uint8_t id[4];
    uint8_t size[4];
    uint16_t flags;
} ID3V23_FrameHead_t;

/* 
 *  MP3 Xing Frame
 */
typedef struct
{
    uint8_t id[4];
    uint8_t flags[4];
    uint8_t frames[4];
    uint8_t fsize[4];
} MP3_FrameXing_t;

/* 
 *  MP3 VBRI Frame
 */
typedef struct
{
    uint8_t id[4];      /* frame id:Xing/Info */
    uint8_t version[2]; /* VBRI Version */
    uint8_t delay[2];   /* delay */
    uint8_t quality[2]; /* audio quality，0~100 */
    uint8_t fsize[4];   /* file size */
    uint8_t frames[4];  /* total frame */
} MP3_FrameVBRI_t;

/**
 * @description: Get genre string by genre id
 * @param {uint16_t} genre_id [0,147]
 * @return {char *} genre string
 */
char *mp3_get_genre_string_by_id(uint16_t genre_id);

/**
 * @description: get mp3 tag info
 * @param {struct mp3_player} *player
 * @return the error code,0 on success
 */
rt_err_t mp3_get_info(struct mp3_player *player);

/**
 * @description: print mp3 info
 * @param {mp3_info_t} mp3_info
 * @return the error code,0 on success
 */
rt_err_t mp3_info_print(mp3_info_t mp3_info);

#endif
