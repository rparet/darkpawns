/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
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

/* $Id: mobact.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "scripts.h"

/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];

void hunt_victim(struct char_data * ch);
void mp_sound(struct char_data *mob);
SPECIAL(shop_keeper);
bool can_speak(struct char_data *ch);

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

void mobile_activity(void)
{
  register struct char_data *ch, *next_ch, *vict, *tmp_ch;
  struct obj_data *obj, *best_obj;
  int door, found, max;
  memory_rec *names;

  extern int no_specials;

  ACMD(do_get);
  ACMD(do_wake);
  ACMD(do_stand);
  ACMD(do_assist);

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (!IS_MOB(ch) || FIGHTING(ch) || !AWAKE(ch))
      continue;

    /* hunt two steps at a time to do it faster */
    if ( (GET_POS(ch) == POS_STANDING) && (MOB_FLAGGED(ch, MOB_HUNTER))
            && (!FIGHTING(ch)) )
                hunt_victim(ch);
    if ( (GET_POS(ch) == POS_STANDING) && (MOB_FLAGGED(ch, MOB_HUNTER))
            && (!FIGHTING(ch)) )
                hunt_victim(ch);

    /* Examine call for special procedure */
    if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
      if (mob_index[GET_MOB_RNUM(ch)].func == NULL) {
    sprintf(buf, "%s (#%d): Attempting to call non-existing mob func",
        GET_NAME(ch), GET_MOB_VNUM(ch));
    log("%s", buf);
    REMOVE_BIT_AR(MOB_FLAGS(ch), MOB_SPEC);
      } else {
    if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
      continue;     /* go to next char */
      }
    }

    /* waking up sleeper'd mobs */

    if (!AWAKE(ch) && !affected_by_spell(ch, SKILL_SLEEPER)) {
       do_wake(ch, "", 0, 0);
       do_stand(ch, "", 0, 0);
    }

    /* Scavenger (picking up objects) */
    if (MOB_FLAGGED(ch, MOB_SCAVENGER))
      if (world[ch->in_room].contents && !number(0, 10)) {
    max = 1;
    best_obj = NULL;
    for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
      if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
        best_obj = obj;
        max = GET_OBJ_COST(obj);
      }
    if (best_obj != NULL) {
      obj_from_room(best_obj);
      obj_to_char(best_obj, ch);
      act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
    }
      }

    /* Mob Movement */
    door = number(0, 18);
    if (!IS_SET_AR(MOB_FLAGS(ch), MOB_SENTINEL)   &&          /* not sentinel */
         (GET_POS(ch) == POS_STANDING)      &&          /* is standing */
     (door < NUM_OF_DIRS)               &&          /* rand door is ok */
         (CAN_GO(ch, door))                 &&          /* able to move */
     ( !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_DEATH) &&
       !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB) ) &&
     ( !IS_SET_AR(MOB_FLAGS(ch), MOB_STAY_ZONE) ||
       (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))
       )  {
       if ( (SECT(EXIT(ch, door)->to_room) == SECT_WATER_SWIM) &&
           !CAN_SWIM(ch) )
         continue;
       if ( (SECT(EXIT(ch, door)->to_room) == SECT_WATER_NOSWIM) &&
           !CAN_SWIM(ch) )
         continue;
       if ( (SECT(EXIT(ch, door)->to_room) == SECT_FLYING) &&
           !IS_FLYING(ch) )
         continue;
       if (!ch || ch->in_room<0)
         continue; /* ch died in the move */
       if (!ch->desc) /* ser 120896 */
          perform_move(ch, door, 1);
    }

    /* Sounds */
    if (!number(0, 15)) {
      mp_sound(ch); /* Legacy sound function */
      if (GET_MOB_SCRIPT(ch) && MOB_SCRIPT_FLAGGED(ch, MS_SOUND)) {
        found = FALSE;
        for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
          if ((vict == ch) || IS_MOB(vict))
            continue;
          else
            found = TRUE;
        }
        if (found)
          run_script(ch, ch, NULL, &world[ch->in_room], NULL, "sound", LT_MOB);
      }
    }

  /* Mob ONPULSE for any room occupants */
    if (GET_MOB_SCRIPT(ch) && MOB_SCRIPT_FLAGGED(ch, MS_ONPULSE_ALL)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if (vict == ch)
          continue;
        else {
          found = TRUE;
          break;
        }
      }
      if (found)
        if (run_script(ch, ch, NULL, &world[ch->in_room], NULL, "onpulse_all", LT_MOB))
          continue;
    }

    if (check_dead(ch))
    continue; /* ch died during the script */

  /* Mob ONPULSE for only PC room occupants */
    if (GET_MOB_SCRIPT(ch) && MOB_SCRIPT_FLAGGED(ch, MS_ONPULSE_PC)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if ((vict == ch) || IS_MOB(vict))
          continue;
        else {
          found = TRUE;
          break;
        }
      }
      if (found)
        run_script(ch, ch, NULL, &world[ch->in_room], NULL, "onpulse_pc", LT_MOB);
  }

    if (check_dead(ch))
    continue; /* ch died during the script */

    /* Aggressive Mobs */
    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE) ||
        MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN)) {
      found = FALSE;
      for (vict = world[ch->in_room].people;
       vict && !found;
       vict = vict->next_in_room) {
    if (IS_NPC(vict) || !CAN_SEE(ch, vict) ||
        PRF_FLAGGED(vict, PRF_NOHASSLE))
      continue;
    if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
      continue;
        if (IS_AFFECTED(vict, AFF_PROTECT_EVIL) && IS_EVIL(ch) && !number(0,5))
      continue;
        if (IS_AFFECTED(vict, AFF_PROTECT_GOOD) && IS_GOOD(ch) && !number(0,5))
          continue;
        if (IS_AFFECTED(vict, AFF_SNEAK) && !number(0,3))
      continue;
    /* old -- if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) || */
    if ( ( ! MOB_FLAGGED(ch, MOB_AGGR_EVIL) &&
                MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) &&
                MOB_FLAGGED(ch, MOB_AGGR_GOOD) ) ||
        (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
        (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
        (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {
      hit(ch, vict, TYPE_UNDEFINED);
      found = TRUE;
    }
        if ( MOB_FLAGGED(ch, MOB_AGGRESSIVE) ) { /* straight up -rparet */
          hit(ch, vict, TYPE_UNDEFINED);
          found = TRUE;
        }
      }
    }

    if (!ch || ch->in_room<0)
    continue; /* ch died in the move */

    /* race hate haters */
    if ( GET_MOB_SPEC(ch) != shop_keeper)
     for (found = FALSE,vict = world[ch->in_room].people;
         vict && !found;
     vict = vict->next_in_room)
     {
      int i = 0;
      for (i = 0; i < 5 && !found; i++)
        if (GET_RACE_HATE(vict, i) == GET_RACE(ch))
    {
     if (!IS_NPC(vict) && CAN_SEE(ch, vict) &&
        !PRF_FLAGGED(vict, PRF_NOHASSLE) &&
           (!IS_AFFECTED(vict, AFF_PROTECT_EVIL) ||
        (IS_EVIL(ch) && !number(0,5))))
      {
        if (!number(0,5) && can_speak(ch))
              act("'Come to destroy my kin? Die!', exclaims $n.",
              FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, vict, TYPE_UNDEFINED);
        found = TRUE;
      }
    }
     }


    /* Mob Memory */
    if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
      found = FALSE;
      for (vict = world[ch->in_room].people;
           vict && !found;
       vict = vict->next_in_room) {
    if (IS_NPC(vict) || !CAN_SEE(ch, vict) ||
        PRF_FLAGGED(vict, PRF_NOHASSLE))
      continue;
    for (names = MEMORY(ch); names && !found; names = names->next)
      if (names->id == GET_IDNUM(vict)) {
        found = TRUE;
        if (can_speak(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
          act("'Hey!  You're the fiend that attacked me!!!', exclaims $n.",
        FALSE, ch, 0, 0, TO_ROOM);
        else if (!ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
          act("You hear a low growl in the back of $n's throat.",
        FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, vict, TYPE_UNDEFINED);
      }
      }
    }

    /* Helper Mobs */
    if (MOB_FLAGGED(ch, MOB_HELPER)) {
      found = FALSE;
      if (IS_AFFECTED(ch, AFF_CHARM) && ch->master)
        if (FIGHTING(ch->master)) {
          do_assist(ch, GET_NAME(ch->master), 0, 0);
          return;
        }
      for (vict = world[ch->in_room].people;
       vict && !found;
       vict = vict->next_in_room)
    if (ch != vict && IS_NPC(vict) && FIGHTING(vict)  &&
        CAN_SEE(ch, FIGHTING(vict)) && !IS_NPC(FIGHTING(vict)) &&
        ch != FIGHTING(vict)) {
      act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);
      hit(ch, FIGHTING(vict), TYPE_UNDEFINED);
      found = TRUE;
    }
    }

    if (MOB_FLAGGED(ch, MOB_AGGR24)) {
      for (vict = 0, tmp_ch = world[ch->in_room].people;
           tmp_ch && !vict; tmp_ch = tmp_ch->next_in_room)
          if(!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
             !PRF_FLAGGED(tmp_ch,PRF_NOHASSLE) &&
             !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
            if (GET_LEVEL(tmp_ch) >= 24)
               vict=tmp_ch;
      if (vict) {
        act("$n grins evilly.", FALSE, ch, 0, 0, TO_ROOM);
    if (can_speak(ch))
    {
         act("'Another dark pawn trying to be a knight, eh?', asks $n.",
        FALSE, ch, 0, 0, TO_ROOM);
         act("$n says, 'I can fix that.'\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }
           hit(ch, vict, 0);
      }
    } /*aggr24 stuff*/

    /* AGGR24 and AGGR and fully healed = AGGR to mobs 3 under your level */
    if (MOB_FLAGGED(ch, MOB_AGGR24) && MOB_FLAGGED(ch, MOB_AGGRESSIVE) &&
        GET_HIT(ch) == GET_MAX_HIT(ch)) {
       for (vict = 0, tmp_ch = world[ch->in_room].people;
           tmp_ch && !vict; tmp_ch = tmp_ch->next_in_room)
          if(IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
             GET_LEVEL(tmp_ch)+3<GET_LEVEL(ch))
               vict=tmp_ch;
      if (vict) {
        act("$n grins evilly.", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, vict, 0);
      }
    } /* aggr24+AGGR stuff */
    /* Add new mobile actions here */

  }             /* end for() */
}



/* Mob Memory Routines */

/* make ch remember victim */
void remember(struct char_data * ch, struct char_data * victim)
{
  memory_rec *tmp;
  bool present = FALSE;

  if (!IS_NPC(ch) || IS_NPC(victim))
    return;

  for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
    if (tmp->id == GET_IDNUM(victim))
      present = TRUE;

  if (!present) {
    CREATE(tmp, memory_rec, 1);
    tmp->next = MEMORY(ch);
    tmp->id = GET_IDNUM(victim);
    MEMORY(ch) = tmp;
  }
}


/* make ch forget victim */
int forget(struct char_data * ch, struct char_data * victim)
{
  memory_rec *curr, *prev = NULL;

  if (!(curr = MEMORY(ch)))
    return FALSE;

  while (curr && curr->id != GET_IDNUM(victim)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    return FALSE;           /* person wasn't there at all. */

  if (curr == MEMORY(ch))
    MEMORY(ch) = curr->next;
  else
    prev->next = curr->next;

  FREE(curr);
  return TRUE;
}


/* erase ch's memory */
void clearMemory(struct char_data * ch)
{
  memory_rec *curr, *next;

  curr = MEMORY(ch);

  while (curr) {
    next = curr->next;
    FREE(curr);
    curr = next;
  }

  MEMORY(ch) = NULL;
}
