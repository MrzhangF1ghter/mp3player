/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2021-06-02     MrzhangF1ghter    first implementation
 * 2022-10-27     MrzhangF1ghter    fix warning
 */

#include "mp3_player.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <optparse.h>
#include <mp3_player.h>

#include <stdlib.h>

enum MP3_PLAYER_ACTTION
{
    MP3_PLAYER_ACTION_HELP = 0,
    MP3_PLAYER_ACTION_START = 1,
    MP3_PLAYER_ACTION_STOP = 2,
    MP3_PLAYER_ACTION_PAUSE = 3,
    MP3_PLAYER_ACTION_RESUME = 4,
    MP3_PLAYER_ACTION_VOLUME = 5,
    MP3_PLAYER_ACTION_DUMP = 6,
    MP3_PLAYER_ACTION_JUMP = 7
};

struct mp3_play_args
{
    int action;
    char *uri;
    int volume;
    int seconds;
};

static const char *state_str[] =
    {
        "STOPPED",
        "PLAYING",
        "PAUSED",
};

static struct optparse_long opts[] =
    {
        {"help", 'h', OPTPARSE_NONE},
        {"start", 's', OPTPARSE_REQUIRED},
        {"stop", 't', OPTPARSE_NONE},
        {"pause", 'p', OPTPARSE_NONE},
        {"resume", 'r', OPTPARSE_NONE},
        {"volume", 'v', OPTPARSE_REQUIRED},
        {"dump", 'd', OPTPARSE_NONE},
        {"jump", 'j', OPTPARSE_REQUIRED},
        {NULL, 0, OPTPARSE_NONE}};

static void usage(void)
{
    rt_kprintf("usage: mp3_play [option] [target] ...\n\n");
    rt_kprintf("usage options:\n");
    rt_kprintf("  -h,     --help                     Print defined help message.\n");
    rt_kprintf("  -s URI, --start=URI                Play mp3 music with URI(local files).\n");
    rt_kprintf("  -t,     --stop                     Stop playing music.\n");
    rt_kprintf("  -p,     --pause                    Pause the music.\n");
    rt_kprintf("  -r,     --resume                   Resume the music.\n");
    rt_kprintf("  -v lvl, --volume=lvl               Change the volume(0~99).\n");
    rt_kprintf("  -d,     --dump                     Dump play relevant information.\n");
    rt_kprintf("  -j,     --jump                     Jump to seconds that given.\n");
}

static void dump_status(void)
{
    rt_kprintf("\nmp3_player status:\n");
    rt_kprintf("uri     - %s\n", mp3_player_uri_get());
    rt_kprintf("status  - %s\n", state_str[mp3_player_state_get()]);
    rt_kprintf("volume  - %d\n", mp3_player_volume_get());
    mp3_disp_time();
    mp3_info_show();
}

int mp3_play_args_prase(int argc, char *argv[], struct mp3_play_args *play_args)
{
    int ch;
    int option_index;
    struct optparse options;
    rt_uint8_t action_cnt = 0;
    rt_err_t result = RT_EOK;

    if (argc == 1)
    {
        play_args->action = MP3_PLAYER_ACTION_HELP;
        return RT_EOK;
    }

    /* Parse cmd */
    optparse_init(&options, argv);
    while ((ch = optparse_long(&options, opts, &option_index)) != -1)
    {
        switch (ch)
        {
        case 'h':
            play_args->action = MP3_PLAYER_ACTION_HELP;
            break;

        case 's':
            play_args->action = MP3_PLAYER_ACTION_START;
            play_args->uri = options.optarg;
            action_cnt++;
            break;

        case 't':
            play_args->action = MP3_PLAYER_ACTION_STOP;
            action_cnt++;
            break;

        case 'p':
            play_args->action = MP3_PLAYER_ACTION_PAUSE;
            action_cnt++;
            break;

        case 'r':
            play_args->action = MP3_PLAYER_ACTION_RESUME;
            action_cnt++;
            break;

        case 'v':
            play_args->action = MP3_PLAYER_ACTION_VOLUME;
            play_args->volume = (options.optarg == RT_NULL) ? (-1) : atoi(options.optarg);
            action_cnt++;
            break;

        case 'd':
            play_args->action = MP3_PLAYER_ACTION_DUMP;
            break;

        case 'j':
            play_args->action = MP3_PLAYER_ACTION_JUMP;
            play_args->seconds = (options.optarg == RT_NULL) ? (-1) : atoi(options.optarg);
            break;

        default:
            result = -RT_EINVAL;
            break;
        }
    }

    if (action_cnt > 1)
    {
        rt_kprintf("START STOP PAUSE RESUME parameter can't be used at the same time.\n");
        result = -RT_EINVAL;
    }

    return result;
}

int mp3_player(int argc, char *argv[])
{
    int result = RT_EOK;
    struct mp3_play_args play_args = {0};

    result = mp3_play_args_prase(argc, argv, &play_args);
    if (result != RT_EOK)
    {
        usage();
        return result;
    }

    switch (play_args.action)
    {
    case MP3_PLAYER_ACTION_HELP:
        usage();
        break;

    case MP3_PLAYER_ACTION_START:
        mp3_player_play(play_args.uri);
        break;

    case MP3_PLAYER_ACTION_STOP:
        mp3_player_stop();
        break;

    case MP3_PLAYER_ACTION_PAUSE:
        mp3_player_pause();
        break;

    case MP3_PLAYER_ACTION_RESUME:
        mp3_player_resume();
        break;

    case MP3_PLAYER_ACTION_VOLUME:
        mp3_player_volume_set(play_args.volume);
        break;

    case MP3_PLAYER_ACTION_DUMP:
        dump_status();
        break;

    case MP3_PLAYER_ACTION_JUMP:
        mp3_seek(play_args.seconds);
        break;
    default:
        result = -RT_ERROR;
        break;
    }

    return result;
}

MSH_CMD_EXPORT_ALIAS(mp3_player, mp3play, play mp3 music);
