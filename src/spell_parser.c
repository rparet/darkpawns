/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University is Copyright (C) 1996, 97 by
  Derek Karnes (dkarnes@mystech.com) and Stephen Thompson (stevet@vt.edu)
  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.
  No original code may be duplicated, reused, or executed without the
  written permission of the authors. All rights reserved.
*/

/* $Id: spell_parser.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

extern struct index_data *obj_index;
extern struct room_data *world;
extern struct str_app_type str_app[];           

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

char *spells[] =
{
  "!RESERVED!",			/* 0 - reserved */

  /* SPELLS */

  "holy ward",			/* 1 */
  "shift reality",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "clone",
  "color spray",		/* 10 */
  "control weather",
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect alignment",
  "detect invisibility",
  "detect magic",		/* 20 */
  "detect poison",
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",		/* 30 */
  "locate object",
  "flame arrow", /* magic missile */
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",			/* 40 */
  "meteor swarm",
  "word of recall",
  "remove poison",
  "sense life",
  "animate dead",
  "dispel good",
  "holy shield",
  "group heal",
  "group recall",
  "infravision",		/* 50 */
  "waterwalk",
  "mass heal",
  "fly",
  "lycanthropy",
  "vampirism",
  "sobriety",
  "group invisibility", 
  "hellfire",
  "enchant armor",
  "identify",			/* 60 */
  "mind poke",
  "mind blast",
  "chameleon",
  "levitate",
  "metalskin",/* 65 */
  "globe of invulnerability",
  "vitality",
  "invigorate",
  "lesser perception", 
  "greater perception",/* 70 */
  "mind attack",
  "adrenaline boost",
  "psychic shield",
  "change density", 
  "acid blast",	/* 75 */
  "dominate",
  "cell adjustment",
  "zen",
  "mirror image",
  "mass dominate",	/* 80 */
  "divine intervention",
  "mind bar",
  "soul leech",
  "mindsight", "transparency",	/* 85 */
  "know align", "gate", "word of intellect", "lay hands", 
  "mental lapse",	/* 90 */
  "smokescreen", "ray of disruption", "disintegration", 
  "calliope", "protection from good",	/* 95 */
  "flame strike", "haste", "slow", "dream travel", "psiblast",	/* 100 */
  "glyph of summoning", "waterbreathe", "!drowning!", "!petrify!", 
  "conjure elemental",
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 110 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 115 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 120 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 125 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 130 */

  /* SKILLS */

  "backstab",			/* 131 */
  "bash",
  "hide",
  "kick",
  "pick lock",
  "punch",
  "rescue",
  "sneak",
  "steal",
  "track",			/* 140 */
  "headbutt", 
  "bearhug", 
  "cutthroat", 
  "trip", 
  "smackheads",			/* 145 */
  "slug", 
  "charge", 
  "shoot",
  "retreat",
  "bite",	/* 150 */
  "peek",
  "subdue",
  "stealth",
  "kabuki",
  "strike of revenge",	  /* 155 */
  "serpent kick",
  "escape of the mongoose",
  "kuji-kiri rin",
  "kuji-kiri kyo",
  "kuji-kiri toh", /* 160 */
  "kuji-kiri kai",
  "kuji-kiri jin",
  "kuji-kiri retsu",
  "kuji-kiri zai",
  "kuji-kiri zhen", /* 165 */
  "kuji-kiri sha",
  "appraise", "flesh alter", "compare", "palm", /* 170 */
  "berserk", "parry", "circle", "groinrip", "sharpen",	/* 175 */
  "scrounge", "disarm", "mindlink", "aid", "search",	/* 180 */
  "shadow", "swordplay", "knifeplay", "disembowel", "turn",	/* 185 */
  "evasion", "sleeper", "dragon kick", "tiger punch", "neckbreak",/*190*/
  "ambush", "scout", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 195 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 200 */

  /* OBJECT SPELLS AND NPC SPELLS/SKILLS */

  "!UNUSED!",			/* 201 */
  "fire breath",
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath",
  "\n"				/* the end */
};


struct syllable {
  char *org;
  char *new;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"},
  {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"},
  {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
  {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};


int mag_manacost(struct char_data * ch, int spellnum)
{
  int mana;

  mana = MAX(SINFO.mana_max - (SINFO.mana_change *
		    (GET_LEVEL(ch) - SINFO.min_level[(int) GET_CLASS(ch)])),
	     SINFO.mana_min);

  return mana;
}


/* say_spell erodes buf, buf1, buf2 */
void
say_spell(struct char_data *ch, int spellnum, struct char_data *tch,
	  struct obj_data *tobj)
{
  char lbuf[256];

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }

  if (tch != NULL && tch->in_room == ch->in_room) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
    {
     if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
     else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
    }
    else
    {
     if (tch == ch)
      sprintf(lbuf, "$n focuses $s will...");
     else
      sprintf(lbuf, "$n stares at $N and focuses $s will...");
    }
  } else if (tobj != NULL &&
	     ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
     sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
    else
     sprintf(lbuf, "$n stares at $p and focuses $s will...");
    }
  else
    {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
     sprintf(lbuf, "$n utters the words, '%%s'.");
    else
      sprintf(lbuf, "$n focuses $s will...");
    }

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i) ||  
	PLR_FLAGGED(i, PLR_WRITING))
      continue;
    if (GET_CLASS(ch) == GET_CLASS(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    GET_CLASS(ch) == GET_CLASS(tch) ? spells[spellnum] : buf);
    else
      sprintf(buf1, "$n focuses $s will on you...");
 
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}


char *skill_name(int num)
{
  int i = 0;

  if (num <= 0) {
    if (num == -1)
      return "UNUSED";
    else
      return "UNDEFINED";
  }

  while (num && *spells[i] != '\n') {
    num--;
    i++;
  }

  if (*spells[i] != '\n')
    return spells[i];
  else
    return "UNDEFINED";
}

	 
int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*spells[++index] != '\n') {
    if (is_abbrev(name, spells[index]))
      return index;

    ok = 1;
    temp = any_one_arg(spells[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}



/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
	     struct obj_data * ovict, int spellnum, int level, int casttype)
{
  int savetype;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return 0;

  if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC)
      && GET_LEVEL(caster) < LVL_IMMORT) 
  {
    if (!IS_PSIONIC(caster) && !IS_MYSTIC(caster))
    {
     send_to_char("Your magic fizzles out and dies.\r\n", caster);
     act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    }
    else
    {
     send_to_char("Your will fades, disturbed by an unseen force.\r\n", caster);
     act("$n's will fades, disturbed by an unseen force.", 
	FALSE, caster, 0, 0, TO_ROOM);
    }
    return 0;
  }

  if (GET_POS(caster) == POS_SITTING) {
     send_to_char("You cannot do this sitting!\r\n", caster);
     return 0;
  }

  if (IS_SET_AR(ROOM_FLAGS(caster->in_room), ROOM_PEACEFUL) &&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
    if (GET_LEVEL(caster) < LVL_IMMORT)
	{
        if (!IS_PSIONIC(caster) && !IS_MYSTIC(caster))
         send_to_char("A flash of white light fills the room, dispelling your "
		 "violent magic!\r\n", caster);
	else
         send_to_char("A flash of white light fills the room, dispelling your "
		 "violent power!\r\n", caster);

        act("White light from no particular source suddenly fills the room, "
	 "then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
        return 0;
	}
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  case CAST_BREATH:
    savetype = SAVING_BREATH;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }


  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    mag_damage(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum)
      {
      case SPELL_TELEPORT:	  MANUAL_SPELL(spell_teleport);break;
      case SPELL_DOMINATE:
      case SPELL_CHARM:		  MANUAL_SPELL(spell_charm); break;
      case SPELL_CREATE_WATER:	  MANUAL_SPELL(spell_create_water); break;
      case SPELL_DETECT_POISON:	  MANUAL_SPELL(spell_detect_poison); break;
      case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
      case SPELL_IDENTIFY:	  MANUAL_SPELL(spell_identify); break;
      case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
      case SPELL_SUMMON:	  MANUAL_SPELL(spell_summon); break;
      case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
      case SPELL_LYCANTHROPY:     MANUAL_SPELL(spell_lycanthropy); break;
      case SPELL_VAMPIRISM:       MANUAL_SPELL(spell_vampirism); break;
      case SPELL_SOBRIETY:        MANUAL_SPELL(spell_sobriety); break;
      case SPELL_HELLFIRE:        MANUAL_SPELL(spell_hellfire); break;
      case SPELL_ENCHANT_ARMOR:   MANUAL_SPELL(spell_enchant_armor); break;
      case SPELL_ZEN:             MANUAL_SPELL(spell_zen); break;
      case SPELL_MINDSIGHT:       MANUAL_SPELL(spell_mindsight); break;
      case SPELL_GATE:            MANUAL_SPELL(spell_gate); break;
      case SPELL_MENTAL_LAPSE:    MANUAL_SPELL(spell_mental_lapse); break;
      case SPELL_CALLIOPE:	  MANUAL_SPELL(spell_calliope); break;
      case SPELL_CONTROL_WEATHER: MANUAL_SPELL(spell_control_weather);break;
      case SPELL_METEOR_SWARM:    MANUAL_SPELL(spell_meteor_swarm); break;
      case SPELL_COC:             MANUAL_SPELL(spell_coc); break;
      case SPELL_CONJURE_ELEMENTAL: MANUAL_SPELL(spell_conjure_elemental); break;
      case SPELL_MIRROR_IMAGE:    MANUAL_SPELL(spell_mirror_image); break;
      case SPELL_DIVINE_INT:      MANUAL_SPELL(spell_divine_int); break;
      default:
	sprintf (buf, "SYSERR: Unknown spellnum %d in manual assign",
		 spellnum);
        mudlog(buf, BRF, LVL_GOD, TRUE);
	log(buf);
      }

  return 1;
}

#define RECALLSCROLL 8052 /* recall scroll vnum */

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */
void
mag_objectmagic(struct char_data * ch, struct obj_data * obj, char *argument)
{
  int i, k, spellnum, wear_pos, equipd = 0;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  for (wear_pos=0;wear_pos < NUM_WEARS;wear_pos++)
    if (GET_EQ(ch, wear_pos))
      if (isname(buf, GET_EQ(ch, wear_pos)->name))
	equipd=wear_pos+1;/*+1 cuz pos 0 is legal yet FALSE*/ 

  spellnum = GET_OBJ_VAL(obj, 3);

  if( equipd==WEAR_HOLD || (GET_OBJ_TYPE(obj) != ITEM_STAFF &&
                            GET_OBJ_TYPE(obj) != ITEM_WAND) )
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_STAFF:
      act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
      if (obj->action_description)
	act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
      else
	act("$n taps $p three times on the ground.",
	    FALSE, ch, obj, 0, TO_ROOM);

      if (GET_OBJ_VAL(obj, 2) <= 0)
	{
	  act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
	  act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
	}
      else
	{
	  GET_OBJ_VAL(obj, 2)--;
	  WAIT_STATE(ch, PULSE_VIOLENCE);
	  for (tch = world[ch->in_room].people; tch; tch = next_tch)
	    {
	      next_tch = tch->next_in_room;
	      if (ch == tch)
		continue;
	      if (GET_OBJ_VAL(obj, 0))
		call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
			   GET_OBJ_VAL(obj, 0), CAST_STAFF);
	      else
		call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3),
			   DEFAULT_STAFF_LVL, CAST_STAFF);
	    }
	}
      break;
    case ITEM_WAND:
      if (k == FIND_CHAR_ROOM)
	{
	  if (tch == ch)
	    {
	      act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	      act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
	    }
	  else
	    {
	      act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	      if (obj->action_description != NULL)
		act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	      else
		act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
	    }
	}
      else
	if ((tobj != NULL) && (IS_SET(SINFO.targets, TAR_OBJ_INV) ||
                              IS_SET(SINFO.targets, TAR_OBJ_ROOM) || 
                             IS_SET(SINFO.targets, TAR_OBJ_WORLD) ||
                             IS_SET(SINFO.targets, TAR_OBJ_EQUIP)))
	  {
	    act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
	    if (obj->action_description != NULL)
	      act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
	    else
	      act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
	  }
	else
	  {
	    act("At what should $p be pointed?",FALSE, ch, obj, NULL, TO_CHAR);
	    return;
	  }

      if (GET_OBJ_VAL(obj, 2) <= 0)
	{
	  act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
	  act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
	  return;
	}
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (GET_OBJ_VAL(obj, 0))
	call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		   GET_OBJ_VAL(obj, 0), CAST_WAND);
      else
	call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		   DEFAULT_WAND_LVL, CAST_WAND);
      break;
    case ITEM_SCROLL:
      if ( (GET_OBJ_VNUM(obj)==RECALLSCROLL) && (FIGHTING(ch)) )
	{
	  act("You can't concentrate enough!.", FALSE, ch, obj, NULL, TO_CHAR);
	  return;
	}
      if (GET_POS(ch) == POS_SITTING)
        {
         send_to_char("You can't do this sitting!!\r\n", ch);
         return;
        }
   
      if (*arg)
	{
	  if (!k)
	    {
	      act("There is nothing to here to affect with $p.", FALSE,
		  ch, obj, NULL, TO_CHAR);
	      return;
	    }
	}
      else
	tch = ch;
      
      act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
      if (obj->action_description)
	act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
      else
	act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

      WAIT_STATE(ch, PULSE_VIOLENCE);
      for (i = 1; i < 4; i++)
	if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
			 GET_OBJ_VAL(obj, 0), CAST_SCROLL)))
	  break;

      if (obj != NULL)
	extract_obj(obj);
      break;
    case ITEM_POTION:
      tch = ch;
      if (GET_POS(ch) == POS_SITTING) {
         stc("You can't do this sitting!\r\n", ch);
         return;
      }
      act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
      if (obj->action_description)
	act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
      else
	act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

      WAIT_STATE(ch, PULSE_VIOLENCE);
      for (i = 1; i < 4; i++)
	if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
			 GET_OBJ_VAL(obj, 0), CAST_POTION)))
	  break;

      if (obj != NULL)
	extract_obj(obj);
      break;
    default:
      log("SYSERR: Unknown object_type in mag_objectmagic");
      break;
    }
  else /* its a staff or wand that's worn (castable eq) */
    {
      if (GET_OBJ_TYPE(obj) == ITEM_WAND)
	{
	  if (k == FIND_CHAR_ROOM)
	    {
	      if (tch == ch)
		{
		  act("Your $p bathes you in a blinding glow!", 
		      FALSE, ch, obj, 0, TO_CHAR);
      		  if (obj->action_description != NULL)
	  	    act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
      		  else
		    act("$n's $p bathes $m in a blinding glow!", 
		        FALSE, ch, obj, 0, TO_ROOM);
		} 
	      else
		{
		 act("Your $p flares up with a blinding glow "
		     "that surges toward $N!", FALSE, ch, obj, tch, TO_CHAR);
      		  if (obj->action_description != NULL)
	  	    act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
      		  else
		     act("$n's $p flares up with a blinding glow "
		         "that surges toward $N!", TRUE, ch, obj, tch, TO_ROOM);
		}
	    }
	  else if ((tobj != NULL) && (IS_SET(SINFO.targets, TAR_OBJ_INV) ||
                                     IS_SET(SINFO.targets, TAR_OBJ_ROOM) ||
                                    IS_SET(SINFO.targets, TAR_OBJ_WORLD) ||
                                    IS_SET(SINFO.targets, TAR_OBJ_EQUIP)))   
	  {
	    act("Your $p flares up with a blinding glow that "
		"surges toward $P!", FALSE, ch, obj, tobj, TO_CHAR);
      	    if (obj->action_description)
	        act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      	    else
	        act("$n's $p flares up with a blinding glow that "
			"surges toward $P!", TRUE, ch, obj, tobj, TO_ROOM);
	  }
	  else 
	  {
	    act("You can't use $p like that.", FALSE, ch, obj, NULL, TO_CHAR);
	    return;
	  }
	  if (GET_OBJ_VAL(obj, 2) <= 0)
	    {
	      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
	      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
	      return;
	    }
	  GET_OBJ_VAL(obj, 2)--;
	  WAIT_STATE(ch, PULSE_VIOLENCE);
	  if (GET_OBJ_VAL(obj, 0))
	    call_magic(ch,tch,tobj,GET_OBJ_VAL(obj,3),
		       GET_OBJ_VAL(obj,0),CAST_WAND);
	  else
	    call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		       DEFAULT_WAND_LVL, CAST_WAND);
	}
      else    	/*staff*/
	{
      	  if (obj->action_description)
	      act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      	  else
	      act("$n's $p sparks blindingly, bathing you in its glow.", 
	          TRUE, ch, obj, 0, TO_ROOM);
	  act("Your $p radiates an ethereal glow that lights the room.", 
	      FALSE, ch, obj, 0, TO_CHAR);
	  if (GET_OBJ_VAL(obj, 2) <= 0) {
	    act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
	    act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
	  } else {
	    GET_OBJ_VAL(obj, 2)--;
	    WAIT_STATE(ch, PULSE_VIOLENCE);
            
            /* Level to cast spell at. */
            k = GET_OBJ_VAL(obj, 0) ? GET_OBJ_VAL(obj, 0) : DEFAULT_STAFF_LVL;

            if (HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, 3), MAG_MASSES | MAG_AREAS)) {
              call_magic(ch, NULL, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
            } else {
	      for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	        next_tch = tch->next_in_room;
	        if (ch != tch)
		  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
              }
	    }
	  }
	}
    }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
	           struct obj_data * tobj, int spellnum)
{
  char buf[256];

  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE)
    {
      sprintf(buf, "SYSERR: cast_spell trying to call spellnum %d\n", spellnum);
      log(buf);
      return 0;
    }
    
  if (GET_WIS(ch) == 0 || GET_INT(ch) == 0)
    {
      send_to_char("You're not smart enough to cast!\r\n", ch);
      return 0;
    }     
                         
  if (GET_MOB_WAIT(ch)) 
    {
     stc("You can't cast anything yet!\r\n", ch);
     return 0;
    }

  if (GET_POS(ch) < SINFO.min_position)
    {
      switch (GET_POS(ch))
	{
	case POS_SLEEPING:
	  send_to_char("You dream about great magical powers.\r\n", ch);
	  break;
	case POS_RESTING:
	  send_to_char("You cannot concentrate while resting.\r\n", ch);
	  break;
	case POS_SITTING:
	  send_to_char("You can't do this sitting!\r\n", ch);
	  break;
	case POS_FIGHTING:
	  send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
	  break;
	default:
	  send_to_char("You can't do much of anything like this!\r\n", ch);
	  break;
	}
      return 0;
    }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch))
    {
      send_to_char("You are afraid you might hurt your master!\r\n", ch);
      return 0;
    }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY))
    {
      if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
        stc("You can only cast this spell upon yourself!\r\n", ch);
      else
        stc("You can only will this power upon yourself!\r\n", ch);
      return 0;
    }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF))
    {
      if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
        stc("You cannot cast this spell upon yourself!\r\n", ch);
      else
        stc("You cannot will this power upon yourself!\r\n", ch);
      return 0;
    }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !IS_AFFECTED(ch, AFF_GROUP))
    {
      if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
        stc("You can't cast this spell if you're not in a group!\r\n",ch);
      else
        stc("You cannot use this power if you are not in a group!\r\n",ch);
      return 0;
    }
  send_to_char(OK, ch);
  say_spell(ch, spellnum, tch, tobj);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */

ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0, weight_add = 0;

  if (IS_NPC(ch))
    return;

  if (CMD_IS("cast") && IS_PSIONIC(ch) && GET_LEVEL(ch) < LVL_IMMORT)
  {
	send_to_char ("Psionics 'will' things, not 'cast' them!\r\n", ch);
	return;
  }

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
       send_to_char("Cast what where?\r\n", ch);
    else
	send_to_char ("Will what?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      send_to_char("Spell names must be enclosed in the "
			"magick symbols: '\r\n", ch);
    else
      send_to_char("Psionic powers must be enclosed in the symbols: '\r\n",ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
        send_to_char("Cast what?!?\r\n", ch);
    else
	send_to_char ("Will what?!?\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) < SINFO.min_level[(int) GET_CLASS(ch)]) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      send_to_char("You do not know that spell!\r\n", ch);
    else
      send_to_char("You are not learned in that power!\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, spellnum) == 0) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      send_to_char("You are unfamiliar with that spell.\r\n", ch);
    else
      send_to_char("You are unfamiliar with that power.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && SINFO.violent)
    {
      stc("This room just has such a peaceful, easy feeling..\r\n", ch);
      return;
    }

  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;

  }
  else
    {			/* if target string is empty */
      if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
	if (FIGHTING(ch) != NULL) {
	  tch = ch;
	  target = TRUE;
	}
      if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
	if (FIGHTING(ch) != NULL) {
	  tch = FIGHTING(ch);
	  target = TRUE;
	}
      /* if no target specified, and the spell isn't violent, default to self */
      if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	  !SINFO.violent) {
	tch = ch;
	target = TRUE;
      }
      if (!target) {
	if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
	  sprintf(buf, "Upon %s should the spell be cast?\r\n",
		  IS_SET(SINFO.targets,
			 TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
		  "what" : "who");
	else
	  sprintf(buf, "Upon %s should the power be willed?\r\n",
		  IS_SET(SINFO.targets,
			 TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
		  "what" : "who");
	send_to_char(buf, ch);
	return;
      }
    }

  if (target && (tch == ch) && SINFO.violent) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      send_to_char("You shouldn't cast that on yourself -- could be bad for "
		"your health!\r\n", ch);
    else
     send_to_char("Exerting that power on yourself could be harmful!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char(OK, ch);
    say_spell(ch, spellnum, tch, tobj); 
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      send_to_char("Cannot find the target of your spell!\r\n", ch);
    else
     send_to_char("Cannot find the target of your will!\r\n", ch);
    return;
  }
  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    if (!IS_PSIONIC(ch) && !IS_MYSTIC(ch))
      send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    else
      send_to_char("You haven't the energy to will that power!\r\n", ch);
    return;
  
  }
  if (IS_CARRYING_W(ch))
    weight_add = (CAN_CARRY_W(ch)/IS_CARRYING_W(ch));
  else
    weight_add = -1;
  if (weight_add >= 4)
	weight_add = 0;
  if (weight_add == 3)
	weight_add = 5;
  if (weight_add == 2)
	weight_add = 7;
  if (weight_add == 1)
	weight_add = 10; 
  if (weight_add == -1)
	weight_add = 0;
  weight_add = MAX(0, weight_add);
  if (GET_LEVEL(ch) >= LEVEL_IMMORT)
	weight_add = -20; /* imms succeed all spells */
  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (number(0, 101+weight_add) > GET_SKILL(ch, spellnum)) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, spellnum))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(ch) = MAX(0, (GET_MANA(ch) - (mana >> 1)));
    if (SINFO.violent && tch && IS_NPC(tch) && !FIGHTING(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (mana > 0)
	GET_MANA(ch) = MAX(0, (GET_MANA(ch) - mana));
    }
  }
}



void spell_level(int spell, int class, int level)
{
  char buf[256];
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    sprintf(buf, "SYSERR: attempting assign to illegal spellnum %d", spell);
    log(buf);
    return;
  }

  if (class < 0 || class >= NUM_CLASSES) {
    sprintf(buf, "SYSERR: assigning '%s' to illegal class %d",
	    skill_name(spell), class);
    log(buf);
    bad = 1;
  }

  if (level < 1 || level > LVL_IMPL) {
    sprintf(buf, "SYSERR: assigning '%s' to illegal level %d",
	    skill_name(spell), level);
    log(buf);
    bad = 1;
  }

  if (!bad)    
    spell_info[spell].min_level[class] = level;
}


/* Assign the spells on boot up */
void spello(int spl, int max_mana, int min_mana, int mana_change, int minpos,
	         int targets, int violent, int routines)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMMORT;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
}


void unused_spell(int spl)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMPL + 1;
  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
}

#define skillo(skill) spello(skill, 0, 0, 0, 0, 0, 0, 0);


/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell).
 *
 * minmana :  The minimum mana this spell will take, no matter how high
 * level the caster is.
 *
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

/*
 * NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
 * In order to make this cleaner, as well as to make adding new classes
 * much easier, spell levels are now assigned in class.c.  You only need
 * a spello() call to define a new spell; to decide who gets to use a spell
 * or skill, look in class.c.  -JE 5 Feb 1996
 */

void mag_assign_spells(void)
{
  int i;

  /* Do not change the loop below */
  for (i = 1; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above */

  spello(SPELL_ARMOR, 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_TELEPORT, 60, 50, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_MANUAL);

  spello(SPELL_BLESS, 36, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_BLINDNESS, 35, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE,
        MAG_AFFECTS);

  spello(SPELL_BURNING_HANDS, 45, 20, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CALL_LIGHTNING, 68, 52, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CHARM, 75, 50, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_CHILL_TOUCH, 35, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CLONE, 80, 65, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_SUMMONS);

  spello(SPELL_COLOR_SPRAY, 58, 38, 4, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CONTROL_WEATHER, 75, 25, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_CREATE_FOOD, 35, 10, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS);

  spello(SPELL_CREATE_WATER, 35, 10, 5, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_CURE_BLIND, 35, 5, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(SPELL_CURE_CRITIC, 70, 40, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_LIGHT, 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURSE, 80, 50, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_FIGHT_VICT,
        TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_DETECT_ALIGN, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_INVIS, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM , FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_MAGIC, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_POISON, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_DISPEL_EVIL, 95, 65, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DISPEL_GOOD, 95, 65, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EARTHQUAKE, 70, 50, 5, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DREAM_TRAVEL, 60, 45, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ENCHANT_WEAPON, 200, 150, 10, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_ENERGY_DRAIN, 60, 45, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HOLY_SHIELD, 90, 65, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_FIREBALL, 70, 50, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GROUP_HEAL, 210, 150, 5, POS_FIGHTING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_CHAMELEON, 50, 30, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_GROUP_RECALL, 155, 125, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_HARM, 105, 75, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HASTE, 140, 140, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_HEAL, 90, 80, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS);

  spello(SPELL_HELLFIRE, 200, 150, 10, POS_FIGHTING,
         TAR_IGNORE, TRUE, MAG_MANUAL | MAG_AREAS); 

  spello(SPELL_INFRAVISION, 25, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM , FALSE, MAG_AFFECTS);

  spello(SPELL_GROUP_INVIS, 135, 135, 1, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_INVISIBLE, 45, 45, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, 
		FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_LEVITATE, 90, 70, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_LIGHTNING_BOLT, 54, 34, 4, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LOCATE_OBJECT, 25, 20, 1, POS_STANDING,
	TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_MAGIC_MISSILE, 30, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_MINDPOKE, 30, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_MINDBLAST, 70, 40, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_POISON, 50, 40, 2, POS_FIGHTING,
	 TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV | TAR_FIGHT_VICT,
         TRUE,
	 MAG_AFFECTS | MAG_ALTER_OBJS);
  
  spello(SPELL_FLAMESTRIKE, 105, 100, 1, POS_STANDING,
         TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_PROT_FROM_EVIL, 50, 50, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_PROT_FROM_GOOD, 50, 50, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_REMOVE_CURSE, 45, 45, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SANCTUARY, 110, 85, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SHOCKING_GRASP, 55, 35, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SLEEP, 40, 35, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_SOBRIETY, 35, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_STRENGTH, 35, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SUMMON, 90, 70, 1, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_COC, 90, 70, 1, POS_STANDING,
         TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_WORD_OF_RECALL, 50, 50, 1, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_REMOVE_POISON, 40, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, 
		FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SENSE_LIFE, 30, 20, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SLOW, 80, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MASS_HEAL, 130, 100, 1, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS);

  spello(SPELL_WATERWALK, 80, 55, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_FLY, 100, 80, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_LYCANTHROPY, 1, 1, 1, POS_STANDING,
 	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);
  
  spello(SPELL_VAMPIRISM, 1, 1, 1, POS_STANDING,
 	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);
  
  spello(SPELL_ENCHANT_ARMOR, 150, 130, 10, POS_STANDING,
      TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_IDENTIFY, 125, 100, 10, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_METALSKIN, 75, 60, 1, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);
  
  spello(SPELL_INVULNERABILITY, 85, 85, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_VITALITY, 110, 100, 1, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_INVIGORATE, 110, 95, 1, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_PSYSHIELD, 30, 20, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ADRENALINE, 35, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_MINDATTACK , 55, 25, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);
                                                         
  spello(SPELL_LESSPERCEPT, 40, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY , FALSE, MAG_AFFECTS);

  spello(SPELL_GREATPERCEPT, 65, 45, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY , FALSE, MAG_AFFECTS);

  spello(SPELL_CHANGE_DENSITY, 70, 55, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ACID_BLAST, 35, 20, 1, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DOMINATE, 75, 50, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_MASS_DOMINATE, 220, 150, 10, POS_STANDING,
         TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_CELL_ADJUSTMENT, 85, 75, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_POINTS);

  spello(SPELL_ZEN, 70, 60, 4, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_MIRROR_IMAGE, 150, 130, 5, POS_STANDING,
         TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_SOUL_LEECH, 60, 55, 1, POS_FIGHTING,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_MINDSIGHT, 70, 60, 1, POS_STANDING,
	 TAR_CHAR_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_MIND_BAR, 115, 100, 1, POS_STANDING,
         TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_TRANSPARENCY, 35, 25, 1, POS_STANDING,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_KNOW_ALIGN, 20, 20, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_GATE, 95, 95, 1, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_INTELLECT, 60, 60, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_LAY_HANDS, 90, 90, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_POINTS);

  spello(SPELL_MENTAL_LAPSE, 100, 90, 1, POS_STANDING,
	 TAR_CHAR_WORLD, FALSE, MAG_MANUAL);
 
  spello(SPELL_SMOKESCREEN, 100, 100, 1, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_MASSES);
  
  spello(SPELL_DISRUPT, 175, 165, 1, POS_FIGHTING,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE); 

  spello(SPELL_DIVINE_INT, 290, 290, 1, POS_STANDING,
         TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_DISINTEGRATE, 120, 120, 1, POS_FIGHTING,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE); 

  spello(SPELL_ANIMATE_DEAD, 120, 100, 10, POS_STANDING,
	TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);
  
  spello(SPELL_CALLIOPE, 100, 50, 10, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

  spello(SPELL_METEOR_SWARM, 180, 170, 5, POS_STANDING,
        TAR_IGNORE, TRUE, MAG_MANUAL);

  spello(SPELL_PSIBLAST, 180, 150, 10, POS_FIGHTING,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE); 

  spello(SPELL_WATER_BREATHE, 92, 58, 6, POS_STANDING,
         TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);
  
  spello(SPELL_CONJURE_ELEMENTAL, 165, 145, 1, POS_STANDING,
	 TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_FIRE_BREATH, 70, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FROST_BREATH, 70, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_GAS_BREATH, 70, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ACID_BREATH, 70, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_LIGHTNING_BREATH, 70, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);


  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  skillo(SKILL_BACKSTAB);
  skillo(SKILL_BASH);
  skillo(SKILL_HIDE);
  skillo(SKILL_KICK);
  skillo(SKILL_PICK_LOCK);
  skillo(SKILL_PUNCH);
  skillo(SKILL_RESCUE);
  skillo(SKILL_SNEAK);
  skillo(SKILL_STEAL);
  skillo(SKILL_TRACK);
  skillo(SKILL_HEADBUTT);
  skillo(SKILL_BEARHUG);
  skillo(SKILL_CUTTHROAT);
  skillo(SKILL_CHARGE);
  skillo(SKILL_SLUG);
  skillo(SKILL_TRIP);
  skillo(SKILL_SMACKHEADS);
  skillo(SKILL_PEEK);
  skillo(SKILL_SUBDUE);
  skillo(SKILL_STEALTH);
  skillo(SKILL_KABUKI);
  skillo(SKILL_STRIKE);
  skillo(SKILL_SERPENT_KICK);
  skillo(SKILL_ESCAPE);
  skillo(SKILL_KK_RIN);
  skillo(SKILL_KK_KYO);
  skillo(SKILL_KK_TOH);
  skillo(SKILL_KK_KAI);
  skillo(SKILL_KK_JIN);
  skillo(SKILL_KK_RETSU);
  skillo(SKILL_KK_ZAI);
  skillo(SKILL_KK_ZHEN);
  skillo(SKILL_KK_SHA);
  skillo(SKILL_APPRAISE);
  skillo(SKILL_FLESH_ALTER);
  skillo(SKILL_COMPARE);
  skillo(SKILL_PALM);
  skillo(SKILL_BERSERK);
  skillo(SKILL_PARRY);
  skillo(SKILL_CIRCLE);
  skillo(SKILL_GROINRIP);
  skillo(SKILL_SHARPEN);
  skillo(SKILL_SCROUNGE);
  skillo(SKILL_FIRST_AID);
  skillo(SKILL_DISARM);
  skillo(SKILL_MINDLINK);
  skillo(SKILL_DETECT);
  skillo(SKILL_SHADOW);
  skillo(SKILL_DISEMBOWEL);
  skillo(SKILL_TURN);
  skillo(SKILL_EVASION);
  skillo(SKILL_SLEEPER);
  skillo(SKILL_NECKBREAK);
  skillo(SKILL_AMBUSH);
  skillo(SKILL_SCOUT);
}

