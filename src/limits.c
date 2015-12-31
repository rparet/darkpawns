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

/* $Id: limits.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "screen.h"
#include "dream.h"

#define READ_TITLE(ch) (titles[(int)GET_CLASS(ch)])

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char *titles[NUM_CLASSES];
extern struct room_data *world;
extern struct index_data *obj_index;
extern int max_exp_gain;
extern int max_exp_loss;
extern field_object_data_t field_objs[NUM_FOS];
extern void clearMemory(struct char_data * ch);

void flesh_alter_from(struct char_data *ch);
void flesh_alter_to(struct char_data *ch);
int find_exp(int class, int level);
int exp_needed_for_level(struct char_data *ch);

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain, i = 0, j = 0;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
  } else {
    gain = 14;

    if (is_veteran(ch))
      gain += 4;

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
    {
      gain <<= 1;
      break;
    }
    case POS_RESTING:
      gain += (gain >> 1);  /* Divide by 2 */
      break;
    case POS_SITTING:
      gain += (gain >> 2);  /* Divide by 4 */
      break;
    }

    /* Item calculations */
    for (i = 0; i < NUM_WEARS; i++)
     if (GET_EQ(ch, i))
       for (j = 0; j < MAX_OBJ_AFFECT; j++)
         if (GET_EQ(ch, i)->affected[j].location==APPLY_MANA_REGEN)
       if( (GET_EQ(ch, i)->affected[j].modifier>0 &&
        GET_POS(ch)==POS_SLEEPING) ||
           GET_EQ(ch, i)->affected[j].modifier<0 )
            gain += GET_EQ(ch, i)->affected[j].modifier;

    /* Class calculations */
    if ((GET_CLASS(ch) == CLASS_MAGIC_USER) || (GET_CLASS(ch) == CLASS_CLERIC))
      gain <<= 1;
    if ((GET_CLASS(ch) == CLASS_MAGUS) || (GET_CLASS(ch) == CLASS_AVATAR))
      gain <<= 1;
    else if ((GET_CLASS(ch) == CLASS_PSIONIC) || (GET_CLASS(ch) == CLASS_NINJA))
      gain += (gain >> 2);
    else if (IS_MYSTIC(ch))
      gain <<= 1;
  }

  /* Skill/Spell calculations */
  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_FLAMING))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_CUTTHROAT))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (ch->in_room>0)
   if (ROOM_FLAGGED(ch->in_room, ROOM_REGENROOM))
     gain += (gain >> 1);   /* Divide by 2 */

  return (gain);
}


int hit_gain(struct char_data * ch)
/* Hitpoint gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    if (GET_LEVEL(ch) < 23)
      gain = 2.5*GET_LEVEL(ch);
    else
      gain = 4.0*(GET_LEVEL(ch));
  } else {
    gain = 20;

    if (is_veteran(ch))
      gain += 12;

    if (!FIGHTING(ch) && affected_by_spell(ch, SKILL_KK_JIN)) {
          gain += (gain >> 2);   /* regen same as resting */
    }

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
    {
      int i = 0, j = 0;
      gain += (gain >> 1);  /* Divide by 2 */
      for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i))
          for (j = 0; j < MAX_OBJ_AFFECT; j++)
        if (GET_EQ(ch, i)->affected[j].location==APPLY_HIT_REGEN)
               gain += GET_EQ(ch, i)->affected[j].modifier;
      break;
    }
    case POS_RESTING:
      gain += (gain >> 2);  /* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);  /* Divide by 8 */
      break;
    }


    /* Class/Level calculations */
    if ((GET_CLASS(ch) == CLASS_MAGIC_USER) || (GET_CLASS(ch) == CLASS_CLERIC))
      gain >>= 1;
  }

  /* Skill/Spell calculations */
  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_FLAMING))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_CUTTHROAT))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (ch->in_room>0)
    if (ROOM_FLAGGED(ch->in_room, ROOM_REGENROOM))
      gain += (gain >> 1);  /* Divide by 2 */

  return (gain);
}



int move_gain(struct char_data * ch)
/* move gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    return (GET_LEVEL(ch));
  } else {
    gain = 20;

    if (is_veteran(ch))
      gain += 4;

    if (!FIGHTING(ch))
      {
        if (affected_by_spell(ch, SKILL_KK_ZHEN))
          gain += (gain >> 2);  /* same as resting */
      }

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
    {
      int i = 0, j = 0;
      for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i))
          for (j = 0; j < MAX_OBJ_AFFECT; j++)
        if (GET_EQ(ch, i)->affected[j].location==APPLY_MOVE_REGEN)
               gain += GET_EQ(ch, i)->affected[j].modifier;
      gain += (gain >> 1);  /* Divide by 2 */
      break;
    }
    case POS_RESTING:
      gain += (gain >> 2);  /* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);  /* Divide by 8 */
      break;
    }
  }

  /* Skill/Spell calculations */
  if (IS_AFFECTED(ch, AFF_POISON) || IS_AFFECTED(ch, AFF_FLAMING))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_CUTTHROAT))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (ch->in_room>0)
   if (ROOM_FLAGGED(ch->in_room, ROOM_REGENROOM))
    gain += (gain >> 1);    /* Divide by 2 */

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
  extern int use_autowiz;
  extern int mini_mud;
  extern int min_wizlist_lev;
  pid_t getpid(void);

  if (!mini_mud && use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT)
  {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
        WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
}

void
gain_exp(struct char_data * ch, int gain)
{
  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  if (IS_NPC(ch))
    {
      GET_EXP(ch) += gain;
      return;
    }
  if (gain > 0)
    {
      int max_exp = find_exp(GET_CLASS(ch), GET_LEVEL(ch)+1)-GET_EXP(ch);
      gain = MIN(max_exp_gain, gain); /* put a cap on the max gain per kill */
      gain = MIN(max_exp-1, gain); /* can only level one time! */
      GET_EXP(ch) += gain;
      if (GET_LEVEL(ch) < LVL_IMMORT-1 && GET_EXP(ch)>exp_needed_for_level(ch))
    {
      if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
        flesh_alter_from(ch);
      GET_LEVEL(ch)++;
      advance_level(ch);
      if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
        flesh_alter_to(ch);
      send_to_char(CCCYN(ch, C_CMP), ch);
      sprintf(buf, "You advance to level %d!\r\n", GET_LEVEL(ch));
      send_to_char(buf, ch);
      send_to_char(CCNRM(ch, C_CMP), ch);
    }
    }
  else if (gain < 0)
    {
      gain = MAX(-max_exp_loss, gain);  /* Cap max exp lost per death */
      GET_EXP(ch) += gain;
      if (GET_EXP(ch) < 0)
    GET_EXP(ch) = 0;
    }
}

void
gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE; /* nothing to do with flesh alter */
  int num_levels = 0;

  GET_EXP(ch) += gain;
  if (GET_EXP(ch) < 0)
    GET_EXP(ch) = 0;

  if (!IS_NPC(ch))
    {
      while (GET_LEVEL(ch) < LVL_IMPL &&
         GET_EXP(ch) >= exp_needed_for_level(ch))
    {
      if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
        flesh_alter_from(ch);
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
        flesh_alter_to(ch);
      is_altered = TRUE;
    }

      if (is_altered)
    {
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else
        {
          sprintf(buf, "You rise %d levels!\r\n", num_levels);
          send_to_char(buf, ch);
        }
      check_autowiz(ch);
    }
    }
}

void
gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (GET_COND(ch, condition) == -1)    /* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(48, GET_COND(ch, condition));

  if (GET_COND(ch, condition)>1 || PLR_FLAGGED(ch, PLR_WRITING))
    return;
  if (GET_COND(ch, condition))
  {
   switch (condition) {
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
   default:
    break;
   }
  }
  else
  {
  switch (condition) {
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
  default:
    break;
  }
 }

}

void check_idling(struct char_data * ch)
{
  extern int free_rent;
  void Crash_rentsave(struct char_data *ch, int cost);

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
        save_char(ch, NOWHERE);
        Crash_crashsave(ch);
        char_from_room(ch);
        char_to_room(ch, 1);
      } else if (ch->char_specials.timer > IDLE_DISCONNECT) {
        if (ch->in_room != NOWHERE)
      char_from_room(ch);
        char_to_room(ch, 3);
        if (ch->desc)
      close_socket(ch->desc);
        ch->desc = NULL;
        if (free_rent)
      Crash_rentsave(ch, 0);
        else
      Crash_idlesave(ch);
        sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
        mudlog(buf, CMP, LVL_GOD, TRUE);
        extract_char(ch);
      }
   }
}

#define dust 18
#define puddle 20
#define puke 21
/* Update PCs, NPCs, and objects */
void
point_update(void)
{
  void update_char_objects(struct char_data * ch);  /* handler.c */
  void extract_obj(struct obj_data * obj);  /* handler.c */
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  SPECIAL(moon_gate);

  /* characters */
  for (i = character_list; i; i = next_char)
    {
      next_char = i->next;


      if (!PRF_FLAGGED(i, PRF_INACTIVE))
    {
          gain_condition(i, FULL, -1);
          gain_condition(i, DRUNK, -1);
          gain_condition(i, THIRST, -1);
    }

      if (TAT_TIMER(i)>0)
         TAT_TIMER(i)--;

      if(GET_JAIL_TIMER(i)>0)
         GET_JAIL_TIMER(i)--;

      if(GET_POS(i) == POS_SLEEPING) {
         dream(i);
      }

      if (GET_POS(i) >= POS_STUNNED)
    {
      if (!PRF_FLAGGED(i, PRF_INACTIVE))
      {
        if (GET_HIT(i) < GET_MAX_HIT(i))
          GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
        if (GET_MANA(i) < GET_MAX_MANA(i))
          GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      }
      if (IS_AFFECTED(i, AFF_POISON))
        damage(i, i, 10, SPELL_POISON);
      if (IS_AFFECTED(i, AFF_CUTTHROAT))
        damage(i, i, 13, SKILL_CUTTHROAT);
          if (GET_POS(i) <= POS_STUNNED)
        update_pos(i);
    }
      else if (GET_POS(i) == POS_INCAP)
    damage(i, i, 1, TYPE_SUFFERING);
      else if (GET_POS(i) == POS_MORTALLYW)
    damage(i, i, 2, TYPE_SUFFERING);

      if (IS_NPC(i) && MEMORY(i))
        {
          if(number(0, 98) == 0)
           clearMemory(i);
        }

      if (!IS_NPC(i))
    {
      update_char_objects(i);
      /* if (GET_LEVEL(i) < LVL_IMMORT) */
      check_idling(i);
    }
    }

  /* objects */
  for (j = object_list; j; j = next_thing)
    {
      next_thing = j->next; /* Next in object list */

      /* if this is a moongate */
      if (GET_OBJ_SPEC(j) == moon_gate)
    {
      /* timer count down */
      if (GET_OBJ_VAL(j, 2) > 0)
        GET_OBJ_VAL(j, 2)--;

        if (!GET_OBJ_VAL(j, 2))
          {
                sprintf(buf, "%s fades out of existence.\r\n", j->short_description);
                CAP(buf);
        if ((j->in_room != NOWHERE) && (world[j->in_room].people))
          send_to_room(buf, j->in_room);
        extract_obj(j);
          }
    }
      else if (GET_OBJ_VNUM(j) == puddle || GET_OBJ_VNUM(j) == puke)
    {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j))
        extract_obj(j);
    }
      /* If this is a pile of dust */
      else if (GET_OBJ_VNUM(j) == dust)
    {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;

        if (!GET_OBJ_TIMER(j))
          {
        if ((j->in_room != NOWHERE) && (world[j->in_room].people))
          send_to_room("The pile of dust is blown away "
                   "by a draft of wind.\r\n", j->in_room);
        extract_obj(j);
          }
    }
     /* if this is a circle of summoning */
     else if (GET_OBJ_VNUM(j) == COC_VNUM)
       {
         /* timer count down */
         if (GET_OBJ_TIMER(j) > 0)
           GET_OBJ_TIMER(j)--;

         if (GET_OBJ_TIMER(j) <= 0)
           {
             if ((j->in_room != NOWHERE) && (world[j->in_room].people))
               send_to_room("The circle on the ground slowly fades away."
                             "\r\n", j->in_room);
             extract_obj(j);
           }
        }
      /* If this is a corpse */
      else if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3))
    {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;

        if (!GET_OBJ_TIMER(j))
          {
        if (j->carried_by)
          act("$p decays in your hands, and"
              " the contents fall to the ground.",
              FALSE, j->carried_by, j, 0, TO_CHAR);
        else if ((j->in_room != NOWHERE) && (world[j->in_room].people))
          {
            switch(number (0,6))
              {
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
        for (jj = j->contains; jj; jj = next_thing2)
          {
            next_thing2 = jj->next_content; /* Next in inventory */
            obj_from_obj(jj);

            if (j->in_obj)
              obj_to_obj(jj, j->in_obj);
            else if (j->carried_by)
              obj_to_room(jj, j->carried_by->in_room);
            else if (j->in_room != NOWHERE)
              obj_to_room(jj, j->in_room);
            else
              assert(FALSE);
          }
        extract_obj(j);
          }
    }/*corpse*/
        else /* check for field object */
      {
        int count = 0;
        for (count = 0; count < NUM_FOS; count++)
        {
        if(GET_OBJ_VNUM(j)==field_objs[count].obj_vnum &&
           j->in_room > 0)
        {
            /* timer count down */
              if (GET_OBJ_TIMER(j) > 0)
                    GET_OBJ_TIMER(j)--;

                  if (!GET_OBJ_TIMER(j))
          {
            struct obj_data *thing = NULL;
            if(field_objs[count].worn_off_obj_num>0)
            {
             thing =
               read_object(field_objs[count].worn_off_obj_num, VIRTUAL);
             if (thing)
             {
                obj_to_room( thing, j->in_room);
            GET_OBJ_TIMER(thing) = 2;
             }
            }
            send_to_room(field_objs[count].wear_off_msg, j->in_room);
            send_to_room("\r\n", j->in_room);
            extract_obj(j);
          }
        }
        }
      }
    }
}
