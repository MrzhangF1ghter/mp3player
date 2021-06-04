/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2021-06-02     MrzhangF1ghter    first implementation
 */

#include "mp3_player.h"
#include <string.h>

#define LOG_TAG "mp3 player"
#define LOG_LVL DBG_INFO
#include <ulog.h>

#include "mp3_tag.h"

#define VOLUME_MIN (0)
#define VOLUME_MAX (100)

#define MP3_PLAYER_MSG_SIZE (10)
#define MP3_THREAD_STATCK_SIZE (1024 * 2)
#define MP3_THREAD_PRIORITY (15)

static struct mp3_player player = {0};

#if (LOG_LVL >= DBG_LOG)

static const char *state_str[] =
    {
        "STOPPED",
        "PLAYING",
        "PAUSED",
};

static const char *event_str[] =
    {
        "NONE",
        "PLAY"
        "STOP"
        "PAUSE"
        "RESUME"};

#endif

static char *MP3Decode_ERR_CODE_get(int err_code);

/**
 * @description: lock player
 * @param None
 * @return None
 */
static void play_lock(void)
{
    rt_mutex_take(player.lock, RT_WAITING_FOREVER);
}

/**
 * @description: unlock player
 * @param None
 * @return None
 */
static void play_unlock(void)
{
    rt_mutex_release(player.lock);
}

/**
 * @description: send msg to player
 * @param {struct mp3_player} *player
 * @param {int} type
 * @param {void} *data
 * @return the error code,0 on success
 */
static rt_err_t play_msg_send(struct mp3_player *player, int type, void *data)
{
    struct play_msg msg;

    msg.type = type;
    msg.data = data;

    return rt_mq_send(player->mq, &msg, sizeof(struct play_msg));
}

/**
 * @description: start playing
 * @param {char} *uri
 * @return the error code,0 on success
 */
int mp3_player_play(char *uri)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);
    play_lock();
    if (player.state != PLAYER_STATE_STOPED)
    {
        mp3_player_stop();
    }
    if (player.uri)
    {
        rt_free(player.uri);
    }
    player.uri = rt_strdup(uri);
    result = play_msg_send(&player, MSG_START, RT_NULL);
    rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    play_unlock();

    return result;
}

/**
 * @description: stop playing
 * @param None
 * @return the error code,0 on success
 */
int mp3_player_stop(void)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);

    play_lock();
    if (player.state != PLAYER_STATE_STOPED)
    {
        result = play_msg_send(&player, MSG_STOP, RT_NULL);
        rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    }
    play_unlock();

    return result;
}

/**
 * @description: pause playing
 * @param None
 * @return the error code,0 on success
 */
int mp3_player_pause(void)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);
    play_lock();
    if (player.state == PLAYER_STATE_PLAYING)
    {
        result = play_msg_send(&player, MSG_PAUSE, RT_NULL);
        rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    }
    play_unlock();

    return result;
}

/**
 * @description: resume playing
 * @param None
 * @return the error code,0 on success
 */
int mp3_player_resume(void)
{
    rt_err_t result = RT_EOK;
    rt_completion_init(&player.ack);
    play_lock();
    if (player.state == PLAYER_STATE_PAUSED)
    {
        result = play_msg_send(&player, MSG_RESUME, RT_NULL);
        rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    }
    play_unlock();
    return result;
}

/**
 * @description: set volume
 * @param {int} volume
 * @return the error code,0 on success
 */
int mp3_player_volume_set(int volume)
{
    struct rt_audio_caps caps;
    if (volume < VOLUME_MIN)
        volume = VOLUME_MIN;
    else if (volume > VOLUME_MAX)
        volume = VOLUME_MAX;
    player.audio_device = rt_device_find(MP3_SOUND_DEVICE_NAME);
    if (player.audio_device == RT_NULL)
        return RT_ERROR;

    player.volume = volume;
    caps.main_type = AUDIO_TYPE_MIXER;
    caps.sub_type = AUDIO_MIXER_VOLUME;
    caps.udata.value = volume;

    LOG_D("set volume = %d", volume);
    return rt_device_control(player.audio_device, AUDIO_CTL_CONFIGURE, &caps);
}

/**
 * @description: get current player volume
 * @param None
 * @return volume
 */
int mp3_player_volume_get(void)
{
    return player.volume;
}

/**
 * @description: get current player state
 * @param None
 * @return enum PLAYER_STATE
 */
int mp3_player_state_get(void)
{
    return player.state;
}

/**
 * @description: get current player uri
 * @param None
 * @return pointer to uri
 */
char *mp3_player_uri_get(void)
{
    return player.uri;
}

/**
 * @description: get mp3 current time in seconds
 * @param {FILE} *fp
 * @param {mp3_info_t} *mp3_info
 * @return current seconds,-1 on error
 */
uint32_t mp3_get_cur_seconds(void)
{
    uint32_t fpos = 0;
    uint32_t fptr;
    uint32_t curent_seconds;

    if (player.fp == RT_NULL)
    {
        return -1;
    }
    fptr = ftell(player.fp);
    if (fptr > player.mp3_info.data_start)
        fpos = fptr - player.mp3_info.data_start;

    curent_seconds = fpos * player.mp3_info.total_seconds / (player.mp3_info.file_size - player.mp3_info.data_start);
    player.mp3_info.curent_seconds = curent_seconds;
    return curent_seconds;
}

/**
 * @description: seek to destination seconds
 * @param {uint32_t} seconds
 * @return the error code,0 on success
 */
rt_err_t mp3_seek(uint32_t seconds)
{
    long fpos;
    if (seconds > player.mp3_info.total_seconds)
        return RT_ERROR;
    /* calculate position by seconds*/
    fpos = seconds * (player.mp3_info.bitrate / 8) + player.mp3_info.data_start;
    if (fpos < player.mp3_info.data_start)
        return RT_ERROR;
    return fseek(player.fp, fpos + player.mp3_info.data_start, SEEK_SET);
}

/**
 * @description: show mp3 info
 * @param None
 * @return None
 */
void mp3_info_show(void)
{
    mp3_info_print(player.mp3_info);
}

/**
 * @description: show mp3 current seconds
 * @param None
 * @return None
 */
void mp3_disp_time(void)
{
    uint32_t cur_seconds;
    cur_seconds = mp3_get_cur_seconds();
    rt_kprintf("%02d:%02d / %02d:%02d\r\n", cur_seconds / 60, cur_seconds % 60, player.mp3_info.total_seconds / 60, player.mp3_info.total_seconds % 60);
}

/**
 * @description: open mp3 player
 * @param {struct mp3_player} *player
 * @return the error code,0 on success
 */
static rt_err_t mp3_player_open(struct mp3_player *player)
{
    rt_err_t result = RT_EOK;
    struct rt_audio_caps caps;

    /* find device */
    player->audio_device = rt_device_find(MP3_SOUND_DEVICE_NAME);
    if (player->audio_device == RT_NULL)
    {
        LOG_E("audio_device %s not found", MP3_SOUND_DEVICE_NAME);
        result = -RT_ERROR;
        goto __exit;
    }

    /* open file */
    player->fp = fopen(player->uri, "rb"); /* readonly */
    if (player->fp == RT_NULL)
    {
        LOG_E("open file %s failed", player->uri);
        result = -RT_ERROR;
        goto __exit;
    }

    /* open sound device */
    result = rt_device_open(player->audio_device, RT_DEVICE_OFLAG_WRONLY);
    if (result != RT_EOK)
    {
        LOG_E("open %s audio_device failed", MP3_SOUND_DEVICE_NAME);
        goto __exit;
    }

    /* init decoder */
    player->mp3_decoder = MP3InitDecoder();
    if (player->mp3_decoder == 0)
    {
        LOG_E("initialize helix mp3 decoder fail!");
        result = RT_ERROR;
        goto __exit;
    }

    /* set sampletate,channels, samplebits */
    caps.main_type = AUDIO_TYPE_OUTPUT;
    caps.sub_type = AUDIO_DSP_PARAM;
    caps.udata.config.samplerate = 44100;
    caps.udata.config.channels = 2;
    caps.udata.config.samplebits = 16;
    rt_device_control(player->audio_device, AUDIO_CTL_CONFIGURE, &caps);

    return RT_EOK;

__exit:
    if (player->fp)
    {
        fclose(player->fp);
        player->fp = RT_NULL;
    }

    if (player->audio_device)
    {
        rt_device_close(player->audio_device);
        player->audio_device = RT_NULL;
    }

    if (player->mp3_decoder)
    {
        MP3FreeDecoder(player->mp3_decoder);
    }

    return result;
}

/**
 * @description:close mp3 player 
 * @param {struct mp3_player} *player
 * @return None
 */
static void mp3_player_close(struct mp3_player *player)
{
    if (player->fp)
    {
        fclose(player->fp);
        player->fp = RT_NULL;
    }
    if (player->audio_device)
    {
        rt_device_close(player->audio_device);
        player->audio_device = RT_NULL;
    }
    if (player->mp3_decoder)
    {
        MP3FreeDecoder(player->mp3_decoder);
    }
    LOG_D("close mp3 player");
}

/**
 * @description: player event handler
 * @param {struct mp3_player} *player
 * @param {int} timeout
 * @return {int} mp3 event
 */
static int mp3_player_event_handler(struct mp3_player *player, int timeout)
{
    int event;
    rt_err_t result;
    struct play_msg msg;
#if (LOG_LVL >= DBG_LOG)
    rt_uint8_t last_state;
#endif

    result = rt_mq_recv(player->mq, &msg, sizeof(struct play_msg), timeout);
    if (result != RT_EOK)
    {
        event = PLAYER_EVENT_NONE;
        return event;
    }
#if (LOG_LVL >= DBG_LOG)
    last_state = player->state;
#endif
    switch (msg.type)
    {
    case MSG_START:
        event = PLAYER_EVENT_PLAY;
        player->state = PLAYER_STATE_PLAYING;
        break;
    case MSG_STOP:
        event = PLAYER_EVENT_STOP;
        player->state = PLAYER_STATE_STOPED;
        break;

    case MSG_PAUSE:
        event = PLAYER_EVENT_PAUSE;
        player->state = PLAYER_STATE_PAUSED;
        break;

    case MSG_RESUME:
        event = PLAYER_EVENT_RESUME;
        player->state = PLAYER_STATE_PLAYING;
        break;

    default:
        event = PLAYER_EVENT_NONE;
        break;
    }
    rt_completion_done(&player->ack);
#if (LOG_LVL >= DBG_LOG)
    LOG_D("EVENT:%s, STATE:%s -> %s", event_str[event], state_str[last_state], state_str[player->state]);
#endif

    return event;
}

/**
 * @description: mp3 player thread
 * @param {void *}parameter
 * @return None
 */
static void mp3_player_entry(void *parameter)
{
    rt_err_t result = RT_EOK;
    rt_int32_t size;
    int event;

    uint32_t count = 0;

    /* decoder relate */
    int i = 0;
    int err;

    uint32_t cur_ms = 0;

    player.in_buffer = rt_malloc(MP3_INPUT_BUFFER_SIZE);
    if (player.in_buffer == RT_NULL)
    {
        LOG_E("can not malloc input buffer for mp3 player.");
        return;
    }
    player.out_buffer = rt_malloc(MP3_OUTPUT_BUFFER_SIZE);
    if (player.out_buffer == RT_NULL)
    {
        LOG_E("can not malloc output buffer for mp3 player.");
        return;
    }
    memset(player.in_buffer, 0, MP3_INPUT_BUFFER_SIZE);
    memset(player.out_buffer, 0, MP3_OUTPUT_BUFFER_SIZE);

    player.mq = rt_mq_create("mp3_mq", 10, sizeof(struct play_msg), RT_IPC_FLAG_FIFO);
    if (player.mq == RT_NULL)
        goto __exit;

    player.lock = rt_mutex_create("mp3_lock", RT_IPC_FLAG_FIFO);
    if (player.lock == RT_NULL)
        goto __exit;

    player.volume = MP3_PLAYER_VOLUME_DEFAULT;
    /* set volume */
    mp3_player_volume_set(player.volume);

    while (1)
    {
        /* wait play event forever */
        event = mp3_player_event_handler(&player, RT_WAITING_FOREVER);
        if (event != PLAYER_EVENT_PLAY)
            continue;

        /* open mp3 player */
        result = mp3_player_open(&player);
        if (result != RT_EOK)
        {
            player.state = PLAYER_STATE_STOPED;
            LOG_I("open mp3 player failed");
            continue;
        }
        LOG_I("play start, uri=%s", player.uri);
        /* get current mp3 basic info  */
        if (mp3_get_info(&player) == RT_EOK)
        {
            mp3_info_print(player.mp3_info);
        }
        
        fseek(player.fp, player.mp3_info.data_start, SEEK_SET);
        size = fread(player.in_buffer, 1, MP3_INPUT_BUFFER_SIZE, player.fp);
        if (size <= 0)
            goto __exit;

        /* set read ptr to inputbuffer */
        player.decode_oper.read_ptr = player.in_buffer;
        player.decode_oper.bytes_left = size;

        while (1)
        {
            event = mp3_player_event_handler(&player, RT_WAITING_NO);
            switch (event)
            {
            case PLAYER_EVENT_NONE:
            {
                /* find syncword */
                player.decode_oper.read_offset = MP3FindSyncWord(player.decode_oper.read_ptr, player.decode_oper.bytes_left);
                if (player.decode_oper.read_offset < 0) /* can not find syncword */
                {
                    size = fread(player.in_buffer, 1, MP3_INPUT_BUFFER_SIZE, player.fp);
                    if (size <= 0)
                        goto __exit;
                    player.decode_oper.read_ptr = player.in_buffer;
                    player.decode_oper.bytes_left = size;
                    continue;
                }

                player.decode_oper.read_ptr += player.decode_oper.read_offset;   /* move read pointer to syncword */
                player.decode_oper.bytes_left -= player.decode_oper.read_offset; /* data size after syncword */
                if (player.decode_oper.bytes_left < MAINBUF_SIZE * 2)            /* append data */
                {
                    i = (uint32_t)(player.decode_oper.bytes_left) & 3;
                    if (i)
                        i = 4 - i; /* bytes need to append */
                    memcpy(player.in_buffer + i, player.decode_oper.read_ptr, player.decode_oper.bytes_left);
                    player.decode_oper.read_ptr = player.in_buffer + i;
                    size = fread(player.in_buffer + player.decode_oper.bytes_left + i, 1, MP3_INPUT_BUFFER_SIZE - player.decode_oper.bytes_left - i, player.fp); /* copy at aligned position */
                    player.decode_oper.bytes_left += size;
                }
                /* start decode */
                err = MP3Decode(player.mp3_decoder, &player.decode_oper.read_ptr, &player.decode_oper.bytes_left, (short *)player.out_buffer, 0);
                if (err != ERR_MP3_NONE)
                {
                    switch (err)
                    {
                    case ERR_MP3_INDATA_UNDERFLOW:
                        LOG_D("ERR_MP3_INDATA_UNDERFLOW");
                        size = fread(player.in_buffer, 1, MP3_INPUT_BUFFER_SIZE, player.fp); /* append data */
                        player.decode_oper.read_ptr = player.in_buffer;
                        player.decode_oper.bytes_left = size;
                        break;
                    case ERR_MP3_MAINDATA_UNDERFLOW:
                        /* do nothing - next call to decode will provide more mainData */
                        LOG_D("ERR_MP3_MAINDATA_UNDERFLOW");
                        break;
                    default:
                        LOG_D("%s", MP3Decode_ERR_CODE_get(err));
                        if (player.decode_oper.bytes_left > 0)
                        {
                            player.decode_oper.bytes_left--;
                            player.decode_oper.read_ptr++;
                        }
                        break;
                    }
                }
                else /* decode success */
                {
                    MP3GetLastFrameInfo(player.mp3_decoder, &player.mp3_frameinfo); /* get decode info */
                    player.mp3_info.outsamples = player.mp3_frameinfo.outputSamps;
                    if (player.mp3_info.outsamples > 0)
                    {
                        if (player.mp3_frameinfo.nChans == 1) /* Mono */
                        {
                            /* Mono need to copy one channel to another */
                            for (i = player.mp3_info.outsamples - 1; i >= 0; i--)
                            {
                                player.out_buffer[i * 2] = player.out_buffer[i];
                                player.out_buffer[i * 2 + 1] = player.out_buffer[i];
                            }
                            player.mp3_info.outsamples *= 2;
                        }
                    }
                    if (player.mp3_frameinfo.samprate != player.mp3_info.samplerate && player.mp3_info.vbr)
                    {
                        /* set samplerate by frameinfo*/
                        player.mp3_info.samplerate = player.mp3_frameinfo.samprate;
                        struct rt_audio_caps caps;
                        /* set sampletate,channels, samplebits */
                        caps.main_type = AUDIO_TYPE_OUTPUT;
                        caps.sub_type = AUDIO_DSP_PARAM;
                        caps.udata.config.samplerate = player.mp3_info.samplerate;
                        caps.udata.config.channels = 2;
                        caps.udata.config.samplebits = 16;
                        rt_device_control(player.audio_device, AUDIO_CTL_CONFIGURE, &caps);
                    }
                    /* write pcm data to soundcard */
                    rt_device_write(player.audio_device, 0, (uint8_t *)player.out_buffer, MP3_OUTPUT_BUFFER_SIZE);
                }
                if (ftell(player.fp) >= player.mp3_info.file_size)
                {
                    /* FILE END*/
                    player.state = PLAYER_STATE_STOPED;
                }
                break;
            }
            case PLAYER_EVENT_PAUSE:
            {
                /* wait resume or stop event forever */
                event = mp3_player_event_handler(&player, RT_WAITING_FOREVER);
            }

            default:
                break;
            }
            if (player.state == PLAYER_STATE_STOPED)
            {
                break;
            }
        }
        /* close mp3 player */
        mp3_player_close(&player);
        LOG_I("play end");
    }

__exit:
    if (player.in_buffer)
    {
        rt_free(player.in_buffer);
        player.in_buffer = RT_NULL;
    }

    if (player.out_buffer)
    {
        rt_free(player.out_buffer);
        player.out_buffer = RT_NULL;
    }

    if (player.mq)
    {
        rt_mq_delete(player.mq);
        player.mq = RT_NULL;
    }

    if (player.lock)
    {
        rt_mutex_delete(player.lock);
        player.lock = RT_NULL;
    }
}

int mp3_player_init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("mp3_player",
                           mp3_player_entry,
                           RT_NULL,
                           MP3_THREAD_STATCK_SIZE,
                           MP3_THREAD_PRIORITY, 10);
    if (tid)
        rt_thread_startup(tid);

    return RT_EOK;
}

INIT_APP_EXPORT(mp3_player_init);

static char *MP3Decode_ERR_CODE_get(int err_code)
{
    switch (err_code)
    {
    case ERR_MP3_NONE:
        return "ERR_MP3_NONE";
    case ERR_MP3_INDATA_UNDERFLOW:
        return "ERR_MP3_INDATA_UNDERFLOW";
    case ERR_MP3_MAINDATA_UNDERFLOW:
        return "ERR_MP3_MAINDATA_UNDERFLOW";
    case ERR_MP3_FREE_BITRATE_SYNC:
        return "ERR_MP3_FREE_BITRATE_SYNC";
    case ERR_MP3_OUT_OF_MEMORY:
        return "ERR_MP3_OUT_OF_MEMORY";
    case ERR_MP3_NULL_POINTER:
        return "ERR_MP3_NULL_POINTER";
    case ERR_MP3_INVALID_FRAMEHEADER:
        return "ERR_MP3_INVALID_FRAMEHEADER";
    case ERR_MP3_INVALID_SIDEINFO:
        return "ERR_MP3_INVALID_SIDEINFO";
    case ERR_MP3_INVALID_SCALEFACT:
        return "ERR_MP3_INVALID_SCALEFACT";
    case ERR_MP3_INVALID_HUFFCODES:
        return "ERR_MP3_INVALID_HUFFCODES";
    case ERR_MP3_INVALID_DEQUANTIZE:
        return "ERR_MP3_INVALID_DEQUANTIZE";
    case ERR_MP3_INVALID_IMDCT:
        return "ERR_MP3_INVALID_IMDCT";
    case ERR_MP3_INVALID_SUBBAND:
        return "ERR_MP3_INVALID_SUBBAND";
    case ERR_UNKNOWN:
        return "ERR_UNKNOWN";
    }
}