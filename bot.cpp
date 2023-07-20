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
// bot.cpp
//

#include "bot.h"

bot_t bots[32];   // max of 32 bots in a game
bool b_observer_mode = FALSE;
bool b_botdontshoot = FALSE;
int bot_chat_percent = 10;

int number_names = 0;

char bot_names[MAX_BOT_NAMES][BOT_NAME_LEN+1];

void BotSpawnInit( bot_t *pBot )
{
   //Fix by Cheeseh (RCBot)
   //float fUpdateTime = gpGlobals->time;
   //float fLastRunPlayerMoveTime = gpGlobals->time - 0.1f;
	
   //pBot->msecnum = 0;
   //pBot->msecdel = 0.0;
   //pBot->msecval = 0.0;

   pBot->pBotEnemy = nullptr;
   pBot->f_bot_see_enemy_time = gpGlobals->time;
   pBot->f_bot_find_enemy_time = gpGlobals->time;
   pBot->b_bot_say_killed = FALSE;
   pBot->f_bot_say_killed = 0.0f;
}

void BotNameInit()
{
   FILE *bot_name_fp;
   char bot_name_filename[256];
   int str_index;
   char name_buffer[80];
   int length, index;

   UTIL_BuildFileName(bot_name_filename, "bot_names.txt", nullptr);

   bot_name_fp = fopen(bot_name_filename, "r");

   if (bot_name_fp != nullptr)
   {
      while ((number_names < MAX_BOT_NAMES) &&
             (fgets(name_buffer, 80, bot_name_fp) != nullptr))
      {
         length = strlen(name_buffer);

         if (name_buffer[length-1] == '\n')
         {
            name_buffer[length-1] = 0;  // remove '\n'
            length--;
         }

         str_index = 0;
         while (str_index < length)
         {
            if ((name_buffer[str_index] < ' ') || (name_buffer[str_index] > '~') ||
                (name_buffer[str_index] == '"'))
            for (index=str_index; index < length; index++)
               name_buffer[index] = name_buffer[index+1];

            str_index++;
         }

         if (name_buffer[0] != 0)
         {
            strncpy(bot_names[number_names], name_buffer, BOT_NAME_LEN);
            number_names++;
         }
      }

      fclose(bot_name_fp);
   }
}


void BotPickName( char *name_buffer )
{
   int name_index, index;
   bool used;
   edict_t *pPlayer;
   int attempts = 0;

   // see if a name exists from a kicked bot (if so, reuse it)
   for (index=0; index < 32; index++)
   {
      if ((bots[index].is_used == FALSE) && (bots[index].name[0]))
      {
         strcpy(name_buffer, bots[index].name);

         return;
      }   
   }

   name_index = RANDOM_LONG(1, number_names) - 1;  // zero based

   // check make sure this name isn't used
   used = TRUE;

   while (used)
   {
      used = FALSE;

      for (index = 1; index <= gpGlobals->maxClients; index++)
      {
         pPlayer = INDEXENT(index);

         if (pPlayer && !pPlayer->free)
         {
            if (strcmp(bot_names[name_index], STRING(pPlayer->v.netname)) == 0)
            {
               used = TRUE;
               break;
            }
         }
      }

      if (used)
      {
         name_index++;

         if (name_index == number_names)
            name_index = 0;

         attempts++;

         if (attempts == number_names)
            used = FALSE;  // break out of loop even if already used
      }
   }

   strcpy(name_buffer, bot_names[name_index]);
}


void BotCreate( edict_t *pPlayer, const char *pname, const char *pskill )
{
   edict_t *BotEnt;
   bot_t *pBot;
   char c_name[BOT_NAME_LEN + 1];
   int skill;
   int i, j, length;

   if ((pname != nullptr) && (*pname != 0))
   {
      strncpy( c_name, pname, BOT_NAME_LEN - 1 );
      c_name[BOT_NAME_LEN] = 0;  // make sure c_name is null terminated
   }
   else
   {
      if (number_names > 0)
         BotPickName( c_name );
      else
         strcpy(c_name, "Bot");
   }

   skill = 0;

   if ((pskill != nullptr) && (*pskill != 0))
      skill = atoi(pskill);
         
   if (skill < 1 || skill > 5)
      skill = default_bot_skill;

   length = strlen(c_name);

   // remove any illegal characters from name...
   for (i = 0; i < length; i++)
   {
      if ((c_name[i] <= ' ') || (c_name[i] > '~') ||
          (c_name[i] == '"'))
      {
         for (j = i; j < length; j++)  // shuffle chars left (and null)
            c_name[j] = c_name[j+1];
         length--;
      }               
   }

   BotEnt = (*g_engfuncs.pfnCreateFakeClient)( c_name );

   if (FNullEnt( BotEnt ))
   {
      if (pPlayer)
         ClientPrint( pPlayer, HUD_PRINTNOTIFY, "Max. Players reached.  Can't create bot!\n");
   }
   else
   {
      char ptr[128];  // allocate space for message from ClientConnect
      int index;

      index = 0;
      while ((bots[index].is_used) && (index < 32))
         index++;

      if (index == 32)
      {
         ClientPrint( pPlayer, HUD_PRINTNOTIFY, "Can't create bot!\n");
         return;
      }

      if (BotEnt->pvPrivateData != nullptr)
         FREE_PRIVATE(BotEnt);
      BotEnt->pvPrivateData = nullptr;
      BotEnt->v.frags = 0;

      // create the player entity by calling MOD's player function
      // (from LINK_ENTITY_TO_CLASS for player object)
      CALL_GAME_ENTITY(PLID, "player", VARS(BotEnt));

      MDLL_ClientConnect( BotEnt, c_name, "127.0.0.1", ptr );
      MDLL_ClientPutInServer( BotEnt );

      BotEnt->v.flags |= FL_FAKECLIENT;

      pBot = &bots[index]; // initialize all the variables for this bot...

      pBot->is_used = TRUE;
      pBot->respawn_state = RESPAWN_IDLE;
      pBot->create_time = gpGlobals->time;
      pBot->name[0] = 0;  // name not set by server yet

      pBot->pEdict = BotEnt;

      BotSpawnInit(pBot);

      pBot->need_to_initialize = FALSE;  // don't need to initialize yet

      BotEnt->v.idealpitch = BotEnt->v.v_angle.x;
      BotEnt->v.ideal_yaw = BotEnt->v.v_angle.y;
      BotEnt->v.pitch_speed = BOT_PITCH_SPEED;
      BotEnt->v.yaw_speed = BOT_YAW_SPEED;

      pBot->bot_skill = skill - 1;  // 0 based for array indexes
   }
}


void BotChangePitch( bot_t *pBot, float speed )
{
   edict_t *pEdict = pBot->pEdict;
   float ideal;
   float current;
   float current_180;  // current +/- 180 degrees
   float diff;

   ClampAngle(pEdict->v.idealpitch);

   // turn from the current v_angle pitch to the idealpitch by selecting
   // the quickest way to turn to face that direction
   current = pEdict->v.v_angle.x;

   ideal = pEdict->v.idealpitch;

   // find the difference in the current and ideal angle
   diff = std::fabs(current - ideal);

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff;  // just need to turn a little bit (less than max)

	// here we have four cases, both angle positive, one positive and
	// the other negative, one negative and the other positive, or
	// both negative.  handle each case separately...
   if (current >= 0 && ideal >= 0)  // both positive
   {
      if (current > ideal)
         speed = -speed;
   }
   else if (current >= 0 && ideal < 0)
   {
      current_180 = current - 180;
      if (current_180 <= ideal)
         speed = -speed;
   }
   else if (current < 0 && ideal >= 0)
   {
      current_180 = current + 180;
      if (current_180 <= ideal)
         speed = -speed;
   }
   else  // (current < 0) && (ideal < 0)  both negative
   {
      if (current > ideal)
         speed = -speed;
   }

   current += speed;
   pEdict->v.v_angle.x = current;
}

void BotChangeYaw( bot_t *pBot, float speed )
{
   edict_t *pEdict = pBot->pEdict;
   float ideal;
   float current;
   float current_180;  // current +/- 180 degrees
   float diff;

   ClampAngle(pEdict->v.ideal_yaw);

   // turn from the current v_angle yaw to the ideal_yaw by selecting
   // the quickest way to turn to face that direction
   current = pEdict->v.v_angle.y;

   ideal = pEdict->v.ideal_yaw;

   // find the difference in the current and ideal angle
   diff = std::fabs(current - ideal);

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff;  // just need to turn a little bit (less than max)

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...
   if (current >= 0 && ideal >= 0)  // both positive
   {
      if (current > ideal)
         speed = -speed;
   }
   else if (current >= 0 && ideal < 0)
   {
      current_180 = current - 180;
      if (current_180 <= ideal)
         speed = -speed;
   }
   else if (current < 0 && ideal >= 0)
   {
      current_180 = current + 180;
      if (current_180 <= ideal)
         speed = -speed;
   }
   else // (current < 0) && (ideal < 0)  both negative
   {
      if (current > ideal)
         speed = -speed;
   }

   current += speed;
   pEdict->v.v_angle.y = current;
}

// The next 2 functions are taken from POD-Bot source

bool IsDeadlyDrop(bot_t *pBot, const Vector& vecTargetPos)
{
   // Returns if given location would hurt Bot with falling damage

   edict_t *pEdict = pBot->pEdict;
   const Vector vecBot = pEdict->v.origin;
   TraceResult tr;
   float height, last_height, distance;

   Vector vecMove = UTIL_VecToAngles(vecTargetPos - vecBot);
   vecMove.x = 0; // reset pitch to 0 (level horizontally)
   vecMove.z = 0; // reset roll to 0 (straight up and down)
   MAKE_VECTORS(vecMove);

   const Vector v_direction = (vecTargetPos - vecBot).Normalize (); // 1 unit long
   Vector v_check = vecBot;
   Vector v_down = vecBot;

   v_down.z = v_down.z - 1000.0f; // straight down 1000 units

   TRACE_HULL(v_check, v_down, ignore_monsters, head_hull, pEdict, &tr);

   // We're not on ground anymore ?
   if (tr.flFraction > 0.036f)
      tr.flFraction = 0.036f;

   last_height = tr.flFraction * 1000.0f; // height from ground
   distance = (vecTargetPos - v_check).Length (); // distance from goal

   while (distance > 16.0f)
   {
      // move 10 units closer to the goal...
      v_check = v_check + (v_direction * 16.0f);

      v_down = v_check;
      v_down.z = v_down.z - 1000.0f; // straight down 1000 units

      TRACE_HULL(v_check, v_down, ignore_monsters, head_hull, pEdict, &tr);

      // Wall blocking ?
      if (tr.fStartSolid)
         return FALSE;

      height = tr.flFraction * 1000.0f; // height from ground

      // Drops more than 100 Units ?
      if (last_height < height - 100)
         return TRUE;

      last_height = height;

      distance = (vecTargetPos - v_check).Length(); // distance from goal
   }

   return FALSE;
}

// Adjust all Bot Body and View Angles to face an absolute Vector
void BotFacePosition(bot_t *pBot, Vector const& vecPos)
{
   edict_t *pEdict = pBot->pEdict;
   Vector vecDirection = UTIL_VecToAngles(vecPos - GetGunPosition(pEdict));
   vecDirection.x = -vecDirection.x;
   ClampAngles(vecDirection);

   pEdict->v.ideal_yaw = vecDirection.y;
   pEdict->v.idealpitch = vecDirection.x;
}

void BotMoveToPosition(bot_t *pBot, Vector const& vecPos)
{
	const edict_t *pEdict = pBot->pEdict;

   if (pBot->pBotEnemy == nullptr)
   {
      // No enemy, just head towards the position
      pBot->vecLookAt = vecPos;
      pBot->f_move_speed = pEdict->v.maxspeed;
   }
   else
   {
      // Otherwise we need to do strafing...
      const float f1 = pEdict->v.angles.y - VEC_TO_YAW(vecPos - pEdict->v.origin);
      const float flCos = cos(f1 * M_PI / 180);
      const float flSin = sin(f1 * M_PI / 180);
      pBot->f_move_speed = pEdict->v.maxspeed * flCos;
      pBot->f_sidemove_speed = pEdict->v.maxspeed * flSin;
   }
}

edict_t *BotFindEnemy( bot_t *pBot )
{
   Vector vecEnd;
   edict_t *pNewEnemy = nullptr;

   edict_t *pEdict = pBot->pEdict;

   if (pBot->pBotEnemy != nullptr)  // does the bot already have an enemy?
   {
      vecEnd = pBot->pBotEnemy->v.origin + pBot->pBotEnemy->v.view_ofs;

      // if the enemy is dead?
      if (!IsAlive(pBot->pBotEnemy))  // is the enemy dead?, assume bot killed it
         pBot->pBotEnemy = nullptr; // don't have an enemy anymore so null out the pointer...
      else if (FInViewCone( &vecEnd, pEdict ) && FVisible( vecEnd, pEdict ) &&
	      std::fabs(pBot->pEdict->v.origin.z - pBot->pBotEnemy->v.origin.z) < 72)
         return pBot->pBotEnemy; // if enemy is still visible and in field of view, keep it
   }

   float nearestdistance = 2500.0f;

   // search the world for players...
   for (int i = 1; i <= gpGlobals->maxClients; i++)
   {
      edict_t *pPlayer = INDEXENT(i);

      // skip invalid players and skip self (i.e. this bot)
      if (pPlayer && !pPlayer->free && pPlayer != pEdict)
      {
         if (!IsAlive(pPlayer))
            continue; // skip this player if not alive (i.e. dead or dying)

         if (b_observer_mode && !(pPlayer->v.flags & FL_FAKECLIENT))
            continue;

         if (pPlayer->v.playerclass != 0)
            continue; // skip this player if he is spectating

         if (pPlayer->v.groupinfo != pEdict->v.groupinfo)
            continue; // skip this player is he is in a different arena

         // see if the enemy is at the same height as the bot
         if (std::fabs(pBot->pEdict->v.origin.z - pPlayer->v.origin.z) >= 72)
            continue;

         vecEnd = pPlayer->v.origin + pPlayer->v.view_ofs;

         // see if bot can see the player...
         if (FInViewCone( &vecEnd, pEdict ) && FVisible( vecEnd, pEdict ))
         {
	         const float distance = (pPlayer->v.origin - pEdict->v.origin).Length();
            if (distance < nearestdistance)
            {
               nearestdistance = distance;
               pNewEnemy = pPlayer;
            }
         }
      }
   }
   return pNewEnemy;
}

void BotShootAtEnemy( bot_t *pBot )
{
   edict_t *pEdict = pBot->pEdict;

   if (FNullEnt(pBot->pBotEnemy))
      return; // no enemy, no need to shoot

   // is the bot's crosshair on the enemy yet?
   const Vector vecDir = (pBot->vecLookAt - GetGunPosition(pBot->pEdict)).Normalize();
   UTIL_MakeVectors(pEdict->v.v_angle);

   if (DotProduct(gpGlobals->v_forward, vecDir) > 0.9f)
   {
      // we will likely hit the enemy, FIRE!!
      // TODO: bounce/teleport attack
      const float distance = (pEdict->v.origin - pBot->pBotEnemy->v.origin).Length();
      // if enemy is near enough, we have enough discs,
      // and we haven't "Power Shot" powerup...
      if (distance < 128 && pBot->disc_number >= 3 && !(pBot->m_iPowerups & POW_HARD)
         && RANDOM_LONG(1, 100) < (6 - pBot->bot_skill) * 20)
         pEdict->v.button |= IN_ATTACK2; // Decapitate him!!!!
      else
         pEdict->v.button |= IN_ATTACK;
   }
}

void BotThink( bot_t *pBot )
{
   //Fix by Cheeseh (RCBot)
   const float msecval = (gpGlobals->time - pBot->fLastRunPlayerMoveTime) * 1000.0f;
   pBot->fLastRunPlayerMoveTime = gpGlobals->time;

   const float fUpdateInterval = 1.0f / 60.0f; // update at 60 fps
   pBot->fUpdateTime = gpGlobals->time + fUpdateInterval;
	
   Vector v_diff;             // vector from previous to current location
   TraceResult tr;

   edict_t *pEdict = pBot->pEdict;

   pEdict->v.flags |= FL_FAKECLIENT;

   if (pBot->name[0] == 0)  // name filled in yet?
      strcpy(pBot->name, STRING(pBot->pEdict->v.netname));

// TheFatal - START from Advanced Bot Framework (Thanks Rich!)
/*
   // adjust the millisecond delay based on the frame rate interval...
   if (pBot->msecdel <= gpGlobals->time)
   {
      pBot->msecdel = gpGlobals->time + 0.5;
      if (pBot->msecnum > 0)
         pBot->msecval = 450.0 / pBot->msecnum;
      pBot->msecnum = 0;
   }
   else
      pBot->msecnum++;

   if (pBot->msecval < 1)    // don't allow msec to be less than 1...
      pBot->msecval = 1;

   if (pBot->msecval > 100)  // ...or greater than 100
      pBot->msecval = 100;

// TheFatal - END
*/
   pEdict->v.button = 0;
   pBot->f_move_speed = 0.0;
   pBot->f_sidemove_speed = 0.0;

   if (FNullEnt(pBot->killer_edict))
      pBot->b_bot_say_killed = FALSE;

   if (pBot->b_bot_say_killed && pBot->f_bot_say_killed < gpGlobals->time)
   {
      int whine_index = 0;
      bool used;
      int i, recent_count;
      char msg[120];

      pBot->b_bot_say_killed = FALSE;

      recent_count = 0;

      while (recent_count < 5)
      {
         whine_index = RANDOM_LONG(0, whine_count - 1);

         used = FALSE;

         for (i = 0; i < 5; i++)
         {
            if (recent_bot_whine[i] == whine_index)
               used = TRUE;
         }

         if (used)
            recent_count++;
         else
            break;
      }

      for (i = 4; i > 0; i--)
         recent_bot_whine[i] = recent_bot_whine[i-1];

      recent_bot_whine[0] = whine_index;

      if (strstr(bot_whine[whine_index], "%s") != nullptr)  // is "%s" in whine text?
         sprintf(msg, bot_whine[whine_index], STRING(pBot->killer_edict->v.netname));
      else
         sprintf(msg, bot_whine[whine_index]);

      UTIL_HostSay(pEdict, msg);
   }

   // if the bot is dead, randomly press fire to respawn...
   if (pEdict->v.health < 1 || pEdict->v.deadflag != DEAD_NO)
   {
      if (pBot->need_to_initialize)
      {
         BotSpawnInit(pBot);

         // did another player kill this bot AND bot whine messages loaded
         if (pBot->killer_edict != nullptr && whine_count > 0)
         {
            if (RANDOM_LONG(1, 100) <= bot_chat_percent)
            {
               pBot->b_bot_say_killed = TRUE;
               pBot->f_bot_say_killed = gpGlobals->time + RANDOM_FLOAT(0.5, 2.5);
            }
         }

         pBot->need_to_initialize = FALSE;
      }

      if (RANDOM_LONG(1, 100) > 50)
         pEdict->v.button = IN_ATTACK;

      g_engfuncs.pfnRunPlayerMove( pEdict, pEdict->v.v_angle, 0,
         0, 0, pEdict->v.button, 0, msecval);

      return;
   }

   // set this for the next time the bot dies so it will initialize stuff
   if (pBot->need_to_initialize == FALSE)
      pBot->need_to_initialize = TRUE;

   // Get Speed Multiply Factor by dividing Target FPS by real FPS
   float fSpeedFactor = CVAR_GET_FLOAT("fps_max") / (1.0f / gpGlobals->frametime);
   if (fSpeedFactor < 1)
      fSpeedFactor = 1.0f;

   BotChangeYaw(pBot, pEdict->v.yaw_speed * fSpeedFactor);
   BotChangePitch(pBot, pEdict->v.pitch_speed * fSpeedFactor);

   // set the body angles to point the gun correctly
   pEdict->v.angles.x = -pEdict->v.v_angle.x / 3;
   pEdict->v.angles.y = pEdict->v.v_angle.y;
   ClampAngles(pEdict->v.angles);
   ClampAngles(pEdict->v.v_angle);

   // handle movement related actions...
   if (b_botdontshoot == 0)
      pBot->pBotEnemy = BotFindEnemy( pBot );
   else
      pBot->pBotEnemy = nullptr;  // clear enemy pointer (no ememy for you!)

   if (pBot->pBotEnemy != nullptr)  // does an enemy exist?
   {
      // Aim at the enemy
      pBot->vecLookAt = pBot->pBotEnemy->v.origin;
      // TODO: bounce/teleport attack
      BotShootAtEnemy( pBot );  // shoot at the enemy
   }

   if (pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND))
   {
      // jump to another disc randomly. TODO: make this much more advanced
      if (pBot->vecTargetPos == Vector(0, 0, 0))
      {
         // find a jump arrow randomly.
         // TODO: let bots go for powerups or find paths on purpose
         int iCount = 0;
         Vector vecWPT[8];

         edict_t *p = nullptr;
         while ((p = UTIL_FindEntityInSphere( p, pBot->pEdict->v.origin, 240.0f )) != nullptr)
         {
            if (strcmp(STRING(p->v.classname), "trigger_jump") == 0)
            {
	            const Vector v_dest = VecBModelOrigin(p);
               if (std::fabs(pEdict->v.absmin.z - p->v.absmax.z) < 32 &&
                  !IsDeadlyDrop(pBot, v_dest))
               {
                  vecWPT[iCount++] = v_dest;
                  if (iCount >= 8)
                     break;
               }
            }
         }

         if (iCount > 0)
            pBot->vecTargetPos = vecWPT[RANDOM_LONG(0, iCount - 1)];
      }

      // try not to knock into other players
      if (pBot->vecTargetPos != Vector(0, 0, 0))
      {
         for (int i = 1; i <= gpGlobals->maxClients; i++)
         {
	         const edict_t *pPlayer = INDEXENT(i);

            if (FNullEnt(pPlayer))
               continue;

            if (pPlayer == pEdict || pPlayer->v.playerclass != 0 ||
               pPlayer->v.groupinfo != pEdict->v.groupinfo ||
               pPlayer->v.origin.z - pEdict->v.origin.z > 128 ||
               pPlayer->v.origin.z - pEdict->v.origin.z < 0 ||
               pPlayer->v.flags & (FL_ONGROUND | FL_PARTIALGROUND))
               continue;

            Vector2D vecPlayer = (pPlayer->v.origin - pEdict->v.origin).Make2D().Normalize();
            Vector2D vecTarget = (pBot->vecTargetPos - pEdict->v.origin).Make2D().Normalize();

            if (DotProduct(vecPlayer, vecTarget) > 0.9f)
            {
               pBot->vecTargetPos = Vector(0,0,0);
               break;
            }
         }
      }

      // move to the destination position
      if (pBot->vecTargetPos != Vector(0, 0, 0))
         BotMoveToPosition(pBot, pBot->vecTargetPos);
   }
   else
      pBot->vecTargetPos = Vector(0, 0, 0);

   BotFacePosition(pBot, pBot->vecLookAt);

   g_engfuncs.pfnRunPlayerMove( pEdict, pEdict->v.v_angle, pBot->f_move_speed,
      pBot->f_sidemove_speed, 0, pEdict->v.button, pEdict->v.impulse, msecval);
}

