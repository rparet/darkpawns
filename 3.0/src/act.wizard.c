/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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

/* $Id: act.wizard.c 1273 2005-12-20 05:19:42Z mcooper $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "olc.h"
#include "faction.h"
#include "constants.h"
#include "shop.h"
#include <mysql/mysql.h>

/*   external vars  */
extern int game_restrict;
extern int isname_with_abbrevs(char *str, char *namelist);
extern struct time_info_data time_info;
extern int min_exp(int level);
extern struct spell_info_type spell_info[];
extern struct shop_data *shop_index;

/* for chars */
extern char *tattoos[];
extern char *dyes[];

/* functions */
ACMD(do_look);
char * get_reagent_names(int spellnum);
long exp_needed_for_level(struct char_data *ch);
void tattoo_af(struct char_data *ch, bool add);
struct char_data *HUNTING(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);
void House2_save_all();
extern int is_shopkeeper(struct char_data * chChar);
char *delete_ansi_controls(char *string);
int find_exp(int level);
void raw_kill(struct char_data * ch, struct char_data * killer, int attacktype);
void shop_control_save(void);
void sql_save_char(struct char_file_u *st);

/*****************************************/ 


struct no_load_type 
{
  int item_num;
  int min_level;
};
typedef struct no_load_type load_limits;

static load_limits ObjLimits[] = 
{
        {81,   LEVEL_GRGOD},    		/* Tracers ring */
        {82,   LEVEL_GRGOD},			/* Sword of doom */
      {8095,   LEVEL_GRGOD},			/* Tracer's master key */
};

int num_objlim = sizeof (ObjLimits) / sizeof (load_limits);

load_limits MobLimits[] = 
{
/*        1,  LEVEL_GRGOD, */                  /* Puff */
};

int num_moblim = sizeof (MobLimits) / sizeof (load_limits);

/*****************************************/ 

ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes.. but what?\r\n", ch);
  else {
    if (subcmd == SCMD_EMOTE)
    {
      if (PLR_FLAGGED(ch, PLR_NOSHOUT) || GET_INT(ch) == 0)
      {
        stc("You try to express yourself but cannot!\r\n", ch);
        return;
      }  
      sprintf(buf, "$n %s", argument);
    }
    else
      strcpy(buf, argument);
    delete_ansi_controls(buf);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}


ACMD(do_send)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
sh_int find_target_room(struct char_data * ch, char *rawroomstr)
{
  int tmp;
  long location = NOWHERE;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return NOWHERE;
  }
  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) 
  {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else if (target_obj->carried_by != NULL)
    {
      if ((target_obj->carried_by)->in_room != NOWHERE)
        location = (target_obj->carried_by)->in_room;
    }
    else if (target_obj->worn_by != NULL)
    {
      if ((target_obj->worn_by)->in_room != NOWHERE)
        location = (target_obj->worn_by)->in_room;
    }
    else if (target_obj->in_obj != NULL)
    {
      if ((target_obj->in_obj)->in_room != NOWHERE)
        location = (target_obj->in_obj)->in_room;
    }
    else {
      send_to_char("That object is not available.\r\n", ch);
      return NOWHERE;
    }
  } else {
    send_to_char("No such creature or object around.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (GET_LEVEL(ch) < LVL_GRGOD) {
    if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
	world[location].people && world[location].people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return NOWHERE;
    }
  }
  return location;
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  int location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("What do you want to do there?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  struct char_data *get_mount(struct char_data *ch);

  sh_int location;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (POOFOUT(ch))
    sprintf(buf, "%s", POOFOUT(ch));
  else
    strcpy(buf, "$n disappears in a puff of smoke.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);
  if (get_mount(ch))
    {
	char_from_room(get_mount(ch));
	char_to_room(get_mount(ch), location);
    }

  if (POOFIN(ch))
    sprintf(buf, "%s", POOFIN(ch));
  else
    strcpy(buf, "$n appears with an ear-splitting bang.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}



ACMD(do_trans)
{
  struct char_data *get_mount(struct char_data *ch);

  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf)))
      send_to_char(NOPERSON, ch);
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char("Go transfer someone your own size.\r\n", ch);
	return;
      }
      act("$n disappears in a blaze of hellfire!",FALSE,victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);

      if (ROOM_FLAGGED(ch->in_room, ROOM_DEATH))    /* Stops crashing */
        return;

      if (get_mount(victim))
       {
        char_from_room(get_mount(victim));
        char_to_room(get_mount(victim), ch->in_room);
       }
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);
    }
  } else {			/* Trans All */
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char("I think not.\r\n", ch);
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && i->character != ch) {
	victim = i->character;
	if (GET_LEVEL(victim) >= GET_LEVEL(ch))
	  continue;
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, ch->in_room);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	look_at_room(victim, 0);
      }
    send_to_char(OK, ch);
  }
}



ACMD(do_teleport)
{
  struct char_data *victim;
  sh_int target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    send_to_char(OK, ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
  }
}



ACMD(do_vnum)
{
  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj"))) {
    send_to_char("Usage: vnum { obj | mob } <name>\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
    if (!vnum_mobile(buf2, ch))
      send_to_char("No mobiles by that name.\r\n", ch);

  if (is_abbrev(buf, "obj"))
    if (!vnum_object(buf2, ch))
      send_to_char("No objects by that name.\r\n", ch);
}



void do_stat_room(struct char_data * ch)
{
  struct extra_descr_data *desc;
  struct room_data *rm = &world[ch->in_room];
  int i, found = 0;
  struct obj_data *j = 0;
  struct char_data *k = 0;

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zone: [%3d], VNum: [%s%5ld%s], RNum: [%5ld], Type: %s\r\n",
	  rm->zone, CCGRN(ch, C_NRM), rm->number, CCNRM(ch, C_NRM), ch->in_room, buf2);
  send_to_char(buf, ch);

  sprintbit(rm->room_flags, room_bits, buf2);
  sprintf(buf, "Room Script: %s, SpecProc: %s, Flags: %s Light: [%d]\r\n",
    (GET_ROOM_SCRIPT(ch->in_room)->name == NULL) ? "None" : GET_ROOM_SCRIPT(ch->in_room)->name,
    (rm->func == NULL) ? "None" : "Exists", buf2, rm->light);
  send_to_char(buf, ch);

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%5ld%s", CCCYN(ch, C_NRM),
		world[rm->dir_option[i]->to_room].number, CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	   rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
    }
  }
}



void do_stat_object(struct char_data * ch, struct obj_data * j)
{
  int i, virtual, found;
  struct obj_data *j2;
  struct extra_descr_data *desc;
  float aps = 0.0;

  virtual = GET_OBJ_VNUM(j);
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
  if (GET_OBJ_TYPE(j) == ITEM_WEAPON)
    strcpy(buf3, GET_OBJ_VAL(j, 0) ? weapon_names[GET_OBJ_VAL(j, 0)] : "None");
  else
    strcpy(buf3, "None");
  if (GET_OBJ_RNUM(j) >= 0)
    strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, Subtype: %s, SpecProc: %s\r\n",
   CCGRN(ch, C_NRM), virtual, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf3, buf2);
  send_to_char(buf, ch);

  sprintf(buf, "Uuid: [%s]\r\n", (j->id ? j->id : "None"));
  send_to_char(buf, ch);

  sprintf(buf, "L-Desc: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Can be worn on: ", ch);
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Extra flags   : ", ch);
  sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Encumbrance: %d, Value: %d, Percent Load: %d%%, Timer: %d, Craft: %s\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_LOAD(j), GET_OBJ_TIMER(j),
     craft_level[GET_CRAFT_VAL(j)]);
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%ld%s", GET_ROOM_VNUM(IN_ROOM(j)), IS_OBJ_STAT(j, ITEM_BURIED) ? " (B)" : "");
    strcat(buf, buf2);
  }
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Worn by: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "Hours left: Infinite");
    else
      sprintf(buf, "Hours left: [%ld]", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "Spells: (Level %ld) %s, %s, %s", GET_OBJ_VAL(j, 0),
	    skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
	    skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_STAFF:
    sprintf(buf, "Spell: %s at level %ld, %ld (of %ld) charges remaining",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    aps = (((GET_OBJ_VAL(j, 2) + 1) / 2.0) * GET_OBJ_VAL(j, 1));
    sprintf(buf, "Todam: %ldd%ld, Message type: %ld, Aps: %.1f, Combat Speed: %d, Rating: %.1f",
            GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3),
            aps, GET_OBJ_SPEED(j), (aps / GET_OBJ_SPEED(j)));
    break;
  case ITEM_WAND:
    sprintf(buf, "Mana modifier: %ld", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_SOUL_GEM:
    sprintf(buf, "Level: %ld, Soul: %s",
      GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1) == -1 ? "Yes" : "No");
    break;
  case ITEM_ARMOR:
    sprintf(buf, "AC-apply: [%ld]", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "Capacity: %ld, Lock Type: %s, Key Num: %ld, Corpse: %s",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "Capacity: %ld, Contains: %ld, Poisoned: %s, Liquid: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
	    buf2);
    break;
  case ITEM_NOTE:
    sprintf(buf, "Tongue: %ld", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    strcpy(buf, "");
    break;
  case ITEM_FOOD:
    sprintf(buf, "Makes full: %ld, Poisoned: %s", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "Coins: %ld", GET_OBJ_VAL(j, 0));
    break;
  default:
    sprintf(buf, "Values 0-4: [%ld] [%ld] [%ld] [%ld] [%ld]",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3), GET_OBJ_VAL(j, 4));
    break;
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  if (IS_CORPSE(j)) {
    sprintf(buf1, j->owner ? j->owner : "None");
    sprintf(buf2, j->killer ? j->killer : "None");
    sprintf(buf, "Owner: &r%s&n  Killer: &r%s&n", buf1, buf2);
    send_to_char(strcat(buf, "\r\n"), ch);
  }

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

  if (j->contains) {
    sprintf(buf, "\r\nContents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier && j->affected[i].location!=APPLY_SPELL) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
/* depricated 20011119 - replacement of PERM AFFECTS with CHAR BITS
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier && j->affected[i].location==APPLY_SPELL) {
      sprintf(buf, "%s casts permanent %s", found++ ? "," : "",
	      affected_bits[j->affected[i].modifier]);
      send_to_char(buf, ch);
    }
*/

  if (!found)
    send_to_char(" None", ch);
  send_to_char("\r\n", ch);

  if (GET_OBJ_RNUM(j) >= 0) {
    sprintf(buf, "Script: %s\r\n", (GET_OBJ_SCRIPT(j)->name ? GET_OBJ_SCRIPT(j)->name : "None"));
    send_to_char(buf, ch);
  }

  found = 0;
  stc("Requirements:", ch);
  for (i = 0; i < MAX_OBJ_REQS; i++)
    if (j->reqs[i].modifier) {
      sprinttype(j->reqs[i].location, req_types, buf2);
      sprintf(buf, "%s %d in %s", found++ ? "," : "",
              j->reqs[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);
  send_to_char("\r\n", ch);

}


char *mob_race_name(int mob_race)
{
  if (mob_race < 0)
    return (str_dup("None"));
  return(str_dup(npc_races[mob_race].race));
}

void do_stat_character(struct char_data * ch, struct char_data * k)
{
  int i, i2, shop_nr, found = 0;
  struct obj_data *j;
  struct follow_type *fol;
  struct char_data *mount;
  struct master_affected_type *aff;
  struct account_data *account;
  extern struct attack_hit_type attack_hit_text[];
  extern char *hometowns[];
  extern int top_shop;
  extern struct account_data *sql_find_account(long id);

  switch (GET_SEX(k)) {
    case SEX_NEUTRAL:    strcpy(buf, "NEUTRAL-SEX");   break;
    case SEX_MALE:       strcpy(buf, "MALE");          break;
    case SEX_FEMALE:     strcpy(buf, "FEMALE");        break;
    default:             strcpy(buf, "ILLEGAL-SEX!!"); break;
  }

  sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%5ld]\r\n",
    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
    GET_NAME(k), GET_IDNUM(k), 
    k->in_room>-1 ? world[k->in_room].number : -1);
  send_to_char(strcat(buf, buf2), ch);

  if (IS_MOB(k)) {
    sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d], Loaded In: [%5ld]\r\n",
      k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k), GET_LOADED_IN(k));
    send_to_char(buf, ch);
  }

  if (!IS_MOB(k)) {
    sprintf(buf, "Title: %s\r\n", (*k->player.title ? k->player.title : "<None>"));
    if (k->desc) {
      *buf2 = '\0';
      for (i = 0; i < k->desc->account->num_chars; i++)
        if (GET_IDNUM(k) != k->desc->account->chars[i])
          sprintf(buf2 + strlen(buf2), "%s ", CAP(get_name_by_id(k->desc->account->chars[i])));

      sprintf(buf + strlen(buf), "Account: %s%s%s    Other Characters: %s%s%s\r\n",
        CCGRN(ch, C_NRM), CAP(k->desc->account->name), CCNRM(ch, C_NRM),
        CCGRN(ch, C_NRM), *buf2 ? buf2 : "<None>", CCNRM(ch, C_NRM));
    } else {
      if (!(account = sql_find_account(GET_IDNUM(k))))
        sprintf(buf + strlen(buf), "Account: %sUnknown account details!%s\r\n",
          CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
      else {
        *buf2 = '\0';
        for (i = 0; i < account->num_chars; i++)
          if (GET_IDNUM(k) != account->chars[i])
            sprintf(buf2 + strlen(buf2), "%s ", CAP(get_name_by_id(account->chars[i])));

        sprintf(buf + strlen(buf), "Account: %s%s%s (%sOffline%s)   Other Characters: %s%s%s\r\n",
          CCGRN(ch, C_NRM), CAP(account->name), CCNRM(ch, C_NRM),
          CCRED(ch, C_SPR), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), *buf2 ? buf2 : "<None>", CCNRM(ch, C_NRM));

        free_account(account);
      }
    }
  } else
    sprintf(buf, "L-Desc: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));

  send_to_char(buf, ch);

  sprintf(buf2, "Lev: [%s%2d%s], XP: [%s%7ld%s], Unassigned [%s%7ld%s], Align: [%4d]\r\n",
    CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
    CCYEL(ch, C_NRM), (long)GET_EXP(k), CCNRM(ch, C_NRM),
    CCYEL(ch, C_NRM), GET_EXP_Q(k), CCNRM(ch, C_NRM),
    GET_ALIGNMENT(k));
  send_to_char(buf2, ch);

  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[24] = buf2[24] = '\0'; 

    sprintf(buf, "Created: [%s], Last Logon: [%s]\r\n", buf1, buf2) ;
    sprintf(buf, "%sPlayed [%dh %dm], Age [%d], Outlaw Time [%dm], Jail Time [%dm]\r\n", buf,
      (k->player.time.played / 3600),
      ((k->player.time.played / 60) % 60), 
      age(k).year,
      OUTLAW_TIME(k) > 0 ? OUTLAW_TIME(k) : 0,
      JAIL_TIME(k) > 0 ? JAIL_TIME(k) : 0);
    send_to_char(buf, ch);

    sprintf(buf, "Hometown: [%d] %s", GET_HOME(k), hometowns[GET_HOME(k)]);
    if (GET_BINDROOM(k))
      sprintf(buf, "%s, Bind Room: [%s%ld%s]", buf, CCCYN(ch, C_NRM), GET_BINDROOM(k), CCNRM(ch, C_NRM));
      
    /*. Display OLC zone for immorts .*/
    if(GET_LEVEL(k) >= LVL_IMMORT)
      sprintf(buf, "%s, OLC[%d]", buf, GET_OLC_ZONE(k));
    strcat(buf, "\r\n");
    sprintf (buf, "%sRace: [%d] %s  XP to level: [%ld]  ", buf,
      GET_RACE(k), npc_races[GET_RACE(k)].race,
      GET_LEVEL(k)< LVL_IMMORT ? exp_needed_for_level(k) : 0);
    if (GET_LAST_DEATH(k) != 0) {
      strcpy(buf1, (char *)asctime(localtime(&(GET_LAST_DEATH(k)))));
      buf1[16] = '\0';
      sprintf(buf, "%sLast Death: [%s]\r\n", buf, buf1);
    } else {
      sprintf(buf, "%sLast Death: [NONE]\r\n", buf);
    }
    sprintf(buf, "%sGroup Points: [%d], Stat Points: [%d], Vitae Points: [%d], "
      "Vitae Exp: [%ld]\r\n", buf,
      GET_GROUP_PTS(k), GET_STAT_PTS(k), GET_VITAE(k), GET_VITAE_EXP(k));
    send_to_char(buf, ch);
  } else { /* NPCs */
    sprintf (buf, "Race: [%d] %s Uuid: [%s]\r\n",
      GET_RACE(k), npc_races[GET_RACE(k)].race, GET_UUID(k));
    send_to_char(buf, ch);
  }
  sprintf (buf, "*************-------------*************-------------*************\r\n") ;

  sprintf(buf,
    "%sStr: [%s%d%s/%d]\tInt: [%s%d%s/%d]\t*\tHit p.:  [%s%d/%d+%d%s]\r\n",
    buf,
    CCCYN(ch, C_NRM), GET_STR(k), CCNRM(ch, C_NRM),
    GET_REAL_STR(k),
    CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
    GET_REAL_INT(k),
    CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), point_gain(k, SCMD_HIT),
    CCNRM(ch, C_NRM)) ;

  sprintf(buf,
    "%sDex: [%s%d%s/%d]\tWis: [%s%d%s/%d]\t*\tMana p.: [%s%d/%d+%d%s]\r\n",
    buf,
    CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
    GET_REAL_DEX(k), 
    CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
    GET_REAL_WIS(k),
    CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), point_gain(k, SCMD_MANA),
    CCNRM(ch, C_NRM)) ;

  sprintf(buf,
    "%sCon: [%s%d%s/%d]\tCha: [%s%d%s/%d]\t*\tMove p.: [%s%d/%d+%d%s]\r\n",
    buf,
    CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
    GET_REAL_CON(k),
    CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM),
    GET_REAL_CHA(k),
    CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), point_gain(k, SCMD_MOVE),
    CCNRM(ch, C_NRM)) ;

  sprintf (buf, "%s*************-------------*************-------------*************\r\n", buf ) ;

  send_to_char(buf, ch);

  if (!IS_NPC(k)) {
   sprintf(buf,"Kills: [%s%9d%s], PKills: [%s%9d%s], Deaths: [%s%9d%s]\r\n",
       CCRED(ch, C_CMP), GET_KILLS(k), CCNRM(ch, C_CMP),
       CCRED(ch, C_CMP), GET_PKS(k), CCNRM(ch, C_CMP),
       CCRED(ch, C_CMP), GET_DEATHS(k), CCNRM(ch, C_CMP));
   send_to_char(buf, ch);
  }

  if (!is_shopkeeper(k))
    sprintf(buf, "Coins: [%9ld], Bank: [%9ld] (Total: %ld)\r\n",
      (long)GET_GOLD(k), (long)GET_BANK_GOLD(k), 
      (long)(GET_GOLD(k) + GET_BANK_GOLD(k)) );
  else {
    for (shop_nr = 0; shop_nr <= top_shop; shop_nr++)
      if (SHOP_KEEPER(shop_nr) == k->nr)
        break;

    if (shop_nr > top_shop)
      sprintf(buf, "Coins: [%9ld], Bank: [%9ld] (Total: %ld)\r\n",
        (long)GET_GOLD(k), (long)GET_BANK_GOLD(k), 
        (long)(GET_GOLD(k) + GET_BANK_GOLD(k)));
    else
      sprintf(buf, "Coins: [%9ld], Bank: [%9ld] (Total: %ld)\r\n",
        (long)GET_GOLD(k), (long)SHOP_BANK(shop_nr), 
        (long)(GET_GOLD(k) + SHOP_BANK(shop_nr)));
  }
  send_to_char(buf, ch);

  sprintf(buf, "AC: [%3d]  Hitroll: [%3d]  Damroll: [%3d]   Dodgeroll: [%3d]\r\n",
    GET_AC(k), k->points.hitroll, k->points.damroll, k->points.dodgeroll);
  sprintf(END_OF(buf), "Saving throws: Para [%d]  Rod [%d]  Petri[%d]  Breath[%d]  Spell [%d]\r\n",
    GET_SAVE(k, 0), GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);

  sprinttype(GET_POS(k), position_types, buf2);
  sprintf(buf, "Pos: %s, Fighting: %s", buf2,
    (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

  if (IS_NPC(k)) {
    strcat(buf, ", Attack type: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
  }
  if (k->desc) {
    sprinttype(k->desc->connected, connected_types, buf2);
    sprintf(buf + strlen(buf), ", Guarding: %s%s%s, Connected: %s\r\n", CCYEL(ch, C_NRM),
      (GUARDING(k) ? GET_NAME(GUARDING(k)) : "Nobody"), CCNRM(ch, C_NRM), buf2);
    sprintbit(k->desc->bits, desc_bits, buf2);
    sprintf(buf + strlen(buf), "Desc: %s", buf2);
  }
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    if (GET_NOISE(k)) {
      sprintf(buf, "\r\nNoise: %s%s%s", CCYEL(ch, C_NRM), GET_NOISE(k), CCNRM(ch, C_NRM));
      send_to_char(buf, ch);
    }
    if (GET_SOUND(k)) {
      sprintf(buf, "\r\nSound: %s%s%s", CCYEL(ch, C_NRM), GET_SOUND(k), CCNRM(ch, C_NRM));
      send_to_char(buf, ch);
    }
  }

  strcpy(buf, "\r\nDefault position: ");
  sprinttype((k->mob_specials.default_pos), position_types, buf2);
  strcat(buf, buf2);

  if (!IS_NPC(k))
    sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
  else
    sprintf(buf2, ", Combat Speed (in rounds) [%d]\r\n", calculate_speed(k) / PULSE_VIOLENCE);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintbit(MOB_FLAGS(k), action_bits, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
    char *mob_race[5];
    int i = 0;
    sprintbit(PLR_FLAGS(k), player_bits, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF_FLAGS(k), preference_bits, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    for (i = 0; i < 5; i++)
      mob_race[i] = mob_race_name(GET_RACE_HATE(k, i));
            
    sprintf(buf, "Racial Hatreds: %s%s, %s, %s, %s, %s%s\r\n",
            CCGRN(ch, C_NRM),
            mob_race[0], mob_race[1], mob_race[2], mob_race[3], mob_race[4],
            CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (!IS_MOB(k) && GET_LEVEL(k)>=LEVEL_IMMORT){
    sprintf(buf, "Poofin: %s\r\nPoofout: %s\r\n",
      POOFIN(k) ? POOFIN(k) : "None",
      POOFOUT(k)? POOFOUT(k) : "None");
    send_to_char(buf, ch);
  }
  if (IS_MOB(k)) {
    sprintf(buf, "Mob Script: %s, Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
      GET_MOB_SCRIPT(k)->name ? GET_MOB_SCRIPT(k)->name : "None",
            (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
             GET_NDD(k), GET_SDD(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Carried: weight: %d, items: %d; ",
    IS_CARRYING_W(k), IS_CARRYING_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (!IS_MOB(k)) {
    sprintf(buf, "Hunger: %d/48, Thirst: %d/48, Drunk: %d/48, Tired: %d/48\r\n",
            GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK), GET_COND(k, TIRED));
    sprintbit(GET_DYE(k), dyes, buf2);
    sprintf(buf, "%sTattoo: %s (%s%s%s)\r\n", buf, tattoos[GET_TATTOO(k)],
            CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    if (GET_MOUNT_NUM(k)) {
      mount = read_mobile(GET_MOUNT_NUM(k), VIRTUAL, NULL);
      char_to_room(mount, 0);

      sprintf(buf, "%sMount: %s (Stabled in room %ld)\r\n", buf, GET_NAME(mount), GET_STABLE(k));
      extract_char(mount);
    }
    send_to_char(buf, ch);
  }

  if(IS_NPC(k) && HUNTING(k)) {
    sprintf( buf, "Hunting: %s\r\n", GET_NAME(HUNTING(k)) );
    send_to_char(buf, ch);
  }


 /* depricated 20000705 -rparet
  if (!IS_NPC(k))
    if (GET_CLAN(k) && GET_CLAN_RANK(k) != 0) 
    {
      clan_num = find_clan_by_id(GET_CLAN(k));
      sprintf(buf, "Clan : %s\r\n", clan[clan_num].name);
      send_to_char(buf, ch);
    } 

 */  

  sprintf(buf, "Master is: %s, Followers are:", ((k->master) ? GET_NAME(k->master) : "<none>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
        send_to_char(strcat(buf, ",\r\n"), ch);
      else
        send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  /* Showing the bitvector */
  sprintbit(AFF_FLAGS(k), affected_bits, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration,
        CCCYN(ch, C_NRM), spell_info[aff->type].name, CCNRM(ch, C_NRM));
      if (aff->modifier) {
        sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
        strcat(buf, buf2);
      }
      if (aff->bitvector) {
        if (*buf2)
          strcat(buf, ", sets ");
        else
          strcat(buf, "sets ");
        sprintbit(aff->bitvector, affected_bits, buf2);
        strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
}


ACMD(do_stat)
{
  struct char_data *victim = 0;
  struct obj_data *object = 0;
  int tmp;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char("Stats on who or what?\r\n", ch);
    return;
  } else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Stats on which mobile?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2)))
        do_stat_character(ch, victim);
      else
        send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2, 0)))
        do_stat_character(ch, victim);
      else
        send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = load_char(buf2))) {
        if (GET_LEVEL(victim) > GET_LEVEL(ch))
          send_to_char("Sorry, you can't do that.\r\n", ch);
        else
          do_stat_character(ch, victim);
        free_char(victim);
      } else
        send_to_char("There is no such player.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char("Stats on which object?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)))
        do_stat_object(ch, object);
      else
        send_to_char("No such object around.\r\n", ch);
    }
  } else {
    if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
      do_stat_object(ch, object);
    else if ((victim = get_char_room_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}

ACMD(do_shutdown)
{
  extern int circle_shutdown, circle_reboot, sql_wipe;

  void sql_erase(void);
  ACMD(do_force);
  void sql_optimize();

     if (subcmd != SCMD_SHUTDOWN) {
        send_to_char("If you want to shut something down, say so!\r\n", ch);
        return;
     }
     one_argument(argument, arg);

     if (!*arg) {
       sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
       log(buf);
       send_to_all("Shutting down.\r\n");
       circle_shutdown = 1;
     } else if (!str_cmp(arg, "reboot")) {
       sprintf(buf, "(GC) Reboot by %s.", GET_NAME(ch));
       log(buf);
       send_to_all("Rebooting.. come back in a minute or two.\r\n");
       touch("../.fastboot");
       circle_shutdown = circle_reboot = 1;
     } else if (!str_cmp(arg, "die")) {
       sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
       log(buf);
       send_to_all("Shutting down for maintenance.\r\n");
       touch("../.killscript");
       circle_shutdown = 1;
     } else if (!str_cmp(arg, "pause")) {
       sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
       log(buf);
       send_to_all("Shutting down for maintenance.\r\n");
       touch("../pause");
       circle_shutdown = 1;
     } else
       send_to_char("Unknown shutdown option.\r\n", ch);

     if (circle_shutdown) {
       House_save_all();
       Crash_save_all();
       shop_control_save();
       if (sql_wipe)
         sql_erase();
       sql_optimize();
     }
}


void stop_snooping(struct char_data * ch)
{
  if (!ch->desc->snooping)
    send_to_char("You aren't snooping anyone.\r\n", ch);
  else {
    send_to_char("You stop snooping.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such person around.\r\n", ch);
  else if (!victim->desc)
    send_to_char("There's no link.. nothing to snoop.\r\n", ch);
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Busy already. \r\n", ch);
  else if (victim->desc->snooping == ch->desc)
    send_to_char("Don't be stupid.\r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
      send_to_char("You can't.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}



ACMD(do_switch)
{
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char("You're already switched.\r\n", ch);
  else if (!*arg)
    send_to_char("Switch with who?\r\n", ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such character.\r\n", ch);
  else if (ch == victim)
    send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
  else if (victim->desc)
    send_to_char("You can't do that, the body is already in use!\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
    send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
  else {
    send_to_char(OK, ch);

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);

    /* JE 2/22/95 */
    /* if someone switched into your original body, disconnect them */
    if (ch->desc->original->desc)
      close_socket(ch->desc->original->desc);

    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}

static void
random_load_msg(struct char_data *ch)
{ 
/* This has NOTHING to do with random load mobs         */
/* This message merely shows when a god loads something */
    switch (number(0,2))
    {
        case 0: 
    	  act("Suddenly the walls run red with blood and a neon '666' sign "
	      "flashes.", TRUE, ch, 0, 0, TO_ROOM);
    	  act("Suddenly the walls run red with blood and a neon '666' sign "
	      "flashes.", TRUE, ch, 0, 0, TO_CHAR);
	  break;
	case 1:
    	  act("The flames of Hell roar up then fade, leaving something behind...", TRUE, ch, 0, 0, TO_ROOM);
    	  act("The flames of Hell roar up then fade, leaving something behind...", TRUE, ch, 0, 0, TO_CHAR);
	  break;
	case 2:
    	  act("A sound not unlike the shriek of a dying dragon fills the room...", TRUE, ch, 0, 0, TO_ROOM);
    	  act("A sound not unlike the shriek of a dying dragon fills the room...", TRUE, ch, 0, 0, TO_CHAR);
	  break;
    }
}


ACMD(do_load)
{
   struct char_data *mob;
   struct obj_data *obj;
   char buf[MAX_STRING_LENGTH];
   int	number = 0;
   int  r_num;
   int  nr = 0;
   int  found = FALSE;
   int  i;
   bool cantload;

   if (IS_NPC(ch))
     return;

   half_chop(argument, buf, buf2);

   if (!isdigit(*buf2) && (is_abbrev(buf, "mob")) ) {
     for (nr = 0; nr <= top_of_mobt; nr++)
       if (isname_with_abbrevs(buf2, mob_proto[nr].player.name)) {
         found=TRUE;
         number=mob_index[nr].virtual;
         nr = top_of_mobt+1;
       }
     if (found==FALSE)
       send_to_char("Mob name not found.\n\r", ch);
   }
   if (!isdigit(*buf2) && (is_abbrev(buf, "object")) ) {
     for (nr = 0; nr <= top_of_objt; nr++)
       if (strstr(obj_proto[nr].short_description, buf2)) {
         found=TRUE;
         number=obj_index[nr].virtual;
         nr = top_of_objt+1;
       }
     if (found==FALSE)
       send_to_char("Object name not found.\n\r", ch);
   }

   if (!*buf || !*buf2 || (!isdigit(*buf2) && number==0) ) {
     send_to_char("Usage: load { obj | mob } <number or name>\n\r", ch);
     return;
   }

   if (number ==0)
     if ((number = atoi(buf2)) < 0) {
       send_to_char("A NEGATIVE number??\n\r", ch);
       return;
     }
   if (is_abbrev(buf, "mob")) {
     if ((r_num = real_mobile(number)) < 0) {
       send_to_char("There is no monster with that number.\n\r", ch);
       return;
     }

     for (i = 0; i < num_moblim; i++)
       if ((number == MobLimits[i].item_num) && (GET_LEVEL(ch) < MobLimits[i].min_level)) {
         send_to_char("You're not godly enough to load that!\n\r", ch);
         return;
       }

     if (ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) {
       send_to_char("You can't load a mob in this room!\r\n", ch);
       return;
     }

     mob = read_mobile(r_num, REAL, NULL);
     char_to_room(mob, ch->in_room);
     GET_LOADED_IN(mob) = world[ch->in_room].number;

     sprintf(buf, "(GC) %s loaded %s at %s.", GET_NAME(ch), GET_NAME(mob), world[ch->in_room].name);
     mudlog(buf, BRF, GET_LEVEL(ch)+1, TRUE);

     act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
     random_load_msg(ch); 
     act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
     act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
   } else if (is_abbrev(buf, "obj")) {
     if ((r_num = real_object(number)) < 0) {
       send_to_char("There is no object with that number.\n\r", ch);
       return;
     }
     for (i = 0; i < num_objlim; i++)
       if ((number == ObjLimits[i].item_num) && (GET_LEVEL(ch) < ObjLimits[i].min_level)) {
         send_to_char("You're not godly enough to load that!\n\r", ch);
         return;
       }
     obj = read_object(r_num, REAL, NULL);

     cantload=FALSE;
     switch (GET_LEVEL(ch)){
       case 101:      /*IMMORT*/
         if (obj->obj_flags.cost > 4000)
           cantload = TRUE;
         break;
       case 102:
         if (obj->obj_flags.cost > 5000)
           cantload = TRUE;
         break;
       case 103:
         if (obj->obj_flags.cost > 5500)
           cantload = TRUE;
         break;
       case 104:
         if (obj->obj_flags.cost > 6000)
           cantload = TRUE;
         break;
       case 105:
         if (obj->obj_flags.cost > 7000)
           cantload = TRUE;
         break;
       default:      /*GR_GOD+ can load anything*/
         break;
      }

     if (cantload){
       send_to_char("That is beyond your godly powers...\n\r",ch);
       extract_obj(obj);
       return;
     }
     /*else*/
     obj_to_char(obj, ch);

     sprintf(buf, "(GC) %s loaded %s at %s", GET_NAME(ch), (obj->short_description), world[ch->in_room].name);
     mudlog(buf, BRF, GET_LEVEL(ch)+1, TRUE);

     act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
     random_load_msg(ch); 
     act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
     act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
   } else
     send_to_char("That'll have to be either 'obj' or 'mob'.\n\r", ch);
}

ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL, NULL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL, NULL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}




/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  one_argument(argument, buf);

  if (*buf) {			/* argument supplied. destroy single object
				 * or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char("Fuuuuuuuuu!\r\n", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
	sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, BRF, LVL_GOD, TRUE);
	if (vict->desc) {
	  close_socket(vict->desc);
	  vict->desc = NULL;
	}
      }
      extract_char(vict);
    } 
    else if((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } 
    else {
      send_to_char("Nothing here by that name.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);
  } else {			/* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict))
	extract_char(vict);
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}



ACMD(do_advance)
{
  struct char_data *victim;
  char *name = arg, *level = buf2;
  int newlevel, oldlevel;
  int str, intel, wis, dex, con, cha;
  void do_start(struct char_data *ch);

  void gain_exp(struct char_data * ch, int gain);

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) != LEVEL_IMPL) {   /* let imps do whatever they want :-) */
    if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
      send_to_char("Maybe that's not such a great idea.\r\n", ch);
      return;
    }
  }

  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Yeah, right.\r\n", ch);
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char("They are already at that level.\r\n", ch);
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) /* demotion */
  {
    str = victim->real_abils.str;
    dex = victim->real_abils.dex;
    intel = victim->real_abils.intel ;
    con = victim->real_abils.con;
    cha = victim->real_abils.cha;
    wis = victim->real_abils.wis;
    
    send_to_char("You are momentarily enveloped by darkness!\r\n"
		 "You can feel all your power and knowledge being\r\n"
                 "drained away from you!\r\n", victim);
    do_start(victim);
    victim->real_abils.intel =  intel;
    victim->real_abils.wis =  wis;
    victim->real_abils.dex =  dex;
    victim->real_abils.str =  str;
    victim->real_abils.con =  con;
    victim->real_abils.cha = cha;
  } else { /* promotion */
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }

  send_to_char(OK, ch);

  if (newlevel < oldlevel)
    log("(GC) %s demoted %s from level %d to %d.",
		GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
  else
    log("(GC) %s has advanced %s to level %d (from %d)",
		GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);

  gain_exp_regardless(victim, min_exp(newlevel) - GET_EXP(victim));
  save_char(victim, NOWHERE);
}



ACMD(do_restore)
{
  struct char_data *vict;
  int i;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to restore?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else {
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);

    if (GET_LEVEL(vict) < LVL_IMMORT) {
      GET_COND(vict, THIRST) = 48;
      GET_COND(vict, FULL)   = 48;
      GET_COND(vict, DRUNK)  = 0;
      GET_COND(vict, TIRED)  = 48;
    } else {
      GET_COND(vict, THIRST) = -1;
      GET_COND(vict, FULL)   = -1;
      GET_COND(vict, DRUNK)  = -1;
      GET_COND(vict, TIRED)  = -1;
    }

    if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
      for (i = 1; i <= MAX_SKILL_GROUPS; i++) {
        SET_SKILL(vict, i, 100)
        GET_GROUP_LVL(vict, i) = 4;
      }

      for (i = 1; i < NUM_SPELLS; i++)
        GET_SPELL(ch, i) = '1';

      if (GET_LEVEL(vict) >= LVL_GRGOD) {
	vict->real_abils.intel = 25;
	vict->real_abils.wis = 25;
	vict->real_abils.dex = 25;
	vict->real_abils.str = 25;
	vict->real_abils.con = 25;
	vict->real_abils.cha = 25;
      }
      vict->aff_abils = vict->real_abils;
    }
    update_pos(vict);
    send_to_char(OK, ch);
    act("The hand of $N touches you, healing your wounds and leaving you "
	"refreshed!", TRUE, vict, 0, ch, TO_CHAR);
  }
}


void perform_immort_vis(struct char_data *ch)
{
  void appear(struct char_data *ch);

  if (!IS_AFFECTED(ch, AFF_HIDE) &&
      !IS_AFFECTED(ch, AFF_INVISIBLE))
    if (!GET_INVIS_LEV(ch))
    {
      send_to_char("You are already fully visible.\r\n", ch);
      return;
    }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("You are now fully visible.\r\n", ch);
}


void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  sprintf(buf, "Your invisibility level is %d.\r\n", level);
  send_to_char(buf, ch);
}
  

ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("You can't do that!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char("You can't go invisible above your own level.\r\n", ch);
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);

  if (!*argument)
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    sprintf(buf, "%s\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character && pt->character != ch)
	send_to_char(buf, pt->character);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      send_to_char(buf, ch);
  }
}


ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:    msg = &(POOFIN(ch));    break;
  case SCMD_POOFOUT:   msg = &(POOFOUT(ch));   break;
  default:    return;    break;
  }

  skip_spaces(&argument);

  if (*msg)
    FREE(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = str_dup(argument);

  send_to_char(OK, ch);
}



ACMD(do_dc)
{
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char("Usage: DC <connection number> (type USERS for a list)\r\n", ch);
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    return;
  }
  /* We used to just close the socket here using close_socket(), but
   * various people pointed out this could cause a crash if you're
   * closing the person below you on the descriptor list.  Just setting
   * to CON_CLOSE leaves things in a massively inconsistent state so I
   * had to add this new flag to the descriptor.
   */
  d->close_me = 1;
  sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
  send_to_char(buf, ch);
  sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
  log(buf);
}

ACMD(do_wizlock)
{
  int value;
  char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    game_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (game_restrict) {
  case 0:
    sprintf(buf, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    sprintf(buf, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
	    game_restrict, when);
    break;
  }
  send_to_char(buf, ch);
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;
  extern time_t boot_time;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sprintf(buf, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
	    ((d == 1) ? "" : "s"), h, m);
  }

  send_to_char(buf, ch);
}

ACMD(do_last)
{
  struct char_data *player;
  extern char *sql_get_host(struct char_data *ch);

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }
  if (!(player = load_char(arg))) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }

  sprintf(buf, "[%3d] %-12s : %-18s : %-20s",
    GET_LEVEL(player), GET_NAME(player), sql_get_host(player), ctime(&player->player.time.logon));
  send_to_char(buf, ch);

  free_char(player);
}

ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char to_force[MAX_INPUT_LENGTH + 2];

  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg)))
      send_to_char(NOPERSON, ch);
    else if (GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char("No, no, no!\r\n", ch);
    else {
      send_to_char(OK, ch);
      if (GET_LEVEL(ch)<LVL_IMPL)
        act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), 
               to_force);
      mudlog(buf, NRM, MAX(GET_LEVEL(ch)+1, GET_INVIS_LEV(ch)), TRUE);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced room %ld to %s", GET_NAME(ch), world[ch->in_room].number, to_force);
    mudlog(buf, NRM, MAX(GET_LEVEL(ch)+1, GET_INVIS_LEV(ch)), TRUE);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
    mudlog(buf, NRM, MAX(GET_LEVEL(ch)+1, GET_INVIS_LEV(ch)), TRUE);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (i->connected || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  void update_review(struct char_data *speaker, char *text, int type);
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;
  int noChosen = FALSE;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (GET_LEVEL(ch) < LVL_IMMORT && !PLR_FLAGGED(ch, PLR_CHOSEN)) {
     send_to_char("Huh?!?", ch);
     return;
  }
  if (!*argument) {
    send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
		 "       wiznet @\r\n", ch);
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      noChosen = TRUE;
      if (level > GET_LEVEL(ch)) {
	send_to_char("You can't wizline above your own level.\r\n", ch);
	return;
      }
    } else if (emote)
      argument++;
    break;
  case '@':
    *buf1 = '\0';
    for (d = descriptor_list; d; d = d->next) 
    {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) 
      {
	if (!any) 
        {
	  sprintf(buf1, "Gods online:\r\n");
	  any = TRUE;
	}
	sprintf(buf1 + strlen(buf1), "  %s", GET_NAME(d->character));
	if (PLR_FLAGGED(d->character, PLR_WRITING))
	  sprintf(buf1 + strlen(buf1), " (Writing)\r\n");
	else if (PLR_FLAGGED(d->character, PLR_MAILING))
	  sprintf(buf1 + strlen(buf1), " (Writing mail)\r\n");
	else
	  strcat(buf1, "\r\n");

      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) 
    {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  CAN_SEE(ch, d->character)) 
      {
	if (!any) 
        {
	  sprintf(buf1 + strlen(buf1), "Gods offline:\r\n");
	  any = TRUE;
	}
	sprintf(buf1, "  %s\r\n", GET_NAME(d->character));
      }
    }
    if (buf1)
      send_to_char(buf1, ch);
    return;
    break;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }
  if (noChosen) {
    sprintf(buf1, "%s: <%d> %s%s\r\n", GET_NAME(ch), level,
	    emote ? "<--- " : "", argument);
    sprintf(buf2, "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "",
	    argument);
  } else {
    sprintf(buf1, "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "",
	    argument);
    sprintf(buf2, "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
  }

  update_review(ch, argument, SCMD_REVWIZN);

  for (d = descriptor_list; d; d = d->next) 
  {
    if ((!d->connected) && 
	/*  they are immortal, or chosen and the wiz is at default level */
	(GET_LEVEL(d->character) >= level || 
	 (PLR_FLAGGED(d->character, PLR_CHOSEN)&& !noChosen)) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
	(!PLR_FLAGGED(d->character, PLR_MAILING) ||
         !PLR_FLAGGED(d->character, PLR_WRITING))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) 
    {
      send_to_char(CCCYN(d->character, C_NRM), d->character);
      if (CAN_SEE(d->character, ch))
	send_to_char(buf1, d->character);
      else
	send_to_char(buf2, d->character);
      send_to_char(CCNRM(d->character, C_NRM), d->character);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
}



ACMD(do_zreset)
{
  void reset_zone(int zone);

  int i, j;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reset world.\r\n", ch);
    sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i >= 0 && i <= top_of_zone_table) {
    reset_zone(i);
    sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
	    zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), zone_table[i].number,
            zone_table[i].name);
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
  struct char_data *vict;
  long result;
  int i;

  if((GET_LEVEL(ch) < LVL_IMMORT)  && !PLR_FLAGGED(ch, PLR_CHOSEN)) {
    stc("Huh?!?\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Yes, but for whom?!?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg)))
    send_to_char("There is no such player.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("You can't do that to a mob!\r\n", ch);
  else if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && (GET_LEVEL(vict) > LVL_IMMORT))
    send_to_char("Hmmm...you'd better not.\r\n", ch);
  else {
    switch (subcmd) {
      case SCMD_PARDON:
        if (!PLR_FLAGGED(vict, PLR_OUTLAW)) {
          send_to_char("Your victim is not flagged.\r\n", ch);
          return;
        }
        REMOVE_BIT(PLR_FLAGS(vict), PLR_OUTLAW);
        OUTLAW_TIME(vict) = 0;
        send_to_char("Pardoned.\r\n", ch);
        send_to_char("You have been pardoned by the Gods!\r\n", vict);
        sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
        break;
      case SCMD_NOTITLE:
        result = PLR_TOG_CHK(vict, PLR_NOTITLE);
        sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result), GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
        break;
      case SCMD_SQUELCH:
        result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
        sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result), GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
        break;
      case SCMD_FREEZE:
        if (ch == vict) {
          send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
          return;
        }
        if (PLR_FLAGGED(vict, PLR_FROZEN)) {
          send_to_char("Your victim is already pretty cold.\r\n", ch);
          return;
        }
        SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
        GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
        send_to_char("A bitter wind suddenly rises and drains every ounce of heat from your body!\r\nYou feel frozen!\r\n", vict);
        send_to_char("Frozen.\r\n", ch);
        act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
        sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
        break;
      case SCMD_THAW:
        if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
          send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
          return;
        }
        if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
          sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
            GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
          send_to_char(buf, ch);
          return;
        }
        sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
        REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
        send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
        send_to_char("Thawed.\r\n", ch);
        act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
        break;
      case SCMD_UNAFFECT:
        for (i = 0; i < 5; i++) {
          GET_RACE_KILLS(vict, GET_RACE_HATE(ch, i)) = 0;
          GET_RACE_HATE(vict, i) = -1;
        }
        send_to_char("Racial hatreds removed.\r\n", ch);
        send_to_char("Your racial hatreds have been removed.\r\n", vict);
        if (PLR_FLAGGED(vict, PLR_IT)) {
          REMOVE_BIT(PLR_FLAGS(vict), PLR_IT);
          send_to_char("Un-tagged!\r\n", ch);
          send_to_char("You're no longer it!\r\n", vict);
        }
        if (PLR_FLAGGED(vict, PLR_VAMPIRE)) {
          REMOVE_BIT(PLR_FLAGS(vict), PLR_VAMPIRE);
          if (IS_AFFECTED(vict, AFF_VAMPIRE))
            REMOVE_BIT(AFF_FLAGS(vict), AFF_VAMPIRE);
          send_to_char("Un-vamped!\r\n", ch);
          send_to_char("You're no longer a creature of the night!\r\n", vict);
        }
        if (PLR_FLAGGED(vict, PLR_WEREWOLF)) {
          REMOVE_BIT(PLR_FLAGS(vict), PLR_WEREWOLF);
          if (IS_AFFECTED(vict, AFF_WEREWOLF))
            REMOVE_BIT(AFF_FLAGS(vict), AFF_WEREWOLF);
          send_to_char("Un-wered!\r\n", ch);
          send_to_char("You're no longer a creature of the night!\r\n", vict);
        }
        if (vict->affected) {
          while (vict->affected)
            affect_remove(vict, vict->affected, TRUE);
          send_to_char("There is a brief flash of light!\r\nYou feel slightly different.\r\n", vict);
          send_to_char("All spells removed.\r\n", ch);
        } else {
          send_to_char("Your victim does not have any affections!\r\n", ch);
          return;
        }
        break;
      default:
        log("SYSERR: Unknown subcmd passed to do_wizutil (act.wizard.c)");
        break;
    }
    save_char(vict, NOWHERE);
  }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, int zone)
{
  sprintf(bufptr, "%s%3d %-30.30s Age: %3d; Reset: %3d (%1d); Top: %5ld\r\n",
	  bufptr, zone_table[zone].number, zone_table[zone].name,
	  zone_table[zone].age, zone_table[zone].lifespan,
	  zone_table[zone].reset_mode, zone_table[zone].top);
}

ACMD(do_show)
{
  int i, j, k, l, con;
  char self = 0;
  struct char_data *vict;
  struct obj_data *obj;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];
  extern char *genders[];
  extern int buf_switches, buf_largecount, buf_overflows;
  extern long gc_objcount;
  void show_shops(struct char_data * ch, char *value);
  void hcontrol_list_houses(struct char_data *ch);
  void sql_Crash_listrent(struct char_data *ch, char *name);
  void sql_listskills(struct char_data *ch, char *name);

  struct show_struct {
    char *cmd;
    char level;
  } fields[] = {
    { "nothing",    0  },           /* 0 */
    { "zones",      LVL_IMMORT },   /* 1 */
    { "player",     LVL_GOD },
    { "rent",       LVL_GOD },
    { "stats",      LVL_IMMORT },
    { "errors",     LVL_IMPL-1 },   /* 5 */
    { "death",      LVL_GOD },
    { "godrooms",   LVL_GOD },
    { "shops",      LVL_IMMORT },
    { "houses",     LVL_GOD },
    { "tattoos",    LVL_IMMORT },   /*10*/
    { "aggr",       LVL_IMPL-1 },
    { "reagents",   LVL_IMMORT },
    { "hooks",      LVL_IMMORT },
    { "neutral",    LVL_IMMORT },
    { "objects",    LVL_IMMORT },   /* 15 */
    { "mobiles",    LVL_IMMORT },
    { "scripts",    LVL_IMMORT },
    { "skills",     LVL_IMMORT },
    { "kills",      LVL_IMMORT },
    { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    strcpy(buf, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
        sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }

  if (!strcmp(value, "."))
    self = 1;
  *buf = '\0';
  switch (l) {
    case 1:      /* zone */
      /* tightened up by JE 4/6/93 */
      if (self)
        print_zone_to_buf(buf, world[ch->in_room].zone);
      else if (*value && is_number(value)) {
        for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
          if (i <= top_of_zone_table)
            print_zone_to_buf(buf, i);
          else {
            send_to_char("That is not a valid zone.\r\n", ch);
            return;
          }
      } else
        for (i = 0; i <= top_of_zone_table; i++)
          print_zone_to_buf(buf, i);
      page_string(ch->desc, buf, TRUE); 
      break;
    case 2:      /* player */
      if (!*value) {
        send_to_char("A name would help.\r\n", ch);
        return;
      }

      if (!(vict = load_char(value))) {
	      send_to_char("There is no such player.\r\n", ch);
	      return;
	    }

      sprintf(buf, "Player: %-12s (%s) [%2d]\r\n", GET_NAME(vict), genders[(int)GET_SEX(vict)], GET_LEVEL(vict));
      sprintf(buf, "%sAu: %-8d  Bal: %-8d  Exp: %-8ld  Align: %-5d\r\n",
        buf, GET_GOLD(vict), GET_BANK_GOLD(vict), GET_EXP(vict), GET_ALIGNMENT(vict));
      strcpy(birth, ctime(&vict->player.time.birth));
      sprintf(buf, "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
	      buf, birth, ctime(&vict->player.time.logon), (int)(vict->player.time.played / 3600), (int) (vict->player.time.played / 60 % 60));
      send_to_char(buf, ch);
      free_char(vict);
      break;
    case 3:    /* rent */
      if (!*value) {
        stc("A name would help.\r\n", ch);
        return;
      }
      sql_Crash_listrent(ch, value);
      break;
    case 4:      /* stats */
      i = 0;
      j = 0;
      k = 0;
      con = 0;
      for (vict = character_list; vict; vict = vict->next) {
        if (IS_NPC(vict))
          j++;
        else if (CAN_SEE(ch, vict)) {
          i++;
          if (vict->desc)
            con++;
        }
      }
      for (obj = object_list; obj; obj = obj->next)
        k++;
      sprintf(buf, "Current stats:\r\n");
      sprintf(buf + strlen(buf), "  %5d players in game  %5d connected\r\n", i, con);
      sprintf(buf + strlen(buf), "  %5d registered\r\n", top_of_p_table + 1);
      sprintf(buf + strlen(buf), "  %5d mobiles          %5d prototypes\r\n",
        j, top_of_mobt + 1);
      sprintf(buf + strlen(buf), "  %5d objects          %5d prototypes\r\n",
        k, top_of_objt + 1);
      sprintf(buf + strlen(buf), "  %5ld rooms            %5d zones\r\n",
        top_of_world + 1, top_of_zone_table + 1);
      sprintf(buf + strlen(buf), "  %5d large bufs\r\n", buf_largecount);
      sprintf(buf + strlen(buf), "  %5d buf switches     %5d overflows\r\n",
        buf_switches, buf_overflows);
      sprintf(buf + strlen(buf), "  %5ld garbage objects collected.\r\n", gc_objcount);
      send_to_char(buf, ch);
      break;
    case 5:
      strcpy(buf, "Errant Rooms\r\n------------\r\n");
      for (i = 0, k = 0; i <= top_of_world; i++) {
        for (j = 0; j < NUM_OF_DIRS; j++)
          if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
            sprintf(buf + strlen(buf), "%2d: [%5ld] %s (connection)\r\n", ++k, 
              GET_ROOM_VNUM(i), world[i].name);

        if (world[i].ex_description)
          if (isname("Linked", world[i].ex_description->description) && 
             (world[i].ex_description->link == 0))
            sprintf(buf + strlen(buf), "%2d: [%5ld] %s (description link)\r\n", ++k,
              GET_ROOM_VNUM(i), world[i].name);
      }

      page_string(ch->desc, buf, TRUE);
      break;
    case 6:
      strcpy(buf, "Death Traps\r\n-----------\r\n");
      for (i = 0, j = 0; i <= top_of_world; i++)
        if (ROOM_FLAGGED(i, ROOM_DEATH))
          sprintf(buf + strlen(buf), "%2d: [%5ld] %s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name);
      page_string(ch->desc, buf, TRUE);
      break;
    case 7:
      strcpy(buf, "Godrooms\r\n--------------------------\r\n");
      for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_GODROOM))
        sprintf(buf + strlen(buf), "%2d: [%5ld] %s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name);
      page_string(ch->desc, buf, TRUE);
      break;
    case 8:
      show_shops(ch, value);
      break;
    case 9:
      hcontrol_list_houses(ch);
      break;
    case 10:
    { 
      int i = 0;
      for (i = 0; i < NUM_TATTOOS; i++) {
        sprintf(buf, "[%2d] %40s\r\n", i, tattoos[i]);
        send_to_char(buf, ch);
      }
      break;
    }
    case 11:
      for (vict = character_list; vict; vict = vict->next)
        if (MOB_FLAGGED(vict, MOB_AGGR75))
          sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_MOB_VNUM(vict), GET_NAME(vict));
      page_string(ch->desc, buf, TRUE);
      break;
    case 12:
    {
      int sortpos;

      for (sortpos = 1; sortpos < MAX_SKILLS; sortpos++) {
        char *reagents = get_reagent_names(find_skill_num(spell_info[sortpos].name));
        if(reagents) {
          char *mystr = tprintf("%s:  %s\r\n",spell_info[sortpos].name, reagents);
          stc(mystr, ch);
          FREE(mystr);
          FREE(reagents);
        }
      }
      break;
    }
    case 13:
      if (!*value) {
        send_to_char("You must supply a zone number!\r\n", ch);
        return;
      } else if (*value && is_number(value)) {
        j = atoi(value);
        for (i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
          if (i > top_of_zone_table) {
            stc("That is not a valid zone.\r\n", ch);
            return;
          } else {
            sprintf(buf, "Connections in zone %d.\r\n", zone_table[i].number);
            strcat(buf,  "========================\r\n");
    
            for(j = 0; j <= top_of_world; j++)
              if (world[j].zone == i)
                for(k = 0; k <NUM_OF_DIRS; k++)
                  if (world[j].dir_option[k] && world[world[j].dir_option[k]->to_room].zone != i)
                  {
                    snprintf(buf + strlen(buf), (MAX_STRING_LENGTH - strlen(buf)), "%5ld leads %s to %-5ld -- %s\r\n", 
                    GET_ROOM_VNUM(j), dirs[k],
                    GET_ROOM_VNUM(world[j].dir_option[k]->to_room),
                    zone_table[world[world[j].dir_option[k]->to_room].zone].name);
                  }
            page_string(ch->desc, buf, TRUE);
          }
      } /* end if(*value && is_number(value)) */
      break;      
    case 14: /* show neutral rooms*/
      strcpy(buf, "Neutral Rooms\r\n-------------\r\n");
      for (i = 0, j = 0; i <= top_of_world; i++)
        if (IS_SET(ROOM_FLAGS(i), ROOM_NEUTRAL))
      sprintf(buf + strlen(buf), "%2d: [%5ld] %s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name);
      page_string(ch->desc, buf, TRUE);
      break;
    case 15:    /* object problems */
      j = 0;

      strcpy(buf, "Object Problems\r\n---------------\r\n");
      for (i = 0; i <= top_of_objt; i++) {
        obj = read_object(i, REAL, NULL);
        *buf1 = '\0';
        sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
        sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf3);
        if (GET_OBJ_WEIGHT(obj) == 0)
          sprintf(buf1, "Zero Weight");
        else if (GET_OBJ_TYPE(obj) == 0)
          sprintf(buf1, "Type Undefined");
        else if (!strcmp(buf2, "UNUSED"))
          sprintf(buf1, "Type UNUSED");
        else if (!strcmp(buf3, "UNUSED"))
          sprintf(buf1, "UNUSED Flag");
        else if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) && (!GET_OBJ_VAL(obj, 0)))
          sprintf(buf1, "Subtype Not Set");
        else if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON || GET_OBJ_TYPE(obj) == ITEM_ARMOR) &&
          (!GET_CRAFT_VAL(obj)))
          sprintf(buf1, "Craft Flawless");
        if (*buf1)
          sprintf(buf + strlen(buf), "%3d. [%5d] %-30s   Problem: %s\r\n",
                  ++j, GET_OBJ_VNUM(obj), obj->short_description, buf1);
        extract_obj(obj);
      }

      for (obj = object_list; obj; obj = obj->next)
        if (obj->in_room > NOWHERE)
          continue;
        else if (obj->carried_by)
          continue;
        else if (obj->worn_by)
          continue;
        else if (obj->in_obj)
          continue;
        else
          sprintf(buf + strlen(buf), "%3d. [%5d] %-30s   Problem: Unknown Location\r\n", ++j,
                  GET_OBJ_VNUM(obj), obj->short_description);

      page_string(ch->desc, buf, TRUE);
      break;
    case 16:  /* mobile problems */
      j = 0;
      *buf = 0;
  
      strcpy(buf, "Mobile Problems\r\n---------------\r\n");
      for (i = 0; i < top_of_mobt; i++) {
        if (strlen(buf) + 80 > MAX_STRING_LENGTH)
          continue;

        vict = read_mobile(i, REAL, NULL);
        *buf1 = '\0';
        char_to_room(vict, 0);
        if (AFF_FLAGGED(vict, AFF_DETECT_ALIGN) || AFF_FLAGGED(vict, AFF_DETECT_MAGIC) ||
           AFF_FLAGGED(vict, AFF_GROUP) || AFF_FLAGGED(vict, AFF_CURSE) ||
           AFF_FLAGGED(vict, AFF_POISON) || AFF_FLAGGED(vict, AFF_SLEEP) ||
           AFF_FLAGGED(vict, AFF_FLESH_ALTER) || AFF_FLAGGED(vict, AFF_DODGE) ||
           AFF_FLAGGED(vict, AFF_BERSERK) || AFF_FLAGGED(vict, AFF_CHARM) ||
           AFF_FLAGGED(vict, AFF_FOLLOW) || AFF_FLAGGED(vict, AFF_CUTTHROAT) ||
           AFF_FLAGGED(vict, AFF_VAMPIRE) || AFF_FLAGGED(vict, AFF_WEREWOLF) ||
           AFF_FLAGGED(vict, AFF_MOUNT) || AFF_FLAGGED(vict, AFF_FLAMING) ||
           AFF_FLAGGED(vict, AFF_NOTHING) || AFF_FLAGGED(vict, AFF_DREAM))
          sprintf(buf1, "Invalid Flag");
        if ((mob_index[GET_MOB_RNUM(vict)].func) && !MOB_FLAGGED(vict, MOB_SPEC) &&
             !is_shopkeeper(vict)) 
          sprintf(buf1, "SPEC Not Set");
        if (MOB_FLAGGED(vict, MOB_SENTINEL) && (MOB_FLAGGED(vict, MOB_STAY_TERR) ||
            MOB_FLAGGED(vict, MOB_STAY_ZONE)))
          sprintf(buf1, "Flag Combination");
        if (is_shopkeeper(vict) && MOB_FLAGGED(vict, MOB_LOOTS))
          sprintf(buf1, "Shopkeeper and LOOT");
        if (*buf1)
          sprintf(buf + strlen(buf), "%3d. [%5d] %-30s   Problem: %s\r\n",
                  ++j, GET_MOB_VNUM(vict), GET_NAME(vict), buf1);
        extract_char(vict);
      }
      page_string(ch->desc, buf, TRUE);
      break;
    case 17:  /* scripts */
      if (!*value) {
        stc("You must specify a room, mob or obj.\r\n", ch);
        return;
      }

      if (!*arg) {
        stc("Which script were you looking for?\r\n", ch);
        return;
      }

      strcpy(argument, arg);
      skip_spaces(&argument);

      if (!strcmp(value, "room")) {
        sprintf(buf, "Room Scripts:\r\n");
        sprintf(buf + strlen(buf),"-------------\r\n");

        for (i = 0, j = 0; i <= top_of_world; i++)
          if (GET_ROOM_SCRIPT(i) && GET_ROOM_SCRIPT(i)->name)
            if (!strcmp(GET_ROOM_SCRIPT(i)->name, argument)) {
              sprintbit(ROOM_SCRIPT_FLAGS(i), rscript_bits, buf1);
              sprintf(buf2, "%3d. [%5ld] %-20s %s%s%s\r\n", ++j, GET_ROOM_VNUM(i), GET_ROOM_SCRIPT(i)->name, 
                CCCYN(ch, C_NRM), buf1, CCNRM(ch, C_NRM));
              if ((strlen(buf) + strlen(buf2)) < MAX_STRING_LENGTH)
                strcat(buf, buf2);
              else
                break;
            }
        page_string(ch->desc, buf, TRUE);
        break;
      }
      else if (!strcmp(value, "mob")) {
        sprintf(buf, "\r\nMobile Scripts:\r\n");
        sprintf(buf + strlen(buf), "---------------\r\n");
        j = 0;

        for (i = 0; i <= top_of_mobt; i++) 
          if (GET_MOB_SCRIPT(&mob_proto[i]) && GET_MOB_SCRIPT(&mob_proto[i])->name)
            if (!strcmp(GET_MOB_SCRIPT(&mob_proto[i])->name, argument)) {
              sprintbit(MOB_SCRIPT_FLAGS(&mob_proto[i]), mscript_bits, buf1);
              sprintf(buf2, "%3d. [%5d] %-20s %s%s%s\r\n", ++j, GET_MOB_VNUM(&mob_proto[i]),
                GET_MOB_SCRIPT(&mob_proto[i])->name, CCCYN(ch, C_NRM), buf1, CCNRM(ch, C_NRM));
              if ((strlen(buf) + strlen(buf2)) < MAX_STRING_LENGTH)
                strcat(buf, buf2);
              else
                break;
            }
        page_string(ch->desc, buf, TRUE);
        break;
      } else if (!strcmp(value, "obj")) {
        sprintf(buf, "Object Scripts:\r\n");
        sprintf(buf + strlen(buf), "---------------\r\n");

        for (i = 0, j = 0; i <= top_of_objt; i++)
          if (GET_OBJ_SCRIPT(&obj_proto[i]) && GET_OBJ_SCRIPT(&obj_proto[i])->name)
            if (!strcmp(GET_OBJ_SCRIPT(&obj_proto[i])->name, argument)) {
              sprintbit(OBJ_SCRIPT_FLAGS(&obj_proto[i]), oscript_bits, buf1);
              sprintf(buf2, "%3d. [%5d] %-20s %s%s%s\r\n", ++j, GET_OBJ_VNUM(&obj_proto[i]),
                GET_OBJ_SCRIPT(&obj_proto[i])->name, CCCYN(ch, C_NRM), buf1, CCNRM(ch, C_NRM));
              if ((strlen(buf) + strlen(buf2)) < MAX_STRING_LENGTH)
                strcat(buf, buf2);
              else
                break;
            }
        page_string(ch->desc, buf, TRUE);
        break;
      }      
      break;
    case 18:    /* skills */
      if (!*value) {
        send_to_char("A name would help.\r\n", ch);
        return;
      }

      sql_listskills(ch, value);
      break;
    case 19:    /* kills */
      if (!*value) {
        send_to_char("A name would help.\r\n", ch);
        return;
      }

      if (!(vict = get_char_vis(ch, value)) || IS_NPC(vict)) {
        sprintf(buf, "%s is not a valid player name.\r\n", value);
        send_to_char(buf, ch);
        return;
      }

      *buf = '\0';
      for (i = 0, j = 0; i < NUM_RACES; i++) {
        sprintf(buf + strlen(buf), "[&y%4d&n] %-19s", GET_RACE_KILLS(vict, i), mob_race_name(i));
        if (!(++j % 3))
          strcat(buf, "\r\n");
      }
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;

    default:
      send_to_char("Sorry, I don't understand that.\r\n", ch);
      break;
  }
}

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))
ACMD(do_set)
{
  int i, l, q = 0;
  struct char_data *vict = NULL, *cbuf = NULL;
  struct account_data *account = NULL;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], val_arg[MAX_INPUT_LENGTH];
  char old_name[MAX_INPUT_LENGTH];
  int on = 0, off = 0, value = 0;
  char is_file = 0, is_mob = 0, is_player = 0, is_account = 0;
  int find_name(char *name);
  int parse_race(char arg); /*utils.c*/
  struct account_data *sql_check_account(char *name);
  void sql_save_account(struct account_data *account);
  void sql_delete_account(char *name);
  void account_modify(struct descriptor_data *d);

  struct set_struct {
    char *cmd;
    char level;
    char pcnpc;
    char type;
  } fields[] = {
    { "brief",    LVL_GOD,     PC,     BINARY },  /* 0 */
    { "expq",     LVL_GRGOD,   PC,     NUMBER },  /* 1 */
    { "title",    LVL_GOD,     PC,     MISC },
    { "nosummon", LVL_GRGOD,   PC,     BINARY },
    { "maxhit",   LVL_GRGOD,   BOTH,   NUMBER },
    { "maxmana",  LVL_GRGOD,   BOTH,   NUMBER },  /* 5 */
    { "maxmove",  LVL_GRGOD,   BOTH,   NUMBER },
    { "hit",      LVL_GRGOD,   BOTH,   NUMBER },
    { "mana",     LVL_GRGOD,   BOTH,   NUMBER },
    { "move",     LVL_GRGOD,   BOTH,   NUMBER },
    { "align",    LVL_GOD,     BOTH,   NUMBER },  /* 10 */
    { "str",      LVL_GRGOD,   BOTH,   NUMBER },
    { "vitae",    LVL_GRGOD,   PC,     NUMBER },
    { "int",      LVL_GRGOD,   BOTH,   NUMBER },
    { "wis",      LVL_GRGOD,   BOTH,   NUMBER },
    { "dex",      LVL_GRGOD,   BOTH,   NUMBER },  /* 15 */
    { "con",      LVL_GRGOD,   BOTH,   NUMBER },
    { "sex",      LVL_GRGOD,   BOTH,   MISC },
    { "ac",       LVL_GRGOD,   BOTH,   NUMBER },
    { "gold",     LVL_GOD,     BOTH,   NUMBER },
    { "bank",     LVL_GOD,     PC,     NUMBER },  /* 20 */
    { "exp",      LVL_GRGOD,   BOTH,   NUMBER },
    { "hitroll",  LVL_GRGOD,   BOTH,   NUMBER },
    { "damroll",  LVL_GRGOD,   BOTH,   NUMBER },
    { "invis",    LVL_IMPL,    PC,     NUMBER },
    { "nohassle", LVL_GRGOD,   PC,     BINARY },  /* 25 */
    { "frozen",   LVL_FREEZE,  PC,     BINARY },
    { "stat_pts", LVL_GRGOD,   PC,     NUMBER },
    { "dye",      LVL_GRGOD,   PC,     MISC },
    { "drunk",    LVL_GRGOD,   BOTH,   MISC },
    { "hunger",   LVL_GRGOD,   BOTH,   MISC },    /* 30 */
    { "thirst",   LVL_GRGOD,   BOTH,   MISC },
    { "tired",    LVL_GRGOD,   BOTH,   MISC },
    { "name",     LVL_GRGOD,   PC,     MISC },
    { "level",    LVL_GRGOD,   BOTH,   NUMBER },
    { "room",     LVL_IMPL,    BOTH,   NUMBER },  /* 35 */
    { "roomflag", LVL_GRGOD,   PC,     BINARY },
    { "siteok",   LVL_GRGOD,   PC,     BINARY },
    { "deleted",  LVL_GRGOD,   PC,     BINARY },
    { "vitae_exp",LVL_GRGOD,   PC,     NUMBER },
    { "nowizlist",LVL_GOD,     PC,     BINARY },  /* 40 */
    { "quest",    LVL_GOD,     PC,     BINARY },
    { "loadroom", LVL_GRGOD,   PC,     MISC },
    { "color",    LVL_GOD,     PC,     BINARY },
    { "idnum",    LVL_IMPL-1,  PC,     NUMBER },
    { "passwd",   LVL_IMPL-1,  PC,     MISC },    /* 45 */
    { "nodelete", LVL_GOD,     PC,     BINARY },
    { "cha",      LVL_GRGOD,   BOTH,   NUMBER },
    { "olc",      LVL_SET_BUILD,   PC,   NUMBER }, 
    { "race",     LVL_GOD,     PC,     MISC }, 
    { "kills",    LVL_GRGOD,   BOTH,   NUMBER },  /* 50 */
    { "pks",      LVL_GRGOD,   BOTH,   NUMBER },
    { "deaths",   LVL_GRGOD,   BOTH,   NUMBER },
    { "home",     LVL_GRGOD,   PC,     NUMBER },
    { "tattoo",   LVL_GRGOD,   PC,     NUMBER },
    { "group_pts",LVL_GRGOD,   PC,     NUMBER }, /* 55 */
    { "chosen",   LVL_GRGOD,   PC,     BINARY },
    { "clan",     LVL_GRGOD,   PC,     NUMBER },
    { "outlaw",   LVL_GOD,     PC,     BINARY },
    { "year",     LVL_IMPL-1,  PC,     NUMBER },
    { "debug",    LVL_GRGOD,   PC,     BINARY },  /* 60 */
    { "\n", 0, BOTH, MISC }
  };

  half_chop(argument, name, buf);
  if (!*argument) {   /* Nothing entered, let's run through the available commands */
    send_to_char("Valid set commands are:\r\n\r\n", ch);
    for (i = 0, l = 0; fields[i].cmd != "\n"; i++) {
      sprintf(buf, "%-10s", fields[i].cmd);
      if (!(++l % 6))
        strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("\r\nUsage: set <type> <victim> <field> <value>\r\n", ch);
    return;
  }

  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "account")) {
    is_account = 1;
    half_chop(buf, name, buf);
  } else {
    send_to_char("Type must be file, player, mob, or account\r\n", ch);
    return;
  }
  
  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name || !*field) {
    send_to_char("Usage: set <type> <victim> <field> <value>\r\n", ch);
    return;
  }
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, 0))) {
        send_to_char("There is no such player.\r\n", ch);
        return;
      }
    } else if (is_mob) {
      if (!(vict = get_char_vis(ch, name))) {
        send_to_char("There is no such creature.\r\n", ch);
        return;
      }
    } else {
      if (!(account = sql_check_account(name))) {
        send_to_char("There is no such account.\r\n", ch);
        return;
      }
    }
  } else if (is_file) {
    if ((cbuf = load_char(name))) {
      if ((str_cmp("Orodreth", GET_NAME(ch)) != 0) && (str_cmp("Serapis", GET_NAME(ch)) != 0))
        if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
          free_char(cbuf);
          send_to_char("Sorry, you can't do that.\r\n", ch);
          return;
        }
      vict = cbuf;
    } else {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }
  if (GET_LEVEL(ch) != LVL_IMPL)  /* let imps do whatever they want :-) */
    if (!is_account)
      if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
        send_to_char("Maybe that's not such a great idea...\r\n", ch);
        free_char(cbuf);
        return;
      }

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    free_char(cbuf);
    free_account(account);
    return;
  }

  if (!is_account) {
    if (IS_NPC(vict) && !(fields[l].pcnpc & NPC)) {
      send_to_char("You can't do that to a beast!\r\n", ch);
      free_char(cbuf);
      return;
    } else if (!IS_NPC(vict) && !(fields[l].pcnpc & PC)) {
      send_to_char("That can only be done to a beast!\r\n", ch);
      free_char(cbuf);
      return;
    }
  }
  
  if (fields[l].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be on or off.\r\n", ch);
      free_char(cbuf);
      free_account(account);
      return;
    }
  } else if (fields[l].type == NUMBER)
    value = atoi(val_arg);

  strcpy(buf, "Okay.");  /* can't use OK macro here 'cause of \r\n */
  switch (l) {
    case 0:
      vict->points.exp = RANGE(0, 50000000);
      break;
    case 1:
      GET_EXP_Q(vict) = RANGE(0, 50000000);
      break;
    case 2:
      set_title(vict, val_arg);
      sprintf(buf, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
      break;
    case 3:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
      on = !on;      /* so output will be correct */
      break;
    case 4:
      vict->points.max_hit = RANGE(1, 5000);
      affect_total(vict);
      break;
    case 5:
      vict->points.max_mana = RANGE(1, 5000);
      affect_total(vict);
      break;
    case 6:
      vict->points.max_move = RANGE(1, 5000);
      affect_total(vict);
      break;
    case 7:
      vict->points.hit = RANGE(-9, vict->points.max_hit);
      affect_total(vict);
      break;
    case 8:
      vict->points.mana = RANGE(0, vict->points.max_mana);
      affect_total(vict);
      break;
    case 9:
      vict->points.move = RANGE(0, vict->points.max_move);
      affect_total(vict);
      break;
    case 10:
      GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
      affect_total(vict);
      break;
    case 11:
      vict->real_abils.str = RANGE(0, 25);
      affect_total(vict);
      break;
    case 12:
      vitae_af(vict, FALSE);
      GET_VITAE(vict) = RANGE(0, 25);
      vitae_af(vict, TRUE);
      break;
    case 13:
      vict->real_abils.intel = RANGE(0, 25);
      affect_total(vict);
      break;
    case 14:
      vict->real_abils.wis = RANGE(0, 25);
      affect_total(vict);
      break;
    case 15:
      vict->real_abils.dex = RANGE(0, 25);
      affect_total(vict);
      break;
    case 16:
      vict->real_abils.con = RANGE(0, 25);
      affect_total(vict);
      break;
    case 17:
      if (!str_cmp(val_arg, "male"))
        vict->player.sex = SEX_MALE;
      else if (!str_cmp(val_arg, "female"))
        vict->player.sex = SEX_FEMALE;
      else if (!str_cmp(val_arg, "neutral"))
        vict->player.sex = SEX_NEUTRAL;
      else {
        send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
        free_char(cbuf);
        return;
      }
      break;
    case 18:
      vict->points.armor = RANGE(-200, 100);
      affect_total(vict);
      break;
    case 19:
      GET_GOLD(vict) = RANGE(0, 100000000);
      break;
    case 20:
      GET_BANK_GOLD(vict) = RANGE(0, 100000000);
      break;
    case 21:
      vict->points.exp = RANGE(0, 50000000);
      break;
    case 22:
      vict->points.hitroll = RANGE(-20, 200);
      affect_total(vict);
      break;
    case 23:
      vict->points.damroll = RANGE(-20, 200);
      affect_total(vict);
      break;
    case 24:
      if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
        send_to_char("You aren't godly enough for that!\r\n", ch);
        return;
      }
      GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
      break;
    case 25:
      if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
        send_to_char("You aren't godly enough for that!\r\n", ch);
        free_char(cbuf);
        return;
      }
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
      break;
    case 26:
      if (ch == vict) {
        send_to_char("Better not -- could be a long winter!\r\n", ch);
        return;
      }
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
      break;
    case 27:
      GET_STAT_PTS(vict) = RANGE(0, 100);
      break;
    case 28:
      tattoo_af(vict, FALSE);
      for (i = 0; i < NUM_DYES; i++)
        if (!str_cmp(dyes[i], val_arg)) {
          SET_BIT(GET_DYE(vict), 1 << i);
          break;
        } else if (!strcmp(val_arg, ""))
          REMOVE_BIT(GET_DYE(vict), 1 << i);
      tattoo_af(vict, TRUE);
      affect_total(vict);
      break;
    case 29:
    case 30:
    case 31:
    case 32:
      if (!str_cmp(val_arg, "off")) {
        GET_COND(vict, (l - 29)) = (char) -1;
        sprintf(buf, "%s's %s now off.", GET_NAME(vict), fields[l].cmd);
      } else if (is_number(val_arg)) {
        value = atoi(val_arg);
        RANGE(0, 48);
        GET_COND(vict, (l - 29)) = (char) value;
        sprintf(buf, "%s's %s set to %d.", GET_NAME(vict), fields[l].cmd, value);
      } else {
        send_to_char("Must be 'off' or a value from 0 to 48.\r\n", ch);
        free_char(cbuf);
        return;
      }
      break;
    case 33:
      if ((q = find_name(GET_NAME(vict))) != -1)
         strcpy((player_table + q)->name, val_arg); 
       strcpy(old_name, GET_NAME(vict));
       strcpy(GET_NAME(vict), val_arg); 
       save_char(vict, NOWHERE);
      break;
    case 34:
      if (value >= GET_LEVEL(ch) || value > LVL_IMPL) {
        send_to_char("You can't do that.\r\n", ch);
        free_char(cbuf);
        return;
      }
      RANGE(0, LVL_IMPL);
      vict->player.level = (byte) value;
      break;
    case 35:
      if ((i = real_room(value)) < 0) {
        send_to_char("No room exists with that number.\r\n", ch);
        return;
      }
      char_from_room(vict);
      char_to_room(vict, i);
      break;
    case 36:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
      break;
    case 37:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
      break;
    case 38:
      if (is_file || is_player) {
        SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
        sprintf(buf, "%s (lev %d) marked for deletion by %s.", GET_NAME(vict), GET_LEVEL(vict), GET_NAME(ch));
        mudlog(buf, NRM, LVL_IMMORT, TRUE);
      } else if (is_account) {
        for (i = 0; i < MAX_CHARS; i++)
          if (account->chars[i]) {
            sql_Crash_delete_file(account->chars[i]);
            if ((cbuf = load_char(get_name_by_id(account->chars[i])))) {
              sql_delete_char(cbuf);
              free_char(cbuf);
            }
          }
        sql_delete_account(account->name);
        sprintf(buf, "Account \"%s\" has been deleted by %s.", account->name, GET_NAME(ch));
        mudlog(buf, NRM, LVL_IMMORT, TRUE);
      }
      break;
    case 39:
      GET_VITAE_EXP(vict) = RANGE(0, 9000000);
      break;
    case 40:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
      break;
    case 41:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
      break;
    case 42:
      if (!str_cmp(val_arg, "off"))
        REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
      else if (is_number(val_arg)) {
        value = atoi(val_arg);
        if (real_room(value) != NOWHERE) {
          SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
          GET_LOADROOM(vict) = value;
          sprintf(buf, "%s will enter at room #%ld.", GET_NAME(vict), GET_LOADROOM(vict));
        } else {
          sprintf(buf, "That room does not exist!");
        }
      } else {
        strcpy(buf, "Must be 'off' or a room's virtual number.\r\n");
      }
      break;
    case 43:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_1);
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_2);
      break;
    case 44:
      if (GET_IDNUM(ch) != 1 || IS_NPC(vict))
        return;
      GET_IDNUM(vict) = value;
      break;
    case 45:
      if (!is_account) {
        send_to_char("You must use set account with this command.\r\n", ch);
        return;
      } else if (!*val_arg) {
        send_to_char("A password must contain at least 1 character!\r\n", ch);
        return;
      }

      if (account->pwd)
        FREE(account->pwd);
      account->pwd = str_dup(CRYPT(val_arg, account->name));

      sprintf(buf, "Password changed to '%s'.", val_arg);
      sql_save_account(account);
      break;
    case 46:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
      break;
    case 47:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(0, 25);
      else
        RANGE(0, 18);
      vict->real_abils.cha = value;
      affect_total(vict);
      break;
    case 48:
      GET_OLC_ZONE(vict) = value;
      break;
    case 49:
      if ((i = parse_race(*val_arg)) == RACE_UNDEFINED) {
        send_to_char("That is not a race.\r\n", ch);
        free_char(cbuf);
        return;
      }
      vict->player_specials->saved.race = i;
      break;
    case 50:
      GET_KILLS(vict) = RANGE(-100, 65534);
      break;
    case 51:
      GET_PKS(vict) = RANGE(-100, 65534);
      break;
    case 52:
      GET_DEATHS(vict) = RANGE(-100, 65534);
      break;
    case 53:
      GET_HOME(vict) = value;
      break;
    case 54:
      tattoo_af(vict, FALSE);
      GET_TATTOO(vict) = RANGE(0, NUM_TATTOOS - 1);
      tattoo_af(vict, TRUE);
      affect_total(vict);
      break;
    case 55:
      GET_GROUP_PTS(vict) = RANGE(0, 100);
      break;
    case 56:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_CHOSEN);
      break;
    case 57:
        GET_CLAN(vict) = value;
        if (value == 0)
          GET_CLAN_RANK(vict) = 0;
        break;
    case 58:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_OUTLAW);
      break;
    case 59:
      time_info.year = value;
      break;
    case 60:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DEBUG);
      break;
    default:
      sprintf(buf, "Can't set that!");
      break;
    }

  if ((fields[l].type == BINARY) && !is_account) {
    sprintf(buf, "%s %s for %s.\r\n", fields[l].cmd, ONOFF(on), GET_NAME(vict));
    CAP(buf);
  } else if ((fields[l].type == NUMBER) && !is_account) {
    sprintf(buf, "%s's %s set to %d.\r\n", GET_NAME(vict), fields[l].cmd, value);
  } else
    strcat(buf, "\r\n");
  send_to_char(CAP(buf), ch);

  if (!is_file && vict)
    if (!IS_NPC(vict))
      save_char(vict, NOWHERE);

  if (is_file) {
    save_char(vict, GET_LOADROOM(vict));
    free_char(cbuf);
    send_to_char("Saved in file.\r\n", ch);
  }
  free_account(account);
}

static char *logtypes[] = {
"off", "brief", "normal", "complete", "\n"};

ACMD(do_syslog)
{
  int tp;

  one_argument(argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1);
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG2);
  if (tp & 1) SET_BIT(PRF_FLAGS(ch), PRF_LOG1);
  if (tp & 2) SET_BIT(PRF_FLAGS(ch), PRF_LOG2);
  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}


/* dnsmod */
ACMD(do_dns)
{
  int i;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char ip[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
  char buf[16384];
  struct dns_entry *dns, *tdns;

  extern struct dns_entry *dns_cache[DNS_HASH_NUM];

  void save_dns_cache(void);

  half_chop(argument, arg1, arg2);

  if(!*arg1) {
    send_to_char("You shouldn't be using this if you don't know what it does!\r\n", ch);
    return;
  }

  if(is_abbrev(arg1, "delete")) {
    if(!*arg2) {
      send_to_char("Delete what?\r\n", ch);
      return;
    }
    CREATE(dns, struct dns_entry, 1);
    if(sscanf(arg2, "%d.%d.%d", dns->ip, dns->ip + 1,
      dns->ip + 2) != 3) {
      send_to_char("Delete what?\r\n", ch);
      return;
    }
    for(i = 0; i < DNS_HASH_NUM; i++) {
      if(dns_cache[i]) {
      for(tdns = dns_cache[i]; tdns; tdns = tdns->next) {
        if(dns->ip[0] == tdns->ip[0] && dns->ip[1] == tdns->ip[1] &&
          dns->ip[2] == tdns->ip[2]) {
          sprintf(arg1, "Deleting %s.\r\n", tdns->name);
          send_to_char(arg1, ch);
          tdns->ip[0] = -1;
        }
      }
      }
    }
    save_dns_cache();
    return;
  } else if(is_abbrev(arg1, "add")) {
    two_arguments(arg2, ip, name);
    if(!*ip || !*name) {
      send_to_char("Add what?\r\n", ch);
      return;
    }
    CREATE(dns, struct dns_entry, 1);
    dns->ip[3] = -1;
    if(sscanf(ip, "%d.%d.%d.%d", dns->ip, dns->ip + 1,
      dns->ip + 2, dns->ip + 3) < 3) {
      send_to_char("Add what?\r\n", ch);
      return;
    }
    i = (dns->ip[0] + dns->ip[1] + dns->ip[2]) % DNS_HASH_NUM;
    dns->name = str_dup(name);
    dns->next = dns_cache[i];
    dns_cache[i] = dns;
    save_dns_cache();
    send_to_char("OK!\r\n", ch);
    return;
  } else if(is_abbrev(arg1, "list")) {
    *buf = '\0';
    sprintf(buf, "IP Address        Host Name\r\n");
    for(i = 0; i < DNS_HASH_NUM; i++) {
      if(dns_cache[i]) {
         for(tdns = dns_cache[i]; tdns; tdns = tdns->next) {
           if (tdns->ip[0] < 0) 
              break;
           if (tdns->ip[0] < 10) 
              sprintf(buf, "%s00%d.", buf, tdns->ip[0]);
           else if (tdns->ip[0] < 100)
              sprintf(buf, "%s0%d.", buf, tdns->ip[0]);
           else
              sprintf(buf, "%s%d.", buf, tdns->ip[0]);
           if (tdns->ip[1] < 10) 
              sprintf(buf, "%s00%d.", buf, tdns->ip[1]);
           else if (tdns->ip[1] < 100)
              sprintf(buf, "%s0%d.", buf, tdns->ip[1]);
           else
              sprintf(buf, "%s%d.", buf, tdns->ip[1]);
           if (tdns->ip[2] < 10) 
              sprintf(buf, "%s00%d.", buf, tdns->ip[2]);
           else if (tdns->ip[2] < 100)
              sprintf(buf, "%s0%d.", buf, tdns->ip[2]);
           else
              sprintf(buf, "%s%d.", buf, tdns->ip[2]);
           if (tdns->ip[3] < 0)
              sprintf(buf, "%s   ", buf);
           else if (tdns->ip[3] < 10) 
              sprintf(buf, "%s00%d", buf, tdns->ip[3]);
           else if (tdns->ip[3] < 100)
              sprintf(buf, "%s0%d", buf, tdns->ip[3]);
           else
              sprintf(buf, "%s%d", buf, tdns->ip[3]);
           sprintf(buf, "%s   %s\r\n", buf, tdns->name);
         }
      }
    }
    page_string(ch->desc, buf, 1);
    return;
  }
}


ACMD(do_dark)
{
	struct char_data *rch;
    
	if (IS_NPC(ch))
       		return;
   	for (rch = world[ch->in_room].people;rch;rch = rch->next_in_room)
     	{
         	if (FIGHTING(rch))
         	{
           		stop_fighting(rch);
           		if (IS_MOB(rch) && MEMORY(rch))
				MEMORY(rch) = NULL;		   
        		send_to_char("The peace of the ancients fills your"
				     " soul.\n\r", rch);
         	}
      }
      send_to_char("You stop the senseless violence in the room with a "
 	            "wave of your hand.\r\n",ch);
      act ("$n stops the senseless violence in the room with a "
 	            "wave of $s hand.",TRUE, ch, 0, 0, TO_ROOM);
      return;
}

ACMD(do_home)
{ 
 int location;
 char buf[MAX_STRING_LENGTH], tmp[10];
 
 if (IS_NPC(ch))
    return;
 one_argument(argument, tmp);
 if (!*tmp)
 {
   if((location = real_room(GET_LOADROOM(ch))) <= 0)
        {
        send_to_char("That room does not exist.\n\r",ch);
        GET_LOADROOM(ch) = 1;
        location = 1;
        send_to_char("Error in your home room. Now set to Limbo.\n\r",ch);
   }
   sprintf(buf, "You feel your soul wrenched as you are");
   sprintf(buf, "%s pulled into a different reality.\r\n", buf);
   send_to_char(buf, ch);
   send_to_char("\n\r",ch);
   if (POOFOUT(ch))
      sprintf(buf, "%s", POOFOUT(ch));
   else
      sprintf(buf, "$n disappears in a blaze of hellfire!");
   act(buf, TRUE, ch, 0, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, location);
   act(buf, TRUE, ch, 0, 0, TO_ROOM);
   do_look(ch,"",15,0);
   return;
 }
 else
 {
  if (isdigit(*tmp))
  {
   SET_BIT(PLR_FLAGS(ch), PLR_LOADROOM);
   location = atoi(tmp);
   
   if (find_target_room(ch, argument) < 0)
        {
        send_to_char("That room does not exist.\n\r",ch);
        return;
        }

   if ( real_room(location) > 0 && 
	IS_SET(world[real_room(location)].room_flags, ROOM_GODROOM) && 
       	GET_LEVEL(ch) < LEVEL_GRGOD )
   {
        send_to_char("You're not godly enough for that room.\r\n",ch);
        return;
   }

   GET_LOADROOM(ch) = location;
   sprintf(buf, "Home room set to %d.\n\r", location);
   send_to_char(buf, ch);
   return;
  }
  else
  {
  send_to_char("Home or Home <room-number>\n\r", ch);
  return;
  }
 }
} 

ACMD (do_olist)
{
  extern struct index_data *obj_index;
  extern struct obj_data *obj_proto;
 
  int j, nr, start, end, found = 0;
  char f;
 
  one_argument(argument, arg);
 
  j = atoi(arg);
  start = (j * 100);
  end = (start + 99);
  f = 'N';
  *buf = '\0';
  for (nr = 0; nr <= top_of_objt; nr++)
    if ((obj_index[nr].virtual >= start) && (obj_index[nr].virtual <= end))
      {
        f = 'Y';
	sprintf(buf, "%s%3d. [%5d] %s\r\n", buf, ++found,
		obj_index[nr].virtual,
		obj_proto[nr].short_description);
      }
  
  if (f == 'N')
    sprintf(buf, "Sorry, there are no objs in that zone.\r\n");
  else
    sprintf(buf, "%s %d Objects found in Zone %d\r\n", buf, found, j);
 
  page_string(ch->desc, buf, 1);
}

ACMD(do_tlist)
{
  extern struct tlist_data *treasure_list;
  extern int top_of_treasure;
  struct obj_data *obj;
  int i, j;

  one_argument(argument, arg);
  j = atoi(arg);

  if ((j < 1) || (j > top_of_treasure)) {
    send_to_char("The desired treasure list does not exist.\r\n", ch);
    return;
  }

  sprintf(buf, "%s\r\n", treasure_list[j-1].type);
  for (i = 0; i < treasure_list[j-1].num_of_items; i++) {
    obj = read_object(treasure_list[j-1].vnum[i], VIRTUAL, NULL);
    sprintf(buf, "%s%3d. [%5ld] %s\r\n", buf, i+1, treasure_list[j-1].vnum[i], obj->short_description);
    extract_obj(obj);
  }

  page_string(ch->desc, buf, 1);
}

ACMD(do_rlist)
{
  int i, j, ii, count = 0;
  bool found = FALSE;
  char buf1[(MAX_STRING_LENGTH*2)];
  char buf2[MAX_STRING_LENGTH];

  one_argument(argument, arg);
 
  j = atoi(arg);
  buf1[0] = '\0';
 
  for (i = 0; i <= top_of_zone_table; i++)
    if (zone_table[i].number == j)
      break;
 
 
  for (ii = 0; ii <= top_of_world; ii++)
    if (world[ii].zone == i)
      {
	count++;
	found = TRUE;
	snprintf(buf2, MAX_STRING_LENGTH, "[%5ld] %s\r\n", world[ii].number, world[ii].name);
        if ((strlen(buf1) + strlen(buf2)) < (MAX_STRING_LENGTH*2))
          strcat(buf1, buf2);
        else
        {
          send_to_char("Room list is too long and is truncated.\r\n", ch);
          break;
        }
      }

 
  if (!found) {
    send_to_char("The desired zone does not exist.\r\n", ch);
    return;
  }
  page_string(ch->desc, buf1, 1);
}


ACMD (do_mlist)
{
  extern struct index_data *mob_index;   /* index table for mobile file   */
  extern struct char_data *mob_proto;    /* prototypes for mobs           */
 
 
  int j, nr, start, end, found = 0;
  char f;
 
  one_argument(argument, arg);
 
  j = atoi(arg);
  start = (j * 100);
  end = (start + 99);
  f = 'N';
  *buf = '\0';
 
  for (nr = 0; nr <= top_of_mobt; nr++) 
    if ((mob_index[nr].virtual >= start) && (mob_index[nr].virtual <= end))
      {
        f = 'Y';
	sprintf(buf, "%s%3d. [%5d] %s\r\n", buf,++found,
		mob_index[nr].virtual,
		mob_proto[nr].player.short_descr);
      }
  if (f == 'N')
    sprintf(buf, "Sorry, there are no mobs in that zone.\r\n");
  else
    sprintf(buf, "%s %d Mobiles found in Zone %d\r\n", buf, found, j);
 
  page_string(ch->desc, buf, 1);
}

int file_to_string(char *name, char *buf);
int file_to_string_alloc(char *name, char **buf);

ACMD(do_sysfile)
{
  char *cp;
  char *readfile = NULL;

  one_argument(argument, arg);

  if (is_abbrev(arg, "bugs"))
    cp = BUG_FILE;
  else if (is_abbrev(arg, "ideas"))
    cp = IDEA_FILE;
  else if (is_abbrev(arg, "todo"))
    cp = TODO_FILE;
  else if (is_abbrev(arg, "typos"))
    cp = TYPO_FILE;
  else if (is_abbrev(arg, "help"))
    cp = HELP_FILE;
  else {
    send_to_char("That isn't a file!\r\n", ch);
    return;
  }

  if (file_to_string_alloc(cp, &readfile) < 0) {
    sprintf(buf, "%s attempted to read the %s, error while reading.\r\n", GET_NAME(ch), cp);
    mudlog(buf, BRF, GET_LEVEL(ch) + 1, TRUE);
    send_to_char("Error reading file, please speak to a higher level staff member.\r\n", ch);
    return;
  }

  page_string(ch->desc, readfile, 1);
  FREE(readfile);
}

ACMD(do_sethunt)
{
  struct char_data *victim;
  struct char_data *hunter;

  if (IS_NPC(ch))
	return;

  half_chop(argument, buf, buf2);

  if(!*buf)
	send_to_char("Who do you wish to hunt?\n\r", ch);
  else if (!(victim = get_char_vis(ch, buf)))
	send_to_char("No-one by that name around.\n\r", ch); 
  else if (!(hunter = get_char_vis(ch, buf2)))
	send_to_char("Who shall be the hunter?\n\r",ch);
  else if (hunter==victim)
	send_to_char("Yeah right.\n\r",ch);
  else if (!IS_NPC(hunter))
	send_to_char("PCs can't be made to hunt.\n\r",ch);
  else {
	if(GET_LEVEL(ch)<GET_LEVEL(victim)) {
		send_to_char("Cant hunt higher than your level.\n\r",ch); 
		return;
	}
 	if(!MOB_FLAGGED(hunter,MOB_HUNTER))
                SET_BIT(MOB_FLAGS(hunter),MOB_HUNTER);
        set_hunting(hunter, victim);
	send_to_char("Ok, they're fucked.\n\r",ch);
  }
}

int
random_room_in_zone(int znum)
{
    int to_room = 0;
    int top = zone_table[znum].top;

    if (znum >= 0 && znum <= top_of_zone_table) {
     do {
          to_room = number(0, top);
     }  
     while( (IS_SET(world[to_room].room_flags,ROOM_PRIVATE))
         ||(IS_SET(world[to_room].room_flags,ROOM_GODROOM))
         ||(IS_SET(world[to_room].room_flags,ROOM_DEATH))
         ||(IS_SET(world[to_room].room_flags,ROOM_HOUSE))
         ||(IS_SET(world[to_room].room_flags,ROOM_ATRIUM))
         ||(IS_SET(world[to_room].room_flags,ROOM_NOMOB))
         ||(world[to_room].zone != zone_table[znum].number) );
    }
    return( to_room );
}

/*-------------------------------------------------------------------------*\
  tick -- make the mud tick now!
\*-------------------------------------------------------------------------*/

ACMD(do_tick)
{
  void weather_and_time(int mode);
  void affect_update(void);
  void hunt_items(void);
  void check_shop_times(void);

  weather_and_time(1);
  affect_update();
  hunt_items();
  check_shop_times();
  hunger_and_thirst();
  point_update();
  item_update();
}  

ACMD(do_newbie)
{
  struct char_data *vict;
  struct obj_data *letter = NULL;

  two_arguments(argument, buf, buf2);
  if (!*buf)
    send_to_char("Whom do you wish to newbie?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (!*buf2)
    send_to_char("What lifestyle should this person be equipped to?\r\n", ch);
  else {
    if (!strncmp("poor", buf2, 4))
      letter = read_object(real_object(1247), REAL, NULL);
    else if (!strncmp("average", buf2, 7))
      letter = read_object(real_object(1246), REAL, NULL);
    else if (!strncmp("rich", buf2, 4))
      letter = read_object(real_object(1245), REAL, NULL);
    else {
      send_to_char("The player can be poor, average or rich. Please specify one.\r\n", ch);
      return;
    }

    if (letter) {
      GET_OBJ_VAL(letter, 0) = GET_IDNUM(vict);
      obj_to_char(letter, vict);
      send_to_char("Newbied.\r\n", ch);
      act("$N makes a magical gesture creating a letter of introduction, and hands it to you!", TRUE, vict, 0, ch, TO_CHAR);
      sprintf(buf, "(GC) %s newbied %s.", GET_NAME(ch), GET_NAME(vict));
      mudlog(buf, BRF, GET_LEVEL(ch)+1, TRUE);
    } else {
      send_to_char("There was an error creating the letter of introduction.\r\n", ch);
      sprintf(buf, "SYSERR: Unable to load letter prototype in do_newbie.");
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    }
  }
}

ACMD(do_zlist)
{
        FILE* f = NULL;
	int j = 0;
        char cp[256];
  
	one_argument(argument, arg);
 
  	j = atoi(arg);

	if (!arg || !*arg)
	  j = zone_table[ world[ch->in_room].zone ].number;

	sprintf(cp, "world/zon/%d.zon", j);
	
        f = fopen(cp,"r");
	if (f)
	{
          fread(buf, MAX_STRING_LENGTH-5,1,f);
          buf[MAX(0,ftell(f))]=0;
          fclose(f);

          buf[MAX_STRING_LENGTH-1]=0;
          page_string(ch->desc, buf, 1);
	}
	else
	  stc("No zone file for that number.\r\n", ch);
}


ACMD(do_idlist) {

  /* we want to loop through the object list and do an identify of it into
     a given text file.  obviously this would not be done too often, as it
     will probably sap the mud for a few seconds. */

  FILE * fp;
  char filename[] = "object_idlist";
  int nr, type, found, i;

  /* open the file here */
  fp = fopen(filename, "w");
  if (fp == NULL) {
    stc("Could not open id list file, cannot complete operation!\r\n", ch);
    return;
  }

  /* loop here */
  for (nr = 0; nr <= top_of_objt; nr++) {
    fprintf(fp, "Object: '%s', Item type: ", 
	    obj_proto[nr].short_description);
    type = obj_proto[nr].obj_flags.type_flag;
    if(type == ITEM_STAFF) {
      fprintf(fp, "CASTING_EQ\n");
    } else {
      sprinttype(type, item_types, buf);
      fprintf(fp, "%s\n", buf);
    }
    fprintf(fp, "Item will give the following abilities:  ");
    sprintbit(obj_proto[nr].obj_flags.bitvector, affected_bits, buf);
    fprintf(fp, "%s\n", buf);
    fprintf(fp, "Item is: ");
    sprintbit(obj_proto[nr].obj_flags.extra_flags, extra_bits, buf);
    fprintf(fp, "%s\n", buf);
    fprintf(fp, "Encumbrance: %d, Value: %d\n", 
	    obj_proto[nr].obj_flags.weight, obj_proto[nr].obj_flags.cost);
    switch(type) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      fprintf(fp, "This %s casts: ", item_types[type]);
      if (obj_proto[nr].obj_flags.value[1] >= 1)
	fprintf(fp, "%s", spell_info[obj_proto[nr].obj_flags.value[1]].name);
      if (obj_proto[nr].obj_flags.value[2] >= 1)
	fprintf(fp, "%s", spell_info[obj_proto[nr].obj_flags.value[2]].name);
      if (obj_proto[nr].obj_flags.value[3] >= 1)
	fprintf(fp, "%s", spell_info[obj_proto[nr].obj_flags.value[3]].name);
      fprintf(fp, "\n");
      break;
    case ITEM_STAFF:
      fprintf(fp, "This %s casts: ", item_types[type]);
      fprintf(fp, "%s", spell_info[obj_proto[nr].obj_flags.value[3]].name);
      fprintf(fp, "It has %ld maximum charge%s and %ld remaining.\n",
	      obj_proto[nr].obj_flags.value[1],
	      obj_proto[nr].obj_flags.value[1] == 1 ? "" : "s",
	      obj_proto[nr].obj_flags.value[2]);
      fprintf(fp, "\n");
      break;
    case ITEM_WEAPON:
      fprintf(fp, "Damage Dice is '%ldD%ld' for an average per-round damage "
	      "of %.1f.\n",
	      obj_proto[nr].obj_flags.value[1],
	      obj_proto[nr].obj_flags.value[2],
	      (((obj_proto[nr].obj_flags.value[2] + 1) / 2.0) *
	       obj_proto[nr].obj_flags.value[1])
	      );
      break;
    case ITEM_ARMOR:
      fprintf(fp, "AC-apply is %ld\n",
	      obj_proto[nr].obj_flags.value[0]);
      break;
    }

    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj_proto[nr].affected[i].location != APPLY_NONE) &&
	  (obj_proto[nr].affected[i].modifier != 0)) {
	if (!found) {
	  fprintf(fp, "Can affect you as :\n");
	  found = TRUE;
	}
	sprinttype(obj_proto[nr].affected[i].location, apply_types, buf);
	fprintf(fp, "   Affects: %s By %d\n",
		buf, obj_proto[nr].affected[i].modifier);
      }
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
  stc("Ok. Id list complete.\r\n", ch);
}


ACMD(do_idlist2) {

  /* we want to loop through the object list and do an identify of it into
     a given text file.  obviously this would not be done too often, as it
     will probably sap the mud for a few seconds. */

  FILE * fp;
  char filename[] = "objects.txt";
  int nr, type;
  char  sbuf[MAX_STRING_LENGTH];

  /* open the file here */
  fp = fopen(filename, "w");
  if (fp == NULL) {
    stc("Could not open id list file, cannot complete operation!\r\n", ch);
    return;
  }

  /* loop here */
  for (nr = 0; nr <= top_of_objt; nr++) {
    type = obj_proto[nr].obj_flags.type_flag;
    if (type != ITEM_WEAPON)
      continue;
    *sbuf = '\0';
    sprintf(sbuf, "%d, %s, %s,", 
	    obj_proto[nr].item_number, obj_proto[nr].short_description,
            weapon_names[obj_proto[nr].obj_flags.value[0]]);

    sprintbit(obj_proto[nr].obj_flags.bitvector, affected_bits, buf);
    sprintf(sbuf + strlen(sbuf), "%s,", buf);
    sprintbit(obj_proto[nr].obj_flags.extra_flags, extra_bits, buf);
    sprintf(sbuf + strlen(sbuf), "%s,", buf);
    sprintf(sbuf + strlen(sbuf), "%d, %d,", 
	    obj_proto[nr].obj_flags.weight, obj_proto[nr].obj_flags.cost);
    sprintf(sbuf + strlen(sbuf), "%ldD%ld, %.1f,",
	      obj_proto[nr].obj_flags.value[1],
	      obj_proto[nr].obj_flags.value[2],
	      (((obj_proto[nr].obj_flags.value[2] + 1) / 2.0) *
	       obj_proto[nr].obj_flags.value[1])
	      );
    fprintf(fp, sbuf);
    fprintf(fp, "\n");
  }
  fclose(fp);
  stc("Ok. Id list complete.\r\n", ch);
}


/* slay a sucka right dead in his tracks */

ACMD(do_slay)
{
  struct char_data *vict;
  one_argument(argument, arg);
  
  if (GET_LEVEL(ch) < LEVEL_IMPL-1 || IS_NPC(ch))
    return;

  if (!*arg) 
    send_to_char("Kill who?\r\n", ch);
  else
    { 
      if (!(vict = get_char_room_vis(ch, arg)))   
        send_to_char("They aren't here.\r\n", ch);
      else if (ch == vict)
        send_to_char("Your mother would be so sad.. :(\r\n", ch);
      else if (GET_LEVEL(vict) == GET_LEVEL(ch))
        send_to_char("No can do, buddy.. \r\n", ch);
      else {
        act("You chop $M to pieces!  Ah!  The blood!",
            FALSE,ch,0,vict, TO_CHAR);
        act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
        act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
        raw_kill(vict, ch, TYPE_SLASH);
      }
    }
}

ACMD(do_scroll)
{
  int i = 0;
  struct obj_data *scroll;
  extern struct obj_data *create_spell_scroll(int spellnum, int group, int level, char *name);
  extern struct skill_tree_data skill_tree[];

  int mob = number(1, 100); 
  int tree_idx = 0;
  int spells_by_level[MAX_SPELLS+1];
  ush_int spell = 0;
  int x = 1;
  skip_spaces(&argument);
  if (!*argument) {
    send_to_char("Usage: scroll <spell name>\r\n", ch);
    send_to_char("No argument detected, creating random scroll.\r\n", ch);


  while (*skill_tree[++tree_idx].name != '\n')
   if ((skill_tree[tree_idx].level <= mob) && (skill_tree[tree_idx].skill_number <= MAX_SPELLS)) {
      spells_by_level[x] = tree_idx;
      x++;
    }

  spell = spells_by_level[number(1, x-1)];

  if (!strcmp(skill_tree[spell].name, "\n")) {
    sprintf(buf, "SYSERR: Attempting invalid spell assign (%d) in load_spell_scroll.",
      spell);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  if (!(scroll = create_spell_scroll(skill_tree[spell].skill_number, skill_tree[spell].group, skill_tree[spell].level, skill_tree[spell].name)))
    return;
  obj_to_char(scroll, ch);
  return;
  }

  /* Find the spell */
  while (*skill_tree[++i].name != '\n')
    if (!str_cmp(skill_tree[i].name, argument))
      break;

  if (!strcmp(skill_tree[i].name, "\n")) {
    send_to_char("That is not a valid spell!\r\n", ch);
    return;
  }

  if (!(scroll = create_spell_scroll(skill_tree[i].skill_number, skill_tree[i].group, skill_tree[i].level, skill_tree[i].name)))
    return;

  sprintf(buf, "A '%s' scroll has been created.\r\n", skill_tree[i].name);
  send_to_char(buf, ch);
  obj_to_char(scroll, ch);
}

ACMD(do_where)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num, found = 0;
  extern char *print_object_location(int num, struct obj_data * obj, struct char_data * ch);
  extern char *sql_do_where(char *searchname, int num);

  one_argument(argument, arg);
  *buf = '\0'; 

  if (!*arg) {  
    send_to_char("Players visible to you\r\n----------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
        i = (d->original ? d->original : d->character);
        if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
          if (d->original)
            sprintf(buf, "%s%-20s - [%s%5ld%s] %s (in %s)\r\n",
                    buf, GET_NAME(i), CCGRN(ch, C_CMP),
                    world[d->character->in_room].number, CCNRM(ch, C_CMP),
                    world[d->character->in_room].name, GET_NAME(d->character));
          else
            sprintf(buf, "%s%-20s - [%s%5ld%s] %s\r\n", buf, GET_NAME(i),
                    CCGRN(ch, C_CMP), world[i->in_room].number, CCNRM(ch, C_CMP),
                    world[i->in_room].name);
        }
      }
      page_string(ch->desc, buf, 1);
  } else {
    for (num = 0, i = character_list; i; i = i->next)
      if (i->in_room != NOWHERE && isname_with_abbrevs(arg, i->player.name)) {
        found = 1;
        snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "M%3d. %-25s - [%5ld] %s\r\n",
                  ++num,GET_NAME(i), world[i->in_room].number, world[i->in_room].name);
        if (num >= 30)
          break;
      }

    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname_with_abbrevs(arg, k->name)) {
        found = 1;
        snprintf(buf + strlen(buf), MAX_STRING_LENGTH, print_object_location(++num, k, ch));
        if (num >= 50)
          break;
      }

    if (*sql_do_where(arg, num)) {
      snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "%s", buf2);
      found = 1;
    }

    page_string(ch->desc, buf, 1);
    if (!found)
      send_to_char("Couldn't find any such thing.\r\n", ch);
  }
}
