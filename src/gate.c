/* ==========================================================================
   FILE   : gate.c - moongate code
   HISTORY: dkarnes 970114 started for Dark Pawns
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

/* $Id: gate.c 1487 2008-05-22 01:36:10Z jravn $ */

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

/* extern variables */
extern struct time_info_data time_info;
extern struct room_data *world;
extern struct index_data *obj_index;
extern int mini_mud;

/* extern functions */
struct char_data *get_mount(struct char_data *ch);
struct char_data *get_rider(struct char_data *mount);
void raw_kill(struct char_data * ch, int attacktype);

/*
  There are 8 moongates... rooms  4001-4008.
  In these eight rooms, blue moon gates will appear
  (during the night only) based on the phase of the moon.
  When a moongate appears, this should be the message:

  A shimmering portal of blue light suddenly appears in the middle of the room.
  ...and...
  The shimmering blue portal fades out of existence.

  Here is the cycles of when moongates are in...
  Room #    Moon Cycle during which the gate exists (night only)
  4001 ---- New Moon
  4002 ---- 1/4 Waxing
  4003 ---- 1/2 Waxing
  4004 ---- 3/4 Waxing
  4005 ---- Full Moon
  4006 ---- 3/4 Waning
  4007 ---- 1/2 Waning
  4008 ---- 1/4 Waning

  When a person enters a moongate, there is a 50% chance of which place they
  will be teleported too (each moongate has 2 different exits)
  Gate room - Exit Rooms
  4001 --- 4004 & 4006
  4002 --- 4005 & 4007
  4003 --- 4006 & 4008
  4004 --- 4001 & 4007
  4005 --- 4002 & 4008
  4006 --- 4001 & 4003
  4007 --- 4002 & 4004
  4008 --- 4003 & 4005

  Ok, now that we have the 8 blue moongates based on the moon cycles,
  there are 8 blue moon gates that are prermanently in place, regardless of
  time or moon phase.
  These gates are in the following places with the following exits
  Gate - Exit
  Room - Room
  4011 - 4001
  4012 - 4002
  4013 - 4003
  4014 - 4004
  4015 - 4005
  4016 - 4006
  4017 - 4007
  4018 - 4008

  In addition to this, there is a spell called gate.
  Gate may only may be cast in rooms 4001-4008
  Gate creates a red moongate in the room that goes to room 4000.
  this should last for like 2 ticks, maybe 3

  BTW - the 8 gates in rooms 4001-4008 are the gates that will be spread
  around the world in various places.
  the 8 gates in rooms 4011-4018 are in "the void" or basically in the middle
  of nowhere. :P


  spells.h:
  -------------
  + #define blue_portal 4001
  + #define red_portal  4002
  -------------


  limits.c:point_update():
  -------------
      next_thing = j->next;

  +
  +    if (GET_OBJ_VNUM(j) == red_portal)
  +	{
  +
  +	  if (GET_OBJ_TIMER(j) > 0)
  +	    GET_OBJ_TIMER(j)--;

  +	    if (!GET_OBJ_TIMER(j))
  +	      {
  +		if ((j->in_room != NOWHERE) && (world[j->in_room].people))
  +		  send_to_room("The shimmering red portal of light "
  +    		               "fades out of existence.\r\n", j->in_room);
  +		extract_obj(j);
  +	      }
  +	}

  !    else if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3))
	{
  -------------




  */

/* room gate appears in, phase it appears in(-1=always), room(s) it exits to */
#define gate_load_room  0
#define gate_load_phase 1
#define gate_exit_room  2
#define gate_exit_room2 3
#define NUM_PARTS       4
const int gate_phase[][NUM_PARTS] =
{
  {4001, MOON_NEW,          4004, 4006},/* 0 */
  {4002, MOON_QUARTER_FULL, 4005, 4007},/* 1 */
  {4003, MOON_HALF_FULL,    4006, 4008},/* 2 */
  {4004, MOON_THREE_FULL,   4001, 4007},/* 3 */
  {4005, MOON_FULL,         4002, 4008},/* 4 */
  {4006, MOON_QUARTER_EMPTY,4001, 4003},/* 5 */
  {4007, MOON_HALF_EMPTY,   4002, 4004},/* 6 */
  {4008, MOON_THREE_EMPTY,  4003, 4005},/* 7 */
  {4011, -1,                4001, 0},/* 8 */
  {4012, -1,                4002, 0},/* 9 */
  {4013, -1,                4003, 0},/* 10 */
  {4014, -1,                4004, 0},/* 11 */
  {4015, -1,                4005, 0},/* 12 */
  {4016, -1,                4006, 0},/* 13 */
  {4017, -1,                4007, 0},/* 14 */
  {4018, -1,                4008, 0} /* 15 */
};

#define NUM_GATES 16


/* **************************************************************************
   NAME       : load_night_gate()
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970114
   OTHER      :
   ************************************************************************ */
void
load_night_gate( void )
{
  int count;

  if (mini_mud)
	return;
  for (count = 0; count < NUM_GATES; count++)
    if ( time_info.moon == gate_phase[count][gate_load_phase] )
      {
	struct obj_data *gate = read_object(blue_portal, VIRTUAL);
	obj_to_room (gate, real_room(gate_phase[count][gate_load_room]));
	send_to_room("A shimmering portal of blue light suddenly appears in "
		     "the darkness!\r\n",
		     real_room(gate_phase[count][gate_load_room]));
      }
}


/* **************************************************************************
   NAME       : remove_night_gate()
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970114
   OTHER      :
   ************************************************************************ */
void
remove_night_gate( void )
{
  int count;

  if (mini_mud)
	return;
  for (count = 0; count < NUM_GATES; count++)
    {
      int gate_room = real_room(gate_phase[count][gate_load_room]);
      struct obj_data *gate = NULL, *next_obj = NULL;
      for (gate = world[gate_room].contents; gate; gate = next_obj)
	{
	  int virtual = GET_OBJ_VNUM(gate);
	  next_obj = gate->next_content;
	  if (virtual == blue_portal &&
	      gate_phase[count][gate_load_phase] != -1)
	    {
	      if (gate)
		extract_obj(gate);
	      send_to_room("The shimmering blue portal of light "
			   "fades out of existence.\r\n", gate_room);
	    }
	}
    }
}


/* **************************************************************************
   NAME       : moon_gate()
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970114
   OTHER      :
   ************************************************************************ */
SPECIAL(moon_gate)
{
  struct obj_data *obj = (struct obj_data *)me;
  int portal_room = 0, count = 0, i = 0;
  ACMD(do_look);

  if (!ch || mini_mud)
    return (FALSE);

  if (!CMD_IS("enter")&&!CMD_IS("look"))
    return FALSE;

  if (CMD_IS("enter"))
    {
      skip_spaces (&argument);

      if (!isname(argument, obj->name))
	 return(FALSE);

      if (GET_OBJ_VNUM(obj) == blue_portal)
	{
	  for (count = 0; count < NUM_GATES; count++)
	    if (real_room(gate_phase[count][gate_load_room]) == ch->in_room)
	      {
		int random_room=(number(0,3)<2)?gate_exit_room:gate_exit_room2;
		if (gate_phase[count][random_room] == 0)
		  random_room = gate_exit_room;
		portal_room = real_room(gate_phase[count][random_room]);
		break;
	      }
	}
      else if (GET_OBJ_COST(obj) > 0)
        portal_room = real_room(GET_OBJ_COST(obj));
      else /* red portal */
	portal_room = real_room(4000);

      send_to_char("You enter the portal and are transferred...\n\r",ch);
      act("$n enters the gate which suddenly starts glowing brightly,\r\n"
	  "obscuring your view of $m!", TRUE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, portal_room);
      act("With a sound like a raging river, a hole in space opens up "
		"and $n steps out.", TRUE, ch, 0, 0, TO_ROOM);
      if (!IS_NPC(ch) && IS_MOUNTED(ch))
	{
	  struct char_data *mount = get_mount(ch);
	  if (mount)
	    {
	      char_from_room(mount);
	      char_to_room(mount, portal_room);
	    }
	}
      if (IS_NPC(ch) && IS_MOUNTED(ch))
	{
	  struct char_data *rider = get_rider(ch);
	  if (rider)
	    {
	      char_from_room(rider);
	      char_to_room(rider, portal_room);
	      look_at_room(rider, TRUE);
	    }
	}
      look_at_room(ch, TRUE);
      return(TRUE);
    }
  else
    { /* look*/
      char temp_arg[MAX_STRING_LENGTH];
      skip_spaces(&argument);
      /* change to lowercase */
      strcpy(temp_arg, argument);
      for (i = 0; temp_arg[i]; i++)
	temp_arg[i] = LOWER(temp_arg[i]);
      if (strcmp(temp_arg, "gate") &&
	  strcmp(temp_arg, "moongate") &&
	  strcmp(temp_arg, "portal"))
	{
	  do_look(ch, temp_arg, 0, 0);
	  return(TRUE);
	}
      send_to_char("You see a shimmering light... you could ENTER "
		   "the gate.\r\n", ch);
    }
  return FALSE;
}


/* **************************************************************************
   NAME       : spell_gate()
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970114
   OTHER      :
   ************************************************************************ */
ASPELL(spell_gate)
{
  int legal_red_rooms[] = { 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008 };
  int num_legal_rooms = 8;
  int count = 0, loaded = FALSE;
  struct obj_data *i;
  struct obj_data *red_gate = read_object(red_portal, VIRTUAL);
  struct obj_data *blue_gate = read_object(blue_portal, VIRTUAL);

  if (mini_mud)
	return;

  for (i = world[ch->in_room].contents; i; i = i->next_content)
  {
     if ((GET_OBJ_VNUM(i) == GET_OBJ_VNUM(red_gate)) ||
         (GET_OBJ_VNUM(i) == GET_OBJ_VNUM(blue_gate)) ) {
       send_to_char("The magick flows through you, then out into the "
                    "world, changing it....\r\n", ch);
       send_to_room("As you watch the red portal slowly fade into "
                    "existence, \r\n"
                    "the existing portal pulses once, then begins to "
                    "expand, \r\n"
                    "consuming the new portal, and then the entire grove."
                    "\r\nThe fabric of time and space warps and stretches"
                    "\r\naround you, and flashes of white light explode "
                    "in the back of your mind.\r\n", ch->in_room);
       send_to_char("In your final moments, the only thing you can "
                    "feel is a\r\nwave of cosmic energy coursing "
                    "through you, tearing your soul to shreds.\r\n", ch);
       raw_kill(ch, TYPE_BLAST);
       extract_obj(i);
       return;
    }
  }
  for (count = 0; count < num_legal_rooms; count++)
    if (real_room(legal_red_rooms[count]) == ch->in_room)
      {
	obj_to_room (red_gate, ch->in_room);
	send_to_char("The magick flows through you, then out into the "
		     "world, changing it....\r\n", ch);
	send_to_room("A shimmering red portal fades into existence.\r\n",
		     ch->in_room);
	GET_OBJ_VAL(red_gate, 2) = 2+(GET_LEVEL(ch)==30);/* 2+1 */
	loaded = TRUE;
	break;
      }
  if (!loaded)
    {
      extract_obj(red_gate);
      extract_obj(blue_gate);
      if (!(ch->in_room == NOWHERE))
        stc("The magic flows through you, but nothing else "
		   "happens.\r\n", ch);
    }
}






