/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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

/* $Id: act.movement.c 1487 2008-05-22 01:36:10Z jravn $ */

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
#include "scripts.h"

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct index_data *mob_index;
extern int rev_dir[];
extern char *dirs[];
extern int movement_loss[];

/* external functs */
int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);

/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data *obj;
  int i;

  if (GET_LEVEL(ch)>=LEVEL_IMMORT)
    return 1;

  if (IS_AFFECTED(ch, AFF_WATERWALK))
    return 1;

  if (IS_AFFECTED(ch, AFF_FLY))
    return 1;

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return 1;

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return 1;

  return 0;
}



/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */
int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  struct char_data *tch = NULL, *tch_next;
  int was_in, need_movement;
  char msg[MAX_STRING_LENGTH];
  char direct[10];

  void mp_greet(struct char_data *who, int room);
  void entry_prog(struct char_data *mob, int room);
  int special(struct char_data *ch, int cmd, char *arg);
  struct char_data *mount = (struct char_data *)NULL;
  struct char_data *get_mount(struct char_data *ch);
  struct char_data *get_rider(struct char_data *ch);
  int riding_mount(struct char_data *rider, struct char_data *mount);
  void unmount(struct char_data *rider, struct char_data *mount);

  /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
  if (need_specials_check && special(ch, dir + 1, ""))
    return 0;

  /* charmed? */
  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master
	&& ch->in_room == ch->master->in_room) {
    send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
    return 0;
  }

  /* if this room or the one we're going to needs a boat, check for one */
  if ((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) {
    if (!has_boat(ch)) {
      send_to_char("You need a boat to go there.\r\n", ch);
      return 0;
    }
  }

  /* move points needed is avg. move loss for src and destination sect type */
  need_movement = (movement_loss[SECT(ch->in_room)] +
		   movement_loss[SECT(EXIT(ch, dir)->to_room)]) >> 1;

  if (IS_MOUNTED(ch))
  {
	if ( (!IS_NPC(ch) && !get_mount(ch)) ||
	     (IS_NPC(ch)  && !get_rider(ch)) )
	{ /* this is an error, handle it ungracefully */
	 unmount(ch, NULL);
	 return 0;
	}
	if ( (IS_NPC(ch) && GET_POS(ch) < POS_STANDING) ||
	     (!IS_NPC(ch) && (GET_POS(get_mount(ch)) < POS_STANDING)) )
	{
	 if (IS_NPC(ch))
	  send_to_char("You're in no position to go ANYWHERE!\r\n", ch);
	 else
	  send_to_char("Your mount is in no position to go ANYWHERE!\r\n", ch);
	 return 0;
	}
	if (IS_NPC(ch) && GET_MOVE(ch) < need_movement)
	{
	 send_to_char("You are too exhausted to carry your rider further.\r\n",
		 ch);
	 send_to_char("Your mount is too exhausted to carry you further.\r\n",
		get_rider(ch));
	 return 0;
	}
	else if (!IS_NPC(ch) && GET_MOVE(get_mount(ch)) < need_movement)
	{
	 send_to_char("Your mount is too exhausted to carry you further.\r\n",
                ch);
	 send_to_char("You are too exhausted to carry your rider further.\r\n",
		get_mount(ch));
	 return 0;
	}
  }
  else if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) {
    if (need_specials_check && ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);
    return 0;
  }

  if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) >= 1) {
    send_to_char("There isn't enough room there!\r\n", ch);
    return 0;
  }

  if (IS_MOUNTED(ch) && IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room),
   ROOM_INDOORS)) {
     send_to_char("You can't ride in there! Dismount first!\r\n", ch);
     return 0;
  }

  was_in = ch->in_room;
  mount = get_mount(ch);
  if (mount)
    {
      if (!IS_AFFECTED(mount, AFF_SNEAK)) {
	sprintf(msg, "$N rides %s on $n.", dirs[dir]);
	act(msg, TRUE, mount, 0, ch, TO_NOTVICT);
      }
      char_from_room(mount);
      char_to_room(mount, world[was_in].dir_option[dir]->to_room);
      GET_MOVE(mount) -= need_movement;
    }
  else
    {
      if (!IS_AFFECTED(ch, AFF_SNEAK)) {
  	sprintf(buf2, "$n leaves %s.", dirs[dir]);
	act(buf2, TRUE, ch, 0, 0, TO_ROOM);
      }
      if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch))
	GET_MOVE(ch) -= need_movement;
    }
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);
  if (!IS_AFFECTED(ch, AFF_SNEAK))
    switch(dir)
      {
      case 0:
      case 1:
      case 2:
      case 3:
	if (dir == 0)
	  sprintf(direct, "south");
	else if (dir == 1)
	  sprintf(direct, "west");
	else if (dir == 2)
	  sprintf(direct, "north");
	else if (dir == 3)
	  sprintf(direct, "east");

	if (IS_AFFECTED(ch, AFF_FLY))
	  sprintf(msg,"$n flies in from the %s.", direct);
        else if (SECT(ch->in_room) == SECT_UNDERWATER)
          sprintf(msg, "$n swims in from the %s.", direct);
	else if (riding_mount(ch, mount))
	  sprintf(msg,"$n rides in from the %s on $N.", direct);
	else
	  sprintf(msg,"$n arrives from the %s.", direct);

	act(msg, TRUE, ch, 0, mount, TO_ROOM);

	break;
      case 4:
	if (IS_AFFECTED(ch, AFF_FLY))
	  act("$n flies in from below.", TRUE, ch, 0, 0, TO_ROOM);
        else if (SECT(ch->in_room) == SECT_UNDERWATER)
          act("$n swims in from below.", TRUE, ch, 0, 0, TO_ROOM);
	else if (riding_mount(ch, mount))
	  act("$n rides in from below on $N.", TRUE, ch, 0, mount, TO_ROOM);
	else
	  act("$n climbs in from below.", TRUE, ch, 0, 0, TO_ROOM);
	break;
      case 5:
	if (IS_AFFECTED(ch, AFF_FLY))
	  act("$n flies in from above.", TRUE, ch, 0, 0, TO_ROOM);
        else if (SECT(ch->in_room) == SECT_UNDERWATER)
          act("$n swims in from above.", TRUE, ch, 0, 0, TO_ROOM);
	else if (riding_mount(ch, mount))
	  act("$n rides in from above on $N.", TRUE, ch, 0, mount, TO_ROOM);
	else
	  act("$n climbs in from above.", TRUE, ch, 0, 0, TO_ROOM);
	break;
      }

  if (ch->desc != NULL)
    look_at_room(ch, 0);
  else
    entry_prog(ch, ch->in_room);

  if (ch->in_room == -1)/* ch died in the entry (?) */
    return(0);

  if (!IS_AFFECTED(ch, AFF_SNEAK)) {
    mp_greet(ch, ch->in_room);
    for (tch = world[ch->in_room].people; tch; tch = tch_next) {
      tch_next = tch->next_in_room;

      if (tch == ch)
        continue;

      if (IS_NPC(tch) && GET_MOB_SCRIPT(tch) && MOB_SCRIPT_FLAGGED(tch, MS_GREET))
        run_script(ch, tch, NULL, &world[tch->in_room], NULL, "greet", LT_MOB);
    }
  }

  if (ch->in_room == -1)/* ch died in the greet (?) */
    return(0);

  if ( (ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) &&
	(GET_LEVEL(ch) < LVL_IMMORT || IS_NPC(ch))  )
  {
    log_death_trap(ch);
    death_cry(ch);
    extract_char(ch);
    if (mount)
      {
	log_death_trap(mount);
	death_cry(mount);
	extract_char(mount);
      }
    return 0;
  }

  if (GET_ROOM_SCRIPT(ch->in_room) && ROOM_SCRIPT_FLAGGED(ch->in_room, RS_ENTER))
    run_script(ch, ch, NULL, &world[ch->in_room], NULL, "enter", LT_ROOM);

  return 1;
}


int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  struct char_data *get_rider(struct char_data *ch);
  int was_in;
  struct follow_type *k, *next;

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS)
    return 0;
  else if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
    send_to_char("Alas, you cannot go that way...\r\n", ch);
  else if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
    if (EXIT(ch, dir)->keyword) {
      if (strstr(EXIT(ch, dir)->keyword, "secret") &&
	  !ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK))
        send_to_char("Alas, you cannot go that way...\r\n", ch);
      else
      {
        sprintf(buf2, "The %s seems to be closed.\r\n",
	        fname(EXIT(ch, dir)->keyword));
        send_to_char(buf2, ch);
      }
    } else
      send_to_char("It seems to be closed.\r\n", ch);
  } else {
    if (!ch->followers)
      return (do_simple_move(ch, dir, need_specials_check));

    was_in = ch->in_room;
    if (!do_simple_move(ch, dir, need_specials_check))
      return 0;

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((k->follower->in_room == was_in) &&
	  (GET_POS(k->follower) >= POS_STANDING) &&
	  (get_rider(k->follower)!=ch) ) {
	act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
 	REMOVE_BIT_AR(AFF_FLAGS(k->follower), AFF_HIDE);
	perform_move(k->follower, dir, 1);
      }
    }
    return 1;
  }
  return 0;
}


ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, cmd - 1, 0);
}


int
find_door(struct char_data *ch, char *type, char *dir, char *cmdname)
{
  int door;

  if (*dir)
    {			/* a direction was specified */
      if ((door = search_block(dir, dirs, FALSE)) == -1)
	{	/* Partial Match */
	  send_to_char("That's not a direction.\r\n", ch);
	  return -1;
	}
      if (EXIT(ch, door) && (!EXIT(ch, door)->keyword  ||
			     (!strstr(EXIT(ch, door)->keyword, "secret") ||
	 		      ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK))))
	{
	  if (EXIT(ch, door)->keyword)
	    {
	      if (isname(type, EXIT(ch, door)->keyword))
		return door;
	      else
		{
		  sprintf(buf2, "I see no %s there.\r\n", type);
		  send_to_char(buf2, ch);
		  return -1;
		}
	    }
	  else
	    return door;
	}
      else
	{
	  stc("I really don't see how you can do anything there.\r\n", ch);
	  return -1;
	}
    }
  else
    {			/* try to locate the keyword */
      if (!*type)
	{
	  sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
	  send_to_char(buf2, ch);
	  return -1;
	}
      for (door = 0; door < NUM_OF_DIRS; door++)
	if (EXIT(ch, door))
	  if (EXIT(ch, door)->keyword)
	    if (isname(type, EXIT(ch, door)->keyword))
	      if (!strstr(EXIT(ch, door)->keyword, "secret") ||
	 	  ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK))
	        return door;

      sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
      send_to_char(buf2, ch);
      return -1;
    }
}


int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return 1;

  if (GET_EQ(ch, WEAR_HOLD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
      return 1;

  return 0;
}



#define NEED_OPEN	1
#define NEED_CLOSED	2
#define NEED_UNLOCKED	4
#define NEED_LOCKED	8

char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)		(world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  int other_room = 0;
  struct room_direction_data *back = 0;
  char doorname[256];

  sprintf(buf, "$n %ss ", cmd_door[scmd]);
  if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
    if ((back = world[other_room].dir_option[rev_dir[door]]))
      if (back->to_room != ch->in_room)
	back = 0;

  switch (scmd) {
  case SCMD_OPEN:
  case SCMD_CLOSE:
    OPEN_DOOR(ch->in_room, obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(OK, ch);
    break;
  case SCMD_UNLOCK:
  case SCMD_LOCK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("*Click*\r\n", ch);
    break;
  case SCMD_PICK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("The lock quickly yields to your skills.\r\n", ch);
    strcpy(buf, "$n skillfully picks the lock on ");
    break;
  }

  /* Notify the room */
  sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
	  (EXIT(ch, door)->keyword ? "$F" : "door"));
  if (!(obj) || (obj->in_room != NOWHERE))
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

  /* Notify the other room */
  if ((scmd == SCMD_OPEN || scmd == SCMD_CLOSE) && back)
  {
    sprintf(doorname, (back->keyword ? fname(back->keyword) : "door"));
    sprintf(buf, "The %s %s %s%s from the other side.\r\n",
	 doorname,
         (strrchr(doorname, 's') && strlen( strrchr(doorname, 's') ) == 1) ? "are" : "is",
         cmd_door[scmd],
	 (scmd == SCMD_CLOSE) ? "d" : "ed");
    if (world[EXIT(ch, door)->to_room].people) {
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
    }
  }
}


int ok_pick(struct char_data *ch, int keynum, int pickproof, int scmd)
{
  int percent, can_break = 0;
  struct obj_data *picks = GET_EQ(ch, WEAR_HOLD);

  percent = number(1, 101);

  if (scmd == SCMD_PICK) {
    if (keynum < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if ((!picks || GET_OBJ_VNUM(picks)!=8027)&& GET_LEVEL(ch)<LVL_IMMORT)
      stc("You'll need to hold a set of lockpicks before you can "
	  "pick a lock!\r\n", ch);
    else if (pickproof)
    {
      send_to_char("It resists your attempts to pick it.\r\n", ch);
      can_break = 2;
    }
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
    {
      send_to_char("You failed to pick the lock.\r\n", ch);
      can_break = 1;
    }
    else
      return (1);
    if (picks && can_break)
      if (GET_LEVEL(ch)<number(0, 30)+can_break)
      {
	act("$n curses as $e bends some of $s lockpicks.",
	    FALSE, ch, 0, 0, TO_ROOM);
	stc("You ruin your lockpicks in the process.\r\n", ch);
	/* replace good lockpicks with broken ones */
	extract_obj(unequip_char(ch, WEAR_HOLD));
	equip_char(ch, read_object(8028, VIRTUAL), WEAR_HOLD);
      }
    return (0);
  }
  return (1);
}


#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))) :\
			(IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF)) : \
			(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)))

#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
  int door = -1, keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
    send_to_char(CAP(buf), ch);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_OPEN))
      send_to_char("But it's already closed!\r\n", ch);
    else if (!DOOR_IS_CLOSED(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_CLOSED))
      send_to_char("But it's currently open!\r\n", ch);
    else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char("It seems to be locked.\r\n", ch);
    else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_GOD) &&
	     ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
      do_doorcmd(ch, obj, door, subcmd);
  }
  return;
}



ACMD(do_enter)
{
  int door;

  one_argument(argument, buf);

  if (*buf) {			/* an argument was supplied, search for door
				 * keyword */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    sprintf(buf2, "There is no %s here.\r\n", buf);
    send_to_char(buf2, ch);
  } else if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are already indoors.\r\n", ch);
  else {
    /* try to locate an entrance */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	      IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("You can't seem to find anything to enter.\r\n", ch);
  }
}


ACMD(do_leave)
{
  int door;

  if (!IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	    !IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{
  ACMD(do_dismount);

  switch (GET_POS(ch)) {
  case POS_STANDING:
    if (!IS_MOUNTED(ch))
      act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
    else
      do_dismount(ch, NULL, 0, 0);
  break;
  case POS_SITTING:
    act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_RESTING:
    act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and put your feet on the ground.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and puts $s feet on the ground.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}


ACMD(do_sit)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    if (!IS_MOUNTED(ch))
      {
	act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SITTING;
      }
    else
      act("You can't rest while mounted.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    if (!IS_MOUNTED(ch))
      {
	act("You sit down and rest your tired bones.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_RESTING;
      }
    else
      act("You can't rest while mounted.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SITTING:
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and stop to rest your tired bones.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_sleep)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    if (IS_MOUNTED(ch))
      {
	act("You can't rest while mounted.", FALSE, ch, 0, 0, TO_CHAR);
	break;
      }
    /* Fall through on purpose */
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    act("You stop floating around, and lie down to sleep.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  struct char_data *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("Maybe you should wake yourself up first.\r\n", ch);
    else if ((vict = get_char_room_vis(ch, arg)) == NULL)
      send_to_char(NOPERSON, ch);
    else if (vict == ch)
      self = 1;
    else if (GET_POS(vict) > POS_SLEEPING)
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (IS_AFFECTED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("$n wakes up $N.", FALSE, ch, 0, vict, TO_NOTVICT);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (IS_AFFECTED(ch, AFF_SLEEP)) {
    send_to_char("You can't wake up!\r\n", ch);
    act("$n tosses and turns uncomfortably.", TRUE, ch, 0, 0, TO_ROOM);
  }
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else {
    send_to_char("You awaken, and sit up.\r\n", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
  }
}


ACMD(do_follow)
{
  struct char_data *leader;
  int quiet = subcmd;

  void stop_follower(struct char_data *ch);
  void add_follower(struct char_data *ch, struct char_data *leader);
  void add_follower_quiet(struct char_data *ch, struct char_data *leader);

  one_argument(argument, buf);

  if (*buf) {
    if (!(leader = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Whom do you wish to follow?\r\n", ch);
    return;
  }

  if (ch->master == leader) {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master))
    act("But you only feel like following $N!",
	FALSE, ch, 0, ch->master, TO_CHAR);
  else {			/* Not Charmed follow person */
    if (leader == ch) {
      if (!ch->master) {
	send_to_char("You are already following yourself.\r\n", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
	act("Sorry, but following in loops is not allowed.",
		FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
      if (ch->master)
	stop_follower(ch);
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
      if (quiet && (GET_SKILL(ch, SKILL_SHADOW)>number(0,101) ||
	  GET_LEVEL(ch)>=LVL_IMMORT))
      {
	struct affected_type af;
  	if (IS_SHADOWING(ch))
  	{
    	  affect_from_char(ch, SKILL_SHADOW);
    	  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_DODGE);
  	}

  	af.type = SKILL_SHADOW;
  	af.duration = GET_LEVEL(ch);
  	af.modifier = 0;
  	af.location = APPLY_NONE;
  	af.bitvector = AFF_DODGE;
  	affect_to_char(ch, &af);

        act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
        add_follower_quiet(ch, leader);
      }
      else
        add_follower(ch, leader);
    }
  }
}
