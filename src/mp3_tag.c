/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2021-06-02     MrzhangF1ghter    first implementation
 */

#include "mp3_tag.h"
#include <rtthread.h>
#include <string.h>

#define LOG_TAG "mp3 tag"
#define LOG_LVL DBG_INFO
#include <ulog.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

char *genre_type[] = {
    "Blues",
    "Classic Rock",
    "Country",
    "Dance",
    "Disco",
    "Funk",
    "Grunge",
    "Hip-Hop",
    "Jazz",
    "Metal",
    "New Age",
    "Oldies",
    "Other",
    "Pop",
    "R&B",
    "Rap",
    "Reggae",
    "Rock",
    "Techno",
    "Industrial",
    "Alternative",
    "Ska",
    "Death Metal",
    "Pranks",
    "Soundtrack",
    "Euro-Techno",
    "Ambient",
    "Trip-Hop",
    "Vocal",
    "Jazz+Funk",
    "Fusion",
    "Trance",
    "Classical",
    "Instrumental",
    "Acid",
    "House",
    "Game",
    "Sound Clip",
    "Gospel",
    "Noise",
    "Alternative Rock",
    "Bass",
    "Soul",
    "Punk",
    "Space",
    "Meditative",
    "Instrumental Pop",
    "Instrumental Rock",
    "Ethnic",
    "Gothic",
    "Darkwave",
    "Techno-Industrial",
    "Electronic",
    "Pop-Folk",
    "Eurodance",
    "Dream",
    "Southern Rock",
    "Comedy",
    "Cult",
    "Gangsta",
    "Top 40",
    "Christian Rap",
    "Pop/Funk",
    "Jungle",
    "Native US",
    "Cabaret",
    "New Wave",
    "Psychadelic",
    "Rave",
    "Showtunes",
    "Trailer",
    "Lo-Fi",
    "Tribal",
    "Acid Punk",
    "Acid Jazz",
    "Polka",
    "Retro",
    "Musical",
    "Rock & Roll",
    "Hard Rock",
    "Folk",
    "Folk-Rock",
    "National Folk",
    "Swing",
    "Fast Fusion",
    "Bebob",
    "Latin",
    "Revival",
    "Celtic",
    "Bluegrass",
    "Avantgarde",
    "Gothic Rock",
    "Progressive Rock",
    "Psychedelic Rock",
    "Symphonic Rock",
    "Slow Rock",
    "Big Band",
    "Chorus",
    "Easy Listening",
    "Acoustic",
    "Humour",
    "Speech",
    "Chanson",
    "Opera",
    "Chamber Music",
    "Symphony",
    "Booty Bass",
    "Primus",
    "Porn Groove",
    "Satire",
    "Slow Jam",
    "Club",
    "Tango",
    "Samba",
    "Folklore",
    "Ballad",
    "Power Ballad",
    "Rhytmic Soul",
    "Freestyle",
    "Duet",
    "Punk Rock",
    "Drum Solo",
    "Acapella",
    "Euro-House",
    "Dance Hall",
    "Goa",
    "Drum & Bass",
    "Club-House",
    "Hardcore",
    "Terror",
    "Indie",
    "BritPop",
    "Negerpunk",
    "Polsk Punk",
    "Beat",
    "Christian Gangsta",
    "Heavy Metal",
    "Black Metal",
    "Crossover",
    "Contemporary C",
    "Christian Rock",
    "Merengue",
    "Salsa",
    "Thrash Metal",
    "Anime",
    "JPop",
    "SynthPop",
};

/**
 * @description: Get genre string by genre id
 * @param {uint16_t} genre_id [0,147]
 * @return {char *} genre string
 */
char *mp3_get_genre_string_by_id(uint16_t genre_id)
{
    if (genre_id < 0 || genre_id > 147)
        return RT_NULL;
    return genre_type[genre_id];
}

/**
 * @description: print id3v1 tag
 * @param {ID3V1_Tag_t} id3v1_tag
 * @return None
 */
void mp3_id3v1_tag_print(ID3V1_Tag_t id3v1_tag)
{
    rt_kprintf("Title:%s\r\n", id3v1_tag.title);
    rt_kprintf("Artist:%s\r\n", id3v1_tag.artist);
    rt_kprintf("Year:%s\r\n", id3v1_tag.year);
    rt_kprintf("Comment:%s\r\n", id3v1_tag.comment);
    rt_kprintf("Genre:%s\r\n", mp3_get_genre_string_by_id(id3v1_tag.genre));
}

/**
 * @description: print mp3 info
 * @param {mp3_info_t} mp3_info
 * @return the error code,0 on success
 */
rt_err_t mp3_info_print(mp3_info_t mp3_info)
{
    rt_kprintf("------------MP3 INFO------------\r\n");
    rt_kprintf("Title:%s\r\n", mp3_info.mp3_basic_info.title);
    rt_kprintf("Artist:%s\r\n", mp3_info.mp3_basic_info.artist);
    rt_kprintf("Year:%s\r\n", mp3_info.mp3_basic_info.year);
    rt_kprintf("Comment:%s\r\n", mp3_info.mp3_basic_info.comment);
    rt_kprintf("Genre:%s\r\n", mp3_get_genre_string_by_id(mp3_info.mp3_basic_info.genre));
    rt_kprintf("Length:%02d:%02d\r\n", mp3_info.total_seconds / 60, mp3_info.total_seconds % 60);
    rt_kprintf("Bitrate:%d kbit/s\r\n", mp3_info.bitrate / 1000);
    rt_kprintf("Frequency:%d Hz\r\n", mp3_info.samplerate);
    rt_kprintf("--------------------------------\r\n");
}

/**
 * @description: id3v1 tag decode
 * @param {uint8_t} *buf
 * @param {ID3V1_Tag_t} *id3v1_tag
 * @return the error code,0 on success
 */
static rt_err_t mp3_id3v1_tag_decode(FILE *fp, uint8_t *buf, mp3_basic_info_t *basic_info)
{
    ID3V1_Tag_t *tag;
    fpos_t file_pos = 0;
    int read_size;
    rt_err_t ret;

    if (fp == RT_NULL || buf == RT_NULL)
    {
        return RT_ERROR;
    }

    file_pos = ftell(fp); /* save current file positon */

    fseek(fp, -128, SEEK_END);       /* move read pointer to id3v1 position */
    if (fread(buf, 1, 128, fp) <= 0) /* read 128 bytes */
    {
        ret = RT_ERROR;
        goto __exit;
    }

    tag = (ID3V1_Tag_t *)buf;
    /* check header */
    if (strncmp("TAG", (char *)tag->id, 3) == 0)
    {
        /* is id3v1 tag,copy to user */
        if (tag->title)
            memcpy(basic_info->title, tag->title, 30);
        if (tag->artist)
            memcpy(basic_info->artist, tag->artist, 30);
        if (tag->year)
            memcpy(basic_info->year, tag->year, 4);
        if (tag->comment)
            memcpy(basic_info->comment, tag->comment, 30);
        basic_info->genre = tag->genre;
        ret = RT_EOK;
    }
    else
    {
        ret = RT_ERROR;
    }

__exit:
    fseek(fp, file_pos, SEEK_SET); /* resume file positon */
    return ret;
}

/**
 * @description: read id3v2 text
 * @param {FILE} *fp
 * @param {uint32_t} data_len
 * @param {char} *buff
 * @param {uint32_t} buff_size
 * @return the error code,0 on success
 * @verbatim  Taken from http://www.mikrocontroller.net/topic/252319
 */
static uint8_t mp3_read_id3v2_text(FILE *fp, uint32_t data_len, char *buff, uint32_t buff_size)
{
    uint32_t read_size = 0;
    uint8_t byEncoding = 0;
    if (fread(&byEncoding, 1, 1, fp) == 1)
    {
        data_len--;
        if (data_len <= (buff_size - 1))
        {
            if ((fread(buff, 1, data_len, fp) == data_len))
            {
                if (byEncoding == 0)
                {
                    // ISO-8859-1 multibyte
                    // just add a terminating zero
                    buff[data_len] = 0;
                }
                else if (byEncoding == 1)
                {
                    // UTF16LE unicode
                    uint32_t r = 0;
                    uint32_t w = 0;
                    if ((data_len > 2) && (buff[0] == 0xFF) && (buff[1] == 0xFE))
                    {
                        // ignore BOM, assume LE
                        r = 2;
                    }
                    for (; r < data_len; r += 2, w += 1)
                    {
                        // should be acceptable for 7 bit ascii
                        buff[w] = buff[r];
                    }
                    buff[w] = 0;
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            // we won't read a partial text
            if (fseek(fp, ftell(fp) + data_len, SEEK_SET) != 0)
            {
                return 1;
            }
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

/**
 * @description: id3v2 decode
 * @param {FILE} *fp
 * @param {mp3_basic_info_t} *mp3_info
 * @return {*}
 * @verbatim  Taken from http://www.mikrocontroller.net/topic/252319
 */
static uint32_t mp3_id3v2_tag_decode(FILE *fp, mp3_basic_info_t *mp3_info)
{
    ID3V2_TagHead_t id3_head;
    ID3V23_FrameHead_t frame_head;

    uint32_t ret = 0;

    uint32_t read_size;
    uint32_t tag_size = 0;
    fpos_t file_pos = 0;

    uint32_t offset = 0;
    uint8_t exhd[4];
    uint32_t frame_to_read = 2;
    uint32_t i;
    uint32_t frame_size = 0;

    if (fp == RT_NULL || mp3_info == RT_NULL)
        return 0;

    file_pos = ftell(fp); /* save current file positon */

    if (fread(&id3_head, 1, 10, fp) != 10)
    {
        ret = 0;
        goto __exit;
    }
    else
    {
        /* check header */
        if (strncmp("ID3", (const char *)id3_head.id, 3) == 0)
        {
            offset += 10; /* move to tag frame */
            tag_size = ((id3_head.size[0] & 0x7f) << 21) | ((id3_head.size[1] & 0x7f) << 14) | ((id3_head.size[2] & 0x7f) << 7) | (id3_head.size[3] & 0x7f);
            ret = tag_size;
            LOG_D("tag_size:%.2f kB", tag_size / 1024.0);
            // try to get some information from the tag
            // skip the extended header, if present
            if (id3_head.flags & 0x40)
            {
                fread(&exhd, 1, 4, fp);
                uint32_t ex_hdr_skip = ((exhd[0] & 0x7f) << 21) | ((exhd[1] & 0x7f) << 14) | ((exhd[2] & 0x7f) << 7) | (exhd[3] & 0x7f);
                ex_hdr_skip -= 4;
                if (fseek(fp, ftell(fp) + ex_hdr_skip, SEEK_SET) != 0)
                {
                    ret = 0;
                    goto __exit;
                }
            }
            while (frame_to_read > 0)
            {
                if (fread(&frame_head, 1, 10, fp) != 10)
                {
                    ret = 0;
                    goto __exit;
                }
                if (frame_head.id[0] == 0 || (strncmp(frame_head.id, "3DI", 3) == 0))
                {
                    break;
                }
                for (; i < 4; i++)
                {
                    if (id3_head.mversion == 3)
                    {
                        /* ID3v2.3 */
                        frame_size <<= 8;
                        frame_size += frame_head.size[i];
                    }
                    if (id3_head.mversion == 4)
                    {
                        /* ID3v2.4 */
                        frame_size <<= 7;
                        frame_size += frame_head.size[i] & 0x7F;
                    }
                }

                if (strcmp(frame_head.id, "TPE1") == 0)
                {
                    /* artist */
                    if (mp3_read_id3v2_text(fp, frame_size, mp3_info->artist, 30) != 0)
                    {
                        break;
                    }
                    frame_to_read--;
                }
                else if (strcmp(frame_head.id, "TIT2") == 0)
                {
                    /* title */
                    if (mp3_read_id3v2_text(fp, frame_size, mp3_info->title, 30) != 0)
                    {
                        break;
                    }
                    frame_to_read--;
                }
                else
                {
                    if (fseek(fp, ftell(fp) + frame_size, SEEK_SET) != 0)
                    {
                        return 0;
                    }
                }
            }
        }
    }
__exit:
    fseek(fp, file_pos, SEEK_SET); /* resume file positon */
    return ret;
}

/**
 * @description: get mp3 tag info
 * @param {struct mp3_player} *player
 * @return the error code,0 on success
 */
rt_err_t mp3_get_info(struct mp3_player *player)
{
    rt_err_t ret = RT_EOK;

    MP3FrameInfo frame_info;
    MP3_FrameXing_t *fxing;
    MP3_FrameVBRI_t *fvbri;

    int offset = 0;
    uint32_t p;
    short samples_per_frame;
    uint32_t total_frame;
    ID3V1_Tag_t ID3V1_Tag = {0};
    uint32_t read_size = 0;
    fpos_t file_pos = 0;

    if (player->fp == NULL)
    {
        LOG_E("%s is not opened", player->uri);
        return RT_ERROR;
    }
    file_pos = ftell(player->fp); /* save current file positon */
    LOG_D("current file pos:%d", file_pos);
    memset(&player->mp3_info, 0, sizeof(mp3_info_t));

    /* get file size */
    fseek(player->fp, 0, SEEK_END);
    player->mp3_info.file_size = ftell(player->fp);
    fseek(player->fp, 0, SEEK_SET);
    LOG_D("%s:%d KB,%.2f MB", player->uri, player->mp3_info.file_size / 1024, player->mp3_info.file_size / 1024 / 1024.0);

    player->mp3_info.data_start = mp3_id3v2_tag_decode(player->fp, &player->mp3_info.mp3_basic_info); /* decode ID3V2 tag*/

    mp3_id3v1_tag_decode(player->fp, player->in_buffer, &player->mp3_info.mp3_basic_info); /* decode ID3V1 tag */

    LOG_D("mp3 data start at :%f KB", player->mp3_info.data_start / 1024.0);

    fseek(player->fp, player->mp3_info.data_start, SEEK_SET);
    while ((read_size = fread(player->in_buffer, 1, MP3_INPUT_BUFFER_SIZE, player->fp)))
    {
        if ((offset = MP3FindSyncWord(player->in_buffer, read_size)) >= 0)
            break;
    }

    LOG_D("sync word offset at:%d", offset);
    if (offset >= 0 && MP3GetNextFrameInfo(player->mp3_decoder, &frame_info, &player->in_buffer[offset]) == 0)
    {

        p = offset + 4 + 32;
        fvbri = (MP3_FrameVBRI_t *)(player->in_buffer + p);
        if (strncmp("VBRI", (char *)fvbri->id, 4) == 0) /* VBRI frame*/
        {
            if (frame_info.version == MPEG1)
                samples_per_frame = 1152;
            else
                samples_per_frame = 576;
            total_frame = ((uint32_t)fvbri->frames[0] << 24) | ((uint32_t)fvbri->frames[1] << 16) | ((uint16_t)fvbri->frames[2] << 8) | fvbri->frames[3]; /* get total frame */
            player->mp3_info.total_seconds = total_frame * samples_per_frame / frame_info.samprate;                                                       /* get total length */
        }
        else /* maybe is Xing frame*/
        {
            if (frame_info.version == MPEG1)
            {
                p = frame_info.nChans == 2 ? 32 : 17;
                samples_per_frame = 1152;
            }
            else
            {
                p = frame_info.nChans == 2 ? 17 : 9;
                samples_per_frame = 576;
            }
            p += offset + 4;
            fxing = (MP3_FrameXing_t *)(player->in_buffer + p);
            if (strncmp("Xing", (char *)fxing->id, 4) == 0 || strncmp("Info", (char *)fxing->id, 4) == 0)
            {
                if (fxing->flags[3] & 0X01) /* TOC is valid */
                {
                    total_frame = ((uint32_t)fxing->frames[0] << 24) | ((uint32_t)fxing->frames[1] << 16) | ((uint16_t)fxing->frames[2] << 8) | fxing->frames[3]; /* get total flame */

                    player->mp3_info.total_seconds = total_frame * samples_per_frame / frame_info.samprate; /* get total length */
                    player->mp3_info.vbr = 1;
                }
                else
                {
                    player->mp3_info.total_seconds = (player->mp3_info.file_size - (128 + player->mp3_info.data_start)) / (frame_info.bitrate / 8);
                    player->mp3_info.vbr = 0;
                }
            }
            else /* CBR Format */
            {
                player->mp3_info.total_seconds = (player->mp3_info.file_size - (128 + player->mp3_info.data_start)) / (frame_info.bitrate / 8);
                player->mp3_info.vbr = 0;
            }
        }
        player->mp3_info.bitrate = frame_info.bitrate;
        player->mp3_info.samplerate = frame_info.samprate;
        if (frame_info.nChans == 2)
            player->mp3_info.outsamples = frame_info.outputSamps;
        else
            player->mp3_info.outsamples = frame_info.outputSamps * 2;
    }
    else
    {
        LOG_E("can not find sync frame");
        ret = RT_ERROR;
    }
__exit:

    if (player->fp)
        fsetpos(player->fp, &file_pos); /* resume file positon */
    return ret;
}