/**********************************************************
 *  dream.c -- dream code                                 *
 *                                                        *
 *  For Dark Pawns -- A CircleMUD derivative              *
 **********************************************************/

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

/* $Id: dream.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "dream.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"

#define NUM_DREAMS 8

const struct dtravel_data dtravel[] = {
  {0, 8004, "that shows you penitent before an altar."       },
  {0, 11111, "about lounging around a beautiful desert oasis."},
  {0, 20400, "in which you are lost in a dark, spooky forest."},
  {0, 4622 , "of tredging through a murky swamp."             }, 
  {0, 4805 , "about being lost in a foreign city."            },
  {0, 4267 , "of being surrounded by cold alpine peaks."      },
  {0, 14213, "in which the stench of death and battle overwhelm you."},
  {1, 12410, "of being hopelessly lost at sea."},
  {1, 12848, "of a dark figure standing over your corpse!"}
};

void dream(struct char_data *ch) {

  long lastdeath, curtime, diff;
  ACMD(do_wake);

  /* last death code would be here */
  lastdeath = GET_LAST_DEATH(ch);

  if (lastdeath != 0) { /* has died */
     if(IS_AFFECTED(ch, AFF_DREAM)) {
       dream_travel(ch, 1); /* bad dream travel subcmd */
       return;
     }
     curtime = (long)time(0);
     diff = curtime - lastdeath;
     if (diff < (24 * 60 * 60)) {  /* one real day */
       if (!number(0,5)) {
          send_to_char("You see the visions of your own death and wake up "
                       "screaming!\r\n", ch);
          act("$n wakes up screaming, with a look of death in $s eyes.",
              FALSE, ch, 0, 0, TO_ROOM);
          do_wake(ch, "", 0, 0);
       }
       return;
     } else if (diff < (2 * 24 * 60 * 60)) {  /* two real days */
       if (!number(0,5)) {
	 send_to_char("In your dreams you keep seeing a dark figure hunched "
		      "over your corpse.\r\n", ch);
	 act("$n shivers in $s sleep.", FALSE, ch, 0, 0, TO_ROOM);
       }
       return;
     } else if (diff < (3 * 24 * 60 * 60)) { /* three real days */
       if (!number(0,5)) {
	 send_to_char("You toss and turn as a dark cloud hovers over your "
		      "dreams.\r\n", ch);
	 act("$n tosses and turns in $s sleep, must be a bad dream.",
	     FALSE, ch, 0, 0, TO_ROOM);
       }
       return;
     } else if (diff < (5 * 24 * 60 *60)) { /* five real days */
       if (!number(0,5)) {
	 send_to_char("You sleep uneasily, as if something looms over your "
		       "past\r\n", ch);
	 act("$n grunts in $s sleep.", FALSE, ch, 0, 0, TO_ROOM);
       }
       return;
     } else {
       GET_LAST_DEATH(ch) = 0;
       /* no bad dreams, it has been too long */
     }
  }

  /* dream travel check */ 
  if(IS_AFFECTED(ch, AFF_DREAM)) {
    dream_travel(ch, 0);
    return;
  }

  /* this is level based dreams */
  switch(GET_LEVEL(ch)) {
  case 0: case 1:
  case 2: case 3:
  case 4: case 5: {
    if (!number(0,15)) {
      send_to_char("You have dreams of showing this world what you are "
                   "really made of.\r\n", ch);
      act("$n smiles in $s sleep.", FALSE, ch, 0, 0, TO_ROOM);
    }
    break;
  }

  case 6: case 7:
  case 8: case 9:
  case 10: {
    if (!number(0,15)) {
      send_to_char("You have a pleasant dream of safe travels to far places "
                   "and a hero's welcome when you return.\r\n", ch);
      act("$n begins to hum a happy ditty in $s sleep.", FALSE, ch, 0, 0,
	  TO_ROOM);
    }
    break;
  }

  case 11: case 12:
  case 13: case 14:
  case 15: case 16:
  case 17: case 18:
  case 19: case 20: {
    if (!number(0,15)) {
      send_to_char("You dream of your conquest of the world.\r\n", ch);
      act("$n begins to grin in $s sleep.", FALSE, ch, 0, 0, TO_ROOM);
    }
    break;
  }

  case 21: case 22:
  case 23: case 24:
  case 25: case 26:
  case 27: case 28: {
    if (!number(0,15)) {
      send_to_char("You dream of slaying the dark creatures of the night.\r\n",
                   ch);
      act("$n smirks in $s sleep.", FALSE, ch, 0, 0, TO_ROOM);
    }
    break;
  }

  case 29: case 30: {
    if (!number(0,15)) {
      send_to_char("You have a fantastic dream of one day attaining "
                   "immortality.\r\n", ch);
      act("$n looks like $e is having big dreams.", FALSE, ch, 0, 0, TO_ROOM);
    }
    break;
  }

  case LVL_IMMORT: {
    if (!number(0,15)) {
      send_to_char("You have big, grand dreams of the power of the Gods.\r\n",
		   ch);
      act("$n glows in $s sleep.", FALSE, ch, 0, 0, TO_ROOM);
    }
    break;
  }

  default: {
    if (!number(0,15)) {
      send_to_char("You toss and turn under the constant fear of the wrath "
		   "of Orodreth :-)\r\n", ch);
      send_to_char("You find yourself wide awake!\r\n", ch);
      act("$n awakens with fear in $s eyes.", FALSE, ch, 0, 0, TO_ROOM);
      do_wake(ch, "", 0, 0);
    }
    break;
  }

  } /* switch */
}

void dream_travel(struct char_data *ch, int subcmd)
{
  char buf[MAX_STRING_LENGTH];
  int i;
  for(i = 0; i <= NUM_DREAMS; i++)
  {
   if(!number(0,15) && !subcmd && !dtravel[i].subcmd) { 
    sprintf(buf, "You have a dream %s \r\n", dtravel[i].descrip);
    send_to_char(buf, ch);
    act("The sleeping body of $n fades from existence.", FALSE, ch,
       0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(dtravel[i].room_num));
    act("The sleeping body of $n fades into existence.", FALSE, ch,
       0, 0, TO_ROOM);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_DREAM);
    return;
   }
  
   if(!number(0,15) && subcmd) {
    sprintf(buf, "You have a dream %s \r\n", dtravel[i].descrip);
    send_to_char(buf, ch);
    act("The sleeping body of $n fades from existence.", FALSE, ch,
       0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(dtravel[i].room_num));
    act("The sleeping body of $n fades into existence.", FALSE, ch,
       0, 0, TO_ROOM);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_DREAM);
    return;
   }         

  }

}
