/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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

/* $Id: spells.c 1329 2007-06-29 04:53:09Z rparet $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"

/* External variables */
/* extern struct cha_app_type cha_app[]; */
extern struct int_app_type int_app[];
extern struct weather_data weather_info;
extern int mini_mud;
extern struct spell_info_type spell_info[];

/* External variables */
extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern struct npc_race_data npc_races[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;
extern int circle_check (struct char_data *ch);

/* External functions */
void act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);
bool damage(struct char_data * ch, struct char_data * victim,
	         int damage, int weapontype);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int num_followers(struct char_data *ch);
int mag_savingthrow(struct char_data * ch, int type);
struct char_data *HUNTING(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);
void send_to_zone(char *messg, struct char_data *ch);      
int is_shopkeeper(struct char_data * chChar);
extern struct char_data *create_mobile(struct char_data *ch, int
                                       mob_number,
                                       int level, int hunting);
void raw_kill(struct char_data * ch, struct char_data *killer, int attacktype);  


/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
  int water;

  void name_to_drinkcon(struct obj_data * obj, int type);
  void name_from_drinkcon(struct obj_data * obj);

  if (!ch || !obj)
    return;

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      if (water > 0) {
        GET_OBJ_VAL(obj, 2) = LIQ_WATER;
        GET_OBJ_VAL(obj, 1) += water;
        weight_change_object(obj, water);
        name_from_drinkcon(obj);
        name_to_drinkcon(obj, LIQ_WATER);
        act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      } else {
        stc("It's already full!\r\n", ch);
        return;
      }
    }
  } else {
    stc("You cannot create water in that!\r\n", ch);
    return;
  }
}


ASPELL(spell_recall)
{
  extern sh_int mortal_start_room[NUM_HOMETOWNS];
  void unmount(struct char_data *rider, struct char_data *mount);
  struct char_data *get_rider(struct char_data *mount);
  struct char_data *get_mount(struct char_data *rider);

  if (!victim || IS_NPC(victim))
    return;

  if ((ROOM_FLAGGED(ch->in_room, ROOM_BFR)) || 
      (ROOM_FLAGGED(victim->in_room, ROOM_BFR)) ) {
      send_to_char("Your magic ebbs and dissolves as you lose your concentration.\r\n", ch);
      return;
  }
  if (FIGHTING(ch)) {
    send_to_char("You can't do this while fighting!\r\n", ch);
    return;
  }

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  
  if (GET_BINDROOM(ch) != NOWHERE)
    char_to_room(victim, real_room(GET_BINDROOM(ch)));
  else
    char_to_room(victim, real_room(mortal_start_room[GET_HOME(victim)]));

  if (!IS_NPC(victim))
    unmount(victim, get_mount(victim));
  else
    unmount(get_rider(victim), victim);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);

  if (AWAKE(victim)) /* for group recall */
    look_at_room(victim, 0);
  else
    stc("You have a strange dream about falling...\r\n", victim);
}

ASPELL(spell_teleport)
{
  int to_room;
  extern long top_of_world;

  if (!IS_NPC(ch) && victim != ch && victim) {
    stc("You can only will this power upon yourself!\r\n", ch);
    return;
  }

  if (IS_MOUNTED(ch)) {
    stc("You should dismount first before attempting this!\r\n", ch);
    return;
  }

  if (IS_NPC(victim) && victim!=ch && mag_savingthrow(victim, SAVING_SPELL)) {
    send_to_char("The magic words fail to form properly.\r\n", ch);
    hit(victim, ch, TYPE_UNDEFINED);
    return;
  }

  if (GET_LEVEL(ch) < LEVEL_IMMORT && victim != ch) {
    if ((IS_NPC(victim) && ((GET_LEVEL(ch) - GET_LEVEL(victim)) < 3))) {
      send_to_char("The magic fails utterly!\r\n", ch);
      hit(victim, ch, TYPE_UNDEFINED);
      return;
    }
  } 

  do {
    if (IS_NPC(ch) && (ch == victim))
      to_room = real_room(number(zone_table[world[ch->in_room].zone].number * 100, zone_table[world[ch->in_room].zone].top));
    else
      to_room = number(1, top_of_world);
  } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE)
         || ROOM_FLAGGED(to_room, ROOM_GODROOM)
         || ROOM_FLAGGED(to_room, ROOM_DEATH)
         || SECT(to_room) == SECT_WATER_NOSWIM
         || to_room < 0
         || to_room > top_of_world);

     send_to_char("The world around you turns black and you suddenly find yourself..\r\n", victim);
     act("$n slowly fades out of existence and is gone.", FALSE, victim, 0, 0, TO_ROOM);
     char_from_room(victim);
     char_to_room(victim, to_room);
     act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
     look_at_room(victim, 0);
     return;
}

#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon)
{
  void unmount(struct char_data *rider, struct char_data *mount);    
  struct char_data *get_rider(struct char_data *mount);

  if (ch == NULL || victim == NULL)
    return;

  if (!circle_check(ch) && !IS_NPC(ch)) {
    send_to_char("You lack the proper glyphs to perform this magick.\r\n",
     ch);
    return;
  }

  if (GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 3))
    {
      send_to_char(SUMMON_FAIL, ch);
      sprintf(buf, "%s just tried to summon you to: %s.\r\n"
                   "%s failed.\r\n",
		  GET_NAME(ch), world[ch->in_room].name,
		  (ch->player.sex == SEX_MALE) ? "He" : "She");
      buf[0] = UPPER(buf[0]);
      stc(buf, victim);
      return;
    }

  if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE) && 
      (GET_LEVEL(ch) < LVL_IMMORT) && !IS_OUTLAW(victim))
  {
    stc(SUMMON_FAIL, ch);
    return;
  }

  if (( mag_savingthrow(victim, SAVING_SPELL) || 
        ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) ||
        ROOM_FLAGGED(victim->in_room, ROOM_PEACEFUL) ||
        ROOM_FLAGGED(victim->in_room, ROOM_NOMAGIC) ||
        MOB_FLAGGED(victim, MOB_NOCHARM) 
     ) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)
     )
    {

       if (!number(0, 20) && !IS_NPC(ch))  /* backfire for PCs only */
       {
	stc("Your spell backfires!\r\n", ch);
  	act("$n disappears suddenly.", TRUE, ch, 0, 0, TO_ROOM);

  	char_from_room(ch);
  	char_to_room(ch, victim->in_room);

  	if (IS_NPC(ch) && IS_MOUNTED(ch))
    	 unmount(get_rider(ch), ch);
  	else if (IS_MOUNTED(ch))
    	 unmount(ch, get_rider(ch));
		
  	act("$n arrives suddenly.", TRUE, ch, 0, 0, TO_ROOM);
	look_at_room(ch, 0);          
        if (IS_NPC(victim) && AWAKE(victim))
		hit(victim, ch, TYPE_UNDEFINED); 
	return;
      }

      send_to_char(SUMMON_FAIL, ch);
      sprintf(buf, "%s just tried to summon you to: %s.\r\n"
                 "%s failed.\r\n",
	         GET_NAME(ch), world[ch->in_room].name,
		 (ch->player.sex == SEX_MALE) ? "He" : "She");
      buf[0] = UPPER(buf[0]);
      stc(buf, victim);
      return;
    }

  /* legal summon from here on out */

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  if (IS_NPC(victim) && IS_MOUNTED(victim))
    unmount(get_rider(victim), victim);
  else if (IS_MOUNTED(victim))
    unmount(victim, get_rider(victim));

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  if (AWAKE(victim))
  {
    act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
    look_at_room(victim, 0);
    if (IS_NPC(victim))
	hit(victim, ch, TYPE_UNDEFINED); 
  }
  else
    send_to_char("You have a strange dream about falling...\r\n", victim);
}


ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  int j;

  strcpy(name, fname(obj->name));
  j = level >> 1;

  for (i = object_list; i && (j > 0); i = i->next)
    {
      if (!isname(name, i->name))
	continue;
     
      if (i->carried_by)
      {
	if (CAN_SEE(ch, i->carried_by))
	  sprintf(buf, "%s is being carried by %s.\n\r",
		  i->short_description, PERS(i->carried_by, ch));
      }
      else if (i->in_room != NOWHERE)
	sprintf(buf, "%s is in %s.\n\r", i->short_description,
		world[i->in_room].name);
      else if (i->in_obj)
	sprintf(buf, "%s is in %s.\n\r", i->short_description,
		i->in_obj->short_description);
      else if (i->worn_by)
      {
	if (CAN_SEE(ch, i->worn_by))
	  sprintf(buf, "%s is being worn by %s.\n\r",
	  	  i->short_description, PERS(i->worn_by, ch));
      }
      else
	sprintf(buf, "%s's location is uncertain.\n\r",
		i->short_description);

      CAP(buf);
      send_to_char(buf, ch);
      j--;
    }

  if (j == level >> 1)
    send_to_char("You sense nothing.\n\r", ch);
}

ASPELL(spell_charm)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_SANCTUARY))
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char("Your victim resists!\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
    send_to_char("You fail.\r\n", ch);
  else if (IS_NPC(victim) && is_shopkeeper(ch))
    send_to_char("Ha ha... Don't think so.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (mag_savingthrow(victim, SAVING_PARA)) {
    send_to_char("Your victim resists!\r\n", ch);
    if (IS_NPC(victim))
      hit(victim,ch, TYPE_UNDEFINED);
  } else if (num_followers(ch) >= GET_CHA(ch) / 2) {
    stc("You can't have any more followers!\r\n", ch);
    return;
  } else if (!IS_NPC(victim) && !IS_OUTLAW(ch))
    stc("Your power fails to effect them because you are not an Outlaw!\r\n", ch);
  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);
    af.type = SPELL_CHARM;
    af.duration = MAX(1, GET_INT(victim) / 5);
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}

#define SPELL_SCROLL  1280
ASPELL(spell_identify)
{
  int i, j, found;
  struct time_info_data age(struct char_data * ch);

  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];
  extern char *craft_level[];
  extern char *req_types[];

  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    if(GET_OBJ_TYPE(obj)==ITEM_STAFF)
      strcat(buf, "CASTING_EQ\n\r");
    else {
      sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
      strcat(buf, buf2);
      strcat(buf, "\r\n");
    } 
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "Craft is: %s\r\n", craft_level[GET_CRAFT_VAL(obj)]);
    send_to_char(buf, ch);

    sprintf(buf, "Encumbrance: %d, Value: %d\r\n", GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj));
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
      case ITEM_SCROLL:
      case ITEM_POTION:
        sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

        if (GET_OBJ_VAL(obj, 1) >= 1)
          sprintf(buf, "%s %s", buf, spell_info[GET_OBJ_VAL(obj, 1)].name);
        if (GET_OBJ_VAL(obj, 2) >= 1)
          sprintf(buf, "%s %s", buf, spell_info[GET_OBJ_VAL(obj, 2)].name);
        if (GET_OBJ_VAL(obj, 3) >= 1)
          sprintf(buf, "%s %s", buf, spell_info[GET_OBJ_VAL(obj, 3)].name);
        sprintf(buf, "%s\r\n", buf);
        send_to_char(buf, ch);
        break;
      case ITEM_WAND:
      case ITEM_STAFF:
        sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
        sprintf(buf, "%s %s\r\n", buf, spell_info[GET_OBJ_VAL(obj, 3)].name);
        sprintf(buf, "%sIt has %ld maximum charge%s and %ld remaining.\r\n",
                buf, GET_OBJ_VAL(obj, 1),
                GET_OBJ_VAL(obj, 1) == 1 ? "" : "s", GET_OBJ_VAL(obj, 2));
        send_to_char(buf, ch);
        break;
      case ITEM_WEAPON:
        sprintf(buf, "Damage Dice is '%ldD%ld'", GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
        sprintf(buf, "%s for an average per-strike damage of %.1f.\r\n", buf,
               (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
        send_to_char(buf, ch);
        break;
      case ITEM_SOUL_GEM:
        if (GET_OBJ_VAL(obj, 1) == -1)
          sprintf(buf, "This gem contains a soul of level %ld.\r\n", GET_OBJ_VAL(obj, 0));
        else
          sprintf(buf, "This gem currently does not contain a soul.\r\n");
        send_to_char(buf, ch);
        break;
      case ITEM_ARMOR:
        sprintf(buf, "AC-apply is %ld\r\n", GET_OBJ_VAL(obj, 0));
        send_to_char(buf, ch);
        break;
    }

    found = FALSE;
    send_to_char("Requirements: ", ch);
    for (i = 0, j = 0; i < MAX_OBJ_REQS; i++) {
      if (obj->reqs[i].modifier) {
        sprinttype(obj->reqs[i].location, req_types, buf2);
        sprintf(buf, "%s %d in %s", j++ ? "," : "", obj->reqs[i].modifier, buf2);
        send_to_char(buf, ch);
        found = TRUE;
      }
    }

    if (found)
      send_to_char("\r\n", ch);
    else
      send_to_char("None\r\n", ch);

    if (GET_OBJ_VNUM(obj) == SPELL_SCROLL) {
      sprintf(buf, "\r\nThis scroll contains information on the '&g%s&n' spell.\r\n", skill_name(GET_OBJ_VAL(obj, 0)));
      send_to_char(buf, ch);
    }

    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
        if (!found) {
          send_to_char("Can affect you as :\r\n", ch);
          found = TRUE;
        }
        if (obj->affected[i].location == APPLY_SPELL) {
          sprintf(buf, "   Permanent %s when equipped.\r\n", affected_bits[obj->affected[i].modifier]);
        } else {
          sprinttype(obj->affected[i].location, apply_types, buf2);
          sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
        }
        send_to_char(buf, ch);
      }
    }
  } else if (victim) {    /* victim */
    if (!IS_NPC(victim) && GET_LEVEL(victim) <= 5) {
       send_to_char("You cannot identify them yet.\r\n", ch);
       return;
    }

    if (IS_NPC(victim)) {
       stc("The magicks fail horribly!\r\n", ch);
       hit(victim, ch, TYPE_UNDEFINED);
       return;
    }

    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf,
        "%s is %d years, %d months, %d days and %d hours old.\r\n",
        GET_NAME(victim), age(victim).year, age(victim).month,
        age(victim).day, age(victim).hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Weight %d pounds.\r\n", GET_WEIGHT(victim));
    sprintf(buf, "%sLevel: %d, Hits: %d, Mana: %d\r\n", buf,
      GET_LEVEL(victim), GET_HIT(victim), GET_MANA(victim));
    sprintf(buf, "%sAC: %d, Hitroll: %d, Damroll: %d\r\n", buf,
      GET_AC(victim), GET_HITROLL(victim), GET_DAMROLL(victim));
    sprintf(buf,
      "%sStr: %d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
      buf, GET_STR(victim), GET_INT(victim),
      GET_WIS(victim), GET_DEX(victim), GET_CON(victim),
      GET_CHA(victim));
    send_to_char(buf, ch);
  }
}


ASPELL(spell_enchant_weapon)
{
  int i;

  if (!ch || !obj)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) && !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
        return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = MAX(1, level / 20);

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = MAX(1, level / 20);

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
  } else
    stc("Nothing seems to happen.\r\n", ch);
}


ASPELL(spell_lycanthropy)
{
  if (victim) {
    if (PLR_FLAGGED(victim, PLR_WEREWOLF)) {
      send_to_char("You're already a werewolf!\r\n", ch);
      return;
    }
    if (PLR_FLAGGED(victim, PLR_VAMPIRE)) {
      send_to_char("You're already a creature of the night!\n\r", ch);
      return;
    }
    send_to_char("You feel a strange sensation in your bones...\n\r", victim);
    SET_BIT(PLR_FLAGS(victim), PLR_WEREWOLF);
  } else {
    send_to_char("You need to specify a target.\n\r", ch);
    return;
  }
}

ASPELL (spell_sobriety)
{
   assert(victim);

   GET_COND(victim, DRUNK) = 0;

   update_pos(victim);

   send_to_char("You are splashed in the face with HOT coffee, ",victim);
   send_to_char("but feel much more sober.\n\r",victim);
}

ASPELL(spell_hellfire)
{
  int dam;
  struct char_data *tch;

  dam = dice(10, 10) + (GET_LEVEL(ch) / 20);

  send_to_char("The bowels of hell open beneath your feet!!\r\n", ch);
  send_to_zone("The shrieks of demons accompany the sounds of fire..\r\n", ch);
  act("$n conjures the flames of hell upon $s enemies!!", TRUE, ch, 0, 0, TO_ROOM);

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;

    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;

    if (are_grouped(ch, tch))
      continue;

    if (!IS_NPC(ch) && !IS_OUTLAW(ch))
      continue;

    if (!IS_NPC(ch) && !IS_OUTLAW(tch))
      continue;

    if (GET_LEVEL(tch) > 10) {
      act("The fires of hell bring blisters on your skin!!\n\r", FALSE, ch, 0, tch, TO_VICT);
      act("The fires of hell scorch the skin of $N!\r\n", FALSE, ch, 0, tch, TO_NOTVICT);
      if (number(1,30) > GET_DEX(ch)) {
        GET_POS(tch) = POS_SITTING;
        act("The fires of hell bring you to your knees!\r\n", FALSE, ch, 0, tch, TO_VICT);
        act("The fires of hell bring $N to $S knees!\r\n", FALSE, ch, 0, tch, TO_NOTVICT);
      }
      damage(ch, tch, dam, SPELL_HELLFIRE);
    } else {
      act("The fires of hell overcome you!!", FALSE, ch, 0, tch, TO_VICT);
      act("The soul of $N is charred by the flames of hell!", FALSE, ch, 0, tch, TO_NOTVICT);
      damage(ch, tch, GET_MAX_HIT(tch) * 10, SPELL_HELLFIRE);
    }
  }
}

ASPELL(spell_vampirism)
{
    if (victim)
      {
        if (IS_NPC(victim))
	  return;
	if (PLR_FLAGGED(victim, PLR_VAMPIRE))
	  {
	    send_to_char("Already a vampire.\n\r", ch);
	    return;
	  }
	if (PLR_FLAGGED(victim, PLR_WEREWOLF))
	  {
	    send_to_char("Already a creature of the night.\n\r", ch);
	    return;
	  }
	send_to_char("You feel a strange sensation in your blood!\n\r",
		     victim);
	SET_BIT(PLR_FLAGS(victim), PLR_VAMPIRE);
      }
    else
      {
	send_to_char("Specify a target.\n\r", ch);
	return;
      }
}

ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\r\n", ch);
      else
        send_to_char("You feel healthy.\r\n", ch);
    } else {
      if (IS_AFFECTED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
      case ITEM_DRINKCON:
      case ITEM_FOUNTAIN:
      case ITEM_FOOD:
        if (GET_OBJ_VAL(obj, 3))
          act("You sense that $p has been contaminated.", FALSE, ch, obj, 0, TO_CHAR);
        else
          act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0, TO_CHAR);
        break;
      default:
        send_to_char("You sense that it should not be consumed.\r\n", ch);
    }
  }
}

ASPELL(spell_enchant_armor)
{
  int i;

  if (!ch || !obj)
    return;

  if (((GET_OBJ_TYPE(obj) == ITEM_ARMOR) || (GET_OBJ_TYPE(obj) == ITEM_WORN)) &&
       !IS_SET(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
        return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_AC;
    obj->affected[0].modifier = MAX(1, level / 20);

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
  } else
    stc("Nothing seems to happen.\r\n", ch);
}

ASPELL(spell_zen)
{
  send_to_char("You begin to meditate deeply, focusing your thoughts only on "
	       "healing.\r\n", ch);
  act("$n sinks into a deep, meditative sleep.", FALSE, ch, 0, 0, TO_ROOM);
  
  GET_POS(ch) = POS_STUNNED;
  GET_HIT(ch) = MIN( GET_MAX_HIT(victim), GET_HIT(victim)+(2*GET_LEVEL(ch)) );
}


#define MISSILE 3
ASPELL(spell_silken_missile)
{
  struct obj_data *tobj;

  if (ch == NULL || obj == NULL)
    return;

  if ( !(GET_OBJ_TYPE(obj) == ITEM_WORN || GET_OBJ_TYPE(obj) == ITEM_ARMOR) )
    {
      act("You can't make anything useful from $p.\r\n",
	  FALSE, ch, obj, 0, TO_CHAR);
      return;
    }
  
  if (!(tobj = read_object(MISSILE, VIRTUAL, NULL)))
    {
      send_to_char("Error, please tell a god.\r\n", ch);
      log("SYSERR: spell_silken_missile: obj not found");
      return;
    }
  
  act("$n takes a strip of cloth from $p and creates an arrow.",
      FALSE, ch, obj, 0, TO_ROOM);
  act("You create an arrow from $p.", FALSE, ch, obj, 0, TO_CHAR);
  extract_obj(obj);
  obj_to_char(tobj, ch);
}

ASPELL(spell_mindsight)
{
  ACMD(do_look);

  if (victim) {
    struct char_data *i = NULL;
    int location, original_loc;
  
    if ((GET_LEVEL(victim) > GET_LEVEL(ch) + 4 && (!number(0,4))) ||
        (!IS_NPC(victim) && GET_LEVEL(victim) >= LEVEL_IMMORT &&
         GET_LEVEL(ch) <= GET_LEVEL(victim))) {
      send_to_char("With a searing pain, your mental energy recoils!\r\n", ch);
      return;
    }

    if (IS_AFFECTED(victim, AFF_NOTRACK)) {
      stc("Cannot find the target of your will!\r\n", ch);
      return;
    }

    if ((location = victim->in_room) < 0)
      return;

    /* a location has been found. */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    do_look(ch, "", 0, 0);
      
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if (i != ch && !IS_NPC(i))
        stc("You feel like you're being watched.\r\n", i);

      /* check if the char is still there */
      if (ch->in_room == location) {
        char_from_room(ch);
        char_to_room(ch, original_loc);
      }
    } else
      log("SYSERR: No victim in spell_mindsight()");
}


ASPELL(spell_mental_lapse)
{
  if (victim) {
    if (!IS_NPC(victim) || !HUNTING(victim)) {
      send_to_char("Your mental projection fails to make contact!\r\n", ch);
      return;
    }

    if (((HUNTING(victim) != ch) && (GET_LEVEL(ch) >= 60)) || (HUNTING(victim) == ch))
      send_to_char("You send out your mental voice...\r\n", ch);
    else {
      send_to_char("Your mental projection fails to make contact!\r\n", ch);
      return;
    }

    act("You sense $N's intentions even from where you are, and $E no longer hunts you.",
      TRUE, HUNTING(victim), 0, victim, TO_CHAR);
    set_hunting(victim, NULL);
    } else
      log("SYSERR: No victim in spell_mental_lapse()");
}

ASPELL(spell_calliope)
{
  if (victim)
  {
    int missiles = MAX(4,number(level/6, level*2));
    int i;
    for (i =0; i < missiles; i++)
	call_magic(ch, victim, 0, SPELL_MAGIC_MISSILE, level, CAST_SPELL);
  }
  else
    log("SYSERR: No victim in spell_calliope()");

}

ASPELL(spell_control_weather) {
  one_argument(argument,arg);

  if (!str_cmp( arg, "better" ) )
    weather_info.change += dice(level / 10, 4);
  else if ( !str_cmp( arg, "worse" ) )
    weather_info.change -= dice(level / 10, 4);
  else
    send_to_char ("Do you want it to get better or worse?\r\n", ch );
  return;
}

ASPELL(spell_coc) /* circle of summoning */
{
  obj = read_object(COC_VNUM, VIRTUAL, NULL);

  if (obj) {
    obj_to_room(obj, ch->in_room);
    GET_OBJ_TIMER(obj) = GET_LEVEL(ch) / 2 + number(-2, 1);
    send_to_char("You draw a magic circle on the ground.\r\n", ch);
    act("$n draws a magic circle on the ground.", 0, ch, 0, 0, TO_ROOM);
  }
 
}

#define EARTH_ELEMENTAL   81
#define WATER_ELEMENTAL   82
#define WIND_ELEMENTAL    83
#define FIRE_ELEMENTAL    84

const int elem_components[][2] = {
   { EARTH_ELEMENTAL,  1203 },
   { WATER_ELEMENTAL,  1204 },
   { WIND_ELEMENTAL,   1205 },
   { FIRE_ELEMENTAL,   1206 }
};

#define NUM_ELEMENTALS    4

ASPELL(spell_conjure_elemental)
{
  void add_follower_quiet(struct char_data *, struct char_data *);
  struct affected_type af;
  struct char_data *elemental;
  struct obj_data *comp;
  int k;

  af.type = SPELL_CONJURE_ELEMENTAL;
  af.duration = MAX(1, GET_LEVEL(ch) / 20);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_CHARM;

  for (k = 0; k < NUM_ELEMENTALS; k++) {
    for (comp = world[ch->in_room].contents; comp; comp = comp->next_content)   /* Let's check the room */
      if (GET_OBJ_VNUM(comp) == elem_components[k][1])
        break;

    if (!comp)
      for (comp = ch->carrying; comp; comp = comp->next_content)                /* Now the inventory */
        if (GET_OBJ_VNUM(comp) == elem_components[k][1])
          break;

    if (comp)
      break;
  }
  
  if (comp) {
    sprintf(buf, "You begin to chant slowly, drawing power from %s.\r\n", comp->short_description);
    stc(buf, ch);
    act("$n chants slowly, drawing power from $p.", TRUE, ch, comp, 0,TO_ROOM);
    elemental = create_mobile(ch, elem_components[k][0], (GET_LEVEL(ch) / 1.5), FALSE);
    affect_to_char(elemental, &af);
    add_follower_quiet(elemental, ch);
    act("$p glows brightly, and $N appears before you!", TRUE, ch, comp, elemental, TO_CHAR);
    act("$p glows brightly, and $N appears before you!", TRUE, ch, comp, elemental, TO_ROOM);
    extract_obj(comp);
  } else {
    stc("You begin to chant, but nothing seems to happen.\r\n", ch);
    act("$n begins to chant slowly, but nothing seems to happen.", TRUE, ch, 0, 0, TO_ROOM);
  }
}

ASPELL(spell_meteor_swarm)
{
  int dam;
  struct char_data *tch;

  dam = dice(10, 10) + (GET_LEVEL(ch) / 20);

  stc("Your incantation brings the heavens down upon your victims!\r\n", ch);
  act("As $n finishes $s incantation, bright lights from the heavens streak downward towards you.",
    TRUE, ch, 0, 0, TO_ROOM);

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;

    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;

    if (are_grouped(ch, tch))
      continue;

    if (!IS_NPC(tch) && !IS_OUTLAW(ch))
      continue;

    if (!IS_NPC(tch) && !IS_OUTLAW(tch))
      continue;
      
    damage(ch, tch, dam, SPELL_METEOR_SWARM); 
  }
}

#define MOB_CLONE	69           

ASPELL(spell_mirror_image)
{
  char szBuffer[256];
  struct char_data *mob = NULL;
  struct char_data *clone = create_mobile(ch, MOB_CLONE, 1, FALSE);

  if ((NULL == ch) || (NULL == clone))
    return;

  if (NULL == GET_NAME(ch)) return;
  if (NULL == ch->player.title) return;
  if (NULL == ch->player.description) return;

/*  GET_NAME(clone) = str_dup(GET_NAME(ch)); -- not required?? mcooper */
  clone->player.short_descr = str_dup(GET_NAME(ch)); 
  sprintf(szBuffer, "%s %s is standing here.\r\n", GET_NAME(ch), ch->player.title);
  clone->player.long_descr = str_dup(szBuffer);
  clone->player.description = str_dup(ch->player.description);
  clone->player.sex = ch->player.sex;

  stc("You divide yourself in two!\r\n", ch);
  act("$n divides $mself in two!", TRUE, ch, 0, 0, TO_ROOM);

  for (mob = character_list; mob; mob = mob->next)
   {
      if (MOB_FLAGGED(mob, MOB_MEMORY) && MEMORY(mob))
        if (forget(mob, ch))
          remember(mob, clone);
      if (HUNTING(mob) == ch)
        set_hunting(mob, clone);
   }      

}

#define GOOD_ANGEL 85;
#define EVIL_ANGEL 86;

ASPELL(spell_divine_int)
{
  struct char_data *angel;
  void add_follower_quiet(struct char_data *, struct char_data *);
  struct affected_type af;
  int mob_num = 85, number = 1;
  
  af.type = SPELL_DIVINE_INT;
  af.duration = MAX(1, GET_LEVEL(ch) / 20);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_CHARM;

  if (IS_NEUTRAL(ch)) {
    stc("Your request for intervention falls on deaf ears.\r\n", ch);
    return;
  }

  if (IS_GOOD(ch))
    mob_num = GOOD_ANGEL;

  if (IS_EVIL(ch))
    mob_num = EVIL_ANGEL;

  stc("You pray for the intervention of your deity.\r\n", ch);
  act("$n grovels in the dirt and prays for a miracle.", TRUE, ch, 0, 0, TO_ROOM);

  if (!mag_savingthrow(ch, SAVING_SPELL)) {
    stc("Nothing seems to happen.\r\n", ch);
    return;
  }

  if (GET_ALIGNMENT(ch) == -1000 || GET_ALIGNMENT(ch) == 1000)
    number += 1;

  stc("Suddenly, a portal of light appears out of nowhere!\r\n", ch);
  act("Suddenly, a portal of light appears out of nowhere!", FALSE, ch, 0, 0, TO_ROOM);
  
  for (; number != 0; number--) {
    angel = create_mobile(ch, mob_num,(GET_LEVEL(ch) / 1.5), FALSE);
    act("$n steps through the portal!", TRUE, angel, 0,0, TO_ROOM);
    affect_to_char(angel, &af);
    add_follower_quiet(angel, ch);
  }

}

ASPELL(spell_soul_trap)
{
  struct affected_type af;
  struct obj_data *gem;

  af.type = SPELL_SOULTRAP;
  af.duration = MAX(1, level / 25);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SOULTRAP;

  if (victim) {
    gem = GET_EQ(ch, WEAR_HOLD);
    if (!gem) {
      send_to_char("You need to be holding a soul gem for this spell to work.\r\n", ch);
      act("$n's magic fizzles out and dies.", TRUE, ch, 0, 0, TO_ROOM);
      return;
    } else if (GET_OBJ_TYPE(gem) != ITEM_SOUL_GEM) {
      act("$p isn't a soul gem!", TRUE, ch, gem, 0, TO_CHAR);
      return;
    } else {
      if (GET_OBJ_VAL(gem, 1) < 0) {
        act("$p already contains a soul!", TRUE, ch, gem, 0, TO_CHAR);
        return;
      } else {
        GET_OBJ_VAL(gem, 2) = 1;      /* Mark this as the gem */
      }
    }

    if (!IS_NPC(victim)) {
      act("$N gives you a puzzled look as you cast your spell.", TRUE, ch, 0, victim, TO_CHAR);
      act("You give $n a puzzled look as $e attempts to cast a spell on you!",
        TRUE, ch, 0, victim, TO_VICT);
      act("$N gives $n a puzzled look as $e attempts to cast a spell on $M!",
        TRUE, ch, 0, victim, TO_NOTVICT);
      return;
    }

    if (AFF_FLAGGED(victim, AFF_SOULTRAP)) {
      act("The spell has already been cast upon $n, just kill $s!",
        TRUE, victim, 0, ch, TO_VICT);
      return;
    }

    /* Mob's level is higher than that which the gem can accept */
    if (GET_LEVEL(victim) > GET_OBJ_VAL(gem, 0)) {
      stc("Your soul gem will not accept a soul of this level.\r\n", ch);
      return;
    }

    if (!mag_savingthrow(victim, SAVING_SPELL)) {
      stc("Nothing seems to happen.\r\n", ch);
      return;
    }

    affect_to_char(victim, &af);
    act("A faint mist forms and circles around $n.", TRUE, victim, 0, 0, TO_ROOM);
    GET_BANK_GOLD(victim) = GET_IDNUM(ch);
  } else
    log("SYSERR: No victim in spell_soul_trap()");
}

ASPELL(spell_syphon_energy)
{
  int hp_gain;

  if (obj) {
    if (!IS_CORPSE(obj))
      act("You cannot syphon energy energy from $p!", TRUE, ch, obj, 0, TO_CHAR);
    else {
      /* The amount of energy is determined by the "freshness" of the corpse */
      if (GET_OBJ_TIMER(obj)) {
        hp_gain = GET_OBJ_TIMER(obj) * 100;
        GET_OBJ_TIMER(obj) = 0;
        send_to_char("You feel energized.\r\n", ch);
        GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + hp_gain);
        act("Energy streams from $p and is absorbed by $n.", TRUE, ch, obj, 0, TO_ROOM);
      } else
        /* The corpse has already been sucked dry, or is about to decay */
        act("$p contains no more energy for you to syphon.", TRUE, ch, obj, 0, TO_CHAR);
    }
  } else
    log("SYSERR: No object in spell_syphon_energy()");
}

ASPELL(spell_group_pfe)
{
  struct char_data *k;
  struct follow_type *f;
  struct affected_type af;
  bool evil = IS_EVIL(ch);

  if (FIGHTING(ch)) {
    send_to_char("You can't do this while fighting!\r\n", ch);
    return;
  }

  if (!(k = ch->master))
    k = ch;

  for (f = k->followers; f; f = f->next) {
    if (f->follower->in_room != ch->in_room)
      continue;

    if (!IS_AFFECTED(f->follower, AFF_GROUP))
      continue;

    if (IS_NPC(f->follower))
      continue;

    if (IS_EVIL(f->follower))
      evil = TRUE;
  }

  af.type = SPELL_PROT_FROM_EVIL;
  af.duration = MAX(1, level / 20);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_PROTECT_EVIL;

  if (evil) {         /* We have an evil member in the group, kill the caster! */
    send_to_char("You cannot protect yourself from the Evil within the group!\r\n", ch);
    act("$n explodes before your eyes!", TRUE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "%s killed by Protection from Evil.", GET_NAME(ch));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    raw_kill(ch, NULL, TYPE_BLAST);
  } else {
    affect_to_char(k, &af);
    send_to_char("A stream of silver light surges from your fingertips, covering the group!\r\n", k);

    for (f = k->followers; f; f = f->next) {
      if (f->follower->in_room != ch->in_room)
        continue;

      if (!IS_AFFECTED(f->follower, AFF_GROUP))
         continue;

      if (IS_NPC(f->follower))
        continue;

      affect_to_char(f->follower, &af);
      act("A stream of silver light surges from $n's fingertips, covering you!",
        TRUE, ch, 0, f->follower, TO_VICT);
      act("A stream of silver light surges from $n's fingertips, covering $N!",
        TRUE, ch, 0, f->follower, TO_NOTVICT);
    }
  }
}

ASPELL(spell_group_pfg)
{
  struct char_data *k;
  struct follow_type *f;
  struct affected_type af;
  bool good = IS_GOOD(ch);

  if (FIGHTING(ch)) {
    send_to_char("You can't do this while fighting!\r\n", ch);
    return;
  }

  if (!(k = ch->master))
    k = ch;

  for (f = k->followers; f; f = f->next) {
    if (f->follower->in_room != ch->in_room)
      continue;

    if (!IS_AFFECTED(f->follower, AFF_GROUP))
      continue;

    if (IS_NPC(f->follower))
      continue;

    if (IS_GOOD(f->follower))
      good = TRUE;
  }

  af.type = SPELL_PROT_FROM_GOOD;
  af.duration = MAX(1, level / 20);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_PROTECT_GOOD;

  if (good) {         /* We have a good member in the group, kill the caster! */
    send_to_char("You cannot protect yourself from the Good within the group!\r\n", ch);
    act("$n explodes before your eyes!", TRUE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "%s killed by Protection from Good.", GET_NAME(ch));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    raw_kill(ch, NULL, TYPE_BLAST);
  } else {
    affect_to_char(k, &af);
    send_to_char("A stream of silver light surges from your fingertips, covering the group!\r\n", k);

    for (f = k->followers; f; f = f->next) {
      if (f->follower->in_room != ch->in_room)
        continue;

      if (!IS_AFFECTED(f->follower, AFF_GROUP))
         continue;

      if (IS_NPC(f->follower))
        continue;

      affect_to_char(f->follower, &af);
      act("A stream of silver light surges from $n's fingertips, covering you!",
        TRUE, ch, 0, f->follower, TO_VICT);
      act("A stream of silver light surges from $n's fingertips, covering $N!",
        TRUE, ch, 0, f->follower, TO_NOTVICT);
    }
  }
}

ASPELL(spell_entrapment)
{
  if (!victim)
    return;

  if (!FIGHTING(victim)) {
    act("$N observes your incantation and decides to strike first!", TRUE, ch, 0, victim, TO_CHAR);
    act("$N observes $n chanting at $M and decides to strike first!", TRUE, ch, 0, victim, TO_ROOM);
    act("You observe $n chanting at you and decides to strike first!", TRUE, ch, 0, victim, TO_VICT);
    hit(victim, ch, TYPE_UNDEFINED);
  }

  act("You extend your fingers and ensnare $N with a series of binding strands!", TRUE, ch, 0, victim, TO_CHAR);
  act("$n extends $s fingers and ensnares $N with a series of binding strands!", TRUE, ch, 0, victim, TO_ROOM);
  act("$n extends $s fingers and ensnares you with a series of binding strands!", TRUE, ch, 0, victim, TO_VICT);

  WAIT_STATE(victim, 3);
  COMBAT_WAIT(victim, (level / 20) + 1);
}
