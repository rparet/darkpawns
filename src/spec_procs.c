/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
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

/* $Id: spec_procs.c 1487 2008-05-22 01:36:10Z jravn $ */

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


/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern int mini_mud;

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
int num_followers(struct char_data *ch);
ACMD(do_gen_comm);
ACMD(do_stand);
ACMD(do_wake);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_parry);
ACMD(do_headbutt);
ACMD(do_berserk);
ACMD(do_charge);
ACMD(do_disarm);
SPECIAL(breed_killer);

int mag_savingthrow(struct char_data * ch, int type);
void raw_kill(struct char_data * ch, int attacktype);
struct char_data *HUNTING(struct char_data *ch);
void send_to_zone(char *messg, struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);

struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS+1];

extern char *spells[];

void
sort_spells(void)
{
  int a, b, tmp;

  /* initialize array */
  for (a = 1; a < MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < MAX_SKILLS - 1; a++)
    for (b = a + 1; b < MAX_SKILLS; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0)
	{
	  tmp = spell_sort_info[a];
	  spell_sort_info[a] = spell_sort_info[b];
	  spell_sort_info[b] = tmp;
	}
}


char *
how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else if (percent <= 98)
    strcpy(buf, " (superb)");
  else
    strcpy(buf, " (MASTER)");

  return (buf);
}

char *prac_types[] = {
  "spell",
  "skill",
  "art"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[4][NUM_CLASSES];

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)])
#define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
#define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])


void
list_skills(struct char_data * ch)
{
  int mag_manacost(struct char_data * ch, int spellnum);
  extern char *spells[];
  extern struct spell_info_type spell_info[];
  int i, sortpos, mana;
  char manastring[30];

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf, "%sYou know of the following %ss:\r\n", buf, SPLSKL(ch));

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos < MAX_SKILLS; sortpos++)
    {
      i = spell_sort_info[sortpos];
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32)
	{
	  strcat(buf2, "**OVERFLOW**\r\n");
	  break;
	}
      if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)])
	{
	  mana = mag_manacost(ch, find_skill_num(spells[i]));   
	  if(mana)
	    sprintf(manastring ,"( %s%d %s%s )", 
		    CCRED(ch, C_CMP), mana, 
		    (IS_PSIONIC(ch) || IS_MYSTIC(ch))?"psi pts":"mana",
		    CCNRM(ch, C_CMP)); 
	  sprintf(buf, "%-20s %s %s\r\n", spells[i],
		  how_good(GET_SKILL(ch, i)), mana?manastring:"");
	  strcat(buf2, buf);
	}
    }

  page_string(ch->desc, buf2, 1);
}


SPECIAL(guild)
{
  int skill_num, percent;

  extern struct spell_info_type spell_info[];
  extern struct int_app_type int_app[];

  if (IS_NPC(ch) || !CMD_IS("practice"))
    return 0;

  skip_spaces(&argument);

  if (!*argument)
    {
      list_skills(ch);
      return 1;
    }
  if (GET_PRACTICES(ch) <= 0)
    {
      send_to_char("You do not seem to be able to practice now.\r\n", ch);
      return 1;
    }

  skill_num = find_skill_num(argument);

  if (skill_num < 1 ||
      GET_LEVEL(ch) < spell_info[skill_num].min_level[(int) GET_CLASS(ch)])
    {
      sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
      send_to_char(buf, ch);
      return 1;
    }
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    {
      send_to_char("You are already learned in that area.\r\n", ch);
      return 1;
    }
  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char("You are now learned in that area.\r\n", ch);

  return 1;
}



SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  ACMD(do_drop);
  char *fname(char *namelist);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents)
    {
      act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
      extract_obj(k);
    }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents)
    {
      act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
      value += MAX(1, MIN(10, GET_OBJ_COST(k) / 10));
      extract_obj(k);
    }

  if (value)
    {
      act("You are awarded for outstanding performance.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n has been awarded by the gods!",
	  TRUE, ch, 0, 0, TO_ROOM);

      if (GET_LEVEL(ch) < 3)
	gain_exp(ch, value);
      else
	GET_GOLD(ch) += value;
    }
  return 1;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */

void
npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0))
    {
      act("You discover that $n has $s hands in your wallet.",
	  FALSE, ch, 0, victim, TO_VICT);
      act("$n tries to steal gold from $N.",
	  TRUE, ch, 0, victim, TO_NOTVICT);
    }
  else
    {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
      if (gold > 0)
	{
	  GET_GOLD(ch) += gold;
	  GET_GOLD(victim) -= gold;
	}
    }
}


SPECIAL(snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING || GET_HIT(ch) < 0)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 32 - GET_LEVEL(ch)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(summoner)
{
  struct char_data *vict = NULL;
  struct descriptor_data *d;
  memory_rec *names;
  bool found = FALSE;

  if (cmd || GET_POS(ch) != POS_STANDING)
    return FALSE;

  if (HUNTING(ch))
  {
    vict = HUNTING(ch);
    found = TRUE;
  }

  if (!vict && MEMORY(ch))
    for (d = descriptor_list; !found && d; d = d->next) 
    {
      if (!d->connected)
      {
       vict = (d->original ? d->original : d->character);
       if (vict && CAN_SEE(ch, vict) && (vict->in_room != NOWHERE))
        for (names = MEMORY(ch); names && !found; names = names->next)
          if (names->id == GET_IDNUM(vict)) 
            found = TRUE;
      }
    }

  if (vict && found && !number(0,3))
  {
    call_magic(ch, vict, 0, SPELL_SUMMON, GET_LEVEL(ch), CAST_SPELL);
    if(ch->in_room == vict->in_room)
      hit(ch, vict, TYPE_UNDEFINED);
    return(TRUE);
  }
  return(FALSE);
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4)))
      {
	npc_steal(ch, cons);
	return TRUE;
      }
  return FALSE;
}


SPECIAL(magic_user)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_HIT(ch) < 0)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);


  switch (number(0,GET_LEVEL(ch)/2)+GET_LEVEL(ch)/2)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
      break;
    case 6:
    case 7:
      cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
      break;
    case 8:
    case 9:
      cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
      break;
    case 10:
    case 11:
      cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
      break;
    case 12:
      if (IS_EVIL(vict) && !IS_EVIL(ch))
        cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
      else if (IS_GOOD(vict) && !IS_GOOD(ch))
        cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD);
      break;
    case 13:
      cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
      break;
    case 14:
      if (!number(0,10))
	cast_spell(ch,vict,NULL,SPELL_TELEPORT);
      break;
    case 15:
    case 16:
    case 17:
      cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
      break;
    case 20:
      cast_spell(ch, vict, NULL, SPELL_HELLFIRE);
      break;
    case 25:
      cast_spell(ch, vict, NULL, SPELL_FLAMESTRIKE);
      break;
    case 30:
      cast_spell(ch, vict, NULL, SPELL_DISINTEGRATE);
      break;
    case 31:
    case 32:
    case 33:
      cast_spell(ch, vict, NULL, SPELL_DISRUPT);
      break;
    case 34:
      cast_spell(ch, ch, NULL, SPELL_INVULNERABILITY);
      break;
    case 35:
    case 36:
      if (OUTSIDE(ch))
        cast_spell(ch, vict, NULL, SPELL_FLAMESTRIKE);
      break;
    case 37:
      if (OUTSIDE(ch))
        cast_spell(ch, vict, NULL, SPELL_METEOR_SWARM);
      break;
    case 38:
      cast_spell(ch, vict, NULL, SPELL_DISRUPT);
      break;
    case 39:
    case 40:
    default:
      cast_spell(ch, vict, NULL, SPELL_FIREBALL);
      break;
    }
  return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */
SPECIAL(fighter)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_HIT(ch) < 0 || !FIGHTING(ch))
    return FALSE;

  if (GET_MOB_WAIT(ch))
    return FALSE;

  /* removed the pick-a-random-fighting character bs  -rparet 19980717 */
  vict = FIGHTING(ch);

  switch(number(0,10))
   {
     case 1: do_headbutt(ch, GET_NAME(vict), 0, 1);
	     	break;
     case 2: do_parry(ch, GET_NAME(vict), 0, 1);
		break;
     case 3: do_bash(ch, GET_NAME(vict), 0, 1);
	     	break;
     case 4: do_berserk(ch, NULL, 0, 1);
                break;
     default: return(FALSE);
   }
  return(TRUE);
}

SPECIAL(paladin)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_HIT(ch) < 0 || !FIGHTING(ch))
    return FALSE;

  if (GET_MOB_WAIT(ch))
    return FALSE;

  vict = FIGHTING(ch);

  switch(number(0, 8))
  {
    case 0: do_parry(ch, GET_NAME(vict), 0, 1);
            break;
    case 1: do_bash(ch, GET_NAME(vict), 0, 1);
            break;
    case 2: do_charge(ch, GET_NAME(vict), 0, 1);
            break;
    case 3: if (IS_EVIL(ch))
	      cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD);
	    else
	      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
	    break;
    case 5: do_disarm(ch, GET_NAME(vict), 0, 1);
            break;
    default: break;
  }
  return TRUE;
}

SPECIAL(guild_guard)
{
  int i;
  extern int guild_info[][3];
  struct char_data *guard = (struct char_data *) me;
  char *buf  = "The guard humiliates you, and blocks your way.\r\n";
  char *buf2 = "The guard humiliates $n, and blocks $s way.";

  if (CMD_IS("flee") || CMD_IS("escape") || CMD_IS("retreat"))
    {
      stc("You try to flee inside the guild but the guard stops you!\r\n",ch);
      act("$n tries to flee inside the guild but the guard block $s way!\r\n",
          FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }

  if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND))
  {
	if (FIGHTING(guard))
	  return(fighter(guard, guard, 0, NULL));	
        return FALSE;
  }

  if (GET_LEVEL(ch) >= LVL_IMMORT || IS_REMORT_ONLY_CLASS(ch) || HUNTING(ch))
    return FALSE;

  for (i = 0; guild_info[i][0] != -1; i++)
    {
      if ((IS_NPC(ch) || GET_CLASS(ch) != guild_info[i][0]) &&
	  world[ch->in_room].number == guild_info[i][1] &&
	  cmd == guild_info[i][2])
	{
	  send_to_char(buf, ch);
	  act(buf2, FALSE, ch, 0, 0, TO_ROOM);
	  return TRUE;
	}
    }

  return FALSE;
}


SPECIAL(puff)
{
   ACMD(do_say);
   ACMD(do_emote);

   if (cmd)
      return(0);

   if (GET_HIT(ch) < 0)
     {
       do_say(ch, "Shit, I'm dead.", 0, 0);
       return(1);
     }

   switch (number(0, 90))
     {
     case 0:
       do_say(ch, "My god!  It's full of stars!", 0, 0);
       return(1);
     case 1:
       do_say(ch, "How'd all those fish get up here?", 0, 0);
       return(1);
     case 2:
       do_say(ch, "I'm a very female dragon.", 0, 0);
       return(1);
     case 3:
       do_say(ch, "I've got this peaceful, easy feeling.", 0, 0);
       return(1);
     case 4:
       return(1);
     case 5:
       return(1);
     case 6:
       return(1);
     case 7:
       do_say(ch, "Goddamn, what a trip! Listen to those colors!", 0, 0);
       return(1);
     case 8:
       do_say(ch, "Bring out your dead!", 0, 0);
       return(1);
     case 9:
       do_say(ch, "Rule number 6...there is NO rule number 6.", 0, 0);
       return(1);
     case 10:
       do_say(ch, "To be rich is no longer a sin...its a MIRACLE!", 0, 0);
       return(1);
     case 11:
       return(1);
     case 12:
       return(1);
     case 13:
       act("$n looks at you and then breaks out in a fit of laughter!", 1, ch, 0,0, TO_ROOM);
       return(1);
     case 14:
       return(1);
     case 15:
       do_say(ch, "What is the sound of down?", 0, 0);
       return(1);
     case 16:
       return(1);
     case 17:
       act("$n wonders where she left that darn wand.",1,ch,0,0,TO_ROOM);
       return(1);
     case 18:
       return(1);
     case 19:
       return(1);
     case 20:
     case 21:
       do_say(ch,"Do you want to stroke my tail?", 0, 0);
       return(1);
     case 22:
       return(1);
     case 23:
     case 24:
       act("$n does female stuff.",1,ch,0,0,TO_ROOM);
       return(1);
     case 25:
       return(1);
     case 26:
       act("$n contemplates the meaning of life.", 1, ch, 0,0, TO_ROOM);
       return(1);
     case 27:
       do_say(ch,"NIH!",0,0);
       return(1);
     case 28:
     case 29:
     case 30:
       return(1);
     case 31:
     case 32:
       act("$n rocks out to some funky beats.", 1, ch, 0,0, TO_ROOM);
     case 33:
       return(1);
     case 34:
     case 35:
     case 36:
       return(1);
     case 37:
     case 38:
     case 39:
       do_say(ch,"I'm gonna kick your ASS!",0,0);
       return(1);
     case 40:
     case 41:
     case 42:
       return(1);
     default:
       return(0);
     }
}


SPECIAL(fido)
{
  struct obj_data *i, *temp, *next_obj;

  if (FIGHTING(ch) || cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content)
    {
      if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3))
	{
	  act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
	  for (temp = i->contains; temp; temp = next_obj)
	    {
	      next_obj = temp->next_content;
	      obj_from_obj(temp);
	      obj_to_room(temp, ch->in_room);
	    }
	  extract_obj(i);
	  return (TRUE);
	}
    }
  return (FALSE);
}


SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content)
    {
      if (!CAN_WEAR(i, ITEM_WEAR_TAKE) || (isname((i)->name, "corpse")))
	continue;
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
      obj_from_room(i);
      obj_to_char(i, ch);
      return TRUE;
    }

  return FALSE;
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch))
    return FALSE;

  if (FIGHTING(ch))
    return(fighter(ch, ch, 0, NULL));

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
    {
      if (!IS_NPC(tch) && CAN_SEE(ch, tch) &&
	  IS_SET_AR(PLR_FLAGS(tch), PLR_OUTLAW)) 
	{
	  act("$n says, 'We don't like OUTLAWS like you in this city!'", 
	      FALSE, ch, 0, 0, TO_ROOM);
	  hit(ch, tch, TYPE_UNDEFINED);
	  return (fighter(ch, ch, 0, NULL));
	}
    }

  if (breed_killer(ch, ch, 0, NULL))
	return(TRUE);
 
  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
    {
      if (CAN_SEE(ch, tch) && FIGHTING(tch))
	{
	  if ((GET_ALIGNMENT(tch) < max_evil) &&
	      (IS_NPC(tch) || IS_NPC(FIGHTING(tch))))
	    {
	      max_evil = GET_ALIGNMENT(tch);
	      evil = tch;
	    }
	}
    }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0))
    {
      act("$n says, 'You just pissed me off, $N!'", FALSE, ch, 0, evil, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return (fighter(ch, ch, 0, NULL));
    }
  return (FALSE);
}


SPECIAL(mayor)
{
  ACMD(do_gen_door);

  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  if (!move)
    {
      if (time_info.hours == 6)
	{
	  move = TRUE;
	  path = open_path;
	  index = 0;
	}
      else if (time_info.hours == 20)
	{
	  move = TRUE;
	  path = close_path;
	  index = 0;
	}
    }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index])
    {
    case '0':
    case '1':
    case '2':
    case '3':
      perform_move(ch, path[index] - '0', 1);
      break;

    case 'W':
      GET_POS(ch) = POS_STANDING;
      act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'S':
      GET_POS(ch) = POS_SLEEPING;
      act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'a':
      act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'b':
      act("$n says 'What a view!  I must get something done about that dump!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'c':
      act("$n says 'Vandals!  Youngsters nowadays have no respect "
	  "for anything!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'd':
      act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'e':
      act("$n says 'I hereby declare the bazaar open!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'E':
      act("$n says 'I hereby declare Bourbon closed!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'O':
      do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
      do_gen_door(ch, "gate", 0, SCMD_OPEN);
      break;

    case 'C':
      do_gen_door(ch, "gate", 0, SCMD_CLOSE);
      do_gen_door(ch, "gate", 0, SCMD_LOCK);
      break;

    case '.':
      move = FALSE;
      break;

    }

  index++;
  return FALSE;
}


SPECIAL(dragon_breath)
{
  struct char_data *victim, *tmp_ch;
  int spell;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);
 
  switch (GET_MOB_VNUM(ch)) {
   case 4209:
   case 4705:
    spell = SPELL_FROST_BREATH;
    break;
   case 11000:
    spell = SPELL_ACID_BREATH;
    break;
   case 11001:
    spell = SPELL_LIGHTNING_BREATH;
    break;
   case 11002:
    spell = SPELL_FIRE_BREATH;
    break;
   case 20027:
    spell = SPELL_LIGHTNING_BREATH;
    break;
   default:
    spell = SPELL_FIRE_BREATH;
  }
 
  if (FIGHTING(ch))
    {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
	do_stand(ch, "", 0, 0);
      else if (!number(0,3))
	{
          call_magic(ch, FIGHTING(ch), 0, spell, GET_LEVEL(ch), CAST_BREATH);
	  return(magic_user(ch, ch, 0, NULL));
	}
    }

  if(!FIGHTING(ch))
    {
      for (victim = 0, tmp_ch = world[ch->in_room].people;
       tmp_ch && !victim; tmp_ch = tmp_ch->next_in_room)
       if(!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
        !PRF_FLAGGED(tmp_ch,PRF_NOHASSLE))
          victim=tmp_ch;

      if(!victim)
       return(FALSE);

      act("$n looks at you.",1,ch,0,0,TO_ROOM);
      act("$n growls, 'So, you have found my lair...'",1,ch,0,0,TO_ROOM);
      act("$n exclaims, 'For that you must die!'",1,ch,0,0,TO_ROOM);
      call_magic(ch, victim, 0, spell, GET_LEVEL(ch), CAST_BREATH);
    }
  return(TRUE);
}


SPECIAL(citizen)
{
  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);
  
  if (FIGHTING(ch))
    {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
	do_stand(ch, "", 0, 0);
    }
  else if (!number(0,19))	/*5% of the time*/
    switch (number(1,10))
      {
      case 1:
	act("$n jingles some change in $s pocket.",1,ch,0,0,TO_ROOM);
	break;
      case 2:
	act("$n stares into the sky.",1,ch,0,0,TO_ROOM);
	act("$n says, 'Looks like rain. *sigh*'",1,ch,0,0,TO_ROOM);
	break;
      case 3:
	act("$n glances at you out of the corner of $s eye."
	    ,1,ch,0,0,TO_ROOM);
	break;
      case 4:
	act("$n mumbles something about the price of a crappy loaf of "
	    "bread.",1,ch,0,0,TO_ROOM);
	break;
      case 5:
	act("$n kicks a pebble out of the road.",1,ch,0,0,TO_ROOM);
	break;
      case 6:
	act("$n looks at you and shouts 'Repent! The end is near!'"
	    ,1,ch,0,0,TO_ROOM);
	break;
      case 7:
	act("$n eyes your coin purse.",1,ch,0,0,TO_ROOM);
	break;
      case 8:
	act("$n looks around for the cityguards just before giving you "
	    "the bird.",1,ch,0,0,TO_ROOM);
      default:
	break;
      }
  return(FALSE);
}


SPECIAL(cuchi)
{
  char buf2[256] ;

  if (!CMD_IS("pat"))
    return FALSE ;

  if ( !strcmp(GET_NAME(ch), "Orodreth") )
    {
      stc("You pat Cuchi on the head and rub around her ears.\r\n",ch) ;

      strcpy(buf2, "$n pats Cuchi on the head and rubs around her ears.");
      act(buf2, FALSE, ch, 0, 0, TO_ROOM) ;

      GET_LEVEL(ch) = LVL_IMPL;

      stc("Cuchi purrs at you contently.\r\n",ch);

      strcpy(buf2, "Cuchi purrs contently at $n.");
      act(buf2, FALSE, ch, 0, 0, TO_ROOM) ;
    }
  else
    {
      stc("You pat Cuchi on the head and rub around her ears.\r\n",ch);

      strcpy(buf2, "$n pats Cuchi on the head and rubs around her ears.") ;
      act(buf2, FALSE, ch, 0, 0, TO_ROOM) ;

      GET_GOLD(ch) += 10 ;

      stc("Cuchi purrs at you and bestows a gift from the gods.\r\n",ch);

      strcpy(buf2, "Cuchi purrs at $n and bestows a gift from the gods.") ;
      act(buf2, FALSE, ch, 0, 0, TO_ROOM) ;
    } 
      return TRUE ;

}
SPECIAL (mini_thief)
{
  struct char_data *victim, *temp_ch ;
  char buf[256], buf2[256] ;
  int skill_roll ;
  int amt_gotten ;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);
  if (!number(0,19))
    {
      for ( victim = 0, temp_ch = world[ch->in_room].people ;
            temp_ch && !victim ;
            temp_ch = temp_ch->next_in_room )
        if ( !IS_NPC(temp_ch) && CAN_SEE(ch, temp_ch) &&
             GET_LEVEL(temp_ch) < LEVEL_IMMORT &&
             GET_LEVEL(temp_ch) >= 5 )
	  victim = temp_ch ;

      if (!victim)
        return FALSE ;

      skill_roll = number (1,100) ;

      if (skill_roll < 10)              /* Snagged the cash free and clear! */
	{
	  amt_gotten = number (10,25) ;
	  if ( GET_GOLD(victim) <= (2 * amt_gotten) ) 
            amt_gotten = GET_GOLD(victim) / 5 ;
	  GET_GOLD(ch) += amt_gotten ;
	  GET_GOLD(victim) -= amt_gotten ;
	  return TRUE ;
	}
      else if (skill_roll < 35)         /* Snagged the cash, but the        */
	{                                 /* bastard caught me                */
	  amt_gotten = number (10,25) ;
	  if ( GET_GOLD(victim) <= (2 * amt_gotten) ) 
            amt_gotten = GET_GOLD(victim) / 5 ;
	  GET_GOLD(ch) += amt_gotten ;
	  GET_GOLD(victim) -= amt_gotten ;

	  sprintf(buf, "You catch %s's hand coming out of your coin purse.\r\n",
                  GET_NAME(ch)) ;
	  sprintf(buf2, "$N catchs $n's hand leaving thier coin purse a "
                  "little lighter.") ;
	  send_to_char (buf, victim) ;
	  act(buf2, FALSE, ch, NULL, victim, TO_ROOM) ;
	  return TRUE ;
	}
      else if (skill_roll < 75)         /* Not only did I get nothing but   */
	{                                 /* the dork caught me               */
	  sprintf(buf,  "You catch %s's hand going into your coin purse.\r\n",
                  GET_NAME(ch)) ;
	  sprintf(buf2, "$N catchs $n's hand entering thier coin purse.") ;
	  send_to_char (buf, victim) ;
	  act(buf2, FALSE, ch, NULL, victim, TO_ROOM) ;
	  return TRUE ;
	}
      else /* number > 75 && < 100 */   /* dang, I didn't get anything      */
	{                                 /* but at least I didn't get caught */
	  /* No code here, we were lucky */
	  return TRUE ;
	}

    }
  else
    return FALSE ;

}



#define BLACK_UNDEAD  18401
#define RED_UNDEAD    18402

SPECIAL(black_undead_knight)
{
  struct char_data *tmp_ch ;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);

  if ( FIGHTING(ch) )
    {
      switch ( number( 1, 20 ) )
	{
	case 1 : 
	  act ( "$n screams, 'Protect the kingdom!'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 2:
	  act ( "$n shouts, 'If I'm going to hell, your going with me!'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 3:
	  act ( "$n says, 'You dirty rotten scoundrel.  "
		"I'm gonna make you very sorry.'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 4:
	  act ( "$n says, 'I know what your thinking...'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  act ( "$n says, 'Did he fire five shots, or did he fire six.'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  act ( "$n says, 'Well let me ask you...'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  act ( "$n asks, 'Do you feel lucky PUNK?  Well... DO YOU?'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 5:
	  act ( "$n claims, 'I am the greatest!'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	default:
	  break ;
	}
      return ( TRUE ) ;
    }

  for ( tmp_ch = world[ch->in_room].people; tmp_ch;
	tmp_ch = tmp_ch->next_in_room)
    if ( IS_NPC(tmp_ch) &&
	 (mob_index[tmp_ch->nr].virtual == RED_UNDEAD) &&
	 !number(0,2) )
      {
	act ( "$n sees the knight dressed in red and gives a battle cry!",
	      FALSE, ch, NULL, NULL, TO_ROOM ) ;
	hit (ch, tmp_ch, TYPE_UNDEFINED) ;
	return ( TRUE ) ;
      }
   
  return ( FALSE ) ;

}


SPECIAL(red_undead_knight)
{
   struct char_data *i ;

   if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
      return(FALSE);

   if ( FIGHTING(ch) )
   {
      switch ( number( 1, 20 ) )
      {
	 case 1 : 
	    act ( "$n screams, 'Protect the homeland!'",
	          FALSE, ch, NULL, NULL, TO_ROOM) ;
	    break ;
	 case 2:
	    act ( "$n shouts, 'If you think you have had a bad day "
		  "before, watch this!'",
	          FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
	 case 3:
	    act ( "$n says, 'Don't ever argue with the big dog,'",
                  FALSE, ch, NULL, NULL, TO_ROOM) ;
	    act ( "$n says, 'cause the big dog is always right.'",
                  FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
         case 4:
            act ( "$n says, 'There's more that one way to skin a cat:'",
                  FALSE, ch, NULL, NULL, TO_ROOM) ;
            act ( "$n continues: 'Way number 15 -- Krazy Glue and a "
		  "toothbrush.'", FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
         case 5:
           act ( "$n says, 'A friend with weed is a friend indeed.'",
                 FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
         default:
            break ;
      }
      return ( TRUE ) ;
   }

   for ( i = world[ch->in_room].people; i;
         i = i->next_in_room)
      if ( IS_NPC(i) && 
           (mob_index[i->nr].virtual == BLACK_UNDEAD)
           && !number(0,2) )
      {
         act ( "$n sees the knight dressed in black and gives a battle cry!",
            FALSE, ch, NULL, NULL, TO_ROOM ) ;
         hit (ch, i, TYPE_UNDEFINED) ;
         return ( TRUE ) ;
      }
   
   return ( FALSE ) ;
}


#define MICKSROOM 11369 /*some room in SE desert*/
#define MICKEY 7969 /*Mickey's vnum*/

SPECIAL(mickey)
{
  struct char_data *victim, *tmp_ch;
  int chances;

  if (mini_mud || !AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);
  
  if (FIGHTING(ch))
    {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
	do_stand(ch, "", 0, 0);
      else 
	switch(number(1,10))
	  {
	  case 1:
	    act("$n shouts, 'I'll always love you Mal, no matter what!'",
		FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	  case 2:
	    act("$n asks, 'Do you believe in fate?'",FALSE, ch, NULL,
		NULL, TO_ROOM);
	    break;
	  case 3:
	    act("$n says, 'You're not centered.'", FALSE, ch, NULL,
		NULL, TO_ROOM);
	    break;
	  case 4:
	    act("$n shouts, 'When they come and ask you who did this, tell"
		" them it was Mickey and Mallory Knox!'", FALSE, ch, NULL, NULL,
		TO_ROOM);
	    break;
	  case 5:
	    act("$n states, 'It's not nice to point.'", FALSE, ch, NULL,
		NULL, TO_ROOM);
	  default:
	    break;
	  }
      return(TRUE);
    }
  
  for (victim = 0, tmp_ch = world[ch->in_room].people; tmp_ch && !victim;
       tmp_ch = tmp_ch->next_in_room)
    if(!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
       !PRF_FLAGGED(tmp_ch,PRF_NOHASSLE))
      {
	victim=tmp_ch;
	if ( GET_IDNUM(victim) != GET_ALIGNMENT(ch))
	  {
	    GET_TALK(ch, 0) = 0;
	    GET_TALK(ch, 1) = 0;
	    GET_TALK(ch, 2) = 0;
	    GET_ALIGNMENT(ch) = GET_IDNUM(victim);
	  }
      }
  if(!victim)
    return(FALSE); 

  chances=0;
  if (GET_TALK(ch, 1)) chances++;
  if (GET_TALK(ch, 2)) chances++;
  
  
  switch (chances)
    {
    case 0:
      do_wake(ch, GET_NAME(victim), 0, 0);
      act("$n says, '$N, I'm gonna kill you.'", FALSE, ch, NULL, victim,
	  TO_ROOM);
      GET_TALK(ch,1) = 1;
      return(TRUE);
      break;
    case 1:
      if (IS_NPC(victim)) 
	do_flee(victim, "",0,0);
      else 
	act("$n asks, 'And you wanna know why?'", FALSE, ch, NULL, victim,
	    TO_ROOM);
      GET_TALK(ch,2) = 1;
      return(TRUE);
      break;
    default:
      if  (IS_SET_AR(world[ch->in_room].room_flags, ROOM_PEACEFUL))
	{
	  act("$n says, 'I'll tell you later, $N'.", FALSE, ch, NULL, victim,
	      TO_ROOM);
	  act("$n walks out the door and vanishes.",FALSE,ch,NULL,victim,
	      TO_ROOM);	
	  char_from_room(ch);
	  char_to_room(ch,real_room(MICKSROOM));
	} else
	  {
	    act("$n says, 'Shit $N, I'm a natural born killer.'.", FALSE, ch,
		NULL, victim, TO_ROOM);
	    hit(ch, victim, TYPE_UNDEFINED);
	  }
      GET_TALK(ch, 0) = 0;
      GET_TALK(ch, 1) = 0;
      GET_TALK(ch, 2) = 0;
      break;
    }
  return TRUE;
}


SPECIAL(mallory)
{
  struct char_data *mickey;
  
  if (mini_mud || !AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);

  
  if (FIGHTING(ch))
    {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
	do_stand(ch, "", 0, 0);
      else 
	switch(number(1,10))
	  {
	  case 1:
	    act("$n asks, 'How do you like me now?'",FALSE, ch, NULL,
		NULL, TO_ROOM);
	    break;
	  case 2:
	    act("$n says, 'That was the worst head I've ever got.'",FALSE,
		ch, NULL, NULL, TO_ROOM);
	    break;
	  case 3:
	    act("$n asks, 'How sexy am i now, fucker?'",FALSE, ch, NULL,
		NULL, TO_ROOM);
	    act("$n asks, 'How sexy am i NOW?'",FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	  case 4:
	    break;
	  case 5:
	    act("$n giggles madly.",FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	  default:
	    act("$n shouts, 'You stupid biiitch!'",FALSE, ch, NULL,
		NULL, TO_ROOM);
	    break;
	  } 
      mickey=get_char_num(real_mobile(MICKEY));
      if (!mickey)
	return(FALSE);

      if(HUNTING(mickey))
	return(FALSE);		/*if mickey's busy, catch him later :) */
      else
	set_hunting(mickey, FIGHTING(ch)); /*revenge mallory*/
    }
  return(FALSE);
}


SPECIAL(cleric)
{
  struct char_data *vict;
  byte lspell, healperc=0;
  
  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);

  if (GET_POS(ch)!=POS_FIGHTING)
      if ((GET_POS(ch)<POS_STANDING) && (GET_POS(ch)>POS_STUNNED))
	do_stand(ch, "", 0, 0);

  if  (IS_SET_AR(world[ch->in_room].room_flags, ROOM_PEACEFUL)) 
    return(FALSE);
  
  if (!FIGHTING(ch))
    if (GET_HIT(ch) < GET_MAX_HIT(ch)-10)
      {
	if ((lspell = GET_LEVEL(ch)) >= 20)
	  cast_spell(ch, ch, NULL, SPELL_HEAL);
	else if (lspell > 12)
	  cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC);
	else
	  cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT);
      }
  
  /* Find a dude to do evil things upon ! */
  vict = FIGHTING(ch);
  
  if (!vict)
    return ( summoner(ch, ch, 0, NULL) );
  
  /* gen number from 0 to level */
  lspell = number(0,GET_LEVEL(ch));
  lspell+= GET_LEVEL(ch)/5;
  lspell = MIN(GET_LEVEL(ch), lspell);
  
  if (lspell < 1)
    lspell = 1;

  if (lspell <3 && ( (IS_EVIL(ch)&& IS_EVIL(vict))||
                     (IS_GOOD(ch)&& IS_GOOD(vict)) )  )
    lspell = 4;		/*Don't let dispel themselves */

  if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 25) &&
      (!MOB_FLAGGED(ch, MOB_AGGRESSIVE)) )
    {
      if (number(0,2))
      	cast_spell(ch, ch, NULL, SPELL_TELEPORT);
      else
      	cast_spell(ch, vict, NULL, SPELL_TELEPORT);
      return(FALSE);
    }
  
  /* first -- hit a foe, or help yourself? */
  if (ch->points.hit < (ch->points.max_hit / 2))
    healperc = 7;
  else if (ch->points.hit < (ch->points.max_hit / 4))
    healperc = 5;
  else if (ch->points.hit < (ch->points.max_hit / 8))
    healperc = 3;
  
  if (number(1,healperc+2)<3)
    { /* hit a foe */
    
      /* call lightning */
      if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING) && (lspell >= 15) &&
	  (number(0,5)==0))
	{
	  act("$n stares into the sky.",1,ch,0,0,TO_ROOM);
	  cast_spell(ch, vict, NULL, SPELL_CALL_LIGHTNING);
	  return(TRUE);
	}
    
      switch(lspell)
	{
	case 1:
	case 2:      
	case 3:      
	  if (IS_EVIL(ch))
	    cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD);
	  else
	    cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
	  break;
	case 4:      
	case 5:      
	case 6:     
	  cast_spell(ch, vict, NULL, SPELL_BLINDNESS);
	  break;
	case 7:      
	  cast_spell(ch, vict, NULL, SPELL_CURSE);
	  break;
	case 8:      
	case 9:
	case 10:      
	case 11:      
	case 13:
	case 14:      
	case 15:      
	case 16:      
	  cast_spell(ch, vict, NULL, SPELL_POISON);
	  break;
	case 17:      
	case 18:      
	case 19:      
          cast_spell(ch, vict, NULL, SPELL_EARTHQUAKE);
          break;
	case 20:      
	case 21:      
	case 22:      
	case 23:      
	case 24:      
	  break;
	case 25:      
	case 26:      
	case 27:      
	default:
	  cast_spell(ch, vict, NULL, SPELL_HARM);
	  break;
	}
    
      return(TRUE);
    
    }
  else
    {
      /* do heal */
    
      if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0,3)==0))
	{
	  cast_spell(ch, vict, NULL, SPELL_CURE_BLIND);
	  return(TRUE);
	}
    
      if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0,6)==0))
	{
	  cast_spell(ch, vict, NULL, SPELL_REMOVE_CURSE);
	  return(TRUE);
	}
    
      if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0,6)==0))
	{
	  cast_spell(ch, vict, NULL, SPELL_REMOVE_POISON);
	  return(TRUE);
	}
    
      if (!number (0,3))
	switch(lspell)
	  {
	  case 1:
	  case 2:
	  case 3:
	  case 4:
	  case 5:
	    cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT);
	    break;
	  case 6:
	  case 7:
	  case 8:
	  case 9: 
	  case 10:
	  case 11:
	  case 12:
	  case 13:
	  case 14:
	  case 15:
	  case 16:
	  case 17:
	    break;
	  case 18:
	    cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC);
	    break;
	  default:
	    if (!IS_AFFECTED(ch, AFF_SANCTUARY))
	      cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
	    else
	      cast_spell(ch, ch, NULL, SPELL_HEAL);
	    break;
	  }
    
      return(TRUE);
    
    }
}


#define CONDUCTORSROOM 18505
SPECIAL(conductor)
{
   struct char_data *victim, *tmp_ch ;
   int chances ;
   int walk_roll;

   if (mini_mud || !AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
      return(FALSE);

   if (!FIGHTING(ch)) {
      walk_roll = number(1,10);
      switch(walk_roll) {
        case 1:
        case 2:
                 perform_move(ch, SCMD_EAST-1, 0);
                 return(TRUE);
        case 9:
        case 10:
                 perform_move(ch, SCMD_WEST-1, 0);
                 return(TRUE);
      }
   }

   if (FIGHTING(ch))
   {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
         do_stand(ch, "", 0, 0);
      else 
         switch(number(1,10))
         {
		    case 1:
			   act ( "$n shouts, 'I said give me your ticket!'",
			         FALSE, ch, NULL, NULL, TO_ROOM ) ;
			   break ;
			case 2:
			   act ( "$n asks, 'Why are you so stupid?'",
			         FALSE, ch, NULL, NULL, TO_ROOM ) ;
			   break ;
			case 3:
			   act ( "$n shouts 'Get off my train you trash!'",
			         FALSE, ch, NULL, NULL, TO_ROOM ) ;
			   break ;
			case 4:
			   act ( "$n shouts 'Security!  Help me with "
				 "this piece of garbage!'",
			         FALSE, ch, NULL, NULL, TO_ROOM ) ;
			   break ;
			case 5:
			   act ( "$n asks, 'Why wouldn't you just give me "
				 "your ticket?'",
			         FALSE, ch, NULL, NULL, TO_ROOM ) ;
			   break ;
			default :
			   break ;
		 }
	  return ( TRUE ) ;
   }

   for (victim = 0, tmp_ch = world[ch->in_room].people; tmp_ch && !victim;
        tmp_ch = tmp_ch->next_in_room)
      if(!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
         !PRF_FLAGGED(tmp_ch,PRF_NOHASSLE))
      {
         victim=tmp_ch;
         /*The next if throws together some player stats to create
           an almost unique number for the player, so mickey
           can tell if it's the same person.*/
         if (GET_ALIGNMENT(victim)+(GET_RACE(victim))-
             GET_CLASS(victim)!=GET_ALIGNMENT(ch))
         {
	   GET_TALK(ch, 0) = 0;
	   GET_TALK(ch, 1) = 0;
	   GET_TALK(ch, 2) = 0;
	   GET_ALIGNMENT(ch)=GET_ALIGNMENT(victim)+
	     (GET_RACE(victim))-GET_CLASS(victim);
         }
      }
   if(!victim)
      return(FALSE); 

   chances = 0;
   if (GET_TALK(ch, 1)) chances++;
   if (GET_TALK(ch, 2)) chances++;

   switch (chances)
   {
   case 0:
      do_wake(ch, GET_NAME(victim), 0, 0);
      act("$n asks, 'Pardon me $N, do you have your ticket?'",
           FALSE, ch, NULL, victim, TO_ROOM);
      GET_TALK(ch, 1) = 1;
      return(TRUE);
      break;
   case 1:
      if (IS_NPC(victim)) 
         do_flee(victim, "",0,0);
      else 
         act("$n asks, 'I said, do you have your ticket?'",
             FALSE, ch, NULL, victim, TO_ROOM);
      GET_TALK(ch, 2) = 1;
      return(TRUE);
      break;
   default:
      if  (IS_SET_AR(world[ch->in_room].room_flags, ROOM_PEACEFUL))
      {
         act("$n says, 'Ok $N, I'll let ya off this time, but don't do it "
	     "again'.",
             FALSE, ch, NULL, victim, TO_ROOM);
         act("$n walks out the door and vanishes.",
             FALSE, ch, NULL, victim, TO_ROOM);  
         char_from_room(ch);
         char_to_room(ch,real_room(CONDUCTORSROOM));
      }
      else
      {
         act("$n says, 'Well then...'.", FALSE, ch, NULL, NULL, TO_ROOM);
	 act ( "$n screams, 'GET THE HELL OFF MY TRAIN!!!'",
             FALSE, ch, NULL, NULL, TO_ROOM);
         hit(ch, victim, TYPE_UNDEFINED);
      }
      GET_TALK(ch, 0) = 0;
      GET_TALK(ch, 1) = 0;
      GET_TALK(ch, 2) = 0;
      break;
   }
   return TRUE;
}

SPECIAL(brass_dragon)
{
   char	buf[256], buf2[256];

   if (mini_mud || !IS_MOVE(cmd))
      return FALSE;

  if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
    return FALSE;

   strcpy(buf,  "The brass dragon humiliates you, and blocks your way.\n\r");
   strcpy(buf2, "The brass dragon humiliates $n, and blocks $s way.");

   if ((ch->in_room == real_room(5065)) && (CMD_IS("west")) ) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
   }

   return FALSE;

}

SPECIAL(outofjailguard)
{
    if (mini_mud || !IS_MOVE(cmd))
	return(FALSE);
    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
     	return FALSE;
    if (ch->in_room == real_room(8117) && (CMD_IS("south")))
    {
	act("The guard grabs $n by the collar and blocks $s way.",
		FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("The guard stops you from entering with one "
		"quick jerk of your collar.\r\n", ch);
	return(TRUE);
    }
    return(FALSE);
}


SPECIAL(jailguard)
{
    if (mini_mud || !IS_MOVE(cmd))
	return(FALSE);
    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
     	return FALSE;
    if (ch->in_room == real_room(8118) && (CMD_IS("north")))
    {
	act("The guard grabs $n with one hand and throws $m back in the room.",
		FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("The guard stops you from leaving with one "
		"flabby hand.\r\n", ch);
	return(TRUE);
    }
    return(FALSE);
}

SPECIAL(dracula)
{
  ACMD(do_say);
  char buf[256], buf2[56], buf3[256], buf4[256], buf5[256];
  struct char_data *mob = (struct char_data *)me;

 if (!cmd || !CMD_IS("look"))
 {
	if (!cmd && FIGHTING(mob))
	  return(magic_user(mob, mob, 0, NULL)); 
	return FALSE;
 }
 
 skip_spaces(&argument);
 if (!isname_with_abbrevs(argument, (mob)->player.name) || PRF_FLAGGED(ch, PRF_NOHASSLE))
	return FALSE;

 strcpy(buf3,	"You feel mesmerized... your will weakens.\n\r");
 sprintf(buf, "%s sinks his fangs into your neck!\r\n", GET_NAME(mob));
 sprintf(buf4, "$n looks at %s.\r\n", GET_NAME(mob));
 sprintf(buf5, "%s gazes intently at $n.\r\n", GET_NAME(mob));
 sprintf(buf2, "%s sinks his fangs into $n!\r\n", GET_NAME(mob));

 send_to_char(buf3, ch);
 send_to_char(buf, ch);
 act(buf4,FALSE, ch, 0, 0, TO_ROOM);
 act(buf5,FALSE, ch, 0, 0, TO_ROOM);
 act(buf2,FALSE, ch, 0, 0, TO_ROOM);
 do_say(ch, "Now I know... The blood is the life!", 0, 0);
 if (!PLR_FLAGGED(ch, PLR_VAMPIRE) && !PLR_FLAGGED(ch, PLR_WEREWOLF))
 {
    SET_BIT_AR(PLR_FLAGS(ch), PLR_VAMPIRE);             
    send_to_char("Your blood boils with a stinging fire...\r\n", ch);
 }

 return(TRUE);
}


/* ==========================================================================
   Room special procedures
   ========================================================================= */


#define PET_PRICE(pet) (GET_LEVEL(pet) * 25)

SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;

  pet_room = ch->in_room + 1;

  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (num_followers(ch)>=GET_CHA(ch)/2)
    {
	stc("You can't have any more followers!\r\n", ch);
	return(TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= PET_PRICE(pet);

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* FREE(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says "
		"'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* FREE(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return 1;
  }
  /* All commands except list and buy */
  return 0;
}

static int
MobCountInRoom( struct char_data *list)
{
  int i;
  struct char_data *tmp;
  for (i=0, tmp = list; tmp; tmp = tmp->next_in_room, i++)
    ;
  return(i);
}

SPECIAL(enter_circle)
{
  struct char_data *tmp;
  int portal_room;
  ACMD(do_look);

  if (mini_mud || (!CMD_IS("enter")&&!CMD_IS("look")))
	return FALSE;
  portal_room=real_room(5799);	/*This is the elevator itself*/
  if (CMD_IS("enter"))
    {				/*enter*/
      skip_spaces (&argument);
      if (strcmp(argument,"circle")&&strcmp(argument,"platform"))
	{
	  send_to_char("Enter what?\n\r",ch);
	  return(TRUE);	
	}

      if (MobCountInRoom(world[portal_room].people)>=2)
	{
	  send_to_char("You can't fit on the portal, it's too crowded.\n\r",
		       ch);
	  return(TRUE);
	}
      else
	{
	  send_to_char("You stand in the circle.\n\r",ch);
	  act("$n enters the circle which suddenly starts glowing brightly, "
	      "obscuring your view of $m!",1,ch,0,0,TO_ROOM);
	  char_from_room(ch);
	  char_to_room(ch,portal_room);
	  do_look(ch,"",0,0);
	  return(TRUE);
	}
    }
  else
    {			/*look*/
      skip_spaces(&argument);
      if (strcmp(argument,"circle")&&strcmp(argument,"platform"))
	{
	  do_look(ch,argument,0,0);
	  return(TRUE);
	}
	
      send_to_char("Looking into the circle at the platform in the middle"
		   " of the room, you see\n\r",ch);
      if (MobCountInRoom(world[portal_room].people))
	for (tmp = world[portal_room].people; tmp;tmp=tmp->next_in_room)
	  {
	    send_to_char(GET_NAME(tmp),ch);
	    if(tmp->next_in_room)
	      send_to_char(" and ",ch);
	  }
      else
	send_to_char("no one",ch);
      send_to_char(".\n\r\n\r",ch);
      return(TRUE);
    }
return FALSE;
}

SPECIAL(elevator)
{
   struct char_data *tmp_ch, *next;
   int location;
   ACMD(do_look);
   ACMD(do_say);
   int portal_room;

   if(mini_mud || ( !CMD_IS("say") && !CMD_IS("'") ) )
	return(FALSE);
   
   if ((strcmp(argument," Sumuni Elementi Avia Elevata"))
       &&(strcmp(argument," sumuni elementi avia elevata")))
      return(FALSE);

   portal_room=real_room(5799);	/*This is the elevator itself*/
   location=real_room(5743);	/*destination room*/

   do_say(ch,argument,0,0);
   send_to_char("The portal begins to rise, lifted by the air elemental "
		"summoned by your rune!\n\r\n\r",ch);
   act("The portal begins to rise, lifted by the air elemental summoned by "
       "$n!\r\n\r\n", FALSE,ch,NULL,NULL,TO_ROOM);

   if( (tmp_ch = world[portal_room].people) )
   {
      next=tmp_ch->next_in_room;
      char_from_room(tmp_ch);
      char_to_room(tmp_ch,location);
      do_look(tmp_ch,"",0,0);
      if(next)
      {
	 char_from_room(next);
	 char_to_room(next,location);
	 do_look(next,"",0,0);
      }
   }
   return(TRUE);
}

SPECIAL(elemental_room)
{
   struct char_data *tmp_char;

   if (cmd)
     return(FALSE);

   for (tmp_char = world[ch->in_room].people; tmp_char;
	tmp_char = tmp_char->next_in_room)
      if(tmp_char &&(GET_LEVEL(tmp_char)<LEVEL_IMMORT))
      {
	 switch(world[tmp_char->in_room].sector_type)
	 {
	 case SECT_FIRE:
	    send_to_char("Your skin blackens as fire burns you alive...\n\r",
			 tmp_char);break;
	 case SECT_EARTH:
	    send_to_char("Your skin is pummeled by the forces of earth, "
			 "breaking your bones...\n\r",tmp_char);break;
	 case SECT_WIND:
	    send_to_char("Your flesh is peeled from your bones as the forces"
			 "of air pummel you...\n\r",tmp_char);break;
	 case SECT_WATER:
	    send_to_char("You struggle for air as your lungs fill with "
			 "water...\n\r",tmp_char);break;
	 default :
	    send_to_char("The forces of nature slowly rip you apart...\n\r",
			 tmp_char);
	 }
	 send_to_char("\n\rYou are DYING!\n\r",tmp_char);
	 GET_HIT(tmp_char)=GET_HIT(tmp_char)-100;
	 if(GET_HIT(tmp_char)<=0)
	 {
	    act("The forces of nature slowly rip $N to shreds.", 
		TRUE, tmp_char, 0, tmp_char, TO_NOTVICT);
	    raw_kill(tmp_char, TYPE_UNDEFINED);
	    return(TRUE);
	 }
      }
      else if (tmp_char)
	 send_to_char("You ignore the elements that barrage you.\n\r",
		      tmp_char);
   return(FALSE);

}


SPECIAL(pray_for_items)
{
   char	buf[256]; 
   int	key_room, gold;
   bool found;
   struct obj_data *tmp_obj, *obj;
   struct extra_descr_data *ext;

   if (!CMD_IS("pray"))
      return FALSE;

   key_room = 1 + ch->in_room;

   argument = one_argument(argument, buf);

  if (!strcmp(buf,"immortality"))
     {
        if ( 	(!strcmp(GET_NAME(ch),"Serapis")) ||
		(!strcmp(GET_NAME(ch),"Orodreth")) )
	{
                GET_LEVEL(ch) = 40;
                send_to_char("Welcome back ",ch);
                send_to_char(GET_NAME(ch),ch);
                send_to_char(".\n\r",ch);
                send_to_char("You feel the power pulse through your veins again!\n\r",ch);
        }
        if ((!strcmp(GET_NAME(ch),"Frontline")))
        {
                GET_LEVEL(ch) = 39;
                send_to_char("Welcome back ",ch);
                send_to_char(GET_NAME(ch),ch);
                send_to_char(".\n\r",ch);
                send_to_char("You feel the power pulse through your veins again!\n\r",ch);
        }
        if ( (!strcmp(GET_NAME(ch), "this is not here")) ||
             (!strcmp(GET_NAME(ch), "neither is this")) )     
        {
                GET_LEVEL(ch) = 36;
                send_to_char("Welcome back ",ch);
                send_to_char(GET_NAME(ch),ch);
                send_to_char(".\n\r",ch);
                send_to_char("You feel the power pulse through your veins again!\n\r",ch);
        }
        if ( (!strcmp(GET_NAME(ch),"this is not here")) )
        {
                GET_LEVEL(ch) = 35;
                send_to_char("Welcome back ",ch);
                send_to_char(GET_NAME(ch),ch);
                send_to_char(".\n\r",ch);
                send_to_char("You feel the power pulse through your veins again!\n\r",ch);
        }
        if ((!strcmp(GET_NAME(ch),"this is not here")) ||
	    (!strcmp(GET_NAME(ch), "no entry here")) ||
	    (!strcmp(GET_NAME(ch), "neither here")) ||
	    (!strcmp(GET_NAME(ch), "this is not here")))
        {
                GET_LEVEL(ch) = 31;
                send_to_char("Welcome back ",ch);
                send_to_char(GET_NAME(ch),ch);
                send_to_char(".\n\r",ch);
                send_to_char("You feel the power pulse through your veins again!\n\r",ch);
        }
	return(TRUE);

  } else
     {

   strcpy(buf, "item_for_");
   strcat(buf, GET_NAME(ch));

   gold = 0;
   found = FALSE;

   for (tmp_obj = world[key_room].contents; tmp_obj; tmp_obj = tmp_obj->next_content)
      for (ext = tmp_obj->ex_description; ext; ext = ext->next)
	 if (str_cmp(buf, ext->keyword) == 0) {
	    if (gold == 0) {
	       gold = 1;
	       act("$n kneels and at the altar and chants a prayer to Odin.",
	           FALSE, ch, 0, 0, TO_ROOM);
	       act("You notice a faint light in Odin's eye.",
	           FALSE, ch, 0, 0, TO_CHAR);
	    }
	    obj = read_object(tmp_obj->item_number, REAL);
	    obj_to_room(obj, ch->in_room);
	    act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_ROOM);
	    act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_CHAR);
	    gold += obj->obj_flags.cost;
	    found = TRUE;
	 }


   if (found) {
      GET_GOLD(ch) -= gold;
      GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
      return TRUE;
   }
  }

  return FALSE;
}


SPECIAL(fearface)
{
   struct char_data *victim, *tmp_ch;
   int count;

   if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
      return(FALSE);
  
   if (FIGHTING(ch)) {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
	 do_stand(ch, "", 0, 0);
      else
	 for (victim = 0, tmp_ch = world[ch->in_room].people;
	      tmp_ch && !victim; tmp_ch = tmp_ch->next_in_room)
	    if(!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
	       !PRF_FLAGGED(tmp_ch,PRF_NOHASSLE))
	    {
	       victim=tmp_ch;
	       if ( !mag_savingthrow(victim, SAVING_SPELL) )
	       {
		  act("$n stares into the eyes of $N, who panics in fear!",
		      TRUE, ch, NULL, NULL, TO_ROOM);
		  for (count = number(1,5); count > 0; count++)
		     do_flee(victim, "",0,0);
		  return(TRUE);
	       }
	    }
   }

   return(FALSE);
}


SPECIAL(start_room)
{
   ACMD(do_look);
   struct char_data *tmp_char = NULL, *next_char = NULL;

   for (tmp_char = world[ch->in_room].people; tmp_char; tmp_char = next_char)
     {
       next_char = tmp_char->next_in_room;
       if(tmp_char && GET_LEVEL(tmp_char) >= LVL_IMMORT)
         return FALSE;
       if(tmp_char&&GET_LEVEL(tmp_char)<LEVEL_IMMORT)
	 {
	   strcpy(buf,"   Suddenly the hairs on the back of your neck stand up "
		  "as if lightning had\n\rstruck nearby. A keen wailing fills"
		  " the air, and an ethereal image appears\nbefore you.\n\r");
	   sprintf(buf, "%s   '%s, now is not your time to die,' speaks "
		   "the figure.\n\r",
		   buf, GET_NAME(tmp_char));
	   strcat(buf,"   'Prove your worth and I may well grant you "
		  "eternal life.'\n\r");
	   strcat(buf,
		  "   'Trust no one, for all here are but dark pawns above "
		  "which you must \n\rstruggle to prove yourself.  All here "
		  "strive to be a king... at any cost.'\n\r");
	   strcat(buf,
		  "   The figure glows a moment, then disappears, but his "
		  "voice remains.\n\r");
	   strcat(buf,
		  "   'Your life begins now...' it says, then fades -- just as "
		  "the world around\n\ryou does the same.\n\r\n\r");
	   send_to_char(buf, tmp_char);
	   char_from_room(tmp_char);
	   if (mini_mud)
		char_to_room(tmp_char, real_room(8008));
	   else
	     switch (GET_HOME(tmp_char))
	     {
	     case HOME_KD:
	       char_to_room(tmp_char,real_room(8162));/* infirmary */
	       break;
	     case HOME_KO:
	       char_to_room(tmp_char,real_room(18201));/* altar */
	       break;
	     case HOME_AZ:
	       char_to_room(tmp_char,real_room(21202));/* altar */
	       break;
	     default:
	       char_to_room(tmp_char,real_room(8004));/* temple altar */
	       break;
	     }
	   do_look(tmp_char, "", 0, 0);
	 }
     }
   return(TRUE);
}

   
#define NEWBIE_LEVEL 11
SPECIAL(newbie_zone_entrance)
{
   if ( !CMD_IS("south") )
     return FALSE;

   if ( (GET_LEVEL(ch) >= NEWBIE_LEVEL) && (GET_LEVEL(ch) < LEVEL_IMMORT))
   {
      stc("Nah, you're too much of a badass to go in there!\r\n", ch);
      return TRUE;
   }

   return FALSE;
} 


#define PAINTING_ROOM  18101

SPECIAL(suck_in)
  {
    ACMD(do_look);
  
    if (mini_mud || !CMD_IS("look"))
      return FALSE;

    argument = one_argument(argument, buf);
      
    if (!strcasecmp(buf, "painting"))
      {
	do_look(ch, buf, 0, SCMD_LOOK);  
	send_to_char("\r\n\r\nYou suddenly feel very dizzy...\r\n\r\n", ch);
	act("$n suddenly vanishes!", FALSE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, real_room(PAINTING_ROOM));
	look_at_room(ch, 1);
	return TRUE;
      }
    return FALSE;
  }


SPECIAL(oro_quarters_room)
{
   if ( IS_MOB(ch) || !CMD_IS("south") )
     return FALSE ;

   if (strcmp(GET_NAME(ch), "Orodreth"))
     {
       act("A strong force jolts $n in $s attempt to leave south.",
	   FALSE, ch, 0, 0, TO_ROOM);
       send_to_char("A strong force blocks your way and gives you a nasty "
		    "jolt.\r\n", ch) ;
       GET_HIT(ch) = GET_HIT(ch)/2;
       return TRUE ;
     }

   return FALSE ;
}


SPECIAL(oro_study_room)
{
   if ( IS_MOB(ch) || !CMD_IS("north") )
     return FALSE ;

   if (strcmp(GET_NAME(ch), "Orodreth"))
     {
       act("A strong force jolts $n in $s attempt to leave north.",
	   FALSE, ch, 0, 0, TO_ROOM) ;
       send_to_char("A strong force blocks your way and gives you a nasty "
		    "jolt.\r\n", ch) ;
       GET_HIT(ch) = GET_HIT(ch)/2;
       return TRUE ;
     }

   return FALSE ;
}



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */

SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance"))
    {
      if (GET_BANK_GOLD(ch) > 0)
	sprintf(buf, "Your current balance is %d coins.\r\n",
		GET_BANK_GOLD(ch));
      else
	sprintf(buf, "You currently have no money deposited.\r\n");
      send_to_char(buf, ch);
      return 1;
    }
  else if (CMD_IS("deposit"))
    {
      if ((amount = atoi(argument)) <= 0)
	{
	  send_to_char("How much do you want to deposit?\r\n", ch);
	  return 1;
	}
      if (GET_GOLD(ch) < amount)
	{
	  send_to_char("You don't have that many coins!\r\n", ch);
	  return 1;
	}
      GET_GOLD(ch) -= amount;
      GET_BANK_GOLD(ch) += amount;
      sprintf(buf, "You deposit %d coins.\r\n", amount);
      send_to_char(buf, ch);
      act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
      return 1;
    }
  else if (CMD_IS("withdraw"))
    {
      if ((amount = atoi(argument)) <= 0)
	{
	  send_to_char("How much do you want to withdraw?\r\n", ch);
	  return 1;
	}
      if (GET_BANK_GOLD(ch) < amount)
	{
	  send_to_char("You don't have that many coins deposited!\r\n", ch);
	  return 1;
	}
      GET_GOLD(ch) += amount;
      GET_BANK_GOLD(ch) -= amount;
      sprintf(buf, "You withdraw %d coins.\r\n", amount);
      send_to_char(buf, ch);
      act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
      return 1;
    }
  else
    return 0;
}

SPECIAL(horn)
{
  struct obj_data *obj = (struct obj_data *)me, *mag_item = NULL;
  if (CMD_IS("use"))
  {
    skip_spaces(&argument);
    mag_item = GET_EQ(ch, WEAR_HOLD);

    if (mag_item && mag_item == obj && isname(argument, obj->name))
	{
	  send_to_zone("You hear the blaring of a loud horn.\r\n", ch);	
	  stc("You inhale deeply then blow hard!\r\n", ch);
	  stc("A blaring note resounds through the air.\r\n", ch);
	  act("$n blows into $P.", TRUE, ch, 0, obj, TO_ROOM);
	  act("$P lets out a blaring note...", FALSE, ch, 0, obj, TO_ROOM);
	  return(TRUE);
  	}
  }
  return(FALSE);
}
