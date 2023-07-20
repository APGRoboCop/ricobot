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
// dll.cpp
//

#include "bot.h"

char welcome_msg[] = "Welcome to Ricobot by Wei Mingzhi\n"
                     "Visit http://yapb.bots-united.com\n"
                     "Compile date: " __DATE__ "\n";

DLL_GLOBAL const Vector g_vecZero = Vector(0,0,0);

int default_bot_skill = 2;
float bot_check_time = 30.0;
int min_bots = -1;
int max_bots = -1;
int num_bots = 0;
int prev_num_bots = 0;
bool g_GameRules = FALSE;
edict_t *listenserver_edict = nullptr;
float welcome_time = 0.0;
bool welcome_sent = FALSE;

FILE *bot_cfg_fp = nullptr;
bool need_to_open_cfg = TRUE;
float bot_cfg_pause_time = 0.0;
float respawn_time = 0.0;
bool spawn_time_reset = FALSE;

char bot_whine[MAX_BOT_WHINE][81];
int whine_count;
int recent_bot_whine[5];

void BotNameInit();
void ProcessBotCfgFile();

void ServerCommand();

void GameDLLInit()
{
   char filename[256];
   char buffer[256];
   int i, length;
   FILE *bfp;
   char *ptr;

   // Register server command
   (*g_engfuncs.pfnAddServerCommand)("bot", ServerCommand);

   whine_count = 0;

   // initialize the bots array of structures...
   memset(bots, 0, sizeof(bots));

   for (i=0; i < 5; i++)
      recent_bot_whine[i] = -1;

   BotNameInit();

   UTIL_BuildFileName(filename, "bot_whine.txt", nullptr);

   bfp = fopen(filename, "r");

   if (bfp != nullptr)
   {
      while ((whine_count < MAX_BOT_WHINE) &&
             (fgets(buffer, 80, bfp) != nullptr))
      {
         length = strlen(buffer);

         if (buffer[length-1] == '\n')
         {
            buffer[length-1] = 0;  // remove '\n'
            length--;
         }

         if ((ptr = strstr(buffer, "%n")) != nullptr)
         {
            *(ptr+1) = 's';  // change %n to %s
         }

         if (length > 0)
         {
            strcpy(bot_whine[whine_count], buffer);
            whine_count++;
         }
      }

      fclose(bfp);
   }
}

gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;
meta_globals_t *gpMetaGlobals;

META_FUNCTIONS gMetaFunctionTable = {
   GetEntityAPI, nullptr, nullptr, nullptr, nullptr,
   nullptr, GetEngineFunctions, nullptr
};

plugin_info_t Plugin_info = { META_INTERFACE_VERSION, "Ricobot", "1.2", __DATE__, "Wei Mingzhi",
    "http://yapb.bots-united.com", "RICOBOT", PT_STARTUP, PT_ANYTIME };

// Metamod requesting info about this plugin
C_DLLEXPORT int Meta_Query(char *, plugin_info_t **pPlugInfo,
                           mutil_funcs_t *pMetaUtilFuncs) 
{
   *pPlugInfo = &Plugin_info; // Give metamod our plugin_info struct
   gpMetaUtilFuncs = pMetaUtilFuncs; // Get metamod utility function table.
   return TRUE;
}

// Metamod attaching plugin to the server
C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME,
   META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, 
   gamedll_funcs_t *pGamedllFuncs) 
{
   if (!pMGlobals)
   {
      LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
      return FALSE;
   }

   gpMetaGlobals = pMGlobals;

   if (!pFunctionTable)
   {
      LOG_ERROR(PLID, "Meta_Attach called with null pFunctionTable");
      return FALSE;
   }

   memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
   gpGamedllFuncs = pGamedllFuncs;

   GameDLLInit();

   return TRUE;
}

// Metamod detaching plugin from the server
C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME, PL_UNLOAD_REASON) 
{
   return TRUE;
}

void ServerCommand()
{
   if (strcmp(CMD_ARGV(1), "addbot") == 0)
   {
      BotCreate(nullptr, CMD_ARGV(2), CMD_ARGV(3) );
      bot_check_time = gpGlobals->time + 5.0f;
   }
   else if (strcmp(CMD_ARGV(1), "min_bots") == 0)
   {
      min_bots = atoi( CMD_ARGV(2) );

      if ((min_bots < 0) || (min_bots > 31))
         min_bots = 1;

      printf("min_bots set to %d\n", min_bots);
   }
   else if (strcmp(CMD_ARGV(1), "max_bots") == 0)
   {
      max_bots = atoi( CMD_ARGV(2) );

      if ((max_bots < 0) || (max_bots > 31)) 
         max_bots = 1;

      printf("max_bots set to %d\n", max_bots);
   }
}

int DispatchSpawn( edict_t *pent )
{
   if (gpGlobals->deathmatch)
   {
	   const char *pClassname = const_cast<char *>(STRING(pent->v.classname));

      if (strcmp(pClassname, "worldspawn") == 0)
      {
         g_GameRules = TRUE;

         bot_cfg_pause_time = 0.0;
         respawn_time = 0.0;
         spawn_time_reset = FALSE;

         prev_num_bots = num_bots;
         num_bots = 0;

         bot_check_time = gpGlobals->time + 30.0f;
      }
   }

   RETURN_META_VALUE(MRES_IGNORED, 0);
}

BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{ 
   if (gpGlobals->deathmatch)
   {
      int i;
      int count = 0;

      // check if this client is the listen server client
      if (strcmp(pszAddress, "loopback") == 0)
         listenserver_edict = pEntity; // save the edict of the listen server client...

      // check if this is NOT a bot joining the server...
      if (strcmp(pszAddress, "127.0.0.1") != 0)
      {
         // don't try to add bots for 60 seconds, give client time to get added
         bot_check_time = gpGlobals->time + 60.0f;

         for (i = 0; i < 32; i++)
         {
            if (bots[i].is_used)  // count the number of bots in use
               count++;
         }

         // if there are currently more than the minimum number of bots running
         // then kick one of the bots off the server...
         if (count > min_bots && min_bots != -1)
         {
            for (i=0; i < 32; i++)
            {
               if (bots[i].is_used)  // is this slot used?
               {
                  char cmd[80];
                  sprintf(cmd, "kick \"%s\"\n", STRING(bots[i].pEdict->v.netname));
                  SERVER_COMMAND(cmd);  // kick the bot using (kick "name")
                  break;
               }
            }
         }
      }
   }

   RETURN_META_VALUE(MRES_IGNORED, 0);
}

void ClientDisconnect( edict_t *pEntity )
{
   if (gpGlobals->deathmatch)
   {
      for (int i = 0; i < 32; i++)
      {
         if (bots[i].pEdict == pEntity)
         {
            // someone kicked this bot off of the server...
            bots[i].is_used = FALSE;  // this slot is now free to use
            bots[i].kick_time = gpGlobals->time;  // save the kicked time
            break;
         }
      }
   }

   RETURN_META(MRES_IGNORED);
}

void ClientCommand( edict_t *pEntity )
{
   // only allow custom commands if deathmatch mode and NOT dedicated server and
   // client sending command is the listen server client...

   if ((gpGlobals->deathmatch) && (!IS_DEDICATED_SERVER()) &&
       (pEntity == listenserver_edict))
   {
      const char *pcmd = CMD_ARGV(0);
      const char *arg1 = CMD_ARGV(1);
      const char *arg2 = CMD_ARGV(2);
      char msg[80];

      if (FStrEq(pcmd, "addbot"))
      {
         BotCreate( pEntity, arg1, arg2 );

         bot_check_time = gpGlobals->time + 5.0f;

         RETURN_META(MRES_SUPERCEDE);
      }
      else if (FStrEq(pcmd, "observer"))
      {
         if ((arg1 != nullptr) && (*arg1 != 0))
         {
	         const int temp = atoi(arg1);
            if (temp)
               b_observer_mode = TRUE;
            else
               b_observer_mode = FALSE;
         }

         if (b_observer_mode)
            ClientPrint(pEntity, HUD_PRINTNOTIFY, "observer mode ENABLED\n");
         else
            ClientPrint(pEntity, HUD_PRINTNOTIFY, "observer mode DISABLED\n");

         RETURN_META(MRES_SUPERCEDE);
      }
      else if (FStrEq(pcmd, "botskill"))
      {
         if ((arg1 != nullptr) && (*arg1 != 0))
         {
	         const int temp = atoi(arg1);

            if (temp < 1 || temp > 5)
               ClientPrint(pEntity, HUD_PRINTNOTIFY, "invalid botskill value!\n");
            else
               default_bot_skill = temp;
         }

         sprintf(msg, "botskill is %d\n", default_bot_skill);
         ClientPrint(pEntity, HUD_PRINTNOTIFY, msg);

         RETURN_META(MRES_SUPERCEDE);
      }
      else if (FStrEq(pcmd, "botdontshoot"))
      {
         if ((arg1 != nullptr) && (*arg1 != 0))
         {
	         const int temp = atoi(arg1);
            if (temp)
               b_botdontshoot = TRUE;
            else
               b_botdontshoot = FALSE;
         }

         if (b_botdontshoot)
            ClientPrint(pEntity, HUD_PRINTNOTIFY, "botdontshoot ENABLED\n");
         else
            ClientPrint(pEntity, HUD_PRINTNOTIFY, "botdontshoot DISABLED\n");

         RETURN_META(MRES_SUPERCEDE);
      }
      else if (FStrEq(pcmd, "bot_chat_percent"))
      {
         if ((arg1 != nullptr) && (*arg1 != 0))
         {
	         const int temp = atoi(arg1);

            if ((temp < 0) || (temp > 100))
               ClientPrint(pEntity, HUD_PRINTNOTIFY, "invalid bot_chat_percent value!\n");
            else
               bot_chat_percent = temp;
         }

         sprintf(msg, "bot_chat_percent is %d\n", bot_chat_percent);
         ClientPrint(pEntity, HUD_PRINTNOTIFY, msg);

         RETURN_META(MRES_SUPERCEDE);
      }
   }

   RETURN_META(MRES_IGNORED);
}

void StartFrame()
{
   if (gpGlobals->deathmatch)
   {
      static int i, index, bot_index;
      static float previous_time = -1.0;
      int count;

      // if a new map has started then (MUST BE FIRST IN StartFrame)...
      if (gpGlobals->time + 0.1 < previous_time)
      {
         char filename[256];
         char mapname[64];

         // check if mapname_bot.cfg file exists...
         strcpy(mapname, STRING(gpGlobals->mapname));
         strcat(mapname, "_bot.cfg");

         UTIL_BuildFileName(filename, "maps", mapname);

         if ((bot_cfg_fp = fopen(filename, "r")) != nullptr)
         {
            for (index = 0; index < 32; index++)
            {
               bots[index].is_used = FALSE;
               bots[index].respawn_state = 0;
               bots[index].kick_time = 0.0;
            }

            if (IS_DEDICATED_SERVER())
               bot_cfg_pause_time = gpGlobals->time + 5.0f;
            else
               bot_cfg_pause_time = gpGlobals->time + 20.0f;
         }
         else
         {
            count = 0;

            // mark the bots as needing to be respawned...
            for (index = 0; index < 32; index++)
            {
               if (count >= prev_num_bots)
               {
                  bots[index].is_used = FALSE;
                  bots[index].respawn_state = 0;
                  bots[index].kick_time = 0.0;
               }

               if (bots[index].is_used)  // is this slot used?
               {
                  bots[index].respawn_state = RESPAWN_NEED_TO_RESPAWN;
                  count++;
               }

               // check for any bots that were very recently kicked...
               if ((bots[index].kick_time + 5.0f) > previous_time)
               {
                  bots[index].respawn_state = RESPAWN_NEED_TO_RESPAWN;
                  count++;
               }
               else
                  bots[index].kick_time = 0.0;  // reset to prevent false spawns later
            }

            // set the respawn time
            if (IS_DEDICATED_SERVER())
               respawn_time = gpGlobals->time + 5.0f;
            else
               respawn_time = gpGlobals->time + 20.0f;
         }

         bot_check_time = gpGlobals->time + 30.0f;
      }

      if (!IS_DEDICATED_SERVER())
      {
         if ((listenserver_edict != nullptr) && (welcome_sent == FALSE) &&
             (welcome_time < 1.0f))
         {
            // are they out of observer mode yet?
            if (IsAlive(listenserver_edict))
               welcome_time = gpGlobals->time + 5.0f;  // welcome in 5 seconds
         }

         if ((welcome_time > 0.0) && (welcome_time < gpGlobals->time) &&
             (welcome_sent == FALSE))
         {
            // send the welcome message to this client
            MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, nullptr, listenserver_edict);
            WRITE_BYTE(TE_TEXTMESSAGE);
            WRITE_BYTE(1);
            WRITE_SHORT(-1);
            WRITE_SHORT(0);
            WRITE_BYTE(0);
            WRITE_BYTE(RANDOM_LONG(10, 255));
            WRITE_BYTE(RANDOM_LONG(10, 255));
            WRITE_BYTE(RANDOM_LONG(10, 255));
            WRITE_BYTE(1);
            WRITE_BYTE(RANDOM_LONG(10, 255));
            WRITE_BYTE(RANDOM_LONG(10, 255));
            WRITE_BYTE(RANDOM_LONG(10, 255));
            WRITE_BYTE(1);
            WRITE_SHORT(256);
            WRITE_SHORT(512);
            WRITE_SHORT(512);
            WRITE_STRING(welcome_msg);
            MESSAGE_END();
            SERVER_COMMAND("speak barney/guyresponsible\n");
            welcome_sent = TRUE;  // clear this so we only do it once
         }
      }

      count = 0;

      for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
      {
         if ((bots[bot_index].is_used) &&  // is this slot used AND
             (bots[bot_index].respawn_state == RESPAWN_IDLE))  // not respawning
         {
            BotThink(&bots[bot_index]);

            count++;
         }
      }

      if (count > num_bots)
         num_bots = count;

      // are we currently respawning bots and is it time to spawn one yet?
      if (respawn_time > 1.0f && respawn_time <= gpGlobals->time)
      {
         int index = 0;

         // find bot needing to be respawned...
         while (index < 32 &&
                bots[index].respawn_state != RESPAWN_NEED_TO_RESPAWN)
            index++;

         if (index < 32)
         {
            bots[index].respawn_state = RESPAWN_IS_RESPAWNING;
            bots[index].is_used = FALSE;      // free up this slot

            // respawn 1 bot then wait a while (otherwise engine crashes)
            char c_skill[2];
            sprintf(c_skill, "%d", bots[index].bot_skill);
            BotCreate(nullptr, bots[index].name, c_skill);

            respawn_time = gpGlobals->time + 2.0f;  // set next respawn time

            bot_check_time = gpGlobals->time + 5.0f;
         }
         else
            respawn_time = 0.0;
      }

      if (g_GameRules)
      {
         if (need_to_open_cfg)  // have we open bot.cfg file yet?
         {
            char filename[256];
            char mapname[64];

            need_to_open_cfg = FALSE;  // only do this once!!!

            // check if mapname_bot.cfg file exists...

            strcpy(mapname, STRING(gpGlobals->mapname));
            strcat(mapname, "_bot.cfg");

            UTIL_BuildFileName(filename, "maps", mapname);

            if ((bot_cfg_fp = fopen(filename, "r")) == nullptr)
            {
               UTIL_BuildFileName(filename, "bot.cfg", nullptr);
               bot_cfg_fp = fopen(filename, "r");
            }

            if (IS_DEDICATED_SERVER())
               bot_cfg_pause_time = gpGlobals->time + 5.0f;
            else
               bot_cfg_pause_time = gpGlobals->time + 20.0f;
         }

         if (!IS_DEDICATED_SERVER() && !spawn_time_reset)
         {
            if (listenserver_edict != nullptr)
            {
               if (IsAlive(listenserver_edict))
               {
                  spawn_time_reset = TRUE;

                  if (respawn_time >= 1.0)
                     respawn_time = fmin(respawn_time, gpGlobals->time + 1.0);

                  if (bot_cfg_pause_time >= 1.0)
                     bot_cfg_pause_time = fmin(bot_cfg_pause_time, gpGlobals->time + 1.0);
               }
            }
         }

         if ((bot_cfg_fp) &&
             (bot_cfg_pause_time >= 1.0f) && (bot_cfg_pause_time <= gpGlobals->time))
         {
            // process bot.cfg file options...
            ProcessBotCfgFile();
         }
      }

      // check if time to see if a bot needs to be created...
      if (bot_check_time < gpGlobals->time)
      {
         int count = 0;
         bot_check_time = gpGlobals->time + 5.0f;

         for (i = 1; i <= gpGlobals->maxClients; i++)
         {
	         const edict_t *pPlayer = INDEXENT(i);
            if (!FNullEnt(pPlayer) && !pPlayer->free &&
               (pPlayer->v.flags & (FL_CLIENT | FL_FAKECLIENT)))
               count++;
         }

         // if there are currently less than the maximum number of "players"
         // then add another bot using the default skill level...
         if (count < max_bots && max_bots != -1)
            BotCreate(nullptr, nullptr, nullptr);
      }

      previous_time = gpGlobals->time;
   }

   RETURN_META(MRES_IGNORED);
}

C_DLLEXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
   memset(pFunctionTable, 0, sizeof(DLL_FUNCTIONS));

   pFunctionTable->pfnSpawn = DispatchSpawn;
   pFunctionTable->pfnClientConnect = ClientConnect;
   pFunctionTable->pfnClientDisconnect = ClientDisconnect;
   pFunctionTable->pfnClientCommand = ClientCommand;
   pFunctionTable->pfnStartFrame = StartFrame;

   return TRUE;
}

void ProcessBotCfgFile()
{
   int ch;
   char cmd_line[256];
   int cmd_index;
   static char server_cmd[80];
   char *cmd, *arg1, *arg2;
   char msg[80];

   if (bot_cfg_pause_time > gpGlobals->time)
      return;

   if (bot_cfg_fp == nullptr)
      return;

   cmd_index = 0;
   cmd_line[cmd_index] = 0;

   ch = fgetc(bot_cfg_fp);

   // skip any leading blanks
   while (ch == ' ')
      ch = fgetc(bot_cfg_fp);

   while ((ch != EOF) && (ch != '\r') && (ch != '\n'))
   {
      if (ch == '\t')  // convert tabs to spaces
         ch = ' ';

      cmd_line[cmd_index] = ch;

      ch = fgetc(bot_cfg_fp);

      // skip multiple spaces in input file
      while ((cmd_line[cmd_index] == ' ') &&
             (ch == ' '))      
         ch = fgetc(bot_cfg_fp);

      cmd_index++;
   }

   if (ch == '\r')  // is it a carriage return?
   {
      ch = fgetc(bot_cfg_fp);  // skip the linefeed
   }

   // if reached end of file, then close it
   if (ch == EOF)
   {
      fclose(bot_cfg_fp);

      bot_cfg_fp = nullptr;

      bot_cfg_pause_time = 0.0;
   }

   cmd_line[cmd_index] = 0;  // terminate the command line

   // copy the command line to a server command buffer...
   strcpy(server_cmd, cmd_line);
   strcat(server_cmd, "\n");

   cmd_index = 0;
   cmd = cmd_line;
   arg1 = arg2 = nullptr;

   // skip to blank or end of string...
   while ((cmd_line[cmd_index] != ' ') && (cmd_line[cmd_index] != 0))
      cmd_index++;

   if (cmd_line[cmd_index] == ' ')
   {
      cmd_line[cmd_index++] = 0;
      arg1 = &cmd_line[cmd_index];

      // skip to blank or end of string...
      while ((cmd_line[cmd_index] != ' ') && (cmd_line[cmd_index] != 0))
         cmd_index++;

      if (cmd_line[cmd_index] == ' ')
      {
         cmd_line[cmd_index++] = 0;
         arg2 = &cmd_line[cmd_index];
      }
   }

   if (cmd_line[0] == '#' || cmd_line[0] == 0)
      return;  // return if comment or blank line

   if (strcmp(cmd, "addbot") == 0)
   {
      BotCreate(nullptr, arg1, arg2 );

      // have to delay here or engine gives "Tried to write to
      // uninitialized sizebuf_t" error and crashes...

      bot_cfg_pause_time = gpGlobals->time + 2.0f;
      bot_check_time = gpGlobals->time + 5.0f;

      return;
   }

   if (strcmp(cmd, "botskill") == 0)
   {
	   const int temp = atoi(arg1);

      if ((temp >= 1) && (temp <= 5))
         default_bot_skill = atoi( arg1 );  // set default bot skill level

      return;
   }

   if (strcmp(cmd, "observer") == 0)
   {
	   const int temp = atoi(arg1);

      if (temp)
         b_observer_mode = TRUE;
      else
         b_observer_mode = FALSE;

      return;
   }

   if (strcmp(cmd, "botdontshoot") == 0)
   {
	   const int temp = atoi(arg1);

      if (temp)
         b_botdontshoot = TRUE;
      else
         b_botdontshoot = FALSE;

      return;
   }

   if (strcmp(cmd, "min_bots") == 0)
   {
      min_bots = atoi( arg1 );

      if ((min_bots < 0) || (min_bots > 31))
         min_bots = 1;

      if (IS_DEDICATED_SERVER())
      {
         sprintf(msg, "min_bots set to %d\n", min_bots);
         printf(msg);
      }

      return;
   }

   if (strcmp(cmd, "max_bots") == 0)
   {
      max_bots = atoi( arg1 );

      if ((max_bots < 0) || (max_bots > 31)) 
         max_bots = 1;

      if (IS_DEDICATED_SERVER())
      {
         sprintf(msg, "max_bots set to %d\n", max_bots);
         printf(msg);
      }

      return;
   }

   if (strcmp(cmd, "pause") == 0)
   {
      bot_cfg_pause_time = gpGlobals->time + atoi( arg1 );
      return;
   }

   if (strcmp(cmd, "bot_chat_percent") == 0)
   {
	   const int temp = atoi(arg1);

      if ((temp >= 0) && (temp <= 100))
         bot_chat_percent = atoi( arg1 );  // set bot chat percent

      return;
   }

   SERVER_COMMAND(server_cmd);
}

