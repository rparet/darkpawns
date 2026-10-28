/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
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

/* $Id: class.c 1229 2005-10-15 12:42:35Z mcooper $ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */
#include <math.h>
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "handler.h"


/* menu for selecting a hometown during char creation */
const char *hometown_menu =
"\r\n"
"Choose your home town:\r\n"
"  [&cK&n]ir Drax'in  - The Main City. (&c*&n).\r\n"
"  Kir-[&cO&n]shi     - The Port City.\r\n"
"  Kir-[&cM&n]orthis  - The Holy City.\r\n"
"  [&cX&n]ixieqi      - Port City of the Southern Continent.  (&c**&n)\r\n"
"  Mis[&ct&n] Keep    - City of the Monks.  (&c**&n)\r\n"
"\r\n"
"&c*&n New players should choose this city.\r\n"
"&c**&n Experienced players may like to choose these cities.\r\n";

/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL	0
#define SKILL	1
#define BOTH    2

/* Some initializations for characters, including initial skills */
/* This function may be replicated by init_char */
void
do_start(struct char_data * ch)
{
  void advance_level(struct char_data * ch);

  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;

  ch->points.max_hit = 10;
  ch->points.max_mana = 50;
  ch->points.max_move = 50;

  advance_level(ch);
  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

  GET_HIT(ch)  = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 48;
  GET_COND(ch, FULL)   = 48;
  GET_COND(ch, DRUNK)  = 0;
  GET_COND(ch, TIRED)  = 48;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  SET_BIT(PRF_FLAGS(ch), PRF_DISPHP | PRF_DISPMANA |PRF_DISPMOVE |PRF_AUTOEXIT); 
  GET_WIMP_LEV(ch) = 5;
}

/* Some initializations for characters based on responses in creation */
void create_start(int lifestyle, struct char_data *ch)
{
  struct obj_data *letter = NULL;
  void Crash_crashsave(struct char_data * ch);
  void sql_rent_write(long id, ubyte code);

  switch(lifestyle) {
    case LIFE_POOR:
      letter = read_object(real_object(1247), REAL, NULL);
      break;
    case LIFE_AVERAGE:
      letter = read_object(real_object(1246), REAL, NULL);
     break;
    case LIFE_RICH:
      letter = read_object(real_object(1245), REAL, NULL);
      break;
    default:
      log("SYSERR: Unknown lifestyle type passed to create_start");
      break;
  }

  /* To stop letters from clogging up memory, which occurs when chars create but don't
  connect, we need to save the letter to the database and then extract it. It will load
  up normally when the player connects */
  if (letter) {
    /* Assign the letter to a character...stops them palming them off to make money */
    GET_OBJ_VAL(letter, 0) = GET_IDNUM(ch);

    obj_to_char(letter, ch);
    Crash_crashsave(ch);
    sql_rent_write(GET_IDNUM(ch), RENT_NEW);
    SET_BIT(GET_OBJ_EXTRA(letter), ITEM_UNLOAD);
    extract_obj(letter);
  }
}

/*
 * FIX FIX FIX
 * 
 */
void
advance_level(struct char_data * ch)
{
  extern sh_int group_mod[][3];
  int i, j, mod[3], hit, mana, move;

  for (i = 0; i < 3; i++)
    mod[i] = 0;

  /* First level = random number between CHA and appropriate abils */
  hit = number(GET_CHA(ch), (GET_STR(ch) + GET_CON(ch))/2);
  mana = number(GET_CHA(ch), (GET_WIS(ch) + GET_INT(ch))/2);
  move = number(GET_CHA(ch), (GET_DEX(ch) + GET_STR(ch))/2);

  /* Second level = modifier based on skill/spell groups known and their levels */
  for (i = 1; i <= MAX_SKILL_GROUPS; i++)
    for (j = 0; j < 3; j++)
      mod[j] += (GET_GROUP_LVL(ch, i) * group_mod[i][j]);

  /* Third level = random number between first calculation and second calculation */
  GET_MAX_HIT(ch) += number(hit, MAX(1, mod[0] * hit));
  GET_MAX_MANA(ch) += number(mana, MAX(1, mod[1] * mana));
  GET_MAX_MOVE(ch) += number(move, MAX(1, mod[2] * move));

  /* Each level milestone below, we want the ability to learn a new skill group */
  switch (GET_LEVEL(ch)) {
    case 1:
    case 10:
    case 20:
    case 30:
    case 40:
      GET_GROUP_PTS(ch)++;
      GET_STAT_PTS(ch)++;
      break;
    case 50:
      GET_GROUP_PTS(ch)++;    /* We'll give them a bonus for halfway */
    case 60:
    case 70:
    case 80:
    case 90:
    case 100:
      GET_GROUP_PTS(ch)++;
    default:
      GET_STAT_PTS(ch)++;
      break;
  }
  save_char(ch, NOWHERE);
}

/* Names of class/levels and exp required for each level */
/* FIX FIX FIX */
const char *titles = "the Player";



/*
   find the exp needed for the next level.  Uses the formula 
   next level's xp = 1000 * current level ^ (3/2).
*/

long 
find_exp(int level)
{
  if (level > LVL_IMPL || level < 0) {
    log("SYSERR: Requesting exp for invalid level %d!", level);
    return 0;
  }
 
  return (1000 * pow(level, 1.5));
}

long
min_exp(int level)
{
  long exp = 0;
  level--;

  if (level > LVL_IMPL || level < 0) {
    log("SYSERR: Requesting exp for invalid level %d!", level);
    return 0;
  }

  for (; level > 0; level--)
     exp += find_exp(level);

  return exp;
}

long
exp_needed_for_level(struct char_data *ch )
{
    return(min_exp(GET_LEVEL(ch)) + find_exp(GET_LEVEL(ch)));
}

long
find_skill_exp (int skill_level)
{
  return (10 * pow (pow ((skill_level + 1), 2), 1.5));
}


/* this function calculates the actual skill value of a character, taking
   into account ability score modifiers as well as any skill enhancers or
   penalties.
*/
int
calculate_skill_value(struct char_data *ch, int skill_num)
{
  int value = 0;
  
  if (skill_num > 23)  /* FIX.. stop the game from crashing until we unify skills */
    return 0;

  if (!IS_NPC(ch))
    value = GET_SKILL(ch, skill_num);  /* number of points they've bought */

  /* if (skill_num == SK_COMBAT)
    value += (GET_STR(ch) + GET_DEX(ch)) / 2;

  if (skill_num == SK_MAGIC)
    value += (GET_INT(ch) + GET_WIS(ch)) / 2; */
  
  /* if (skill_tree[skill_num].prereqnum)  / skill has a pre-req /
    value += calculate_skill_value(ch, skill_tree[skill_num].prereqnum) / 2; */


  /* +/- skill affs go here */
 
  return value;
}

/* THAC0 for characters.  (To Hit Armor Class 0) */
const int thaco[LVL_IMPL + 1] = {
  100, 20, 17, 15, 14, 13, 12, 12, 11, 10,
   10, 10,  9,  9,  9,  8,  8,  8,  7,  7,
    7,  7,  7,  6,  6,  6,  6,  6,  6,  5,
    5,  5,  5,  5,  5,  5,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0
};
/* magic.c --> saving throws */
