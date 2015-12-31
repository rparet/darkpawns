/*************************************************************************
*   File: Spec_procs2.c                                 Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996, 97, 98 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

/* $Id: spec_procs2.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include <stdio.h>

/*   external vars  */
extern int top_of_world;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int top_of_world;
extern struct dex_skill_type dex_app_skill[];
extern char *pc_class_types[];
extern char *tattoos[];
extern struct tat_data tat[];
extern int mini_mud;
extern struct house_control_rec house_control[MAX_HOUSES];

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
int num_followers(struct char_data *ch);
int isname_with_abbrevs(char *str, char *namelist);
int find_house(int vnum);  /* house.c */
ACMD(do_gen_comm);
ACMD(do_get);
ACMD(do_tell);
ACMD(do_say);
ACMD(do_stand);
ACMD(do_wake);
ACMD(do_flee);
ACMD(do_look);
ACMD(do_backstab);
ACMD(do_spike);
SPECIAL(shop_keeper);
SPECIAL(magic_user);
SPECIAL(breed_killer);
SPECIAL(fighter);
ACMD(do_steal);
int mag_savingthrow(struct char_data * ch, int type);
void raw_kill(struct char_data * ch, int attacktype);
void tattoo_af(struct char_data *ch, bool add);
struct char_data *get_mount(struct char_data *ch);
SPECIAL(elemental_room);
SPECIAL(field_object);
bool can_speak(struct char_data *ch);
struct char_data *HUNTING(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);
void send_to_zone(char *messg, struct char_data *ch);
void flow_room(struct char_data *ch);
void unmount(struct char_data *rider, struct char_data *mount);

/* internal funcs */
SPECIAL(bat);
SPECIAL(bat_room);

struct social_type {
  char *cmd;
  int next_line;
};


int
do_first_remort_adjust(struct char_data *ch)
{
/* first, find a stat to increase by +2 */
  enum abils { DEFAULT, con, str, wis, intl, dex, cha};

  if (GET_ORIG_CON(ch) < 17) { ch->real_abils.con += 2; GET_ORIG_CON(ch) += 2; return con; }
  if (ch->real_abils.str < 17) { ch->real_abils.str += 2; return str; }
  if (ch->real_abils.wis < 17) { ch->real_abils.wis += 2; return wis; }
  if (ch->real_abils.intel < 17) { ch->real_abils.intel += 2; return intl; }
  if (ch->real_abils.dex < 17) { ch->real_abils.dex += 2; return dex; }
  if (ch->real_abils.cha < 17) { ch->real_abils.cha += 2; return cha; }

  return DEFAULT;
}

void
do_second_remort_adjust(struct char_data *ch, int stat)
{
  enum abils { DEFAULT, con, str, wis, intl, dex, cha};

 /* exceptional strength */
 if (ch->real_abils.str == 18 && (stat != str) && ch->real_abils.str_add < 100)
 {
  if (ch->real_abils.str_add != 0)
    ch->real_abils.str_add += 20;
  else
    ch->real_abils.str_add = 20;
 }



 /* find a second stat to raise by +1 */
 if (GET_ORIG_CON(ch) < 18 && (stat != con))   { GET_ORIG_CON(ch)++; return; }
 if (ch->real_abils.str < 18 && (stat != str)) { ch->real_abils.str++; return; }
 if (ch->real_abils.wis < 18 && (stat != wis)) { ch->real_abils.wis++; return; }
 if (ch->real_abils.intel < 18 && (stat != intl)) { ch->real_abils.intel++; return; }
 if (ch->real_abils.dex < 18 && (stat != dex)) { ch->real_abils.dex++; return; }
 if (ch->real_abils.cha < 18 && (stat != cha)) { ch->real_abils.cha++; return; }

 /* if we get here, gotta assume all stats are 18 */
 /* so lets turn off hunger/thirst */

 if (GET_COND(ch, THIRST) != -1)
 {
   GET_COND(ch, THIRST) = -1;
   return;
 }

 if (GET_COND(ch, FULL) != -1)
 {
   GET_COND(ch, FULL) = -1;
   return;
 }

}

/* Normal Red and Black checkers */
SPECIAL(normal_checker)
{
  struct char_data *i;
  char buf[256];

  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch))
    return (FALSE);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    {
      if (!IS_NPC(i) && GET_LEVEL(i) < LVL_IMMORT)
	{
	  act("$n sees $N and jumps quite high!", TRUE, ch, NULL, i,TO_NOTVICT);
	  sprintf(buf, "%s sees you and jumps high, right at you!\r\n",
		  GET_NAME(ch));
	  buf[0] = UPPER(buf[0]);
	  send_to_char(buf, i);
	  hit(ch,i,TYPE_UNDEFINED);
	  return(TRUE);
	}
    }
  return(FALSE);
}


SPECIAL(ninelives)
{
  struct char_data *i;
  struct char_data *mobile = (struct char_data *) me;

  if ((!AWAKE(ch) && GET_HIT(ch)>0))
    return (FALSE);

  if (cmd)
    {
      skip_spaces(&argument);

      if (!*argument)
	return(FALSE);
      if (!strstr(argument, "chest"))
	return(FALSE);
      if (CMD_IS("open") || CMD_IS("look") || CMD_IS("examine"))
	{
	  if(!IS_NPC(ch) && CAN_SEE(mobile, ch)&&!PRF_FLAGGED(ch,PRF_NOHASSLE))
	    hit(mobile, ch, TYPE_UNDEFINED);
	  return(TRUE);
	}
      else
	return(FALSE);
    }

  if (!FIGHTING(ch) || GET_HIT(ch) > 0)
    return (FALSE);
  /* MAX_MOVE is used to set the number of lives */
  if ( GET_MAX_MOVE(ch) > 0 )
    {
      if (GET_MAX_MOVE(ch) >8)
	GET_MAX_MOVE(ch) = 8;
      else
	GET_MAX_MOVE(ch)--;

	GET_HIT(ch) = GET_MAX_HIT(ch);

      if (FIGHTING(ch))
	{
	  i = FIGHTING(ch);
	  act("$n rises from the dead and keeps fighting!\n\n",
	      TRUE, ch, NULL, i, TO_ROOM);
	  if (FIGHTING(i) == ch)
	    stop_fighting(i);
	  stop_fighting(ch);
	  hit(ch, i, TYPE_UNDEFINED);
	}
      return(TRUE);
    }
  return(FALSE);
}


SPECIAL(whirlpool)
{
  struct char_data *vict, *tmp;
  int spec_occured = FALSE, to_room = 0;
  struct char_data *mobile = (struct char_data *)me;

  if (mini_mud || !ch)
    return FALSE;

  for (vict = world[ch->in_room].people; vict; vict = tmp)
    {
      tmp = vict->next_in_room;
      if (!PRF_FLAGGED(vict,PRF_NOHASSLE) && vict !=mobile && !IS_NPC(vict))
	{
	  do
	    {
	      to_room = number(real_room(4600), real_room(4699));
	    }
	  while( (IS_SET_AR(world[to_room].room_flags,ROOM_PRIVATE)) ||
		 (IS_SET_AR(world[to_room].room_flags,ROOM_GODROOM)) ||
		 (IS_SET_AR(world[to_room].room_flags,ROOM_DEATH))   ||
		 (IS_SET_AR(world[to_room].room_flags,ROOM_NOMOB)));
	  char_from_room(vict);
	  char_to_room(vict, to_room);
	  send_to_char("A ravaging whirlpool sucks you under!\n\r", vict);
	  send_to_char("You finally surface, sputtering...\n\r\n\r", vict);
	  look_at_room(vict, 0);
	  spec_occured = TRUE;
	}
    }
  return(spec_occured);
}


#define MIMIC_ROOM_VNUM 5798
/********************************************************************
  MIMIC -the mob- must load in room MIMIC_ROOM_VNUM
  ********************************************************************/
SPECIAL (couch)
{
  struct char_data *mimic;
  struct obj_data *obj;

  if (!argument || mini_mud)
    return(FALSE);
  skip_spaces(&argument);
  if (!( (CMD_IS("look") || CMD_IS("examine")) && !strcmp(argument, "couch")) )
    return(FALSE);

  for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj) && (!strcmp(obj->name, "couch")))
      {
	obj_from_room(obj);
	extract_obj(obj);
	if ((mimic = world[real_room(MIMIC_ROOM_VNUM)].people))
	  {
	    char_from_room(mimic);
	    char_to_room(mimic, ch->in_room);
	    act("Starved and needing food to make more pillows, the mimic "
		"attacks!\n\r",TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("Starved and needing food to make more pillows, the "
		"mimic attacks you!\n\r\n\r",ch);
	    hit(mimic, ch, TYPE_UNDEFINED);
	    return(TRUE);
	  }
      }
  return(FALSE);

}

#define HORSE_VNUM 8021
SPECIAL(stableboy)
{
  struct char_data *mobile = (struct char_data *) me;
  struct char_data *horse = NULL;
  struct follow_type *k = NULL;
  char msg[256];
  int days, cost, renttime;

  skip_spaces(&argument);
  if (CMD_IS("buy"))
   if ( (argument && strcmp(argument, "horse")) || !argument)
   {
      sprintf (msg, "%s Buy what, fine adventurer?",
		   GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }

  if (CMD_IS("list")) {
    sprintf (msg, "%s You can buy a horse for 300 gold coins.",
             GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return (TRUE);
  } else if (CMD_IS("buy")) {
    if (num_followers(ch)>=GET_CHA(ch)/2) {
      stc("You can't have any more followers!\r\n", ch);
      return(TRUE);
    }
    if (GET_GOLD(ch) < 300) {
      sprintf (msg, "%s You can't afford a mount!", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }
    horse = read_mobile(HORSE_VNUM, VIRTUAL);
    if (horse) {
       GET_EXP(horse) = 0;
       SET_BIT_AR(AFF_FLAGS(horse), AFF_CHARM);
       char_to_room(horse, ch->in_room);
       act("$N brings $n up from the stables out back.", FALSE, horse, 0, mobile, TO_ROOM);
       add_follower(horse, ch);

       /* Be certain that pets can't get/carry/use/wield/wear items */
       IS_CARRYING_W(horse) = 1000;
       IS_CARRYING_N(horse) = 100;
       GET_MOVE(horse) = 230;
       GET_MAX_MOVE(horse) = 230;
       GET_GOLD(ch) -= 300;
       sprintf (msg, "%s That'll be 300 coins, treat'er well",
                GET_NAME(ch));
       do_tell(mobile, msg, find_command("tell"), 0);
       return(TRUE);
    } else {
       sprintf (msg, "%s Sorry we are all out of mounts at the moment, "
                "try again later.", GET_NAME(ch));
       do_tell(mobile, msg, find_command("tell"), 0);
       sprintf(msg, "Mount not loaded in stable.");
       mudlog(buf, BRF, LVL_GRGOD, TRUE);
       return(TRUE);
    }  /* if (horse) */
  } else if (CMD_IS("stable")) {
    if (IS_MOUNTED(ch)) {
      horse = get_mount(ch);
      unmount(ch, get_mount(ch));
      REMOVE_BIT_AR(AFF_FLAGS(horse), AFF_CHARM);
      stop_follower(horse);
    } else {
      if(!ch->followers) {
         sprintf(msg, "%s How do you expect to stable a mount, you don't "
                 "have a mount!", GET_NAME(ch));
         do_tell(mobile, msg, find_command("tell"), 0);
         return TRUE;
      }
      for (k = ch->followers; k; k = k->next) {
         if (IS_MOUNTABLE(k->follower)) {
            horse = k->follower;
            REMOVE_BIT_AR(AFF_FLAGS(horse), AFF_CHARM);
            stop_follower(k->follower);
            break;
         }
      }
      if (k == NULL) {
         sprintf(msg, "%s How do you expect to stable a mount, you don't "
                 "have a mount!", GET_NAME(ch));
         do_tell(mobile, msg, find_command("tell"), 0);
         return TRUE;
      }
    }  /* have a mount to rent now */
    GET_MOUNT_RENT_TIME(ch) = (long)time(0);
    GET_MOUNT_NUM(ch) = GET_MOB_VNUM(horse);
    GET_MOUNT_COST_DAY(ch) = 5;
    act("$N takes $n out back to the stables.", FALSE, horse, 0, mobile, TO_ROOM);
    extract_char(horse);
    sprintf(msg, "%s I will take good care of 'em, for %d coins a day.",
            GET_NAME(ch), GET_MOUNT_COST_DAY(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
  } else if (CMD_IS("collect")) {
    if (GET_MOUNT_NUM(ch) == 0) {   /* no mount to collect */
       sprintf(msg, "%s Hey now, you need to have stabled a mount to pick one "
               "up.", GET_NAME(ch));
       do_tell(mobile, msg, find_command("tell"), 0);
       return TRUE;
    }
    renttime = (long)time(0) - GET_MOUNT_RENT_TIME(ch);
    if (renttime != 0) {
       days = renttime / 60 / 60 / 24;
       if (days <= 0) days = 1;
    } else
       days = 1;
    cost = GET_MOUNT_COST_DAY(ch) * days;
    if (cost > GET_GOLD(ch)) {
       sprintf(msg, "%s Hey man, you can't afford the %d gold you need to get "
               "your mount outa' hock.", GET_NAME(ch), cost);
       do_tell(mobile, msg, find_command("tell"), 0);
       return TRUE;
    }
    horse = read_mobile(GET_MOUNT_NUM(ch), VIRTUAL);
    if (horse) {
       GET_MOUNT_NUM(ch) = 0;
       GET_MOUNT_COST_DAY(ch) = 0;
       GET_MOUNT_RENT_TIME(ch) = 0;
       GET_EXP(horse) = 0;
       SET_BIT_AR(AFF_FLAGS(horse), AFF_CHARM);
       char_to_room(horse, ch->in_room);
       act("$N brings $n up from the stables out back.", FALSE, horse, 0, mobile, TO_ROOM);
       add_follower(horse, ch);
       /* Be certain that pets can't get/carry/use/wield/wear items */
       IS_CARRYING_W(horse) = 1000;
       IS_CARRYING_N(horse) = 100;
       GET_MOVE(horse) = 230;
       GET_MAX_MOVE(horse) = 230;
       GET_GOLD(ch) -= cost;
       sprintf(msg, "%s Here ya go pal, all patted down and ready to go... "
               "cost ya %d to keep 'em here.", GET_NAME(ch), cost);
       do_tell(mobile, msg, find_command("tell"), 0);
       return TRUE;
    } else {
       sprintf(msg, "%s Sorry, we are unable to gather your mount, try back "
               "later.", GET_NAME(ch));
       do_tell(mobile, msg, find_command("tell"), 0);
       sprintf(buf, "Mount not loaded in stable");
       mudlog(buf, BRF, LVL_GRGOD, TRUE);
       return TRUE;
    }
  }
  return(FALSE);
}


SPECIAL(tipster)
{
  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch))
    return (FALSE);

  switch (number (0, 50))
    {
    case 0:
      do_say(ch, "For ansi color, type COLOR COMPLETE!", 0, 0); break;
    case 1:
      do_say(ch, "Wargs make a tasty meal if you CARVE their corpse.",
	     0, 0); break;
    case 2:
      do_say(ch, "AUTO EXIT will show you the exits for every room.",
	     0, 0); break;
    case 3:
      do_say(ch, "It's always safest to quit in the temple.", 0, 0);
      break;
    case 4:
      do_say(ch, "You're allowed to play up to 3 characters at once.",
	     0, 0); break;
    case 5:
      do_say(ch, "You can hire a mercenary by giving him 100 coins.",
	     0, 0); break;
    case 6:
      do_say(ch, "A bribe of a couple hundred coins will make a guard"
	     " look the other way while you fight.", 0, 0);
      do_say(ch, "Guards will attack you if you fight in front of "
	     "them.", 0, 0); break;
    case 7:
      do_say(ch, "Use the CONSIDER command!", 0, 0); break;
    case 8:
      do_say(ch, "If you don't like something, use the IDEA command."
	     , 0, 0); break;
    case 9:
      do_say(ch, "If you see something out of place, use the BUG "
	     "command.", 0, 0); break;
    case 10:
      do_say(ch, "If you see something spelled incorrectly, use "
	     "the TYPO command.", 0, 0); break;
    case 11:
      do_say(ch, "Use an identify scroll on yourself to see "
	     "your numerical statistics. (Available at your "
	     "local magick shop.)", 0, 0);
      break;
    case 12:
      do_say(ch, "Check out HELP ALIAS to see how to abbreviate "
	     "commands or do multiple commands at once.", 0, 0);
      break;
    default:
      break;
    }
  return FALSE;
}


SPECIAL(rescuer)
{
  ACMD(do_rescue);

  struct char_data *i = NULL;

  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch) && FIGHTING(FIGHTING(ch)) == ch)
    return (FALSE);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (IS_NPC(i) && FIGHTING(i) && GET_MOB_SPEC(i) != rescuer &&
	!IS_NPC(FIGHTING(i)))
      {
	do_rescue(ch, GET_NAME(i), 0, 1);
	return(TRUE);
      }
  return(FALSE);
}


SPECIAL(pissedalchemist)
{
  struct char_data *i = NULL;
  bool cont = FALSE;

  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch))
    return (FALSE);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (!IS_NPC(i) && GET_LEVEL(i) < LEVEL_IMMORT)
      cont = TRUE;

  if (!cont)
    return(FALSE);

  if (GET_ALIGNMENT(ch) != -69) /*arbitrary number, just a number that
				  he is NOT*/
    {
      GET_ALIGNMENT(ch) = -69;
      do_say(ch, "So, finally come to steal my secret!?!", 0, 0);
      do_say(ch, "The secret of the Philosopher's Stone!?!", 0, 0);
      return(FALSE);
    }
  do_say(ch, "Well -- Ya can't have it!! Die!!!", 0, 0);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (!IS_NPC(i) &&  GET_CLASS(i) == CLASS_MAGIC_USER)
      {
	hit(ch, i, TYPE_UNDEFINED);
	return(TRUE);
      }
  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (!IS_NPC(i))
      {
	hit(ch, i, TYPE_UNDEFINED);
	return(TRUE);
      }

  SET_BIT_AR(MOB_FLAGS(ch), MOB_AGGRESSIVE);

  return(FALSE);
}


void
kender_steal(struct char_data *ch, struct char_data *victim)
{
  struct obj_data *tmp_obj = NULL;

  if (!IS_NPC(victim))
    return;

  for (tmp_obj = victim->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 600) < GET_LEVEL(ch)))
	{
	  int percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;
	  int pcsteal = 0;

	  if (GET_POS(victim) < POS_SLEEPING)
	    percent = -1; /* Success */

	  if (!IS_NPC(victim))
	    pcsteal = 1;

	  /* NO NO With Imp's and Shopkeepers,
	     and if player thieving is not allowed */
	  if (GET_LEVEL(victim) >= LVL_IMMORT ||
	      (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) ||
	      GET_MOB_SPEC(victim) == shop_keeper)
	    percent = 101; /* Failure */

          /* newbie protection */
          if ((GET_LEVEL(ch) <= 10) || (GET_LEVEL(victim) <= 10))
            return;

	  if (GET_LEVEL(ch) > LVL_IMMORT && GET_LEVEL(victim) < GET_LEVEL(ch))
	    percent = -1;  /* Success */

	  if ( (!IS_NPC(victim)) && (percent < GET_SKILL(ch, SKILL_STEAL)) )
	    {
	      obj_from_char(tmp_obj);
	      obj_to_char(tmp_obj, ch);
              sprintf(buf, "%s kender stole %s from %s.\r\n", GET_NAME(ch),
               tmp_obj->name, GET_NAME(victim));
              mudlog(buf, BRF, LVL_IMMORT, TRUE);
	    }
	  else if (GET_LEVEL(ch) > 5)
	    {
	      char tmp_obj_name[80], tmp_mob_name[80];
	      one_argument((tmp_obj)->name, tmp_obj_name);
	      one_argument(victim->player.name, tmp_mob_name);
	      sprintf(buf, "%s %s", tmp_obj_name, tmp_mob_name);
	      do_steal(ch, buf, 0, 1);
	    }
	}
    }
}

/* for remorter spec */
#define NUM_REMORT_CLASSES 10
#define REMORT_COST 60000

int
find_remort_class(struct char_data *ch)
{
  int race = GET_RACE(ch);

  if (IS_WARRIOR(ch) || IS_PALADIN(ch) || IS_RANGER(ch))
  {
    if ((race == RACE_HUMAN) || (race == RACE_ELF)
        || (race == RACE_DWARF))
      return CLASS_PALADIN;
    else
      return CLASS_RANGER;
  }

  if (IS_CLERIC(ch))
     return CLASS_AVATAR;

  if (IS_THIEF(ch))
    return CLASS_ASSASSIN;

  if (IS_MAGIC_USER(ch))
    return CLASS_MAGUS;

  if (IS_PSIONIC(ch))
    return CLASS_MYSTIC;

  return GET_CLASS(ch);
}


SPECIAL(remorter)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[80];
  int i;

  if (IS_NPC(ch))
    return(FALSE);

  skip_spaces(&argument);

  if (CMD_IS("buy") || CMD_IS("list"))
   {
      sprintf (msg, "%s Type REMORT to remort!", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }
  if (!(CMD_IS("remort") || CMD_IS("buy")))
    return(FALSE);


  if (GET_LEVEL(ch) < LEVEL_IMMORT-1)
    {
      sprintf (msg, "%s You can't remort until level %d!\r\n",
		   GET_NAME(ch), LEVEL_IMMORT-1);
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }

  if (GET_LEVEL(ch) >= LEVEL_IMMORT)
    {
      sprintf(msg, "%s Immortals cannot remort!\r\n", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return TRUE;
    }

  if (GET_GOLD(ch) < REMORT_COST)
    {
	  sprintf (msg, "%s It costs %d gold to work my magicks.",
		   GET_NAME(ch), REMORT_COST);
	  do_tell(mobile, msg, find_command("tell"), 0);
	  return (TRUE);
    }
 for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      {
        send_to_char("You must come unto me naked, wearing only thy old "
			"body.\r\n", ch);
	return(TRUE);
      }

 if (PLR_FLAGGED(ch, PLR_IT))
   REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IT);
 if (PLR_FLAGGED(ch, PLR_VAMPIRE))
   {
     REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_VAMPIRE);
     if (IS_AFFECTED(ch, AFF_VAMPIRE))
       REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_VAMPIRE);
     if (GET_MANA(ch) > GET_MAX_MANA(ch))
        GET_MANA(ch) = GET_MAX_MANA(ch);
   }
 if (PLR_FLAGGED(ch, PLR_WEREWOLF))
   {
     REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_WEREWOLF);
     if (IS_AFFECTED(ch, AFF_WEREWOLF))
       REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_WEREWOLF);
   }
 if (ch->affected)
   while (ch->affected)
	affect_remove(ch, ch->affected);

      GET_GOLD(ch) -= REMORT_COST;
      GET_CLASS(ch) = find_remort_class(ch);
      GET_LEVEL(ch) = 1;
      GET_EXP(ch) = 1;
      GET_WIMP_LEV(ch) = 0;

      ch->points.max_hit = number(30,40);
      ch->points.max_mana = 100+(number(20,30));

      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);

      tattoo_af(ch, FALSE);  /* remove nasty tattoo affs */

      /* this pair of functions controls remort stat adjusts and
         anti-hunger, anti-thirst  */

      do_second_remort_adjust(ch, do_first_remort_adjust(ch));

      SET_BIT_AR(PLR_FLAGS(ch), PLR_REMORT);  /* new remort flag */

      for (i = 1; i <= MAX_SKILLS; i++)
	SET_SKILL(ch, i, 0);

      switch (GET_CLASS(ch))
	{
	case CLASS_MAGIC_USER:
	  SET_SKILL(ch, SPELL_MAGIC_MISSILE, 20);
	  SET_SKILL(ch, SPELL_ACID_BLAST, 20);
	  break;
	case CLASS_MAGUS:
	  SET_SKILL(ch, SPELL_MAGIC_MISSILE, 40);
	  SET_SKILL(ch, SPELL_ACID_BLAST, 40);
	  break;
	case CLASS_CLERIC:
	  SET_SKILL(ch, SPELL_CURE_LIGHT, 20);
	  SET_SKILL(ch, SPELL_ARMOR, 20);
	  break;
	case CLASS_AVATAR:
	  SET_SKILL(ch, SPELL_CURE_LIGHT, 40);
	  SET_SKILL(ch, SPELL_ARMOR, 40);
	  break;
	case CLASS_WARRIOR:
	  SET_SKILL(ch, SKILL_KICK, 20);
	  break;
	case CLASS_PALADIN:
        case CLASS_RANGER:
	  SET_SKILL(ch, SKILL_KICK, 40);
	  break;
	case CLASS_NINJA:
	  break;
	case CLASS_PSIONIC:
        case CLASS_MYSTIC:
	  SET_SKILL(ch, SPELL_MINDPOKE, 20);
	  break;
	case CLASS_THIEF:
	case CLASS_ASSASSIN:
	  SET_SKILL(ch, SKILL_SNEAK, 20);
	  SET_SKILL(ch, SKILL_HIDE, 10);
	  SET_SKILL(ch, SKILL_STEAL, 30);
	  SET_SKILL(ch, SKILL_BACKSTAB, 20);
	  SET_SKILL(ch, SKILL_PICK_LOCK, 20);
	  SET_SKILL(ch, SKILL_TRACK, 20);
	  break;
       default:
          break;
	}

      if (GET_RACE(ch) == RACE_KENDER)
	SET_SKILL(ch, SKILL_STEAL, 45);

      if (GET_RACE(ch) == RACE_MINOTAUR)
        SET_SKILL(ch, SKILL_HEADBUTT, 45);

      GET_PRACTICES(ch) = 10;

      tattoo_af(ch, TRUE);   /* add tattoo affects back in */
      affect_total(ch);      /* and total */

      mudlog("Due to remorting:", BRF, LVL_IMMORT, TRUE);
      advance_level(ch);
      sprintf (msg, "%s Enjoy your new life...",
	       GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      send_to_char("The remorter moves his hands over your eyes, closing them"
		   " with a touch.\r\nColors spiral in your sight... You open"
		   " your eyes, feeling refreshed.\r\n", ch);
      return(TRUE);
}

#define ASSASSIN_PRICE(as) (GET_LEVEL(as) * 1000)
SPECIAL(assassin)
{
  int as_loc;
  struct char_data *as, *vict;

  as_loc = ch->in_room + 1;

  if (CMD_IS("list"))
    {
      send_to_char("To hire an assassin: hire <assassin> <victim>\r\n", ch);
      send_to_char("Available assassins are:\r\n", ch);
      for (as = world[as_loc].people;as;as = as->next_in_room)
	{
	  sprintf(buf, "%8d - %s\r\n", ASSASSIN_PRICE(as), GET_NAME(as));
	  send_to_char(buf, ch);
	}
      return (TRUE);
    }
  else if (CMD_IS("hire"))
    {
      two_arguments(argument, buf, buf2);
      if(!*buf)
	{
	  send_to_char("Hire who?\r\n", ch);
	  return (TRUE);
	}
      if (!(as = get_char_room(buf, as_loc)))
	{
	  send_to_char("There is nobody called that!\r\n", ch);
	  return (TRUE);
	}
      if(!IS_NPC(as))
	{
	  send_to_char("GET THE HELL OUT OF THAT ROOM, NOW !!!\r\n", as);
	  sprintf(buf, "%s is in the assassin store room.", GET_NAME(as));
	  mudlog(buf, BRF, LVL_IMMORT, TRUE);
	  send_to_char("You can't hire players.\r\n", ch);
	  return (TRUE);
	}
      if (!*buf2)
	{
	  send_to_char("Whom do you want to assassinate?\r\n", ch);
	  return (TRUE);
	}
      if (GET_GOLD(ch) < ASSASSIN_PRICE(as))
	{
	  send_to_char("You don't have enough gold!\r\n", ch);
	  return 1;
	}
      if ((vict = get_player_vis(as, buf2, 0)))
	{
	  if (GET_LEVEL(vict) < 5)
	    {
	      send_to_char("We cannot lower ourselves to such easy prey.\r\n",
			   ch);
	      return(TRUE);
	    }
	  GET_GOLD(ch) -= ASSASSIN_PRICE(as);
	  as = read_mobile(GET_MOB_RNUM(as), REAL);
	  char_to_room(as, ch->in_room);
	  if(!MOB_FLAGGED(as,MOB_HUNTER))
	    SET_BIT_AR(MOB_FLAGS(as),MOB_HUNTER);
	  set_hunting(as, vict);
	  send_to_char("We cannot contact you if the job succeeds or "
		       "not...security, you know.\r\n", ch);
	  act("$n hires $N for a job.", FALSE, ch, 0, as, TO_ROOM);
	  sprintf(buf, "%s hires %s to kill %s.\r\n", GET_NAME(ch),
		  GET_NAME(as), GET_NAME(vict));
	  mudlog(buf, BRF, LVL_IMMORT, TRUE);
	  return (TRUE);
	}
      else
	{
	  send_to_char("Our underground doesn't know the whereabouts of "
		       "the victim!\r\n", ch);
	  return (TRUE);
	}
    }
  return (FALSE);
}

static void
give_tat(struct char_data *ch, struct char_data *mobile, int tat_num, int gold)
{
  GET_GOLD(ch)-=gold;
  GET_TATTOO(ch)=tat_num;
  act("$n starts to work on $N's tattoo...", TRUE,mobile, 0, ch, TO_NOTVICT);
  act("A ghastly scream is ripped from $N's lips just before "
      "$E blacks out.", TRUE, mobile, 0, ch, TO_NOTVICT);
  act("$n starts to work on your tattoo...", TRUE, mobile, 0, ch, TO_VICT);
  send_to_char("The pain is incredible; it seems to eat into your soul.\r\n"
               "A scream is ripped from your lips...\r\n", ch);
  do_gen_comm(ch, "Arrrrrrrrrgggggggghhhh!", 0, SCMD_SHOUT);
  send_to_char("You black out.\r\n", ch);
  GET_POS(ch)=POS_STUNNED;
    update_pos(ch);
  tattoo_af(ch, TRUE);
  affect_total(ch);
}

SPECIAL(tattoo1)
{
  struct char_data *mobile = (struct char_data *)me;

  int tats[] = { 1, 2, 8, TATTOO_FOX, TATTOO_OWL };
  int num_tats = 5;
  int i = 0;
  if (CMD_IS("list"))
    {
      send_to_char("To buy a tattoo: BUY <number of tattoo>.\r\n", ch);
      send_to_char("Available tattoos are:\r\n", ch);
      for (i = 0; i < num_tats; i++)
      {
         sprintf(buf, "[%d] - (%d) tattoo %s : %s\r\n", i,
                 tat[tats[i]].price, tattoos[tat[tats[i]].tattoo_num],
		 tat[tats[i]].descrip);
         send_to_char(buf, ch);
      }
      return (TRUE);
    }
  else if (CMD_IS("buy"))
  {
    int choice = -1;

    if (IS_NPC(ch))
      return FALSE;

    if (GET_TATTOO(ch))
    {
     char msg[264];
     sprintf (msg, "%s Your magickal center is already tattooed. "
	    "Get a new arm or get rid of that tattoo then come back.",
                  GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }

    skip_spaces(&argument);

    if (!*argument)
    {
     send_to_char("Buy what number?\r\n", ch);
     return(TRUE);
    }
    if (!isdigit(*argument) || ((choice=atoi(argument))>=num_tats))
    {
      send_to_char("Buy by number!\r\n", ch);
      return(TRUE);
    }

    if (GET_GOLD(ch)<(tat[tats[choice]].price))
    {
     char msg[160];
     sprintf (msg, "%s You look a little short on the price there, kid.",
                 GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }

    give_tat(ch, mobile, tats[choice], tat[tats[choice]].price);
    return(TRUE);
  }
  return(FALSE);
}

SPECIAL(tattoo2)
{
  struct char_data *mobile = (struct char_data *)me;

  int tats[] = {4, 9, 10, 13};
  int num_tats = 4;
  int i = 0;
  if (CMD_IS("list"))
    {
      send_to_char("To buy a tattoo: BUY <number of tattoo>.\r\n", ch);
      send_to_char("Available tattoos are:\r\n", ch);
      for (i = 0; i < num_tats; i++)
      {
         sprintf(buf, "[%d] - (%d) tattoo %s : %s\r\n", i,
                 tat[tats[i]].price, tattoos[tat[tats[i]].tattoo_num],
		 tat[tats[i]].descrip);
         send_to_char(buf, ch);
      }
      return (TRUE);
    }
  else if (CMD_IS("buy"))
  {
    int choice = -1;

    skip_spaces(&argument);

    if (IS_NPC(ch))
      return FALSE;

    if (!*argument)
    {
     send_to_char("Buy what number?\r\n", ch);
     return(TRUE);
    }
    if (!isdigit(*argument) || ((choice=atoi(argument))>=num_tats))
    {
      send_to_char("Buy by number!\r\n", ch);
      return(TRUE);
    }
    if (GET_TATTOO(ch))
    {
      char msg[254];
      sprintf (msg, "%s Your magickal center is already tattooed. "
		    "Your tattoo... is enough magick for such as yourself.",
                   GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return(TRUE);
     }

     if (GET_GOLD(ch)<(tat[tats[choice]].price))
     {
      char msg[160];
      sprintf (msg, "%s Without more coins, I can give no wisdom.",
                   GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return(TRUE);
     }

     give_tat(ch, mobile, tats[choice], tat[tats[choice]].price);
     return(TRUE);
  }
  return(FALSE);
}


SPECIAL(tattoo3)
{
  struct char_data *mobile = (struct char_data *)me;

  int tats[] = {6, 7, 11, 14};
  int num_tats = 4;
  int i = 0;
  if (CMD_IS("list"))
    {
      send_to_char("To buy a tattoo: BUY <number of tattoo>.\r\n", ch);
      send_to_char("Available tattoos are:\r\n", ch);
      for (i = 0; i < num_tats; i++)
      {
         sprintf(buf, "[%d] - (%d) tattoo %s : %s\r\n", i,
                 tat[tats[i]].price, tattoos[tat[tats[i]].tattoo_num],
		 tat[tats[i]].descrip);
         send_to_char(buf, ch);
      }
      return (TRUE);
    }
  else if (CMD_IS("buy"))
  {
    int choice =-1;

    skip_spaces(&argument);

    if (IS_NPC(ch))
      return FALSE;

    if (!*argument)
    {
     send_to_char("Buy what number?\r\n", ch);
     return(TRUE);
    }
    if (!isdigit(*argument) || ((choice=atoi(argument))>=num_tats))
    {
      send_to_char("Buy by number!\r\n", ch);
      return(TRUE);
    }
    if (GET_TATTOO(ch))
    {
     char msg[254];
     sprintf (msg, "%s Your mathickal thenter is awready tattooed. "
 	     "Your tattoo... ith enough mathick for such as yoursewf.",
             GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }

    if (GET_GOLD(ch)<(tat[tats[choice]].price))
    {
     char msg[160];
     sprintf (msg, "%s You don't have enough cash, hot stuff.",
              GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }

    give_tat(ch, mobile, tats[choice], tat[tats[choice]].price);
    return(TRUE);
  }
  return(FALSE);
}

SPECIAL(eviltrade)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *obj = NULL;
  char msg[160];
  if (CMD_IS("give"))
  {
    if (!*argument)
  	return(FALSE);

    two_arguments(argument, buf, buf2);
    if ( !*buf || !*buf2 || !isname(buf2, mobile->player.name) )
	return(FALSE);

    obj = get_obj_in_list_vis(ch, buf, ch->carrying);
    if (!obj)
	return(FALSE);
    if (GET_OBJ_VNUM(obj) != 13111) /* gold watch */
    {
     sprintf (msg, "%s Don't give me that crap.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    if (GET_ALIGNMENT(ch)>0)
    {
     sprintf (msg, "%s Get lost, goody-goody.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    obj_from_char(obj);
    obj_to_char(obj, mobile);

    sprintf (msg, "%s eneeswseswseswseswsesw", GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return(TRUE);
  }
  return(FALSE);
}

static int
val_cost(struct obj_data *obj)
{
  int cost = GET_OBJ_COST(obj);
  int price = 1;
  if (cost < 5000)
	price = cost/10;
  else
	price = cost*.14;

  if (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC))
	price += cost/20;
  return(MAX(1, price));
}

SPECIAL(identifier)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *obj = NULL;
  char msg[160];

  if (CMD_IS("list"))
  {
   sprintf (msg, "%s Just read the sign!", GET_NAME(ch));
   do_tell(mobile, msg, find_command("tell"), 0);
   return TRUE;
  }

  if (CMD_IS("value"))
  {
    skip_spaces(&argument);
    if (!*argument)
    {
     sprintf (msg, "%s Value what?", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    obj = get_obj_in_list_vis(ch, argument, ch->carrying);
    if (!obj)
    {
     sprintf (msg, "%s You don't seem to have that.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    sprintf (msg, "%s I'll identify that fully for about %d coins.",
             GET_NAME(ch), val_cost(obj));
    do_tell(mobile, msg, find_command("tell"), 0);
    return(TRUE);
  }
  if (CMD_IS("give"))
  {
    if (!*argument)
  	return(FALSE);

    two_arguments(argument, buf, buf2);
    if ( !*buf || !*buf2 || !isname_with_abbrevs(buf2, mobile->player.name) )
	return(FALSE);

    obj = get_obj_in_list_vis(ch, buf, ch->carrying);
    if (!obj)
	return(FALSE);

    if (GET_GOLD(ch)<val_cost(obj))
    {
      sprintf (msg, "%s That's a fine item, but I'll need %d coins from you"
	            " to id it.. and you're a little short..",
	       GET_NAME(ch), val_cost(obj));
      do_tell(mobile, msg, find_command("tell"), 0);
      sprintf (msg, "%s Keep it until you get the gold.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return(TRUE);
    }
    GET_GOLD(ch)-=val_cost(obj);

    act("You give $p to $N.", FALSE, ch, obj, mobile, TO_CHAR);
    act("$n gives you $p.", FALSE, ch, obj, mobile, TO_VICT);
    act("$n gives $p to $N.", TRUE, ch, obj, mobile, TO_NOTVICT);

    act("$n studies it carefully, comparing it to ancient texts,\r\n"
        "weighing it on scales, and chanting a number of odd spells over "
	"its surface.", TRUE, mobile, 0, 0, TO_ROOM);

    act("Finally looking up, you give $p back to $N.",
	FALSE, mobile, obj, ch, TO_CHAR);
    act("Finally looking up, $n gives you back $p.",
	FALSE, mobile, obj, ch, TO_VICT);
    act("Finally looking up, $n gives back $p to $N.",
	TRUE, mobile, obj, ch, TO_NOTVICT);

    act("$N touches your forehead, and knowledge fills your mind.",
	FALSE, ch, obj, mobile, TO_CHAR);
    act("You touch $n gently on the forehead.",
	FALSE, ch, obj, mobile, TO_VICT);
    act("$N touches $n gently on the forehead.",
	TRUE, ch, obj, mobile, TO_NOTVICT);

    send_to_char("\r\n", ch);
    spell_identify(LEVEL_IMMORT, ch, NULL, obj, NULL);

    return(TRUE);
  }
  return(FALSE);
}

SPECIAL(tattoo4)
{
  struct char_data *mobile = (struct char_data *)me;

  int tats[] = {5, 1, 3};
  int num_tats = 3;
  int i = 0;
  if (CMD_IS("list"))
    {
      send_to_char("To buy a tattoo: BUY <number of tattoo>.\r\n", ch);
      send_to_char("Available tattoos are:\r\n", ch);
      for (i = 0; i < num_tats; i++)
      {
         sprintf(buf, "[%d] - (%d) tattoo %s : %s\r\n", i,
                 tat[tats[i]].price, tattoos[tat[tats[i]].tattoo_num],
		 tat[tats[i]].descrip);
         send_to_char(buf, ch);
      }
      return (TRUE);
    }
  else if (CMD_IS("buy"))
  {
    int choice =-1;

    skip_spaces(&argument);

    if (!*argument)
    {
     send_to_char("Buy what number?\r\n", ch);
     return(TRUE);
    }
    if (!isdigit(*argument) || ((choice=atoi(argument))>=num_tats))
    {
      send_to_char("Buy by number!\r\n", ch);
      return(TRUE);
    }
    if (GET_TATTOO(ch))
    {
     char msg[254];
     sprintf (msg, "%s Get outta here, punk, you already have one. ",
             GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }

    if (GET_GOLD(ch)<(tat[tats[choice]].price))
    {
     char msg[160];
     sprintf (msg, "%s You don't have enough gold, get outta here!",
              GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }

    give_tat(ch, mobile, tats[choice], tat[tats[choice]].price);
    return(TRUE);
  }
  return(FALSE);
}

SPECIAL(evillead)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *obj = NULL;
  char msg[160];
  if (CMD_IS("give"))
  {
    if (!*argument || mini_mud)
        return(FALSE);
    two_arguments(argument, buf, buf2);
    if ( !*buf || !*buf2 || !isname(buf2, mobile->player.name))
        return(FALSE);
    if (!obj)
        return(FALSE);
    if (GET_OBJ_VNUM(obj) != 13100) /* slice of cheese */
    {
     sprintf (msg, "%s Don't give me that crap.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    if (GET_ALIGNMENT(ch)>0)
    {
     sprintf (msg, "%s Get lost, goody-goody.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    obj_from_char(obj);
    obj_to_char(obj, mobile);
    act("$n leads $N through a hidden trapdoor and returns a moment later,"
	" alone.\r\n", TRUE, mobile, 0, ch, TO_ROOM);
    send_to_char("  You follow Swiss though a small trapdoor into a maze of"
		 " twists and\r\nturns, winding through the various tunnels"
		 " as if he's been doing it all his\r\nlife. As you near a"
		 " narrow, well lit passage, Swiss turns around and "
		 "vanishes\r\nin a puff of smoke!\r\n", ch);
    char_from_room(ch);
    char_to_room(ch, real_room(13195));
    if (get_mount(ch))
      {
        char_from_room(get_mount(ch));
        char_to_room(get_mount(ch), real_room(13195));
      }
    look_at_room(ch, 0);
    return(TRUE);
  }
  return(FALSE);
}


SPECIAL(little_boy)
{
  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0 || FIGHTING(ch))
    return FALSE;

  if (!number(0,10))
    {
      act("$n exclaims, 'Don't let the bad man hurt me, mommy!", FALSE, ch,
          NULL, NULL, TO_ROOM);
      return TRUE;
    }
  return FALSE;
}

SPECIAL(ira)
{
  struct char_data *tmp_ch = NULL;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0 || FIGHTING(ch))
    return FALSE;

  for ( tmp_ch = world[ch->in_room].people; tmp_ch;
	tmp_ch = tmp_ch->next_in_room)
    {
      if ( !IS_NPC(tmp_ch) && !PRF_FLAGGED(tmp_ch, PRF_NOHASSLE))
	{
	  act("$n exclaims, 'Dirty Orange Scum!'", FALSE, ch, NULL, NULL,
	      TO_ROOM);
	  hit(ch, tmp_ch, TYPE_UNDEFINED);
	  return TRUE;
	}
    }

  return FALSE;
}

SPECIAL(take_to_jail)
{
  struct char_data *tch, *evil = NULL;
  int max_evil = 1000;

  if (cmd || !AWAKE(ch))
    return FALSE;

  if (FIGHTING(ch))
    return(fighter(ch, ch, 0, NULL));

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
    {
      if (!IS_NPC(tch) && CAN_SEE(ch, tch) &&
	  IS_SET_AR(PLR_FLAGS(tch), PLR_OUTLAW))
	{
	  act("$n says 'We don't like OUTLAWS like you in this city!'",
	      FALSE, ch, 0, 0, TO_ROOM);
	  hit(ch, tch, TYPE_UNDEFINED);
	  return (fighter(ch, ch, 0, NULL));
	}
      else if (breed_killer(ch, ch, 0, NULL))
	  return (TRUE);
      else if (CAN_SEE(ch, tch) && FIGHTING(tch))
	{
	  if ((GET_ALIGNMENT(tch) < max_evil) &&
	      (!IS_NPC(tch) || IS_NPC(FIGHTING(tch))))
	    {
	      max_evil = GET_ALIGNMENT(tch);
	      evil = tch;
	    }
	}
     }
     if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0))
       {
         act("$n says, 'You just pissed me off, $N!'"
             , FALSE, ch, 0, evil, TO_ROOM);
         hit(ch, evil, TYPE_UNDEFINED);
         return (fighter(ch, ch, 0, NULL));
       }
  return (FALSE);
}

SPECIAL(jail)
{
  if (cmd || mini_mud)
    return(FALSE);

  if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
    return FALSE;

  if (GET_JAIL_TIMER(ch) == 0 && GET_INVIS_LEV(ch)<LVL_IMMORT)
  {
      if(!AWAKE(ch))
         GET_POS(ch) = POS_SITTING;
      act("The guard says, 'Time's up, scum!'", TRUE, ch, 0, ch, TO_ROOM);
      act("The guard says, 'Time's up, scum!'", TRUE, ch, 0, ch, TO_VICT);
      act("$N gets thrown out of the cell!", TRUE, ch, 0, ch, TO_NOTVICT);
      act("The guard throws you out of the cell!\r\n",
		TRUE, ch, 0, ch, TO_VICT);;
      char_from_room(ch);
      char_to_room(ch, real_room(8117));
      look_at_room(ch, 0);
      return(TRUE);
  }
  return(FALSE);
}

void
eq_sound(struct char_data *ch)
{
  struct obj_data *obj = NULL;
  int i;
  char *to_char = NULL, *to_room = NULL;

  for (i = 0; i < NUM_WEARS && !to_room && !to_char; i++)
  {
    if ( !(number(0,52)) && (obj = GET_EQ(ch, i)) )
	switch (GET_OBJ_VNUM(obj))
	{
	case 2753:
		to_room = "A stream of smoke trails off of $p that $n wields.";
		to_char = "A stream of smoke trails off of $p.";
		break;
	case 4108:
		to_char = "You feel a vibration from $p.";
		break;
	case 4801:
	case 4802:
		to_room = "$n swings $p through the air, making a "
			"shrill whistling sound.";
		to_char = "You casually twirl $p around, making a "
			"shrill whistling sound.";
		break;
	default:
		break;
	}
  }
  if (to_char != NULL)
    act(to_char, TRUE, ch, obj, NULL, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, NULL, TO_ROOM);
}

void
loud_mobs(struct char_data * ch)
{
   int door, was_in;

   if (!GET_NOISE(ch) || ch->in_room == NOWHERE || (number(0,20)>0))
      return;

   was_in = ch->in_room;

   for (door = 0; door < NUM_OF_DIRS; door++)
   {
      if (CAN_GO(ch, door))
      {
         ch->in_room = world[was_in].dir_option[door]->to_room;
         act(GET_NOISE(ch), FALSE, ch, 0, 0, TO_ROOM);
         ch->in_room = was_in;
      }
   }
}

SPECIAL(medusa)
{
  struct char_data *mobile = (struct char_data *)me;
  if (!cmd && FIGHTING(mobile))
    return(magic_user(mobile, mobile, 0, NULL));
  if (!argument)
    return(FALSE);
  skip_spaces(&argument);
  if (!( (CMD_IS("look") || CMD_IS("examine")) &&
	 isname(argument, mobile->player.name) ))
    return(FALSE);

  if(!mag_savingthrow(ch, SAVING_PETRI))
   {
	char mybuf[MAX_STRING_LENGTH];
	act("With a sound like that of a crashing wave, $N slowly "
	    "turns to stone!", FALSE, ch, 0, ch, TO_NOTVICT);
	act("With growing horror and increasing agony, your body slowly turns"
	    " to stone!", FALSE, ch, 0, 0, TO_CHAR);
        sprintf(mybuf, "%s killed by Medusa special at %s",
		GET_NAME(ch), world[ch->in_room].name);
        mudlog(mybuf, BRF, LVL_IMMORT, TRUE);
        GET_DEATHS(ch)++;

	gain_exp(ch, -(GET_LEVEL(ch)*GET_LEVEL(ch)*GET_LEVEL(ch)));
	raw_kill(ch, SPELL_PETRIFY);
	return(TRUE);
   }
  return(FALSE);
}
void
npc_kender_steal(struct char_data *ch, struct char_data *victim)
{
  struct obj_data *tmp_obj = NULL;

  for (tmp_obj = victim->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 60) < GET_LEVEL(ch)))
	{
	  int percent = number(1, 101);

	  if (GET_POS(victim) < POS_SLEEPING)
	    percent = -1; /* Success */

	  if (GET_LEVEL(victim) >= LVL_IMMORT ||
	      (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)))
	    percent = 101; /* Failure */

	  if (GET_LEVEL(ch) > LVL_IMMORT && GET_LEVEL(victim) < GET_LEVEL(ch))
	    percent = -1;  /* Success */

	  if ( (!IS_NPC(victim)) && (percent < number(50,100)) )
	    {
	      obj_from_char(tmp_obj);
	      obj_to_char(tmp_obj, ch);
	      return;
	    }
	}
    }
}

SPECIAL(eq_thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4)))
      {
	ACMD(do_drop);
	struct obj_data *obj = NULL;
        npc_kender_steal(ch, cons);
	obj = ch->carrying;
	if (obj && (GET_OBJ_TYPE(obj) == ITEM_CONTAINER))
	{
	  char mybuf[256];
	  sprintf(mybuf, "all.black %s", OBJN(obj,ch));
	  do_get(ch, mybuf, 0, 0);
	}
	if ((obj = (get_obj_in_list_vis(ch, "black", ch->carrying))))
        {
          /* fake a junking */
          act("You junk $p. It vanishes in a puff of smoke!", FALSE, ch, obj, 0, TO_CHAR);
          act("$n junks $p. It vanishes in a puff of smoke!", TRUE, ch, obj, 0, TO_ROOM);
          extract_obj(obj);
        }
        return TRUE;
      }
  return FALSE;
}

SPECIAL(portal_room)
{
   int location;
   ACMD(do_look);
   ACMD(do_say);

   if(mini_mud || ( !CMD_IS("say") && !CMD_IS("'") ) )
        return(FALSE);

   location=real_room(21264);    /*destination room*/

   skip_spaces(&argument);
   if (strcasecmp(argument,"kallinistra"))
      return(FALSE);

   do_say(ch, argument, 0, 0);

   stc("\r\nWith a blinding flash of light and a crack of thunder, you are"
       " teleported...\r\n", ch);
   act ("\r\nWith a blinding flash of light and a crack of thunder, $n "
	"disappears!\r\n\r\n", FALSE, ch, NULL, NULL, TO_ROOM);

   char_from_room(ch);
   char_to_room(ch, location);
   act ("\r\nWith a blinding flash of light and a crack of thunder, $n "
	"appears!\r\n\r\n", FALSE, ch, NULL, NULL, TO_ROOM);
   do_look(ch,"",0,0);

   return(TRUE);
}

SPECIAL(breed_killer)
{
  struct char_data *mob = (struct char_data *)me;
  struct char_data *i = NULL;
  bool found = FALSE;

  if (cmd || !AWAKE(ch))
    return FALSE;

  if (FIGHTING(ch))
    return(fighter(ch, ch, 0, NULL));

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
     if (IS_NIGHTBREED(i))
	{
	 found = TRUE;
	 break;
	}
  if (found && CAN_SEE(mob, i))
  {
   if (can_speak(mob)  && !PRF_FLAGGED(i, PRF_NOHASSLE))
	act("$n exclaims, 'Die, nightbreed!!'", TRUE, ch, 0, 0, TO_ROOM);
   else if (!number(0, 5)) {
        act("You hear a low growl in the back of $n's throat.",
            FALSE, ch, 0, 0, TO_ROOM);
   }
   if (GET_EQ(mob, WEAR_WIELD))
   {
      if (strstr(OBJN(GET_EQ(mob, WEAR_WIELD), mob), "spike")  ||
	  strstr(OBJN(GET_EQ(mob, WEAR_WIELD), mob), "stake") )
	{
	  int scmd;
	  if (strstr(OBJN(GET_EQ(mob, WEAR_WIELD), mob), "spike"))
		scmd = SCMD_SPIKE;
	  else
		scmd = SCMD_STAKE;
	  do_spike(mob, GET_NAME(i), 0, scmd);
	}
   }
   if (i && i->in_room == mob->in_room && !PRF_FLAGGED(i, PRF_NOHASSLE)
        && CAN_SEE(mob, i))
	hit(mob, i, TYPE_UNDEFINED);
   return(TRUE);
  }
  return(FALSE);
}

SPECIAL(carrion)
{
  struct char_data *i;

  if ((!AWAKE(ch) && GET_HIT(ch)>0) || mini_mud)
    return (FALSE);

  if (cmd)
    {
      skip_spaces(&argument);

      if ( (!*argument) ||
	   (!strstr(argument, "corpse") && !strstr(argument, "corpses") &&
	    !strstr(argument, "pile")) )
	return(FALSE);

      if ((i = read_mobile(14308, VIRTUAL)))
      {
	GET_LEVEL(i) = GET_LEVEL(ch);
	GET_DAMROLL(i) = GET_LEVEL(ch);
	char_to_room(i, ch->in_room);
	act("Suddenly $n skitters from out of a corpse!",
		TRUE, i, 0, 0, TO_ROOM);
	hit(i, ch, TYPE_UNDEFINED);
        return(TRUE);
       }
    }
  return(FALSE);
}

SPECIAL(bat_room)
{
 struct char_data *i = NULL;
 for (i=world[ch->in_room].people; i; i=i->next_in_room)
   if (isname("bat", i->player.name) && !AWAKE(i))
     if (!number(0,6))
     {
       act("Your movements wake up $n!", TRUE, i, 0, 0, TO_ROOM);
       GET_POS(i) = POS_STANDING;
       update_pos(i);
     }
 return (FALSE);
}

SPECIAL(bat)
{
  struct char_data *mobile = (struct char_data *)me;
  if ( cmd || (!AWAKE(mobile) && GET_HIT(mobile)>0))
    return (FALSE);

  switch(number(0,6))
  {
   case 0:
   case 1:
	act("$n swoops down, narrowly missing your face.",
		TRUE, mobile, 0, 0, TO_ROOM);
	break;
   case 2:
   case 3:
	act("$n lets out a shrill, piercing scream.",
		TRUE, mobile, 0, 0, TO_ROOM);
	break;
   case 4:
   case 5:
	act("Circling around your head, $n lets out a high-pitched shriek.",
		TRUE, mobile, 0, 0, TO_ROOM);
	break;
   default:
   {
	struct char_data *i, *bat = NULL;
	bool waking = FALSE;

  	for (i=world[mobile->in_room].people; i; i=i->next_in_room)
		if (!IS_NPC(i))
			waking = TRUE;
	if (!waking)/* go to sleep */
	{
		GET_POS(mobile) = POS_SLEEPING;
		update_pos(mobile);
		return(TRUE);
	}
  	for (i=world[mobile->in_room].people; i; i=i->next_in_room)
	  if (isname("bat", i->player.name) && !AWAKE(i))
		bat = i;
	if (bat)
	{
		act("$n wakes $n up with it's flapping!",
			TRUE, mobile, 0, bat, TO_ROOM);
		GET_POS(bat) = POS_STANDING;
		update_pos(bat);
		return(TRUE);
	}
   }
  }
  return(FALSE);
}

SPECIAL(no_move_east)
{
    struct char_data *mobile = (struct char_data *)me;

    if (!cmd || mini_mud || !IS_MOVE(cmd) || !AWAKE(mobile))
       return FALSE ;

    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
       return FALSE;

    if (CMD_IS("east") && GET_LEVEL(ch) < LEVEL_IMMORT )
       {
           act("$n humiliates $N, and blocks $S way.",
		FALSE, mobile, 0, ch, TO_NOTVICT) ;
	   act("$n humiliates you and blocks your way.",
		FALSE, mobile, 0, ch, TO_VICT) ;
           return TRUE ;
       }
    return FALSE ;
}

/* is_owner assumes that room_vnum is the HOUSE vnum */
bool
is_owner(struct char_data *ch, int room_vnum)
{
   int i, j;

   if (IS_NPC(ch))
      return FALSE;

   if ((i = find_house(room_vnum)) < 0)
     return FALSE;

   /* let the house owner in */
   if (ch && (GET_IDNUM(ch) == house_control[i].owner))
	return TRUE;

   /* let any house guests in */
   for (j = 0; j < house_control[i].num_of_guests; j++)
   {
      if (ch && (GET_IDNUM(ch) == house_control[i].guests[j]))
        return TRUE;
   }

   return FALSE;
}

SPECIAL(key_seller)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *obj;
  int i;
  char msg[256];

  if (!ch || IS_NPC(ch) || !cmd)
    return FALSE;

  if (!CMD_IS("buy") && !CMD_IS("list"))
    return FALSE;

  if ((i = find_house(world[mobile->in_room].number)) < 0)
    return FALSE;

  if (GET_IDNUM(ch) != house_control[i].owner)
  {
    sprintf(msg, "%s Sorry, I only serve the house owner.", GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
  }

  skip_spaces(&argument);

  if (CMD_IS("buy"))
  {
   if ((argument && strcmp(argument, "key")) || !argument)
   {
      sprintf (msg, "%s I only sell keys, currently.",
                   GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }
   if (GET_GOLD(ch) < 10000)
   {
      sprintf (msg, "%s You can't afford a key.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }
   if (argument && is_abbrev(argument, "key"))
   {
     obj = read_object(house_control[i].key, VIRTUAL);
     if (obj)
     {
       obj_to_char(obj, ch);
       act("$n produces $p from the folds of $s long robe and hands it to $N.", TRUE, me, obj, ch, TO_NOTVICT);
       act("$n produces $p from the folds of $s long robe and hands it to you.", TRUE, me, obj, ch, TO_VICT);
       GET_GOLD(ch) -= 10000;
       return TRUE;
     }
   }
  }
  if (CMD_IS("list"))
  {
    sprintf (msg, "%s You can buy a key for 10,000 gold coins.",
             GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return (TRUE);
  }

  return FALSE;
}

SPECIAL(castle_guard_east)
{
    struct char_data *mobile = (struct char_data *)me;
    struct char_data *i = NULL;

    if (mini_mud || !AWAKE(mobile))
       return FALSE;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
       return FALSE;

    if (cmd && CMD_IS("east") && GET_LEVEL(ch) < LEVEL_IMMORT  && (mobile!=ch)&&
	!is_owner(ch, world[ch->in_room].number+2))
    {
      if( ch->master &&
	  (ch->master == ch || are_grouped(ch, ch->master)) &&
	  (is_owner(ch->master, world[ch->in_room].number+2)) )
	act("$n snaps to attention as you pass.", FALSE, mobile, 0, 0, TO_ROOM);
      else
      {
	act("$n yells, 'Stay outta there!'", FALSE, mobile, 0, 0, TO_ROOM);
	hit(mobile, ch, TYPE_UNDEFINED);
	return(TRUE);
      }
    }
    if (!cmd)
     for (i=world[mobile->in_room].people; i; i=i->next_in_room)
     {
	if (!FIGHTING(mobile) && FIGHTING(i) &&
	    GET_MOB_SPEC(i)==castle_guard_east && FIGHTING(i)!=mobile)
	{
	  hit(mobile, FIGHTING(i), TYPE_UNDEFINED);
	  return(TRUE);
	}
     }
    return(FALSE);
}

SPECIAL(mindflayer)
{
  struct char_data *vict = NULL;
  if (cmd || !FIGHTING(ch) || !AWAKE(ch))
	return(FALSE);
  vict = FIGHTING(ch);

  switch(number(0,15))
  {
	case 0:
        case 5:
		act("The tentacles on $n's face surge forward, wrapping "
		    "around $N's head!", FALSE, ch, 0, vict, TO_NOTVICT);
		act("The tentacles on $n's face surge forward, wrapping "
		    "around your head!", FALSE, ch, 0, vict, TO_VICT);
		damage(ch, vict, GET_LEVEL(vict), SPELL_SOUL_LEECH);
		GET_HIT(ch)+=GET_LEVEL(vict);
		break;
	case 15:
		act("Blood runs from $N's nose and ears as $n stares "
			"intently at $M.", FALSE, ch, 0, vict, TO_NOTVICT);
		act("$n stares intently at you.. you feel $m battering "
		 	"your mind!", FALSE, ch, 0, vict, TO_VICT);
		damage(ch, vict, GET_LEVEL(ch), SPELL_PSIBLAST);
		break;
	default:
		return(FALSE);
  }
  return(TRUE);
}

SPECIAL(backstabber)
{
     struct char_data *i = NULL;

     if (cmd || FIGHTING(ch) || !AWAKE(ch))
	return(FALSE);

     for (i=world[ch->in_room].people; i; i=i->next_in_room)
	if (!IS_NPC(i) && !PRF_FLAGGED(i, PRF_NOHASSLE) && CAN_SEE(ch, i))
	{
	 do_backstab(ch, GET_NAME(i), 0, 1);
	 return(TRUE);
	}
     return(FALSE);
}

SPECIAL(teleporter)
{
  if (cmd || !FIGHTING(ch) || !AWAKE(ch))
    return(FALSE);

  if (GET_HIT(ch)< GET_MAX_HIT(ch)/2)
  {
    act("$n says, 'My work here is done.'", TRUE, ch, 0, 0, TO_ROOM);
    call_magic(ch, ch, 0, SPELL_TELEPORT, GET_LEVEL(ch), CAST_SPELL);
    return(TRUE);
  }
  return(FALSE);
}

SPECIAL(no_move_west)
{
    struct char_data *mobile = (struct char_data *)me;

    if (!cmd || mini_mud || !IS_MOVE(cmd) || !AWAKE(mobile))
       return FALSE ;

    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
       return FALSE;

    if (CMD_IS("west") && GET_LEVEL(ch) < LEVEL_IMMORT )
       {
           act("$n humiliates $N, and blocks $S way.",
		FALSE, mobile, 0, ch, TO_NOTVICT) ;
	   act("$n humiliates you and blocks your way.",
		FALSE, mobile, 0, ch, TO_VICT) ;
           return TRUE ;
       }
    return FALSE ;
}

SPECIAL(no_move_north)
{
    struct char_data *mobile = (struct char_data *)me;

    if (!cmd || mini_mud || !IS_MOVE(cmd) || !AWAKE(mobile))
       return FALSE ;

    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
       return FALSE;

    if (CMD_IS("north") && GET_LEVEL(ch) < LEVEL_IMMORT )
       {
           act("$n blocks $N's way.", FALSE, mobile, 0, ch, TO_NOTVICT) ;
	   act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT) ;
	   act("$n says 'I cannot let you pass.'",
		FALSE, mobile, 0, 0, TO_ROOM) ;
           return TRUE ;
       }
    return FALSE ;
}

SPECIAL(never_die)
{
    struct char_data *mobile = (struct char_data *)me;

    if (cmd)
      return FALSE;

      if (GET_HIT(mobile)<GET_MAX_HIT(mobile))
      {
  	GET_HIT(mobile)=GET_MAX_HIT(mobile);
        update_pos(mobile);
        return TRUE;
      }

    return FALSE;
}

SPECIAL(no_move_south)
{
    struct char_data *mobile = (struct char_data *)me;

    if (!cmd || mini_mud || !IS_MOVE(cmd) || !AWAKE(mobile))
       return FALSE ;

    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
       return FALSE;

    if (CMD_IS("south") && GET_LEVEL(ch) < LEVEL_IMMORT )
       {
           act("$n blocks $N's way.", FALSE, mobile, 0, ch, TO_NOTVICT) ;
	   act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT) ;
	   act("$n says 'Thou shalt not pass.'",
		FALSE, mobile, 0, 0, TO_ROOM) ;
           return TRUE ;
       }
    return FALSE ;
}

SPECIAL(chosen_guard)
{
    struct char_data *mobile = (struct char_data *)me;

    if (!cmd || !IS_MOVE(cmd) || !AWAKE(mobile))
       return FALSE ;

    if (GET_LEVEL(ch) >= LVL_IMMORT || IS_CHOSEN(ch))
       return FALSE;

    if (CMD_IS("south"))
       {
           act("$n blocks $N's way.", FALSE, mobile, 0, ch, TO_NOTVICT) ;
	   act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT) ;
	   act("$n says 'Thou shalt not pass.'",
		FALSE, mobile, 0, 0, TO_ROOM) ;
           return TRUE ;
       }
    return FALSE ;
}

SPECIAL(castle_guard_down)
{
    struct char_data *mobile = (struct char_data *)me;
    struct char_data *i = NULL;

    if (mini_mud || !AWAKE(mobile))
       return FALSE;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
       return FALSE;

    if (cmd && CMD_IS("down") && GET_LEVEL(ch) < LEVEL_IMMORT  && (mobile!=ch)&&
        !is_owner(ch, world[ch->in_room].number+2))
    {
      if( ch->master &&
          (ch->master == ch || are_grouped(ch, ch->master)) &&
          (is_owner(ch->master, world[ch->in_room].number+2)) )
      {
        act("$n moves aside and allows you to pass.", FALSE, mobile, 0, ch, TO_VICT);
        act("$n moves aside and allows $N to pass.", FALSE, mobile, 0, ch, TO_NOTVICT);
      }
      else
      {
        act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT);
	act("$n blocks $N's path.", FALSE, mobile, 0, ch, TO_NOTVICT);
        act("$n states, 'Thou shalt not pass.'", FALSE, mobile, 0, 0, TO_ROOM);
        return(TRUE);
      }
    }
    if (!cmd)
     for (i=world[mobile->in_room].people; i; i=i->next_in_room)
     {
        if (!FIGHTING(mobile) && FIGHTING(i) &&
            GET_MOB_SPEC(i)==castle_guard_down && FIGHTING(i)!=mobile)
        {
          hit(mobile, FIGHTING(i), TYPE_UNDEFINED);
          return(TRUE);
        }
     }
    return(FALSE);
}

SPECIAL(castle_guard_up)
{
    struct char_data *mobile = (struct char_data *)me;
    struct char_data *i = NULL;

    if (mini_mud || !AWAKE(mobile))
       return FALSE;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
       return FALSE;

    if (cmd && CMD_IS("up") && GET_LEVEL(ch) < LEVEL_IMMORT  && (mobile!=ch)&&
        !is_owner(ch, world[ch->in_room].number+1))
    {
      if( ch->master &&
          (ch->master == ch || are_grouped(ch, ch->master)) &&
          (is_owner(ch->master, world[ch->in_room].number)) )
      {
        act("$n moves aside and allows you to pass.", FALSE, mobile, 0, ch, TO_VICT);
        act("$n moves aside and allows $N to pass.", FALSE, mobile, 0, ch, TO_NOTVICT);
      }
      else
      {
        act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT);
	act("$n blocks $N's path.", FALSE, mobile, 0, ch, TO_NOTVICT);
        act("$n states, 'Thou shalt not pass.'", FALSE, mobile, 0, 0, TO_ROOM);
        return(TRUE);
      }
    }
    if (!cmd)
     for (i=world[mobile->in_room].people; i; i=i->next_in_room)
     {
        if (!FIGHTING(mobile) && FIGHTING(i) &&
            GET_MOB_SPEC(i)==castle_guard_up && FIGHTING(i)!=mobile)
        {
          hit(mobile, FIGHTING(i), TYPE_UNDEFINED);
          return(TRUE);
        }
     }
    return(FALSE);
}

SPECIAL(castle_guard_north)
{
    struct char_data *mobile = (struct char_data *)me;
    struct char_data *i = NULL;

    if (mini_mud || !AWAKE(mobile))
       return FALSE;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
       return FALSE;

    if (cmd && CMD_IS("north") && GET_LEVEL(ch) < LEVEL_IMMORT  && (mobile!=ch)&&
        !is_owner(ch, world[ch->in_room].number+2))
    {
      if( ch->master &&
          (ch->master == ch || are_grouped(ch, ch->master)) &&
          (is_owner(ch->master, world[ch->in_room].number+2)) )
      {
        act("$n moves aside and allows you to pass.", FALSE, mobile, 0, ch, TO_VICT);
        act("$n moves aside and allows $N to pass.", FALSE, mobile, 0, ch, TO_NOTVICT);
      }
      else
      {
        act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT);
	act("$n blocks $N's path.", FALSE, mobile, 0, ch, TO_NOTVICT);
        act("$n states, 'Thou shalt not pass.'", FALSE, mobile, 0, 0, TO_ROOM);
        return(TRUE);
      }
    }
    if (!cmd)
     for (i=world[mobile->in_room].people; i; i=i->next_in_room)
     {
        if (!FIGHTING(mobile) && FIGHTING(i) &&
            GET_MOB_SPEC(i)==castle_guard_north && FIGHTING(i)!=mobile)
        {
          hit(mobile, FIGHTING(i), TYPE_UNDEFINED);
          return(TRUE);
        }
     }
    return(FALSE);
}

SPECIAL(wall_guard_ns)
{
  struct char_data *mobile = (struct char_data *)me;
  struct char_data *tch;
  struct char_data *tch_next;
  static int dir_to_move;
  static int talk = TRUE;

  if (cmd || (GET_POS(mobile) <= POS_SLEEPING) ||
      FIGHTING(mobile))
    return (fighter(ch, ch, 0, NULL));

  if (FIGHTING(mobile))
    return FALSE;

  if (CAN_GO(mobile, NORTH) && !CAN_GO(mobile, SOUTH))  /* at southern end */
    dir_to_move = NORTH;

  if (CAN_GO(mobile, SOUTH) && !CAN_GO(mobile, NORTH)) /* at northern end */
    dir_to_move = SOUTH;

  perform_move(mobile, dir_to_move, 1);   /* walk the wall */

  for (tch = world[mobile->in_room].people; tch; tch = tch_next)
    {
      tch_next = tch->next_in_room;
      if (tch == mobile)
        continue;

      if (IS_NPC(tch) && GET_MOB_VNUM(tch) == 8020 && talk)  /* church guard */
      {
        act("$n snaps to attention and salutes $N!", TRUE,  ch, 0, tch, TO_ROOM);
        act("$n says, 'Hello gents!'", TRUE, ch, 0, tch, TO_ROOM);
        act("$N nods at $n.", TRUE, ch, 0, tch, TO_ROOM);
	act("$N says, 'On your way, soldier!'", TRUE, ch, 0, tch, TO_ROOM);
        talk = FALSE;
      }
    }
  talk = TRUE;
  return FALSE;
}
