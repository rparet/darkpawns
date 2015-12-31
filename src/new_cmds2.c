/* ==========================================================================
   FILE   : new_cmds2.c
   HISTORY:  dlkarnes
   ========================================================================= */

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

/* $Id: new_cmds2.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "olc.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern char *dirs[];
extern struct review_t review[25];
extern int mini_mud;
extern const int rev_dir[];
extern struct zone_data *zone_table;



/* extern functions */
struct char_data *HUNTING(struct char_data *ch);
ACMD(do_tell);
ACMD(do_flee);
void raw_kill(struct char_data * ch, int attacktype);
void set_hunting(struct char_data *ch, struct char_data *victim);
void send_to_zone(char *messg, struct char_data *ch);
void improve_skill(struct char_data *ch, int skill_num);
int real_zone(int number);
struct char_data *get_mount(struct char_data *rider);

/* this command used to be called do_forage -rparet 19981125 */
ACMD(do_scrounge)
{
   int sector_type  = world[ch->in_room].sector_type;
   struct obj_data *obj = NULL;
   int percent, prob, food = 0;
   bool find = TRUE;

   if (!GET_SKILL(ch, SKILL_SCROUNGE))
   {
     stc("You can't seem to find anything edible.\r\n", ch);
     WAIT_STATE(ch, PULSE_VIOLENCE*2);
     return;
   }

   if (IS_MOUNTED(ch))
   {
     stc("Dismount first!\r\n",ch);
     return;
   }

   percent = number(1, 101);
   prob = GET_SKILL(ch, SKILL_SCROUNGE);

      switch(sector_type)
    {
         case SECT_FOREST:
        find = FALSE; /* kill*/
            food = 28;
            break;
         case SECT_FIELD:
         case SECT_HILLS:
        find = FALSE; /* kill*/
            food = 29;
            break;
         case SECT_DESERT:
        find = FALSE; /* kill*/
            food = 30;
            break;
         case SECT_MOUNTAIN:
            food = 31;
            break;
         case SECT_WATER_SWIM:
         case SECT_WATER_NOSWIM:
         case SECT_UNDERWATER:
        find = FALSE; /* kill*/
            food = 27;
            break;
         default:
            stc("You need to be in the wilderness to scrounge!\r\n", ch);
            break;
    }
      if (food && percent < prob)
      {
    obj = read_object(real_object(food), REAL);
    if (obj)
    {
          obj_to_char(obj, ch);
      if (find)
           act("You find $p.", TRUE, ch, obj, 0, TO_CHAR);
      else
           act("You capture and kill $p.", TRUE, ch, obj, 0, TO_CHAR);
          improve_skill(ch, SKILL_SCROUNGE);
          WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
      }
      else if (food)
      {
        stc("You can't seem to find anything edible.\r\n", ch);
        WAIT_STATE(ch, PULSE_VIOLENCE*2);
      }

   act("$n searches for something to eat.", TRUE, ch, 0, 0, TO_ROOM);
}


ACMD(do_first_aid)
{
   struct char_data *vict = NULL;
   int percent, prob;

   one_argument(argument, arg);

   if (!GET_SKILL(ch, SKILL_FIRST_AID))
   {
     stc("You have no idea how!\r\n", ch);
     return;
   }

   if (!*arg)
    send_to_char("Aid who?\r\n", ch);
   else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
   else if (vict == ch)
    stc("You wish you could.\r\n", ch);
   else
   {
    if ((GET_HIT(vict) >= 1))
    {
      send_to_char("They don't really need first aid.\r\n", ch);
      return;
    }
    percent = number(1, 101+GET_LEVEL(vict));
    prob = GET_SKILL(ch, SKILL_FIRST_AID);

    if (percent < prob || GET_LEVEL(ch)>LVL_IMMORT)
      {
      GET_HIT(vict) = 1;
      update_pos(vict);
      act("You apply some makeshift bandages to $N's wounds.",
    FALSE, ch, 0, vict, TO_CHAR);
      act("$n applies some bandaging to $N's wounds.",
    FALSE, ch, 0, vict, TO_NOTVICT);
      act("$n applies some bandaging to your wounds.",
    TRUE, ch, 0, vict, TO_VICT);
      improve_skill(ch, SKILL_FIRST_AID);
      WAIT_STATE(vict, PULSE_VIOLENCE);
      }
    else
      {
      send_to_char("You fumble and ruin the bandages.\r\n",ch);
      act("$n fumbles with some bandaging and drops it all over the place!",
    TRUE, ch, 0, 0, TO_ROOM);
      }
    WAIT_STATE(ch, PULSE_VIOLENCE + 3);
   }
}

ACMD(do_disarm)
{
   struct char_data *vict = NULL;
   struct obj_data *obj = NULL;
   int percent, prob;

   one_argument(argument, arg);

   if (!GET_SKILL(ch, SKILL_DISARM))
   {
      stc("You'd better leave all the martial arts to fighters.\r\n", ch);
      if (!subcmd)
        return;
   }
   if (FIGHTING(ch))
      vict = FIGHTING(ch);
   else if (!(vict = get_char_room_vis(ch, arg)))
   {
    send_to_char("Disarm who?\r\n", ch);
    return;
   }
   else if (vict == ch)
   {
    stc("Just try removing your weapon instead.\r\n", ch);
    return;
   }
   if (!(obj = GET_EQ(vict, WEAR_WIELD)))
   {
     act("$E doesn't have anything to disarm.", FALSE, ch, 0, vict, TO_CHAR);
     return;
   }

   if (!FIGHTING(ch) || vict != FIGHTING(ch))
   {
     stc("You can't disarm them if you aren't fighting them!\r\n", ch);
     return;
   }

   percent = number(1, 101+GET_LEVEL(vict));
   prob = subcmd?200:GET_SKILL(ch, SKILL_DISARM);
   if (percent < prob)
   {
      act("You disarm $N and $p goes flying!", 0, ch, obj, vict, TO_CHAR);
      act("$n knocks $p from $N's hand!", TRUE, ch, obj, vict, TO_NOTVICT);
      act("$n deftly disarms you, knocking $p from your hand!", TRUE, ch, obj, vict, TO_VICT);
      obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
      if (!FIGHTING(vict))
        hit(vict, ch, TYPE_UNDEFINED);
      if (!subcmd)
        improve_skill(ch, SKILL_DISARM);
   }
   else
   {
      act("You try to disarm $N but fail, tumbling to the ground in the process!", FALSE, ch, 0, vict, TO_CHAR);
      act("$n tries to disarm you but fails and falls flat on $s face instead!", TRUE, ch, 0, vict, TO_VICT);
      act("$n tries to disarm $N, but fails and falls flat on $s face!", TRUE, ch, 0, vict, TO_NOTVICT);
      GET_POS(ch) = POS_SITTING;
      if (!FIGHTING(vict))
        hit(vict, ch, TYPE_UNDEFINED);
   }
   WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

ACMD(do_mindlink)
{
   struct char_data *vict = NULL;
   int percent, prob, x = 0;

   one_argument(argument, arg);

   if (!*arg)
    send_to_char("Link your mind to whose?\r\n", ch);
   else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
   else if (vict == ch)
    stc("You wish you could.\r\n", ch);
   else if (!GET_SKILL(ch, SKILL_MINDLINK))
    stc("Yeah, right.\r\n", ch);
   else
   {
    if (!IS_NPC(vict))
   {
      act("$N stares at you blankly.", FALSE, ch, 0, vict, TO_CHAR);
      act("$n stares at $N for a while and then falls flat on $s face.",
      FALSE, ch, 0, vict, TO_ROOM);
      stc("You fail.\r\n", ch);
      return;
    }
    if (FIGHTING(ch) || FIGHTING(vict))
    {
      stc("There's too much going on to establish a mind link.\r\n", ch);
      return;
    }
    if (GET_HIT(ch) < 100)
    {
      send_to_char("You don't have enough life to spare!\r\n", ch);
      return;
    }
    if (GET_MANA(vict) < 100)
    {
      send_to_char("They don't have enough energy to spare!\r\n", ch);
      return;
    }
    percent = number(1, 101);
    prob = GET_SKILL(ch, SKILL_MINDLINK);

    if ((IS_PSIONIC(vict) || IS_MYSTIC(vict)) && percent<prob)
    {
      x = number((20 + GET_LEVEL(ch)), 100);
      GET_HIT(ch) -= x;
      GET_MANA(vict) += x;
      send_to_char("A wealth of power overcomes you!\r\n", vict);
      act("$n and $N stare at each other for a while and drop to the "
      "ground in unison!", FALSE, ch, 0, vict, TO_ROOM);
      GET_POS(vict) = POS_STUNNED;
    }
    else
    {
      act("$n stares at $N for a while and then falls flat on $s face.",
    FALSE, ch, 0, vict, TO_ROOM);
      GET_HIT(ch) -= 100;
      improve_skill(ch, SKILL_MINDLINK);
    }
    send_to_char("You feel a little drained...", ch);
    GET_POS(ch) = POS_STUNNED;
   }
}


/*  Beholder spec_proc Features:
 *
 *  - stops all mages from casting any spells in the same room with it
 *  - randomly attempts to charm, sleep, or curse any player it can see
 *  - once attacked, casts spells at random attackers
 *
 */

SPECIAL(beholder)
{
  struct char_data *mobile = (struct char_data *) me;
  struct char_data *vict;
  int targets = 0, victim, found = FALSE;

  if (cmd) {
     if (CMD_IS("cast") || CMD_IS("recite")) {   /* Stop spell casters */
         act("A ray of light shoots out from one of $n's eyestalks, hitting"
        " $N!", TRUE, mobile, 0, ch, TO_NOTVICT);
     act("A ray of light shoots out from one of $n's eyestalks, breaking"
        " your concentration!", FALSE, mobile, 0, ch, TO_VICT);
         return TRUE;
     }
     return FALSE;
  }

  if (GET_POS(ch) != POS_FIGHTING) {
        /* If he ain't fighting, try to charm, sleep, or curse */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
       targets++;

    if (targets<=1)
       return FALSE;

    victim = number(0, targets); /* Find our actual victim */
    if(!victim)
      return FALSE;

    targets = 0;
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
       if(++targets == victim)
         break;

    if(vict == ch || IS_NPC(vict))
       return FALSE;

    switch (number(0, 2)) {
       case 0: cast_spell(ch, vict, NULL, SPELL_SLEEP); break;
       case 1: cast_spell(ch, vict, NULL, SPELL_CHARM); break;
       case 2: cast_spell(ch, vict, NULL, SPELL_CURSE); break;
       default:
        break;
      }
     return FALSE;
  }

  /* Code for casting on attacking chars */

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      {
    found = TRUE;
        break;
      }

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (!found || vict == NULL)
    vict = FIGHTING(ch);

  /* now that we've got the victim, blast him */

  if(number(0,10) == 0)
   cast_spell(ch, vict, NULL, SPELL_DISRUPT);
  else if (number(0,5) == 0)
   cast_spell(ch, vict, NULL, SPELL_DISINTEGRATE);
  else
   return FALSE;
  return TRUE;
}


/*
 *  Recharger mob special procedure   Part of Dark Pawns (www.mystech.com 4000)
 *
 *  Based on original recharger code by
 *        David A. Carver <DCARVER@cougar.colstate.cc.oh.us>
 *
 *  Rewritten for mobiles and "personality" by
 *        Frontline (rparet@rubens.artisan.calpoly.edu) on 6/6/97.
 *  The author maintains exclusive rights to this code.
 *  Use only with permission.
 *
 */


SPECIAL(recharger)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  int maxcharge = 0, mincharge = 0, price = 0;

  if (!(CMD_IS("list") || CMD_IS("help") || CMD_IS("recharge")))
    return FALSE;

  if (CMD_IS("list") || CMD_IS("help"))
  {
    act("$n sighs loudly.", FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'I recharge wands and staves, of course!'",
    FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'My price is 100 coins per spell level per charge..'",
    FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'So a wand casting heal at level 25 would cost 2500 "
    "coins per charge.'", FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'If it had 10 max charges at first, it would have 9 max "
    "after charging.",FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'That means that an item could have 1 charge maximum, with"
    " 2 charges remaining (after a recharge)'", FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'To recharge an item type: recharge <staff or wand>.'",
    FALSE, me, 0, 0, TO_ROOM);
    act("$n stares expectantly at $N.", 1, me, 0, ch, TO_NOTVICT);
    act("$n stares expectantly at you.", FALSE, me, 0, ch, TO_VICT);
    return TRUE;
  } else
    if (CMD_IS("recharge")) {
    argument = one_argument(argument, buf);

    if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
      act("$n tells you, 'You don't have that!'", FALSE, me, 0, ch, TO_VICT);
      act("$n whaps $N upside the head.", 1, me, 0, ch, TO_NOTVICT);
      act("$n whaps you upside the head.", FALSE, me, 0, ch, TO_VICT);
      return TRUE;
    }
    if (GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_WAND)
    {
      act("$n tells you, 'Ummm... does that look like a wand or staff to you?'",
        FALSE, me, 0, ch, TO_VICT);
      act("$n shakes his head in disgust.", FALSE, me, 0, 0, TO_ROOM);
      return TRUE;
    }
    price = MAX(100, GET_OBJ_VAL(obj, 0)*100);
    if (GET_GOLD(ch) < price) {
      act("$n tells you, 'You don't have enough gold!'",
    FALSE, me, 0, ch, TO_VICT);
      return TRUE;
    }
    maxcharge = GET_OBJ_VAL(obj, 1);
    mincharge = GET_OBJ_VAL(obj, 2);

    if (mincharge < maxcharge)
    {
     GET_OBJ_VAL(obj, 2)++;
     GET_OBJ_VAL(obj, 1)--;
     GET_GOLD(ch) -= price;
     act("$n waves his hands around and chants strange things.",
    FALSE, me, 0, 0, TO_ROOM);
     sprintf(buf, "The item now has %d charges remaining.\r\n",
        GET_OBJ_VAL(obj, 2));
     send_to_char(buf, ch);
     }
   else
     act("$n tells you, 'The item does not need recharging, stop wasting "
    "my time!'", FALSE, me, 0, ch, TO_VICT);
    return TRUE;
  }
  return FALSE;
}



/* Bare-bones detect skill      Part of Dark Pawns (www.mystech.com 4000)
 *
 * Allows players to detect "secret" doors
 *
 * Frontline 6/7/97
 *
 * Updated and modernized on 19990429 by rparet
 *
 */

ACMD(do_detect)
{
  int dir;

  if (!GET_SKILL(ch, SKILL_DETECT) && !(GET_RACE(ch) == RACE_ELF))
  {
   send_to_char("Yeah, right.\r\n", ch);
   return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
  {
   send_to_char("You're fucking blind, you can't find anything!!\r\n",ch);
   return;
  }

  send_to_char("You carefully check the room...\r\n", ch);

  if (GET_SKILL(ch, SKILL_DETECT) <= number(1, 101))
  {
   send_to_char("You can't seem to find anything.\r\n", ch);
   WAIT_STATE(ch, PULSE_VIOLENCE + 1);
   return;
  }

  for (dir = 0; dir < NUM_OF_DIRS; dir++)
  {
     if (EXIT(ch, dir) && EXIT(ch, dir)->keyword)
       if (strstr(EXIT(ch, dir)->keyword, "secret"))
       {
         if (dir == UP)
           sprintf(buf, "You notice something funny about the ceiling.\r\n");
         else if (dir == DOWN)
           sprintf(buf, "You notice something funny about the floor.\r\n");
         else
           sprintf(buf, "You notice something funny about the %s wall.\r\n", dirs[dir]);
         send_to_char(buf, ch);
     if (!ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK))
       SET_BIT_AR(ROOM_FLAGS(ch->in_room), ROOM_SECRET_MARK);
    }
  }

}


/*
 * Function: attacks a PC that tries to get an object from
 * the room it is in.
 *
 * By Frontline 6/7/97.
 * Modified 8/27/97 to handle !palm, too -Ser
 */
SPECIAL(no_get)
{
  struct char_data *mobile = (struct char_data *)me;

  if (!AWAKE(mobile) || GET_HIT(mobile)<=0)
    return(FALSE);

  if(CMD_IS("get") || CMD_IS("palm") || CMD_IS("take"))
  {
     argument = two_arguments(argument, buf1, buf2);

     if (*buf1 && *buf2)
        return FALSE;
     else
     {
       act("$n strikes at $N's hand!", TRUE, mobile, 0, ch, TO_NOTVICT);
       act("$n strikes at your hand!", FALSE, mobile, 0, ch, TO_VICT);
       hit(me, ch, TYPE_UNDEFINED);
       return TRUE;
     }
   }
   return FALSE;
}


/* This procedure used to be called create_and_start_hunting.
   I have modified it so that you may call create_mobile to create
   an instance of a mobile that is statistically correct for any level
   that you pass to this function.  This allows you to have spellcasters
   summon mobiles that have powers based upon the level of the caster.
   Important procedural note: if you call this function with the hunting
   integer set to TRUE, the mobile will be loaded in room 18201 and start
   hunting the player ch.  -rparet 19981107
*/

struct char_data
*create_mobile (struct char_data *ch, int mob_number, int level, int hunting)
{
  struct char_data *mob = read_mobile(mob_number, VIRTUAL);

  GET_LEVEL(mob) = level;
  GET_EXP(mob) = 0;
  GET_DAMROLL(mob) = (level > 10 ? ((level+1) / 1.50) :
                     ((level+1) / 2));
  GET_NDD(mob) = level > 10 ? level/1.50 : (level+1)/2;
  GET_SDD(mob) = 4;
  GET_HITROLL(mob) = level;
  GET_AC(mob) = 100 - (10*level);
  GET_MAX_HIT(mob) = 10*level+10;
  GET_MAX_HIT(mob) = level > 22 ? GET_MAX_HIT(mob) + (13*(level-22)) :
                     GET_MAX_HIT(mob);
  GET_MAX_HIT(mob) = level > 30 ? GET_MAX_HIT(mob) + (560*(level-30)):
                     GET_MAX_HIT(mob);
  GET_HIT(mob) = GET_MAX_HIT(mob);
  IS_CARRYING_N(mob) = 0;
  IS_CARRYING_W(mob) = 0;

  if (mob && !mini_mud && ch && hunting)
  {
    char_to_room(mob, real_room(18201)); /* start em at temple in Kir-Oshi */
    set_hunting(mob, ch);
  }

  if (mob && !hunting)
  {
    char_to_room(mob, ch->in_room);
  }

return mob;

}

SPECIAL(black_horn)
{
  struct obj_data *obj = (struct obj_data *)me, *mag_item = NULL;
  if (CMD_IS("use"))
  {
    skip_spaces(&argument);
    mag_item = GET_EQ(ch, WEAR_HOLD);

    if (mag_item && mag_item == obj && isname(argument, obj->name))
        {
          send_to_zone("A deep, foreboding tone resounds through the "
            "air.\r\n", ch);
          act("You inhale and blow deeply on $P.",TRUE, ch, 0, obj, TO_CHAR);
          stc("A deep, foreboding tone resounds through the air.\r\n", ch);
          act("$n blows on $P.", TRUE, ch, 0, obj, TO_ROOM);
          act("A deep, foreboding tone resounds through the air.",
        FALSE, ch, 0, 0, TO_ROOM);
      switch(number(0, 5))
      {
            case 0: create_mobile(ch, 14503, 25, TRUE); break;
            case 1: create_mobile(ch, 14504, 25, TRUE); break;
            case 2: create_mobile(ch, 14513, 25, TRUE); break;
            case 3: create_mobile(ch, 14514, 25, TRUE); break;
            case 4: create_mobile(ch, 14515, 25, TRUE); break;
            default: create_mobile(ch,14516, 25, TRUE); break;
      }
          return(TRUE);
        }
  }
  return(FALSE);
}



SPECIAL(zen_master)
{
 struct char_data *vict;

 if (cmd || GET_POS(ch) != POS_FIGHTING || !AWAKE(ch) || GET_HIT(ch)<=0)
    return FALSE;

 for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
       break;

 if (vict == NULL)
    vict = FIGHTING(ch);

 switch(number(0,20)) {
    case 0:
     act("$n touches $N on the arm.", 1, ch, 0, vict, TO_NOTVICT);
     act("$n touches you lightly on the arm.", 1, ch, 0, vict, TO_VICT);
     call_magic(ch, vict, 0, SPELL_WORD_OF_RECALL,GET_LEVEL(ch),CAST_SPELL);
     return TRUE;
    case 20:
     act("$n says, 'You have violence, but not thought.'",
     FALSE,ch,0,0,TO_ROOM);
     call_magic(ch, vict, 0, SPELL_TELEPORT, GET_LEVEL(ch), CAST_SPELL);
     return TRUE;
    default:
     break;
 }
 return FALSE;
}





ACMD(do_serpent_kick)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_SERPENT_KICK))
    {
      stc("You'd better leave all the martial arts to others.\r\n", ch);
      return;
    }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
        vict = FIGHTING(ch);
      else
        {
          send_to_char("Kick who?\r\n", ch);
          return;
        }
    }
  if (vict == ch)
    {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }
   if (IS_MOUNTED(ch))
   {
     stc("Dismount first!\r\n",ch);
     return;
   }

  percent = ((7 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
  /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_SERPENT_KICK);

  if (GET_POS(vict) <= POS_SLEEPING)
    prob=110;

  if (percent > prob)
    damage(ch, vict, 0, SKILL_SERPENT_KICK);
  else
    {
      damage(ch, vict, GET_LEVEL(ch)*1.5, SKILL_SERPENT_KICK);
      if(!number(0,80) && GET_LEVEL(ch) > 18)
        create_mobile(ch, 18221, GET_LEVEL(ch)+3, TRUE);
      improve_skill(ch, SKILL_SERPENT_KICK);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}



#define h_item    0
#define h_hunter  1
#define h_hunter2 2
#define h_hunter3 3
#define h_percent 4
#define h_parts   5
const int hunteds[][h_parts] = {
  {4209,   4207,     0,     0,  1},  /* opal/bhyroga warrior */
  {4210,   4207,     0,     0,  1},  /* jade/bhyroga warrior */
  {14223, 14203, 14205, 14207,  5}   /* spiked armor/Skarash infidel nonesuch */
};
#define NUM_HUNTEDS 3

/*
 * Cycle through the char list
 * If ch has h_item and %, find an available h_hunter and set_hunting
 */

void
hunt_items(void)
{
  struct descriptor_data *i;
  struct char_data *ch = NULL, *next_ch = NULL;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character)
    {
     int j = 0, k = 0;
     for (j = 0; j < NUM_WEARS; j++)
     {
       struct obj_data *obj = NULL, *hunting = NULL;
       bool found = FALSE;

       if ( (obj = GET_EQ(i->character, j)) )
           for (k = 0; k < NUM_HUNTEDS; k++)
             if (GET_OBJ_VNUM(obj)==hunteds[k][h_item])
               if(number(0,100)<hunteds[k][h_percent])
               {
                 for (ch = character_list; !found && ch; ch = next_ch)
                   {
                     next_ch = ch->next;
                     if ( (GET_MOB_VNUM(ch)!=hunteds[k][h_hunter] &&
                        GET_MOB_VNUM(ch)!=hunteds[k][h_hunter2]&&
                        GET_MOB_VNUM(ch)!=hunteds[k][h_hunter3]) ||
                        FIGHTING(ch) || !AWAKE(ch) ||
                      IS_AFFECTED(ch, AFF_CHARM) || HUNTING(ch))
                     continue;
                     else
                     {
                         found = TRUE;
                         hunting = obj;
                         break;
                     }
                   }
                 if (found && ch)
                 {
                   char msg[MAX_STRING_LENGTH];
                 sprintf(msg, "%s I must have %s!", GET_NAME(ch),
                       (hunting)->short_description);
                   do_tell(ch, msg, find_command("tell"), 0);
                 set_hunting(ch, i->character);
                 }
               }
     }
   }
}


/*
 * From: Corey Hoitsma (choitsma@netcom.com)
 */
ACMD(do_dig)
{
  /* Only works if you have Oasis OLC */
  extern void olc_add_to_save_list(int zone, byte type);

  char buf2[10], buf3[10], buf[80];
  int iroom = 0, rroom = 0, dir = 0;

  two_arguments(argument, buf2, buf3);
  /* buf2 is the direction, buf3 is the room */

  iroom = atoi(buf3);
  rroom = real_room(iroom);

  if (!*buf2 || !*buf3)
  {
    send_to_char("Format: dig <dir> <room number>\r\n", ch);
    return;
  }
  if (rroom <= 0)
  {
    sprintf(buf, "There is no room with the number %d.\r\n", iroom);
    send_to_char(buf, ch);
    return;
  }

  if ((GET_LEVEL(ch) < LVL_SET_BUILD) &&
      (zone_table[world[ch->in_room].zone].number != GET_OLC_ZONE(ch) ||
       real_zone(rroom) != real_zone(ch->in_room)) )
  {
    if (zone_table[world[ch->in_room].zone].number != GET_OLC_ZONE(ch))
    stc("You don't have permission to edit this zone.\r\n", ch);
    else
    stc("You don't have permission to edit that zone.\r\n", ch);
    return;
  }

  /* Main stuff */
  switch (*buf2)
  {
   case 'n': case 'N': dir = NORTH; break;
   case 'e': case 'E': dir = EAST; break;
   case 's': case 'S': dir = SOUTH; break;
   case 'w': case 'W': dir = WEST; break;
   case 'u': case 'U': dir = UP; break;
   case 'd': case 'D': dir = DOWN; break;
   default:
         stc("Valid dirs are n,s,e,w,u and d.\r\n", ch);
  }

  CREATE(world[rroom].dir_option[rev_dir[dir]], struct room_direction_data,1);
  world[rroom].dir_option[rev_dir[dir]]->general_description = NULL;
  world[rroom].dir_option[rev_dir[dir]]->keyword = NULL;
  world[rroom].dir_option[rev_dir[dir]]->to_room = ch->in_room;

  CREATE(world[ch->in_room].dir_option[dir], struct room_direction_data,1);
  world[ch->in_room].dir_option[dir]->general_description = NULL;
  world[ch->in_room].dir_option[dir]->keyword = NULL;
  world[ch->in_room].dir_option[dir]->to_room = rroom;

  /* Only works if you have Oasis OLC */
  olc_add_to_save_list((iroom/100), OLC_SAVE_ROOM);

  sprintf(buf, "You make an exit %s to room %d.\r\n", buf2, iroom);
  send_to_char(buf, ch);
}


void
flow_room(struct char_data *ch)
{
  char msg[256], direct[30];
  int dir = -1, was_in = ch->in_room;
  struct char_data *mount = NULL;

  if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_NORTH))
  {
    dir = NORTH;
    sprintf(direct, "the south");
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_SOUTH))
  {
    sprintf(direct, "the north");
    dir = SOUTH;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_EAST))
  {
    sprintf(direct, "the west");
    dir = EAST;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_WEST))
  {
    sprintf(direct, "the east");
    dir = WEST;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_UP))
  {
    sprintf(direct, "below");
    dir = UP;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_DOWN))
  {
    sprintf(direct, "above");
    dir = DOWN;
  }

  if(dir < 0 || !world[was_in].dir_option[dir] ||
     (GET_LEVEL(ch) >= LVL_IMMORT && PRF_FLAGGED(ch, PRF_NOHASSLE)) ||
     IS_NPC(ch) || world[was_in].dir_option[dir]->to_room==NOWHERE)
    return;



  /*
   *  Prolly won't have mounts on water.. but other rooms might
   *  "flow" later
   */
  if (IS_MOUNTED(ch) && !IS_NPC(ch))
    mount = get_mount(ch);
  if (mount)
    {
      sprintf(msg, "$N drifts %s on $n.", dirs[dir]);
      act(msg, TRUE, mount, 0, ch, TO_NOTVICT);
      char_from_room(mount);
      char_to_room(mount, world[was_in].dir_option[dir]->to_room);
      sprintf(msg, "$N drifts in from %s on $n.", direct);
      act(msg, TRUE, mount, 0, ch, TO_NOTVICT);
    }
  else
    {
      sprintf(msg, "$n drifts %s.", dirs[dir]);
      act(msg, TRUE, ch, 0, 0, TO_ROOM);
    }
  sprintf(msg, "You drift %s.\r\n", dirs[dir]);
  stc(msg, ch);
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);
  look_at_room(ch, 0);
  sprintf(msg, "$n drifts in from %s.", direct);
  act(msg, TRUE, ch, 0, 0, TO_NOTVICT);
}

ACMD(do_turn)
{
  int diff=0, need_mana=0;
  struct char_data *tch, *next_tch;

  if(!GET_SKILL(ch, SKILL_TURN)) {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }

  if (IS_EVIL(ch) || IS_NEUTRAL(ch))
    {
      stc("You are not holy enough to turn away the Undead!\r\n", ch);
      return;
    }

  send_to_char("You attempt to turn away the unholy presence in "
    "this room.\r\n", ch);
  act("$n's eyes are suddenly filled with blinding white light!",
     FALSE, ch, 0, 0, TO_ROOM);

  need_mana = GET_LEVEL(ch) * 3 + number(1,55);

  if(need_mana > GET_MANA(ch)) {
    stc("You fail to summon enough energy to dispel the darkness!\r\n",
        ch);
    GET_MANA(ch) = GET_MANA(ch) - (need_mana / 4);
    if (GET_MANA(ch) < 0)
      GET_MANA(ch) = 0;
    return;
  }

  GET_MANA(ch) = GET_MANA(ch) - need_mana;

  for (tch = world[ch->in_room].people; tch; tch = next_tch)
  {
    next_tch = tch->next_in_room;
    diff = 0;

    if(GET_RACE(tch) == RACE_UNDEAD || GET_RACE(tch) == RACE_VAMPIRE)
    {
     diff = GET_LEVEL(ch) - GET_LEVEL(tch);

     if (diff <= -5) {
      act("$n shivers uncomfortably.", FALSE, tch, 0, 0, TO_ROOM);
      act("A disturbing feeling washes over your body.", FALSE, tch,
         0, 0, TO_CHAR);
      continue;
     }

     if (diff >= 15) {  /* destroy! */
      act("$n grimaces and then explodes into a cloud of dust!",
       FALSE, tch, 0, 0, TO_ROOM);
      act("You feel your body twist horribly and disintegrate into nothing!", FALSE, tch, 0, 0, TO_CHAR);
      raw_kill(tch, SPELL_DISINTEGRATE);
      continue;
     }

     if (diff > 3) { /* flee em */
      act("$n shrieks in terror!", FALSE, tch, 0, 0, TO_ROOM);
      act("You are suddenly terrified!", FALSE, tch, 0, 0, TO_CHAR);
      do_flee(tch, "", 0, 0);
      continue;
     }

  }
}
}
