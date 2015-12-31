/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
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


/* $Id: act.offensive.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "events.h"
#include "queue.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern char *dirs[];

/* extern functions */
void raw_kill(struct char_data * ch, int attacktype);
void improve_skill(struct char_data *ch, int skill_num);
extern int is_shopkeeper(struct char_data * chChar);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist "
         "someone else?\r\n", ch);
    return;
  }

  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else
    {
      for (opponent = world[ch->in_room].people;
       opponent && (FIGHTING(opponent) != helpee);
       opponent = opponent->next_in_room)
    ;

      if (!opponent)
    act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
      else if (!CAN_SEE(ch, opponent))
    act("You can't see who is fighting $M!",
        FALSE, ch, 0, helpee, TO_CHAR);
      else
    {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
    }
}


ACMD(do_hit)
{
  struct char_data *vict;
  ACMD(do_dismount);

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch)
    {
      send_to_char("You hit yourself...OUCH!.\r\n", ch);
      act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
    }
  else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.",
    FALSE, ch, 0, vict, TO_CHAR);
  else
    {
      if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch)))
    {
      if (IS_MOUNTED(ch))
            do_dismount(ch, NULL, 0, 0);
          hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
    }
}


ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_IMPL-1) || IS_NPC(ch))
    {
      do_hit(ch, argument, cmd, subcmd);
      return;
    }
  one_argument(argument, arg);

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
    raw_kill(vict, TYPE_SLASH);
      }
    }
}

ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_BACKSTAB))
    {
      send_to_char("You have no idea how.\r\n", ch);
      if (!subcmd)
        return;
    }

  if (!(vict = get_char_room_vis(ch, buf)))
    {
      send_to_char("Backstab who?\r\n", ch);
      return;
    }
  if (vict == ch)
    {
      send_to_char("How can you sneak up on yourself?\r\n", ch);
      return;
    }
  if (!GET_EQ(ch, WEAR_WIELD))
    {
      send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
      return;
    }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)
    {
      send_to_char("Only piercing weapons can be used for backstabbing.\r\n",
           ch);
      return;
    }
  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }
  if (FIGHTING(vict))
    {
      send_to_char("You can't backstab a fighting person -- they're too "
           "alert!\r\n", ch);
      return;
    }

  if ( IS_NPC(vict) && MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict))
    {
      act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
      act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
      act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
      hit(vict, ch, TYPE_UNDEFINED);
      return;
    }

  percent = number(1, 101); /* 101% is a complete failure */
  prob = subcmd?number(50,100):GET_SKILL(ch, SKILL_BACKSTAB);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else
    {
      hit(ch, vict, SKILL_BACKSTAB);
      improve_skill(ch, SKILL_BACKSTAB);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_disembowel)
{
 int percent, prob;
 struct char_data *vict;

 one_argument(argument, buf);

 if (!GET_SKILL(ch, SKILL_DISEMBOWEL))
   {
     send_to_char("You have no idea how.\r\n", ch);
     if (!subcmd)
       return;
   }


 if (!(vict = get_char_room_vis(ch, buf)))
    {
      if (FIGHTING(ch))
         vict = FIGHTING(ch);
      else {
         send_to_char("Disembowel who?\r\n", ch);
         return;
      }
    }
 if (vict == ch)
    {
      send_to_char("Nah. Hari Kari is for wimps.\r\n", ch);
      return;
    }
 if (!GET_EQ(ch, WEAR_WIELD))
    {
      send_to_char("You need to wield a weapon to make it a success.\r\n",ch);
      return;
    }
 if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)
    {
      send_to_char("Only piercing weapons can be used for "
       "disemboweling.\r\n", ch);
      return;
    }
  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */
  prob = subcmd?GET_SKILL(ch, SKILL_DISEMBOWEL): number(50,100);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_DISEMBOWEL);
  else
    {
      hit(ch, vict, SKILL_DISEMBOWEL);
      improve_skill(ch, SKILL_DISEMBOWEL);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}


ACMD(do_order)
{
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name))
       && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else
    {
    if (IS_AFFECTED(ch, AFF_CHARM))
      {
    send_to_char("Your superior would not aprove of you giving "
             "orders.\r\n", ch);
    return;
      }
    if (vict)
      {
    sprintf(buf, "$N orders you to '%s'", message);
    act(buf, FALSE, vict, 0, ch, TO_CHAR);
    act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

    if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
      act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
    else
      {
        send_to_char(OK, ch);
        command_interpreter(vict, message);
      }
      }
    else
      {         /* This is order "followers" */
    sprintf(buf, "$n issues the order '%s'.", message);
    act(buf, FALSE, ch, 0, vict, TO_ROOM);

    org_room = ch->in_room;

    for (k = ch->followers; k; k = k->next)
      {
        if (k->follower && (org_room == k->follower->in_room))
          if (k->follower && IS_AFFECTED(k->follower, AFF_CHARM))
        {
          found = TRUE;
          command_interpreter(k->follower, message);
        }
      }
    if (found)
      send_to_char(OK, ch);
    else
      send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
      }
    }
}


ACMD(do_flee)
{
  int i, attempt, loss=0;

  /* have to do it up here before FIGHTING(ch) gets destroyed */
  if (FIGHTING(ch))
  {
    loss = GET_MAX_HIT(FIGHTING(ch)) - GET_HIT(FIGHTING(ch));
    loss *= GET_LEVEL(FIGHTING(ch));
  }

  if (GET_POS(ch)<POS_FIGHTING)
  {
    stc("Get on your feet first!\r\n", ch);
    return;
  }

  if (IS_NPC(ch) && GET_MOB_WAIT(ch))
    {
      stc("You can't flee yet!\r\n", ch);
      return;
    }


  if (IS_THIEF(ch) || IS_ASSASSIN(ch))
    if (!IS_NPC(ch) && CHECK_WAIT(ch))
      {
        stc("You attempt to flee but cannot!\r\n", ch);
        return;
      }


  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);   /* Select a random direction */
    if (CAN_GO(ch, attempt) &&
    !IS_SET_AR(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH))
      {
    act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
    if (do_simple_move(ch, attempt, TRUE)) /* successful flee */
      {
        send_to_char("You flee head over heels.\r\n", ch);
            if (!IS_NPC(ch))
        {
          if (GET_LEVEL(ch)>10)
        loss += 500*(GET_LEVEL(ch)/2.6);
          gain_exp(ch, -loss);
            }
      }
    else
      {
        act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
      }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;
  int needed_moves = 10;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_BASH))
    {
    send_to_char("You'd better leave all the martial arts to fighters.\r\n",
         ch);
    if (!subcmd)
      return;
    }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n",
      ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg)))
    {
    if (FIGHTING(ch) && FIGHTING(ch)->in_room == ch->in_room)
      vict = FIGHTING(ch);
    else
      {
    send_to_char("Bash who?\r\n", ch);
    return;
      }
    }
  if (vict == ch)
    {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }
  if (GET_POS(vict)<POS_FIGHTING)
  {
    stc("You can't bash someone who's sitting already!\r\n", ch);
    return;
  }

  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  if (GET_MOVE(ch) < needed_moves)
    {
      stc("You haven't the energy!\r\n", ch);
      return;
    }
  else
    GET_MOVE(ch) = GET_MOVE(ch) - needed_moves;

  percent = ((5 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
  prob = subcmd?131:GET_SKILL(ch, SKILL_BASH);

  if (MOB_FLAGGED(vict, MOB_NOBASH) && GET_LEVEL(ch) < LEVEL_IMMORT)
    percent = 101;
  if ((GET_POS(vict) <= POS_SLEEPING) || (GET_LEVEL(ch) >= LEVEL_IMMORT))
    percent = 0;

  if (percent > prob) /* failed */
    {
      damage(ch, vict, 0, SKILL_BASH);
      GET_POS(ch) = POS_SITTING;
      WAIT_STATE(ch, PULSE_VIOLENCE); /* one extra pulse for failing */
    }
  else if (damage(ch, vict, (GET_LEVEL(ch)/2)+1, SKILL_BASH))
    {  /* only knock people over you can damage successfully */
      if (!subcmd)
        improve_skill(ch, SKILL_BASH);
      GET_POS(vict) = POS_SITTING;
      WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    }
 if (!IS_NPC(ch))
   WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_RESCUE))
    {
      stc("But only true warriors can do this!\r\n", ch);
      if (!subcmd)
        return;
    }

  if (!(vict = get_char_room_vis(ch, arg)))
    {
      send_to_char("Whom do you want to rescue?\r\n", ch);
      return;
    }
  if (vict == ch)
    {
      send_to_char("What about fleeing instead?\r\n", ch);
      return;
    }
  if (FIGHTING(ch) == vict)
    {
      send_to_char("How can you rescue someone you are trying to kill?\r\n",
           ch);
      return;
    }
  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && !IS_OUTLAW(ch))
  {
    send_to_char("This room just has such a peaceful,"
     " easy feeling...\r\n", ch);
    return;
  }

  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch)
    {
      act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
      return;
    }

  percent = number(1, 101); /* 101% is a complete failure */
  prob = subcmd?100:GET_SKILL(ch, SKILL_RESCUE);

  if (percent > prob)
    {
      stc("You fail the rescue!\r\n", ch);
      return;
    }
  else
    {
      send_to_char("Banzai!  To the rescue...\r\n", ch);
      act("You are rescued by $N, you are confused!",
      FALSE, vict, 0, ch, TO_CHAR);
      act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      improve_skill(ch, SKILL_RESCUE);

      if (FIGHTING(vict) == tmp_ch)
    stop_fighting(vict);
      if (FIGHTING(tmp_ch))
    stop_fighting(tmp_ch);
      if (FIGHTING(ch))
    stop_fighting(ch);

      set_fighting(ch, tmp_ch);
      set_fighting(tmp_ch, ch);
      if (subcmd)
    hit(vict, tmp_ch, TYPE_UNDEFINED);

      WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
    }

}


ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_KICK))
    {
      send_to_char("You'd better leave all the martial arts to fighters.\r\n",
           ch);
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
    stc("Dismount first!\r\n", ch);
    return;
  }


  percent = ((7 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
  /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_KICK);

  if (percent > prob)
    damage(ch, vict, 0, SKILL_KICK);
  else
  {
    damage(ch, vict, GET_LEVEL(ch) >> 1, SKILL_KICK);
    improve_skill(ch, SKILL_KICK);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE + 2);
}

ACMD(do_dragon_kick)
{
  struct char_data *vict;
  int percent, prob;
  int needed_moves = 10;

  if (!GET_SKILL(ch, SKILL_DRAGON_KICK))
    {
      stc("What's that, idiot-san?\r\n", ch);
      return;
    }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
    vict = FIGHTING(ch);
      else
    {
      stc("Kick who?\r\n", ch);
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
    stc("Dismount first!\r\n", ch);
    return;
  }

  if (GET_MOVE(ch) < needed_moves)
    {
      stc("You're too exhausted!\r\n", ch);
      return;
    }
  else
    GET_MOVE(ch) = GET_MOVE(ch) - needed_moves;

  percent = ((5 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
  /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_DRAGON_KICK);

  if (percent > prob)
    damage(ch, vict, 0, SKILL_DRAGON_KICK);
  else
  {
    damage(ch, vict, GET_LEVEL(ch)*1.5, SKILL_DRAGON_KICK);
    improve_skill(ch, SKILL_DRAGON_KICK);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE +2);

}

ACMD(do_tiger_punch)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_TIGER_PUNCH))
    {
      stc("What's that, idiot-san?\r\n", ch);
      return;
    }
  if (GET_EQ(ch, WEAR_WIELD))
    {
      stc("That's pretty tough to do while wielding a weapon.\r\n", ch);
      return;
    }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
    vict = FIGHTING(ch);
      else
    {
      stc("Hit who?\r\n", ch);
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
      stc("Dismount first!\r\n", ch);
      return;
    }
  percent = ((7 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
  /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_TIGER_PUNCH);

  if (percent > prob)
    damage(ch, vict, 0, SKILL_TIGER_PUNCH);
  else
  {
    damage(ch, vict, GET_LEVEL(ch) * 2.5, SKILL_TIGER_PUNCH);
    improve_skill(ch, SKILL_TIGER_PUNCH);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_shoot)
{
  struct char_data *to = NULL;
  struct obj_data *projectile = NULL, *bow = NULL;
  extern struct dex_app_type dex_app[];
  int dir = 0, dam = 0, targ_room = 0, percent, prob;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char *from = NULL;

  void update_pos(struct char_data *victim);
  void die(struct char_data *ch);

  if (!GET_SKILL(ch, SKILL_SHOOT)) {
     send_to_char("You have no idea how.\r\n", ch);
     return;
  }

  /* is shooter already fighting? */
  if (FIGHTING(ch))
    {
      send_to_char("But you are already engaged in close-range combat!\r\n",ch);
      return;
    }

  half_chop(argument, arg1, buf);
  half_chop(buf, arg2, buf);
  strcpy(arg3, buf);

  if (!*arg1)
    {
      send_to_char("Shoot what where?\r\n", ch);
      return;
    }
  else if (!*arg2)
    {
      send_to_char("Where would you like to shoot it?\r\n", ch);
      return;
    }
  else if (!*arg3)
    {
      send_to_char("Who would you like to shoot it at in that direction?\r\n",
           ch);
      return;
    }
  else
    {
      if (!(projectile = get_obj_in_list_vis(ch, arg1, ch->carrying)))
    {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
      send_to_char(buf, ch);
      return;
    }

      /* have what they want to shoot? */
      if (!projectile)
    {
      sprintf(buf, "You do not seem to have %s %s.\r\n", AN(arg2), arg2);
      send_to_char(buf, ch);
      return;
    }
      /* is it shootable? */
      else if (GET_OBJ_TYPE(projectile) != ITEM_MISSILE)
    {
      act("$p is not a projectile!", FALSE, ch, projectile, 0, TO_CHAR);
      return;
    }
      else if ((dir = search_block(arg2, dirs, FALSE)) >= 0)
    {
      if ( (dir > NUM_OF_DIRS) || (dir == -1) || !EXIT(ch, dir) )
        {
          send_to_char("Interesting Direction.\r\n", ch);
          return;
        }

      /* wielding a bow? */
      bow = ch->equipment[WEAR_WIELD];
      if ((!bow) || (GET_OBJ_TYPE(bow) != ITEM_FIREWEAPON))
        {
          send_to_char("You must wield a bow or sling to fire a "
               "projectile.\r\n", ch);
          return;
        }

      /* this is where they are shooting to */
      targ_room = EXIT(ch, dir)->to_room;

      /* room there? */
      if (dir < 0 || dir >= NUM_OF_DIRS)
        return;
      else if (!EXIT(ch, dir) || targ_room == NOWHERE)
        {
          send_to_char("Alas, you cannot shoot that way...\r\n", ch);
          return;
        }
          else if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED))
        {
          if (EXIT(ch, dir)->keyword)
        {
          sprintf(buf2, "The %s seems to be closed.\r\n",
              fname(EXIT(ch, dir)->keyword));
          send_to_char(buf2, ch);
            }
          else
        send_to_char("It seems to be closed.\r\n", ch);
          return;
        }

      if (ROOM_FLAGGED(targ_room, ROOM_PEACEFUL) ||
          ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        {
          send_to_char("You feel too peaceful to contemplate "
               "violence.\r\n", ch);
          return;
        }
      to = get_char_room(arg3, targ_room);
      if (!to)
        to = world[targ_room].people;

      if (!to)
        {
          send_to_char("Twang...\r\n", ch);
          obj_from_char(projectile);
          obj_to_room(projectile, targ_room);
          return;
        }

      if (dir == 0) from = "the south";
      if (dir == 1) from = "the west";
      if (dir == 2) from = "the north";
      if (dir == 3) from = "the east";
      if (dir == 4) from = "below";
      if (dir == 5) from = "above";

      if (!IS_NPC(to) && ((GET_LEVEL(to) < 10)||(GET_LEVEL(to) > 30)) )
        {
          send_to_char("Maybe that isn't such a great idea...\r\n", ch);
          return;
        }

      /* not when they are fighting... */
      if (FIGHTING(to))
        {
          send_to_char("It looks like they are fighting, you can't "
               "aim properly.\r\n", ch);
          return;
        }

      /* no shooting stationary mobs */
      if ( (to) && IS_NPC(to) && MOB_FLAGGED(to, MOB_SENTINEL) )
        {
          send_to_char("You cannot see well enough to aim...\r\n", ch);
          return;
        }

      /* its okay for them to shoot it... */

      sprintf(buf, "$n fires %s %s with %s.", AN(arg1), arg1,
          (bow)->short_description);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);

      send_to_char("Twang... your projectile flies into the distance.\r\n",
               ch);
      obj_from_char(projectile);


          /* calculate if they hit */
          percent = number(1, 101);
          prob = GET_SKILL(ch, SKILL_SHOOT);

          prob += (dex_app[GET_DEX(ch)].miss_att * 10);
          prob -= (dex_app[GET_DEX(to)].reaction * 10);

      if (percent < prob) /* success */
        {
          /* calc damage */
          dam = GET_DAMROLL(ch);
          dam += dice(GET_OBJ_VAL(projectile, 1),
              GET_OBJ_VAL(projectile, 2));
          dam += dice(GET_OBJ_VAL(bow, 1), GET_OBJ_VAL(bow, 2));

          send_to_char("You hear a roar of pain!\r\n", ch);
          extract_obj(projectile);
              improve_skill(ch, SKILL_SHOOT);

          /* did they shoot at a mob? */
          if ( (to) && IS_NPC(to) )
        {
          sprintf(buf, "Some kind of %s streaks in from %s and "
              "strikes $n!", arg1, from);
          act(buf, FALSE, to, 0, 0, TO_ROOM);
          send_to_char("Suddenly some kind of projectile pierces your "
                   "arm!\r\n", to);
          GET_HIT(to) -= dam;
          update_pos(to);
          if (GET_POS(to) == POS_DEAD)
            {
              die(to);
              return;
            }
          send_to_char("You decide to go investigate...\r\n", to);
          char_from_room(to);
          char_to_room(to, ch->in_room);
          sprintf(buf, "$n bursts into the room and scowls at $N.\r\n");
          act(buf, FALSE, to, 0, ch, TO_NOTVICT);
          sprintf(buf, "%s bursts into the room and scowls at "
              "you!\r\n", GET_NAME(to));
          send_to_char(buf, ch);
          hit(to, ch, TYPE_UNDEFINED);

        }
          /* no, its another player */
          else
        {
          sprintf(buf, "Some kind of %s streaks in from %s and "
              "hits you!\r\n", arg1, from);
          send_to_char(buf, to);
          sprintf(buf, "Some kind of %s streaks in from %s and "
              "strikes $n!", arg1, from);
          act(buf, FALSE, to, 0, 0, TO_ROOM);
          GET_HIT(to) -= dam;
          update_pos(to);
          if (GET_POS(to) == POS_DEAD)
            {
              die(to);
              return;
            }
        }

          /* it shot but they missed */
        }
      else
        {
          sprintf(buf, "Some kind of %s streaks in from %s and just "
              "misses you!\r\n", arg1, from);
          send_to_char(buf, to);
          sprintf(buf, "Some kind of %s streaks in from %s and narrowly "
              "misses $n!", arg1, from);
          act(buf, FALSE, to, 0, 0, TO_ROOM);

          obj_to_room(projectile, to->in_room);
        }

      /* no such direction */
    }
      else
    {
      send_to_char("Interesting direction.\r\n", ch);
      return;
    }
    }
}


ACMD(do_retreat)
{
  int i, attempt, percent, prob;
  char mybuf[80];
  char capmsg[20];
  char lowmsg[20];

  if (GET_CLASS(ch) == CLASS_NINJA)
    {
      strcpy(capmsg, "Escape");
      strcpy(lowmsg, "escape");
    }
  else
    {
      strcpy(capmsg, "Retreat");
      strcpy(lowmsg, "retreat");
    }

  if (GET_POS(ch)<POS_FIGHTING)
  {
    stc("Get on your feet first!\r\n", ch);
    return;
  }

  if ( !GET_SKILL(ch, SKILL_ESCAPE) && !GET_SKILL(ch, SKILL_RETREAT) )
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

  if (!FIGHTING(ch))
    {
      sprintf(mybuf,"%s from what? You aren't fighting!\n\r", capmsg);
      send_to_char(mybuf, ch);
      return;
    }


  percent = number(1, 101);   /* 101% is a complete failure */
  if (GET_CLASS(ch) == CLASS_NINJA)
    prob = GET_SKILL(ch, SKILL_ESCAPE);
  else
    prob = GET_SKILL(ch, SKILL_RETREAT);
  if (percent > prob)
    {
      sprintf(mybuf, "You try to %s but get cornered in the process!\r\n",
          lowmsg);
      improve_skill(ch, (GET_CLASS(ch)==CLASS_NINJA?
             SKILL_ESCAPE:
             SKILL_RETREAT));
      send_to_char(mybuf, ch);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
      return;
    }

  for (i = 0; i < 6; i++)
    {
      attempt = number(0, NUM_OF_DIRS - 1);
      if (CAN_GO(ch, attempt) &&
      !IS_SET_AR(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH))
    {
      sprintf(mybuf, "$n realizes it's a losing cause and gracefully"
          " attempts to %s.", lowmsg);
      act(mybuf, TRUE, ch, 0, 0, TO_ROOM);
      if (do_simple_move(ch, attempt, TRUE))
        {
          sprintf(mybuf, "You make a hasty %s.\r\n", lowmsg);
          send_to_char(mybuf, ch);
        }
      else
        {
          sprintf(mybuf, "$n is cornered and fails to %s!", lowmsg);
          act(mybuf, TRUE, ch, 0, 0, TO_ROOM);
        }
      return;
    }
    }
  sprintf(mybuf, "You are cornered and fail to %s!\r\n", lowmsg);
  send_to_char(mybuf, ch);
  sprintf(mybuf, "$n is cornered and fails to %s!", lowmsg);
  act(mybuf, TRUE, ch, 0, 0, TO_ROOM);
}

ACMD(do_subdue)
{
  struct char_data *victim;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_SUBDUE))
    {
      send_to_char("You have no idea how!\r\n", ch);
      return;
    }
  if (FIGHTING(ch))
    {
      send_to_char("You're too busy right now!\r\n", ch);
      return;
    }
  if (!(victim = get_char_room_vis(ch, arg)))
    {
      send_to_char("Subdue who?\r\n", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
      send_to_char("You can't contemplate violence in such a place!\r\n", ch);
      return;
    }
  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }
  if (FIGHTING(victim))
    {
      send_to_char("You can't get close enough!\r\n", ch);
      return;
    }

  if (!IS_NPC(victim) && !IS_OUTLAW(ch) && !subcmd)
  {
    stc("You can't subdue them because you are not an Outlaw!\r\n", ch);
    sprintf(buf, "%s failed to subdue you because %s is not an Outlaw.\r\n", GET_NAME(ch), GET_NAME(ch));
    stc(buf, victim);
    return;
  }

  if (is_shopkeeper(victim))
   {
     send_to_char("Haha.. Don't think so.\r\n", ch);
     return;
   }

  if (GET_POS(victim) <= POS_STUNNED) {
    send_to_char("What's the point of doing that now?\r\n", ch);
    return;
  }

  percent = number(1, 101+GET_LEVEL(victim));   /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_SUBDUE);

  if (GET_LEVEL(ch) >= LEVEL_IMMORT)
    percent = 0;
  if (MOB_FLAGGED(victim, MOB_AWARE))
    prob = 0;

  if (!IS_NPC(victim) && GET_LEVEL(ch) < LVL_IMMORT)
    if ((GET_LEVEL(victim) > GET_LEVEL(ch) + 3) || (GET_LEVEL(victim) < GET_LEVEL(ch) - 3))
      prob = 0;

  percent += MAX(GET_LEVEL(victim)-GET_LEVEL(ch),0);

  if (percent > prob)
    {
      act("$n misses a blow to the back of your head.",
      TRUE, ch, 0, victim, TO_VICT);
      act("$N avoids your misplaced blow to the back of $S head.",
      TRUE, ch, 0, victim, TO_CHAR);
      act("$N avoids $n's misplaced blow to the back of $S head.",
      TRUE, ch, 0, victim, TO_NOTVICT);
      hit(victim, ch, TYPE_UNDEFINED);
    }
  else
    {
      send_to_char("Someone sneaks up behind you and knocks you out!\r\n",
           victim);
      act("You knock $M out cold.", TRUE, ch, 0, victim, TO_CHAR);
      act("$n knocks out $N with a well-placed blow to the back of the head.",
      TRUE, ch, 0, victim, TO_NOTVICT);
      GET_POS(victim) = POS_STUNNED;
      WAIT_STATE(ch, PULSE_VIOLENCE * 1);
      improve_skill(ch, SKILL_SUBDUE);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_sleeper)
{
  struct char_data *vict;
  struct affected_type af;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_SLEEPER))
  {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (FIGHTING(ch))
  {
    send_to_char("You can't do this while fighting!\r\n", ch);
    return;
  }

  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
  {
    send_to_char("This room just has such a peaceful,"
     " easy feeling...\r\n", ch);
    return;
  }

 if (GET_EQ(ch, WEAR_WIELD))
   {
     stc("You can't get a good grip on them while you are holding that"
         " weapon!\r\n", ch);
     return;
   }

 if(!(vict = get_char_room_vis(ch, arg))) {
   send_to_char("Sleeper who?\r\n", ch);
   return;
 }

 if(vict == ch) {
   send_to_char("Can't get to sleep fast enough, huh?\r\n", ch);
   return;
 }

 if (!IS_NPC(vict) && !IS_OUTLAW(ch) && !subcmd)
 {
   stc("You can not sleeper them because you are not an Outlaw!\r\n", ch);
   sprintf(buf, "%s failed to sleeper you because %s is not an Outlaw.\r\n", GET_NAME(ch), GET_NAME(ch));
   stc(buf, vict);
   return;
 }

 if (FIGHTING(vict))
 {
   stc("You can't get a good grip on them while they're fighting!\r\n", ch);
   return;
 }

 if(is_shopkeeper(vict))
 {
   stc("Ha Ha. Don't think so.\r\n", ch);
   return;
 }

 if(GET_POS(vict) <= POS_SLEEPING) {
   send_to_char("What's the point of doing that now?\r\n", ch);
   return;
 }

 percent = number(1, 101 + GET_LEVEL(vict));
 prob = GET_SKILL(ch, SKILL_SLEEPER);

 if(MOB_FLAGGED(vict, MOB_AWARE) || MOB_FLAGGED(vict, MOB_NOSLEEP))
   prob = 0;

 if (!IS_NPC(vict) && GET_LEVEL(ch) < LVL_IMMORT)
   if ((GET_LEVEL(vict) > GET_LEVEL(ch) + 3) || (GET_LEVEL(vict) < GET_LEVEL(ch) - 3))
     prob = 0;

 percent += (MAX(GET_LEVEL(vict)-GET_LEVEL(ch),0));

 if (percent > prob) { /* failed */
   act("You try to grab $N in a sleeper hold but fail!", TRUE, ch, 0, vict, TO_CHAR);
   act("$n tries to put a sleeper hold on you, but you break free!",TRUE, ch, 0, vict, TO_VICT);
   act("$n tries to put $N in a sleeper hold...", TRUE, ch, 0, vict, TO_NOTVICT);
   hit(vict, ch, TYPE_UNDEFINED);
 } else {
   act("You put $N in a sleeper hold.", TRUE, ch, 0, vict, TO_CHAR);
   act("You feel very sleepy... Zzzzz..", TRUE, ch, 0, vict, TO_VICT);
   act("$n puts $N in a sleeper hold.", TRUE, ch, 0, vict, TO_ROOM);
   act("$N goes to sleep.", TRUE, ch, 0, vict, TO_ROOM);
   GET_POS(vict) = POS_SLEEPING;

    af.type = SKILL_SLEEPER;
    af.duration = GET_LEVEL(ch) / 9;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_to_char(vict, &af);

  improve_skill(ch, SKILL_SLEEPER);
 }
WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_neckbreak)
{
  struct char_data *vict;
  int percent, prob, dam;
  int needed_moves = 51;

  if (!GET_SKILL(ch, SKILL_NECKBREAK))
    {
      stc("What's that, idiot-san?\r\n", ch);
      return;
    }

  if (GET_EQ(ch, WEAR_WIELD))
    {
      stc("You can't do this and wield a weapon at the same time!\r\n",ch);
      return;
    }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg)))
    {
      stc("I don't see them here.\r\n", ch);
      return;
    }

 if (is_shopkeeper(vict))
   {
     send_to_char("Haha.. Don't think so.\r\n", ch);
     return;
   }


 if (vict == ch)
   {
     stc("Aren't we funny today...\r\n", ch);
     return;
   }

 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
   {
     stc("You can't contemplate violence in such a place!\r\n", ch);
     return;
   }

 if (IS_MOUNTED(ch))
   {
      stc("Dismount first!\r\n", ch);
      return;
   }


 if (GET_MOVE(ch) < needed_moves)
   {
     stc("You haven't the energy to do this!\r\n", ch);
     return;
   }
 else
   GET_MOVE(ch) = GET_MOVE(ch) - needed_moves;

 percent = ((7 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
 /* 101% is a complete failure */
 prob = GET_SKILL(ch, SKILL_NECKBREAK);

 if (percent > prob)
   {
     act("You try to break $S neck, but $E is too strong!",
         TRUE, ch, 0, vict, TO_CHAR);
     act("$n tries to break your neck, but can't!",
         TRUE, ch, 0, vict, TO_VICT);
     act("$n tries to break $N's neck, but $N slips free!",
         TRUE, ch, 0, vict, TO_NOTVICT);
     hit(vict, ch, TYPE_UNDEFINED);
  }
 else
  {
    dam = dice(18, GET_LEVEL(ch));
    damage(ch, vict, dam, SKILL_NECKBREAK);
    improve_skill(ch, SKILL_NECKBREAK);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);

}

struct ambush_event {
  struct char_data *ch;
  struct char_data *victim;
  int was_in;
};


EVENTFUNC(ambush_event)
{
  struct ambush_event *ambush = (struct ambush_event *) event_obj;
  struct char_data *ch = NULL;
  struct char_data *victim = NULL;
  struct obj_data *wielded = NULL;
  extern struct str_app_type str_app[];

  int was_in = 0, now_in = 0;
  int percent=0, prob=0, dam=0;

  ch = ambush->ch;
  victim = ambush->victim;
  was_in = ambush->was_in;
  now_in = ch->in_room;
  FREE(event_obj);

  if (!GET_ACTION(ch))
    return 0;

  GET_ACTION(ch) = NULL;

  if (FIGHTING(ch))
    return 0;

  if ((was_in != now_in) || (IN_ROOM(victim) != now_in) )
  {
    stc("You seem to have lost your prey.\r\n", ch);
    return 0;
  }

  percent = number(1, 131);
  prob = GET_SKILL(ch, SKILL_AMBUSH);

  if (MOB_FLAGGED(victim, MOB_AWARE))
    percent = 200;

  if (percent > prob) // failure
    damage(ch, victim, 0, SKILL_AMBUSH);
  else
  {
    // we could just call hit() here, like backstab, etc.
    // but since ambush is a special skill we'll let it
    // make an end-run around AC for now.

    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);

    wielded = GET_EQ(ch, WEAR_WIELD);

    if (wielded)
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));

    dam += (GET_LEVEL(ch)*2.6); // the bonus damage

    if (IS_AFFECTED(ch, AFF_HIDE)) // 10% more damage if hidden
      dam += (dam * .10);

    damage(ch, victim, dam, SKILL_AMBUSH);
    improve_skill(ch, SKILL_AMBUSH);
    if (victim)
      WAIT_STATE(victim, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);
  return 0;
}


ACMD(do_ambush)
{
  struct char_data *victim;
  struct ambush_event *ambush;

  one_argument(argument, arg);

  if (!(victim = get_char_room_vis(ch, arg)))
  {
    stc("Ambush who?\r\n", ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_AMBUSH))
  {
    stc("You'd better not.\r\n", ch);
    return;
  }

  if (GET_ACTION(ch))
  {
    stc("You are a little busy for that right now!\r\n", ch);
    return;
  }

  if (ch == victim)
  {
    stc("Ambush yourself? You idiot!\r\n", ch);
    return;
  }

  if (SECT(ch->in_room) != SECT_FOREST &&
      SECT(ch->in_room) != SECT_HILLS &&
      SECT(ch->in_room) != SECT_MOUNTAIN &&
      SECT(ch->in_room) != SECT_CITY)
  {
    stc("Ambush someone here? Impossible!\r\n", ch);
    return;
  }

  if (FIGHTING(victim))
  {
     stc("They're too alert for that, currently.\r\n", ch);
     return;
  }

  stc("You crouch in the shadows and plan your ambush...\r\n", ch);

  CREATE(ambush, struct ambush_event, 1);
  ambush->ch = ch;
  ambush->victim = victim;
  ambush->was_in = ch->in_room;
  GET_ACTION(ch) = event_create(ambush_event, ambush, PULSE_VIOLENCE*2);

}


