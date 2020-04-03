/*
Ricobot (C) Copyright 2004, Wei Mingzhi
All rights reserved.

Redistribution and use in source and binary forms with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. 

3. Neither the name of the author nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY WEI MINGZHI "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// Ricobot - a bot example for Valve Software's Ricochet MOD
//
// bot.h
//

#ifndef BOT_H
#define BOT_H

#pragma warning (disable : 4710) // function not inlined

#include <string.h>
#include <ctype.h>

#include "extdll.h"
#include "dllapi.h"
#include "h_export.h"
#include "meta_api.h"
#include "enginecallback.h"
#include "cbase.h"
#include "entity_state.h"

#define BOT_PITCH_SPEED 20
#define BOT_YAW_SPEED 20

#define RESPAWN_IDLE             1
#define RESPAWN_NEED_TO_RESPAWN  2
#define RESPAWN_IS_RESPAWNING    3

#define BOT_SKIN_LEN 32
#define BOT_NAME_LEN 32

#define MAX_BOT_WHINE 100

typedef struct
{
   bool is_used;
   int respawn_state;
   edict_t *pEdict;
   bool need_to_initialize;
   char name[BOT_NAME_LEN+1];
   char skin[BOT_SKIN_LEN+1];
   int bot_skill;
   int not_started;
   float kick_time;
   float create_time;

   //Fix by Cheeseh (RCBot)
   //int msecnum;
   //float msecdel;
   //float msecval;

   float fUpdateTime;
   float fLastRunPlayerMoveTime;
	
   float f_max_speed;

   edict_t *pBotEnemy;
   float f_bot_see_enemy_time;
   float f_bot_find_enemy_time;
   edict_t *killer_edict;
   bool  b_bot_say_killed;
   float f_bot_say_killed;

   float f_move_speed;
   float f_sidemove_speed;

   int disc_number;
   int m_iPowerups;

   Vector vecLookAt;
   Vector vecTargetPos;
} bot_t;

extern edict_t *listenserver_edict;

extern int default_bot_skill;
extern char bot_whine[MAX_BOT_WHINE][81];
extern int whine_count;

extern int recent_bot_whine[5];

extern bot_t bots[32];

extern enginefuncs_t g_engfuncs;
extern globalvars_t  *gpGlobals;
extern bot_t bots[32];
extern bool b_observer_mode;
extern bool b_botdontshoot;
extern int bot_chat_percent;

#define MAX_BOT_NAMES 100

// Ricochet specific
// Powerups
#define POW_TRIPLE (1<<0)
#define POW_FAST   (1<<1)
#define POW_HARD   (1<<2)
#define POW_FREEZE (1<<3)

#include "bot_func.h"

#endif // BOT_H

