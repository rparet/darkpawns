/* ************************************************************************
*   File: scripts.c                                     Part of CircleMUD *
*  Usage: Lua scripting APIs and Lua handling                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996-1999 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "events.h"
#include "vt100.h"
#include "shop.h"

/* functions in this file */
void table_to_char(lua_State *L);
void table_to_obj(lua_State *L);
void table_to_room(lua_State *L);
void char_to_table(lua_State *L, struct char_data *ch);
void obj_to_table(lua_State *L, struct obj_data *obj);
void room_to_table(lua_State *L, struct room_data *room, struct char_data *me);
void clear_stack(lua_State *L);
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);

/* external variables */
extern struct shop_data *shop_index;
extern struct zone_data *zone_table;
extern int top_of_zone_table;

extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct player_special_data dummy_mob;

extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern int top_of_mobt;

extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;
extern int top_of_objt;

extern struct player_index_element *player_table;
extern int top_of_p_table;


/* internal variables */
lua_State *lua_state = NULL;
#define L       lua_state

static int lua_act(lua_State *L)
{
  struct char_data *me = NULL, *vict = NULL;
  struct obj_data *obj = NULL;
  char *txt = NULL;
  int is_invis = 0, where = 0;

  void act(char *str, int hide_invisible, struct char_data *ch,
           struct obj_data *obj, void *vict_obj, int type);

  if (lua_isstring(L, 1))
    txt = (char *)lua_tostring(L, 1);

  if (lua_isnumber(L, 2))
    is_invis = (int)lua_tonumber(L, 2);

  if (lua_istable(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 3);
    me = (struct char_data *)lua_touserdata(L, -1);
  }

  if (lua_istable(L, 4)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 4);
    obj = (struct obj_data *)lua_touserdata(L, -1);
  }

  if (lua_istable(L, 5)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 5);
    vict = (struct char_data *)lua_touserdata(L, -1);
  }

  if (lua_isnumber(L, 6))
    where = (int)lua_tonumber(L, 6);

  act(txt, is_invis, me, obj, vict, where);

  return 0;
}

static int lua_action(lua_State *L)
{
  struct char_data *vict = NULL;
  char *argument = NULL;

  void command_interpreter(struct char_data *ch, char *argument);

  if (lua_istable(L, 1) && lua_isstring(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    argument = (char *)lua_tostring(L, 2);
    command_interpreter(vict, argument);
  } else
    mudlog("[Lua] Invalid argument passed to lua_action.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_aff_flagged(lua_State *L)
{
  struct char_data *plr = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isnumber(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    plr = (struct char_data *)lua_touserdata(L, -1);
    flag = (int)lua_tonumber(L, 2);

    if (AFF_FLAGGED(plr, flag))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_aff_flagged.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_aff_flags(lua_State *L)
{
  struct char_data *plr = NULL;
  char *type = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    plr = (struct char_data *)lua_touserdata(L, -1);
    type = (char *)lua_tostring(L, 2);
    flag = (int)lua_tonumber(L, 3);

    if (!strcmp(type, "set"))
      SET_BIT_AR(AFF_FLAGS(plr), flag);
    else if (!strcmp(type, "remove"))
      REMOVE_BIT_AR(AFF_FLAGS(plr), flag);
    else {
      mudlog("[Lua] Invalid set/remove to lua_aff_flags.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }
  } else
    mudlog("[Lua] Invalid argument passed to lua_aff_flags.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_canget(lua_State *L)
{
  struct obj_data *obj = NULL;
  struct char_data *ch = NULL;

  extern struct str_app_type str_app[];

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  ch = (struct char_data *)lua_touserdata(L, -1);

  if (lua_istable(L, 1)) {            /* Was an object defined? */
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    if (CAN_GET_OBJ(ch, obj))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument to lua_canget.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_cansee(lua_State *L)
{
  struct char_data *me = NULL, *vict = NULL;

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  me = (struct char_data *)lua_touserdata(L, -1);

  if (lua_istable(L, 1)) {            /* Was an victim defined? */
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    if (CAN_SEE(me, vict))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument to lua_cansee.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

/* XXX not supported yet - needs 3.0 event system */
/* static int lua_create_event(lua_State *L) */
/* { */
/*   struct char_data *me = NULL; */
/*   struct char_data *ch = NULL; */
/*   struct obj_data *obj = NULL; */
/*   struct room_data *room = NULL; */
/*   char *fname = NULL; */
/*   char *type = NULL; */
/*   char *arg = NULL; */
/*   int time = 0; */
/*   struct event_data *event; */

/*   if (lua_istable(L, 1)) { */
/*     lua_pushstring(L, "struct"); */
/*     lua_gettable(L, 1); */
/*     me = (struct char_data *)lua_touserdata(L, -1); */
/*     room = &world[me->in_room]; */
/*   } */

/*   if (lua_istable(L, 2)) { */
/*     lua_pushstring(L, "struct"); */
/*     lua_gettable(L, 2); */
/*     ch = (struct char_data *)lua_touserdata(L, -1); */
/*   } else */
/*     ch = me;      /\* This is so the script will execute correctly *\/ */

/*   if (lua_istable(L, 3)) { */
/*     lua_pushstring(L, "struct"); */
/*     lua_gettable(L, 3); */
/*     obj = (struct obj_data *)lua_touserdata(L, -1); */
/*   } */

/*   if (lua_isstring(L, 4)) */
/*     arg = (char *)lua_tostring(L, 4); */

/*   if (!me && !obj) { */
/*     mudlog("[Lua] No char or obj passed to lua_create_event.", BRF, LVL_IMMORT, FALSE); */
/*     return 0; */
/*   } */

/*   if (lua_isstring(L, 5)) */
/*     fname = (char *)lua_tostring(L, 5); */

/*   if (lua_isnumber(L, 6)) */
/*     time = (int)lua_tonumber(L, 6); */

/*   if (lua_isstring(L, 7)) */
/*     type = (char *)lua_tostring(L, 7); */

/*   /\* These should always be specified, but if they're not, the mob is probably dead *\/ */
/*   if (!fname || !type) */
/*     return 0; */

/*   CREATE(event, struct event_data, 1); */
/*   CREATE(event->script, struct script_event, 1); */

/*   event->ch = ch; */
/*   if (arg) */
/*     event->args = str_dup(arg); */
/*   event->count = PULSE_VIOLENCE * time; */
/*   event->script->me = me; */
/*   event->script->obj = obj; */
/*   event->script->room = room; */
/*   event->script->fname = fname; */
/*   event->script->type = type; */

/*   add_event(event); */
/*   return 1; */
/* } */

static int lua_direction(lua_State *L)
{
  long room1, room2, dir;
  int find_first_step(long src, long target);

  if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
    room1 = real_room((long)lua_tonumber(L, 1));
    room2 = real_room((long)lua_tonumber(L, 2));

    if (room1 <= 0 || room2 <= 0 || room1 > top_of_world || room2 > top_of_world) {
      sprintf(buf, "[Lua] Invalid room specified in lua_direction.");
      mudlog(buf, BRF, LVL_IMMORT, FALSE);
      lua_pushnil(L);
      return 0;
    }

    dir = find_first_step(room1, room2);
    lua_pushnumber(L, dir);
    return 1;
  } else
    mudlog("[Lua] Invalid arguments passed to lua_direction.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_echo(lua_State *L)
{
  struct char_data *ch = NULL;
  struct room_data *room;
  char *type = NULL, *argument = NULL;

  ACMD(do_echo);
  ACMD(do_gecho);
  void send_to_zone(char *messg, struct char_data *ch);

  if (lua_istable(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3)) {
    type = (char *)lua_tostring(L, 2);
    argument = (char *)lua_tostring(L, 3);

    if (!strcmp(type, "room")) {
      lua_pushstring(L, "struct");
      lua_gettable(L, 1);
      room = (struct room_data *)lua_touserdata(L, -1);
      send_to_room(argument, real_room(room->number));
    } else if (!strcmp(type, "outdoor"))
      send_to_outdoor(argument);
    else if (!strcmp(type, "zone")) {
      lua_pushstring(L, "struct");
      lua_gettable(L, 1);
      ch = (struct char_data *)lua_touserdata(L, -1);

      send_to_zone(argument, ch);
    } else {
      lua_pushstring(L, "struct");
      lua_gettable(L, 1);
      ch = (struct char_data *)lua_touserdata(L, -1);

      if (!strcmp(type, "local"))
        do_echo(ch, argument, 0, SCMD_ECHO);
      else if (!strcmp(type, "global"))
        do_gecho(ch, argument, 0, 0);
    }
  } else
    mudlog("[Lua] Invalid arguments passed to lua_echo.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_emote(lua_State *L)
{
  struct char_data *ch = NULL;
  char *argument = NULL;

  ACMD(do_echo);

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);

  ch = (struct char_data *)lua_touserdata(L, -1);
  argument = (char *)lua_tostring(L, 1);

  do_echo(ch, argument, find_command("emote"), SCMD_EMOTE);
  return 0;
}

static int lua_equip_char(lua_State *L)
{
  struct char_data *ch = NULL;
  struct obj_data *obj = NULL;

  int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);

  if (lua_istable(L, 1) && lua_istable(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    ch = (struct char_data *)lua_touserdata(L, -1);

    lua_pushstring(L, "struct");
    lua_gettable(L, 2);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    obj_from_char(obj);
    equip_char(ch, obj, find_eq_pos(ch, obj, NULL));
  } else
    mudlog("[Lua] Invalid arguments passed to lua_equip_char.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_exit_flags(lua_State *L)
{
  char *type = NULL;
  int door = 0, flag = 0;
  struct room_data *room;

  if (lua_istable(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3) && lua_isnumber(L, 4)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    room = (struct room_data *)lua_touserdata(L, -1);

    door = (int)lua_tonumber(L, 2);
    type = (char *)lua_tostring(L, 3);
    flag = (int)lua_tonumber(L, 4);

    if (!strcmp(type, "set"))
      SET_BIT(room->dir_option[door]->exit_info, flag);
    else if (!strcmp(type, "remove"))
      REMOVE_BIT(room->dir_option[door]->exit_info, flag);
    else {
      mudlog("[Lua] Invalid set/remove to lua_exit_flags.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }
  } else
    mudlog("[Lua] Invalid argument passed to lua_exit_flags.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_exit_flagged(lua_State *L)
{
  int door = 0, flag = 0;
  struct room_data *room;

  if (lua_istable(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    room = (struct room_data *)lua_touserdata(L, -1);

    door = (int)lua_tonumber(L, 2);
    flag = (int)lua_tonumber(L, 3);

    if (EXIT_FLAGGED(room->dir_option[door], flag))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_exit_flagged.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_extchar(lua_State *L)
{
  struct char_data *ch = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    ch = (struct char_data *)lua_touserdata(L, -1);
    extract_char(ch);
    ch = NULL;          /* This should occur anyway */
  } else
    mudlog("[Lua] Invalid char passed to lua_extchar", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_extobj(lua_State *L)
{
  struct obj_data *obj = NULL;

  if (lua_istable(L, 1)) {            /* Was an object defined? */
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);
    extract_obj(obj);
    obj = NULL;         /* This should occur anyway */
  } else
    mudlog("[Lua] Invalid object passed to lua_extobj", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_extra(lua_State *L)
{
  struct obj_data *obj = NULL;
  struct extra_descr_data *ex_desc = NULL, *new_descr = NULL, *ed = NULL;
  char *desc = NULL;

  if (lua_istable(L, 1) && lua_isstring(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    desc = (char *)lua_tostring(L, 2);

    for (ed = obj->ex_description; ed; ed = ed->next) {
      sprintf(buf, "%s%s", ed->description, desc);
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = str_dup(ed->keyword);
      new_descr->description = str_dup(buf);
      new_descr->next = ex_desc;
      ex_desc = new_descr;
    }

    obj->ex_description = ex_desc;
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_extra.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_follow(lua_State *L)
{
  struct char_data *me = NULL, *ch = NULL;
  int charm = 0;

  ACMD(do_follow);

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  me = (struct char_data *)lua_touserdata(L, -1);

  if (lua_istable(L, 1) && lua_isnumber(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    ch = (struct char_data *)lua_touserdata(L, -1);

    charm = (int)lua_tonumber(L, 2);
    do_follow(me, ch->player.name, 0, 0);

    if (charm)
      SET_BIT_AR(AFF_FLAGS(me), AFF_CHARM);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_follow.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_gossip(lua_State *L)
{
  struct char_data *me = NULL;
  char *argument = NULL;

  ACMD(do_gen_comm);

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);

  me = (struct char_data *)lua_touserdata(L, -1);
  argument = (char *)lua_tostring(L, 1);

  do_gen_comm(me, argument, 0, SCMD_GOSSIP);
  return 0;
}

static int lua_inworld(lua_State *L)
{
  struct char_data *vict = NULL;
  struct char_data *located = NULL;
  char *type, *name;
  long vnum = 0;

  extern struct char_data *character_list;

  if (lua_isstring(L, 1) && (lua_isstring(L,2) || lua_isnumber(L, 2))) {
    type = (char *)lua_tostring(L, 1);

    for (vict = character_list; vict; vict = vict->next) {
      if (!str_cmp(type, "mob")) {
        if (lua_isnumber(L, 2)) {
          vnum = (long)lua_tonumber(L, 2);
          if (GET_MOB_VNUM(vict) == vnum)
            located = vict;
        } else {
          mudlog("[Lua] Invalid argument passed to lua_inworld.", BRF, LVL_IMMORT, FALSE);
          return 0;
        }
      } else if (!str_cmp(type, "char")) {
        if (lua_isstring(L, 2)) {
          name = (char *)lua_tostring(L, 2);
          if (!strcmp(GET_NAME(vict),name))
            located = vict;
        } else {
          mudlog("[Lua] Invalid argument passed to lua_inworld.", BRF, LVL_IMMORT, FALSE);
          return 0;
        }
      } else {
        mudlog("[Lua] Invalid argument passed to lua_inworld.", BRF, LVL_IMMORT, FALSE);
        return 0;
      }
    }
    if (located)
      char_to_table(L, located);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_inworld.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_iscorpse(lua_State *L)
{
  struct obj_data *obj = NULL;

  if (lua_istable(L, 1)) {            /* Was an object defined? */
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    if (IS_CORPSE(obj))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_iscorpse.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_isfighting(lua_State *L)
{
  struct char_data *vict = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    if (FIGHTING(vict))
      char_to_table(L, FIGHTING(vict));
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_isfighting.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_ishunt(lua_State *L)
{
  struct char_data *vict = NULL;
  struct char_data *HUNTING(struct char_data *ch);

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    if (HUNTING(vict))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_ishunt.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_isnpc(lua_State *L)
{
  struct char_data *ch = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    ch = (struct char_data *)lua_touserdata(L, -1);

    if (IS_NPC(ch))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument to lua_isnpc.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_item_check(lua_State *L)
{
  struct char_data *me = NULL;
  struct obj_data *obj = NULL;
  int shop_nr, counter;
  extern int top_shop;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    lua_getglobal(L, "me");
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    me = (struct char_data *)lua_touserdata(L, -1);

    for (shop_nr = 0; shop_nr < top_shop; shop_nr++)
      if (SHOP_KEEPER(shop_nr) == me->nr)
        break;

    if (shop_nr >= top_shop) {
      mudlog("[Lua] Unable to determine shop in lua_item_check.", BRF, LVL_IMMORT, FALSE);
      lua_pushnil(L);
      return 1;
    }

    for (counter = 0; SHOP_BUYTYPE(shop_nr, counter) != NOTHING; counter++)
      if (SHOP_BUYTYPE(shop_nr, counter) == GET_OBJ_TYPE(obj)) {
        lua_pushnumber(L, TRUE);
        return 1;
      }

    lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument to lua_item_check.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_load_room(lua_State *L)
{
  struct char_data *me = NULL;
  struct room_data *room = NULL;
  long room_no = 0;

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  me = (struct char_data *)lua_touserdata(L, -1);

  if (lua_isnumber(L, 1)) {
    room_no = (long)lua_tonumber(L, 1);
    room = &world[real_room(room_no)];
    room_to_table(L, room, me);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_load_room.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_log(lua_State *L)
{
  char *txt;

  if (lua_isstring(L, 1))
    txt = (char *)lua_tostring(L, 1);
  else {
    mudlog("[Lua] Invalid argument passed to lua_log.", BRF, LVL_IMMORT, FALSE);
    return 1;
  }

  mudlog(txt, BRF, LVL_IMMORT, TRUE);
  return 0;
}

static int lua_mload(lua_State *L)
{
  struct char_data *mobile = NULL;
  int mob_vnum = 0;
  long room_no = 0, i;

  if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
    mob_vnum = (int)lua_tonumber(L, 1);
    room_no = (long)lua_tonumber(L, 2);

    if ((i = real_mobile(mob_vnum)) < 0) {
      mudlog("[Lua] Invalid mobile vnum passed to lua_mload.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }

    mobile = read_mobile(mob_vnum, VIRTUAL);
    char_to_room(mobile, real_room(room_no));
    char_to_table(L, mobile);
    return 1;
  } else
    mudlog("[Lua] Invalid arguments passed to lua_mload.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_mob_flagged(lua_State *L)
{
  struct char_data *mob = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isnumber(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    mob = (struct char_data *)lua_touserdata(L, -1);
    flag = (int)lua_tonumber(L, 2);

    if (MOB_FLAGGED(mob, flag))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_mob_flagged.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_mob_flags(lua_State *L)
{
  struct char_data *mob = NULL;
  char *type = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    mob = (struct char_data *)lua_touserdata(L, -1);
    type = (char *)lua_tostring(L, 2);
    flag = (int)lua_tonumber(L, 3);

    if (!strcmp(type, "set"))
      SET_BIT_AR(MOB_FLAGS(mob), flag);
    else if (!strcmp(type, "remove"))
      REMOVE_BIT_AR(MOB_FLAGS(mob), flag);
    else {
      mudlog("[Lua] Invalid set/remove to lua_mob_flags.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }
  } else
    mudlog("[Lua] Invalid argument passed to lua_mob_flags.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_mount(lua_State *L)
{
  struct char_data *rider = NULL, *mount = NULL;
  char *position = NULL;

  ACMD(do_ride);
  ACMD(do_dismount);
  void unmount(struct char_data *rider, struct char_data *mount);
  struct char_data *get_mount(struct char_data *ch);

  if (lua_istable(L, 1) && (lua_istable(L, 2) || lua_isnil(L, 2)) && lua_isstring(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    rider = (struct char_data *)lua_touserdata(L, -1);

    if (lua_istable(L, 2)) {
      lua_pushstring(L, "struct");
      lua_gettable(L, 2);
      mount = (struct char_data *)lua_touserdata(L, -1);
    }

    position = (char *)lua_tostring(L, 3);

    if (!strcmp(position, "ride"))
      do_ride(rider, GET_NAME(mount), 0, 0);
    else if (!strcmp(position, "dismount"))
      do_dismount(rider, NULL, 0, 0);
    else if (!strcmp(position, "unmount"))
      unmount(rider, get_mount(rider));
    else
      mudlog("[Lua] Invalid position to lua_mount.", BRF, LVL_IMMORT, FALSE);
  } else
    mudlog("[Lua] Invalid argument passed to lua_mount.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_number(lua_State *L)
{
  int from, to;

  from = (int)lua_tonumber(L, 1);
  to   = (int)lua_tonumber(L, 2);

  lua_pop(L, 1);
  lua_pop(L, 2);

  lua_pushnumber(L, (double)number(from, to));

  return 1;
}

static int lua_obj_extra(lua_State *L)
{
  struct obj_data *obj = NULL;
  char *type = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    obj = (struct obj_data *)lua_touserdata(L, -1);
    type = (char *)lua_tostring(L, 2);
    flag = (int)lua_tonumber(L, 3);

    if (!strcmp(type, "set"))
      SET_BIT_AR(GET_OBJ_EXTRA(obj), flag);
    else if (!strcmp(type, "remove"))
      REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), flag);
    else {
      mudlog("[Lua] Invalid set/remove to lua_obj_extra.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }
  } else
    mudlog("[Lua] Invalid argument passed to lua_obj_extra.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_obj_flagged(lua_State *L)
{
  struct obj_data *obj = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isnumber(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    obj = (struct obj_data *)lua_touserdata(L, -1);
    flag = (int)lua_tonumber(L, 2);

    if (OBJ_FLAGGED(obj, flag))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_obj_flagged.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_oload(lua_State *L)
{
  int vnum = 0, rnum;
  char *location;
  struct obj_data *obj = NULL;
  struct char_data *ch = NULL;

  if (lua_istable(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    ch = (struct char_data *)lua_touserdata(L, -1);
    vnum = (int)lua_tonumber(L, 2);
    location = (char *)lua_tostring(L, 3);

    if ((rnum = real_object(vnum)) < 0) { /* no such object */
      mudlog("[Lua] lua_oload returned an unknown object.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }

    obj = read_object(rnum, REAL);

    if (!str_cmp(location, "room"))
      obj_to_room(obj, ch->in_room);
    else if (!str_cmp(location, "char"))
      obj_to_char(obj, ch);
    else {
      mudlog("[Lua] Invalid location specified in lua_oload.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }

    obj_to_table(L, obj);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_oload.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_objfrom(lua_State *L)
{
  struct obj_data *obj = NULL;
  char *argument = NULL;

  if (lua_istable(L, 1) && lua_isstring(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    argument = (char *)lua_tostring(L, 2);

    if (!strcmp(argument, "room"))
      obj_from_room(obj);
    else if (!strcmp(argument, "char"))
      obj_from_char(obj);
    else if (!strcmp(argument, "obj"))
      obj_from_obj(obj);

    lua_pushuserdata(L, obj);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_objfrom.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_obj_list(lua_State *L)
{
  struct char_data *me = NULL, *vict = NULL;
  struct obj_data *obj = NULL, *list = NULL, *corpse = NULL;
  char *argument = NULL, *where = NULL;
  int items;

  int isname_with_abbrevs(char *str, char *namelist);

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  me = (struct char_data *)lua_touserdata(L, -1);

  if (lua_isstring(L, 1) && lua_isstring(L, 2)) {
    argument = (char *)lua_tostring(L, 1);
    where = (char *)lua_tostring(L, 2);

    if (!strcmp(where, "room"))                       /* In the room */
      list = world[me->in_room].contents;
    else if (!strcmp(where, "char")) {
      for (items = 0; items < NUM_WEARS; items++)     /* Worn by the char */
        if (GET_EQ(me, items))
          if (isname_with_abbrevs(argument, GET_EQ(me,items)->name)) {
            obj_to_table(L, GET_EQ(me, items));       /* Found the object, set the global */
            lua_setglobal(L, "obj");
            lua_pushnumber(L, TRUE);
            return 1;
          }
      list = me->carrying;                            /* Carried by the char */
    } else if (!strcmp(where, "vict"))                /* Carried by another char */
      for (vict = world[me->in_room].people; vict; vict = vict->next_in_room) {
        if ((obj = get_obj_in_list_vis(me, argument, vict->carrying))) {
          list = vict->carrying;
          break;
      }
      for (items = 0; items < NUM_WEARS; items++)     /* Worn by another char */
        if (GET_EQ(vict, items))
          if (isname_with_abbrevs(argument, GET_EQ(vict,items)->name)) {
            char_to_table(L, vict);
            lua_setglobal(L, "ch");
            obj_to_table(L, GET_EQ(vict, items));     /* Found the object, set the global */
            lua_setglobal(L, "obj");
            lua_pushnumber(L, TRUE);
            return 1;
          }
      }
    else if (!strcmp(where, "cont")) {                /* Within a container */
      for (corpse = world[me->in_room].contents; corpse; corpse = corpse->next_content)
        if (corpse->contains) {
          if ((obj = get_obj_in_list_vis(me, argument, corpse->contains))) {
            list = corpse->contains;
            break;
          }
        }
    }
    else if (!strcmp(where, "corpse")) {              /* Within a corpse */
      for (corpse = world[me->in_room].contents; corpse; corpse = corpse->next_content)
        if (IS_CORPSE(corpse)) {
          if ((obj = get_obj_in_list_vis(me, argument, corpse->contains))) {
            list = corpse->contains;
            break;
          }
        }
    }
    else {
      mudlog("[Lua] Invalid location to search in lua_obj_list.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }

    if ((obj = get_obj_in_list_vis(me, argument, list))) {
      obj_to_table(L, obj);                       /* Found the object, set the global */
      lua_setglobal(L, "obj");
      if (vict) {                                 /* Found a victim, set the global */
        char_to_table(L, vict);
        lua_setglobal(L, "ch");
      }
      lua_pushnumber(L, TRUE);
    } else
      lua_pushnil(L);

    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_obj_list.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_objto(lua_State *L)
{
  int vnum = 0, rnum;
  struct obj_data *obj = NULL, *into = NULL;
  struct char_data *ch = NULL;
  char *argument = NULL;

  if (lua_istable(L, 1) && lua_isstring(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    argument = (char *)lua_tostring(L, 2);

    if (!strcmp(argument, "room")) {
      vnum = (int)lua_tonumber(L, 3);
      if ((rnum = real_room(vnum)) < 0) /* No such room */
        return -1;
      if (obj)
        obj_to_room(obj, rnum);
    } else if (!strcmp(argument, "char")) {
      if (lua_istable(L, 3)) {
        lua_pushstring(L, "struct");
        lua_gettable(L, 3);
        ch = (struct char_data *)lua_touserdata(L, -1);
      }
      if (obj && ch)
        obj_to_char(obj, ch);
    } else if (!strcmp(argument, "obj")) {
      if (lua_istable(L, 3)) {
        lua_pushstring(L, "struct");
        lua_gettable(L, 3);
        into = (struct obj_data *)lua_touserdata(L, -1);
      }
      if (obj && into)
        obj_to_obj(obj, into);
    }

    lua_pushuserdata(L, obj);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_objto.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_plr_flagged(lua_State *L)
{
  struct char_data *plr = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isnumber(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    plr = (struct char_data *)lua_touserdata(L, -1);
    flag = (int)lua_tonumber(L, 2);

    if (PLR_FLAGGED(plr, flag))
      lua_pushnumber(L, TRUE);
    else
      lua_pushnil(L);
    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_plr_flagged.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_plr_flags(lua_State *L)
{
  struct char_data *plr = NULL;
  char *type = NULL;
  int flag = 0;

  if (lua_istable(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    plr = (struct char_data *)lua_touserdata(L, -1);
    type = (char *)lua_tostring(L, 2);
    flag = (int)lua_tonumber(L, 3);

    if (!strcmp(type, "set"))
      SET_BIT_AR(PLR_FLAGS(plr), flag);
    else if (!strcmp(type, "remove"))
      REMOVE_BIT_AR(PLR_FLAGS(plr), flag);
    else {
      mudlog("[Lua] Invalid set/remove to lua_plr_flags.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }
  } else
    mudlog("[Lua] Invalid argument passed to lua_plr_flags.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_raw_kill(lua_State *L)
{
  struct char_data *vict = NULL;
  struct char_data *killer = NULL;
  int type = 0;

  void raw_kill(struct char_data * ch, struct char_data * killer, int attacktype);

  if (lua_istable(L, 1) && (lua_istable(L, 2) || lua_isnil(L, 2)) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    if (lua_istable(L, 2)) {
      lua_pushstring(L, "struct");
      lua_gettable(L, 2);
      killer = (struct char_data *)lua_touserdata(L, -1);
    }

    type = (int)lua_tonumber(L, 3);
    if (!IS_NPC(vict)) {
      if (killer)
        sprintf(buf, "%s killed by %s at %s.", GET_NAME(vict), GET_NAME(killer),
          world[vict->in_room].name);
      else
        sprintf(buf, "%s killed at %s.", GET_NAME(vict), world[vict->in_room].name);
      mudlog(buf, BRF, LVL_IMMORT, FALSE);
    }
    raw_kill(vict, killer, type);
  } else
    mudlog("[Lua] Invalid arguments passed to lua_raw_kill.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_round(lua_State *L)
{
  int value;

  value = (int)lua_tonumber(L, 1);
  lua_pop(L, 1);

  lua_pushnumber(L, (double)value);
  return 1;
}

static int lua_say(lua_State *L)
{
  struct char_data *ch = NULL;
  char *argument = NULL;

  ACMD(do_say);

  if (lua_isstring(L, 1)) {
    lua_getglobal(L, "me");
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    ch = (struct char_data *)lua_touserdata(L, -1);

    argument = (char *)lua_tostring(L, 1);
    do_say(ch, argument, 0, 0);
  } else
    mudlog("[Lua] Invalid argument passed to lua_say.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_save_char(lua_State *L)
{
  struct char_data *vict = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    vict = (struct char_data *)lua_touserdata(L, -1);

    table_to_char(L);
    affect_total(vict);  // for lua code that applies affects
  } else
    mudlog("[Lua] Invalid argument passed to lua_save_char.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_save_obj(lua_State *L)
{
  struct obj_data *obj = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    table_to_obj(L);
  } else
    mudlog("[Lua] Invalid argument passed to lua_save_obj.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_save_room(lua_State *L)
{
  struct room_data *room = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    room = (struct room_data *)lua_touserdata(L, -1);

    table_to_room(L);
  } else
    mudlog("[Lua] Invalid argument passed to lua_save_room.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_set_hunt(lua_State *L)
{
  struct char_data *hunter = NULL, *vict = NULL;

  void set_hunting(struct char_data *ch, struct char_data *vict);

  if (lua_istable(L, 1) && (lua_istable(L, 2) || lua_isnil(L, 2))) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    hunter = (struct char_data *)lua_touserdata(L, -1);

    if (lua_istable(L, 2)) {
      lua_pushstring(L, "struct");
      lua_gettable(L, 2);
      vict = (struct char_data *)lua_touserdata(L, -1);
    }

    set_hunting(hunter, vict);
  } else
    mudlog("[Lua] Invalid arguments passed to lua_set_hunt.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_set_skill(lua_State *L)
{
  struct char_data *ch = NULL;
  int group_num = 0, skill_lvl = 0;
  void affect_total(struct char_data * ch);

  if (lua_istable(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);

    ch = (struct char_data *)lua_touserdata(L, -1);
    group_num = (int)lua_tonumber(L, 2);
    skill_lvl = (int)lua_tonumber(L, 3);
    SET_SKILL(ch, group_num, skill_lvl);
    affect_total(ch);
  }

  return 1;
}

static int lua_skip_spaces(lua_State *L)
{
  if (lua_isstring(L, 1))
  {
    char *text;
    text = (char *)lua_tostring(L, 1);
    skip_spaces(&text);
    lua_pushstring(L, text);
  } else
    mudlog("[Lua] Invalid argument passed to lua_skip_spaces.", BRF, LVL_IMMORT, FALSE);

  return 1;
}

static int lua_social(lua_State *L)
{
  struct char_data *me = NULL, *vict = NULL;
  int social = 0;

  ACMD(do_action);

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);

  me = (struct char_data *)lua_touserdata(L, -1);

  if (lua_istable(L, 1) && lua_isstring(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    social = find_command((char *)lua_tostring(L, 2));
    if (!social)
      mudlog("[Lua] Unknown command passed to lua_social.", BRF, LVL_IMMORT, FALSE);
    else
      do_action(me, GET_NAME(vict), social, 0);
  } else
    mudlog("[Lua] Invalid argument passed to lua_social.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_spell(lua_State *L)
{
  struct char_data *me = NULL, *vict = NULL, *ch = NULL;
  struct obj_data *obj = NULL;
  int spell = 0, vocal = 0;

  int cast_spell(struct char_data * ch, struct char_data * tch,
                 struct obj_data * tobj, int spellnum);

  lua_getglobal(L, "me");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  me = (struct char_data *)lua_touserdata(L, -1);

  lua_getglobal(L, "ch");
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  ch = (struct char_data *)lua_touserdata(L, -1);

  if ((lua_istable(L, 1) || lua_isnil(L, 1)) && (lua_istable(L, 2) || lua_isnil(L, 2)) &&
    lua_isnumber(L, 3) && lua_isnumber(L, 4)) {

    if (lua_istable(L, 1)) {
      lua_pushstring(L, "struct");
      lua_gettable(L, 1);
      vict = (struct char_data *)lua_touserdata(L, -1);
    }

    if (lua_istable(L, 2)) {            /* Was an object defined? */
      lua_pushstring(L, "struct");
      lua_gettable(L, 2);
      obj = (struct obj_data *)lua_touserdata(L, -1);
    }

    spell = (int)lua_tonumber(L, 3);
    vocal = (int)lua_tonumber(L, 4);

    if (!spell) {
      mudlog("[Lua] No spell passed to lua_spell.", BRF, LVL_IMMORT, FALSE);
      return 0;
    }

    if (vocal)
      cast_spell(me, vict, obj, spell);
    else
      call_magic(me, vict, obj, spell, GET_LEVEL(me), CAST_SPELL);

    /* Now save the char in case damage/healing was done */
    if (vict) {
      char_to_table(L, vict);
      if (vict == me)
        lua_setglobal(L, "me");
      else if (vict == ch)
        lua_setglobal(L, "ch");
    }

    return 1;
  } else
    mudlog("[Lua] Invalid argument passed to lua_spell.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_steal(lua_State *L)
{
  struct char_data *me = NULL, *vict = NULL;
  struct obj_data *obj = NULL;

  if (lua_istable(L, 1) && lua_istable(L, 2)) {
    lua_getglobal(L, "me");
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    me = (struct char_data *)lua_touserdata(L, -1);

    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    lua_pushstring(L, "struct");
    lua_gettable(L, 2);
    obj = (struct obj_data *)lua_touserdata(L, -1);

    obj_from_char(obj);
    obj_to_char(obj, me);
  } else
    mudlog("[Lua] Invalid argument passed to lua_steal.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_tport(lua_State *L)
{
  struct char_data *vict = NULL;
  struct char_data *me = NULL, *ch = NULL;
  int to_room = 0;

  if (lua_istable(L, 1) && lua_isnumber(L, 2)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    to_room = (int)lua_tonumber(L, 2);

    if (real_room(to_room) < 0) {
      mudlog("[Lua] Invalid room passed to lua_tport.", BRF, LVL_IMMORT, FALSE);
      return -1;
    }
    char_from_room(vict);
    char_to_room(vict, real_room(to_room));
    look_at_room(vict, 0);

    /* Because the globals aren't "saved", we need to resave them to update things like
       position and fighting status */

    lua_getglobal(L, "me");
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    me = (struct char_data *)lua_touserdata(L, -1);

    lua_getglobal(L, "ch");
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);
    ch = (struct char_data *)lua_touserdata(L, -1);

    char_to_table(L, vict);
    if (vict == me)
      lua_setglobal(L, "me");
    else if (vict == ch)
      lua_setglobal(L, "ch");

  } else
    mudlog("[Lua] Invalid argument passed to lua_tport.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_tell(lua_State *L)
{
  struct char_data *me = NULL;
  char *txt = NULL, *vict_name = NULL;

  ACMD(do_tell);

  if (lua_isstring(L, 1) && lua_isstring(L, 2)) {
    lua_getglobal(L, "me");
    lua_pushstring(L, "struct");
    lua_gettable(L, -2);

    me  = (struct char_data *)lua_touserdata(L, -1);
    vict_name = (char *)lua_tostring(L, 1);
    txt = (char *)lua_tostring(L, 2);

    strcpy(buf, " ");
    strcat(buf, vict_name); /* name */
    strcat(buf, " "); /* space */
    strcat(buf, txt); /* message */
    do_tell(me, buf, find_command("tell"), 0);
  } else
    mudlog("[Lua] Invalid argument passed to lua_tell.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static int lua_unaffect(lua_State *L)
{
  struct char_data *vict = NULL;

  if (lua_istable(L, 1)) {
    lua_pushstring(L, "struct");
    lua_gettable(L, 1);
    vict = (struct char_data *)lua_touserdata(L, -1);

    if (vict->affected)
        while (vict->affected)
        affect_remove(vict, vict->affected);
  } else
    mudlog("[Lua] Invalid argument passed to lua_unaffect.", BRF, LVL_IMMORT, FALSE);

  return 0;
}

static const struct luaL_reg cmdlib[] = {
  { "act"           , lua_act           },      /* act function */
  { "action"        , lua_action        },      /* Allows script to perform any command */
  { "aff_flagged"   , lua_aff_flagged   },      /* AFF_FLAGGED(ch, flag) macro */
  { "aff_flags"     , lua_aff_flags     },      /* AFF_FLAGS(ch) macro - set or remove */
  { "canget"        , lua_canget        },      /* CAN_GET_OBJ(obj) macro */
  { "cansee"        , lua_cansee        },      /* CAN_SEE(ch, vict) macro */
/*   { "create_event"  , lua_create_event  },      /\* Event delayed for (time) period *\/ */
  { "direction"     , lua_direction     },      /* find_first_step(room1, room2) function */
  { "echo"          , lua_echo          },      /* do_echo either locally, globally or outdoors */
  { "emote"         , lua_emote         },      /* emote */
  { "equip_char"    , lua_equip_char    },      /* equip_char(ch, obj) function */
  { "exit_flags"    , lua_exit_flags    },      /* EXIT(room, door) macro - set or remove */
  { "exit_flagged"  , lua_exit_flagged  },      /* EXIT_FLAGGED(room, door) macro */
  { "extchar"       , lua_extchar       },      /* extract character */
  { "extobj"        , lua_extobj        },      /* extract object */
  { "extra"         , lua_extra         },      /* modify the obj's extra desc */
  { "follow"        , lua_follow        },      /* do_follow command */
  { "gossip"        , lua_gossip        },      /* gossip */
  { "inworld"       , lua_inworld       },      /* locate a character in the world */
  { "iscorpse"      , lua_iscorpse      },      /* IS_CORPSE(obj) macro */
  { "isfighting"    , lua_isfighting    },      /* FIGHTING(ch) macro */
  { "ishunt"        , lua_ishunt        },      /* HUNTING(ch) macro */
  { "isnpc"         , lua_isnpc         },      /* ISNPC(ch) macro */
  { "item_check"    , lua_item_check    },      /* determine if a shop keeper can sell this item */
  { "load_room"     , lua_load_room     },      /* room_to_table called */
  { "log"           , lua_log           },      /* log to syslog */
  { "mload"         , lua_mload         },      /* mobile load */
  { "mob_flagged"   , lua_mob_flagged   },      /* MOB_FLAGGED(mob, flag) macro */
  { "mob_flags"     , lua_mob_flags     },      /* MOB_FLAGS(mob) macro - set or remove */
  { "mount"         , lua_mount         },      /* ride or dismount function */
  { "number"        , lua_number        },      /* random number generator */
  { "obj_extra"     , lua_obj_extra     },      /* GET_OBJ_EXTRA(obj) macro - set or remove */
  { "obj_flagged"   , lua_obj_flagged   },      /* OBJ_FLAGGED(obj, flag) macro */
  { "oload"         , lua_oload         },      /* object load */
  { "objfrom"       , lua_objfrom       },      /* obj_from_XXX based on argument */
  { "obj_list"      , lua_obj_list      },      /* get_obj_in_list_vis function */
  { "objto"         , lua_objto         },      /* obj_to_XXX based on argument */
  { "plr_flagged"   , lua_plr_flagged   },      /* PLR_FLAGGED(ch, flag) macro */
  { "plr_flags"     , lua_plr_flags     },      /* PLR_FLAGS(ch) macro - set or remove */
  { "raw_kill"      , lua_raw_kill      },      /* raw_kill(vict, killer, type) function */
  { "round"         , lua_round         },      /* round the number provided */
  { "save_char"     , lua_save_char     },      /* table_to_char called */
  { "save_obj"      , lua_save_obj      },      /* table_to_obj called */
  { "save_room"     , lua_save_room     },      /* table_to_room called */
  { "say"           , lua_say           },      /* say */
  { "set_hunt"      , lua_set_hunt      },      /* set_hunting(ch, vict) function */
  { "set_skill"     , lua_set_skill     },      /* used to set skill levels */
  { "skip_spaces"   , lua_skip_spaces   },      /* skip_spaces function */
  { "social"        , lua_social        },      /* social */
  { "spell"         , lua_spell         },      /* call_magic function */
  { "steal"         , lua_steal         },      /* steal function */
  { "tport"         , lua_tport         },      /* teleport function */
  { "tell"          , lua_tell          },      /* tell */
  { "unaffect"      , lua_unaffect      },      /* Remove all spells affections from a char */
};

int open_lua_file(char *buf, char *script_name)
{
  int err;

  if ((err = lua_dofile(L, buf)) != 0)
  {
    switch (err)
    {
      case 1:
        sprintf(buf, "[Lua] Could not call script %s: Execution failed.", script_name);
        break;
      case 2:
        sprintf(buf, "[Lua] Could not call script %s: No such file.", script_name);
        break;
      case 3:
        sprintf(buf, "[Lua] Could not call script %s: Syntax error.", script_name);
        break;
      case 4:
        sprintf(buf, "[Lua] Could not call script %s: Memory error.", script_name);
        break;
      case 5:
        sprintf(buf, "[Lua] Could not call script %s: Generic Error.", script_name);
        break;
      default:
      sprintf(buf, "[Lua] Could not call script %s: Unknown error %d.", script_name, err);
      break;
    }

    mudlog(buf, CMP, LVL_IMMORT, TRUE);
    return -1;
  }

  return 1;
}

int boot_lua()
{
  L=lua_open(0);
  lua_baselibopen(L);
  lua_mathlibopen(L);
  lua_strlibopen(L);

  /* register the functions in lua */
  luaL_openl(L, cmdlib);
  sprintf(buf, "%s/globals.lua", SCRIPT_DIR);
  open_lua_file(buf, "globals.lua");

  lua_getglobal(L, "default");
  lua_call(L, 0, 0);
  return 0;
}

int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type)
{
  int top_of_stack, retval = 0, en = 0;
  char *script_name;

  CREATE(script_name, char, MAX_STRING_LENGTH);

  top_of_stack = lua_gettop(L);

  if (ch) {
    char_to_table(L, ch);
    lua_setglobal(L, "ch");
  }

  if (me) {
    char_to_table(L, me);
    lua_setglobal(L, "me");
  }

  if (room) {
    room_to_table(L, room, me);
    lua_setglobal(L, "room");
  }

  if (obj) {
    obj_to_table(L, obj);
    lua_setglobal(L, "obj");
  }

  if (!strcmp(type, LT_MOB)) {
    if (GET_MOB_SCRIPT(me)->name)
      strcpy(script_name, GET_MOB_SCRIPT(me)->name);
  } else if (!strcmp(type, LT_ROOM)) {
    if (GET_ROOM_SCRIPT(real_room(room->number))->name)
      strcpy(script_name, GET_ROOM_SCRIPT(real_room(room->number))->name);
  } else if (!strcmp(type, LT_OBJ)) {
    if (GET_OBJ_SCRIPT(obj)->name != NULL)
      strcpy(script_name, GET_OBJ_SCRIPT(obj)->name);
  } else {  /* bad type */
    FREE(script_name);
    return TRUE;
  }

  if (!*script_name) {
    sprintf(buf, "SYSERR: Attempting to call unassigned script for %s (#%d).",
            GET_NAME(me), GET_MOB_VNUM(me));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    FREE(script_name);
    return(TRUE);
  }

  if (argument) {
    lua_pushstring(L, argument);
    lua_setglobal(L, "argument");
  }

  sprintf(buf, "%s/%s/%s", SCRIPT_DIR, type, script_name);

  if (open_lua_file(buf, script_name) == -1) { /* error */
    sprintf(buf2, "SYSERR: Error opening lua script %s.", script_name);
    mudlog(buf2, BRF, LVL_IMMORT, TRUE);
    clear_stack(L);
    if (!strcmp(type, LT_MOB))
      MOB_SCRIPT_FLAGS(me) = 0;
    else if (!strcmp(type, LT_ROOM))
      ROOM_SCRIPT_FLAGS(real_room(room->number)) = 0;
    else
      OBJ_SCRIPT_FLAGS(obj) = 0;
    FREE(script_name);
    return TRUE;
  }

  if (fname) {
    lua_settop(L, top_of_stack);
    lua_getglobal(L, fname);
    en = lua_call(L, 0, 1);
    if (en == 0) {
      retval = (int)lua_tonumber(L, -1);
      lua_setglobal(L, "retval");
    } else {
      sprintf(buf, "[Lua] Script %s being called with an error, function '%s'.", script_name, fname);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    }

  }

  if (ch && ch->in_room > 0) {
    if (ch) {
      lua_getglobal(L, "ch");
      table_to_char(L);
    }

    if (me && (me != ch)) {
      lua_getglobal(L, "me");
      table_to_char(L);
    }
  }
  clear_stack(L);

  FREE(script_name);
  return (retval);
}

void
char_to_table(lua_State *L, struct char_data *ch)
{
  struct obj_data *obj = NULL;
  int i, j;
  bool wear = FALSE;

  int num_followers(struct char_data *ch);

  lua_newtable(L);
  lua_pushstring(L, "name"); lua_pushstring(L, GET_NAME(ch)); lua_settable(L, -3);
  lua_pushstring(L, "align"); lua_pushnumber(L, GET_ALIGNMENT(ch)); lua_settable(L, -3);
  lua_pushstring(L, "gold"); lua_pushnumber(L, GET_GOLD(ch)); lua_settable(L, -3);
  lua_pushstring(L, "level"); lua_pushnumber(L, GET_LEVEL(ch)); lua_settable(L, -3);
  lua_pushstring(L, "hp"); lua_pushnumber(L, GET_HIT(ch)); lua_settable(L, -3);
  lua_pushstring(L, "maxhp"); lua_pushnumber(L, GET_MAX_HIT(ch)); lua_settable(L, -3);
  lua_pushstring(L, "pos"); lua_pushnumber(L, GET_POS(ch)); lua_settable(L, -3);
  lua_pushstring(L, "alias"); lua_pushstring(L, ch->player.name); lua_settable(L, -3);

  if (ch->carrying) {
    lua_pushstring(L, "objs"); lua_newtable(L);
    for (i = 1, obj = ch->carrying; obj; i++, obj = obj->next_content) {
      lua_pushnumber(L, i); obj_to_table(L, obj); lua_settable(L, -3);
    }
    lua_settable(L, -3);
  }

  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i)) {
      wear = TRUE;
      break;
    }

  if (wear) {
    lua_pushstring(L, "wear"); lua_newtable(L);
    for (i = 0, j = 1; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
        lua_pushnumber(L, j); obj_to_table(L, GET_EQ(ch, i)); lua_settable(L, -3);
        j++;
      }
    lua_settable(L, -3);
  }

  if (IS_EVIL(ch))
    { lua_pushstring(L, "evil"); lua_pushnumber(L, TRUE); lua_settable(L, -3); }
  else
    { lua_pushstring(L, "evil"); lua_pushnumber(L, FALSE); lua_settable(L, -3); }

  if (!ch->master || ch->master == ch)
    { lua_pushstring(L, "leader"); lua_pushnil(L); lua_settable(L, -3); }
  else
    { lua_pushstring(L, "leader"); char_to_table(L, ch->master); lua_settable(L, -3); }

  if (!IS_NPC(ch)) {
    lua_pushstring(L, "id"); lua_pushnumber(L, GET_IDNUM(ch)); lua_settable(L, -3);
    lua_pushstring(L, "exp"); lua_pushnumber(L, GET_EXP(ch)); lua_settable(L, -3);
    lua_pushstring(L, "bank"); lua_pushnumber(L, GET_BANK_GOLD(ch)); lua_settable(L, -3);
    lua_pushstring(L, "mana"); lua_pushnumber(L, GET_MANA(ch)); lua_settable(L, -3);
    lua_pushstring(L, "move"); lua_pushnumber(L, GET_MOVE(ch)); lua_settable(L, -3);
    lua_pushstring(L, "cha"); lua_pushnumber(L, GET_CHA(ch)); lua_settable(L, -3);
    lua_pushstring(L, "jail"); lua_pushnumber(L, GET_JAIL_TIMER(ch)); lua_settable(L, -3);
    lua_pushstring(L, "tattoo"); lua_pushnumber(L, GET_TATTOO(ch)); lua_settable(L, -3);
    lua_pushstring(L, "followers"); lua_pushnumber(L, num_followers(ch)); lua_settable(L, -3);
  } else {
    lua_pushstring(L, "vnum"); lua_pushnumber(L, GET_MOB_VNUM(ch)); lua_settable(L, -3);
    lua_pushstring(L, "timer"); lua_pushnumber(L, GET_MOB_WAIT(ch)); lua_settable(L, -3);
  }

  lua_pushstring(L, "struct"); lua_pushuserdata(L, ch); lua_settable(L, -3);
}

void
obj_to_table(lua_State *L, struct obj_data *obj)
{
  int i;
  struct obj_data *cont = NULL;

  lua_newtable(L);

  lua_pushstring(L, "name"); lua_pushstring(L, obj->short_description); lua_settable(L, -3);
  lua_pushstring(L, "alias"); lua_pushstring(L, obj->name); lua_settable(L, -3);
  lua_pushstring(L, "vnum"); lua_pushnumber(L, GET_OBJ_VNUM(obj)); lua_settable(L, -3);
  lua_pushstring(L, "cost"); lua_pushnumber(L, GET_OBJ_COST(obj)); lua_settable(L, -3);
  lua_pushstring(L, "type"); lua_pushnumber(L, GET_OBJ_TYPE(obj)); lua_settable(L, -3);
  lua_pushstring(L, "weight"); lua_pushnumber(L, GET_OBJ_WEIGHT(obj)); lua_settable(L, -3);
  lua_pushstring(L, "perc_load"); lua_pushnumber(L, GET_OBJ_LOAD(obj)); lua_settable(L, -3);
  lua_pushstring(L, "timer"); lua_pushnumber(L, GET_OBJ_TIMER(obj)); lua_settable(L, -3);

  lua_pushstring(L, "val"); lua_newtable(L);
  for (i = 1; i < 5; i++) {
    lua_pushnumber(L, i); lua_pushnumber(L, GET_OBJ_VAL(obj, (i - 1))); lua_settable(L, -3);
  }
  lua_settable(L, -3);

  if (obj->contains) {
    lua_pushstring(L, "contents"); lua_newtable(L);
    for (i = 1, cont = obj->contains; cont; i++, cont = cont->next_content) {
      lua_pushnumber(L, i); obj_to_table(L, cont); lua_settable(L, -3);
    }
    lua_settable(L, -3);
  }

  lua_pushstring(L, "struct"); lua_pushuserdata(L, obj); lua_settable(L, -3);
}

void
room_to_table(lua_State *L, struct room_data *room, struct char_data *me)
{
  int i;
  struct char_data *ppl = NULL;
  struct obj_data *obj = NULL;

  lua_newtable(L);
  lua_pushstring(L, "vnum"); lua_pushnumber(L, room->number); lua_settable(L, -3);
  lua_pushstring(L, "sect"); lua_pushnumber(L, room->sector_type); lua_settable(L, -3);

  lua_pushstring(L, "exit"); lua_newtable(L);
  for (i = 0; i < NUM_OF_DIRS; i++) {
    lua_pushnumber(L, i);
    if (room->dir_option[i])
      lua_pushnumber(L, GET_ROOM_VNUM(room->dir_option[i]->to_room));
    else
      lua_pushnil(L);
    lua_settable(L, -3);
  }
  lua_settable(L, -3);

  for (i = 1, ppl = room->people; ppl; i++, ppl = ppl->next_in_room)
    if (ppl == me)
      continue;

  if (i >= 1) {              /* Don't include "me" in this list */
    lua_pushstring(L, "char"); lua_newtable(L);
    for (i = 1, ppl = room->people; ppl; i++, ppl = ppl->next_in_room)
      if (ppl != me) {
        lua_pushnumber(L, i); char_to_table(L, ppl); lua_settable(L, -3);
      } else
        i--;                /* Decrement the index for missing "me" */
    lua_settable(L, -3);
  }

  if (room->contents) {
    lua_pushstring(L, "objs"); lua_newtable(L);
    for (i = 1, obj = room->contents; obj; i++, obj = obj->next_content) {
      lua_pushnumber(L, i); obj_to_table(L, obj); lua_settable(L, -3);
    }
    lua_settable(L, -3);
  }

  lua_pushstring(L, "struct"); lua_pushuserdata(L, room); lua_settable(L, -3);
}

void
table_to_char(lua_State *L)
{
  struct char_data *ch;

  lua_pushvalue(L, 1);
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  ch = (struct char_data *)lua_touserdata(L, -1);

  /* If mob/PC died during the script, we don't want to set previous values! */
  if (MOB_FLAGGED(ch, MOB_EXTRACT) || PLR_FLAGGED(ch, PLR_EXTRACT))
    return;

  lua_pushvalue(L, 1);
  lua_pushstring(L, "align");
  lua_gettable(L, -2);
  GET_ALIGNMENT(ch) = MAX(-1000, MIN(1000, (int)lua_tonumber(L, -1)));

  lua_pushvalue(L, 1);
  lua_pushstring(L, "gold");
  lua_gettable(L, -2);
  GET_GOLD(ch) = MAX(0, (int)lua_tonumber(L, -1));

  lua_pushvalue(L, 1);
  lua_pushstring(L, "level");
  lua_gettable(L, -2);
  GET_LEVEL(ch) = MAX(0, (int)lua_tonumber(L, -1));

  lua_pushvalue(L, 1);
  lua_pushstring(L, "hp");
  lua_gettable(L, -2);
  GET_HIT(ch) = (int)lua_tonumber(L, -1);

  lua_pushvalue(L, 1);
  lua_pushstring(L, "pos");
  lua_gettable(L, -2);
  GET_POS(ch) = MAX(0, (int)lua_tonumber(L, -1));

  if (IS_NPC(ch)) {
    lua_pushvalue(L, 1);
    lua_pushstring(L, "timer");
    lua_gettable(L, -2);
    GET_MOB_WAIT(ch) = MAX(0, (int)lua_tonumber(L, -1));
  }

  if (!IS_NPC(ch)) {
    lua_pushvalue(L, 1);
    lua_pushstring(L, "bank");
    lua_gettable(L, -2);
    GET_BANK_GOLD(ch) = MAX(0, (int)lua_tonumber(L, -1));

    lua_pushvalue(L, 1);
    lua_pushstring(L, "exp");
    lua_gettable(L, -2);
    GET_EXP(ch) = MAX(0, (int)lua_tonumber(L, -1));

    lua_pushvalue(L, 1);
    lua_pushstring(L, "mana");
    lua_gettable(L, -2);
    GET_MANA(ch) = (int)lua_tonumber(L, -1);

    lua_pushvalue(L, 1);
    lua_pushstring(L, "move");
    lua_gettable(L, -2);
    GET_MOVE(ch) = (int)lua_tonumber(L, -1);

    lua_pushvalue(L, 1);
    lua_pushstring(L, "jail");
    lua_gettable(L, -2);
    GET_JAIL_TIMER(ch) = MAX(0, (int)lua_tonumber(L, -1));

    lua_pushvalue(L, 1);
    lua_pushstring(L, "tattoo");
    lua_gettable(L, -2);
    GET_TATTOO(ch) = MAX(0, (int)lua_tonumber(L, -1));
  }
}

void
table_to_obj(lua_State *L)
{
  struct obj_data *obj = NULL;
  int i;

  lua_pushvalue(L, 1);
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  obj = (struct obj_data *)lua_touserdata(L, -1);

  lua_pushvalue(L, 1);
  lua_pushstring(L, "name");
  lua_gettable(L, -2);
  strcpy(obj->short_description, (char *)lua_tostring(L, -1));

  lua_pushvalue(L, 1);
  lua_pushstring(L, "cost");
  lua_gettable(L, -2);
  GET_OBJ_COST(obj) = MAX(0, (int)lua_tonumber(L, -1));

  lua_pushvalue(L, 1);
  lua_pushstring(L, "weight");
  lua_gettable(L, -2);
  GET_OBJ_WEIGHT(obj) = MAX(1, (int)lua_tonumber(L, -1));

  for (i = 1; i < 5; i++) {
    lua_pushvalue(L, 1);      /* Reload the "val" table each time */
    lua_pushstring(L, "val");
    lua_gettable(L, -2);

    lua_pushnumber(L, i);
    lua_gettable(L, -2);
    GET_OBJ_VAL(obj, (i - 1)) = (int)lua_tonumber(L, -1);
  }

  lua_pushvalue(L, 1);
  lua_pushstring(L, "timer");
  lua_gettable(L, -2);
  GET_OBJ_TIMER(obj) = MAX(0, (int)lua_tonumber(L, -1));
}

void
table_to_room(lua_State *L)
{
  struct room_data *room = NULL;

  lua_pushvalue(L, 1);
  lua_pushstring(L, "struct");
  lua_gettable(L, -2);
  room = (struct room_data *)lua_touserdata(L, -1);

  lua_pushvalue(L, 1);
  lua_pushstring(L, "sect");
  lua_gettable(L, -2);
  room->sector_type = (int)lua_tonumber(L, -1);
}

void clear_stack(lua_State *L)
{
  while (lua_gettop(L))  /* zero = stack empty */
    lua_pop(L, -1);
}
