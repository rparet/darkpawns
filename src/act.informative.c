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

/* $Id: act.informative.c 1515 2008-06-06 21:31:34Z jravn $ */

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
#include "oc.h"
#include "clan.h"
#include "utils.h"
#include "vt100.h"
#include "timezone.h"
#include "svn_revision.h"

#include <sys/stat.h>

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct spell_info_type spell_info[];
extern char *races[];
extern char *pc_class_types[];
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char *titles[NUM_CLASSES];
extern struct index_data *obj_index;
extern int isname_with_abbrevs(char *str, char *namelist);

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *future;
extern char *dirs[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *connected_types[];
extern char *class_abbrevs[];
extern char *room_bits[];
extern char *spells[];
extern char *abil_names[];
ACMD(do_look);
void kender_steal(struct char_data *ch, struct char_data *victim);
void do_description(char *, struct char_data *);
struct char_data *get_rider(struct char_data *mount);
struct char_data *get_mount(struct char_data *ch);
int find_target_room(struct char_data *ch, char *rawroomstr);
long find_class_bitvector(char arg);
extern char *phases[];
extern char *hometowns[];
int exp_needed_for_level(struct char_data *ch);
int find_exp( int class, int level );
extern char *tattoos[];

void
show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
  bool found;

  *buf = '\0';
  if ((mode == 0) && object->description)
    strcpy(buf, object->description);
  else if (object->short_description &&
	   ((mode == 1) || (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buf, object->short_description);
  else if (mode == 5)
    {
      if (GET_OBJ_TYPE(object) == ITEM_NOTE)
	{
	  if (object->action_description)
	    {
	      strcpy(buf, "There is something written upon it:\r\n\r\n");
	      strcat(buf, object->action_description);
	      page_string(ch->desc, buf, 1);
	    }
	  else
	    act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
	  return;
	}
      else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON)
	strcpy(buf, "You see nothing special..");
      else			/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
	strcpy(buf, "It looks like a drink container.");
    }
  if (mode != 3)
    {
      found = FALSE;
      if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
	{
	  strcat(buf, " (invisible)");
	  found = TRUE;
	}
      if (IS_OBJ_STAT(object, ITEM_BLESS) && IS_AFFECTED(ch, AFF_DETECT_ALIGN))
	{
          strcat(buf, " (");
          if (COLOR_LEV(ch)==C_CMP)
            strcat(buf, KBLU);
          strcat(buf, "blue glow");
          if (COLOR_LEV(ch)==C_CMP)
            strcat(buf, KNRM);
          strcat(buf, ")");
	  found = TRUE;
	}
      if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC))
	{
          strcat(buf, " (");
          if (COLOR_LEV(ch)==C_CMP)
            strcat(buf, KYEL);
          strcat(buf, "yellow glow");
          if (COLOR_LEV(ch)==C_CMP)
            strcat(buf, KNRM);
          strcat(buf, ")");
	  found = TRUE;
	}
      if (IS_OBJ_STAT(object, ITEM_GLOW))
	{
	  strcat(buf, " (");
	  if (COLOR_LEV(ch)==C_CMP)
	    strcat(buf, KWHT);
	  strcat(buf, "glowing");
	  if (COLOR_LEV(ch)==C_CMP)
	    strcat(buf, KNRM);
	  strcat(buf, ")");
	  found = TRUE;
	}
      if (IS_OBJ_STAT(object, ITEM_HUM))
	{
	  strcat(buf, " (humming)");
	  found = TRUE;
	}
      if (object->worn_on)
        if (IS_COVERED(object->worn_by, object->worn_on))
	{
	  strcat(buf, " (covered)");
	  found = TRUE;
	}
    }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}


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
      if (object->obj_flags.type_flag == ITEM_NOTE)
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
      else if((object->obj_flags.type_flag != ITEM_DRINKCON))
	strcpy(buffer,"You see nothing special..");
      else /* ITEM_TYPE == ITEM_DRINKCON */
	strcpy(buffer, "It looks like a drink container.");
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
    }

  if (num>1)
    {
      sprintf(tmp,"[%d]", num);
      strcat(buffer, tmp);
    }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void
list_obj_to_char(struct obj_data *list, struct char_data *ch,
		 int mode, bool show)
{
  struct obj_data *i;
  bool found = FALSE;
  struct howmany *temp = NULL;
  struct howmany *head = NULL;
  char text[256];

  for (i = list; i; i = i->next_content)
    {
      if (CAN_SEE_OBJ(ch, i))
	{
	  if (mode & 16)
	    strcpy(text, i->description);
	  else
	    strcpy(text, i->short_description);

	  if (! oc_onlist(GET_OBJ_VNUM(i),
                          i->obj_flags.extra_flags, GET_OBJ_WEIGHT(i), text, head))
	    {
	      temp = oc_get_node();

	      if (mode & 16)
		strcpy(temp->text, i->description);
	      else
		strcpy(temp->text, i->short_description);

	      temp->weight = GET_OBJ_WEIGHT(i);
	      temp->vnum = GET_OBJ_VNUM(i);
	      temp->count = 1;
	      temp->extras = *i->obj_flags.extra_flags;
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
	  /* sprintf(buf,"[%2d] ",Num_Inventory++);
	     send_to_char(buf,ch);*/
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

  if (!IS_NPC(i))
    {
      if (IS_AFFECTED(i, AFF_FLESH_ALTER))
        {
          char *weapon = flesh_alter_weapon(i);
          sprintf(buf, "%s is %s, but %s hand is a %s!\r\n",PERS(i,ch),
                  races[GET_RACE(i)], HSHR(i), weapon);
          FREE(weapon);
        }
      else
        sprintf(buf, "%s is %s.\r\n",PERS(i,ch), races[GET_RACE(i)]);
      send_to_char(buf, ch);
    }

  if (GET_TATTOO(i))
   {
     sprintf(buf, "On %s right arm is a tattoo %s... it %sglows%s softly.\r\n",
	     HSHR(i), tattoos[GET_TATTOO(i)],
	     CCWHT(ch, C_CMP), CCNRM(ch, C_CMP));
     send_to_char(buf, ch);
   }
  if (IS_AFFECTED(i, AFF_VAMPIRE))
  {
     sprintf(buf, "Looking at %s closely, you see two white fangs --  %s is"
	" a vampire!\r\n", HMHR(i), HSSH(i));
     stc(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_WEREWOLF))
  {
     sprintf(buf, "May the gods have mercy --  %s is a werewolf!\r\n", HSSH(i));
     stc(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_METALSKIN))
  {
    sprintf(buf, "You notice %s skin has a metallic hue!\r\n", HSHR(i));
    stc(buf, ch);
  }
  if (IS_AFFECTED(i, AFF_CUTTHROAT))
  {
    sprintf(buf, "%s's throat has been slit from ear to ear!\r\n", PERS(i, ch));
    stc(buf, ch);
  }

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found)
    {
      act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j = 0; j < NUM_WEARS; j++)
	if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
	  {
	    if (!IS_COVERED(i, j) || ch==i|| GET_LEVEL(ch)>LVL_IMMORT)
	    {
	      send_to_char(where[j], ch);
	      show_obj_to_char(GET_EQ(i, j), ch, 1);
	    }
	  }
    }
  if (ch != i && (GET_CLASS(ch) == CLASS_THIEF ||
	GET_LEVEL(ch) >= LVL_IMMORT ||
	GET_CLASS(ch) == CLASS_ASSASSIN))
    {
      found = FALSE;
      act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
      if(!IS_NPC(i) &&
	 (GET_LEVEL(i)>LVL_IMMORT && GET_LEVEL(ch)<GET_LEVEL(i)))
      {
	found = TRUE;
	stc("Your soul (burning)\r\n", ch);
      }
      else
      {
        for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
	  if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch)))
	  {
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

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i))
    {
      if (IS_AFFECTED(i, AFF_INVISIBLE))
	strcpy(buf, "*");
      else
	*buf = '\0';

      if (IS_AFFECTED(ch, AFF_DETECT_ALIGN))
	{
	  if (IS_EVIL(i))
	    strcat(buf, "(Red Aura) ");
	  else if (IS_GOOD(i))
	    strcat(buf, "(Blue Aura) ");
	}
      strcat(buf, i->player.long_descr);
      send_to_char(buf, ch);

      if (IS_AFFECTED(i, AFF_SANCTUARY))
	{
	  if (IS_EVIL(i))
	    act("...$e is surrounded by a black aura!",
		FALSE, i, 0, ch, TO_VICT);
	  else
	    act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
	}
      if (IS_AFFECTED(i, AFF_BLIND))
	act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
      if (IS_AFFECTED(i, AFF_INVULN))
	act("...$e is surrounded by a glowing sphere!",
	    FALSE, i, 0, ch, TO_VICT);
      if (IS_AFFECTED(i, AFF_FLAMING))
        act("...$e is engulfed in flames!", FALSE, i, 0, ch, TO_VICT);

      return;
    }
  if (IS_NPC(i))
    {
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
  else if (IS_MOUNTED(i))
    {
      strcat(buf, " is here, mounted on ");
      if (get_rider(i) == ch)
	strcat(buf, "YOU!");
      else
	{
	  if (get_mount(i))
	    strcat(buf, PERS(get_mount(i), ch));
	  else
	    strcat(buf, "thin air");
	  strcat(buf, ".");
	}
    }
  else
    {
    if (FIGHTING(i))
      {
	strcat(buf, " is here, fighting ");
	if (FIGHTING(i) == ch)
	  strcat(buf, "YOU!");
	else
	  {
	    if (i->in_room == FIGHTING(i)->in_room)
	      strcat(buf, PERS(FIGHTING(i), ch));
	    else
	      strcat(buf, "someone who has already left");
	    strcat(buf, "!");
	  }
      }
    else			/* NIL fighting pointer */
      strcat(buf, " is here struggling with thin air.");
  }

  if (IS_AFFECTED(ch, AFF_DETECT_ALIGN))
    {
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

  if (IS_AFFECTED(i, AFF_SANCTUARY))
  {
     if (IS_EVIL(i))
        act("...$e is surrounded by a black aura!", FALSE, i, 0, ch, TO_VICT);
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
	     ( IS_SET_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT) &&
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
	(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) ||
	 GET_LEVEL(ch)>=LVL_IMMORT))
    {
      bool io /* Imm Only */= (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
         		       GET_LEVEL(ch)>=LVL_IMMORT);
      sprintf(buf, "%s%s%s%s%s%s ", buf,
		io?CCRED(ch, C_CMP):"",
		io?"(":"",dirs[door],io?")":"",
		io?CCCYN(ch, C_NRM):"");
    }

  sprintf(buf2, "%s[ Exits: %s]%s\r\n", CCCYN(ch, C_NRM),
	  *buf ? buf : "None! ", CCNRM(ch, C_NRM));

  send_to_char(buf2, ch);
}


ACMD(do_exits)
{
  int door;

  *buf = '\0';

  if (IS_AFFECTED(ch, AFF_BLIND))
    {
      send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
      return;
    }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      {
	if (GET_LEVEL(ch) >= LVL_IMMORT)
	  sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],
		  world[EXIT(ch, door)->to_room].number,
		  world[EXIT(ch, door)->to_room].name);
	else
	  {
	    sprintf(buf2, "%-5s - ", dirs[door]);
	    if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
	      strcat(buf2, "Too dark to tell\r\n");
	    else
	      {
		strcat(buf2, world[EXIT(ch, door)->to_room].name);
		strcat(buf2, "\r\n");
	      }
	  }
	strcat(buf, CAP(buf2));
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
         send_to_char("It is too dark here to see much of anything...\r\n", ch);
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
      sprintbitarray(ROOM_FLAGS(ch->in_room), room_bits, RF_ARRAY_MAX, buf);
      sprintf(buf2, "[%5d] %s [ %s]", world[ch->in_room].number,
	      world[ch->in_room].name, buf);
      switch(world[ch->in_room].sector_type) {
         case SECT_INSIDE:  sprintf(buf2, "%s [ Inside ]", buf2);
                            break;
         case SECT_CITY:  sprintf(buf2, "%s [ City ]", buf2);
                          break;
         case SECT_FIELD:  sprintf(buf2, "%s [ Field ]", buf2);
                           break;
         case SECT_FOREST:  sprintf(buf2, "%s [ Forest ]", buf2);
                            break;
         case SECT_HILLS:  sprintf(buf2, "%s [ Hills ]", buf2);
                           break;
         case SECT_MOUNTAIN:  sprintf(buf2, "%s [ Mountain ]", buf2);
                              break;
         case SECT_WATER_SWIM:  sprintf(buf2, "%s [ Water Swim ]", buf2);
                                break;
         case SECT_WATER_NOSWIM:  sprintf(buf2, "%s [ Water Noswim ]", buf2);
                                  break;
         case SECT_UNDERWATER:  sprintf(buf2, "%s [ Underwater ]", buf2);
                                break;
         case SECT_FLYING:  sprintf(buf2, "%s [ Flying ]", buf2);
                            break;
         case SECT_DESERT:  sprintf(buf2, "%s [ Desert ]", buf2);
                            break;
         case SECT_FIRE:  sprintf(buf2, "%s [ Fire ]", buf2);
                          break;
         case SECT_EARTH:  sprintf(buf2, "%s [ Earth ]", buf2);
                          break;
         case SECT_WIND:  sprintf(buf2, "%s [ Wind ]", buf2);
                          break;
         case SECT_WATER:  sprintf(buf2, "%s [ Water ]", buf2);
                           break;
         default:  sprintf(buf2, "%s [ Unknown ]", buf2);
      }
      send_to_char(buf2, ch);
    }
  else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) {
    do_description(world[ch->in_room].description, ch);
  }

  /* autoexits */
  if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_auto_exits(ch);


  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  /* list_obj_in_heap(world[ch->in_room].contents, ch); */


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
	  sprintf(buf, "The %s is closed.\n\r",
		  fname(EXIT(ch, dir)->keyword));
	  send_to_char(buf, ch);
	}
      else
	{
	  if (IS_SET(EXIT(ch, dir)->exit_info, EX_ISDOOR) &&
              EXIT(ch, dir) && EXIT(ch, dir)->keyword &&
	      (ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK)    ||
	       !strstr(EXIT(ch, dir)->keyword, "secret")))
	    {
	      sprintf(buf, "The %s is open.\n\r",
		      fname(EXIT(ch, dir)->keyword));
	      send_to_char(buf, ch);
	    }
	}
      if(!ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK) &&
         EXIT(ch, dir) && EXIT(ch, dir)->keyword &&
	 strstr(EXIT(ch, dir)->keyword, "secret"))
	   send_to_char("You see nothing special.\n\r", ch);


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

           original_loc = ch->in_room;
	   char_from_room(ch);
	   char_to_room(ch, location);
	   look_at_room(ch, 0);
	   char_from_room(ch);
	   char_to_room(ch, original_loc);
         }
         else
         {
           stc("You see nothing special.", ch);
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
				 FIND_OBJ_EQUIP, ch, &dummy, &obj)))
    {
      sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    }
  else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	   (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	   (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    send_to_char("There's nothing inside that!\r\n", ch);
  else
    {
      if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)
	{
	  if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
	    send_to_char("It is closed.\r\n", ch);
	  else
	    {
	      send_to_char(fname(obj->name), ch);
	      switch (bits)
		{
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
	    }
	}
      else
	{		/* item must be a fountain or drink container */
	  if (GET_OBJ_VAL(obj, 1) <= 0)
	    send_to_char("It is empty.\r\n", ch);
	  else
	    {
	      if (GET_OBJ_VAL(obj,0) <= 0 ||
		  GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0))
		sprintf(buf, "Its contents seem somewhat murky.\r\n"); /* BUG */
	      else
		{
		  amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
		  sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
		  sprintf(buf, "It's %sfull of a %s liquid.\r\n",
			  fullness[amt], buf2);
		}
	      send_to_char(buf, ch);
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


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void
look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = 0, j;
  struct char_data *found_char = NULL;
  struct obj_data *obj = NULL, *found_obj = NULL;
  char *desc;

  if (!*arg)
    {
      send_to_char("Look at what?\r\n", ch);
      return;
    }
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL)
    {
      look_at_char(found_char, ch);
      if (ch != found_char)
	{
	  if (CAN_SEE(found_char, ch))
	    act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
	  act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
	}
      return;
    }
  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL)
    {
      send_to_char(desc, ch);
      return;
    }
  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL)
	{
	  send_to_char(desc, ch);
	  found = 1;
	}
  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL)
	  {
	    send_to_char(desc, ch);
	    found = 1;
	  }
    }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found;
       obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL)
	{
	  send_to_char(desc, ch);
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
  if (!ch->desc)
    return;

  stc("Your current ability scores: \r\n", ch);
  sprintf(buf, "Strength:      (%s)\r\n", abil_names[GET_STR(ch)]);
  stc(buf, ch);
  sprintf(buf, "Dexterity:     (%s)\r\n", abil_names[GET_DEX(ch)]);
  stc(buf, ch);
  sprintf(buf, "Intelligence:  (%s)\r\n", abil_names[GET_INT(ch)]);
  stc(buf, ch);
  sprintf(buf, "Wisdom:        (%s)\r\n", abil_names[GET_WIS(ch)]);
  stc(buf, ch);
  sprintf(buf, "Constitution:  (%s)\r\n", abil_names[GET_CON(ch)]);
  stc(buf, ch);
  sprintf(buf, "Charisma:      (%s)\r\n", abil_names[GET_CHA(ch)]);
  stc(buf, ch);

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
  else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
  else
      look_at_target(ch, arg);
}


ACMD(do_examine)
{
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  one_argument(argument, arg);

  if (!*arg)
    {
      send_to_char("Examine what?\r\n", ch);
      return;
    }
  look_at_target(ch, arg);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object)
    {
      if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	  (GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	  (GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER))
	{
	  send_to_char("When you look inside, you see:\r\n", ch);
	  look_in_obj(ch, arg);
	}
    }
}


ACMD(do_score)
{
  extern char *affected_bits[];
  extern char *affected_names[];
  extern char *flesh_alter_weapon( struct char_data *ch );
  int ac, align, weight = 0;
  char tmpbuf[MAX_STRING_LENGTH];
  struct time_info_data play_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  int any = FALSE, afnum = 0;
  struct master_affected_type *aff;

  strcpy(buf, "\0");
/*  strcat(buf, "****************************"
	 "************************************\r\n\r\n"); */
  sprintf(buf, "%s%s%s%s", buf,
	  CCGRN(ch, C_CMP),GET_NAME(ch), CCNRM(ch, C_CMP));
  sprintf(buf, "%s                           Age: %d years", buf, GET_AGE(ch));

  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf, " (It's your birthday today.)\r\n");
  else
    strcat(buf, "\r\n");

  sprintf(buf,
	  "%sHit points: %s%d%s(%s%d%s)  %s points: %s%d%s(%s%d%s)  "
	  "Movement points: %s%d%s(%s%d%s)\r\n",
	  buf, CCGRN(ch, C_CMP), GET_HIT(ch), CCNRM(ch, C_CMP),
	  CCRED(ch, C_CMP), GET_MAX_HIT(ch), CCNRM(ch, C_CMP),
	  (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))?"Mana":"Mind/Psi",
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
    strcpy(tmpbuf,       "You are little more evil than neutral, but not very exciting.");
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
  ac = GET_AC(ch);
  if (ac == 100)
    strcat(tmpbuf, "are naked, have you no shame?");
  else if (ac > 70)
    strcat(tmpbuf, "are lightly clothed.");
  else if (ac > 40)
    strcat(tmpbuf, "are pretty well clothed.");
  else if (ac > 10)
    strcat(tmpbuf, "are lightly armored.");
  else if (ac > -10)
    strcat(tmpbuf, "are well armored.");
  else if (ac > -40)
    strcat(tmpbuf, "are getting pretty sweaty with all that armor on.");
  else if (ac > -20)
    strcat(tmpbuf, "are very well armored.");
  else if (ac > -50)
    strcat(tmpbuf, "are extremely well armored");
  else if (ac > -75)
    strcat(tmpbuf, "are decked out in full battle armor.");
  else if (ac > -125)
    strcat(tmpbuf, "are armored like a wyvern!");
  else if (ac > -150)
    strcat(tmpbuf, "are armored like a dragon!");
  else if (ac > -175)
    strcat(tmpbuf, "could walk through the gates of Hell in all that"
            " armor!");
  else
    strcat(tmpbuf, "are armored like a god!");

  sprintf(buf, "%s%s\r\n", buf,tmpbuf);

  sprintf(buf, "%sExperience:    %s%d%s points\r\n",
	  buf, CCGRN(ch, C_CMP), GET_EXP(ch), CCNRM(ch, C_CMP));
  sprintf(buf, "%sCoins carried: %s%d%s gold coins    ",
	  buf, CCGRN(ch, C_CMP), GET_GOLD(ch), CCNRM(ch, C_CMP));
  sprintf(buf, "%sCoins in bank: %s%d%s gold coins\r\n",
	  buf, CCGRN(ch, C_CMP),GET_BANK_GOLD(ch), CCNRM(ch, C_CMP));
  sprintf(buf, "%sKills: %d  Pks: %d  Deaths: %d\r\n", buf,
          (int)GET_KILLS(ch), (int)GET_PKS(ch), (int)GET_DEATHS(ch));

  if (!IS_NPC(ch))
    {
      if (GET_LEVEL(ch) < LVL_IMMORT-1)
	sprintf(buf, "%sYou need %s%d%s exp to reach your next level.\r\n", buf,
	 	CCGRN(ch, C_CMP),(exp_needed_for_level(ch)-GET_EXP(ch)),
		CCNRM(ch, C_CMP));

      play_time = playing_time(ch);
      sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
	      buf, play_time.day, play_time.hours);

      if (is_veteran(ch))
        sprintf(buf, "%sYou are a veteran of many battles.\r\n", buf);

      sprintf(buf, "%sYou are a citizen of %s.\r\n", buf,
       hometowns[GET_HOME(ch)]);
      if (GET_CLAN(ch) && GET_CLAN_RANK(ch) != 0)
        {
          int clan_num = find_clan_by_id(GET_CLAN(ch));
          sprintf(buf, "%sYou are a %s of %s.\r\n", buf,
           clan[clan_num].rank_name[GET_CLAN_RANK(ch)-1], clan[clan_num].name);
        }
      sprintf(buf, "%sThis ranks you as %s %s (level %d).\r\n", buf,
	      GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch));
      sprintf(buf, "%sYou are %s %s ", buf,
	      AN(races[GET_RACE(ch)]), races[GET_RACE(ch)]);
      sprinttype(GET_CLASS(ch), pc_class_types, buf2);
      strcat(buf, buf2);
      sprintf(buf, "%s.\n\r", buf);

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
              aff->bitvector!=AFF_ROBBED &&
	      aff->bitvector!=AFF_BERSERK)
	  {
	   any = TRUE;   /* used to be -20.20 */
	   sprintf(buf, "%s     %s%-24.24s%s", buf,
		  CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
	   if(!(++columns % 2))
	    strcat(buf, "\r\n");
	  }
	}
      if (any)
        send_to_char(buf, ch);
     }

  any = FALSE;
  strcpy(buf, "\r\nEquipment spells affecting you:");
  {
   int nr, teller, matched = FALSE, found = FALSE;

   for(teller = 0; teller < AF_ARRAY_MAX && !found; teller++)
      for(nr = 0; nr < 32 && !found; nr++)
      {
         if(IS_SET_AR(AFF_FLAGS(ch), (teller*32)+nr))
            if(*affected_bits[(teller*32)+nr] != '\n')
               if(*affected_bits[(teller*32)+nr] != '\0')
	       {
		afnum = (teller *32)+nr;
		matched = FALSE;
		aff = NULL;
		if (ch->affected)
		  for (aff = ch->affected; aff; aff = aff->next)
		    if (aff->bitvector == afnum)
			matched = TRUE;
		if ( !matched && (afnum != AFF_GROUP) &&
		     (afnum != AFF_CUTTHROAT) &&
		     (afnum != AFF_WEREWOLF) &&
		     (afnum != AFF_MOUNT) &&
		     (afnum != AFF_NOTHING) &&
		     (afnum != AFF_VAMPIRE) )
		{
		  sprintf(buf, "%s\r\n%s%s%s", buf, CCCYN(ch, C_NRM),
			  affected_names[(teller*32)+nr], CCNRM(ch, C_NRM));
		  any = TRUE;
		}
	       }
         if(*affected_bits[(teller*32)+nr] == '\n')
            found = TRUE;
      }
  }
  strcat(buf, "\r\n");
  if (any && !IS_NPC(ch))
        send_to_char(buf, ch);

/*
  stc("\r\n***************************"
      "*************************************\r\n", ch); */
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
  for (i = 0; i < NUM_WEARS; i++)
    {
      if (GET_EQ(ch, i))
	{
	  if (CAN_SEE_OBJ(ch, GET_EQ(ch, i)))
	    {
	      send_to_char(where[i], ch);
	      show_obj_to_char(GET_EQ(ch, i), ch, 1);
	      found = TRUE;
	    }
	  else
	    {
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
  extern const char *weekdays[];
  extern const char *month_name[];

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

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
  char * filename;
  FILE * fl;
  char * date;
  time_t ct;
  int r;

  int chk, bot, top, mid, minlen;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument)
  {
     page_string(ch->desc, help, 0);
     return;
  }
  if (!help_table)
  {
     send_to_char("No help available.\r\n", ch);
     return;
  }

  bot = 0;
  top = top_of_helpt;
  minlen = strlen(argument);

  for (;;)
  {
     mid = (bot + top) / 2;

     if (bot > top)
     {
        char buf[MAX_INPUT_LENGTH + 33];
        sprintf(buf, "There is no help on: %s\r\n", argument);
        send_to_char(buf, ch);
        sprintf(buf, "HELP: %s attempted to get help on %s", GET_NAME(ch),
                argument);
        mudlog(buf, NRM, LVL_IMMORT, TRUE);
        filename = HELP_FILE;
        if (!(fl = fopen(filename, "a")))
        {
           perror("do_help");
           log("Unable to open misc/help file.");
           return;
        }
        ct = time(0);
        date = asctime(localtime(&ct));
        r = fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (date + 4),
                    world[ch->in_room].number, argument);
        if (r < 0)
          log("Unable to write to misc/help file.");
        fclose(fl);

        return;
     }
     else if (!(chk = strn_cmp(argument, help_table[mid].keyword, minlen)))
     {
        /* trace backwards to find first matching entry. Thanks Jeff Fink! */
        while ((mid > 0) &&
               (!(chk = strn_cmp(argument,
                                 help_table[mid - 1].keyword, minlen))))
           mid--;
        if ((GET_LEVEL(ch) < LVL_IMMORT) &&
            strstr(help_table[mid].entry, "wizonly"))
        {
           sprintf(buf, "There is no help on: %s\r\n", argument);
           send_to_char(buf, ch);
        }
        else
        {
           char *help = help_table[mid].entry;
           char *topic = str_dup(help_table[mid].keyword);
           int i = 0;

           for (i=0; i<strlen(topic); i++)
           topic[i] = UPPER(topic[i]);
           stc(CCGRN(ch, C_CMP), ch);
           stc("\r\n[ ", ch);
           stc(CCCYN(ch, C_CMP), ch);
           stc(topic, ch);
           stc(CCGRN(ch, C_CMP), ch);
           stc(" ]\r\n", ch);
           stc(CCNRM(ch, C_CMP), ch);
           FREE(topic);
           while (*help != '\n')
              help++;
           stc(CCRED(ch, C_CMP), ch);
           stc(" --------------------------------------"
	       "-------------------------------------", ch);
           stc(CCNRM(ch, C_CMP), ch);
           page_string(ch->desc, help, 0);
        }
        return;
     }
     else
     {
        if (chk > 0)
           bot = mid + 1;
        else
           top = mid - 1;
     }
  }
}


#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-o] [-q] \r\n"

ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char name_search[MAX_INPUT_LENGTH];
  char mode;
  size_t i;
  int low = 1, high = LVL_IMPL, localwho = 0, questwho = 0;
  int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf)
    {
      half_chop(buf, arg, buf1);
      if (isdigit(*arg))
	{
	  sscanf(arg, "%d-%d", &low, &high);
	  strcpy(buf, buf1);
	}
      else if (*arg == '-')
	{
	  mode = *(arg + 1);	/* just in case; we destroy arg in the switch */
	  switch (mode)
	    {
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
	    case 'c':
	      half_chop(buf1, arg, buf);
	      for (i = 0; i < strlen(arg); i++)
		showclass |= find_class_bitvector(arg[i]);
	      break;
	    default:
	      send_to_char(WHO_FORMAT, ch);
	      return;
	      break;
	    }				/* end of switch */

	}
      else
	{			/* endif */
	  send_to_char(WHO_FORMAT, ch);
	  return;
	}
    }				/* end while (parser) */

  send_to_char("Players\r\n-------\r\n", ch);

  for (d = descriptor_list; d; d = d->next)
    {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	  !strstr(GET_TITLE(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_OUTLAW))
        continue;
      if (ROOM_FLAGGED(tch->in_room, ROOM_NO_WHO_ROOM) &&
	  GET_LEVEL(ch)<LEVEL_IMPL)
	continue;
      if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
	continue;
      if (who_room && (tch->in_room != ch->in_room))
	continue;
      if (showclass && !(showclass & (1 << GET_CLASS(tch))))
	continue;
      if (short_list)
	{
	  if (GET_LEVEL(tch)>=LEVEL_IMPL)
	    {
	      sprintf(buf, "%s[ *IMP*  ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_GRGOD)
	    {
	      sprintf(buf, "%s[ GRGOD  ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_HIGOD)
	    {
	      sprintf(buf, "%s[ HIGOD  ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_LEGEND)
	    {
	      sprintf(buf, "%s[ LEGEND ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_GOD)
	    {
	      sprintf(buf, "%s[  GOD   ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_IMMORT+1)
	    {
	      sprintf(buf, "%s[ TITAN  ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_IMMORT)
	    {
	      sprintf(buf, "%s[ IMMORT ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	  else
	    {
	      sprintf(buf, "%s[ %2d  %s ] %-12.12s%s%s",
		      CCYEL(ch, C_SPR), GET_LEVEL(tch), CLASS_ABBR(tch),
		      GET_NAME(tch), CCNRM(ch, C_SPR),
		      ((!(++num_can_see % 4)) ? "\n\r" : ""));
	      send_to_char(buf, ch);
	    }
	}
      else
	{
	  num_can_see++;
	  if (GET_LEVEL(tch)>=LEVEL_IMMORT)
	    {
	      sprintf(buf, "%s&r[ Wizard ] %s %s", buf, GET_NAME(tch), GET_TITLE(tch));
	    }
	 /*  else if (GET_LEVEL(tch)>=LEVEL_GRGOD)
	    {
	      sprintf(buf, "%s%s[ GRGOD  ] %s %s", buf,
		      CCYEL(ch, C_SPR), GET_NAME(tch), GET_TITLE(tch));
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_HIGOD)
	    {
	      sprintf(buf, "%s%s[ HIGOD  ] %s %s", buf,
		      CCYEL(ch, C_SPR), GET_NAME(tch), GET_TITLE(tch));
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_LEGEND)
	    {
	      sprintf(buf, "%s%s[ LEGEND ] %s %s", buf,
		      CCYEL(ch, C_SPR), GET_NAME(tch), GET_TITLE(tch));
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_GOD)
	    {
	      sprintf(buf, "%s%s[  GOD   ] %s %s", buf,
		      CCYEL(ch, C_SPR), GET_NAME(tch), GET_TITLE(tch));
	    }
	  else if (GET_LEVEL(tch)>=LEVEL_IMMORT+1)
	    {
	      sprintf(buf, "%s%s[ TITAN  ] %s %s", buf,
		      CCYEL(ch, C_SPR), GET_NAME(tch), GET_TITLE(tch));
	    }
         */
	  else if (IS_CHOSEN(tch))
	    {
	      sprintf(buf, "%s&y[ Chosen ] %s %s", buf, GET_NAME(tch), GET_TITLE(tch));
	    }
	  else
	    {
	      sprintf(buf, "%s[ %2d  %s ] %s %s", buf,
		      GET_LEVEL(tch), CLASS_ABBR(tch),
		      GET_NAME(tch), GET_TITLE(tch));
	    }


	  if (GET_INVIS_LEV(tch) && GET_LEVEL(ch)>=LEVEL_IMMORT)
	    sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
	  else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	    strcat(buf, " (invis)");

	  if (PLR_FLAGGED(tch, PLR_MAILING))
	    strcat(buf, " (mailing)");
	  else if (PLR_FLAGGED(tch, PLR_WRITING))
	    strcat(buf, " (writing)");

          if (d->original)
            strcat(buf, " (out of body)");

          if (STATE(d) == CON_OEDIT)
            strcat(buf, " (Object Edit)");
          if (STATE(d) == CON_MEDIT)
            strcat(buf, " (Mobile Edit)");
          if (STATE(d) == CON_ZEDIT)
            strcat(buf, " (Zone Edit)");
          if (STATE(d) == CON_SEDIT)
            strcat(buf, " (Shop Edit)");
          if (STATE(d) == CON_REDIT)
            strcat(buf, " (Room Edit)");
          if (STATE(d) == CON_TEDIT)
            strcat(buf, " (Text Edit)");

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
	/*  if (GET_LEVEL(ch) >= LVL_IMMORT && PLR_FLAGGED(tch, PLR_CHOSEN))
	    strcat(buf, " (CHOSEN)"); */
	  if (GET_LEVEL(tch) >= LVL_IMMORT || IS_CHOSEN(tch))
	    strcat(buf, "&n");

	  strcat(buf, "\r\n");
	 /* send_to_char(buf, ch); */
	}
    }
  if (num_can_see == 0)
    sprintf(buf, "%s\r\nNo-one at all!\r\n", buf);
  else if (num_can_see == 1)
    sprintf(buf, "%s\r\nOne character displayed.\r\n", buf);
  else
    sprintf(buf, "%s\r\n%d characters displayed.\r\n", buf, num_can_see);

  if (!short_list)
    page_string(ch->desc, buf, 1);
  if (short_list && (num_can_see % 4))
  {
    send_to_char(buf, ch);
    send_to_char("\r\n", ch);
  }
}

#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
  extern char *connected_types[];
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, *format, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  size_t i;
  int low = 0, high = LVL_IMPL, num_can_see = 0;
  int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;
  char buffer[MAX_STRING_LENGTH];

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-')
      {
	mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
	switch (mode)
	  {
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
	  case 'c':
	    playing = 1;
	    half_chop(buf1, arg, buf);
	    for (i = 0; i < strlen(arg); i++)
	      showclass |= find_class_bitvector(arg[i]);
	    break;
	  default:
	    send_to_char(USERS_FORMAT, ch);
	    return;
	    break;
	  }				/* end of switch */

      }
    else
      {			/* endif */
	send_to_char(USERS_FORMAT, ch);
	return;
      }
  }				/* end while (parser) */
  strcpy(line,
	 "Num Class   Name         State          Idl Login@   Site\r\n");
  strcat(line,
	 "--- ------- ------------ -------------- --- -------- --------"
	 "----------------\r\n");
  strcpy(buffer, line);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next)
    {
      if (d->connected && playing)
	continue;
      if (!d->connected && deadweight)
	continue;
      if (!d->connected)
	{
	  if (d->original)
	    tch = d->original;
	  else if (!(tch = d->character))
	    continue;

	  if (*host_search && !strstr(d->host, host_search))
	    continue;
	  if (*name_search && str_cmp(GET_NAME(tch), name_search))
	    continue;
	  if (!CAN_SEE(ch, tch) ||
	      GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	    continue;
	  if (showclass && !(showclass & (1 << GET_CLASS(tch))))
	    continue;
	  if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	    continue;

	  if (d->original)
	    sprintf(classname, "[%2d %s]", GET_LEVEL(d->original),
		    CLASS_ABBR(d->original));
	  else
	    sprintf(classname, "[%2d %s]", GET_LEVEL(d->character),
		    CLASS_ABBR(d->character));
	}
      else
	strcpy(classname, "   -   ");

      timeptr = asctime(localtime(&d->login_time));
      timeptr += 11;
      *(timeptr + 8) = '\0';

      if (!d->connected && d->original)
	strcpy(state, "Switched");
      else
	strcpy(state, connected_types[d->connected]);

      if (d->character && !d->connected &&
          ( (GET_LEVEL(d->character) < GET_LEVEL(ch)) ||
            GET_LEVEL(ch) == LVL_IMPL ) )
	sprintf(idletime, "%3d", d->character->char_specials.timer *
		SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
      else
	strcpy(idletime, "");

      format = "%3d %-7s %-12s %-14s %-3s %-8s ";

      if (d->character && d->character->player.name)
	{
	  if (d->original)
	    sprintf(line, format, d->desc_num, classname,
		    d->original->player.name, state, idletime, timeptr);
	  else
	    sprintf(line, format, d->desc_num, classname,
		    d->character->player.name, state, idletime, timeptr);
	}
      else
	sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED",
		state, idletime, timeptr);

      if (d->host && *d->host)
	sprintf(line + strlen(line), "[%s]\r\n", d->host);
      else
	strcat(line, "[Hostname unknown]\r\n");

      if (d->connected)
	{
	  sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
	  strcpy(line, line2);
	}
      if (d->connected || (!d->connected && CAN_SEE(ch, d->character)))
	{
	  strcat (buffer, line);
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
  extern int top_of_p_table ;
  extern struct player_index_element *player_table;
  extern char * compile_time_str;
  extern time_t compile_time;
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
      page_string(ch->desc, motd, 0);
      break;
    case SCMD_IMOTD:
      page_string(ch->desc, imotd, 0);
      break;
    case SCMD_CLEAR:
      send_to_char("\033[H\033[J", ch);
      break;
    case SCMD_VERSION:
      send_to_char("Dark Pawns 2.3-" SVN_REVISION "\r\n", ch);
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
      for (i = 0; i <= top_of_p_table ; i++)
	{
	  sprintf(buf, "%-20.20s", (player_table +i)->name);
	  stc(buf, ch);
	  count++;
	  if ( count == 3)
	    {
	      count = 0;
	      stc("\r\n", ch);
	    }
	}
      stc("\r\n", ch);
      break ;
    default:
      return;
      break;
    }
}


char *print_object_location(int num, struct obj_data * obj, struct char_data * ch,
		      int recur)
{
  static char buf[MAX_STRING_LENGTH];

  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%s%33s", buf, " - ");

  if (obj->in_room > NOWHERE)
    {
      sprintf(buf + strlen(buf), "[%5d] %s\r\n",
	      world[obj->in_room].number, world[obj->in_room].name);
    }
  else if (obj->carried_by)
    {
      sprintf(buf + strlen(buf), "carried by %s\r\n",
	      PERS(obj->carried_by, ch));
    }
  else if (obj->worn_by)
    {
      sprintf(buf + strlen(buf), "worn by %s\r\n",
	      PERS(obj->worn_by, ch));
    }
  else if (obj->in_obj)
    {
      sprintf(buf + strlen(buf), "inside %s%s\r\n",
	      obj->in_obj->short_description, (recur ? ", which is" : " "));
      if (recur)
	print_object_location(0, obj->in_obj, ch, recur);
    }
  else
    {
      sprintf(buf + strlen(buf), "in an unknown location\r\n");
    }

  return buf;
}


ACMD(do_where)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  one_argument(argument, arg);
  *buf = '\0';

  if (!*arg)
    {
      send_to_char("Players\r\n-------\r\n", ch);
      for (d = descriptor_list; d; d = d->next)
	if (!d->connected)
	  {
            i = (d->original ? d->original : d->character);
	    if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE))
	      {
		if (d->original)
		  sprintf(buf, "%s%-20s - [%s%5d%s] %s (in %s)\r\n",
			  buf, GET_NAME(i),
			  CCGRN(ch, C_CMP),
			  world[d->character->in_room].number,
			  CCNRM(ch, C_CMP),
			  world[d->character->in_room].name,
			  GET_NAME(d->character));
		else
		  sprintf(buf, "%s%-20s - [%s%5d%s] %s\r\n", buf, GET_NAME(i),
			  CCGRN(ch, C_CMP),
			  world[i->in_room].number,
			  CCNRM(ch, C_CMP),
			  world[i->in_room].name);
	      }
	  }
      page_string(ch->desc, buf, 1);
    }
  else
    {
      for (i = character_list; i; i = i->next)
	if (CAN_SEE(ch, i) &&
	    i->in_room != NOWHERE && isname_with_abbrevs(arg,i->player.name))
	  {
	    found = 1;
	    snprintf(buf+ strlen(buf),MAX_STRING_LENGTH, "M%3d. %-25s - [%5d]%s\r\n",
                    ++num,GET_NAME(i),
		    world[i->in_room].number, world[i->in_room].name);
            if (num >= 30)
              break;
	  }
      for (num = 0, k = object_list; k; k = k->next)
	if (CAN_SEE_OBJ(ch, k) && isname_with_abbrevs(arg, k->name))
	  {
	    found = 1;
	    snprintf(buf + strlen(buf), MAX_STRING_LENGTH,
                     print_object_location(++num, k, ch, TRUE));
            if (num >= 30)
              break;
	  }
      page_string(ch->desc, buf, 1);
      if (!found)
	send_to_char("Couldn't find any such thing.\r\n", ch);
    }
}



ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch))
    {
      send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
      return;
    }
  *buf = '\0';

  for (i = 1; i < LVL_IMMORT; i++)
    sprintf(buf + strlen(buf), "[%2d] %8d-%-8d    (%6d)\r\n", i,
	    find_exp(GET_CLASS(ch), i-1), find_exp(GET_CLASS(ch), i),
	    find_exp(GET_CLASS(ch), i)-find_exp(GET_CLASS(ch), i-1));
  page_string(ch->desc, buf, 1);
}


ACMD(do_consider)
{
  struct char_data *victim;
  int hitdiff = 0, damdiff = 0, leveldiff = 0,
    victdam = 0, chardam = 0;
  struct obj_data *charwielded = GET_EQ(ch, WEAR_WIELD);
  struct obj_data *victwielded = NULL;
  char buf[MAX_STRING_LENGTH];

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf)))
    {
      send_to_char("Consider killing who?\r\n", ch);
      return;
    }
  if (victim == ch)
    {
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
  else
    {
      if (IS_NPC(ch))
	chardam += dice(ch->mob_specials.damnodice,
			ch->mob_specials.damsizedice);
      else/* Max. lvl/3 dam with bare hands */
	chardam += number(0, GET_LEVEL(ch)/3);
    }

  if (!IS_NPC(victim) && victwielded)
    victdam += dice(GET_OBJ_VAL(victwielded, 1), GET_OBJ_VAL(victwielded, 2));
  else
    {
      if (IS_NPC(victim))
	victdam += dice(victim->mob_specials.damnodice,
		    victim->mob_specials.damsizedice);
      else/* Max. lvl/3 dam with bare hands */
	victdam += number(0, GET_LEVEL(victim)/3);
    }

  hitdiff = GET_HIT(victim)-GET_HIT(ch);
  damdiff = victdam-chardam;
  leveldiff = GET_LEVEL(victim)-GET_LEVEL(ch);

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

  if (hitdiff > 4*GET_HIT(ch))
    strcat(buf, "looks to be\r\nin much better physical shape than you, ");
  else if (hitdiff > 2*GET_HIT(ch))
    strcat(buf, "looks to be\r\nin a lot better physical shape than you, ");
  else if (hitdiff > GET_HIT(ch))
    strcat(buf, "looks to be\r\nin better physical shape than you, ");
  else if (hitdiff >= 0)
    strcat(buf, "looks to be\r\nin about the same physical shape as you, ");
  else if (hitdiff > -25)
    strcat(buf, "looks to be\r\nin a little worse physical shape as you, ");
  else if (hitdiff > -50)
    strcat(buf, "looks to be\r\nin a worse physical shape than you, ");
  else
    strcat(buf, "looks to be\r\nin a lot worse physical shape than you, ");

  if (leveldiff > GET_LEVEL(ch))
    strcat(buf, "and moves with an ease telling\r\nof many won battles.");
  else if (leveldiff > GET_LEVEL(ch)/2)
    strcat(buf, "and seems to know $S opponent.");
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
  act(buf, TRUE, ch, 0, victim, TO_CHAR);
}


ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf)
    {
      if (!(vict = get_char_room_vis(ch, buf)))
	{
	  send_to_char(NOPERSON, ch);
	  return;
	}
      else
	diag_char_to_char(vict, ch);
    }
  else
    {
      if (FIGHTING(ch))
	diag_char_to_char(FIGHTING(ch), ch);
      else
	send_to_char("Diagnose who?\r\n", ch);
    }
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
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);
  if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
  if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);

  sprintf(buf, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR),
	  CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  char buf3[20];

  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "OFF");
  else
    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

  strcpy(buf3, ctypes[COLOR_LEV(ch)]);

  sprintf(buf,
	  "Hit Pnt Display: %-3s    "
	  "     Brief Mode: %-3s    "
	  " Summon Protect: %-3s\r\n"

	  "   Move Display: %-3s    "
	  "   Compact Mode: %-3s    "
	  "       On Quest: %-3s\r\n"

	  "   Mana Display: %-3s    "
	  "         NoTell: %-3s    "
	  "   Repeat Comm.: %-3s\r\n"

	  " Auto Show Exit: %-3s    "
          "      Auto Loot: %-3s    "
          "      Auto Gold: %-3s\r\n"

          "     Auto Split: %-3s    "
	  "           Deaf: %-3s    "
	  "     Wimp Level: %-3s\r\n"

	  " Gossip Channel: %-3s    "
	  "Auction Channel: %-3s    "
	  "  Grats Channel: %-3s\r\n"

	  "  Dsp Tank Stat: %-3s    "
	  "Dsp Fightg Stat: %-3s    "
	  "    Color Level: %s\r\n"

          " Newbie Channel: %-3s"
          "    Clan tells: %-3s "
          "    Broadcasts: %-3s",
	  ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOGOLD)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOSPLIT)),
	  YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
	  buf2,

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPTANK)),
	  ONOFF(PRF_FLAGGED(ch, PRF_DISPTARGET)),
	  CAP(buf3),

	  ONOFF(!PRF_FLAGGED(ch, PRF_NONEWBIE)),
          ONOFF(!PRF_FLAGGED(ch, PRF_NOCTELL)),
          ONOFF(!PRF_FLAGGED(ch, PRF_NOBROAD)) );


  sprintf(buf, "%s\r\n", buf);
  send_to_char(buf, ch);
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


ACMD(do_skills)
{
  int parse_class(char arg);
  int i, level, skill, found, col=0;
  char mybuf[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if (!*arg)
    {
      send_to_char("Usage: skills <class abbrev>\r\n"
		    "([M]age, [C]leric, [T]hief, [W]arrior, "
		    "Ps[i]onic, [N]inja, \r\n A[s]sassin, "
		    "M[a]gus, A[v]atar, [P]aladin)\r\n",
		    ch);
      return;
    }
  if ((i = parse_class(argument[1])) == CLASS_UNDEFINED)
    {
      send_to_char("That's not a valid class.\r\n"
		    "Usage: skills <class-abbrev>\r\n"
		    "([M]age, [C]leric, [T]hief, [W]arrior, "
		    "Ps[i]onic, [N]inja, \r\n A[s]sassin, "
		    "M[a]gus, A[v]atar, [P]aladin)\r\n",
		    ch);
      return;
    }
  sprintf(mybuf,"Skills/Spells for the class '%s%s%s':\r\n"
	  "%s --------------------------------------"
	  "-------------------------------------%s\r\n",
	  CCYEL(ch, C_CMP), pc_class_types[i], CCNRM(ch, C_CMP),
	  CCRED(ch, C_CMP), CCNRM(ch, C_CMP));
  for (level = 1; level < 31; level++)
    {
      found = FALSE;
      for (skill = 1; skill < MAX_SKILLS; skill++)
	{
	  if (spell_info[skill].min_level[i] == level)
	    {
	      if (found == FALSE)
		sprintf(mybuf, "%s\r\n         < %sLevel %2d%s >\r\n",
			mybuf, CCRED(ch, C_CMP), level, CCNRM(ch, C_CMP));
	      found = TRUE;
	      sprintf(mybuf, "      %s   %s  ", mybuf, spells[skill]);
	      if (++col % 3 == 0)
               sprintf(mybuf, "%s\r\n", mybuf);
            }
	}
  }
  strcat(mybuf, "\r\n\r\n");
  page_string(ch->desc, mybuf, 0);
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


void do_description(char *description, struct char_data *ch) {
  struct extra_descr_data *i;
  int num_extras = 0, j;
  char extras[60][255];
  char scratch[MAX_STRING_LENGTH];
  char *token;
  char *begin;
  char *end;
  char rem1, rem2;


  for (i = world[ch->in_room].ex_description; i; i = i->next) {
    strcpy(buf, i->keyword);
    token = strtok(buf, " ");
    num_extras++;
    strcpy(extras[num_extras], token);
    while ((token = strtok(NULL, " "))) {
      num_extras++;
      strcpy(extras[num_extras], token);
    }
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
          while (!isspace(*end)) end++;
          begin--;
          rem1 = *begin;
          rem2 = *end;
          *begin = *end = 0;
          begin++; end++;
          sprintf(scratch, "%s%s%c%s%c%s%s",
                 buf2, VT_BOLDTEX, rem1, begin, rem2, VT_NORMALT, end);
          strcpy(buf2, scratch);
        }
     }
  }

  send_to_char(buf2, ch);
}

