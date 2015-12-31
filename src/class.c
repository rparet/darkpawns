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

/* $Id: class.c 1487 2008-05-22 01:36:10Z jravn $ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */

#include <math.h>

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"

void  obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);

/* Names first */

const char *class_abbrevs[] = {
  "Mu",
  "Cl",
  "Th",
  "Wa",
  "Ma",
  "Av",
  "As",
  "Pa",
  "Ni",
  "Ps",
  "Ra",
  "My",
  "\n"
};


const char *pc_class_types[] = {
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "Magus",
  "Avatar",
  "Assassin",
  "Paladin",
  "Ninja",
  "Psionic",
  "Ranger",
  "Mystic",
  "\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
"\r\n"
"Select a class:\r\n"
"  [C]leric     - Healers and warriors of the gods\r\n"
"  [T]hief      - Stealthy, quick-fingered, lock-picking back-stabbers\r\n"
"  [W]arrior    - Fierce, battle-trained fighters\r\n"
"  [M]agic-user - Spell-casters trained in the art of magick\r\n"
"  Ps[i]onic    - Fighters endowed with the powers of the mind";

/* menu for selecting a hometown during char creation */
const char *hometown_menu =
"\r\n"
"Choose your home town:\r\n"
"  [K]ir Drax'in  - The Main City. New players should choose this.\r\n"
"  Kir-[O]shi     - The Port City.\r\n"
"  [A]laozar      - The Holy City.\r\n";


const char *human_class_menu =
"\r\n"
"Select a class:\r\n"
"  [C]leric     - Healers and warriors of the gods\r\n"
"  [T]hief      - Stealthy, quick-fingered, lock-picking back-stabbers\r\n"
"  [W]arrior    - Fierce, battle-trained fighters\r\n"
"  [M]agic-user - Spell-casters trained in the art of magick\r\n"
"  [N]inja      - Stealthy, magick-endowed warriors from the orient\r\n"
"  Ps[i]onic    - Fighters endowed with the powers of the mind";



/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int
parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg)
    {
    case 'm':
      return CLASS_MAGIC_USER;
      break;
    case 'c':
      return CLASS_CLERIC;
      break;
    case 'w':
      return CLASS_WARRIOR;
      break;
    case 't':
      return CLASS_THIEF;
      break;
    case 'a':
      return CLASS_MAGUS;
      break;
    case 'v':
      return CLASS_AVATAR;
      break;
    case 's':
      return CLASS_ASSASSIN;
      break;
    case 'p':
      return CLASS_PALADIN;
      break;
    case 'n':
      return CLASS_NINJA;
      break;
    case 'i':
      return CLASS_PSIONIC;
      break;
    case 'r':
      return CLASS_RANGER;
      break;
    case 'y':
      return CLASS_MYSTIC;
      break;
    default:
      return CLASS_UNDEFINED;
      break;
    }
}


/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long
find_class_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg)
    {
    case 'm':
      return (1 << 0);
      break;
    case 'c':
      return (1 << 1);
      break;
    case 't':
      return (1 << 2);
      break;
    case 'w':
      return (1 << 3);
      break;
    case 'a':
      return (1 << 4);/* magus */
      break;
    case 'v':
      return (1 << 5);/* avatar */
      break;
    case 's':
      return (1 << 6);/* assassin */
      break;
    case 'p':
      return (1 << 7);/* paladin */
      break;
    case 'n':
      return (1 << 8);/* ninja */
      break;
    case 'i':
      return (1 << 9);/* psionic */
      break;
    case 'r':
      return (1 << 10); /* ranger */
      break;
    case 'y':
      return (1 << 11); /* mystic */
      break;
    default:
      return 0;
      break;
    }
}


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

#define SPELL   0
#define SKILL   1
#define BOTH    2

/* #define LEARNED_LEVEL    0  % known which is considered "learned" */
/* #define MAX_PER_PRAC     1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC     2  min percent gain in skill per practice */
/* #define PRAC_TYPE        3  should it say 'spell' or 'skill'?    */

int prac_params[4][NUM_CLASSES] = {
/* MAG CLE   THE   WAR  MAGU  AVA   ASS    PAL  NIN    PSI RAN  MYS*/
{95,   95,   85,   80,   95,  95,   85,    80,  85,    95,  80,  95},/*learned level*/
{100,  100,  25,   25,   100, 100,  25,    25,  25,   100,  25, 100},/*max per prac*/
{25,   25,   0,    0,    25,   25,   0,    0,    0,    25,  0,   25},/*min per prac */
{SPELL,SPELL,SKILL,SKILL,SPELL,BOTH,SKILL, BOTH,BOTH, BOTH,SKILL,BOTH}/*prac name(s) */
};

/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 8014, only MAGIC_USERS are allowed
 * to go north.
 */
int guild_info[][3] = {

  {CLASS_MAGIC_USER,    8014,   SCMD_NORTH},

  {CLASS_THIEF,         8028,   SCMD_NORTH},

  {CLASS_CLERIC,        8027,   SCMD_SOUTH},

  {CLASS_WARRIOR,       8015,   SCMD_NORTH},

  {CLASS_PSIONIC,       8518,   SCMD_WEST },

  {CLASS_NINJA,         8525,   SCMD_SOUTH},

  /* this must go last -- add new guards above! */
  {-1,                    -1,           -1}
};


/* THAC0 for classes and levels.  (To Hit Armor Class 0) */

/* [class], [level] (all) */
const int thaco[NUM_CLASSES][LVL_IMPL + 1] = {

/* MAGE */
   { 100, 20, 20, 20, 19, 19, 19, 18, 18, 18,
      17, 17, 17, 16, 16, 16, 15, 15, 15, 14,
      14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
      11, 10, 10, 10, 9,   9,  9,  9,  9,  9, 9 },

/* CLERIC */
   { 100, 20, 20, 20, 18, 18, 18, 16, 16, 16,
      14, 14, 14, 12, 12, 12, 10, 10, 10, 8,
       8,  8,  6, 6 ,  6,  4,  4,  4,  2, 2,
       2,  1,  1,  1,  1,  1,  1, 1 , 1 , 1, 1 },

/* THIEF */
   { 100, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      16, 15, 15, 14, 13, 13, 12, 12, 11, 11,
      10, 10,  9,  9,  8, 8 ,  7,  7, 6 , 6,
       5,  5,  4,  4, 3 , 3 ,  3,  3,  3, 3, 3 },

/* WARRIOR */
   { 100, 20, 19, 18, 17, 16, 15, 14, 13, 12,
      11, 10, 9,  8,  7 , 6,    5, 4, 3,   2,
       1, 1,  1,  1, 1 ,  1,   1,  1, 1,   1,
       1, 1,  1,  1, 1,   1,   1,  1, 1,   1, 1 },

   /* MAGUS */
   { 100, 20, 20, 20, 19, 19, 19, 18, 18, 18,
      17, 17, 17, 16, 16, 16, 15, 15, 15, 14,
      14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
      11, 10, 10, 10, 9,   9,  9,  9, 9,  9 , 9 },

   /* AVATAR */
   { 100, 20, 20, 20, 18, 18, 18, 16, 16, 16,
      14, 14, 14, 12, 12, 12, 10, 10, 10, 8,
       8,  8,  6,  6, 6 ,  4, 4 ,  4,  2, 2,
       2, 1,   1, 1 , 1 , 1 , 1 , 1 , 1 , 1, 1 },

   /* ASSASSIN */
   { 100, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      16, 15, 15, 14, 13, 13, 12, 12, 11, 11,
      10, 10, 9,   9, 8 ,  8,  7, 7 , 6 ,  6,
       5,  5, 4,  4,  3,   3 , 3, 3,  3,   3, 3 },

   /* PALADIN */
   { 100, 20, 19, 18, 17, 16, 15 , 14, 13, 12,
      11, 10,  9,  8,  7,  6,  5 ,  4,  3,  2,
       1,  1,  1,  1,  1,  1,  1 ,  1,  1,  1,
       1,  1,  1,  1,  1,  1,  1 ,  1,  1,  1, 1 },

   /* NINJA */
   { 100, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      16, 15, 15, 14, 13, 13, 12, 12, 11, 11,
      10, 10,  9,  9,  8, 8 ,  7,  7, 6 , 6,
       5,  5,  4,  4, 3 , 3 ,  3,  3,  3, 3, 3 },

   /* PSIONIC */
   { 100, 20, 20, 19, 18, 18, 17, 16, 16, 16,
      15, 15, 14, 14, 14, 13, 12, 12, 10, 10,
       9, 9,   8,  8,  7, 7 , 6 ,  5,  5, 4,
       4, 3,   3,  3,  2,  2, 1,   1,  1, 1, 1 },

   /* Ranger */
   { 100, 20, 19, 18, 17, 16, 15 , 14, 13, 12,
      11, 10,  9,  8,  7,  6,  5 ,  4,  3,  2,
       1,  1,  1,  1,  1,  1,  1 ,  1,  1,  1,
       1,  1,  1,  1,  1,  1,  1 ,  1,  1,  1, 1 },

   /* Mystic */
   { 100, 20, 20, 20, 19, 19, 19, 18, 18, 18,
      17, 17, 17, 16, 16, 16, 15, 15, 15, 14,
      14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
      11, 10, 10, 10, 9,   9,  9,  9, 9,  9 , 9 }

};


/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 */
void
roll_real_abils(struct char_data * ch)
{
  int i, j, k, temp;
  ubyte table[6];
  ubyte rolls[4];

  for (i = 0; i < 6; i++)
    table[i] = 0;

  for (i = 0; i < 6; i++)
    {
      for (j = 0; j < 4; j++)
    rolls[j] = number(1, 6);

      temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
    MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

      for (k = 0; k < 6; k++)
    if (table[k] < temp)
      {
        temp ^= table[k];
        table[k] ^= temp;
        temp ^= table[k];
      }
    }

  ch->real_abils.str_add = 0;
  ch->real_abils.str = 0;
  ch->real_abils.intel = 0;
  ch->real_abils.wis = 0;
  ch->real_abils.dex = 0;
  ch->real_abils.con = 0;
  ch->real_abils.cha = 0;

  switch (GET_CLASS(ch))
    {
    case CLASS_MAGIC_USER:
    case CLASS_MAGUS:
    case CLASS_PSIONIC:
    case CLASS_MYSTIC:
      ch->real_abils.intel = table[0];
      ch->real_abils.wis = table[1];
      ch->real_abils.dex = table[2];
      ch->real_abils.str = table[3];
      ch->real_abils.con = table[4];
      ch->real_abils.cha = table[5];
      break;
    case CLASS_CLERIC:
    case CLASS_AVATAR:
      ch->real_abils.wis = table[0];
      ch->real_abils.intel = table[1];
      ch->real_abils.str = table[2];
      ch->real_abils.dex = table[3];
      ch->real_abils.con = table[4];
      ch->real_abils.cha = table[5];
      break;
    case CLASS_THIEF:
    case CLASS_ASSASSIN:
    case CLASS_NINJA:
      ch->real_abils.dex = table[0];
      ch->real_abils.str = table[1];
      ch->real_abils.con = table[2];
      ch->real_abils.intel = table[3];
      ch->real_abils.wis = table[4];
      ch->real_abils.cha = table[5];
      break;
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
    case CLASS_RANGER:
      ch->real_abils.str = table[0];
      ch->real_abils.dex = table[1];
      ch->real_abils.con = table[2];
      ch->real_abils.wis = table[3];
      ch->real_abils.intel = table[4];
      ch->real_abils.cha = table[5];
      if (ch->real_abils.str == 18)
    ch->real_abils.str_add = number(0, 100);
      break;
    }
  switch (GET_RACE(ch))
  {
  case RACE_HUMAN:
    ch->real_abils.cha = MIN(ch->real_abils.cha+1, 18);
    break;
  case RACE_ELF:
    ch->real_abils.intel = MIN(ch->real_abils.intel+1, 18);
    ch->real_abils.str = MIN(ch->real_abils.str, 18);
    if (ch->real_abils.str==18)
      ch->real_abils.str_add = 0;
    break;
  case RACE_DWARF:
    ch->real_abils.wis = MIN(ch->real_abils.wis+1, 18);
    break;
  case RACE_KENDER:
    ch->real_abils.dex = MIN(ch->real_abils.dex+1, 18);
    ch->real_abils.str = MIN(ch->real_abils.str, 18);
    if (ch->real_abils.str==18)
      ch->real_abils.str_add = 0;
    break;
  case RACE_MINOTAUR:
    ch->real_abils.str = MIN(ch->real_abils.str+1, 18);
        if (ch->real_abils.str == 18 && (GET_CLASS(ch) == CLASS_WARRIOR))
        ch->real_abils.str_add = number(0, 100);
    break;
  case RACE_RAKSHASA:
    ch->real_abils.str = MIN(ch->real_abils.str+1, 18);
        if (ch->real_abils.str == 18  && (GET_CLASS(ch) == CLASS_WARRIOR))
        ch->real_abils.str_add = number(0, 100);
    break;
  case RACE_SSAUR:
    ch->real_abils.con = MIN(ch->real_abils.con+1, 18);
    ch->real_abils.wis = MIN(ch->real_abils.wis, 16);
    break;
  default: break;
  }
  ch->aff_abils = ch->real_abils;
}


/* Some initializations for characters, including initial skills */
void
do_start(struct char_data * ch)
{
  void advance_level(struct char_data * ch);
  void obj_to_char(struct obj_data *object, struct char_data *ch);

  struct obj_data *pack = read_object(real_object(8038), REAL);

  switch ( GET_CLASS(ch) )
    {
    case CLASS_THIEF :
      obj_to_obj(read_object(real_object(8027), REAL), pack);   /* lock picks */
      obj_to_char(read_object(real_object(8036), REAL), ch);     /* dagger */
      break;
    case CLASS_MAGIC_USER :
      obj_to_char(read_object(real_object(8036), REAL), ch);     /* dagger */
      obj_to_char(read_object(real_object(1239), REAL), ch);     /* obsidian */
      obj_to_char(read_object(real_object(1239), REAL), ch);     /* obsidian */
      break;
    case CLASS_NINJA:
      obj_to_char(read_object(real_object(8036), REAL), ch);     /* dagger */
      break;
    case CLASS_WARRIOR :
    case CLASS_PSIONIC:
      obj_to_char(read_object(real_object(8037), REAL), ch);     /* sm swd */
      break;
    default :
      obj_to_char(read_object(real_object(8023), REAL), ch);     /* club   */
    }

  obj_to_char(read_object(real_object(8019), REAL), ch);        /* tunic  */
  obj_to_obj(read_object(real_object(8010), REAL), pack);       /* bread */
  obj_to_obj(read_object(real_object(8063), REAL), pack);       /* water skin */
  obj_to_char(pack, ch);

  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;

  /* roll_real_abils used to be here, but now its in nanny() */
  ch->points.max_hit = 10;
  ch->points.max_mana = 100;

  switch (GET_CLASS(ch))
    {
    case CLASS_MAGIC_USER:
    case CLASS_MAGUS:
    case CLASS_CLERIC:
    case CLASS_AVATAR:
    case CLASS_WARRIOR:
    case CLASS_PALADIN:
    case CLASS_NINJA:
    case CLASS_PSIONIC:
    case CLASS_RANGER:
    case CLASS_MYSTIC:
      break;

    case CLASS_THIEF:
    case CLASS_ASSASSIN:
      SET_SKILL(ch, SKILL_SNEAK, 10);
      SET_SKILL(ch, SKILL_HIDE, 5);
      SET_SKILL(ch, SKILL_PEEK, 15);
      SET_SKILL(ch, SKILL_STEAL, 15);
      SET_SKILL(ch, SKILL_BACKSTAB, 10);
      SET_SKILL(ch, SKILL_PICK_LOCK, 10);
      break;
    }

  if (GET_RACE(ch) == RACE_KENDER)
    SET_SKILL(ch, SKILL_STEAL, 25);
  if (GET_RACE(ch) == RACE_MINOTAUR)
    SET_SKILL(ch, SKILL_HEADBUTT, 25);

  advance_level(ch);

  GET_HIT(ch)  = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 36;
  GET_COND(ch, FULL)   = 36;
  GET_COND(ch, DRUNK)  = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
  SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
  SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
  GET_WIMP_LEV(ch) = 5;
  SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOEXIT);
  GET_PRACTICES(ch)+=2;
  GET_ORIG_CON(ch) = GET_CON(ch);
}


/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void
advance_level(struct char_data * ch)
{
  int add_hp = 0, add_mana = 0, add_move = 0, i;

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

  add_hp = con_app[GET_CON(ch)].hitp;

  switch (GET_CLASS(ch))
    {
    case CLASS_MAGIC_USER:
      add_hp += number(4,8);
      add_mana = number(GET_LEVEL(ch), (int) (3 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 10);
      add_move = number(1, 3);
      GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
      break;

    case CLASS_MAGUS :
      add_hp += number(5,9);
      add_mana = number(GET_LEVEL(ch), (int) (3 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 10);
      add_move = number(1, 3);
      GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
      break;

    case CLASS_CLERIC:
      add_hp += number(5, 9);
      add_mana = number(GET_LEVEL(ch), (int) (3 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 10);
      add_move = number(1, 3);
      GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
      break;

    case CLASS_AVATAR:
      add_hp += number(6, 11);
      add_mana = number(GET_LEVEL(ch), (int) (3 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 10);
      add_move = number(1, 3);
      GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
      break;

    case CLASS_ASSASSIN :
      add_hp += number(8, 14);
      add_mana = number(GET_LEVEL(ch), (int)(2 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 5);
      add_move = number(1, 4);
      GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));
      break;

    case CLASS_THIEF:
      add_hp += number(7, 13);
      add_move = number(1, 4);
      GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));
      break;

    case CLASS_PALADIN :
      add_mana = number(GET_LEVEL(ch), (int)(2 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 5);
      add_hp += number(12, 16);
      add_move = number(1, 4);
      GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));
      break;
    case CLASS_RANGER:
      add_hp += number(13, 16);
      add_move = number (2, 4);
      GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));
      break;
    case CLASS_WARRIOR:
      add_hp += number(11, 14);
      add_move = number(1, 4);
      GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));
      break;

    case CLASS_NINJA :
      add_hp += number(8, 13);
      add_mana = number(GET_LEVEL(ch), (int)(2 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 10);
      add_move = number(1, 4);
      GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));
      break;

    case CLASS_PSIONIC:
      add_hp += number(4,8);
      add_mana = number(GET_LEVEL(ch), (int)(2 * GET_LEVEL(ch)));
      add_mana = MIN(add_mana, 10);
      add_move = number(1, 4);
      GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
      break;
   case CLASS_MYSTIC:
     add_hp += number(5, 9);
     add_mana = number(GET_LEVEL(ch), (int)(2 * GET_LEVEL(ch)));
     add_mana = MIN(add_mana, 10);
     add_move = number(1, 4);
     GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
     break;
    }

  ch->points.max_hit  += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);

  if (GET_LEVEL(ch) > 1)
    ch->points.max_mana += add_mana;

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    {
      for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (char) -1;
      SET_BIT_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT);
    }

  save_char(ch, NOWHERE);

  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}


int
backstab_mult(int level)
{
  if (level <= 0)
    return 1;     /* level 0 */
  if (level < LVL_IMMORT)
    return ( (level*.2)+1 );
  else
    return 20;    /* immortals */
}


/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int
invalid_class(struct char_data *ch, struct obj_data *obj)
{
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && IS_MAGIC_USER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_PSIONIC) && IS_PSIONIC(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NINJA) && IS_NINJA(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_PALADIN) && IS_PALADIN(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MAGUS) && IS_MAGUS(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_AVATAR) && IS_AVATAR(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ASSASSIN) && IS_ASSASSIN(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_RANGER) && IS_RANGER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MYSTIC) && IS_MYSTIC(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch)))
    return TRUE;
  else if (CAN_WEAR(obj,ITEM_WEAR_WIELD) &&
       GET_OBJ_VAL(obj, 3) == TYPE_SLASH - TYPE_HIT && IS_CLERIC(ch))
    return TRUE;
  else if (CAN_WEAR(obj, ITEM_WEAR_SHIELD) &&
           (IS_THIEF(ch) || IS_ASSASSIN(ch) || IS_NINJA(ch)))
    return TRUE;
  else
    return FALSE;
}


/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */
void
init_spell_levels(void)
{
  /* MAGES */
  spell_level(SPELL_MAGIC_MISSILE, CLASS_MAGIC_USER, 1);
  spell_level(SPELL_INFRAVISION, CLASS_MAGIC_USER, 1);
  spell_level(SPELL_ACID_BLAST, CLASS_MAGIC_USER, 2);
  spell_level(SPELL_DETECT_INVIS, CLASS_MAGIC_USER, 2);
  spell_level(SPELL_DETECT_MAGIC, CLASS_MAGIC_USER, 2);
  spell_level(SPELL_CHILL_TOUCH, CLASS_MAGIC_USER, 3);
  spell_level(SPELL_INVISIBLE, CLASS_MAGIC_USER, 4);
  spell_level(SPELL_BURNING_HANDS, CLASS_MAGIC_USER, 5);
  spell_level(SPELL_STRENGTH, CLASS_MAGIC_USER, 6);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_MAGIC_USER, 7);
  spell_level(SPELL_SLEEP, CLASS_MAGIC_USER, 8);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_MAGIC_USER, 9);
  spell_level(SPELL_BLINDNESS, CLASS_MAGIC_USER, 9);
  spell_level(SPELL_DETECT_POISON, CLASS_MAGIC_USER, 10);
  spell_level(SPELL_COLOR_SPRAY, CLASS_MAGIC_USER, 11);
  spell_level(SPELL_WATERWALK, CLASS_MAGIC_USER, 12);
  spell_level(SPELL_SENSE_LIFE, CLASS_MAGIC_USER, 12);
  spell_level(SPELL_ENERGY_DRAIN, CLASS_MAGIC_USER, 13);
  spell_level(SPELL_CURSE, CLASS_MAGIC_USER, 14);
  spell_level(SPELL_FIREBALL, CLASS_MAGIC_USER, 15);
  spell_level(SPELL_HELLFIRE, CLASS_MAGIC_USER, 20);
  spell_level(SPELL_METALSKIN, CLASS_MAGIC_USER, 21);
  spell_level(SPELL_WATER_BREATHE, CLASS_MAGIC_USER, 22);
  spell_level(SPELL_ENCHANT_ARMOR, CLASS_MAGIC_USER, 24);
  spell_level(SPELL_DISINTEGRATE, CLASS_MAGIC_USER, 25);
  spell_level(SPELL_ENCHANT_WEAPON, CLASS_MAGIC_USER, 26);
  spell_level(SPELL_INVULNERABILITY, CLASS_MAGIC_USER, 28);

  /* CLERICS */
  spell_level(SKILL_TURN, CLASS_CLERIC, 1);
  spell_level(SPELL_CURE_LIGHT, CLASS_CLERIC, 1);
  spell_level(SPELL_CREATE_FOOD, CLASS_CLERIC, 2);
  spell_level(SPELL_CREATE_WATER, CLASS_CLERIC, 2);
  spell_level(SPELL_DETECT_POISON, CLASS_CLERIC, 3);
  spell_level(SPELL_DETECT_ALIGN, CLASS_CLERIC, 4);
  spell_level(SPELL_CURE_BLIND, CLASS_CLERIC, 4);
  spell_level(SPELL_BLESS, CLASS_CLERIC, 5);
  spell_level(SPELL_ARMOR, CLASS_CLERIC, 6);
  spell_level(SPELL_BLINDNESS, CLASS_CLERIC, 6);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_CLERIC, 8);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_CLERIC, 8);
  spell_level(SPELL_CURE_CRITIC, CLASS_CLERIC, 9);
  spell_level(SPELL_COC, CLASS_CLERIC, 10);
  spell_level(SPELL_SUMMON, CLASS_CLERIC, 10);
  spell_level(SPELL_REMOVE_POISON, CLASS_CLERIC, 10);
  spell_level(SPELL_HOLY_SHIELD, CLASS_CLERIC, 11);
  spell_level(SPELL_WORD_OF_RECALL, CLASS_CLERIC, 12);
  spell_level(SPELL_POISON, CLASS_CLERIC, 13);
  spell_level(SPELL_DISPEL_EVIL, CLASS_CLERIC, 14);
  spell_level(SPELL_DISPEL_GOOD, CLASS_CLERIC, 14);
  spell_level(SPELL_SANCTUARY, CLASS_CLERIC, 15);
  spell_level(SPELL_HEAL, CLASS_CLERIC, 16);
  spell_level(SPELL_GROUP_HEAL, CLASS_CLERIC, 22);
  spell_level(SPELL_MASS_HEAL, CLASS_CLERIC, 23);
  spell_level(SPELL_INVIGORATE, CLASS_CLERIC, 25);
  spell_level(SPELL_REMOVE_CURSE, CLASS_CLERIC, 26);
  spell_level(SPELL_VITALITY, CLASS_CLERIC, 27);
  spell_level(SPELL_GROUP_RECALL, CLASS_CLERIC, 29);

  /* THIEVES */
  spell_level(SKILL_PEEK, CLASS_THIEF, 1);
  spell_level(SKILL_BACKSTAB, CLASS_THIEF, 1);
  spell_level(SKILL_COMPARE, CLASS_THIEF, 1);
  spell_level(SKILL_SNEAK, CLASS_THIEF, 2);
  spell_level(SKILL_STEAL, CLASS_THIEF, 3);
  spell_level(SKILL_PALM, CLASS_THIEF, 3);
  spell_level(SKILL_PICK_LOCK, CLASS_THIEF, 4);
  spell_level(SKILL_HIDE, CLASS_THIEF, 5);
  spell_level(SKILL_APPRAISE, CLASS_THIEF, 7);
  spell_level(SKILL_DETECT, CLASS_THIEF, 8);
  spell_level(SKILL_TRIP, CLASS_THIEF, 9);
  spell_level(SKILL_SUBDUE, CLASS_THIEF, 11);
  spell_level(SKILL_CIRCLE, CLASS_THIEF, 15);
  spell_level(SKILL_GROINRIP, CLASS_THIEF, 18);
  spell_level(SKILL_CUTTHROAT, CLASS_THIEF, 20);
  spell_level(SKILL_DISEMBOWEL, CLASS_THIEF, 27);

  /* WARRIORS */
  spell_level(SKILL_KICK, CLASS_WARRIOR, 1);
  spell_level(SKILL_BASH, CLASS_WARRIOR, 3);
  spell_level(SKILL_RESCUE, CLASS_WARRIOR, 4);
  spell_level(SKILL_RETREAT, CLASS_WARRIOR, 5);
  spell_level(SKILL_BEARHUG, CLASS_WARRIOR, 7);
  spell_level(SKILL_BERSERK, CLASS_WARRIOR, 8);
  spell_level(SKILL_TRACK, CLASS_WARRIOR, 9);
  spell_level(SKILL_SLEEPER, CLASS_WARRIOR, 12);
  spell_level(SKILL_PARRY, CLASS_WARRIOR, 13);
  spell_level(SKILL_HEADBUTT, CLASS_WARRIOR, 15);
  spell_level(SKILL_SLUG, CLASS_WARRIOR, 17);
  spell_level(SKILL_SMACKHEADS, CLASS_WARRIOR, 20);
  spell_level(SKILL_CHARGE, CLASS_WARRIOR, 23);


  /* PSIONICS */
  spell_level(SPELL_MINDPOKE, CLASS_PSIONIC, 1);
  spell_level(SKILL_FLESH_ALTER, CLASS_PSIONIC, 1);
  spell_level(SPELL_PSYSHIELD, CLASS_PSIONIC, 2);
  spell_level(SPELL_LESSPERCEPT, CLASS_PSIONIC, 3);
  spell_level(SPELL_MINDSIGHT, CLASS_PSIONIC, 4);
  spell_level(SPELL_MINDATTACK, CLASS_PSIONIC, 5);
  spell_level(SPELL_CHAMELEON, CLASS_PSIONIC, 6);
  spell_level(SPELL_ADRENALINE, CLASS_PSIONIC, 7);
  spell_level(SPELL_LEVITATE, CLASS_PSIONIC, 8);
  spell_level(SPELL_MINDBLAST, CLASS_PSIONIC, 9);
  spell_level(SPELL_CHANGE_DENSITY, CLASS_PSIONIC, 11);
  spell_level(SPELL_DREAM_TRAVEL, CLASS_PSIONIC, 13);
  spell_level(SPELL_TELEPORT, CLASS_PSIONIC, 15);
  spell_level(SPELL_GREATPERCEPT, CLASS_PSIONIC, 18);
  spell_level(SPELL_DOMINATE, CLASS_PSIONIC, 20);
  spell_level(SPELL_MIRROR_IMAGE, CLASS_PSIONIC, 22);
  spell_level(SPELL_CELL_ADJUSTMENT, CLASS_PSIONIC, 23);
  spell_level(SPELL_MENTAL_LAPSE, CLASS_PSIONIC, 25);
  spell_level(SPELL_PSIBLAST, CLASS_PSIONIC, 26);
  spell_level(SPELL_MIND_BAR, CLASS_PSIONIC, 28);

  /* NINJA */
  spell_level(SKILL_STRIKE, CLASS_NINJA, 1);
  spell_level(SKILL_KK_KYO, CLASS_NINJA, 2);
  spell_level(SKILL_STEALTH, CLASS_NINJA, 3);
  spell_level(SKILL_SERPENT_KICK, CLASS_NINJA, 4);
  spell_level(SKILL_ESCAPE, CLASS_NINJA, 5);
  spell_level(SKILL_KK_SHA, CLASS_NINJA, 5);
  spell_level(SKILL_KABUKI, CLASS_NINJA, 7);
  spell_level(SKILL_KK_ZAI, CLASS_NINJA, 8);
  spell_level(SKILL_KK_KAI, CLASS_NINJA, 9);
  spell_level(SKILL_TIGER_PUNCH, CLASS_NINJA, 11);
  spell_level(SKILL_KK_RETSU, CLASS_NINJA, 12);
  spell_level(SKILL_SUBDUE, CLASS_NINJA, 13);
  spell_level(SKILL_KK_TOH, CLASS_NINJA, 15);
  spell_level(SKILL_EVASION, CLASS_NINJA, 17);
  spell_level(SKILL_KK_RIN, CLASS_NINJA, 18);
  spell_level(SKILL_DRAGON_KICK, CLASS_NINJA, 20);
  spell_level(SKILL_KK_JIN, CLASS_NINJA, 22);
  spell_level(SKILL_KK_ZHEN, CLASS_NINJA, 25);
  spell_level(SKILL_CUTTHROAT, CLASS_NINJA, 26);
  spell_level(SKILL_NECKBREAK, CLASS_NINJA, 28);
  spell_level(SPELL_SMOKESCREEN, CLASS_NINJA, 29);
  spell_level(SPELL_SOUL_LEECH, CLASS_NINJA, 30);

  /* MAGUS */
  spell_level(SPELL_MAGIC_MISSILE, CLASS_MAGUS, 1);
  spell_level(SPELL_INFRAVISION, CLASS_MAGUS, 1);
  spell_level(SPELL_ACID_BLAST, CLASS_MAGUS, 2);
  spell_level(SPELL_DETECT_INVIS, CLASS_MAGUS, 2);
  spell_level(SPELL_DETECT_MAGIC, CLASS_MAGUS, 2);
  spell_level(SPELL_CHILL_TOUCH, CLASS_MAGUS, 3);
  spell_level(SPELL_INVISIBLE, CLASS_MAGUS, 4);
  spell_level(SPELL_BURNING_HANDS, CLASS_MAGUS, 5);
  spell_level(SPELL_STRENGTH, CLASS_MAGUS, 6);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_MAGUS, 7);
  spell_level(SPELL_SLEEP, CLASS_MAGUS, 8);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_MAGUS, 9);
  spell_level(SPELL_BLINDNESS, CLASS_MAGUS, 9);
  spell_level(SPELL_DETECT_POISON, CLASS_MAGUS, 10);
  spell_level(SPELL_COLOR_SPRAY, CLASS_MAGUS, 11);
  spell_level(SPELL_SENSE_LIFE, CLASS_MAGUS, 12);
  spell_level(SPELL_WATERWALK, CLASS_MAGUS, 12);
  spell_level(SPELL_ENERGY_DRAIN, CLASS_MAGUS, 13);
  spell_level(SPELL_CURSE, CLASS_MAGUS, 14);
  spell_level(SPELL_FIREBALL, CLASS_MAGUS, 15);
  spell_level(SPELL_GATE, CLASS_MAGUS, 20);
  spell_level(SPELL_HELLFIRE, CLASS_MAGUS, 20);
  spell_level(SPELL_METALSKIN, CLASS_MAGUS, 21);
  spell_level(SPELL_FLY, CLASS_MAGUS, 22);
  spell_level(SPELL_WATER_BREATHE, CLASS_MAGUS, 22);
  spell_level(SPELL_GROUP_INVIS, CLASS_MAGUS, 23);
  spell_level(SPELL_ENCHANT_ARMOR, CLASS_MAGUS, 24);
  spell_level(SPELL_FLAMESTRIKE, CLASS_MAGUS, 23);
  spell_level(SPELL_DISINTEGRATE, CLASS_MAGUS, 25);
  spell_level(SPELL_ENCHANT_WEAPON, CLASS_MAGUS, 26);
  spell_level(SPELL_DISRUPT, CLASS_MAGUS, 27);
  spell_level(SPELL_INVULNERABILITY, CLASS_MAGUS, 28);
  spell_level(SPELL_CONJURE_ELEMENTAL, CLASS_MAGUS, 29);
  spell_level(SPELL_METEOR_SWARM, CLASS_MAGUS, 30);

  /* AVATARS */
  spell_level(SKILL_TURN, CLASS_AVATAR, 1);
  spell_level(SPELL_CURE_LIGHT, CLASS_AVATAR, 1);
  spell_level(SPELL_CREATE_FOOD, CLASS_AVATAR, 2);
  spell_level(SPELL_CREATE_WATER, CLASS_AVATAR, 2);
  spell_level(SPELL_DETECT_POISON, CLASS_AVATAR, 3);
  spell_level(SPELL_DETECT_ALIGN, CLASS_AVATAR, 4);
  spell_level(SPELL_CURE_BLIND, CLASS_AVATAR, 4);
  spell_level(SPELL_BLESS, CLASS_AVATAR, 5);
  spell_level(SPELL_ARMOR, CLASS_AVATAR, 5);
  spell_level(SPELL_BLINDNESS, CLASS_AVATAR, 6);
  spell_level(SPELL_POISON, CLASS_AVATAR, 7);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_AVATAR, 8);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_AVATAR, 8);
  spell_level(SPELL_CURE_CRITIC, CLASS_AVATAR, 9);
  spell_level(SPELL_COC, CLASS_AVATAR, 10);
  spell_level(SPELL_SUMMON, CLASS_AVATAR, 10);
  spell_level(SPELL_REMOVE_POISON, CLASS_AVATAR, 10);
  spell_level(SPELL_HOLY_SHIELD, CLASS_AVATAR,11);
  spell_level(SPELL_WORD_OF_RECALL, CLASS_AVATAR, 12);
  spell_level(SPELL_INTELLECT, CLASS_AVATAR, 13);
  spell_level(SPELL_DISPEL_EVIL, CLASS_AVATAR, 14);
  spell_level(SPELL_DISPEL_GOOD, CLASS_AVATAR, 14);
  spell_level(SPELL_SANCTUARY, CLASS_AVATAR, 15);
  spell_level(SPELL_HEAL, CLASS_AVATAR, 16);
  spell_level(SPELL_GROUP_HEAL, CLASS_AVATAR, 22);
  spell_level(SPELL_MASS_HEAL, CLASS_AVATAR, 23);
  spell_level(SPELL_INVIGORATE, CLASS_AVATAR, 25);
  spell_level(SPELL_REMOVE_CURSE, CLASS_AVATAR, 26);
  spell_level(SPELL_VITALITY, CLASS_AVATAR, 27);
  spell_level(SPELL_GROUP_RECALL, CLASS_AVATAR, 29);
  spell_level(SPELL_DIVINE_INT, CLASS_AVATAR, 30);

  /* ASSASSINS */
  spell_level(SKILL_PEEK, CLASS_ASSASSIN, 1);
  spell_level(SKILL_BACKSTAB, CLASS_ASSASSIN, 1);
  spell_level(SKILL_COMPARE, CLASS_ASSASSIN, 1);
  spell_level(SPELL_BLINDNESS, CLASS_ASSASSIN, 2);
  spell_level(SKILL_SNEAK, CLASS_ASSASSIN, 2);
  spell_level(SKILL_STEAL, CLASS_ASSASSIN, 3);
  spell_level(SKILL_PALM, CLASS_ASSASSIN, 3);
  spell_level(SKILL_PICK_LOCK, CLASS_ASSASSIN, 4);
  spell_level(SKILL_SHARPEN, CLASS_ASSASSIN, 4);
  spell_level(SKILL_HIDE, CLASS_ASSASSIN, 5);
  spell_level(SKILL_DETECT, CLASS_ASSASSIN, 5);
  spell_level(SKILL_APPRAISE, CLASS_ASSASSIN, 7);
  spell_level(SKILL_CIRCLE, CLASS_ASSASSIN, 8);
  spell_level(SKILL_TRIP, CLASS_ASSASSIN, 9);
  spell_level(SKILL_GROINRIP, CLASS_ASSASSIN, 10);
  spell_level(SKILL_SHADOW, CLASS_ASSASSIN, 12);
  spell_level(SKILL_SUBDUE, CLASS_ASSASSIN, 15);
  spell_level(SPELL_SMOKESCREEN, CLASS_ASSASSIN, 16);
  spell_level(SKILL_EVASION, CLASS_ASSASSIN, 17);
  spell_level(SKILL_CUTTHROAT, CLASS_ASSASSIN, 20);
  spell_level(SKILL_DISEMBOWEL, CLASS_ASSASSIN, 25);


  /* PALADINS */
  spell_level(SKILL_KICK, CLASS_PALADIN, 1);
  spell_level(SKILL_BASH, CLASS_PALADIN, 3);
  spell_level(SKILL_RESCUE, CLASS_PALADIN, 3);
  spell_level(SKILL_SHARPEN, CLASS_PALADIN, 4);
  spell_level(SPELL_DETECT_ALIGN, CLASS_PALADIN, 5);
  spell_level(SKILL_RETREAT, CLASS_PALADIN, 5);
  spell_level(SKILL_BEARHUG, CLASS_PALADIN, 7);
  spell_level(SKILL_BERSERK, CLASS_PALADIN, 8);
  spell_level(SKILL_TRACK, CLASS_PALADIN, 9);
  spell_level(SPELL_LAY_HANDS, CLASS_PALADIN, 10);
  spell_level(SKILL_SLEEPER, CLASS_PALADIN, 12);
  spell_level(SKILL_PARRY, CLASS_PALADIN, 13);
  spell_level(SKILL_HEADBUTT, CLASS_PALADIN, 15);
  spell_level(SPELL_DISPEL_EVIL, CLASS_PALADIN, 14);
  spell_level(SPELL_DISPEL_GOOD, CLASS_PALADIN, 14);
  spell_level(SKILL_SLUG, CLASS_PALADIN, 17);
  spell_level(SKILL_SMACKHEADS, CLASS_PALADIN, 20);
  spell_level(SKILL_CHARGE, CLASS_PALADIN, 23);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_PALADIN, 25);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_PALADIN, 25);
  spell_level(SKILL_DISARM, CLASS_PALADIN, 30);

/* MYSTICS */

  spell_level(SPELL_MINDPOKE, CLASS_MYSTIC, 1);
  spell_level(SKILL_FLESH_ALTER, CLASS_MYSTIC, 1);
  spell_level(SPELL_PSYSHIELD, CLASS_MYSTIC, 2);
  spell_level(SPELL_LESSPERCEPT, CLASS_MYSTIC, 3);
  spell_level(SPELL_MINDSIGHT, CLASS_MYSTIC, 4);
  spell_level(SPELL_MINDATTACK, CLASS_MYSTIC, 5);
  spell_level(SPELL_CHAMELEON, CLASS_MYSTIC, 6);
  spell_level(SPELL_ADRENALINE, CLASS_MYSTIC, 7);
  spell_level(SPELL_LEVITATE, CLASS_MYSTIC, 8);
  spell_level(SPELL_MINDBLAST, CLASS_MYSTIC, 9);
  spell_level(SPELL_CHANGE_DENSITY, CLASS_MYSTIC, 11);
  spell_level(SPELL_DREAM_TRAVEL, CLASS_MYSTIC, 13);
  spell_level(SPELL_TELEPORT, CLASS_MYSTIC, 15);
  spell_level(SPELL_GREATPERCEPT, CLASS_MYSTIC, 16);
  spell_level(SPELL_DOMINATE, CLASS_MYSTIC, 19);
  spell_level(SPELL_CELL_ADJUSTMENT, CLASS_MYSTIC, 21);
  spell_level(SPELL_MIRROR_IMAGE, CLASS_MYSTIC, 22);
  spell_level(SPELL_MENTAL_LAPSE, CLASS_MYSTIC, 24);
  spell_level(SPELL_PSIBLAST, CLASS_MYSTIC, 25);
  spell_level(SPELL_MIND_BAR, CLASS_MYSTIC, 26);
  spell_level(SPELL_MASS_DOMINATE, CLASS_MYSTIC, 28);
  spell_level(SPELL_HASTE, CLASS_MYSTIC, 30);

  /* RANGERS */

  spell_level(SKILL_KICK, CLASS_RANGER, 1);
  spell_level(SKILL_SCROUNGE, CLASS_RANGER, 2);
  spell_level(SKILL_BASH, CLASS_RANGER, 3);
  spell_level(SKILL_SHARPEN, CLASS_RANGER, 4);
  spell_level(SKILL_RESCUE, CLASS_RANGER, 5);
  spell_level(SKILL_RETREAT, CLASS_RANGER, 6);
  spell_level(SKILL_FIRST_AID, CLASS_RANGER, 6);
  spell_level(SKILL_BEARHUG, CLASS_RANGER, 7);
  spell_level(SKILL_DETECT, CLASS_RANGER, 8);
  spell_level(SKILL_TRACK, CLASS_RANGER, 9);
  spell_level(SKILL_HIDE, CLASS_RANGER, 10);
  spell_level(SPELL_SENSE_LIFE, CLASS_RANGER, 11);
  spell_level(SKILL_SLEEPER, CLASS_RANGER, 12);
  spell_level(SKILL_PARRY, CLASS_RANGER, 13);
  spell_level(SKILL_HEADBUTT, CLASS_RANGER, 15);
  spell_level(SKILL_SLUG, CLASS_RANGER, 17);
  spell_level(SKILL_SCOUT, CLASS_RANGER, 19);
  spell_level(SKILL_CHARGE, CLASS_RANGER, 22);
  spell_level(SKILL_AMBUSH, CLASS_RANGER, 23);
  spell_level(SKILL_SHOOT, CLASS_RANGER, 26);
  spell_level(SKILL_DISARM, CLASS_RANGER, 30);

}


/* Names of class/levels and exp required for each level */

const char *titles[NUM_CLASSES] =
{
  "the Mage", "the Cleric", "the Thief", "the Warrior", "the Magus",
  "the Avatar", "the Assassin", "the Paladin", "the Ninja", "the Psionic",
  "the Ranger", "the Mystic"
};

int
find_exp(int class, int level)
{
  double modifier = 1;

  switch(class)
    {
    case CLASS_MAGIC_USER:
      modifier = .3;
      break;
    case CLASS_CLERIC:
      modifier = .4;
      break;
    case CLASS_WARRIOR:
      modifier = .7;
      break;
    case CLASS_THIEF:
      modifier = .1;
      break;
    case CLASS_MAGUS:
    case CLASS_MYSTIC:
      modifier = 1.5;
      break;
    case CLASS_AVATAR:
      modifier = 1.6;
      break;
    case CLASS_ASSASSIN:
      modifier = 1.2;
      break;
    case CLASS_PALADIN:
    case CLASS_RANGER:
      modifier = 1.9;
      break;
    case CLASS_NINJA:
      modifier = .6;
      break;
    case CLASS_PSIONIC:
      modifier = .6;
      break;
    default:
      modifier = 1.0;
      break;
    }
  switch(level)
    {
    case 0:
      return(     1);
    case 1:
      return(  1500);
    case 2:
      return(  3000);
    case 3:
      return(  6000);
    case 4:
      return( 11000);
    case 5:
      return( 21000);
    case 6:
      return( 42000);
    case 7:
      return( 80000);
    case 8:
      return(155000);
    case 9:
      return(300000);
    case 10:
      return(450000);
    case 11:
      return(650000);
    case 12:
      return(870000);
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    default:
      return(900000+((level-13)*level*20000)+(level*level*1000)+(modifier*10000*level));
      break;
    }

}

int
exp_needed_for_level(struct char_data *ch )
{
    return(find_exp((int)GET_CLASS(ch), (int)GET_LEVEL(ch)));
}


/* magic.c --> saving throws */
