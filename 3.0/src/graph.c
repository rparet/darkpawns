/* ************************************************************************
*   File: graph.c                                       Part of CircleMUD *
*  Usage: various graph algorithms                                        *
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

/* $Id: graph.c 1242 2005-11-07 11:47:28Z mcooper $ */

#define TRACK_THROUGH_DOORS

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"


/* Externals */
extern const int rev_dir[];
extern const char *dirs[];
extern void improve_skill(struct char_data *ch, int skill_num);    

struct bfs_queue_struct {
  long room;
  char dir;
  struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = 0, *queue_tail = 0;

ACMD(do_follow);
ACMD(do_tell);
ACMD(do_gen_comm);
bool is_intelligent(struct char_data *ch);
bool can_speak(struct char_data *ch);
struct char_data *HUNTING(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);

/* Utility macros */
#define MARK(room)	(SET_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define UNMARK(room)	(REMOVE_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define IS_MARKED(room)	(ROOM_FLAGGED(room, ROOM_BFS_MARK))
#define TOROOM(x, y)	(world[(x)].dir_option[(y)]->to_room)
#define IS_CLOSED(x, y)	(EXIT_FLAGGED(world[(x)].dir_option[(y)], EX_CLOSED))
#define SECT_TYPE(x, y) (SECT(world[(x)].dir_option[(y)]->to_room))

#ifdef TRACK_THROUGH_DOORS
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK)) && \
	                  (SECT_TYPE(x, y) != SECT_WATER_SWIM) && \
                          (SECT_TYPE(x, y) != SECT_WATER_NOSWIM) && \
        		  (!IS_MARKED(TOROOM(x, y))))
#else
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!IS_CLOSED(x, y)) &&		\
			  (!ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK)) && \
			  (!IS_MARKED(TOROOM(x, y))))
#endif

void bfs_enqueue(long room, int dir)
{
  struct bfs_queue_struct *curr;

  CREATE(curr, struct bfs_queue_struct, 1);
  curr->room = room;
  curr->dir = dir;
  curr->next = 0;

  if (queue_tail) {
    queue_tail->next = curr;
    queue_tail = curr;
  } else
    queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
  struct bfs_queue_struct *curr;

  curr = queue_head;

  if (!(queue_head = queue_head->next))
    queue_tail = 0;
  FREE(curr);
}


void bfs_clear_queue(void)
{
  while (queue_head)
    bfs_dequeue();
}


/* find_first_step: given a source room and a target room, find the first
   step on the shortest path from the source to the target.

   Intended usage: in mobile_activity, give a mob a dir to go if they're
   tracking another mob or a PC.  Or, a 'track' skill for PCs.
*/

int find_first_step(long src, long target)
{
  int curr_dir;
  long curr_room;

  if (src < 0 || src > top_of_world || target < 0 || target > top_of_world) {
    log("SYSERR: Illegal value %ld or %ld passed to find_first_step. (%s)", src, target, __FILE__);
    return BFS_ERROR;
  }
  if (src == target)
    return BFS_ALREADY_THERE;

  /* clear marks first */
  for (curr_room = 0; curr_room <= top_of_world; curr_room++)
    UNMARK(curr_room);

  MARK(src);

  /* first, enqueue the first steps, saving which direction we're going. */
  for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
    if (VALID_EDGE(src, curr_dir)) {
      MARK(TOROOM(src, curr_dir));
      bfs_enqueue(TOROOM(src, curr_dir), curr_dir);
    }
  /* now, do the classic BFS. */
  while (queue_head) {
    if (queue_head->room == target) {
      curr_dir = queue_head->dir;
      bfs_clear_queue();
      return curr_dir;
    } else {
      for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
	if (VALID_EDGE(queue_head->room, curr_dir)) {
	  MARK(TOROOM(queue_head->room, curr_dir));
	  bfs_enqueue(TOROOM(queue_head->room, curr_dir), queue_head->dir);
	}
      bfs_dequeue();
    }
  }

  return BFS_NO_PATH;
}


/************************************************************************
*  Functions and Commands which use the above fns		        *
************************************************************************/
ACMD(do_track)
{
  struct char_data *vict;
  int dir, tries = 20, modify = 0;  /* tries prevents infinite loop */
  long room_vnum;
  long sql_find_mob(char *searchname);

  one_argument(argument, arg);
  if (!(vict = get_char_vis(ch, arg))) {   /* First check - are they in the world? */
    if (!(room_vnum = sql_find_mob(arg))) {
      send_to_char("Who are you trying to track?\r\n", ch);
      return;
    } else {
      if (!ZONE_FLAGGED(world[real_room(room_vnum)].zone, ZONE_UNLOAD))
        reload_room_descs(world[real_room(room_vnum)].zone);
      if (!(vict = get_char_vis(ch, arg))) {            /* Still can't find them?!? */
        send_to_char("Who are you trying to track?\r\n", ch);
        return;
      }
    }
  }

  if (IS_AFFECTED(vict, AFF_NOTRACK) || MOB_FLAGGED(vict, MOB_SENTINEL)) {
    send_to_char("You sense no trail.\r\n", ch);
    return;
  }

  if (!IS_NPC(vict) && GET_SKILL(vict, SKILL_EVASION)) 
    if (number(1, 150) <= GET_SKILL(vict, SKILL_EVASION)) {
      send_to_char("You sense no trail.\r\n", ch);
      return;
    }

  dir = find_first_step(ch->in_room, vict->in_room);

  switch (dir) {
    case BFS_ERROR:
      send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
      break;
    case BFS_ALREADY_THERE:
      send_to_char("You're already in the same room!!\r\n", ch);
      break;
    case BFS_NO_PATH:
      sprintf(buf, "You can't sense a trail to %s from here.\r\n", HMHR(vict));
      send_to_char(buf, ch);
      break;
    default:
      if (OUTSIDE(ch) && weather_info.sky == SKY_RAINING)
        modify = 35;
      if (!skill_success(ch, SKILL_TRACK, modify)) {
        do {
	        dir = number(0, NUM_OF_DIRS - 1);
        } while (!CAN_GO(ch, dir) && --tries); 
      } else
        improve_skill(ch, SKILL_TRACK); 
  
      if (CAN_GO(ch, dir))
        sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
      else
        sprintf(buf, "There doesn't seem to be any way out of here!\r\n");

      send_to_char(buf, ch);
      break;
  }
}

void hunt_victim(struct char_data * ch)
{
  ACMD(do_say);
  extern struct char_data *character_list;

  int dir;
  int found = 0;
  struct char_data *tmp;

  if (!ch)
    return;

  if ((ch)->char_specials.hunting_id == -1) { /* return to origin hack */ 
    found = 2;
    sprintf(buf, "%s returning to origin.", GET_NAME(ch));
    mudlog(buf, CMP, LVL_IMMORT, FALSE);
  }
  
  if (!ch || !HUNTING(ch) || HUNTING(ch)->in_room < 2) 
    if (!found)
      return;

  /* make sure the char still exists */
  for (tmp = character_list; tmp && !found; tmp = tmp->next)
    if (HUNTING(ch) == tmp)
      found = 1;

  if (!found) {
    if (can_speak(ch))
      do_say(ch, "Damn!  My prey is gone!!", 0, 0);
    set_hunting(ch, NULL);
    return;
  }

  /* find first step in path */
  if (found == 1)
    dir = find_first_step(ch->in_room, HUNTING(ch)->in_room);
  else 
    dir = find_first_step(ch->in_room, real_room(GET_LOADED_IN(ch)));
    
  if (found == 1) {
    if (GET_SKILL(HUNTING(ch), SKILL_EVASION) && !(dir == BFS_ALREADY_THERE)) {
       if (number(1, 151) < GET_SKILL(HUNTING(ch), SKILL_EVASION)) {
         if (can_speak(ch) && !number(0, 6))
           do_say(ch, "Where the hell did my prey go?!", 0, 0);
         else if (can_speak(ch) && !number(0, 6))
           do_say(ch, "Fuck this...", 0, 0);
         return;
       }
    }

    /* Serapis 141303ZMAY97.. Don't hunt if char is in a safe room or a city */
    if (ROOM_FLAGGED((HUNTING(ch))->in_room, ROOM_PEACEFUL) ||
        ROOM_FLAGGED((HUNTING(ch))->in_room, ROOM_HOUSE) ||
        ZONE_FLAGGED(world[(HUNTING(ch))->in_room].zone, ZONE_LAW))
      return;
  }

  if (dir == BFS_ALREADY_THERE && found == 2) {
    (ch)->char_specials.hunting_id = 0;  /* turn off return to origin */
    sprintf(buf, "%s succesfully returned to origin.", GET_NAME(ch));
    mudlog(buf, CMP, LVL_IMMORT, FALSE);
    return;
  }

  if (dir < 0) {
    if(dir == BFS_ALREADY_THERE && found == 1) {
      if (ch && ch->in_room != NOWHERE && ch->in_room == HUNTING(ch)->in_room) {
        do_follow(ch, GET_NAME(HUNTING(ch)), 0, 0);
        hit(ch, HUNTING(ch), TYPE_UNDEFINED);
        return;
      }
    }
    if (found == 2) {  /* at this point, it must be an error */
      (ch)->char_specials.hunting_id = 0;  /* turn off return to origin */
      if (ch->in_room > 0)
        GET_LOADED_IN(ch) = GET_ROOM_VNUM(ch->in_room);
      sprintf(buf, "-E- %s : No path to origin or BFS Error", GET_NAME(ch));
      mudlog(buf, CMP, LVL_IMMORT, FALSE);
      return;
    }
    (ch)->char_specials.hunting = NULL;
  } else {
    if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) && is_intelligent(ch)) 
      if (EXIT(ch, dir)->keyword) {
        char mybuf[20];
        char mybuf2[80];
        ACMD(do_gen_door);

        switch (rev_dir[dir]) {
          case 0: sprintf(mybuf, "south"); break;
          case 1: sprintf(mybuf, "west"); break;
          case 2: sprintf(mybuf, "north"); break;
          case 3: sprintf(mybuf, "east"); break;
          case 4: sprintf(mybuf, "down"); break;
          case 5: sprintf(mybuf, "up"); break;
          default: strcpy(mybuf, "\0");
        }
        sprintf(mybuf2, "%s %s", EXIT(ch, dir)->keyword, mybuf);
        do_gen_door(ch, mybuf2, 0, SCMD_OPEN);
      }
    if (!ZONE_FLAGGED(world[EXIT(ch, dir)->to_room].zone, ZONE_UNLOAD))
      reload_room_descs(world[EXIT(ch, dir)->to_room].zone);

    perform_move(ch, dir, 1);
    if (ch && ch->in_room != NOWHERE && HUNTING(ch) && ch->in_room == HUNTING(ch)->in_room && found == 1) {
      do_follow(ch, GET_NAME(HUNTING(ch)), 0, 0);
      hit(ch, HUNTING(ch), TYPE_UNDEFINED);
    } else if (can_speak(ch) && found == 1) {
      char msg[MAX_STRING_LENGTH];
      switch (number(0,250)) {
        case 0: sprintf(msg, "%s Let's have an ass-kicking contest", GET_NAME(HUNTING(ch)));
          do_tell (ch, msg, 0, 0);
          break;
        case 1: sprintf(msg, "Corpse of %s for sale in a minute.. %d coins.",  GET_NAME(HUNTING(ch)), number(1000, 4000));
          do_gen_comm(ch, msg, 0, SCMD_AUCTION);
          break;
        case 2: sprintf(msg, "%s Run to your momma, pansy!", GET_NAME(HUNTING(ch)));
          do_tell (ch, msg, 0, 0);
          break;
        case 3: sprintf(msg, "%s I'm coming to kill you!", GET_NAME(HUNTING(ch)));
          do_tell (ch, msg, 0, 0);
          break;
        case 4: sprintf(msg, "I hear %s thinks %s's bad.", GET_NAME(HUNTING(ch)), HSSH(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        case 5: sprintf(msg, "Your momma ain't gonna save you this time, %s.", GET_NAME(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        case 6: sprintf(msg, "%s flees like a rabbit...",GET_NAME(HUNTING(ch)));
          if(!number(0,20))
            do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        case 7: sprintf(msg, "%s Come out and fight!", GET_NAME(HUNTING(ch)));
          do_tell (ch, msg, 0, 0);
          break;
        case 8: sprintf(msg, "%s Watch out! Here I come to get you!", GET_NAME(HUNTING(ch)));
          do_tell(ch, msg, 0, 0);
          break;
        case 9: sprintf(msg, "How much will I get for the head of %s?", GET_NAME(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_AUCTION);
          break;
        case 10: sprintf(msg, "Where is that little wimp, %s?", GET_NAME(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        case 11: sprintf(msg, "%s, you jerk!", GET_NAME(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        case 12: sprintf(msg, "Damn it!");
          do_gen_comm(ch, msg, 0, SCMD_SHOUT);
          break;
        case 13:
          if (HUNTING(ch)->char_specials.hunted_obj) {
            sprintf(msg, "Return %s or face the consequences %s!",
              HUNTING(ch)->char_specials.hunted_obj->short_description, GET_NAME(HUNTING(ch)));
            do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          }
          break;
        case 14: sprintf(msg, "Prepare yourself %s, I come for you!", GET_NAME(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        case 15:
          if (HUNTING(ch)->char_specials.hunted_obj) {
            sprintf(msg, "%s Thief! Return %s immediately!",
              GET_NAME(HUNTING(ch)), HUNTING(ch)->char_specials.hunted_obj->short_description);
            do_tell(ch, msg, 0, 0);
          }
          break;
        case 16: sprintf(msg, "I shall enjoy watching you die, %s!", GET_NAME(HUNTING(ch)));
          do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
          break;
        default: break;
      }
    }
    return;
  }
}
