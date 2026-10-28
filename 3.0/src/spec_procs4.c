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

/* CVS: $Id: spec_procs4.c 1330 2007-08-05 01:18:21Z jravn $ */

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

/*   external vars  */
extern int rev_dir[];
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);

/* extern functions */
int find_first_step(sh_int src, sh_int target);

/* ********************************************************************
*  Stonekeep (Zone 14)                                                *
********************************************************************* */

struct puzzle_struct {
  long room_in;				   /* What room is the char in? */
  int connect_to;	                   /* What room does the current room connect to? */
  int exit_from;			   /* What direction is the connection? */
  char *desc;				   /* Which door will open/close? */
} doors[] = {
{ 1415, 1417, SOUTH, "northern" },
{ 1418, 1417, WEST , "eastern"  },
{ 1420, 1417, NORTH, "southern" },
{ 1416, 1417, EAST , "western"  },
{ 1425, 1427, SOUTH, "northern" },
{ 1428, 1427, WEST , "eastern"  },
{ 1429, 1427, NORTH, "southern" },
{ 1426, 1427, EAST , "western"  },
{ 1430, 1432, SOUTH, "northern" },
{ 1433, 1432, WEST , "eastern"  },
{ 1434, 1432, NORTH, "southern" },
{ 1431, 1432, EAST , "western"  }
};

int init_done = 0;                  /* Has the zone been initialized? */
int button[12];                     /* Which button opens what door? */

#define EXIT_HERE(i) (world[real_room(doors[(i)].room_in)].dir_option[doors[(i)].exit_from])
#define EXIT_THERE(i)(world[real_room(doors[(i)].connect_to)].dir_option[rev_dir[doors[(i)].exit_from]])

/* Room defines */
#define NEST_ROOM       1401        /* Where is the nest which allows zone access? */
#define START_ROOM      1402        /* Where does the zone begin? */
#define CIRCLE_ROOM     1410        /* Where is the circle of empowering? */


void initialise_nest(void)
{
  /* Find a FOREST sector within the game and then connect the NEST_ROOM to it */

  int nest_room = real_room(NEST_ROOM);
  int connect_room = number(0, top_of_world);
  long room;

  for (room = 0; room <= top_of_world; room++) {    /* Remove any existing links */
    if (SECT(room) != SECT_FOREST)
      continue;
    if (!world[room].dir_option[UP])
      continue;
    if (world[room].dir_option[UP]->to_room == nest_room) {
      FREE(world[room].dir_option[UP]);
      world[room].dir_option[UP] = NULL;
    }
  }

  while (SECT(connect_room) != SECT_FOREST || world[connect_room].dir_option[UP])
    connect_room = number(0, top_of_world);

  CREATE(world[connect_room].dir_option[UP], struct room_direction_data, 1);
  world[connect_room].dir_option[UP]->to_room = nest_room;
  CREATE(world[nest_room].dir_option[DOWN], struct room_direction_data, 1);
  world[nest_room].dir_option[DOWN]->to_room = connect_room;
}

int initialise_buttons(void)
{
  /* Randomly assign a puzzle button with a puzzle door and then store these numbers
  on global variables. */

  int i, j, total, counter, num;
  int doors[12] =
    { 1415, 1418, 1420, 1416, 1425, 1428, 1429, 1426, 1430, 1433, 1434, 1431 };

  for (i = 0; i < 9; i+= 4)        /* Which random door to assign? */
    for (j = 0; j < 4; j++) {
      counter = 0;
      total = i + j;
      num = number(0 + i, 3 + i);
      while (total == num || doors[num] == 0) {
        counter++;
        num = number(0 + i,3 + i);
        if (counter > 200)		    /* Breaks an infinite loop */
          return (FALSE);
      }
      button[total] = doors[num];
      doors[num] = 0;
    }

  for (i = 0;;) {                   /* Can the puzzle be solved? */
    doors[i] = 1;
    switch (button[i]) {            /* Which button opens which door? */
      case 1415: i = 0; break;
      case 1418: i = 5; break;
      case 1420: i = 2; break;
      case 1416: i = 3; break;
      case 1425: i = 4; break;
      case 1428: i = 5; break;
      case 1429: i = 10; break;
      case 1426: i = 7; break;
      case 1430: i = 8; break;
      case 1433: i = 9; break;
      case 1434: i = 10; break;
      case 1431: return (TRUE);     /* Puzzle end, success */
    }
    if (doors[i])                   /* Been here before, puzzle not solvable */
      return (FALSE);
  }

  return (TRUE);
}

int maze_init(void)
{
  /* Given two levels of 10x10 rooms, design a random maze. */

  int i, dir, curr_room, connect_to, counter;
  int a, b, x, y, one_start = 1500, two_start = 1600;
  int maze[10][10];
  bool exists = FALSE, terminate = FALSE, repeat = FALSE;

  for (a = 0; a < 10; a++)                  /* Set up the room matrix */
    for (b = 0; b < 10; b++)
	  maze[a][b] = one_start++;

  for (a = 1500; a < 1700; a++) {              /* Make sure in-game rooms have no exits */
    TOGGLE_BIT(world[real_room(a)].room_flags, ROOM_NOTRACK);
    for (b = 0; b < 4; b++)
      if (world[real_room(a)].dir_option[b] != NULL) 
      {
        FREE(world[real_room(a)].dir_option[b]);
        world[real_room(a)].dir_option[b] = NULL;
      }
  }

  a = b = x = y = 0;
  while (!terminate) {
    curr_room = real_room(maze[a][b]);          /* Where am I currently? */
    dir = number(NORTH, WEST);                  /* Random direction */
    counter = 0;
    while (world[curr_room].dir_option[dir]) {  /* Does an exit already exist in this dir? */
      counter++;
      if (counter > 500)                        /* Breaks an infinite loop */
        return (FALSE);
      dir = number(NORTH, WEST);
    }

    switch(dir)
    {
      case NORTH: a--; break;
      case EAST: b++; break;
      case SOUTH: a++; break;
      case WEST: b--; break;
    }

    if (a < 0)                                  /* Move maze pointer as required */
      a = 9;
    else if (a > 9)
      a = 0;

    if (b < 0)
      b = 9;
    else if (b > 9)
      b = 0;

    connect_to = real_room(maze[a][b]);

    for (i = 0; i < 4; i++)
      if (world[connect_to].dir_option[i])
        exists = TRUE;

    CREATE(world[curr_room].dir_option[dir], struct room_direction_data,1);
    world[curr_room].dir_option[dir]->general_description =
      "You can't see that far into the darkness.\r\n";
    world[curr_room].dir_option[dir]->keyword = NULL;
    world[curr_room].dir_option[dir]->to_room = connect_to;
   
    CREATE(world[connect_to].dir_option[rev_dir[dir]], struct room_direction_data,1);
    world[connect_to].dir_option[rev_dir[dir]]->general_description =
      "You can't see that far into the darkness.\r\n";
    world[connect_to].dir_option[rev_dir[dir]]->keyword = NULL;
    world[connect_to].dir_option[rev_dir[dir]]->to_room = curr_room;

    while (exists) {            /* Exit exists...progress along matrix */
      y++;
      exists = FALSE;
      if (y > 9) {              /* Move to the next line in matrix */
        y = 0;
        x++;
      }
      if (x > 9) {              /* All done, finish loop */
        if (!repeat) {          /* Have we done second level of maze? */
          for (a = 0; a < 10; a++)                    /* Set up the room matrix */
            for (b = 0; b < 10; b++)
              maze[a][b] = two_start++;
		  repeat = TRUE;
		  a = b = x = y = 0;                  /* Reset variables */
        } else
          terminate = TRUE;
      } else
        for (i = 0; i < 4; i++)               /* Does the next room already have an exit? */
          if (world[real_room(maze[x][y])].dir_option[i])
            exists = TRUE;
      a = x;
      b = y;
    }
  }
  x = find_first_step(real_room(1513), real_room(1567));
  y = find_first_step(real_room(1667), real_room(1605));
  
  for (a = 1500; a < 1700; a++)               /* Reset the NOTRACK flag */
    TOGGLE_BIT(world[real_room(a)].room_flags, ROOM_NOTRACK);

  if (x == BFS_NO_PATH || y == BFS_NO_PATH)
    return (FALSE);
  else
    return (TRUE);
}

void stonekeep_init(void)
{
  /* Used to initialise the dynamic variables of the zone including the location of the
  entry point, the configuration of the keep puzzle and the design of the lower level
  mazes. */

  int rerun = FALSE;

  if (!init_done) {		        /* Default start, do initialisation sequence */
    initialise_nest();
    while (!rerun)				/* Keep going until values assigned */
      rerun = initialise_buttons();

    rerun = FALSE;
    while (!rerun)                              /* Keep going until a valid path exists */
      rerun = maze_init();

    sprintf(buf, "Initialisation of zone 14 is now complete.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    init_done = 1;
  }

  return;
}

SPECIAL(stonekeep_puzzle)
{
  int i, j, open_door = 0;

  if ((!cmd) || (!CMD_IS("push")))
    return (FALSE);

  skip_spaces(&argument);
  if ((!*argument) || (!strstr(argument, "button"))) {
    stc("What do you want to push?\r\n", ch);
    return (TRUE);
  }

  for (i = 0; i < 12; i++)                        /* Where am I? */
    if (GET_ROOM_VNUM(ch->in_room) == doors[i].room_in) {
      open_door = button[i];
      break;
    }

  for (j = 0; j < 12; j++)                        /* What door should I open? */
    if (doors[j].room_in == open_door)
      break;

  if (EXIT_FLAGGED(EXIT_HERE(i), EX_CLOSED)) {	  /* Is the panel already closed? */
    act("You press the button but nothing happens.", TRUE, ch, 0, 0, TO_CHAR);
    act("$n presses the button but nothing happens.", TRUE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  }

  SET_BIT(EXIT_HERE(i)->exit_info, EX_CLOSED);	  /* Close and lock the panel in front of me */
  SET_BIT(EXIT_HERE(i)->exit_info, EX_LOCKED);
  SET_BIT(EXIT_THERE(i)->exit_info, EX_CLOSED);	  /* Close and lock the panel on the other side */
  SET_BIT(EXIT_THERE(i)->exit_info, EX_LOCKED);
  act("You press the button and a panel closes in front of you with a thud.",
    TRUE, ch, 0, 0, TO_CHAR);
  act("$n presses the button and a panel closes in front of you with a thud.",
    TRUE, ch, 0, 0, TO_ROOM);

  REMOVE_BIT(EXIT_HERE(j)->exit_info, EX_LOCKED);   /* Open the appropriate panel */
  REMOVE_BIT(EXIT_HERE(j)->exit_info, EX_CLOSED);
  REMOVE_BIT(EXIT_THERE(j)->exit_info, EX_CLOSED);  /* Appropriate panel on other side */
  REMOVE_BIT(EXIT_THERE(j)->exit_info, EX_CLOSED);

  sprintf(buf, "A panel on the %s wall closes while another on the %s wall opens.\r\n",
    doors[i].desc, doors[j].desc);
  send_to_room(buf, real_room(doors[i].connect_to));  /* Send the messages */
 
  if (EXIT_FLAGGED(EXIT_HERE(j), EX_CLOSED))
    sprintf(buf, "A panel closes in front of you with a thud.\r\n");
  else
    sprintf(buf, "A panel opens in front of you.\r\n");
  
  send_to_room(buf, real_room(doors[j].room_in));
  return (TRUE);
}

SPECIAL(repairer)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *obj, *next_obj;
  extern char *craft_level[];
  int number = 0;
  ACMD(do_give);
  ACMD(do_tell);

  if (!cmd || !(CMD_IS("give") || CMD_IS("list")))
    return (FALSE);

  *buf = '\0';
  if (CMD_IS("list")) {
    for (obj = ch->carrying; obj; obj = obj->next_content)
      if (OBJ_FLAGGED(obj, ITEM_BROKEN)) {
        number++;
        sprintf(buf + strlen(buf), "\r\n   %-25s (%d)", obj->short_description, GET_OBJ_COST(obj) / 2);
      }

    if (number) {
      sprintf(buf1, "You have %d %s that appear broken. The cost to repair %s will be:%s",
        number, (number > 1 ? "items" : "item"), (number > 1 ? "them" :"it"), buf);
      act(buf1, FALSE, ch, 0, 0, TO_CHAR);
    } else {
      sprintf(buf1, "%s You don't appear to have any broken items.", GET_NAME(ch));
      do_tell(mobile, buf1, find_command("tell"), 0);
    }

    return (TRUE);
  }

  do_give(ch, argument, 0, 0);

  /* Can we find a broken object? */
  for (obj = mobile->carrying; obj; obj = next_obj) {
    next_obj = obj->next_content;
    if (!OBJ_FLAGGED(obj, ITEM_BROKEN)) {
      act("$n examines $p with a quizzical look.", TRUE, mobile, obj, 0, TO_ROOM);
      sprintf(buf1, "%s %s does not appear to be broken.", GET_NAME(ch), obj->short_description);
      do_tell(mobile, buf1, find_command("tell"), 0);
      one_argument(obj->name, arg);
      sprintf(buf, "%s to %s", arg, GET_NAME(ch));
      do_give(mobile, buf, 0, 0);
      continue;
    } else {
      /* Is it beyond repair? */
      if (!strcmp(craft_level[GET_CRAFT_VAL(obj)], "Broken")) {
        act("$n examines $p and shakes $s head.", TRUE, mobile, obj, 0, TO_ROOM);
        sprintf(buf1, "%s Unfortunately, %s %s beyond repair.",
          GET_NAME(ch), obj->short_description, PLURAL(obj->short_description) ? "are" : "is");
        do_tell(mobile, buf1, find_command("tell"), 0);
        one_argument(obj->name, arg);
        sprintf(buf, "%s to %s", arg, GET_NAME(ch));
        do_give(mobile, buf, 0, 0);
        continue;
      }
    }

    /* Not enough gold to pay for it! */
    if (GET_GOLD(ch) < (GET_OBJ_COST(obj) / 2)) {
      act("$n examines $p and shakes $s head.", TRUE, mobile, obj, 0, TO_ROOM);
      sprintf(buf1, "%s You don't have the %d gold coins I need for payment.", GET_NAME(ch), GET_OBJ_COST(obj) / 2);
      do_tell(mobile, buf1, find_command("tell"), 0);
      one_argument(obj->name, arg);
      sprintf(buf, "%s to %s", arg, GET_NAME(ch));
      do_give(mobile, buf, 0, 0);
      continue;
    }

    GET_CRAFT_VAL(obj)++;
    GET_GOLD(ch) -= (GET_OBJ_COST(obj) / 2);

    /* It's now broken for good */
    if (!strcmp(craft_level[GET_CRAFT_VAL(obj)], "Broken")) {
      act("$n sets to work on $p, but it is too damaged to repair.", TRUE, ch, obj, 0, TO_CHAR);
      one_argument(obj->name, arg);
      sprintf(buf, "%s to %s", arg, GET_NAME(ch));
      do_give(mobile, buf, 0, 0);
      continue;
    }

    /* All fixed! */
    REMOVE_BIT(GET_OBJ_EXTRA(obj), ITEM_BROKEN);
    act("$n works on $p for some time and is able to repair it.", TRUE, mobile, obj, 0, TO_ROOM);
    one_argument(obj->name, arg);
    sprintf(buf, "%s to %s", arg, GET_NAME(ch));
    do_give(mobile, buf, 0, 0);
  }

  return (TRUE);
}
