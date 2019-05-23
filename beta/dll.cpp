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

DLL_GLOBAL const Vector g_vecZero = Vector(0, 0, 0);

int default_bot_skill = 2;
float bot_check_time = 30.0;
int min_bots = -1;
int max_bots = -1;
int num_bots = 0;
int prev_num_bots = 0;

static int bot_total_varies = 0;
static bool bot_bot_balance = FALSE;
bool g_GameRules = FALSE;
edict_t* clients[32];
edict_t* listenserver_edict = NULL;

float welcome_time = 0.0;
bool welcome_sent = FALSE;

FILE* bot_cfg_fp = NULL;
bool need_to_open_cfg = TRUE;
float bot_cfg_pause_time = 0.0;
static bool need_to_open_cfg2 = FALSE;
static int cfg_file = 1;

float respawn_time = 0.0;
bool spawn_time_reset = FALSE;

// this tracks the number of bots "wanting" to play on the server
// it should never be less than min_bots or more than max_bots
static int interested_bots = -1;

// define the sources that a bot option/setting can be changed from
// used primarily by the changeBotSetting() function
enum {
    SETTING_SOURCE_CLIENT_COMMAND, // command issued at console by host on Listen server
    SETTING_SOURCE_SERVER_COMMAND, // command issued to the server directly
    SETTING_SOURCE_CONFIG_FILE
}; // command issued by a config file

static void ProcessBotCfgFile(void);
static void kickBots(int totalToKick, const int team);
static void kickRandomBot(void);

void BotNameInit();
void ServerCommand();

// If bot_total_varies is active then this function will periodically alter
// the number of bots that want to play on the server.
static void varyBotTotal(void)
{
    if(bot_total_varies == 0)
        return; // just in case

    // this governs when the number of bots wanting to play will next change
    static float f_interested_bots_change = 0;

    if(f_interested_bots_change < gpGlobals->time) {
        if(bot_total_varies == 3) // busy server(players coming/going often)
            f_interested_bots_change = gpGlobals->time + random_float(10.0f, 120.0f);
        else if(bot_total_varies == 2)
            f_interested_bots_change = gpGlobals->time + random_float(40.0f, 360.0f);
        else // slow changes in number of bots
            f_interested_bots_change = gpGlobals->time + random_float(90.0f, 600.0f);

        //	UTIL_BotLogPrintf("interested_bots:%d, time:%f, next change:%f\n",
        //	interested_bots, gpGlobals->time, f_interested_bots_change);

        // try and get some bots interested in joining the
        // game if the game has just started
        if(interested_bots < 0) {
            if(max_bots > 0 && max_bots > min_bots) {
                if(min_bots > -1)
                    interested_bots = random_long(min_bots, max_bots);
                else
                    interested_bots = random_long(1, max_bots);
            } else
                interested_bots = min_bots;
        }

        // randomly increase/decrease the number of interested bots
        if(max_bots > 0 && max_bots > min_bots) {
            if(random_long(1, 1000) > 500) {
                // favor increasing the bots
                // if max_bots is reached decrease the bots
                if(interested_bots < max_bots)
                    ++interested_bots;
                else if(min_bots > 0) {
                    if(interested_bots > min_bots)
                        --interested_bots;
                } else if(interested_bots > 0)
                    --interested_bots;
            } else {
                // favor decreasing the bots
                // if min_bots is reached increase the bots
                if(min_bots > 0 && interested_bots > min_bots)
                    --interested_bots;
                else if(min_bots < 1 && interested_bots > 0)
                    --interested_bots;
                else if(interested_bots < max_bots)
                    ++interested_bots;
            }
        }

        //	UTIL_BotLogPrintf("interested_bots changed to:%d\n", interested_bots);
    } else {
        // make sure f_interested_bots_change is sane
        // (gpGlobals->time resets on map change)
        if((f_interested_bots_change - 601.0f) > gpGlobals->time)
            f_interested_bots_change = gpGlobals->time + random_float(10.0f, 120.0f);
    }
}

void GameDLLInit()
{
    char filename[256];
    char buffer[256];
    int i, length;
    FILE* bfp;
    char* ptr;

    // Register server command
    (*g_engfuncs.pfnAddServerCommand)("bot", ServerCommand);

    // initialize the bots array of structures...
    memset(bots, 0, sizeof(bots));

    BotNameInit();

    bfp = fopen(filename, "r");

    if(bfp != NULL) {
        fclose(bfp);
    }
}

gamedll_funcs_t* gpGamedllFuncs;
mutil_funcs_t* gpMetaUtilFuncs;
meta_globals_t* gpMetaGlobals;

META_FUNCTIONS gMetaFunctionTable = { GetEntityAPI, NULL, NULL, NULL, NULL, NULL, GetEngineFunctions, NULL };

plugin_info_t Plugin_info = { META_INTERFACE_VERSION, "Ricobot", "1.2", __DATE__, "Wei Mingzhi",
    "http://yapb.bots-united.com", "RICOBOT", PT_STARTUP, PT_ANYTIME };

// Metamod requesting info about this plugin
C_DLLEXPORT int Meta_Query(char*, plugin_info_t** pPlugInfo, mutil_funcs_t* pMetaUtilFuncs)
{
    *pPlugInfo = &Plugin_info;        // Give metamod our plugin_info struct
    gpMetaUtilFuncs = pMetaUtilFuncs; // Get metamod utility function table.
    return TRUE;
}

// Metamod attaching plugin to the server
C_DLLEXPORT int
Meta_Attach(PLUG_LOADTIME, META_FUNCTIONS* pFunctionTable, meta_globals_t* pMGlobals, gamedll_funcs_t* pGamedllFuncs)
{
    if(!pMGlobals) {
        LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
        return FALSE;
    }

    gpMetaGlobals = pMGlobals;

    if(!pFunctionTable) {
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

static void ProcessBotCfgFile(void)
{
    int ch;
    char cmd_line[512];
    int cmd_index;

    if(bot_cfg_pause_time > gpGlobals->time)
        return;

    if(bot_cfg_fp == NULL) {
        if(cfg_file == 1)
            need_to_open_cfg2 = TRUE;
        if(cfg_file == 2) {
            cfg_file = 0;
            bot_cfg_pause_time = 0.0; // stop it checking the file again :)
        }
        return;
    }

    cmd_index = 0;
    cmd_line[cmd_index] = 0;
    cmd_line[510] = 0;

    ch = fgetc(bot_cfg_fp);

    // skip any leading blanks
    while(ch == ' ')
        ch = fgetc(bot_cfg_fp);

    while((ch != EOF) && (ch != '\r') && (ch != '\n') && (cmd_index < 510)) {
        if(ch == '\t') // convert tabs to spaces
            ch = ' ';

        cmd_line[cmd_index] = static_cast<char>(ch);

        ch = fgetc(bot_cfg_fp);

        // skip multiple spaces in input file
        while((cmd_line[cmd_index] == ' ') && (ch == ' '))
            ch = fgetc(bot_cfg_fp);

        cmd_index++;
    }

    if(ch == '\r') // is it a carriage return?
    {
        ch = fgetc(bot_cfg_fp); // skip the linefeed
    }

    // if reached end of file, then close it
    if(ch == EOF) {
        fclose(bot_cfg_fp);

        bot_cfg_fp = NULL;
    }
}

// This function can be used to process a specified bot setting command.
// It can handle bot options specified in a config file or from the
// command line or console.
// e.g. the max_bots setting.
static void changeBotSetting(const char* settingName,
    int* setting,
    const char* arg1,
    const int minValue,
    const int maxValue,
    const int settingSource)
{
    bool settingWasChanged = FALSE;

    char msg[128] = "";

    // if the setting change is from a config file set up a message about it
    char configMessage[] = "[Config] ";
    if(settingSource != SETTING_SOURCE_CONFIG_FILE)
        configMessage[0] = '\0';

    // if an argument was supplied change the setting with it
    if(arg1 != NULL && (*arg1 != 0)) {
        int temp = atoi(arg1);
        if(temp >= minValue && temp <= maxValue) {
            *setting = temp;
            settingWasChanged = TRUE;
        } else {
            // report that a bad setting was requested

            _snprintf(msg, 128, "%s%s should be set from %d to %d\n", configMessage, settingName, minValue, maxValue);
            msg[127] = '\0';

            if(settingSource == SETTING_SOURCE_CLIENT_COMMAND)
                ClientPrint(INDEXENT(1), HUD_PRINTNOTIFY, msg);
            else if(settingSource == SETTING_SOURCE_SERVER_COMMAND)
                printf(msg);
            else if(settingSource == SETTING_SOURCE_CONFIG_FILE) {
                if(IS_DEDICATED_SERVER())
                    printf(msg);
                else
                    ALERT(at_console, msg);
            }
        }
    }

    // report if the setting was actually changed or not
    if(settingWasChanged) {
        _snprintf(msg, 128, "%s%s has been set to %d\n", configMessage, settingName, *setting);
        msg[127] = '\0';
    } else {
        _snprintf(msg, 128, "%s%s is currently set to %d\n", configMessage, settingName, *setting);
        msg[127] = '\0';
    }

    if(settingSource == SETTING_SOURCE_CLIENT_COMMAND)
        ClientPrint(INDEXENT(1), HUD_PRINTNOTIFY, msg);
    else if(settingSource == SETTING_SOURCE_SERVER_COMMAND)
        printf(msg);
    else if(settingSource == SETTING_SOURCE_CONFIG_FILE) {
        if(IS_DEDICATED_SERVER())
            printf(msg);
        else
            ALERT(at_console, msg);
    }
}

// This a general purpose function for kicking bots.
// Set team to -1 if you don't want to specify a team.
// Set totalToKick to MAX_BOTS if you want to kick all of them.
static void kickBots(int totalToKick, const int team)
{
    // sanity check
    if(totalToKick < 1 || team > 3)
        return;

    if(totalToKick >= MAX_BOTS)
        totalToKick = MAX_BOTS - 1;

    for(int index = MAX_BOTS - 1; index >= 0 && totalToKick > 0; index--) {
        if(bots[index].is_used // is this slot used?
            && !FNullEnt(bots[index].pEdict)) {
            char cmd[80];

            _snprintf(cmd, 80, "kick \"%s\"\n", bots[index].name);
            cmd[79] = '\0';
            SERVER_COMMAND(cmd); // kick the bot using (kick "name")
            bots[index].is_used = FALSE;
            --totalToKick;
        }
    }
}

void ServerCommand(void)
{
    if(strcmp(CMD_ARGV(1), "addbot") == 0) {
        BotCreate(NULL, CMD_ARGV(2), CMD_ARGV(3));
        bot_check_time = gpGlobals->time + 5.0;
    } else if(strcmp(CMD_ARGV(1), "min_bots") == 0) {
        min_bots = atoi(CMD_ARGV(2));

        if((min_bots < 0) || (min_bots > 31))
            min_bots = 1;

        printf("min_bots set to %d\n", min_bots);
    } else if(strcmp(CMD_ARGV(1), "max_bots") == 0) {
        max_bots = atoi(CMD_ARGV(2));

        if((max_bots < 0) || (max_bots > 31))
            max_bots = 1;

        printf("max_bots set to %d\n", max_bots);
    }
}

int DispatchSpawn(edict_t* pent)
{
    if(gpGlobals->deathmatch) {
        char* pClassname = (char*)STRING(pent->v.classname);

        if(strcmp(pClassname, "worldspawn") == 0) {
            g_GameRules = TRUE;

            bot_cfg_pause_time = 0.0;
            respawn_time = 0.0;
            spawn_time_reset = FALSE;

            prev_num_bots = num_bots;
            num_bots = 0;

            bot_check_time = gpGlobals->time + 30.0;
        }
    }

    RETURN_META_VALUE(MRES_IGNORED, 0);
}

BOOL ClientConnect(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
    if(gpGlobals->deathmatch) {
        int i;
        int count = 0;

        // check if this client is the listen server client
        if(strcmp(pszAddress, "loopback") == 0)
            listenserver_edict = pEntity; // save the edict of the listen server client...

        // check if this is NOT a bot joining the server...
        if(strcmp(pszAddress, "127.0.0.1") != 0) {
            // don't try to add bots for 60 seconds, give client time to get added
            bot_check_time = gpGlobals->time + 60.0;

            for(i = 0; i < 32; i++) {
                if(bots[i].is_used) // count the number of bots in use
                    count++;
            }

            // if there are currently more than the minimum number of bots running
            // then kick one of the bots off the server...
            if(count > min_bots && min_bots != -1) {
                for(i = 0; i < 32; i++) {
                    if(bots[i].is_used) // is this slot used?
                    {
                        char cmd[80];
                        sprintf(cmd, "kick \"%s\"\n", STRING(bots[i].pEdict->v.netname));
                        SERVER_COMMAND(cmd); // kick the bot using (kick "name")
                        break;
                    }
                }
            }
        }
    }

    RETURN_META_VALUE(MRES_IGNORED, 0);
}

void ClientDisconnect(edict_t* pEntity)
{
    if(gpGlobals->deathmatch) {
        int i, index = -1;

        for(i = 0; i < 32; i++) {
            if(bots[i].pEdict == pEntity && bots[i].is_used) {
                index = i;
                break;
            }
        }

        if(index != -1) {
            // someone has kicked this bot off of the server...
            bots[index].is_used = FALSE;               // this bot index is now free to re-use
        } else {
            i = 0;

            while((i < 32) && (clients[i] != pEntity))
                i++;

            if(i < 32)
                clients[i] = NULL;
            // human left?
            // what about level changes?
        }
    }
}

void ClientCommand(edict_t* pEntity)
{
    // only allow custom commands if deathmatch mode and NOT dedicated server and
    // client sending command is the listen server client...

    if((gpGlobals->deathmatch) && (!IS_DEDICATED_SERVER()) && (pEntity == listenserver_edict)) {
        const char* pcmd = CMD_ARGV(0);
        const char* arg1 = CMD_ARGV(1);
        const char* arg2 = CMD_ARGV(2);
        char msg[80];

        if(FStrEq(pcmd, "addbot")) {
            BotCreate(pEntity, arg1, arg2);

            bot_check_time = gpGlobals->time + 5.0;

            RETURN_META(MRES_SUPERCEDE);
        } else if(FStrEq(pcmd, "observer")) {
            if((arg1 != NULL) && (*arg1 != 0)) {
                int temp = atoi(arg1);
                if(temp)
                    b_observer_mode = TRUE;
                else
                    b_observer_mode = FALSE;
            }

            if(b_observer_mode)
                ClientPrint(pEntity, HUD_PRINTNOTIFY, "observer mode ENABLED\n");
            else
                ClientPrint(pEntity, HUD_PRINTNOTIFY, "observer mode DISABLED\n");

            RETURN_META(MRES_SUPERCEDE);
        } else if(FStrEq(pcmd, "botskill")) {
            if((arg1 != NULL) && (*arg1 != 0)) {
                int temp = atoi(arg1);

                if(temp < 1 || temp > 5)
                    ClientPrint(pEntity, HUD_PRINTNOTIFY, "invalid botskill value!\n");
                else
                    default_bot_skill = temp;
            }

            sprintf(msg, "botskill is %d\n", default_bot_skill);
            ClientPrint(pEntity, HUD_PRINTNOTIFY, msg);

            RETURN_META(MRES_SUPERCEDE);
        } else if(FStrEq(pcmd, "botdontshoot")) {
            if((arg1 != NULL) && (*arg1 != 0)) {
                int temp = atoi(arg1);
                if(temp)
                    b_botdontshoot = TRUE;
                else
                    b_botdontshoot = FALSE;
            }

            if(b_botdontshoot)
                ClientPrint(pEntity, HUD_PRINTNOTIFY, "botdontshoot ENABLED\n");
            else
                ClientPrint(pEntity, HUD_PRINTNOTIFY, "botdontshoot DISABLED\n");

            RETURN_META(MRES_SUPERCEDE);
        } 
    }

    RETURN_META(MRES_IGNORED);
}

void StartFrame( void )
{
	if (gpGlobals->deathmatch)
	{
		edict_t *pPlayer;
        static float check_server_cmd;
        check_server_cmd = gpGlobals->time;
		static int i, index, player_index, bot_index;
		static float previous_time = -1.0;
		char msg[256];
		int count;

		// if a new map has started then (MUST BE FIRST IN StartFrame)...
		if ((gpGlobals->time + 0.1) < previous_time)
		{
			char filename[256];
			char mapname[64];

			// check if mapname_bot.cfg file exists...

			strcpy(mapname, STRING(gpGlobals->mapname));
			strcat(mapname, "_bot.cfg");

			UTIL_BuildFileName(filename, "maps", mapname);

			if ((bot_cfg_fp = fopen(filename, "r")) != NULL)
			{
				sprintf(msg, "Executing %s\n", filename);
				ALERT( at_console, msg );

				for (index = 0; index < 32; index++)
				{
					bots[index].is_used = FALSE;
					bots[index].respawn_state = 0;
				}
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
					}

					if (bots[index].is_used)  // is this slot used?
					{
						bots[index].respawn_state = RESPAWN_NEED_TO_RESPAWN;
						count++;
					}

				}
			}

			bot_check_time = gpGlobals->time + 60.0;
		}

		count = 0;

		if (count > num_bots)
			num_bots = count;

		for (player_index = 1; player_index <= gpGlobals->maxClients; player_index++)
		{
			pPlayer = INDEXENT(player_index);
		}
		if (g_GameRules)
		{
			if (need_to_open_cfg)  // have we open bot.cfg file yet?
			{
				char filename[256];
				char mapname[64];

                need_to_open_cfg = FALSE; // only do this once!!!
                cfg_file = 1;
                
				// check if mapname_bot.cfg file exists...

				strcpy(mapname, STRING(gpGlobals->mapname));
				strcat(mapname, "_bot.cfg");

				UTIL_BuildFileName(filename, "maps", mapname);

				if ((bot_cfg_fp = fopen(filename, "r")) != NULL)
				{
					sprintf(msg, "Executing %s\n", filename);
					ALERT( at_console, msg );
				}
				else
				{
					UTIL_BuildFileName(filename, "bot.cfg", NULL);

					sprintf(msg, "Executing %s\n", filename);
					ALERT( at_console, msg );

					bot_cfg_fp = fopen(filename, "r");

					if (bot_cfg_fp == NULL)
						ALERT( at_console, "bot.cfg file not found\n" );
				}
			}

			if ((bot_cfg_fp) &&
				 (bot_cfg_pause_time >= 1.0) && (bot_cfg_pause_time <= gpGlobals->time))
			{
				// process bot.cfg file options...
				ProcessBotCfgFile();
			}

		}		

// if time to check for server commands then do so...
        if((check_server_cmd <= gpGlobals->time) && IS_DEDICATED_SERVER()) {
            check_server_cmd = gpGlobals->time + 1.0;

            char* cvar_bot = (char*)CVAR_GET_STRING("bot");

            if(cvar_bot && cvar_bot[0]) {
                char cmd_line[80];
                char *cmd, *arg1, *arg2, *arg3;

                strcpy(cmd_line, cvar_bot);

                index = 0;
                cmd = cmd_line;
                arg1 = arg2 = arg3 = NULL;

                // skip to blank or end of string...
                while((cmd_line[index] != ' ') && (cmd_line[index] != 0))
                    index++;

                if(cmd_line[index] == ' ') {
                    cmd_line[index++] = 0;
                    arg1 = &cmd_line[index];

                    // skip to blank or end of string...
                    while((cmd_line[index] != ' ') && (cmd_line[index] != 0))
                        index++;

                    if(cmd_line[index] == ' ') {
                        cmd_line[index++] = 0;
                        arg2 = &cmd_line[index];

                        // skip to blank or end of string...
                        while((cmd_line[index] != ' ') && (cmd_line[index] != 0))
                            index++;

                        if(cmd_line[index] == ' ') {
                            cmd_line[index++] = 0;
                            arg3 = &cmd_line[index];

                            // skip to blank or end of string...
                            while((cmd_line[index] != ' ') && (cmd_line[index] != 0))
                                index++;

                            if(cmd_line[index] == ' ') {
                                cmd_line[index++] = 0;
                            }
                        }
                    }
                }

                if(strcmp(cmd, "addbot") == 0) {
                    BotCreate(NULL, arg1, arg2);
                    bot_check_time = gpGlobals->time + 5.0;
                } else if(strcmp(cmd, "min_bots") == 0) {
                    changeBotSetting("min_bots", &min_bots, arg1, -1, 31, SETTING_SOURCE_SERVER_COMMAND);
                } else if(strcmp(cmd, "max_bots") == 0) {
                    changeBotSetting("max_bots", &max_bots, arg1, -1, MAX_BOTS, SETTING_SOURCE_SERVER_COMMAND);
                } else if(strcmp(cmd, "bot_bot_balance") == 0) {
                    if((arg1 != NULL)) {
                        if((*arg1 != 0)) {
                            int temp = atoi(arg1);
                            if(temp)
                                bot_bot_balance = TRUE;
                            else
                                bot_bot_balance = FALSE;
                        }
                    }

                    if(bot_bot_balance)
                        printf("bot_bot_balance (1) On\n");
                    else
                        printf("bot_bot_balance (0) Off\n");
                } else if(strcmp(cmd, "kickall") == 0) {
                    // kick all bots off of the server after time/frag limit...
                    kickBots(MAX_BOTS, -1);}
            }
            // moved this line down one
            CVAR_SET_STRING("bot", "");
        }

		// check if time to see if a bot needs to be created...
		if (bot_check_time < gpGlobals->time)
		{
			int count = 0;

			bot_check_time = gpGlobals->time + 5.0;

			for (i = 0; i < 32; i++)
			{
				if (clients[i] != NULL)
					count++;
			}

			// if there are currently less than the maximum number of "players"
			// then add another bot using the default skill level...
			if(count < max_bots)
			{
				BotCreate( NULL,NULL,NULL);
			}
			// Kick bots if there are to many
			else if(((count > max_bots) && (max_bots > 0)) || (max_bots == -1))
			{
				for (i=0; i < 32; i++)
				{
					if (bots[i].is_used)  // is this slot used?
					{
						char cmd[80];

						sprintf(cmd, "kick \"%s\"\n", bots[i].name);

						SERVER_COMMAND(cmd);  // kick the bot using (kick "name")

						break;
					}
				}
			}
		}

		previous_time = gpGlobals->time;
	}

	RETURN_META (MRES_IGNORED);
}

// This function will kick a random bot.
static void kickRandomBot(void)
{
    int bot_list[MAX_BOTS];
    int bot_total = 0;
    int index;

    // count and list the bots
    for(index = 0; index < MAX_BOTS; index++) {
        if(bots[index].is_used // is this slot used?
            && !FNullEnt(bots[index].pEdict)) {
            bot_list[bot_total] = index;
            ++bot_total;
        }
    }

    // sanity checking
    if(bot_total < 1 || bot_total >= MAX_BOTS)
        return;

    // kick a random bot from the list
    // double check we're kicking an active bot too
    index = bot_list[random_long(0, bot_total - 1)];
    if(index > -1 && index < MAX_BOTS && bots[index].is_used && !FNullEnt(bots[index].pEdict)) {
        char cmd[80];

        _snprintf(cmd, 80, "kick \"%s\"\n", bots[index].name);
        cmd[79] = '\0';
        SERVER_COMMAND(cmd); // kick the bot using (kick "name")
        bots[index].is_used = FALSE;
    }
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