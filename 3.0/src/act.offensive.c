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


/* $Id: act.offensive.c 1259 2005-12-13 11:35:34Z mcooper $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "events.h"

/* extern variables */
extern char *dirs[];    
extern struct combat_skill_table combat_table[];

/* extern functions */
void improve_skill(struct char_data *ch, int skill_num);
void raw_kill(struct char_data * ch, struct char_data * killer, int attacktype);
ACMD(do_stand);
int find_moves(int skill_number);

/* internal functions */
void set_guarding(struct char_data *ch, struct char_data *vict);
void stop_guarding(struct char_data *ch);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }

  if (IS_MOUNTED(ch)) {
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
  else {
    for (opponent = world[ch->in_room].people;
         opponent && (FIGHTING(opponent) != helpee);
         opponent = opponent->next_in_room);
      
      if (!opponent)
        act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
      else if (!CAN_SEE(ch, opponent))
        act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
      else {
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
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
      send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
      return;
    }
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      if (IS_MOUNTED(ch))
        do_dismount(ch, NULL, 0, 0);
      hit(ch, vict, TYPE_UNDEFINED);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}

/* do_kill is just an alias for two other nasty functions */
ACMD(do_kill)
{
  ACMD(do_slay);

  if ((GET_LEVEL(ch) < LVL_IMMORT) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  } else {
    do_slay(ch, argument, cmd, subcmd);
    return;
  }
}

ACMD(do_backstab)
{
  struct char_data *vict;

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }

  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }

  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }
  
  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_BACKSTAB)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_BACKSTAB);
  }

  if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  if (!AWAKE(vict) || skill_success(ch, SKILL_BACKSTAB, 0)) {
    hit(ch, vict, SKILL_BACKSTAB);
    improve_skill(ch, SKILL_BACKSTAB);
    COMBAT_WAIT(vict, 1);
  } else
    damage(ch, vict, 0, SKILL_BACKSTAB);

  COMBAT_WAIT(ch, 5);
}

ACMD(do_disembowel)
{
  struct char_data *vict;

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n",ch);
    return;
  }

  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for disemboweling.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_DISEMBOWEL)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_DISEMBOWEL);
  }

  if (!AWAKE(vict) || skill_success(ch, SKILL_DISEMBOWEL, 0)) {
    hit(ch, vict, SKILL_DISEMBOWEL);
    improve_skill(ch, SKILL_DISEMBOWEL);
    COMBAT_WAIT(vict, 2);
  } else
    damage(ch, vict, 0, SKILL_DISEMBOWEL);

  COMBAT_WAIT(ch, 6);
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

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
    send_to_char("You cannot give any orders!\r\n", ch);
  else if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name))
     && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);
  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);
  
      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
        act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
        send_to_char(OK, ch);
        command_interpreter(vict, message);
      }
    } else {      /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);
  
      org_room = ch->in_room;
  
      for (k = ch->followers; k; k = k->next) {
        if (k->follower && (org_room == k->follower->in_room))
          if (k->follower && IS_AFFECTED(k->follower, AFF_CHARM)) {
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
  int i, attempt;
  struct event_data *event;
  struct char_data *attacker;
  extern struct event_data *pending_events;

  /* Last minute check! */
  if (check_dead(ch))
    return;

  if (GET_POS(ch) <= POS_INCAP) {
    stc("How do you expect to do that in your condition?\r\n", ch);
    return;
  }

  if (GET_POS(ch) < POS_FIGHTING)
    do_stand(ch, "", 0, 0);

  if (GET_COMBAT_WAIT(ch)) {
    if (pending_events)
      for (event = pending_events; event; event = event->next)
        if ((event->ch == ch) && (event->command != do_flee))
          remove_event(event);

    CREATE(event, struct event_data, 1);
    event->ch = ch;
    event->args = str_dup(argument);
    event->cmd = cmd;
    event->subcmd = subcmd;
    event->count = GET_COMBAT_WAIT(ch);
    event->fighting = FIGHTING(ch);
    event->command = do_flee;
    DELAY(ch) = 0;
    add_event(event);
    return;
  }

  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);  /* Select a random direction */
    if (CAN_GO(ch, attempt) && !IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH) &&
        !IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_SHOP_CLOSED)) {
      if (IS_NPC(ch))
        if (IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_NOMOB))
          return;

      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      GET_WAS_IN(ch) = ch->in_room;
      if (do_simple_move(ch, attempt, TRUE)) {
        send_to_char("You flee head over heels.\r\n", ch);
        for (attacker = world[GET_WAS_IN(ch)].people; attacker; attacker = attacker->next_in_room)
          if (FIGHTING(attacker) == ch)
            stop_fighting(attacker);
        stop_fighting(ch);
      } else {
        act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
        WAIT_STATE(ch, 1);
      }
      GET_WAS_IN(ch) = NOWHERE;
      return;
    }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_bash)
{
  struct char_data *vict;
  int dam = dice(3, 4) + (GET_LEVEL(ch) / 5);

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);
  
  if (GET_POS(vict) < POS_FIGHTING) {
    stc("You can't bash someone who's sitting already!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_BASH)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_BASH);
  }

  if ((skill_success(ch, SKILL_BASH, 0) && !MOB_FLAGGED(vict, MOB_NOBASH)) || !AWAKE(vict)) {
    damage(ch, vict, dam, SKILL_BASH);
    improve_skill(ch, SKILL_BASH);
    if (!check_dead(vict)) {
      GET_POS(vict) = POS_SITTING;
      COMBAT_WAIT(vict, 2);
    }
  } else {
    damage(ch, vict, 0, SKILL_BASH);
    GET_POS(ch) = POS_SITTING;
    WAIT_STATE(ch, 1);    /* one extra for missing */
  }
  WAIT_STATE(ch, 3);
  COMBAT_WAIT(ch, 3);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;  

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Rescue who??\r\n", ch);
    return;
  }

  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }

  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);
    if (!tmp_ch) {
      act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
      return;
    }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (!IS_NPC(tmp_ch) && !IS_OUTLAW(ch)) {
      act("Both you and $N's attacker must be an outlaw for you to rescue $M.", TRUE, ch, 0, vict, TO_CHAR);
      return;
    }

    if (!IS_NPC(tmp_ch) && !IS_OUTLAW(tmp_ch)) {
      act("Both you and $N's attacker must be an outlaw for you to rescue $M.", TRUE, ch, 0, vict, TO_CHAR);
      return;
    }
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_RESCUE)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_RESCUE);
  }

  if (skill_success(ch, SKILL_RESCUE, 0)) {
    send_to_char("Banzai!  To the rescue...\r\n", ch);
    act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
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

    COMBAT_WAIT(vict, 2);
  } else
    send_to_char("You fail the rescue!\r\n", ch);

  COMBAT_WAIT(ch, 5);
}


ACMD(do_kick)
{
  struct char_data *vict;
  int dam = dice(3, 4) + (GET_LEVEL(ch) / 5);

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_KICK)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_KICK);
  }

  if (!AWAKE(vict) || skill_success(ch, SKILL_KICK, 0)) {
    damage(ch, vict, dam, SKILL_KICK);
    improve_skill(ch, SKILL_KICK);
    COMBAT_WAIT(vict, 1);
  } else
    damage(ch, vict, 0, SKILL_KICK);

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 3);
}


ACMD(do_dragon_kick)
{
  struct char_data *vict;

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_DRAGON_KICK)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_DRAGON_KICK);
  }

  if (skill_success(ch, SKILL_DRAGON_KICK, ((5 - (GET_AC(vict) / 10)) << 1))) {
    damage(ch, vict, GET_LEVEL(ch)*1.5, SKILL_DRAGON_KICK);
    improve_skill(ch, SKILL_DRAGON_KICK);
  } else
    damage(ch, vict, 0, SKILL_DRAGON_KICK);

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 3);
}

ACMD(do_tiger_punch)
{
  struct char_data *vict;

  if (GET_EQ(ch, WEAR_WIELD)) {
    stc("That's pretty tough to do while wielding a weapon.\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_TIGER_PUNCH)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_TIGER_PUNCH);
  }

  if (skill_success(ch, SKILL_TIGER_PUNCH, ((7 - (GET_AC(vict) / 10)) << 1))) {
    damage(ch, vict, GET_LEVEL(ch) * 2.5, SKILL_TIGER_PUNCH);
    improve_skill(ch, SKILL_TIGER_PUNCH);
  } else
    damage(ch, vict, 0, SKILL_TIGER_PUNCH);

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 3);
}

ACMD(do_retreat)   
{
  int i, attempt;

  if (!FIGHTING(ch)) {
    send_to_char("Retreat from what? You aren't fighting!\r\n", ch);
    return;
  }

  /* Last minute check! */
  if (check_dead(ch))
    return;

  if (GET_POS(ch) <= POS_INCAP) {
    stc("How do you expect to do that in your condition?\r\n", ch);
    return;
  }

  if (GET_POS(ch) < POS_FIGHTING)
    do_stand(ch, "", 0, 0);

  if (skill_success(ch, SKILL_RETREAT, 0)) {
    for (i = 0; i < 6; i++) {
      attempt = number(0, NUM_OF_DIRS - 1);
      if (CAN_GO(ch, attempt) && !IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH) &&
          !IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_SHOP_CLOSED)) {
        act("$n realizes it's a losing cause and gracefully attempts to retreat.", TRUE, ch, 0, 0, TO_ROOM);
        if (do_simple_move(ch, attempt, TRUE)) {
          send_to_char("You make a hasty retreat.\r\n", ch);
          improve_skill(ch, SKILL_RETREAT);
        } else
          act("$n is cornered and fails to retreat!", TRUE, ch, 0, 0, TO_ROOM);
        return;
      }
    }
    send_to_char("You are cornered and fail to retreat!\r\n", ch);
    act("$n is cornered and fails to retreat!",TRUE, ch, 0, 0, TO_ROOM);
  } else
    send_to_char("You try to retreat but get cornered in the process!\r\n", ch);
}


ACMD(do_subdue)
{
  struct char_data *vict;

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Subdue who??\r\n", ch);
    return;
  }

  if (FIGHTING(vict)) {
    send_to_char("You can't get close enough!\r\n", ch);
    return;
  }

  if (GET_POS(vict) <= POS_STUNNED) {
    send_to_char("What's the point of doing that now?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_SUBDUE)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_SUBDUE);
  }

  if (!AWAKE(vict) || (skill_success(ch, SKILL_SUBDUE, 0) && !MOB_FLAGGED(vict, MOB_AWARE))) {
    if (GET_LEVEL(vict) + 10 < GET_LEVEL(ch))
      if (!skill_success(ch, SKILL_SUBDUE, 0)) {
        /* Greater than a 10 level split, must be successful twice! */
        act("$n misses a blow to the back of your head.", TRUE, ch, 0, vict, TO_VICT);
        act("$N avoids your misplaced blow to the back of $S head.", TRUE, ch, 0, vict, TO_CHAR);
        act("$N avoids $n's misplaced blow to the back of $S head.", TRUE, ch, 0, vict, TO_NOTVICT);
        hit(vict, ch, TYPE_UNDEFINED);
        WAIT_STATE(ch, 3);
        COMBAT_WAIT(ch, 5);
        return;
      }

    send_to_char("Someone sneaks up behind you and knocks you out!\r\n", vict);
    act("You knock $M out cold.", TRUE, ch, 0, vict, TO_CHAR);
    act("$n knocks out $N with a well-placed blow to the back of the head.", TRUE, ch, 0, vict, TO_NOTVICT);
    GET_POS(vict) = POS_STUNNED;
    improve_skill(ch, SKILL_SUBDUE);
  } else {
    act("$n misses a blow to the back of your head.", TRUE, ch, 0, vict, TO_VICT);
    act("$N avoids your misplaced blow to the back of $S head.", TRUE, ch, 0, vict, TO_CHAR);
    act("$N avoids $n's misplaced blow to the back of $S head.", TRUE, ch, 0, vict, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, 1);
  }

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 5);
}

ACMD(do_sleeper)
{
  struct char_data *vict;
  struct affected_type af;

  if (GET_EQ(ch, WEAR_WIELD)) {
    stc("You can't get a good grip on them while you are holding that weapon!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    return;

  if (!IS_NPC(ch) && !IS_NPC(vict) && GET_LEVEL(vict) <= 10) {
    stc("Go sleeper someone your own size!\r\n", ch); 
    return;
  }        

  if (FIGHTING(vict)) {
    stc("You can't get a good grip on them while they're fighting!\r\n", ch);
    return;
  }

  if(GET_POS(vict) <= POS_SLEEPING) {
    send_to_char("What's the point of doing that now?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_SLEEPER)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_SLEEPER);
  }

  if (skill_success(ch, SKILL_SLEEPER, 0) && !MOB_FLAGGED(vict, MOB_AWARE) && !MOB_FLAGGED(vict, MOB_NOSLEEP)) {
    if (GET_LEVEL(vict) + 10 < GET_LEVEL(vict))
      if (!skill_success(ch, SKILL_SLEEPER, 0)) {
        /* Greater than a 10 level split, must be successful twice! */
        act("You try to grab $N in a sleeper hold but fail!", TRUE, ch, 0, vict, TO_CHAR);
        act("$n tries to put a sleeper hold on you, but you break free!",TRUE, ch, 0, vict, TO_VICT);
        act("$n tries to put $N in a sleeper hold...", TRUE, ch, 0, vict, TO_NOTVICT);
        hit(vict, ch, TYPE_UNDEFINED);
        WAIT_STATE(ch, 3);
        COMBAT_WAIT(ch, 6);
        return;
      }

    act("You put $N in a sleeper hold.", TRUE, ch, 0, vict, TO_CHAR);
    act("You feel very sleepy... Zzzzz..", TRUE, ch, 0, vict, TO_VICT);
    act("$n puts $N in a sleeper hold.", TRUE, ch, 0, vict, TO_ROOM);
    act("$N goes to sleep.", TRUE, ch, 0, vict, TO_ROOM);
    GET_POS(vict) = POS_SLEEPING;

    af.type = SKILL_SLEEPER;
    af.duration = MAX(1, (GET_LEVEL(ch) / 25));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_to_char(vict, &af);
  
    improve_skill(ch, SKILL_SLEEPER);
  } else {
    act("You try to grab $N in a sleeper hold but fail!", TRUE, ch, 0, vict, TO_CHAR);
    act("$n tries to put a sleeper hold on you, but you break free!",TRUE, ch, 0, vict, TO_VICT);
    act("$n tries to put $N in a sleeper hold...", TRUE, ch, 0, vict, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, 1);
  }
  WAIT_STATE(ch, 1);
  COMBAT_WAIT(ch, 6);
}  

ACMD(do_neckbreak)
{
  struct char_data *vict;
  int dam = dice(20, 7) + (GET_LEVEL(ch) / 5);

  if (GET_EQ(ch, WEAR_WIELD)) {
    stc("You can't do this and wield a weapon at the same time!\r\n",ch);
    return;
  }

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  /* if (GET_LEVEL(ch) < LVL_IMMORT) { */
    if (GET_MOVE(ch) < find_moves(SKILL_NECKBREAK)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_NECKBREAK);
  /* } */

  if (!AWAKE(vict) || skill_success(ch, SKILL_NECKBREAK, 0)) {
    damage(ch, vict, dam, SKILL_NECKBREAK);
    improve_skill(ch, SKILL_NECKBREAK);
    COMBAT_WAIT(vict, 2);
  } else
    hit(vict, ch, TYPE_UNDEFINED);

  WAIT_STATE(ch, 3);
  COMBAT_WAIT(ch, 6);
}


ACMD(ambush_event)
{
  struct char_data *vict;
  int dam;

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    stc("You seem to have lost your prey.\r\n", ch);
    return;
  }

  if (FIGHTING(vict)) {
    stc("They're too alert for that, currently.\r\n", ch);
    return;
  }

  if (SECT(ch->in_room) != SECT_FOREST &&
      SECT(ch->in_room) != SECT_HILLS &&
      SECT(ch->in_room) != SECT_MOUNTAIN) {
    stc("Ambush someone here? Impossible!\r\n", ch);
    return;
  }

  if (check_dead(vict) || FIGHTING(ch)) {
    send_to_char("There's no point in ambushing now!\r\n", ch);
    return;
  }

  if (skill_success(ch, SKILL_AMBUSH, 0) && !MOB_FLAGGED(vict, MOB_AWARE)) {
    send_to_char("The ambush is successful!\r\n", ch);
    dam = dice(10, 8) + (GET_LEVEL(ch) / 5);
    if (IS_AFFECTED(ch, AFF_HIDE))  /* more damage if you are hidden */
      dam += 25;
    damage(ch, vict, dam, SKILL_AMBUSH);
    improve_skill(ch, SKILL_AMBUSH);
    COMBAT_WAIT(vict, 2);
  } else {
    act("$N evades your ambush and strikes!", TRUE, ch, 0, vict, TO_CHAR);
    hit(ch, vict, TYPE_UNDEFINED);
  }

  WAIT_STATE(ch, 1);
  COMBAT_WAIT(ch, 5);
  return;
}


ACMD(do_ambush)
{
  struct char_data *vict;
  struct event_data *event;

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Ambush who??\r\n", ch);
    return;
  }

  if (FIGHTING(vict)) {
    stc("They're too alert for that, currently.\r\n", ch);
    return;
  }

  if (SECT(ch->in_room) != SECT_FOREST &&
      SECT(ch->in_room) != SECT_HILLS &&
      SECT(ch->in_room) != SECT_MOUNTAIN) {
    stc("Ambush someone here? Impossible!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_AMBUSH)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_AMBUSH);
  }

  stc("You crouch in the shadows and plan your ambush...\r\n", ch);

  CREATE(event, struct event_data, 1);
  event->ch = ch;
  event->args = str_dup(argument);
  event->cmd = cmd;
  event->subcmd = subcmd;
  event->count = 2 * PULSE_VIOLENCE;
  event->fighting = NULL;
  event->command = ambush_event;
  add_event(event);
}


/* **************************************************************************
   NAME       : do_bearhug
   PURPOSE    : bearhug skill
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_bearhug)
{
  struct char_data *vict;
  int dam = dice(5, 4) + (GET_LEVEL(ch) / 5);

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);
    
  if (GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to be bare handed to get a good grip.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_BEARHUG)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_BEARHUG);
  }

  if ((skill_success(ch, SKILL_BEARHUG, 0) && !MOB_FLAGGED(vict, MOB_NOBASH)) || !AWAKE(vict)) {
    damage(ch, vict, dam, SKILL_BEARHUG);
    improve_skill(ch, SKILL_BEARHUG);
    COMBAT_WAIT(vict, 1);
  } else
    damage(ch, vict, 0, SKILL_BEARHUG);

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 4);
}

/* **************************************************************************
   NAME       : do_headbutt
   PURPOSE    :
   RETURNS    :  
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_headbutt)
{
  struct char_data *vict;
  int dam = dice(3, 4) + (GET_LEVEL(ch) / 5);
  
  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);
  
  if (GET_LEVEL(ch) / 2 > GET_HIT(ch)) {
    send_to_char("But that could kill you!\n\r",ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_HEADBUTT)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_HEADBUTT);
  }

  if ((skill_success(ch, SKILL_HEADBUTT, 0) && !MOB_FLAGGED(vict, MOB_NOBASH)) || !AWAKE(vict)) {
    if (!GET_EQ(ch, WEAR_HEAD))
      GET_HIT(ch) -= (dam * 0.5);
    else
      GET_HIT(ch) -= dam;
    damage(ch, vict, dam, SKILL_HEADBUTT);
    improve_skill(ch, SKILL_HEADBUTT);
    if (!check_dead(vict)) {
      if (GET_POS(vict) > POS_STUNNED)
        GET_POS(vict) = POS_SITTING;
      update_pos(vict);
      COMBAT_WAIT(vict, 1);
    }
  } else
    damage(ch, vict, 0, SKILL_HEADBUTT);

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 3);
}


void die_guards(struct char_data *ch)
{
  struct char_data *temp_ch = NULL, *next_ch = NULL;

  if (!ch)
    log("die_guards(): Invalid 'ch' char_data pointer.");
  else {
    /* Stop them from guarding if they currently are. */
    if (GUARDING(ch))
      stop_guarding(ch);

    /* Stop anyone who is presently guarding them. */
    for (temp_ch = ch->guarded_by; temp_ch; temp_ch = next_ch) {
      next_ch = temp_ch->next_guarding;
      stop_guarding(temp_ch);
    }
  }
}


void set_guarding(struct char_data *ch, struct char_data *vict)
{
  if (!ch)
    log("set_guarding(): Invalid character pointer.");
  else if (!vict)
    log("set_guarding(): No 'vict' character to guard.");
  else if (ch == vict)
    log("set_guarding(): Can't set character '%s' guarding %sself.",
        GET_NAME(ch), HMHR(ch));
  else {
    /* If we're guarding, stop. */
    if (GUARDING(ch))
      stop_guarding(ch);

    /* Add ch to the list of the charge's guards. */
    ch->next_guarding = vict->guarded_by;
    vict->guarded_by = ch;
    GUARDING(ch) = vict;
  }
}


void stop_guarding(struct char_data *ch)
{
  struct char_data *temp = NULL;

  if (!ch)
    log("stop_guarding(): Invalid character pointer.");
  else if (GUARDING(ch)) {
    REMOVE_FROM_LIST(ch, GUARDING(ch)->guarded_by, next_guarding);
    GUARDING(ch) = NULL;
    ch->next_guarding = NULL;
  }
}


ACMD(do_guard)
{
  char arg1[MAX_INPUT_LENGTH] = {'\0'};
  struct char_data *vict = NULL;

  argument = one_argument(argument, arg1);

  if (IS_NPC(ch)) {
    send_to_char("You're not allowed to do that!\r\n", ch);
    return;
  }

  if (*arg1 == '\0') {
    if (GUARDING(ch))
      act("You are guarding $N.", TRUE, ch, 0, GUARDING(ch), TO_CHAR);
    else
      send_to_char("You are guarding only yourself.\r\n", ch);
  } else if (!(vict = get_char_room_vis(ch, arg1)))
    send_to_char(NOPERSON, ch);
  else if (FIGHTING(ch) == vict || FIGHTING(vict) == ch)
    send_to_char("You can't guard someone you're fighting!\r\n", ch);
  else {
    if (vict == ch) {
      if (!GUARDING(ch))
        send_to_char("You're already guarding yourself!\r\n", ch);
      else {
        act("$n is no longer your guardian.", TRUE, ch, 0, GUARDING(ch), TO_VICT);
        act("You are no longer $N's guardian.", TRUE, ch, 0, GUARDING(ch), TO_CHAR);
        stop_guarding(ch);
      }
    } else {
      if (GUARDING(ch) == vict)
        act("You're already guarding $N.", TRUE, ch, 0, GUARDING(ch), TO_CHAR);
      else if (ch == GUARDING(vict))
        act("$N is already guarding you - you can't guard each other!", TRUE, ch, 0, vict, TO_CHAR);
      else {
        act("$n has elected to be your guardian.", TRUE, ch, 0, vict, TO_VICT);
        act("You have elected to be $N's guardian.", TRUE, ch, 0, vict, TO_CHAR);
        set_guarding(ch, vict);
      }
    }
  }
}

/* do_circle: circle around your victim to make a back-attack */
ACMD(do_circle)
{
  struct char_data *vict;

  one_argument(argument, buf);
  if (!(vict = get_char_room_vis(ch, buf)))
    vict = FIGHTING(ch);

  if (vict == ch) {
    send_to_char("You run circles around yourself.\r\n", ch);
    return;
  }

  if (FIGHTING(ch) && FIGHTING(FIGHTING(ch)) == ch) {
    send_to_char("You're already fighting someone else!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_CIRCLE)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_CIRCLE);
  }

  if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
    act("You notice $N lunging at you!", TRUE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", TRUE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", TRUE, vict, 0, ch, TO_NOTVICT);
    if (!FIGHTING(vict))
      hit(vict, ch, TYPE_UNDEFINED);
    COMBAT_WAIT(ch, 5);
    return;
  }

  if (!AWAKE(vict) || skill_success(ch, SKILL_CIRCLE, 0)) {
    if (FIGHTING(vict)) {
      stop_fighting(vict);
      hit(vict, ch, TYPE_UNDEFINED);
    }
    act("You circle around $N.", TRUE, ch, 0, vict, TO_CHAR);
    act("$n circles around you.", TRUE, ch, 0, vict, TO_VICT);
    act("$n circles around $N.", TRUE, ch, 0, vict, TO_NOTVICT);
    hit(ch, vict, SKILL_CIRCLE);
    COMBAT_WAIT(vict, 1);
  } else {
    act("You fail to circle around $N!", TRUE, ch, 0, vict, TO_CHAR);
    act("$n fails to circle around you!", TRUE, ch, 0, vict, TO_VICT);
    act("$n tries to circle around $N but fails!", TRUE, ch, 0, vict, TO_NOTVICT);
    damage(ch, vict, 0, SKILL_CIRCLE);
  }
  COMBAT_WAIT(ch, 5);
}

/* ************************************************************************** 
   NAME       : do_trip
   PURPOSE    : trip skill 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_trip)
{
  struct char_data *vict;
  
  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (IS_AFFECTED(vict, AFF_FLY)) {
    stc("You can't trip something that's FLYING!\r\n", ch);
    return;
  }

  if (GET_POS(vict) <= POS_SLEEPING) {
    stc("Whats the point of doing that now?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_TRIP)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_TRIP);
  }

  if (skill_success(ch, SKILL_TRIP, 0) && !MOB_FLAGGED(vict, MOB_NOBASH)) {
    damage(ch, vict, 1, SKILL_TRIP);
    improve_skill(ch, SKILL_TRIP);
    if (!check_dead(vict)) {
      GET_POS(vict) = POS_SITTING;
      update_pos(vict);
    }
    COMBAT_WAIT(vict, 2);
  } else {
    damage(ch, vict, 0, SKILL_TRIP);
    GET_POS(ch) = POS_SITTING;
    update_pos(ch);
    WAIT_STATE(ch, 1);
  }
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 4);
}

ACMD(do_slug)
{
  struct char_data *vict;
  int dam = dice(5, 4) + (GET_LEVEL(ch) / 5);

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (ch->equipment[WEAR_WIELD]) {
    send_to_char("You can't make a fist while wielding a weapon!\n\r", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_SLUG)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_SLUG);
  }

  if (!AWAKE(vict) || skill_success(ch, SKILL_SLUG, 0)) {
    damage(ch, vict, dam, SKILL_SLUG);
    improve_skill(ch, SKILL_SLUG);
    COMBAT_WAIT(vict, 3);
  } else {
    damage(ch, vict, 0, SKILL_SLUG);
    WAIT_STATE(ch, 1);
  }
  WAIT_STATE(ch, 3);
  COMBAT_WAIT(ch, 4);
}

ACMD(do_charge)
{
  struct char_data *vict;
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int dam;
  int mountdam = 0;

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (!wielded) {
    send_to_char("You're barehanded, try it with a sword or lance next time.\n\r", ch);
    return;
  }

  if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] != 12 
      && ch->equipment[WEAR_WIELD]->obj_flags.value[3] != 3) {
    stc("You need sword or a lance to run 'em through!\r\n",ch);
    return;
  }

  dam = dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2)) + (GET_LEVEL(ch) / 5);
  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_CHARGE)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_CHARGE);
  }

  if (!AWAKE(vict) || (skill_success(ch, SKILL_CHARGE, 0) && !MOB_FLAGGED(vict, MOB_NOBASH))) {
    if (IS_MOUNTED(ch))
      mountdam = 50;
    damage(ch, vict, mountdam + dam, SKILL_CHARGE);
    improve_skill(ch, SKILL_CHARGE);
    COMBAT_WAIT(vict, 2);
  } else {
    damage(ch, vict, 0, SKILL_CHARGE);
    if (!IS_MOUNTED(ch))
      GET_POS(ch) = POS_SITTING;
    update_pos(ch);
  }
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 5);
}

/* ************************************************************************** 
   NAME       : do_smackheads
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_smackheads)
{
  struct char_data *victim1, *victim2;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int dam = dice(10, 8) + (GET_LEVEL(ch) / 5);

  half_chop(argument,arg, arg2);
  if ((!arg) && (!arg2)) {
    send_to_char("Smack whose heads together?",ch);
    return;
  }

  if (!(victim1 = get_char_room_vis(ch, arg))
      || !(victim2 = get_char_room_vis(ch, arg2)) || (victim1==victim2)) {
    send_to_char("Looks like the gangs not all here...\n\r", ch);
    return;
  }

  if (victim1 == ch || victim2 == ch) {
    send_to_char("We call that 'headbutt' around here, son...\n\r", ch);
    return;
  }

  if (victim1==ch && victim2 == ch) {
    send_to_char("You smack your head twice and say 'DOH!'\r\n",ch);
    return;
  }

  if (ch->equipment[WEAR_WIELD]) {
    send_to_char("You need your hands free to smack some heads!\n\r", ch);
    return;
  }

  if (FIGHTING(ch)) {
    stc("You're a little busy right now!\r\n", ch);
    return;
  }

  if (FIGHTING(victim1) || FIGHTING(victim2)) {
    stc("They are too busy fighting at the moment!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_SMACKHEADS)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_SMACKHEADS);
  }

  if ((!AWAKE(victim1) && !AWAKE(victim2)) || skill_success(ch, SKILL_SMACKHEADS, 0)) {
    strcpy(buf, "You grab the heads of ");
    strcat(buf, GET_NAME(victim1));
    strcat(buf, " and ");
    strcat(buf, GET_NAME(victim2));
    strcat(buf, " and bang them together with a sickening *SMACK*.");
    act(buf, FALSE, ch, 0, 0, TO_CHAR);    
    strcpy(buf,GET_NAME(ch));
    strcat(buf, " grabs the heads of ");
    strcat(buf, GET_NAME(victim1));
    strcat(buf, " and ");
    strcat(buf, GET_NAME(victim2));
    strcat(buf, " and bangs them together with a sickening *SMACK*.");
    act(buf, FALSE, ch, 0, 0, TO_ROOM);    
    GET_POS(victim1) = POS_STUNNED;
    GET_POS(victim2) = POS_STUNNED;
    damage(ch, victim1, dam, SKILL_SMACKHEADS);
    damage(ch, victim2, dam, SKILL_SMACKHEADS);
    improve_skill(ch, SKILL_SMACKHEADS);
    COMBAT_WAIT(victim1, 2);
    COMBAT_WAIT(victim2, 2);
  } else {
    strcpy(buf, GET_NAME(victim1));
    strcat(buf, " and ");
    strcat(buf, GET_NAME(victim2));
    strcat (buf, " slip out of your hands!");
    act(buf, FALSE, ch, 0, 0, TO_CHAR);    
    strcpy(buf, GET_NAME(victim1));
    strcat(buf, " and ");
    strcat(buf, GET_NAME(victim2));
    strcat (buf, " duck as ");
    strcat (buf, GET_NAME(ch));
    strcat (buf, " lunges at them!");
    act(buf, FALSE, ch, 0, 0, TO_ROOM);    
    damage(ch, victim1, 0, SKILL_SMACKHEADS);
    damage(ch, victim2, 0, SKILL_SMACKHEADS);
  }
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 5);
}

/* ************************************************************************** 
   NAME       : do_berserk
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_berserk)
{
  struct affected_type af;
  
  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("You're unable to summon your battle rage right now.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_BERSERK)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_BERSERK);
  }

  if (skill_success(ch, SKILL_BERSERK, 0)) {
    send_to_char("Your vision turns sanguine as you summon up your battle rage!\r\n", ch);
    improve_skill(ch, SKILL_BERSERK);

    af.modifier  = MAX(1, (GET_LEVEL(ch) / 20));
    af.duration  = 1;
    af.type      = SKILL_BERSERK;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_BERSERK;
    affect_to_char(ch, &af);
  
    af.modifier  = MAX(1, (GET_LEVEL(ch) / 20));  
    af.location  = APPLY_DAMROLL;
    affect_to_char(ch, &af);
  
    af.modifier  = -25;
    af.location  = APPLY_AC;
    affect_to_char(ch, &af);
  } else
    send_to_char("You fail to summon up your battle rage.\r\n", ch);

  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 5);
}

/* ************************************************************************** 
   NAME       : do_cutthroat
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_cutthroat)
{
  struct master_affected_type af;
  struct char_data *vict;
  int dam = dice(5, 4) + (GET_LEVEL(ch) / 5);

  one_argument(argument, buf);
  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Cut who's throat?\r\n", ch);
    return;
  }

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\n\r", ch);
    return;
  }

  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != 11) {
    send_to_char("Only daggers and such can be used for cutting a throat.\n\r", ch);
    return;
  }

  if (!IS_NPC(vict) && GET_LEVEL(vict) <= 10) {
    act("Ancient forces protect $N from your wrath!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }

  if (IS_AFFECTED(vict, AFF_CUTTHROAT)) {
    send_to_char("Their throat is already slit!\r\n", ch);
    return;
  }

  if (FIGHTING(ch) || FIGHTING(vict)) {
    send_to_char("You can't get close enough!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_CUTTHROAT)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_CUTTHROAT);
  }

  if (skill_success(ch, SKILL_CUTTHROAT, 0)) {
    af.type = SKILL_CUTTHROAT;
    af.duration = MAX(1, (GET_LEVEL(ch) / 20));
    af.modifier = -2;
    af.location = APPLY_HITROLL;
    af.bitvector = AFF_CUTTHROAT;
    af.by_type = BY_SPELL;
    af.obj_num = 0;

    affect_join(vict, &af, FALSE, FALSE, FALSE);
    sprintf(buf, "Suddenly %s slits your throat!\r\n", GET_NAME(ch));
    send_to_char(buf, vict);
    send_to_char("You slit their throat from ear to ear!\r\n", ch);
    improve_skill(ch, SKILL_CUTTHROAT);
    damage(ch, vict, dam, SKILL_CUTTHROAT);
    COMBAT_WAIT(vict, 1);
  } else {
    send_to_char("Your slash at their throat barely misses!\n\r",ch);
    sprintf(buf, "%s makes a vicious lunge at your throat!\r\n", GET_NAME(ch));
    send_to_char(buf, vict);
    hit(ch, vict, SKILL_CUTTHROAT);
  }
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 4);
}

ACMD(do_groinrip)
{
  struct char_data *vict;
  int dam = dice(10, 10) + (GET_LEVEL(ch) / 5);

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);
 
  if (GET_SEX(vict) != SEX_MALE) {
    send_to_char("Umm, they have nothing there to tug on!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_GROINRIP)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_GROINRIP);
  }

  if (!AWAKE(vict) || skill_success(ch, SKILL_GROINRIP, 0)) {
    damage(ch, vict, dam, SKILL_GROINRIP);
    if (!check_dead(vict)) {
      act("$n falls to $s knees, clutching $s groin and throwing up everywhere!", TRUE, vict, 0, 0, TO_ROOM);
      if (!number(0,10)) {
        struct obj_data *obj = read_object(21, VIRTUAL, NULL); /*puke*/
        if (obj) {
          obj_to_room(obj, ch->in_room);
          GET_OBJ_TIMER(obj) = 2;
        }
      }
    }
    improve_skill(ch, SKILL_GROINRIP);
    COMBAT_WAIT(vict, 2);
  } else {
    send_to_char("You go for the groin but can't quite get a grip!\r\n", ch);
    sprintf(buf, "%s makes a vicious lunge at your groin!\r\n", GET_NAME(ch));
    send_to_char(buf, vict);
    hit(ch, vict, SKILL_GROINRIP);
  }
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 6);
}

ACMD(do_parry)
{
  if (!FIGHTING(ch)) {
    send_to_char("But you aren't fighting anyone!\r\n", ch);
    return;
  }

  if (FIGHTING(FIGHTING(ch)) != ch) {
    send_to_char("But noone's attacking you!\r\n", ch);
    return;
  }

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char ("Parry with what? You're unarmed!\r\n", ch);
    return;
  }

  if (!skill_success(ch, SKILL_PARRY, 0)) {
    send_to_char("With a dazzling show of swordplay, you attempt to parry...but are outmaneuvered!\r\n", ch);
    WAIT_STATE(ch, 3);
    COMBAT_WAIT(ch, 4);
    return;
  }       

  send_to_char("With a dazzling show of swordplay, you move into defensive position...\r\n", ch);
  act("$n displays a dazzling show of swordplay, fending off $N's every blow!", TRUE, ch, 0, FIGHTING(ch), TO_ROOM);
  act("$n displays a dazzling show of swordplay, fending off your every blow!", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
  IS_PARRIED(FIGHTING(ch)) = TRUE;
  improve_skill(ch, SKILL_PARRY);
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 4);
}

ACMD(do_turn)
{
  int diff=0, need_mana=0;
  struct char_data *tch, *next_tch;

  if (IS_EVIL(ch) || IS_NEUTRAL(ch)) {
    stc("You are not holy enough to turn away the Undead!\r\n", ch);
    return;
  }

  send_to_char("You attempt to turn away the unholy presence in this room.\r\n", ch);
  act("$n's eyes are suddenly filled with blinding white light!", FALSE, ch, 0, 0, TO_ROOM);

  need_mana = 150 - (GET_LEVEL(ch) / 2);
  if (!skill_success(ch, SKILL_TURN, 0) || need_mana > GET_MANA(ch)) {
    stc("You fail to summon enough energy to dispel the darkness!\r\n", ch);
    GET_MANA(ch) -= (need_mana / 4);
    GET_MANA(ch) = MAX(0, GET_MANA(ch));
    return;
  }
  
  GET_MANA(ch) -= need_mana;
  improve_skill(ch, SKILL_TURN);

  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room; 
    diff = 0;

    if (GET_RACE(tch) == RACE_UNDEAD || GET_RACE(tch) == RACE_VAMPIRE ||
        GET_RACE(tch) == RACE_GHOST) {

      if (!IS_NPC(tch))
        continue;

      diff = GET_LEVEL(ch) - GET_LEVEL(tch);

      if (diff <= -5) {
        act("$n shivers uncomfortably.", FALSE, tch, 0, 0, TO_ROOM);
        act("A disturbing feeling washes over your body.", FALSE, tch, 0, 0, TO_CHAR);
        hit(ch, tch, TYPE_UNDEFINED);
        continue;
      }

      if (diff >= 30) {  /* destroy! */
        act("$n grimaces and then explodes into a cloud of dust!", FALSE, tch, 0, 0, TO_ROOM);
        act("You feel your body twist horribly and disintegrate into nothing!", FALSE, tch, 0, 0, TO_CHAR);
        raw_kill(tch, ch, SPELL_DISINTEGRATE);
        continue;
      }

      if (diff > 3) { /* flee em */
        act("$n shrieks in terror!", FALSE, tch, 0, 0, TO_ROOM);
        act("You are suddenly terrified!", FALSE, tch, 0, 0, TO_CHAR);
        do_flee(tch, "", 0, 0);
        continue;
      }

      act("$N appears unaffected by your skill.", TRUE, ch, 0, tch, TO_CHAR);
      hit(ch, tch, TYPE_UNDEFINED);
    }
  }
}

ACMD(do_mindlink)
{
  struct char_data *vict = NULL;
  int life = 0;

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    mudlog("SYSERR: do_mindlink called without a victim.", BRF, LVL_IMMORT, TRUE);
    return;
  } else {
    if (!IS_NPC(vict)) {
      act("$N stares at you blankly.", FALSE, ch, 0, vict, TO_CHAR);
      act("$n stares at $N for a while and then falls flat on $s face.", FALSE, ch, 0, vict, TO_ROOM);
      stc("You fail.\r\n", ch);
      return;
    }

    if (FIGHTING(vict)) {
      stc("There's too much going on to establish a mind link.\r\n", ch);
      return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) || ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
      send_to_char("Something in the room is disrupting your concentration.\r\n", ch);
      return;
    }

    if (GET_HIT(ch) < 100) {
      send_to_char("You are too weak to initiate the link.\r\n", ch);
      return;
    }

    if (GET_HIT(vict) < 100) {
      act("$N is too weak to sustain the link.", TRUE, ch, 0, vict, TO_CHAR);
      return;
    }

    if (skill_success(ch, SKILL_MINDLINK, 0)) {
      life = number((20 + (GET_LEVEL(ch) / 3)), 100);
      GET_MANA(ch) += life;
      GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
      send_to_char("A wealth of power overcomes you!\r\n", ch);
      act("$n and $N stare at each other for a while and drop to the ground in unison!", FALSE, ch, 0, vict, TO_ROOM);
      improve_skill(ch, SKILL_MINDLINK);
      GET_POS(vict) = POS_STUNNED;
      damage(ch, vict, life, SKILL_MINDLINK);
    } else {
      act("$n stares at $N for a while and then falls flat on $s face.", FALSE, ch, 0, vict, TO_ROOM);
      stc("Your link fails and you are overcome by an intense wave of pain.\r\n", ch);
      GET_HIT(ch) -= 100;
    }
    send_to_char("You feel a little drained...\r\n", ch);
    GET_POS(ch) = POS_STUNNED;
    update_pos(ch);
  }
}

ACMD(do_disarm)
{   
  struct char_data *vict;
  struct obj_data *obj;
  
  one_argument(argument, arg);
    
  if (!(vict = get_char_room_vis(ch, arg)))
    vict = FIGHTING(ch);

  if (!(obj = GET_EQ(vict, WEAR_WIELD))) {
    act("$E doesn't have anything to disarm.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
        
  if (!FIGHTING(ch) || vict != FIGHTING(ch)) {
    stc("You can't disarm them if you aren't fighting them!\r\n", ch);
    return;
  }   

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_MOVE(ch) < find_moves(SKILL_DISARM)) {
      send_to_char("You don't have the energy to do that!\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= find_moves(SKILL_DISARM);
  }

  if (skill_success(ch, SKILL_DISARM, 0)) {
    act("You disarm $N and $p goes flying!", 0, ch, obj, vict, TO_CHAR);
    act("$n knocks $p from $N's hand!", TRUE, ch, obj, vict, TO_NOTVICT);
    act("$n deftly disarms you, knocking $p from your hand!", TRUE, ch, obj, vict, TO_VICT);
    obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
    improve_skill(ch, SKILL_DISARM);
  } else {
    act("You try to disarm $N but fail, tumbling to the ground in the process!", FALSE, ch, 0, vict, TO_CHAR);
    act("$n tries to disarm you but fails and falls flat on $s face instead!", TRUE, ch, 0, vict, TO_VICT);
    act("$n tries to disarm $N, but fails and falls flat on $s face!", TRUE, ch, 0, vict, TO_NOTVICT);
    GET_POS(ch) = POS_SITTING;
  }
  WAIT_STATE(ch, 2);
  COMBAT_WAIT(ch, 6);

  if (!FIGHTING(vict))
    hit(vict, ch, TYPE_UNDEFINED);
}
