/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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

/* $Id: act.informative.c 1330 2007-08-05 01:18:21Z jravn $ */

#include "conf.h"
#include "sysdep.h"
#include <math.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "oc.h"
#include "faction.h"
#include "vt100.h"
#include "constants.h"
#include "timezone.h"

#include <sys/stat.h>

/* extern variables */
extern struct spell_info_type spell_info[]; 
extern int isname_with_abbrevs(char *str, char *namelist);

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *motd_mxp;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *future;
extern ush_int max_play;

/* for chars */
extern char *tattoos[];
extern char *dyes[];
extern char *hometowns[];

/* External functions */
ACMD(do_look);
void kender_steal(struct char_data *ch, struct char_data *victim);
void do_description(char *description, struct char_data *ch, int subcmd);
struct char_data *get_rider(struct char_data *mount);
struct char_data *get_mount(struct char_data *ch);
sh_int find_target_room(struct char_data *ch, char *rawroomstr);
long exp_needed_for_level(struct char_data *ch);
int find_exp(int level);

/* Local definitions */
#define OPEN_DOOR(room, obj)	(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED))


void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
  bool found;

  *buf = '\0';
  if ((mode == 0) && object->description)
    strcpy(buf, object->description);
  else if (object->short_description && ((mode == 1) || (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buf, object->short_description);
  else if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
    send_to_char("\r\n", ch);
    if (object->action_description) {
      strcpy(buf, "There is something written upon it:\r\n\r\n");
      strcat(buf, object->action_description);
      page_string(ch->desc, buf, 1);
    } else
      act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  } else if (mode == 5) {
    if (object->killer) {
      if (!strcmp(object->short_description, object->killer))
        sprintf(buf, "You see nothing special about %s.", object->short_description);
      else
        sprintf(buf, "You see nothing special about %s.\r\nIt was killed by %s.", 
                object->short_description, object->killer);
    } else if (GET_OBJ_TYPE(object) == ITEM_DRINKCON || GET_OBJ_TYPE(object) == ITEM_FOUNTAIN)
      strcpy(buf, "It looks like something you could drink from.");
    else if (GET_OBJ_TYPE(object) == ITEM_KEY)
      strcpy(buf, "It looks like a key to something.");
    else if (GET_OBJ_TYPE(object) == ITEM_FOOD)
      strcpy(buf, "It looks quite edible.");
    else 
      strcpy(buf, "You see nothing special.");

  }
  if ((mode == 0) || (mode == 5)) {
    if (GET_OBJ_TYPE(object) == ITEM_KEY) {
      sprintf(buf1, "\r\nNumber of uses left: %d", GET_OBJ_TIMER(object));
      strcat(buf, buf1);
    }
  }
  if (mode != 3) {
    found = FALSE;
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buf, " (invisible)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      strcat(buf, " (&bblue glow&n)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
      strcat(buf, " (&yyellow glow&n)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buf, " (&wglowing&n)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buf, " (humming)");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BROKEN)) {
      strcat(buf, " (broken)");
      found = TRUE;
    }
    if (object->worn_on)
      if (IS_COVERED(object->worn_by, object->worn_on)) {
        strcat(buf, " (covered)");
        found = TRUE;
      }
  }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}


/* deprecated. schedule for removal. 20021204 rparet */
/*
void
show_mult_obj_to_char(struct obj_data *object, struct char_data *ch,
		      int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];

  buffer[0] = 0;
  tmp[0] = 0;
 
  if ((mode == 0) && object->description)
    strcpy(buffer,object->description);
  else if (object->short_description &&
	   ((mode == 1) || (mode == 2) || (mode==3) || (mode == 4)))
    strcpy(buffer, object->short_description);
  else if (mode == 5)
    {
      if (GET_OBJ_TYPE(object) == ITEM_NOTE)
	{
	  if (object->action_description)
	    {
	      strcpy(buffer, "There is something written upon it:\n\r\n\r");
	      strcat(buffer, object->action_description);
	      page_string(ch->desc, buffer, 1);
	    }
	  else
	    act("It's blank.", FALSE, ch,0,0,TO_CHAR);
	  return;
	}
      else if (GET_OBJ_TYPE(object) == ITEM_DRINKCON || GET_OBJ_TYPE(object) == ITEM_FOUNTAIN)
        strcpy(buffer, "It looks like something you could drink from.");
      else
	strcpy(buffer,"You see nothing special.");
    }

  if (mode != 3)
    {
      if (IS_OBJ_STAT(object,ITEM_INVISIBLE))
	strcat(buffer,"(invisible)");
      if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD))
	strcat(buffer,"(red glow)");
      if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC))
	strcat(buffer,"(blue glow)");
      if (IS_OBJ_STAT(object,ITEM_GLOW))
	strcat(buffer,"(glowing)");
      if (IS_OBJ_STAT(object,ITEM_HUM))
	strcat(buffer,"(humming)");
      if (IS_OBJ_STAT(object, ITEM_BROKEN))
        strcat(buffer,"(broken)");
    }

  if (num>1)
    {
      sprintf(tmp,"[%d]", num);
      strcat(buffer, tmp);
    }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}
*/

void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, int show)
{
  struct obj_data *i;
  bool found = FALSE;
  struct howmany *temp = NULL;
  struct howmany *head = NULL;
  char text[256];

  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (IS_OBJ_STAT(i, ITEM_BURIED)) {
        strcpy(text, "*");
        strcat(text, i->description);
      } else if (mode & 16)
        strcpy(text, i->description);
      else
        strcpy(text, i->short_description);
  
      if (!oc_onlist(GET_OBJ_VNUM(i), i->obj_flags.extra_flags, GET_OBJ_WEIGHT(i), text, head)) {
        temp = oc_get_node();
    
        if (IS_OBJ_STAT(i, ITEM_BURIED)) {
          strcpy(temp->text, "*");
          strcat(temp->text, i->description);
        } else if (mode & 16)
          strcpy(temp->text, i->description);
        else
          strcpy(temp->text, i->short_description);

        temp->weight = GET_OBJ_WEIGHT(i);
        temp->vnum = GET_OBJ_VNUM(i);
        temp->count = 1;
        temp->extras = i->obj_flags.extra_flags;
        head = oc_add_front(temp, head);
      }
      
      found = TRUE;
    }       
  }
     
  if (found)
    oc_show_list(head, ch, (mode & 8), (mode & 4), (mode & 2), (mode & 1));
          
  oc_dispose_list(head);
          
  if (!found && show)
    send_to_char("Nothing.\r\n", ch);
}

/* deprecated.  schedule for removal. 20021204 rparet */
/*
void
list_obj_in_heap(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[255];
  int k, cond_top, cond_tot[255], found=FALSE;
 
  int Num_Inventory = 1;
  cond_top = 0;
 
  for (i=list; i; i = i->next_content)
    {
      if (CAN_SEE_OBJ(ch, i))
	{
	  if (cond_top< 50)
	    {
	      found = FALSE;
	      for (k=0;(k<cond_top&& !found);k++)
		{
		  if (cond_top>0)
		    {
		      if ((i->item_number == cond_ptr[k]->item_number) &&
			  (i->short_description &&
			   cond_ptr[k]->short_description &&
			   (!strcmp(i->short_description,
				    cond_ptr[k]->short_description))) )
			{
			  cond_tot[k] += 1;
			  found=TRUE;
			}
		    }
		}
	      if (!found)
		{
		  cond_ptr[cond_top] = i;
		  cond_tot[cond_top] = 1;
		  cond_top+=1;
		}
	    }
	  else
	    show_obj_to_char(i,ch,2);
	}
    }

  if (cond_top)
    {
      for (k=0; k<cond_top; k++)
	{
	  if (cond_tot[k] > 1)
	    {
	      Num_Inventory += cond_tot[k] - 1;
	      show_mult_obj_to_char(cond_ptr[k],ch,2,cond_tot[k]);
	    }
	  else
	    show_obj_to_char(cond_ptr[k],ch,2);
	}
    }
  else
    send_to_char("Nothing.\n\r", ch);
}
*/

void
diag_char_to_char(struct char_data *i, struct char_data *ch)
{
  int percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;		/* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  if (percent >= 100)
    strcat(buf, " is in excellent condition.\r\n");
  else if (percent >= 90)
    strcat(buf, " has a few scratches.\r\n");
  else if (percent >= 75)
    strcat(buf, " has some small wounds and bruises.\r\n");
  else if (percent >= 50)
    strcat(buf, " has quite a few wounds.\r\n");
  else if (percent >= 30)
    strcat(buf, " has some big nasty wounds and scratches.\r\n");
  else if (percent >= 15)
    strcat(buf, " looks pretty hurt.\r\n");
  else if (percent >= 0)
    strcat(buf, " is in awful condition.\r\n");
  else
    strcat(buf, " is bleeding awfully from big wounds.\r\n");

  send_to_char(buf, ch);
}


void
look_at_char(struct char_data *i, struct char_data *ch)
{
  extern char *flesh_alter_weapon( struct char_data *ch );
  int j, found;
  struct obj_data *tmp_obj;

  if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  if (!IS_NPC(i)) {
    if (IS_AFFECTED(i, AFF_FLESH_ALTER)) {
      char *weapon = flesh_alter_weapon(i);
      sprintf(buf, "%s is %s, but %s hand is a %s!\r\n",PERS(i,ch), 
        npc_races[GET_RACE(i)].race, HSHR(i), weapon);    
      FREE(weapon);
    } else 
      sprintf(buf, "%s is %s.\r\n",PERS(i,ch), npc_races[GET_RACE(i)].race);  
    send_to_char(buf, ch);
  }

  if (GET_TATTOO(i)) {
    sprintf(buf, "On %s right arm is a tattoo %s... it %sglows%s softly.\r\n", 
            HSHR(i), tattoos[GET_TATTOO(i)], CCWHT(ch, C_CMP), CCNRM(ch, C_CMP));
    send_to_char(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_VAMPIRE)) {
    sprintf(buf, "Looking at %s closely, you see two white fangs --  %s is"
     " a vampire!\r\n", HMHR(i), HSSH(i));
    stc(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_WEREWOLF)) {
    sprintf(buf, "May the gods have mercy --  %s is a werewolf!\r\n", HSSH(i));
    stc(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_METALSKIN)) {
    sprintf(buf, "You notice %s skin has a metallic hue!\r\n", HSHR(i));
    stc(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_CUTTHROAT)) {
    sprintf(buf, "%s's throat has been slit from ear to ear!\r\n", PERS(i, ch));
    stc(buf, ch);
  }

  diag_char_to_char(i, ch);

  /* If the target is a mob, and the character is not an Immortal, don't let them peek */
  if (IS_NPC(i) && GET_LEVEL(ch) < LVL_IMMORT)
    return;

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
     if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
       if (!IS_COVERED(i, j) || ch==i|| GET_LEVEL(ch)>LVL_IMMORT) {
         if (IS_MXP(ch))
           send_to_char(where_mxp[j], ch);
         else
           send_to_char(where[j], ch);
         show_obj_to_char(GET_EQ(i, j), ch, 1);
     }
   }
  }
  if (ch != i && GET_SKILL(ch, SKILL_PEEK)) {
    found = FALSE;
    act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
    if(!IS_NPC(i) && (GET_LEVEL(i)>LVL_IMMORT && GET_LEVEL(ch)<GET_LEVEL(i))) {
     found = TRUE;
     stc("Your soul (burning)\r\n", ch);
    } else {
      for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
       if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch))) {
         show_obj_to_char(tmp_obj, ch, 1);
         found = TRUE;
       }
    }

    if (!found)
     send_to_char("You can't see anything.\r\n", ch);
  }
  if (ch != i && GET_RACE(ch) == RACE_KENDER && GET_LEVEL(ch)<LVL_IMMORT &&
    GET_LEVEL(i)<LVL_IMMORT)
    kender_steal(ch, i);
}


void
list_one_char(struct char_data *i, struct char_data *ch)
{
  char *mob_race_name(int mob_race);
  char *positions[] = {
    " is lying here, dead.",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here.",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here."
  };

  if (IS_NPC(i) && MOB_FLAGGED(i, MOB_EXTRACT))
    return;

  /* Completely "hide" those mobs that are flagged as NODISPLAY */
  if (MOB_FLAGGED(i, MOB_NODISPLAY) && (GET_LEVEL(ch) < LVL_IMMORT))
    return;

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    if (IS_AFFECTED(i, AFF_INVISIBLE))
      strcpy(buf, "*");
    else if (MOB_FLAGGED(i, MOB_NODISPLAY))
      strcpy(buf, "**");
    else
      *buf = '\0';

    if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      if (IS_EVIL(i))
        strcat(buf, "(Red Aura) ");
      else if (IS_GOOD(i))
        strcat(buf, "(Blue Aura) ");
    }

    if (IS_AFFECTED(ch, AFF_DETECT_RACE)) {
      sprintf(buf1, "(%s) ", mob_race_name(GET_RACE(i)));
      strcat(buf, buf1);
    }

    strcat(buf, i->player.long_descr);
    send_to_char(buf, ch);

    if (IS_AFFECTED(i, AFF_SANCTUARY)) {
      if (IS_EVIL(i))
        act("...$e is surrounded by a red aura!", FALSE, i, 0, ch, TO_VICT);
      else
        act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    }
    if (IS_AFFECTED(i, AFF_BLIND))
      act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i, AFF_INVULN))
      act("...$e is surrounded by a glowing sphere!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i, AFF_FLAMING))
      act("...$e is engulfed in flames!", FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i, AFF_SOULTRAP))
      act("...$e is surrounded by a faint mist!", FALSE, i, 0, ch, TO_VICT);
      
    return;
  }
  if (IS_NPC(i)) {
    if (!i->player.long_descr)
      return;
    strcpy(buf, i->player.short_descr);
    CAP(buf);
  }
  else
    sprintf(buf, "%s %s", i->player.name, GET_TITLE(i));

  if (IS_AFFECTED(i, AFF_INVISIBLE))
    strcat(buf, " (invisible)");
  if (IS_AFFECTED(i, AFF_HIDE))
    strcat(buf, " (hidden)");
  if (!IS_NPC(i) && !i->desc)
    strcat(buf, " (linkless)");
  if (PLR_FLAGGED(i, PLR_WRITING))
    strcat(buf, " (writing)");
  if (PLR_FLAGGED(i, PLR_IT))
    strcat(buf, " (IT)");

  if (GET_POS(i) != POS_FIGHTING && !IS_MOUNTED(i))
    strcat(buf, positions[(int) GET_POS(i)]);
  else if (IS_MOUNTED(i)) {
    strcat(buf, " is here, mounted on ");
    if (get_rider(i) == ch)
      strcat(buf, "YOU!");
    else {
      if (get_mount(i))
        strcat(buf, PERS(get_mount(i), ch));
      else
        strcat(buf, "thin air");
      strcat(buf, ".");
    }
  } else {
    if (FIGHTING(i)) {
      strcat(buf, " is here, fighting ");
      if (FIGHTING(i) == ch)
        strcat(buf, "YOU!");
      else {
        if (i->in_room == FIGHTING(i)->in_room)
          strcat(buf, PERS(FIGHTING(i), ch));
        else
          strcat(buf, "someone who has already left");
        strcat(buf, "!");
      }
    } else   /* NIL fighting pointer */
      strcat(buf, " is here struggling with thin air.");
  }

  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, " (Red Aura)");
    else if (IS_GOOD(i))
      strcat(buf, " (Blue Aura)");
  }
  if (!IS_NPC(i))
    if (PRF_FLAGGED(i, PRF_AFK))
      strcat(buf, " (AFK)");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  if (IS_AFFECTED(i, AFF_SANCTUARY)) {
    if (IS_EVIL(i))
      act("...$e is surrounded by a red aura!", FALSE, i, 0, ch, TO_VICT);
    else
      act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
  }
  if (IS_AFFECTED(i, AFF_INVULN))
    act("...$e is surrounded by a glowing sphere!", FALSE, i, 0, ch, TO_VICT);

  if (IS_AFFECTED(i, AFF_FLAMING))
    act("...$e is engulfed in flames!", FALSE, i, 0, ch, TO_VICT);
}


void
list_char_to_char(struct char_data * list, struct char_data * ch)
{
  struct char_data *i;
  
  for (i = list; i; i = i->next_in_room)
    if (ch != i && !get_rider(i))
      {
	if ( (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)) || 
	     ( IS_SET(PRF_FLAGS(ch), PRF_HOLYLIGHT) &&
               (GET_REAL_LEVEL(ch) >= GET_INVIS_LEV(i))) )
	  list_one_char(i, ch);
	else if (IS_AFFECTED(i, AFF_HIDE) && IS_AFFECTED(ch, AFF_SENSE_LIFE))
	  send_to_char("You sense a hidden presence in the room.\r\n", ch);
	else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
		 IS_AFFECTED(i, AFF_INFRAVISION))
	{
	  stc("You see a pair of glowing ", ch);
	  stc(CCRED(ch, C_CMP), ch);
	  stc("red",ch);
	  stc(CCNRM(ch, C_CMP), ch);
	  stc(" eyes looking your way.\r\n", ch);
	}
      }
}


void
do_auto_exits(struct char_data * ch)
{
  int door;

  *buf = '\0';

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	(!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) || GET_LEVEL(ch)>=LVL_IMMORT))
    {
      bool io /* Imm Only */= (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
         		       GET_LEVEL(ch)>=LVL_IMMORT);
      bool mxp = IS_MXP(ch);

      sprintf(buf + strlen(buf), "%s%s%s%s%s%s%s%s ",
                mxp ? "[1z" : "",
		io ? "&r" : "",
		io ? "(" : "",
                mxp ? "<ex>" : "",
                dirs[door],
                mxp ? "</ex>" : "",
                io ? ")" : "",
		io ? "&c": "");
    }

  sprintf(buf2, "&c[ Exits: %s]&n\r\n", *buf ? buf : "None! ");

  send_to_char(buf2, ch);
}

ACMD(do_exits)
{
  int door;
  bool immort = ((GET_LEVEL(ch) >= LVL_IMMORT) ? TRUE : FALSE);

  *buf = *buf2 = '\0';

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  
  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It's too dark for you see any exits!\r\n", ch);
    return;
  }

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (immort) {
      if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) {
        if (!IS_MXP(ch))
          sprintf(buf2, "%-5s - [%5ld] %s", dirs[door], world[EXIT(ch, door)->to_room].number,
                  world[EXIT(ch, door)->to_room].name);
        else
          sprintf(buf2, "\033[1z<ex>%-5s</ex> - [%5ld] %s", dirs[door], world[EXIT(ch, door)->to_room].number,
                  world[EXIT(ch, door)->to_room].name);
        if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
          strcat(buf2, "  &R(closed)&n\r\n");
        else
          strcat(buf2, "\r\n");
        strcat(buf, CAP(buf2));
      }
    } else {
      if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
          !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
        if (!IS_MXP(ch))
          sprintf(buf2, "%-5s - ", dirs[door]);
        else
          sprintf(buf2, "\033[1z<ex>%-5s</ex> - ", dirs[door]);

        if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
          strcat(buf2, "Too dark to tell\r\n");
        else {
          strcat(buf2, world[EXIT(ch, door)->to_room].name);
          strcat(buf2, "\r\n");
        }
        strcat(buf, CAP(buf2));
      }
    }
    send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);
}

void
look_at_room(struct char_data * ch, int ignore_brief) {

  struct char_data *tch, *next_tch;


  /*  First, if we have a dark or blind look, do that and return */
  if ((IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) ||
     (IS_AFFECTED(ch, AFF_BLIND))) {

     /* if the person is blind, have a special message :P */
     if (IS_AFFECTED(ch, AFF_BLIND)) {
         send_to_char(CCCYN(ch, C_NRM), ch);
         send_to_char("Darkness\r\n\r\n", ch);
         send_to_char(CCNRM(ch, C_NRM), ch);
         send_to_char("You see nothing but infinite darkness...\r\n", ch);
         return;
     } else {
         send_to_char(CCCYN(ch, C_NRM), ch);
         send_to_char("Darkness\r\n\r\n", ch);
         send_to_char(CCNRM(ch, C_NRM), ch);
         send_to_char("It is too dark here to see much of anything...\r\nYou are likely to be eaten by a grue, whatever that is.\r\n", ch);
     }

     for (tch = world[ch->in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (tch == ch) continue;
        if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT) continue;
        if (IS_AFFECTED(tch, AFF_SNEAK)) continue;
        if (IS_AFFECTED(tch, AFF_HIDE)) continue;
        if (IS_AFFECTED(tch, AFF_INFRAVISION)) {
           send_to_char("You see a pair of glowing red eyes.\r\n", ch);
           continue;
        }
     
        send_to_char("You hear someone or something moving around nearby.\r\n",
                     ch);
     }
     return;
  }

  send_to_char(CCCYN(ch, C_NRM), ch);

  if (PRF_FLAGGED(ch, PRF_ROOMFLAGS))
    {
      sprintbit(ROOM_FLAGS(ch->in_room), room_bits, buf);
      sprintf(buf2, "[%5ld] %s [ %s]", world[ch->in_room].number,
	      world[ch->in_room].name, buf);
      sprintf(buf2, "%s [ %s ]", buf2, sector_types[world[ch->in_room].sector_type]);
      send_to_char(buf2, ch);
    }
  else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) {
    if (!ZONE_FLAGGED(world[ch->in_room].zone, ZONE_UNLOAD))
      reload_room_descs(world[ch->in_room].zone);
    do_description(world[ch->in_room].description, ch, SCMD_ROOM);
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_BIND))  /* let players know this is a bind room */
    stc("&BA warm blue glow fills this place.&n\r\n", ch);

  /* autoexits */
  do_auto_exits(ch);


  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);

  /*
   * mode = 20 is 10100 binary and means:
   * long format, no weights in list, wide format, no indent, no header
   */
  list_obj_to_char(world[ch->in_room].contents, ch, 20, FALSE);

  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}


void
look_in_direction(struct char_data * ch, int dir)
{
  int rp, location, original_loc;
	  
  if (FIGHTING(ch))
    {
      stc("You're a little busy right now!\r\n", ch);
      return;
    }

  sprintf(buf,"You look %swards.", dirs[dir]);
  act(buf, FALSE, ch, 0, 0, TO_CHAR);

  if (EXIT(ch, dir)) 
    {
	if (EXIT(ch, dir)->general_description)
        {
	  send_to_char(EXIT(ch, dir)->general_description, ch);
          return;
        }
      if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) &&
          EXIT(ch, dir) && EXIT(ch, dir)->keyword &&
	  (ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK)    ||
	   !strstr(EXIT(ch, dir)->keyword, "secret")))
	{
	  sprintf(buf, "The %s %s closed.\n\r",
		  fname(EXIT(ch, dir)->keyword),
		  PLURAL(fname(EXIT(ch, dir)->keyword)) ? "are" : "is");
	  send_to_char(buf, ch);
	}
      else
	{
	  if (IS_SET(EXIT(ch, dir)->exit_info, EX_ISDOOR) &&
              EXIT(ch, dir) && EXIT(ch, dir)->keyword &&
	      (ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK)    ||
	       !strstr(EXIT(ch, dir)->keyword, "secret")))
	    {
	      sprintf(buf, "The %s %s open.\r\n\n",
		      fname(EXIT(ch, dir)->keyword),
                      PLURAL(fname(EXIT(ch, dir)->keyword)) ? "are" : "is");
	      send_to_char(buf, ch);
	    }
	}
      if(!ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK) &&
         EXIT(ch, dir) && EXIT(ch, dir)->keyword &&
	 strstr(EXIT(ch, dir)->keyword, "secret"))
	   send_to_char("You see nothing special.\r\n", ch);
      
      
      if (EXIT(ch, dir) && EXIT(ch,dir)->to_room && 
	  (!IS_SET(EXIT(ch,dir)->exit_info, EX_ISDOOR) || 
	   (!IS_SET(EXIT(ch,dir)->exit_info, EX_CLOSED))))
	{
          rp = world[(EXIT(ch,dir)->to_room)].number;
          if(rp>0)
          {
            sprintf(buf,"%d",rp);
            if ((location = find_target_room(ch, buf)) < 0)
              return; /*This can't happen, i don't think :) */
          
           if (!ROOM_FLAGGED(location, ROOM_DEATH)) {
             original_loc = ch->in_room;
  	     char_from_room(ch);
  	     char_to_room(ch, location);
	     look_at_room(ch, 0);
	     char_from_room(ch);
	     char_to_room(ch, original_loc);
           } else
             stc("You see nothing special.\r\n", ch);	/* Stop crash when looking at DT */
         }
         else
         {
           stc("You see nothing special.\r\n", ch);
         }
	}  
    }
} 


void
look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("Look in what?\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
             FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
            (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
            (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    act("There's nothing inside $p!", TRUE, ch, obj, 0, TO_CHAR);
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
      if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) {
        act("$p is closed and locked.", TRUE, ch, obj, 0, TO_CHAR);
        return;
      }
      if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) {
        OPEN_DOOR(ch->in_room, obj);    /* Prevents Okay message */
        act("$p is closed, but you open it.", TRUE, ch, obj, 0, TO_CHAR);
      }
      send_to_char(obj->short_description, ch);
      switch (bits) {
      case FIND_OBJ_INV:
        send_to_char(" (carried): \r\n", ch);
        break;
      case FIND_OBJ_ROOM:
        send_to_char(" (here): \r\n", ch);
        break;
      case FIND_OBJ_EQUIP:
        send_to_char(" (used): \r\n", ch);
        break;
      }
      /*
       * mode = 15 which is 01111 in binary and means:
       * short (not long) descr, show weights,
       * wide list, do indent, do header
       */
      list_obj_to_char(obj->contains, ch, 15, TRUE);
    } else {		/* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
        act("$p is empty.", TRUE, ch, obj, 0, TO_CHAR);
      else {
        if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0))
          sprintf(buf, "$p's contents seem somewhat murky."); /* BUG */
        else {
          amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
          sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
          sprintf(buf, "$p is  %sfull of a %s liquid.", fullness[amt], buf2);
        }
        act(buf, TRUE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}


char * 
find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname_with_abbrevs(word, i->keyword))
      return (i->description);

  return NULL;
}

long find_exlink(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      if (i->link)
        return (i->link);

  return (0);
}

/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void
look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = 0, i = 0, j, fnum, link, original_loc;
  struct char_data *found_char = NULL;
  struct obj_data *obj = NULL, *found_obj = NULL;
  char *desc;

  if (!ch->desc)
    return;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }
 
 bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
        act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
        act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }

  /* Strip off "number." from 2.foo and friends. */ 
  if (!(fnum = get_number(&arg))) {    
    send_to_char("Look at what?\r\n", ch);
    return;
  }

  /* Does the argument match an extra desc in the room which is linked to another? */
  if ((link = find_exlink(arg, world[ch->in_room].ex_description)) != 0 && ++i == fnum) {
    if (FIGHTING(ch)) {
      send_to_char("You're too busy to be looking at that now!\r\n", ch);
      return;
    }
    original_loc = ch->in_room;
    if (GET_ROOM_VNUM(link)) {
      char_from_room(ch);
      char_to_room(ch, link);
      look_at_room(ch, 0);
      char_from_room(ch);
      char_to_room(ch, original_loc);
      return;
    } else {
      sprintf(buf, "SYSERR: Invalid extra descr link in room %ld.", ch->in_room);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    }
  }

  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL && ++i == fnum)
    {
      do_description(desc, ch, SCMD_ROOM);		/* Enable highlighting of extra desc keywords */
      return;
    }

  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL && ++i == fnum)
	{
          do_description(desc, ch, SCMD_OBJ_EQ);         /* Enable highlighting of extra desc keywords */	  
	  found = 1;
	}
  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum)
	  {
            do_description(desc, ch, SCMD_OBJ_INV);         /* Enable highlighting of extra desc keywords */
	    found = 1;
	  }
    }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found;
       obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum)
	{
          do_description(desc, ch, SCMD_OBJ_ROOM);         /* Enable highlighting of extra desc keywords */
	  found = 1;
	}
  if (bits)
    {			/* If an object was found back in generic_find */
      if (!found)
	show_obj_to_char(found_obj, ch, 5);	/* Show no-description */
      else
	show_obj_to_char(found_obj, ch, 6);	/* Find hum, glow etc */
    }
  else if (!found)
    send_to_char("You do not see that here.\r\n", ch);
}

ACMD(do_abils)
{
  long exp;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg) {
    stc("Your current ability scores: \r\n", ch);
    sprintf(buf, "Strength:      (%3d) (%ld)\r\n", GET_STR(ch), find_skill_exp(GET_REAL_STR(ch)));
    stc(buf, ch);
    sprintf(buf, "Dexterity:     (%3d) (%ld)\r\n", GET_DEX(ch), find_skill_exp(GET_REAL_DEX(ch)));
    stc(buf, ch);
    sprintf(buf, "Intelligence:  (%3d) (%ld)\r\n", GET_INT(ch), find_skill_exp(GET_REAL_INT(ch)));
    stc(buf, ch);
    sprintf(buf, "Wisdom:        (%3d) (%ld)\r\n", GET_WIS(ch), find_skill_exp(GET_REAL_WIS(ch)));
    stc(buf, ch);
    sprintf(buf, "Constitution:  (%3d) (%ld)\r\n", GET_CON(ch), find_skill_exp(GET_REAL_CON(ch)));
    stc(buf, ch);
    sprintf(buf, "Charisma:      (%3d) (%ld)\r\n", GET_CHA(ch), find_skill_exp(GET_REAL_CHA(ch)));
    stc(buf, ch);

    sprintf(buf, "\r\nYou have %d %s to use on ability upgrades.\r\n",
      GET_STAT_PTS(ch), (GET_STAT_PTS(ch) == 1 ? "point" : "points"));
    stc(buf, ch);
    return;
  } else {
    if (!GET_STAT_PTS(ch)) {
      stc("You don't have any points to use on ability upgrades.\r\n", ch);
      return;
    }

    if (strstr(arg, "str"))
      exp = find_skill_exp(GET_REAL_STR(ch));
    else if (strstr(arg, "dex"))
      exp = find_skill_exp(GET_REAL_DEX(ch));
    else if (strstr(arg, "int"))
      exp = find_skill_exp(GET_REAL_INT(ch));
    else if (strstr(arg, "wis"))
      exp = find_skill_exp(GET_REAL_WIS(ch));
    else if (strstr(arg, "con"))
      exp = find_skill_exp(GET_REAL_CON(ch));
    else if (strstr(arg, "cha"))
      exp = find_skill_exp(GET_REAL_CHA(ch));
    else {
      stc("What ability is that??\r\n", ch);
      return;
    }

    if ((GET_EXP_Q(ch) - exp) < 0) {
      stc("You don't have enough experience points to raise that statistic.\r\n", ch);
      return;
    } else {
      if (strstr(arg, "str") && (GET_REAL_STR(ch) < MAX_ABILITY))
        GET_REAL_STR(ch)++;
      else if (strstr(arg, "dex") && (GET_REAL_DEX(ch) < MAX_ABILITY))
        GET_REAL_DEX(ch)++;
      else if (strstr(arg, "int") && (GET_REAL_INT(ch) < MAX_ABILITY))
        GET_REAL_INT(ch)++;
      else if (strstr(arg, "wis") && (GET_REAL_WIS(ch) < MAX_ABILITY))
        GET_REAL_WIS(ch)++;
      else if (strstr(arg, "con") && (GET_REAL_CON(ch) < MAX_ABILITY))
        GET_REAL_CON(ch)++;
      else if (strstr(arg, "cha") && (GET_REAL_CHA(ch) < MAX_ABILITY))
        GET_REAL_CHA(ch)++;
      else {
        stc("You have already reached the maximum value for this ability.\r\n", ch);
        return;
      }
      GET_EXP_Q(ch) -= exp;
      GET_STAT_PTS(ch)--;
    }
  }
  stc("Ability upgraded.\r\n", ch);
  affect_total(ch);
}

ACMD(do_look)
{
  static char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING) {
    send_to_char("You can't see anything but stars!\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }

  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);    
    list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
    send_to_char(CCGRN(ch, C_NRM), ch);
    list_obj_to_char(world[ch->in_room].contents, ch, 20, FALSE);
    send_to_char(CCNRM(ch, C_NRM), ch);
    return;
  }

  half_chop(argument, arg, arg2);

  if (subcmd == SCMD_READ) {
     if (!*arg)
        send_to_char("Read what?\r\n", ch);
     else
        look_at_target(ch, arg);
     return;
  }
  if (!*arg)    /* "look" alone, without an argument at all */
     look_at_room(ch, 1);
  else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
  /* did the char type 'look <direction>?' */
  else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
  else {
    if (is_abbrev(arg, "at")) {
      look_at_target(ch, arg2);
      strcpy(arg, arg2);
    } else
      look_at_target(ch, arg);
  }
}


ACMD(do_score)
{
  extern char *affected_names[];
  extern char *flesh_alter_weapon( struct char_data *ch );
  int i, align, weight = 0;
  float ac;
  char tmpbuf[MAX_STRING_LENGTH];
  struct time_info_data playing_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  char *mob_race_name(int mob_race);
  int any = FALSE;
  struct master_affected_type *aff;
  
  strcpy(buf, "\0");
  sprintf(buf + strlen(buf), "[%s%d%s] %s %-40s",
    CCYEL(ch, C_NRM), GET_LEVEL(ch), CCNRM(ch, C_NRM), GET_NAME(ch), GET_TITLE(ch));
  sprintf(buf + strlen(buf), "  Age: %d years\r\n", GET_AGE(ch));

  sprintf(buf,
	  "%sHit points: %s%d%s(%s%d%s)  %s points: %s%d%s(%s%d%s)  "
	  "Movement points: %s%d%s(%s%d%s)\r\n",
	  buf, CCGRN(ch, C_CMP), GET_HIT(ch), CCNRM(ch, C_CMP), 
	  CCRED(ch, C_CMP), GET_MAX_HIT(ch), CCNRM(ch, C_CMP), 
	  "Mana",
	  CCGRN(ch, C_CMP), GET_MANA(ch), CCNRM(ch, C_CMP), 
	  CCRED(ch, C_CMP), GET_MAX_MANA(ch), CCNRM(ch, C_CMP), 
	  CCGRN(ch, C_CMP), GET_MOVE(ch),CCNRM(ch, C_CMP), 
	  CCRED(ch, C_CMP), GET_MAX_MOVE(ch), CCNRM(ch, C_CMP));
  
  align = GET_ALIGNMENT(ch);
  if (align ==  1000)
    strcpy(tmpbuf,       "You are the Epitome of Righteousness!");
  else if (align >= 900)
    strcpy(tmpbuf,       "You're so good, you make the angels jealous.");
  else if (align >= 750)
    strcpy(tmpbuf,       "You are feeling pretty righteous.");
  else if (align >= 500)
    strcpy(tmpbuf,       "You are aligned with the path of right.");
  else if (align >= 350)
    strcpy(tmpbuf,       "You are feeling pretty good today.");
  else if (align >= 100)
    strcpy(tmpbuf,       "You are a little more good than neutral, but yet still bland.");
  else if (align > -100)
    strcpy(tmpbuf,       "You are neutral, how boring.");
  else if (align > -350)
    strcpy(tmpbuf,       "You are a little more evil than neutral, but not very exciting.");
  else if (align > -500)
    strcpy(tmpbuf,       "I actually think you would kill your own mother.");
  else if (align > -750)
    strcpy(tmpbuf,       "You are so evil it hurts.");
  else if (align > -900)
    strcpy(tmpbuf,       "Charles Manson is in your fan club.");
  else 
    strcpy(tmpbuf,       "You are the Epitome of Evil!");
  sprintf(buf, "%s%s\r\n", buf, tmpbuf);
  strcpy(tmpbuf, "You ");
  ac = (float)GET_AC(ch) / (float)MAX_AC;
  if (ac == 0)
    strcat(tmpbuf, "are completely unarmored.");
  else if (ac <= .10)
    strcat(tmpbuf, "are lightly clothed.");
  else if (ac <= .15)
    strcat(tmpbuf, "are pretty well clothed.");
  else if (ac <= .25)
    strcat(tmpbuf, "are lightly armored.");
  else if (ac <= .35)
    strcat(tmpbuf, "are well armored.");
  else if (ac <= .45)
    strcat(tmpbuf, "are getting pretty sweaty with all that armor on.");
  else if (ac <= .55)
    strcat(tmpbuf, "are very well armored.");
  else if (ac <= .65)
    strcat(tmpbuf, "are extremely well armored.");
  else if (ac <= .75)
    strcat(tmpbuf, "are decked out in full battle armor.");
  else if (ac <= .85)
    strcat(tmpbuf, "are armored like a wyvern!");
  else if (ac <= .95)
    strcat(tmpbuf, "are armored like a dragon!");
  else if (ac <= .99)
    strcat(tmpbuf, "could walk through the gates of Hell in all that"
            " armor!");
  else
    strcat(tmpbuf, "are armored like a god!");

  sprintf(buf, "%s%s\r\n", buf,tmpbuf);

  sprintf(buf + strlen(buf), "Total Experience: \t%s%ld%s points\r\n",
    CCGRN(ch, C_NRM), GET_EXP(ch), CCNRM(ch, C_NRM));
  sprintf(buf + strlen(buf), "Unassigned Experience: \t%s%ld%s points\r\n",
    CCGRN(ch, C_NRM), GET_EXP_Q(ch), CCNRM(ch, C_NRM));
  sprintf(buf + strlen(buf), "Vitae Points: %s%d%s  Vitae Experience Required: %s%ld%s\r\n",
    CCRED(ch, C_NRM), GET_VITAE(ch), CCNRM(ch, C_NRM),
    CCRED(ch, C_NRM), GET_VITAE_EXP(ch), CCNRM(ch, C_NRM));
  sprintf(buf + strlen(buf), "Group Points: %s%d%s  Ability Points: %s%d%s\r\n",
    CCGRN(ch, C_NRM), GET_GROUP_PTS(ch), CCNRM(ch, C_NRM),
    CCGRN(ch, C_NRM), GET_STAT_PTS(ch), CCNRM(ch, C_NRM));
  sprintf(buf + strlen(buf), "Coins carried: %s%d%s gold coins    ",
    CCYEL(ch, C_NRM), GET_GOLD(ch), CCNRM(ch, C_NRM));
  sprintf(buf + strlen(buf), "Coins in bank: %s%d%s gold coins\r\n",
    CCYEL(ch, C_NRM), GET_BANK_GOLD(ch), CCNRM(ch, C_NRM));
  sprintf(buf + strlen(buf), "Kills: %s%d%s  Pks: %s%d%s  Deaths: %s%d%s\r\n",
    CCRED(ch, C_NRM), (int)GET_KILLS(ch), CCNRM(ch, C_NRM),
    CCRED(ch, C_NRM), (int)GET_PKS(ch), CCNRM(ch, C_NRM),
    CCRED(ch, C_NRM), (int)GET_DEATHS(ch), CCNRM(ch, C_NRM));

  if (!IS_NPC(ch))
    {
      if (GET_LEVEL(ch) < LVL_IMMORT-1)
	sprintf(buf, "%sYou need %s%d%s exp to reach your next level.\r\n", buf,
        CCGRN(ch, C_CMP), MAX(0, (exp_needed_for_level(ch) - GET_EXP(ch))), CCNRM(ch, C_CMP));

      playing_time = real_time_passed((time(0) - ch->player.time.logon) +
				      ch->player.time.played, 0);
      sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
	      buf, playing_time.day, playing_time.hours);

      if ((age(ch).month == 0) && (age(ch).day == 0))
        strcat(buf, "It's your birthday today. Happy birthday!\r\n");
      if (GET_AGE(ch) >= 80)
        strcat(buf, "You are a Veteran of Dark Pawns.\r\n");

      sprintf(buf, "%sYou are %s %s citizen of %s.\r\n", 
        buf, AN(npc_races[GET_RACE(ch)].race), npc_races[GET_RACE(ch)].race, hometowns[GET_HOME(ch)]);
      if (GET_TATTOO(ch)) {
        sprintbit(GET_DYE(ch), dyes, buf2);
        sprintf(buf + strlen(buf), "You have a tattoo %s (%s)\r\n", tattoos[GET_TATTOO(ch)], buf2);
      }
      if (GET_STABLE(ch)) {
        struct char_data *mount = read_mobile(GET_MOUNT_NUM(ch), VIRTUAL, NULL);
        if (mount) {
          char_to_room(mount, 0);
          sprintf(buf + strlen(buf), "You have %s stabled in %s.\r\n", GET_NAME(mount),
            zone_table[world[real_room(GET_STABLE(ch))].zone].name);
          extract_char(mount);
        }
      }
      sprintf(buf + strlen(buf), "Racial hatreds: ");
      *tmpbuf = 0;
      for (i = 0; i < 5; i++)
        if (GET_RACE_HATE(ch, i) != -1)
          sprintf(tmpbuf + strlen(tmpbuf), "%s%s%s, ",
            CCGRN(ch, C_NRM), mob_race_name(GET_RACE_HATE(ch, i)), CCNRM(ch, C_NRM));
      if (*tmpbuf) {
        tmpbuf[strlen(tmpbuf) - 2] = '\0';
        strcat(buf, tmpbuf);
        strcat(buf, "\r\n");
      } else
        sprintf (buf + strlen(buf), "%sNone%s\r\n", CCGRN(ch, C_NRM), CCNRM(ch, C_NRM));
    }

  if (IS_CARRYING_W(ch))
    weight = CAN_CARRY_W(ch)/IS_CARRYING_W(ch);
  else
    weight = -1;
  weight = MAX(-1, weight);
  if (weight >= 4)
    strcat(buf, "Your pack is light.\r\n");
  if (weight == 3)
    strcat(buf, "Your pack is fairly light.\r\n");
  if (weight == 2)
    strcat(buf, "Your pack is fairly heavy.\r\n");
  if (weight == 1)
    strcat(buf, "Your pack is heavy.\r\n");
  if (weight == 0)
    strcat(buf, "Your pack is almost too heavy to lift.\r\n");
  if (weight == -1)
    strcat(buf, "Your pack is empty.\r\n");
  if (PLR_FLAGGED(ch, PLR_CHOSEN))
    strcat(buf, "You are a chosen of the gods.(BadMuthaFucker)\r\n");
  if (GET_PATRON(ch))
    sprintf(buf + strlen(buf), "You are a vassal of %s.\r\n", CAP(get_name_by_id(GET_PATRON(ch))));
  if (GUARDING(ch))
    sprintf(buf + strlen(buf), "You are guarding %s.\r\n", PERS(GUARDING(ch), ch));

  switch (GET_POS(ch))
    {
    case POS_DEAD:
      strcat(buf, "You are DEAD!(Tell a god)\r\n");
      break;
    case POS_MORTALLYW:
      strcat(buf, "You are mortally wounded!  You should seek help!\r\n");
      break;
    case POS_INCAP:
      strcat(buf, "You are incapacitated, slowly fading away...\r\n");
      break;
    case POS_STUNNED:
      strcat(buf, "You are stunned!  You can't move!\r\n");
      break;
    case POS_SLEEPING:
      strcat(buf, "You are sleeping.\r\n");
      break;
    case POS_RESTING:
      strcat(buf, "You are resting.\r\n");
      break;
    case POS_SITTING:
      strcat(buf, "You are sitting.\r\n");
      break;
    case POS_FIGHTING:
      if (FIGHTING(ch))
	sprintf(buf, "%sYou are fighting %s.\r\n", buf, PERS(FIGHTING(ch), ch));
      else
	strcat(buf, "You are fighting thin air.\r\n");
      break;
    case POS_STANDING:
      strcat(buf, "You are standing.\r\n");
      break;
    default:
      strcat(buf, "You are floating.\r\n");
      break;
    }

  if (GET_COND(ch, DRUNK) > 10)
    strcat(buf, "You are intoxicated.\r\n");

  if (GET_COND(ch, FULL) == 0)
    strcat(buf, "You are hungry.\r\n");

  if (GET_COND(ch, THIRST) == 0)
    strcat(buf, "You are thirsty.\r\n");

  if (GET_COND(ch, TIRED) == 0)
    strcat(buf, "You are exhausted, and in desperate need of sleep.\r\n");

  if (IS_AFFECTED(ch, AFF_BLIND))
    strcat(buf, "You have been blinded!\r\n");

  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
    strcat(buf, "You are summonable by other players.\r\n");

  if (IS_AFFECTED(ch, AFF_WEREWOLF))
    strcat(buf, "You're a lycanthrope!\r\n");

  if (IS_AFFECTED(ch, AFF_VAMPIRE))
    strcat(buf, "You're a vampire!\r\n");

  if (IS_AFFECTED(ch, AFF_MOUNT))
    strcat(buf, "You're mounted.\r\n");

  if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
    {
      char *weapon = flesh_alter_weapon(ch);
      sprintf(tmpbuf, "Your hand is a %s!\r\n", weapon);    
      strcat(buf, tmpbuf);
      FREE(weapon);
    }

  stc(buf, ch);
  if (ch->affected)
    {
      int columns = 0;
      strcpy(buf, "Spells affecting you:\r\n");
      for (aff = ch->affected; aff; aff = aff->next)
	{
	  if (aff->bitvector!=AFF_SNEAK &&
	      aff->bitvector!=AFF_DODGE &&
	      (!(aff->bitvector==AFF_KUJI_KIRI && !aff->modifier))&&
	      aff->bitvector!=AFF_BERSERK)
	  {
	   any = TRUE;   /* used to be -20.20 */
	   sprintf(buf, "%s     %s%-24.24s%s", buf,
		  CCCYN(ch, C_NRM), spell_info[aff->type].name, CCNRM(ch, C_NRM));
	   if(!(++columns % 2))
	    strcat(buf, "\r\n");
	  }
	}
      if (any) {
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
      }
     }

  any = FALSE;
  strcpy(buf, "\r\nEquipment spells affecting you:\r\n");

  for (i = 0; i < NUM_WEARS; i++)
  {
    if (GET_EQ(ch, i) && GET_OBJ_BITV(GET_EQ(ch, i)) > 0)
    {
      sprintbit(GET_OBJ_BITV(GET_EQ(ch, i)), affected_names, buf + strlen(buf));
      any = TRUE;
    }
  }
 
  strcat(buf, "\r\n");
  if (any && !IS_NPC(ch))
        send_to_char(buf, ch);

}


ACMD(do_inventory)
{
  send_to_char("You are carrying:\r\n", ch);

  /* 15 means the same as it did last time, go read that comment */
  list_obj_to_char(ch->carrying, ch, 15, TRUE);

}


ACMD(do_equipment)
{
  int i, found = 0;

  send_to_char("You are using:\r\n", ch);
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
        if (IS_MXP(ch))
          send_to_char(where_mxp[i], ch);
        else
          send_to_char(where[i], ch);
        show_obj_to_char(GET_EQ(ch, i), ch, 1);
        found = TRUE;
      } else {
        if (IS_MXP(ch))
          send_to_char(where_mxp[i], ch);
        else
          send_to_char(where[i], ch);
        send_to_char("Something.\r\n", ch);
        found = TRUE;
      }
    }
  }
  if (!found)
    send_to_char(" Nothing.\r\n", ch);
}


ACMD(do_time)
{
  char *suf;
  int weekday, day;
  extern struct time_info_data time_info;

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  weekday = ((DAYS_PER_MONTH * time_info.month) + time_info.day + 1) % DAYS_PER_WEEK;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  suf = "th";
  if (((day % 100) / 10) != 1) {                  /* suffix to the day */
    switch (day % 10) {
    case 1:
      suf = "st";
      break;
    case 2:
      suf = "nd";
      break;
    case 3:
      suf = "rd";
      break;
    }
  }

  sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);
  send_to_char(buf, ch);
  if ( weather_info.sunlight == SUN_DARK ) {
     sprintf(buf, "The moon is %s.\r\n", phases[time_info.moon]);
     send_to_char(buf, ch);
  }
}


ACMD(do_weather)
{
  static char *sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
  "lit by flashes of lightning"};

  if (OUTSIDE(ch))
    {
      sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
	      (weather_info.change >= 0 ? "you feel a warm wind from south" :
	       "the clouds tell you bad weather is due"));
      send_to_char(buf, ch);
    }
  else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}

ACMD(do_help)
{
  extern int top_of_helpt;
  extern struct help_index_element *help_table;
  extern char *help;
  char *filename;
  struct stat fbuf;
  extern int max_filesize;
  FILE * fl;
  char * date;
  time_t ct;

  int chk, bot, top, mid, minlen;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_table) {
    send_to_char("No help available.\r\n", ch);
    return;
  }

  bot = 0;
  top = top_of_helpt;
  minlen = strlen(argument);

  for (;;) {
    mid = (bot + top) / 2;

    if (bot > top) {
      char buf[MAX_INPUT_LENGTH + 33];
      sprintf(buf, "There is no help on: %s\r\n", argument);
      send_to_char(buf, ch);
      sprintf(buf, "HELP: %s attempted to get help on %s", GET_NAME(ch), argument);
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
      filename = HELP_FILE;
      if (stat(filename, &fbuf) < 0) {
        perror("Error statting file");
        return;
      }
      if (fbuf.st_size >= max_filesize)
        return;
      if (!(fl = fopen(filename, "a"))) {
        perror("do_help");
        return;
      }
      ct = time(0);
      date = asctime(localtime(&ct));
      fprintf(fl, "%-8s (%6.6s) [%5ld] %s\n", GET_NAME(ch), (date + 4), world[ch->in_room].number, argument);
      fclose(fl);

      return;
    } else if (!(chk = strn_cmp(argument, help_table[mid].keyword, minlen))) {
      /* trace backwards to find first matching entry. Thanks Jeff Fink! */
      while ((mid > 0) && (!(chk = strn_cmp(argument, help_table[mid - 1].keyword, minlen))))
        mid--;
      if ((GET_LEVEL(ch) < LVL_IMMORT) && strstr(help_table[mid].entry, "wizonly")) {
        sprintf(buf, "There is no help on: %s\r\n", argument);
        send_to_char(buf, ch);
      } else {
        char *entry = help_table[mid].entry;
        char *topic = str_dup(help_table[mid].keyword);
        char *begin, *end, word[MAX_STRING_LENGTH], *new_entry;
        size_t i = 0;

        /* Let's move past the TOPIC identifiers */
        while (*entry != '\n')
          entry++;

        /* HELP file defaults to an MXP client, so we have to remove the &lt; and &gt; and
        replace them with < and > respectively */
        if (!IS_MXP(ch)) {
          CREATE(new_entry, char, strlen(entry) + 1);
          if (strstr(entry, "&lt;") || strstr(entry, "&&gt;")) {
            while (*entry) {
              if (*entry == '&')
                switch (*(++entry)) {
                  case 'l':
                    new_entry[i++] = '<';
                    entry += 3;
                    break;
                  case '&':
                    new_entry[i++] = '>';
                    entry += 4;
                    break;
                  default:
                    new_entry[i++] = *entry;
                    break;
              } else
                new_entry[i++] = *entry++;
            }
          } else
            strcpy(new_entry, entry);
        } else {
          /* MXP elements for additional help */
          CREATE(new_entry, char, MAX_STRING_LENGTH);
          strcpy(new_entry, entry);
          if ((begin = strstr(new_entry, "See Also:"))) {
            *buf = '\0';
            begin += 10;     /* Move past the "See Also:" */
            end = begin;
            while (*end) {
              one_word(end, word);
              if (*end == '\"')
                end += 2;    /* Need to "remove" the quotes */
              for (i = 0; i < strlen(word); i++)
                word[i] = UPPER(word[i]);
              end += i;      /* Which is the strlen(word) */
              skip_spaces(&end);
              sprintf(buf + strlen(buf), "\033[1z<send \"help %s\">%s</send> ", word, word);
            }
            *begin = 0;      /* Erase everything after "See Also:" */
            sprintf(buf2, "%s%s\r\n", new_entry, buf);
            strcpy(new_entry, buf2);
          }
        }

        for (i = 0; i < strlen(topic); i++)
          topic[i] = UPPER(topic[i]);
        
        stc(CCGRN(ch, C_CMP), ch);
        stc("\r\n[ ", ch);
        stc(CCCYN(ch, C_CMP), ch);
        stc(topic, ch);
        stc(CCGRN(ch, C_CMP), ch) ;
        stc(" ]\r\n", ch);
        stc(CCNRM(ch, C_CMP), ch);
        stc(CCRED(ch, C_CMP), ch);
        stc(" --------------------------------------"
            "-------------------------------------", ch);
        stc(CCNRM(ch, C_CMP), ch);
        strcpy(buf3, new_entry);
        page_string(ch->desc, buf3, 0);

        FREE(topic);
        FREE(new_entry);
      }
      return;
    } else {
      if (chk > 0)
        bot = mid + 1;
      else
        top = mid - 1;
    }
  }
}

#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-s] [-o] [-q] \r\n"

ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char name_search[MAX_INPUT_LENGTH];
  char mode;
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
  int short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1); /* just in case; we destroy arg in the switch */
      switch (mode) {
        case 'o':
        case 'k':
          outlaws = 1;
          strcpy(buf, buf1);
          break;
        case 's':
          short_list = 1;
          strcpy(buf, buf1);
          break;
        case 'q':
          questwho = 1;
          strcpy(buf, buf1);
          break;
        case 'l':
          half_chop(buf1, arg, buf);
          sscanf(arg, "%d-%d", &low, &high);
          break;
        case 'n':
          half_chop(buf1, name_search, buf);
          break;
        default:
          send_to_char(WHO_FORMAT, ch);
          return;
          break;
      }    /* end of switch */
    } else {   /* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }    /* end while (parser) */

  send_to_char("Players visible to you\r\n----------------------\r\n", ch);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected)
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) && !strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_OUTLAW))
      continue;
    if (ROOM_FLAGGED(tch->in_room, ROOM_NO_WHO_ROOM) && GET_LEVEL(ch)<LEVEL_IMPL) 
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (short_list) {
      if (GET_LEVEL(tch) >= LEVEL_IMPL) {
        sprintf(buf, "%s[ *IMP*  ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else if (GET_LEVEL(tch) >= LEVEL_GRGOD) {
        sprintf(buf, "%s[ GRGOD  ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else if (GET_LEVEL(tch) >= LEVEL_HIGOD) {
        sprintf(buf, "%s[ HIGOD  ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else if (GET_LEVEL(tch) >= LEVEL_LEGEND) {
        sprintf(buf, "%s[ LEGEND ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else if (GET_LEVEL(tch) >= LEVEL_GOD) {
        sprintf(buf, "%s[  GOD   ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else if (GET_LEVEL(tch) >= LEVEL_IMMORT + 1) {
        sprintf(buf, "%s[ TITAN  ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else if (GET_LEVEL(tch) >= LEVEL_IMMORT) {
        sprintf(buf, "%s[ IMMORT ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      } else {
        sprintf(buf, "%s[ %2d ] %-12.12s%s%s",
          CCYEL(ch, C_SPR), GET_LEVEL(tch), GET_NAME(tch), CCNRM(ch, C_SPR),
          ((!(++num_can_see % 4)) ? "\n\r" : ""));
        send_to_char(buf, ch);
      }
    } else {
      num_can_see++;

      if (GET_LEVEL(ch) >= LVL_IMMORT)
        sprintf(buf + strlen(buf), "[%3d] ", GET_LEVEL(tch));

      /* '+' in who denotes staff */
      if (GET_LEVEL(tch) >= LEVEL_IMMORT)
        sprintf(buf + strlen(buf), "&r+%s %s", GET_NAME(tch), GET_TITLE(tch));
      else if (IS_CHOSEN(tch))
        sprintf(buf + strlen(buf), "&y+%s %s", GET_NAME(tch), GET_TITLE(tch));
      else
        sprintf(buf + strlen(buf), "%s %s", GET_NAME(tch), GET_TITLE(tch));
        
      if (GET_INVIS_LEV(tch) && GET_LEVEL(ch)>=LEVEL_IMMORT)
        sprintf(buf + strlen(buf), " (i%d)", GET_INVIS_LEV(tch));
      else if (IS_AFFECTED(tch, AFF_INVISIBLE))
        strcat(buf, " (invis)");

      if (PLR_FLAGGED(tch, PLR_MAILING))
        strcat(buf, " (mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
        strcat(buf, " (writing)");

      if ((GET_LEVEL(ch) >= LVL_IMMORT) && (PLR_FLAGGED(tch, PLR_DEBUG)))
        strcat(buf, " (DEBUG)");
      if (PRF_FLAGGED(tch, PRF_DEAF))
        strcat(buf, " (deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
        strcat(buf, " (notell)");
      if (PRF_FLAGGED(tch, PRF_QUEST))
        strcat(buf, " (quest)");
      if (PRF_FLAGGED(tch, PRF_AFK))
        strcat(buf, " (AFK)");
      if (PRF_FLAGGED(tch, PRF_INACTIVE))
        strcat(buf, " (INACTIVE)");
      if (PLR_FLAGGED(tch, PLR_IT))
        strcat(buf, " (IT)");
      if (PLR_FLAGGED(tch, PLR_OUTLAW))
        strcat(buf, " (OUTLAW)");
      if (GET_LEVEL(tch) >= LVL_IMMORT || IS_CHOSEN(tch)) /* end coloured text */
        strcat(buf, "&n");

      strcat(buf, "\r\n");
    }    
  } 

  if (num_can_see == 0)
    sprintf(buf, "%s\r\nNo-one at all!\r\n", buf);
  else if (num_can_see == 1)
    sprintf(buf, "%s\r\nOne character displayed, ", buf);
  else
    sprintf(buf, "%s\r\n%d characters displayed, ", buf, num_can_see);

  sprintf(buf, "%smaximum of %d characters this session.\r\n", buf, max_play);
  if (!short_list)
    page_string(ch->desc, buf, 1);
  if (short_list && (num_can_see % 4)) {
    send_to_char(buf, ch);
    send_to_char("\r\n", ch);
  }
}

#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-o] [-p]\r\n"

ACMD(do_users)
{
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, *format, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  int low = 0, high = LVL_IMPL, num_can_see = 0;
  int outlaws = 0, playing = 0, deadweight = 0;
  char buffer[MAX_STRING_LENGTH];

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
	mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
	switch (mode) {
	  case 'o':
	  case 'k':
	    outlaws = 1;
	    playing = 1;
	    strcpy(buf, buf1);
	    break;
	  case 'p':
	    playing = 1;
	    strcpy(buf, buf1);
	    break;
	  case 'd':
	    deadweight = 1;
	    strcpy(buf, buf1);
	    break;
	  case 'l':
	    playing = 1;
	    half_chop(buf1, arg, buf);
	    sscanf(arg, "%d-%d", &low, &high);
	    break;
	  case 'n':
	    playing = 1;
	    half_chop(buf1, name_search, buf);
	    break;
	  case 'h':
	    playing = 1;
	    half_chop(buf1, host_search, buf);
	    break;
	  default:
	    send_to_char(USERS_FORMAT, ch);
	    return;
	    break;
	}				/* end of switch */
    } else {			/* endif */
	send_to_char(USERS_FORMAT, ch);
	return;
    }
  }				/* end while (parser) */
  strcpy(line,
	 "Num Level Name       State       Idl Login Site\r\n");
  strcat(line,
	 "--- ----- ---------- ----------- --- ----- --------------------------------\r\n");
  strcpy(buffer, line);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && playing)
      continue;
    if (STATE(d) == CON_PLAYING && deadweight)
      continue;
    if (IS_PLAYING(d)) {
      if (d->original)
        tch = d->original;
      else if (!(tch = d->character))
	  continue;
      if (*host_search && !strstr(d->host, host_search))
        continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
        continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
        continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_OUTLAW))
        continue;
      if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
        continue;

      if (d->original)
        sprintf(classname, "[%3d]", GET_LEVEL(d->original));
      else
        sprintf(classname, "[%3d]", GET_LEVEL(d->character));
    } else
	strcpy(classname, " - ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 5) = '\0';

    if (STATE(d) == CON_PLAYING && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[STATE(d)]);

    if (d->character && STATE(d) == CON_PLAYING && ((GET_LEVEL(d->character) < GET_LEVEL(ch)) ||
      GET_LEVEL(ch) == LVL_IMPL))
      sprintf(idletime, "%3d", d->character->char_specials.timer);
      /* Doragar: Removed (* SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN) after timer */
    else
      strcpy(idletime, "");

    format = "%3d %-5s %-10s %-11s %-3s %-5s ";

    if (d->character && d->character->player.name) {
      if (d->original)
	  sprintf(line, format, d->desc_num, classname, d->original->player.name, state, 
          idletime, timeptr);
	else
	  sprintf(line, format, d->desc_num, classname, d->character->player.name, state,
          idletime, timeptr);
    } else
	sprintf(line, format, d->desc_num, "   -  ", "UNDEFINED", state, idletime, timeptr);

    if (d->host && *d->host)
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
    else
      strcat(line, "[Hostname unknown]\r\n");

    if (STATE(d) != CON_PLAYING) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }

    if (STATE(d) != CON_PLAYING ||
		(STATE(d) == CON_PLAYING && CAN_SEE(ch, d->character))) {
      strcat(buffer, line);
      num_can_see++;
    }
  }

  sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
  strcat(buffer, line);
  page_string(ch->desc, buffer, TRUE);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  extern char * compile_time_str;
  extern time_t compile_time;
  extern int version_major;
  extern int version_minor;
  extern int version_build;
  extern bool daylight_saving_time;
  int i, count = 0;
  time_t t;
  int zone = 0, dst;

  switch (subcmd)
    {
    case SCMD_CREDITS:
      page_string(ch->desc, credits, 0);
      break;
    case SCMD_NEWS:
      page_string(ch->desc, news, 0);
      break;
    case SCMD_INFO:
      page_string(ch->desc, info, 0);
      break;
    case SCMD_WIZLIST:
      page_string(ch->desc, wizlist, 0);
      break;
    case SCMD_IMMLIST:
      page_string(ch->desc, immlist, 0);
      break;
    case SCMD_HANDBOOK:
      page_string(ch->desc, handbook, 0);
      break;
    case SCMD_POLICIES:
      page_string(ch->desc, policies, 0);
      break;
    case SCMD_FUTURE:
      page_string(ch->desc, future, 0);
      break;
    case SCMD_MOTD:
      if (IS_MXP(ch))
        page_string(ch->desc, motd_mxp, 0);
      else
        page_string(ch->desc, motd, 0);
      break;
    case SCMD_IMOTD:
      page_string(ch->desc, imotd, 0);
      break;
    case SCMD_CLEAR:
      send_to_char("\033[H\033[J", ch);
      break;
    case SCMD_VERSION:
      send_to_char(circlemud_version, ch);
      sprintf(buf, "Currently Running: v%d.%d.%d\r\n", version_major,
              version_minor, version_build);
      send_to_char(buf, ch);
      if (GET_LEVEL(ch) >= LVL_IMMORT) {
         t = compile_time + 60 * time_zone_table[zone & TZ_LOWBITS].offset;
         dst = (zone & TZ_DST_ON) || (!(zone & TZ_DST_OFF) && zone > 1 &&
               daylight_saving_time);
         if (dst)
            t += 3600;
         strftime(buf1, 50, "%A, %B %d, %Y   %H:%M", gmtime(&t));
         if (compile_time_str) {
            sprintf(buf, "Compile Time: %s\r\n", compile_time_str);
            send_to_char(buf, ch);
         }
      }
      break;
    case SCMD_WHOAMI:
      send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
      break;
    case SCMD_PLAYER_LIST:
      stc("A list of registered players:\r\n", ch);
      for (i = 0; i <= top_of_p_table ; i++) {
        if (*(player_table + i)->name) {
          sprintf(buf, "%-19.19s", (player_table +i)->name);
          stc(buf, ch);
          count++;
          if (count == 4) {
            count = 0;
            stc("\r\n", ch);
          }
        }
      }
      stc("\r\n", ch);
      break ;
    default:
      return;
      break;
    }
}


char *print_object_location(int num, struct obj_data * obj, struct char_data * ch)
{
  static char buf[128];

  sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);

  if (obj->in_room > NOWHERE) {
    if (IS_OBJ_STAT(obj, ITEM_BURIED))
      sprintf(buf + strlen(buf), "buried in %s [%5ld]\r\n",
        world[obj->in_room].name, GET_ROOM_VNUM(IN_ROOM(obj)));
    else
      sprintf(buf + strlen(buf), "[%5ld] %s\r\n", GET_ROOM_VNUM(IN_ROOM(obj)), 
        world[obj->in_room].name);
  } else if (obj->carried_by) {
      sprintf(buf + strlen(buf), "carried by %s\n\r", PERS(obj->carried_by, ch));
  } else if (obj->worn_by) {
      sprintf(buf + strlen(buf), "worn by %s\n\r", PERS(obj->worn_by, ch));
  } else if (obj->in_obj) {
      sprintf(buf + strlen(buf), "inside %s which is", obj->in_obj->short_description);
      if (IN_ROOM(obj->in_obj) > NOWHERE)     /* Object is in a room */
        sprintf(buf + strlen(buf), " in\r\n%33s [%5ld] %s\r\n", " -",
          world[IN_ROOM(obj->in_obj)].number, world[IN_ROOM(obj->in_obj)].name);
      else if (obj->in_obj->in_obj)           /* Container is within another object */
        sprintf(buf + strlen(buf), "\r\n%33s inside %s\r\n", " ", 
          obj->in_obj->in_obj->short_description);
      else if (obj->in_obj->carried_by)       /* Object is carried by a char */
        sprintf(buf + strlen(buf), "\r\n%33s carried by %s\r\n", " ",
          PERS(obj->in_obj->carried_by, ch));
      else
        sprintf(buf + strlen(buf), "\r\n%33s worn by %s\r\n", " ",
          PERS(obj->in_obj->worn_by, ch));
  } else {
      sprintf(buf + strlen(buf), "in an unknown location\n\r");
  }

  return buf;
}

/* FIX FIX To be removed from production 3.0 release! */
ACMD(do_levels)
{
  int i, exp = 0;

  if (IS_NPC(ch))
    {
      send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
      return;
    }
  *buf = '\0';

  for (i = 1; i < LVL_IMMORT; i++)
  {
    exp += find_exp(i-1);
    sprintf(buf + strlen(buf), "[%2d] %8d-%-8d    (%6d)\r\n", i,
	    exp, find_exp(i) + exp,
	    find_exp(i));
  }
  page_string(ch->desc, buf, 1);
}

ACMD(do_consider)
{
  struct char_data *victim;
  int condition = 0, damdiff = 0, leveldiff = 0, victdam = 0, chardam = 0, hitchance = 0;
  struct obj_data *charwielded = GET_EQ(ch, WEAR_WIELD);
  struct obj_data *victwielded = NULL;
  char buf[MAX_STRING_LENGTH];

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("Consider killing who?\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("Easy!  Very easy indeed!\r\n", ch);
    return;
  }

  victwielded = GET_EQ(victim, WEAR_WIELD);

  chardam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
  victdam = str_app[STRENGTH_APPLY_INDEX(victim)].todam;
  chardam += GET_DAMROLL(ch);
  victdam += GET_DAMROLL(victim);

  if (!IS_NPC(ch) && charwielded)
    chardam += dice(GET_OBJ_VAL(charwielded, 1), GET_OBJ_VAL(charwielded, 2));
  else {
    if (IS_NPC(ch)) 
     chardam += dice(GET_NDD(ch), GET_SDD(ch));
    else/* Max. lvl/3 dam with bare hands */
     chardam += number(0, GET_LEVEL(ch)/3);
  }
  
  if (!IS_NPC(victim) && victwielded)
    victdam += dice(GET_OBJ_VAL(victwielded, 1), GET_OBJ_VAL(victwielded, 2));
  else {
    if (IS_NPC(victim)) 
     victdam += dice(victim->mob_specials.damnodice, victim->mob_specials.damsizedice);
    else/* Max. lvl/3 dam with bare hands */
     victdam += number(0, GET_LEVEL(victim)/3);
  }

  condition = GET_HIT(victim) - GET_HIT(ch);
  damdiff = victdam-chardam;
  leveldiff = GET_LEVEL(victim) - GET_LEVEL(ch);
  hitchance = pow(((leveldiff < 0) ? (-leveldiff) : (leveldiff)), 1.5) * 1.6;
  if (leveldiff < 0)
    hitchance += 50;
  else
    hitchance = -hitchance + 50;

  if (damdiff > 20)
    strcpy(buf, "$N looks like $E could eat you for lunch, ");
  else if (damdiff > 10)
    strcpy(buf, "$N looks like $E could tear you up in a fight, ");
  else if (damdiff > 5)
    strcpy(buf, "$N looks like $E could hurt you in a fight, ");
  else if (damdiff > -3)
    strcpy(buf, "$N looks like a fair fight, ");
  else if (damdiff > -5)
    strcpy(buf, "$N looks like an easy kill, ");
  else if (damdiff > -10)
    strcpy(buf, "$N looks like a very easy kill, ");
  else
    strcpy(buf, "$N might not even be worth the effort to kill, ");
  
  if (condition > 4 * GET_HIT(ch))
    strcat(buf, "looks to be\r\nin much better physical shape than you, ");
  else if (condition > 2 * GET_HIT(ch))
    strcat(buf, "looks to be\r\nin a lot better physical shape than you, ");
  else if (condition > GET_HIT(ch))
    strcat(buf, "looks to be\r\nin better physical shape than you, ");
  else if (condition >= 0)
    strcat(buf, "looks to be\r\nin about the same physical shape as you, ");
  else if (condition > -GET_HIT(ch))
    strcat(buf, "looks to be\r\nin a little worse physical shape as you, ");
  else if (condition > -(2 * GET_HIT(ch)))
    strcat(buf, "looks to be\r\nin a worse physical shape than you, ");    
  else
    strcat(buf, "looks to be\r\nin a lot worse physical shape than you, ");

  if (leveldiff > GET_LEVEL(ch))
    strcat(buf, "and moves with an ease telling\r\nof many won battles.");
  else if (leveldiff > GET_LEVEL(ch)/2)
    strcat(buf, "and seems to know $S opponent, ");
  else if (leveldiff > -1)
    strcat(buf, "and seems about as confident in\r\nbattle as you do.");
  else if (leveldiff > -3)
    strcat(buf, "and seems less confident in\r\nbattle than you do.");
  else if (leveldiff > -5)
    strcat(buf, "and seems much less confident in\r\nbattle than you do.");
  else if (leveldiff > -7)
    strcat(buf, "and seems ready to run from a\r\nfight.");
  else
    strcat(buf, "and seems like $E's never been\r\nin battle before.");

  if (hitchance >= 100)
    strcat(buf, " You should hit $M with every blow!");
  else if (hitchance > 80)
    strcat(buf, " You have a very good chance of hitting $M.");
  else if (hitchance > 60)
    strcat(buf, " You have a good chance of hitting $M.");
  else if (hitchance >= 45)
    strcat(buf, " You have an even chance of hitting $M.");
  else if (hitchance > 30)
    strcat(buf, " You have a less than even chance of hitting $M.");
  else if (hitchance > 20)
    strcat(buf, " You have a poor chance of hitting $M.");
  else if (hitchance > 10)
    strcat(buf, " You have a very poor chance of hitting $M.");
  else
    strcat(buf, " You have virtually no chance of hitting $M.");
  act(buf, TRUE, ch, 0, victim, TO_CHAR);
  diag_char_to_char(victim, ch);
}

static char *ctypes[] = {
"off", "sparse", "normal", "complete", "\n"};

ACMD(do_color)
{
  int tp;

  if (IS_NPC(ch))
    return;

  any_one_arg(argument, arg);
  
  if (!strcmp(arg, "on"))
   {
	do_color(ch, "complete", 0, 0);
	return;
   }

  if (!*arg)
    {
      sprintf(buf, "Your current color level is %s.\r\n",
	      ctypes[COLOR_LEV(ch)]);
      send_to_char(buf, ch);
      return;
    }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1))
    {
      send_to_char("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
      return;
    }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1);
  REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_2);
  if (tp & 1) SET_BIT(PRF_FLAGS(ch), PRF_COLOR_1);
  if (tp & 2) SET_BIT(PRF_FLAGS(ch), PRF_COLOR_2);

  sprintf(buf, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR),
	  CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  char buf3[20];
  int i, j;
  struct toggle_struct {
    char *cmd;
    int subcmd;
    char level;
  } toggle[] = {
    { "gossip",     SCMD_NOGOSSIP,  0 },  /* 0 */
    { "shout",      SCMD_DEAF,      0 },
    { "broadcast",  SCMD_NOBROAD,   0 },
    { "auction",    SCMD_NOAUCTION, 0 },
    { "quest",      SCMD_QUEST,     0 },
    { "tell",       SCMD_NOTELL,    0 },  /* 5 */
    { "ctell",      SCMD_NOCTELL,   0 },
    { "grats",      SCMD_NOGRATZ,   0 },
    { "mxp",        SCMD_MXP,       0 },
    { "compact",    SCMD_COMPACT,   0 },
    { "brief",      SCMD_BRIEF,     0 },  /* 10 */
    { "loot",       0,              0 },
    { "gold",       0,              0 },
    { "split",      0,              0 },
    { "exits",      0,              0 },
    { "repeat",     SCMD_NOREPEAT,  0 },  /* 15 */
    { "hassle",     SCMD_NOHASSLE,  LVL_IMMORT },
    { "roomflags",  SCMD_ROOMFLAGS, LVL_IMMORT },
    { "holylight",  SCMD_HOLYLIGHT, LVL_IMMORT },
    { "sql",        SCMD_NOSQL,     LVL_GRGOD },
    { "wiznet",     SCMD_NOWIZ,     LVL_IMMORT },  /* 20 */
    { "slowns",     SCMD_SLOWNS,    LVL_IMPL },
    { "msp",        SCMD_MSP,       0 },
    { "mccp",       SCMD_MCCP,      0 },
    { "\n",         0,              0 }
  };

  ACMD(do_auto);
  ACMD(do_gen_tog);
  extern int sql_wipe;
  extern ubyte nameserver_is_slow;

  if (IS_NPC(ch))
    return;

  skip_spaces(&argument);
  if (!*argument) {
    if (GET_WIMP_LEV(ch) == 0)
      strcpy(buf2, "OFF");
    else
      sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

    strcpy(buf3, ctypes[COLOR_LEV(ch)]);

    sprintf(buf,
      "&WCommunications&n\r\n"
      "    Gossip: %s%-3s%s\t     Broadcast: %s%-3s%s\t        Tell: %s%-3s%s\r\n"
      "   Auction: %s%-3s%s\t         Quest: %s%-3s%s\t   Clan Tell: %s%-3s%s\r\n"
      "     Shout: %s%-3s%s\t         Grats: %s%-3s%s\r\n"

      "\r\n&WDisplay&n\r\n"
      "Hit Points: %s%-3s%s\t Dsp Tank Stat: %s%-3s%s\tCompact Mode: %s%-3s%s\r\n"
      "      Mana: %s%-3s%s\tDsp Fight Stat: %s%-3s%s\t  Brief Mode: %s%-3s%s\r\n"
      "     Moves: %s%-3s%s\t           MXP: %s%-3s%s\t Color Level: %s%s%s\r\n"

      "\r\n&WPreferences&n\r\n"
      " Auto Loot: %s%-3s%s\t    Auto Exits: %s%-3s%s\t         MSP: %s%-3s%s\r\n"
      " Auto Gold: %s%-3s%s\t    Wimp Level: %s%-3s%s\r\n"
      "Auto Split: %s%-3s%s\t  Repeat Comms: %s%-3s%s\r\n",

      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOBROAD)), CCNRM(ch, C_NRM), 
      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOTELL)), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_QUEST)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOCTELL)), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_DEAF)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_DISPTANK)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), YESNO(PRF_FLAGGED(ch, PRF_COMPACT)), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_DISPTARGET)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), YESNO(PRF_FLAGGED(ch, PRF_BRIEF)), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(IS_SET(ch->desc->bits, DESC_MXP)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), CAP(buf3), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), YESNO(PRF_FLAGGED(ch, PRF_AUTOLOOT)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), YESNO(PRF_FLAGGED(ch, PRF_AUTOEXIT)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), ONOFF(IS_SET(ch->desc->bits, DESC_MSP)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), YESNO(PRF_FLAGGED(ch, PRF_AUTOGOLD)), CCNRM(ch, C_NRM),

      CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), YESNO(PRF_FLAGGED(ch, PRF_AUTOSPLIT)), CCNRM(ch, C_NRM),
      CCGRN(ch, C_NRM), YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)), CCNRM(ch, C_NRM));

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      sprintf(buf1,
        "\r\n&WImmortal Preferences&n\r\n"
        "  NoHassle: %s%-3s%s\t         Wiznet: %s%-3s%s\r\n"
        "Room Flags: %s%-3s%s\t      Erase SQL: %s%-3s%s\r\n"
        " Holylight: %s%-3s%s\tSlow Nameserver: %s%-3s%s\r\n",

        CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_NOHASSLE)), CCNRM(ch, C_NRM),
        CCGRN(ch, C_NRM), ONOFF(!PRF_FLAGGED(ch, PRF_NOWIZ)), CCNRM(ch, C_NRM),
        CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_ROOMFLAGS)), CCNRM(ch, C_NRM),

        CCGRN(ch, C_NRM), YESNO(sql_wipe), CCNRM(ch, C_NRM),
        CCGRN(ch, C_NRM), ONOFF(PRF_FLAGGED(ch, PRF_HOLYLIGHT)), CCNRM(ch, C_NRM),
        CCGRN(ch, C_NRM), YESNO(nameserver_is_slow), CCNRM(ch, C_NRM));
      strcat(buf, buf1);
    }
    send_to_char(buf, ch);
    return;
  } else {
    for (i = 0; *(toggle[i].cmd) != '\n'; i++)
      if (!strncmp(argument, toggle[i].cmd, strlen(argument)))
        break;

    if (GET_LEVEL(ch) < toggle[i].level) {
      send_to_char("That's not a valid toggle command!\r\n", ch);
      return;
    }

    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
        do_gen_tog(ch, argument, 0, toggle[i].subcmd);
        break;
      case 11:
      case 12:
      case 13:
      case 14:
        do_auto(ch, argument, 0, 0);
        break;
      default:
        send_to_char("Valid toggle commands are:\r\n\r\n", ch);
        for (i = 0, j = 0; *(toggle[i].cmd) != '\n'; i++)
          if (GET_LEVEL(ch) >= toggle[i].level) {
            sprintf(buf, "%-15s", toggle[i].cmd);
            if (!(++j % 5))
              strcat(buf, "\r\n");
            send_to_char(buf, ch);
          }
        send_to_char("\r\n", ch);
        return;
    }
  }
}


struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void
sort_commands(void)
{
  int a, b, tmp;

  ACMD(do_action);

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++)
    {
      cmd_sort_info[a].sort_pos = a;
      cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
    }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0)
	{
	  tmp = cmd_sort_info[a].sort_pos;
	  cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	  cmd_sort_info[b].sort_pos = tmp;
	}
}


ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg)
    {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict))
      {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict))
      {
	send_to_char("You can't see the commands of people "
		     "above your level.\r\n", ch);
	return;
      }
  }
  else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++)
    {
      i = cmd_sort_info[cmd_num].sort_pos;
      if (cmd_info[i].minimum_level >= 0 &&
	  GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
	  (cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
	  (wizhelp || socials == cmd_sort_info[i].is_social))
	{
	  sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
	  if (!(no % 7))
	    strcat(buf, "\r\n");
	  no++;
	}
    }

  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}


ACMD(do_coins)
{
  int coins = GET_GOLD(ch), bank = GET_BANK_GOLD(ch);
  *buf = '\0';
  
  sprintf(buf, "You are currently carrying %d coins,\r\n", coins);
  send_to_char(buf, ch);
  sprintf(buf, "and in your bank account, you have %d coins.\r\n", bank);
  send_to_char(buf, ch);
  sprintf(buf, "Your current net-worth is %d coins.\r\n", coins+bank);
  send_to_char(buf, ch);
}


/* do_description:
   shows room description *description
   to character *ch
*/
void do_description(char *description, struct char_data *ch, int subcmd) {
  struct extra_descr_data *i;
  struct obj_data *obj;
  int num_extras = 0, j;
  char extras[60][255];
  char scratch[MAX_STRING_LENGTH];
  char *token;
  char *begin;
  char *end;
  char mxp_begin[MAX_STRING_LENGTH];
  char rem1, rem2;


  switch(subcmd) {
    case SCMD_ROOM:
      for (i = world[ch->in_room].ex_description; i; i = i->next) {
        strcpy(buf, i->keyword);
        token = strtok(buf, " .,;:!");
        num_extras++;
        strcpy(extras[num_extras], token);
        while ((token = strtok(NULL, " .,;:!"))) {
          num_extras++;
          strcpy(extras[num_extras], token);
        }
      }
      break;
    case SCMD_OBJ_EQ:
      for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
          for (i = GET_EQ(ch, j)->ex_description; i; i = i->next) {
            strcpy(buf, i->keyword);
            token = strtok(buf, " .,;:!");
            num_extras++;
            strcpy(extras[num_extras], token);
            while ((token = strtok(NULL, " .,;:!"))) {
              num_extras++;
              strcpy(extras[num_extras], token);
            }
          }
      break;
    case SCMD_OBJ_INV:
      for (obj = ch->carrying; obj; obj = obj->next_content)
        if (CAN_SEE_OBJ(ch, obj))
          for (i = obj->ex_description; i; i = i->next) {
            strcpy(buf, i->keyword);
            token = strtok(buf, " .,;:!");
            num_extras++;
            strcpy(extras[num_extras], token);
            while ((token = strtok(NULL, " .,;:!"))) {
              num_extras++;
              strcpy(extras[num_extras], token);
            }
          }
      break;
    case SCMD_OBJ_ROOM:
      for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
        if (CAN_SEE_OBJ(ch, obj))
          for (i = obj->ex_description; i; i = i->next) {
            strcpy(buf, i->keyword);
            token = strtok(buf, " .,;:!");
            num_extras++;
            strcpy(extras[num_extras], token);
            while ((token = strtok(NULL, " .,;:!"))) {
              num_extras++;
              strcpy(extras[num_extras], token);
            }
          }
      break;
    default:
      sprintf(buf, "SYSERR: Unknown subcmd passed to do_description.");
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      send_to_char(description, ch);
      return;
  }

  if (num_extras == 0) {
    send_to_char(description, ch);
    return;
  }

  sprintf(buf2, "%s ", description);
  if (COLOR_LEV(ch) > 0) {
     for (j = 1; j <= num_extras; j++) {
        if ((begin = strstr(buf2, extras[j]))) {
          end = begin;
          while (!isspace(*end) && !ispunct(*end)) end++;
          begin--;
          rem1 = *begin;
          rem2 = *end;
          *begin = *end = 0;
          begin++; end++;
          if (IS_MXP(ch))
          {
            sprintf(mxp_begin, "[1z<send \"look %s\">%s</send>", begin, begin);
            sprintf(scratch, "%s%s%c%s%c%s%s",
                   buf2, VT_BOLDTEX, rem1, mxp_begin, rem2, VT_NORMALT, end);
          }
          else
            sprintf(scratch, "%s%s%c%s%c%s%s",
                   buf2, VT_BOLDTEX, rem1, begin, rem2, VT_NORMALT, end);
          strcpy(buf2, scratch);
        }
     }    
  }

  send_to_char(buf2, ch);
}

ACMD(do_examine) 
{
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;
         
  one_argument(argument, arg);
          
  if (!*arg) { 
    send_to_char("Examine what?\r\n", ch);
    return;
  }
  look_at_target(ch, arg);
 
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
                      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
  
  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
        (GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
        (GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      if (IS_SET(GET_OBJ_VAL(tmp_object, 1), CONT_CLOSED)) {
        OPEN_DOOR(ch->in_room, tmp_object);    /* Prevents Okay message */
        act("$p is closed, but you open it.", TRUE, ch, tmp_object, 0, TO_CHAR);
      }
      send_to_char("When you look inside, you see:\r\n", ch);
      look_in_obj(ch, arg);
    }
  }
}

ACMD(do_diagnose)
{
  struct char_data *vict;
        
  one_argument(argument, buf);
 
  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    } else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("Diagnose who?\r\n", ch);
  }
}
