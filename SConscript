'''
Author: your name
Date: 2021-05-25 14:57:35
LastEditTime: 2021-06-04 14:31:00
LastEditors: Please set LastEditors
Description: In User Settings Edit
FilePath: \gd32f407-sbrgate\third-packages\mp3player\SConscript
'''
Import('rtconfig')
from building import *

cwd = GetCurrentDir()

path = [cwd,
    cwd + '/inc']

src = []
src +=  Split('''
        src/mp3_player.c
        src/mp3_player_cmd.c
        src/mp3_tag.c
        ''')

group = DefineGroup('mp3player', src, depend = ['PKG_USING_MP3PLAYER'], CPPPATH = path)

Return('group')
