/***
*
*    Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*    This product contains software technology licensed from Id 
*    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*    All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// util.cpp
//

#include "bot.h"

int gmsgTextMsg = 0;
int gmsgSayText = 0;
int gmsgShowMenu = 0;

Vector UTIL_VecToAngles( const Vector &vec )
{
   float rgflVecOut[3];
   VEC_TO_ANGLES(vec, rgflVecOut);
   return Vector(rgflVecOut);
}

// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr )
{
   TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass?0x100:0), pentIgnore, ptr );
}


void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
   TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr );
}


void UTIL_MakeVectors( const Vector &vecAngles )
{
   MAKE_VECTORS( vecAngles );
}


edict_t *UTIL_FindEntityInSphere( edict_t *pentStart, const Vector &vecCenter, float flRadius )
{
   edict_t  *pentEntity;

   pentEntity = FIND_ENTITY_IN_SPHERE( pentStart, vecCenter, flRadius);

   if (!FNullEnt(pentEntity))
      return pentEntity;

   return nullptr;
}


edict_t *UTIL_FindEntityByString( edict_t *pentStart, const char *szKeyword, const char *szValue )
{
   edict_t *pentEntity;

   pentEntity = FIND_ENTITY_BY_STRING( pentStart, szKeyword, szValue );

   if (!FNullEnt(pentEntity))
      return pentEntity;
   return nullptr;
}

edict_t *UTIL_FindEntityByClassname( edict_t *pentStart, const char *szName )
{
   return UTIL_FindEntityByString( pentStart, "classname", szName );
}

edict_t *UTIL_FindEntityByTargetname( edict_t *pentStart, const char *szName )
{
   return UTIL_FindEntityByString( pentStart, "targetname", szName );
}


int UTIL_PointContents( const Vector &vec )
{
   return POINT_CONTENTS(vec);
}


void UTIL_SetSize( entvars_t *pev, const Vector &vecMin, const Vector &vecMax )
{
   SET_SIZE( ENT(pev), vecMin, vecMax );
}


void UTIL_SetOrigin( entvars_t *pev, const Vector &vecOrigin )
{
   SET_ORIGIN(ENT(pev), vecOrigin );
}


void ClientPrint( edict_t *pEntity, int msg_dest, const char *msg_name)
{
   if (gmsgTextMsg == 0)
      gmsgTextMsg = REG_USER_MSG( "TextMsg", -1 );

   MESSAGE_BEGIN( MSG_ONE_UNRELIABLE, gmsgTextMsg, nullptr, pEntity );
   WRITE_BYTE( msg_dest );
   WRITE_STRING( msg_name );
   MESSAGE_END();
}

void UTIL_SayText( const char *pText, edict_t *pEdict )
{
   if (gmsgSayText == 0)
      gmsgSayText = REG_USER_MSG( "SayText", -1 );

   MESSAGE_BEGIN( MSG_ONE_UNRELIABLE, gmsgSayText, nullptr, pEdict );
   WRITE_BYTE( ENTINDEX(pEdict) );
   WRITE_STRING( pText );
   MESSAGE_END();
}


void UTIL_HostSay( edict_t *pEntity, char *message )
{
   int   j;
   char  text[128];
   char *pc;
   edict_t *client;

   // make sure the text has content
   for ( pc = message; pc != nullptr && *pc != 0; pc++ )
   {
      if ( isprint( *pc ) && !isspace( *pc ) )
      {
         pc = nullptr;   // we've found an alphanumeric character,  so text is valid
         break;
      }
   }

   if ( pc != nullptr)
      return;  // no character found, so say nothing

   // turn on color set 2  (color on,  no sound)
   sprintf( text, "%c%s: ", 2, STRING( pEntity->v.netname ) );

   j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
   if ( static_cast<int>(strlen(message)) > j )
      message[j] = 0;

   strcat( text, message );
   strcat( text, "\n" );

   // loop through all players
   // Start with the first player.
   // This may return the world in single player if the client types something between levels or during spawn
   // so check it, or it will infinite loop

   if (gmsgSayText == 0)
      gmsgSayText = REG_USER_MSG( "SayText", -1 );

   client = nullptr;
   while ( ((client = UTIL_FindEntityByClassname( client, "player" )) != nullptr) &&
           (!FNullEnt(client)) ) 
   {
      if ( client == pEntity )  // skip sender of message
         continue;

      MESSAGE_BEGIN( MSG_ONE_UNRELIABLE, gmsgSayText, nullptr, client );
         WRITE_BYTE( ENTINDEX(pEntity) );
         WRITE_STRING( text );
      MESSAGE_END();
   }

   // print to the sending client
   MESSAGE_BEGIN( MSG_ONE_UNRELIABLE, gmsgSayText, nullptr, pEntity );
      WRITE_BYTE( ENTINDEX(pEntity) );
      WRITE_STRING( text );
   MESSAGE_END();
}


#ifdef   DEBUG
edict_t *DBG_EntOfVars( const entvars_t *pev )
{
   if (pev->pContainingEntity != NULL)
      return pev->pContainingEntity;
   ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
   edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
   if (pent == NULL)
      ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");
   ((entvars_t *)pev)->pContainingEntity = pent;
   return pent;
}
#endif //DEBUG


int UTIL_GetBotIndex(edict_t *pEdict)
{
   int index;

   for (index=0; index < 32; index++)
   {
      if (bots[index].pEdict == pEdict)
      {
         return index;
      }
   }

   return -1;  // return -1 if edict is not a bot
}


bot_t *UTIL_GetBotPointer(edict_t *pEdict)
{
   int index;

   for (index=0; index < 32; index++)
   {
      if (bots[index].pEdict == pEdict)
      {
         break;
      }
   }

   if (index < 32)
      return (&bots[index]);

   return nullptr;  // return NULL if edict is not a bot
}


bool IsAlive(edict_t *pEdict)
{
   return (pEdict->v.deadflag == DEAD_NO && pEdict->v.health > 0 &&
           !(pEdict->v.flags & FL_NOTARGET) && pEdict->v.takedamage != 0.0f);
}

bool FInViewCone(Vector *pOrigin, edict_t *pEdict)
{
   Vector2D vec2LOS;
   float    flDot;

   UTIL_MakeVectors ( pEdict->v.angles );

   vec2LOS = ( *pOrigin - pEdict->v.origin ).Make2D();
   vec2LOS = vec2LOS.Normalize();

   flDot = DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

   if ( flDot > 0.50f )  // 60 degree field of view 
      return TRUE;
   else
      return FALSE;
}


bool FVisible( const Vector &vecOrigin, edict_t *pEdict )
{
   TraceResult tr;
   Vector      vecLookerOrigin;

   // look through caller's eyes
   vecLookerOrigin = pEdict->v.origin + pEdict->v.view_ofs;

   const int bInWater = (UTIL_PointContents (vecOrigin) == CONTENTS_WATER);
   const int bLookerInWater = (UTIL_PointContents (vecLookerOrigin) == CONTENTS_WATER);

   // don't look through water
   if (bInWater != bLookerInWater)
      return FALSE;

   UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, pEdict, &tr);

   if (tr.flFraction != 1.0f)
      return FALSE;  // Line of sight is not established
   else
      return TRUE;  // line of sight is valid.
}

Vector Center(edict_t * pEdict)
{
	return Vector();
}


Vector GetGunPosition(edict_t *pEdict)
{
   return (pEdict->v.origin + pEdict->v.view_ofs);
}


Vector VecBModelOrigin(edict_t *pEdict)
{
   return pEdict->v.absmin + (pEdict->v.size * 0.5);
}


void UTIL_BuildFileName(char *filename, char *arg1, char *arg2)
{
   GET_GAME_DIR(filename);
   strcat(filename, "/");

   if ((arg1) && (*arg1) && (arg2) && (*arg2))
   {
      strcat(filename, arg1);
      strcat(filename, "/");
      strcat(filename, arg2);
   }
   else if ((arg1) && (*arg1))
   {
      strcat(filename, arg1);
   }
}

void ClampAngle(float &angle)
{
   if (angle >= 180)
      angle -= 360 * static_cast<int>(angle / 360 + 0.5);
   if (angle < 180)
      angle += 360 * static_cast<int>(-angle / 360 + 0.5);
}

void ClampAngles(Vector &angles)
{
   ClampAngle(angles.x);
   ClampAngle(angles.y);
   angles.z = 0.0;
}
