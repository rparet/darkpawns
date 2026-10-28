/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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

/* $Id: limits.c 1259 2005-12-13 11:35:34Z mcooper $ */

#include <math.h>
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "screen.h"
#include "dream.h"
#include "house.h"
#include "vt100.h"

#define READ_TITLE(ch) (titles)

/* External Variables */
extern char *titles;
extern int max_exp_gain;
extern int max_exp_loss;
extern void clearMemory(struct char_data * ch); 
extern int min_exp(int level);

/* External Functions */
SPECIAL(shop_keeper);
void flesh_alter_from(struct char_data *ch);
void flesh_alter_to(struct char_data *ch);
int find_exp(int level);
void InfoBarUpdate(struct char_data *ch, int update);

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);		/* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 5));	/* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 5));	/* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 5));	/* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 10));	/* 60..79 */
  else
    return (p6);		/* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* Updates for hitpoints, mana and movement */
int point_gain(struct char_data * ch, int update)
{
  int gain, i = 0, j = 0;

  if (IS_NPC(ch))
    gain = GET_LEVEL(ch);
  else
    gain = graf(age(ch).year, 40, 55, 75, 100, 130, 180, 30);

  for (i = 0; i < NUM_WEARS; i++) 
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
        switch (update) {
          case SCMD_HIT:
            if (GET_EQ(ch, i)->affected[j].location == APPLY_HIT_REGEN)
              gain += GET_EQ(ch, i)->affected[j].modifier;
            break;
          case SCMD_MANA:
            if (GET_EQ(ch, i)->affected[j].location == APPLY_MANA_REGEN)
              gain += GET_EQ(ch, i)->affected[j].modifier;
            break;
          case SCMD_MOVE:
            if (GET_EQ(ch, i)->affected[j].location == APPLY_MOVE_REGEN)
              gain += GET_EQ(ch, i)->affected[j].modifier;
            break;
      }

  switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain >> 1);  /* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain >> 2);  /* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);  /* Divide by 8 */
      break;
    case POS_FIGHTING:
      gain = (gain >> 2);   /* 1/4 gain total */
      break;
    case POS_STANDING:      
      gain = (gain >> 1);   /* 1/2 gain total */
      break;
  }

  if (IS_AFFECTED(ch, AFF_POISON) || IS_AFFECTED(ch, AFF_FLAMING) ||  IS_AFFECTED(ch, AFF_CUTTHROAT))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0) || (GET_COND(ch, TIRED) == 0))
    gain >>= 2;

  if (ch->in_room > 0)
    if (ROOM_FLAGGED(ch->in_room, ROOM_REGENROOM))
      gain += (gain >> 1);  /* Add another half of the gain */

  gain *= log10(GET_LEVEL(ch) + 1);
  return (gain);
}

void set_title(struct char_data * ch, char *title)
{
  if (title == NULL)
    title = READ_TITLE(ch);

  if (strlen(title) > MAX_TITLE_LENGTH)
    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL)
    FREE(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}


void check_autowiz(struct char_data * ch)
{
  char buf[100];
  extern ubyte use_autowiz;
  extern int mini_mud;
  extern ubyte min_wizlist_lev;
  pid_t getpid(void);

  if (!mini_mud && use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) 
  {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}


void gain_exp(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;

  long exp_needed_for_level(struct char_data *ch);

  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  if (IS_NPC(ch)) {
    GET_EXP_Q(ch) += gain;
    GET_EXP(ch) += gain;
    return;
  }
  
  if (gain > 0) {
    int max_exp = find_exp(GET_LEVEL(ch));
    gain = MIN(max_exp_gain, gain); /* put a cap on the max gain per kill */
    gain = MIN(max_exp-1, gain); /* can only level one time! */
    GET_EXP(ch) += gain;
    GET_EXP_Q(ch) += gain;

    if (GET_INFOBAR(ch))
      InfoBarUpdate(ch, INFO_EXP | INFO_EXP_Q);

    /* Do we have a vitae penalty to work off? */
    if (GET_VITAE_EXP(ch)) {
      GET_VITAE_EXP(ch) -= gain;
      if (GET_VITAE_EXP(ch) <= 0) {     /* We've worked off a vitae penalty point */
        GET_VITAE_EXP(ch) = 0;
        if (GET_VITAE(ch)) {
          vitae_af(ch, FALSE);
          GET_VITAE(ch)--;
          vitae_af(ch, TRUE);
          send_to_char(CCCYN(ch, C_CMP), ch);         
          send_to_char("You have regained a vitae point!\r\n", ch);
          send_to_char(CCNRM(ch, C_CMP), ch);         
          if (GET_VITAE(ch))            /* We've still got one left, reset the exp */
            GET_VITAE_EXP(ch) = (long)((exp_needed_for_level(ch) - GET_EXP(ch)) * 0.1);
        }
      }
    }
       /* vitae_af(ch); */


    if (GET_LEVEL(ch) < LVL_IMMORT - 1  && (GET_EXP(ch) >= min_exp(GET_LEVEL(ch) + 1)))
      if (!GET_VITAE(ch)) {   /* don't advance with vitae points */
        GET_LEVEL(ch) += 1;
        num_levels++;
        advance_level(ch);
        is_altered = TRUE;    
      }

    if (is_altered) {
      sprintf(buf, "%s advanced %d level%s to level %d.", GET_NAME(ch), num_levels,
        num_levels == 1 ? "" : "s", GET_LEVEL(ch));
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

      /* if (IS_AFFECTED(ch, AFF_FLESH_ALTER))     
        flesh_alter_from(ch);  -- FIX FIX FIX */

      if (num_levels == 1) {
        send_to_char(CCCYN(ch, C_CMP), ch);         
        sprintf(buf, "You advance to level %d!\r\n", GET_LEVEL(ch));
        send_to_char(buf, ch);
	      send_to_char(CCNRM(ch, C_CMP), ch);         
      } else {
        send_to_char(CCCYN(ch, C_CMP), ch);
        sprintf(buf, "You advance %d levels to level %d!\r\n", num_levels, GET_LEVEL(ch));
        send_to_char(buf, ch);
        send_to_char(CCNRM(ch, C_CMP), ch);
      }
    }
  } else if (gain < 0)
    return;      
}

void
gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE; /* nothing to do with flesh alter */
  int num_levels = 0;

  GET_EXP(ch) += gain;
  GET_EXP_Q(ch) += gain;

  if (GET_EXP(ch) < 0)
    GET_EXP(ch) = 0;

  if (!IS_NPC(ch)) {
    while ((GET_LEVEL(ch) < LVL_IMPL) && (GET_EXP(ch) >= min_exp(GET_LEVEL(ch) + 1)))
    /* if (IS_AFFECTED(ch, AFF_FLESH_ALTER))     
        flesh_alter_from(ch);	*/
      if (!GET_VITAE(ch)) {   /* don't advance with vitae points */
        GET_LEVEL(ch) += 1;
        num_levels++;
        advance_level(ch);

     /* if (IS_AFFECTED(ch, AFF_FLESH_ALTER))     
	        flesh_alter_to(ch); */
        is_altered = TRUE;
      }

    if (is_altered) {
      sprintf(buf, "%s advanced %d level%s to level %d.", GET_NAME(ch), num_levels,
        num_levels == 1 ? "" : "s", GET_LEVEL(ch));
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      if (num_levels == 1)        
        send_to_char("You rise a level!\r\n", ch);
      else {
        sprintf(buf, "You rise %d levels!\r\n", num_levels);	
        send_to_char(buf, ch);
      }
      set_title(ch, NULL);
      check_autowiz(ch);
    }
  }
}


void
gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (GET_COND(ch, condition) == -1)	/* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(48, GET_COND(ch, condition));

  if (GET_COND(ch, condition) > 1 || PLR_FLAGGED(ch, PLR_WRITING) || 
      (ch->char_specials.timer > IDLE_TO_VOID))
    return;

  if (GET_COND(ch, condition))
  {
   switch (condition) 
   {
   case FULL:
    send_to_char("Your stomach growls with hunger.\r\n", ch);
    return;
   case THIRST:
    send_to_char("You feel a bit parched.\r\n", ch);
    return;
   case DRUNK:
    if (intoxicated)
      send_to_char("Your head starts to clear.\r\n", ch);
    return;
   case TIRED:
    if (AWAKE(ch))
      send_to_char("You are feeling a bit sleepy, perhaps it's time for a nap?\r\n", ch);
    return;
   default:
    break;
   }
  }
  else
  {
  switch (condition) 
  {
  case FULL:
    send_to_char("You are hungry.\r\n", ch);
    return;
  case THIRST:
    send_to_char("You are thirsty.\r\n", ch);
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  case TIRED:
    if (AWAKE(ch))
      send_to_char("You are exhausted.\r\n", ch);
    return;
  default:
    break;
  }
 }

}


void check_idling(struct char_data * ch)
{
  void Crash_rentsave(struct char_data *ch);

  if (++(ch->char_specials.timer) > IDLE_TO_VOID)
    if (GET_LEVEL(ch) < LVL_IMMORT) {
      if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
        GET_WAS_IN(ch) = ch->in_room;
        if (FIGHTING(ch)) {
	  stop_fighting(FIGHTING(ch));
	  stop_fighting(ch);
        }
        act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
        save_char(ch, IN_ROOM(ch));
        Crash_crashsave(ch);
        char_from_room(ch);
        char_to_room(ch, 1);
        if (!ZONE_FLAGGED(world[ch->in_room].zone, ZONE_UNLOAD))
          reload_room_descs(world[ch->in_room].zone);
      } else if (ch->char_specials.timer > IDLE_DISCONNECT) {
        if (ch->in_room != NOWHERE)
	  char_from_room(ch);
        char_to_room(ch, 1);
        if (!ZONE_FLAGGED(world[ch->in_room].zone, ZONE_UNLOAD))
          reload_room_descs(world[ch->in_room].zone);
        if (ch->desc)
	  close_socket(ch->desc);
        ch->desc = NULL;
	Crash_rentsave(ch);
        sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
        mudlog(buf, CMP, LVL_GOD, TRUE);
        extract_char(ch);
      }
   }
}

#define dust    18
#define puddle  20
#define puke    21

/* Update those objects on the mud hour that require it */
void item_update(void)
{
  struct char_data *i;
  struct obj_data *j, *jj, *next_thing;

  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;                   /* Next in object list */

    if (IS_FOOD(j)) {
      /* We only want to decay food if it's being carried by a player, or sitting on
      the open ground */

      if ((jj = j->in_obj)) {             /* It's in a container */
        while (jj) {
          jj->carried_by ? (i = jj->carried_by) : (i = jj->worn_by);
          jj = jj->in_obj;                /* In a container within a container */
        }
        if (i && IS_NPC(i))               /* Carried by a mob */
          continue;
      } else if ((i = j->carried_by)) {
        if (IS_NPC(i))                    /* Carried by a mob */
          continue;
      } else if (j->in_room != NOWHERE)
        i = world[j->in_room].people;     /* In the room and not in a container */

      if (!i)                           /* Couldn't find a location */
        continue;

      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {
        if (j->in_room != NOWHERE)
          act("$p decays before your eyes.", TRUE, 0, j, 0, TO_ROOM);
        else
          act("$p that you were carrying decays before your eyes.", TRUE, i, j, 0, TO_CHAR);
        House_crashsave(i->in_room);
        extract_obj(j);
      }
    }
  }
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
  void update_char_objects(struct char_data * ch);  /* handler.c */
  void extract_obj(struct obj_data * obj);  /* handler.c */
  void make_wight(struct obj_data *corpse); /* utils.c */

  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;
  int k;

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
 
    if (IS_MOB(i))
      if (GET_MOB_TIMER(i) > 0)
        GET_MOB_TIMER(i)--;

    if (!IS_NPC(i))  /* non NPC stuff */
    {
      if (JAIL_TIME(i) > 0)
        JAIL_TIME(i)--;

      if (OUTLAW_TIME(i) > 0)
        OUTLAW_TIME(i)--;

      for (k = 0; k <= MAX_SKILL_GROUPS; k++)
        if (IMPROVE_TIME(i, k) > 0)
          IMPROVE_TIME(i, k)--;

      if (GET_POS(i) == POS_SLEEPING)
        dream(i);
      
       if (GET_INFOBAR(i))
        InfoBarUpdate(i, INFO_EXP | INFO_EXP_Q | INFO_GOLD | INFO_BANK);
    }

    if (IS_AFFECTED(i, AFF_POISON))
      damage(i, i, 2 * point_gain(i, SCMD_HIT), SPELL_POISON);

    if (IS_AFFECTED(i, AFF_CUTTHROAT))
      damage(i, i, 3 * point_gain(i, SCMD_HIT), SKILL_CUTTHROAT);

    if (IS_NPC(i)) {
      /* Allow NPCs to regen normally */
      GET_HIT(i) = MAX(0, MIN(GET_HIT(i) + point_gain(i, SCMD_HIT), GET_MAX_HIT(i)));
      GET_MANA(i) = MAX(0, MIN(GET_MANA(i) + point_gain(i, SCMD_MOVE), GET_MAX_MANA(i)));
      GET_MOVE(i) = MAX(0, MIN(GET_MOVE(i) + point_gain(i, SCMD_MANA), GET_MAX_MOVE(i)));
    } else if (!PRF_FLAGGED(i, PRF_INACTIVE) || (GET_LEVEL(i) >= LVL_IMMORT)) {
      if (GET_POS(i) >= POS_STUNNED) {
        GET_HIT(i) = MAX(0, MIN(GET_HIT(i) + point_gain(i, SCMD_HIT), GET_MAX_HIT(i)));
        GET_MANA(i) = MAX(0, MIN(GET_MANA(i) + point_gain(i, SCMD_MANA), GET_MAX_MANA(i)));
        GET_MOVE(i) = MAX(0, MIN(GET_MOVE(i) + point_gain(i, SCMD_MOVE), GET_MAX_MOVE(i)));
      }
    }

    if (GET_POS(i) <= POS_STUNNED)
      update_pos(i);
    if (GET_POS(i) == POS_INCAP)
      damage(i, i, 1, TYPE_SUFFERING);
    if (GET_POS(i) == POS_MORTALLYW)
      damage(i, i, 2, TYPE_SUFFERING);

    if (IS_NPC(i) && MEMORY(i))
      if(number(0, 98) == 0)
        clearMemory(i);
     
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (!FIGHTING(i))
        check_idling(i);
    }
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;  /* Next in object list */
      
    /* if this is a red moongate */
    if (GET_OBJ_VNUM(j) == red_portal) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;
      
      if (!GET_OBJ_TIMER(j)) {
        if ((j->in_room != NOWHERE) && (world[j->in_room].people))
          send_to_room("The shimmering red portal of light fades out of existence.\r\n", j->in_room);
        extract_obj(j);
      }
    } else if (GET_OBJ_VNUM(j) == puddle || GET_OBJ_VNUM(j) == puke) {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;
      
      if (!GET_OBJ_TIMER(j))
        extract_obj(j);
    } else if (GET_OBJ_VNUM(j) == dust) {
      /* If this is a pile of dust */
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;
      
      if (!GET_OBJ_TIMER(j)) {
        if ((j->in_room != NOWHERE) && (world[j->in_room].people))
          send_to_room("The pile of dust is blown away by a draft of wind.\r\n", j->in_room);
        extract_obj(j);
      }
    } else if (IS_CORPSE(j)) {
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;
    
      if (!GET_OBJ_TIMER(j)) {
        if (j->carried_by)
          act("$p decays in your hands, and the contents fall to the ground.",
              FALSE, j->carried_by, j, 0, TO_CHAR);
        else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
          switch(number (0,6)) {
            case 0:
              act("A quivering horde of maggots consumes $p.",
                TRUE, world[j->in_room].people, j, 0, TO_ROOM);
              act("A quivering horde of maggots consumes $p.",
                TRUE, world[j->in_room].people, j, 0, TO_CHAR);
              break;
            case 1:
            case 2:
              act("Dissolving into the ground, $p disappears.",
                TRUE, world[j->in_room].people, j, 0, TO_ROOM);
              act("Dissolving into the ground, $p disappears.",
                TRUE, world[j->in_room].people, j, 0, TO_CHAR);
              break;
            case 3:
            case 4:
              act("A horde of flesh-eating ants consumes $p.",
                TRUE, world[j->in_room].people, j, 0, TO_ROOM);
              act("A horde of flesh-eating ants consumes $p.",
                TRUE, world[j->in_room].people, j, 0, TO_CHAR);
              break;
            case 5:
            case 6:
            default:
              act("The earth opens up and swallows $p.",
                TRUE, world[j->in_room].people, j, 0, TO_ROOM);
              act("The earth opens up and swallows $p.",
                TRUE, world[j->in_room].people, j, 0, TO_CHAR);
              break;
          }
        }

        for (jj = j->contains; jj; jj = next_thing2) {
          next_thing2 = jj->next_content;  /* Next in inventory */
          obj_from_obj(jj);

          if (j->in_obj)
            obj_to_obj(jj, j->in_obj);
          else if (j->carried_by)
            obj_to_room(jj, j->carried_by->in_room);
          else if (j->in_room != NOWHERE)
            obj_to_room(jj, j->in_room);
          else
            assert(FALSE);

          House_crashsave(j->in_room);
        }
        extract_obj(j);
      }
    } /*corpse*/
  }
}

void hunger_and_thirst()
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (!PRF_FLAGGED(i, PRF_INACTIVE)) {
      gain_condition(i, FULL, -1);
      gain_condition(i, DRUNK, -1);
      gain_condition(i, THIRST, -1);
      if (!(GET_POS(i) == POS_SLEEPING))
        gain_condition(i, TIRED, -1);
      else
        gain_condition(i, TIRED, 2);
    }
}
