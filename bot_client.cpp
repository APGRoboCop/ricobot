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
// bot_client.cpp
//

#include "bot.h"

// This message gets sent when the bots get killed
void BotClient_Ricochet_DeathMsg(void *p, int bot_index)
{
   static int state = 0;
   static int killer_index;
   static int victim_index;
   static edict_t *victim_edict;
   static int index;

   if (state == 0)
   {
      state++;
      killer_index = *static_cast<int *>(p);  // ENTINDEX() of killer
   }
   else if (state == 1)
   {
      state++;
      victim_index = *static_cast<int *>(p);  // ENTINDEX() of victim
   }
   else if (state == 2)
   {
      state = 0;

      victim_edict = INDEXENT(victim_index);
      index = UTIL_GetBotIndex(victim_edict);

      // is this message about a bot being killed?
      if (index != -1)
      {
         // bot killed by world (worldspawn) or killed self?
         if (killer_index == 0 || killer_index == victim_index)
            bots[index].killer_edict = nullptr;
         else // store edict of player that killed this bot...
            bots[index].killer_edict = INDEXENT(killer_index);
      }
   }
}

// This message is sent whenever ammo amounts are adjusted (up or down).
void BotClient_Ricochet_AmmoX(void *p, int bot_index)
{
   static int state = 0;
   if (state == 0)
      state++;
   else if (state == 1)
   {
      state = 0;
      bots[bot_index].disc_number = *static_cast<int *>(p);  // store it away
   }
}

// This message is sent whenever a player gets or loses powerup(s).
void BotClient_Ricochet_Powerup(void *p, int bot_index)
{
   bots[bot_index].m_iPowerups = *static_cast<int *>(p);
}
