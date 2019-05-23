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
// engine.cpp
//

#include "bot.h"

void (*botMsgFunction)(void *, int) = NULL;
void (*botMsgEndFunction)(void *, int) = NULL;
int botMsgIndex;

void pfnClientCommand(edict_t* pEdict, char* szFmt, ...)
{
   if (!(pEdict->v.flags & FL_FAKECLIENT))
      RETURN_META(MRES_IGNORED);
   RETURN_META(MRES_SUPERCEDE);
}

void pfnMessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   if (gpGlobals->deathmatch)
   {
      botMsgFunction = NULL;  // no msg function until known otherwise
      botMsgIndex = UTIL_GetBotIndex(ed); // index of bot receiving message

      if (msg_type == GET_USER_MSG_ID(PLID, "DeathMsg", NULL))
         botMsgFunction = BotClient_Ricochet_DeathMsg;
      else if (msg_type == GET_USER_MSG_ID(PLID, "AmmoX", NULL))
         botMsgFunction = BotClient_Ricochet_AmmoX;
      else if (msg_type == GET_USER_MSG_ID(PLID, "Powerup", NULL))
         botMsgFunction = BotClient_Ricochet_Powerup;
   }

   RETURN_META(MRES_IGNORED);
}

void pfnMessageEnd(void)
{
   if (gpGlobals->deathmatch)
   {
      if (botMsgEndFunction)
         (*botMsgEndFunction)(NULL, botMsgIndex);  // NULL indicated msg end

      // clear out the bot message function pointers...
      botMsgFunction = NULL;
      botMsgEndFunction = NULL;
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteByte(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&iValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteChar(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&iValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteShort(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&iValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteLong(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&iValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteAngle(float flValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&flValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteCoord(float flValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&flValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteString(const char *sz)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)((void *)sz, botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

void pfnWriteEntity(int iValue)
{
   if (gpGlobals->deathmatch)
   {
      // if this message is for a bot, call the client message function...
      if (botMsgFunction)
         (*botMsgFunction)(static_cast<void *>(&iValue), botMsgIndex);
   }

   RETURN_META(MRES_IGNORED);
}

C_DLLEXPORT int GetEngineFunctions(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
   memset(pengfuncsFromEngine, 0, sizeof(enginefuncs_t));

   pengfuncsFromEngine->pfnClientCommand = pfnClientCommand;
   pengfuncsFromEngine->pfnMessageBegin = pfnMessageBegin;
   pengfuncsFromEngine->pfnMessageEnd = pfnMessageEnd;
   pengfuncsFromEngine->pfnWriteByte = pfnWriteByte;
   pengfuncsFromEngine->pfnWriteChar = pfnWriteChar;
   pengfuncsFromEngine->pfnWriteShort = pfnWriteShort;
   pengfuncsFromEngine->pfnWriteLong = pfnWriteLong;
   pengfuncsFromEngine->pfnWriteAngle = pfnWriteAngle;
   pengfuncsFromEngine->pfnWriteCoord = pfnWriteCoord;
   pengfuncsFromEngine->pfnWriteString = pfnWriteString;
   pengfuncsFromEngine->pfnWriteEntity = pfnWriteEntity;

   return TRUE;
}

