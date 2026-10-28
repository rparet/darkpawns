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

/* $Id: spell_parser.c 1273 2005-12-20 05:19:42Z mcooper $ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "events.h"

#define SINFO spell_info[spellnum]

/* extern funcs */
extern struct str_app_type str_app[];           
extern struct skill_tree_data skill_tree[];
struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];
void improve_skill(struct char_data *ch, int skill_num);

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

char *unused_spellname = "!UNUSED!"; /* So we can get &unused_spellname */

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
  int mana, modify = 0;
  struct obj_data *wand = GET_EQ(ch, WEAR_WIELD);

  if (wand && (GET_OBJ_TYPE(wand) == ITEM_WAND))
    modify = GET_OBJ_VAL(wand, 0);

  mana = MAX((SINFO.mana_max - (SINFO.mana_change * modify)), SINFO.mana_min);

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
  strcpy(lbuf, spell_info[spellnum].name);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }

  if (tch != NULL && tch->in_room == ch->in_room) 
  {
     if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
     else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
  } 
  else if (tobj != NULL &&
	  ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    {
     sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
    }
  else
     sprintf(lbuf, "$n utters the words, '%%s'.");

  sprintf(buf1, lbuf, spell_info[spellnum].name);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i) ||  
	PLR_FLAGGED(i, PLR_WRITING))
      continue;
    perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
      sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    spell_info[spellnum].name);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

/*
 * This function should be used anytime you are not 100% sure that you have
 * a valid spell/skill number.  A typical for() loop would not need to use
 * this because you can guarantee > 0 and <= TOP_SPELL_DEFINE.
 */
char *skill_name(int num)
{
  if (num > 0 && num <= TOP_SPELL_DEFINE)
    return (spell_info[num].name);
  else if (num == -1)
    return ("UNUSED");
  else
    return ("UNDEFINED");
}

	 
int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*skill_tree[++index].name != '\n') {
    if (is_abbrev(name, skill_tree[index].name))
      return skill_tree[index].skill_number;

    ok = TRUE;
    temp = any_one_arg(skill_tree[index].name, first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return skill_tree[index].skill_number;
  }

  return -1;
}


/* Returns the group based on the provided skill/spell name */
int find_group(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*skill_tree[++index].name != '\n') {
    if (is_abbrev(name, skill_tree[index].name))
      return skill_tree[index].group;

    ok = TRUE;
    temp = any_one_arg(skill_tree[index].name, first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
        ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return skill_tree[index].group;
  }

  return -1;
}

/* Returns the group level based on the provided skill/spell name */
int find_group_level(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*skill_tree[++index].name != '\n') {
    if (is_abbrev(name, skill_tree[index].name))
      return skill_tree[index].level;

    ok = TRUE;
    temp = any_one_arg(skill_tree[index].name, first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
        ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return skill_tree[index].level;
  }

  return 0;
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
     send_to_char("Your magic fizzles out and dies.\r\n", caster);
     act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    return 0;
  }

  if (GET_POS(caster) == POS_SITTING) {
     send_to_char("You cannot do this sitting!\r\n", caster);
     return 0;
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
      case SPELL_SOULTRAP:        MANUAL_SPELL(spell_soul_trap); break;
      case SPELL_SYPHON:          MANUAL_SPELL(spell_syphon_energy); break;
      case SPELL_GROUP_PROT_FROM_EVIL:  MANUAL_SPELL(spell_group_pfe); break;
      case SPELL_GROUP_PROT_FROM_GOOD:  MANUAL_SPELL(spell_group_pfg); break;
      case SPELL_ENTRAP:          MANUAL_SPELL(spell_entrapment); break;

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
 * only be called by the 'drink', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-4:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num [4] unused
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num [4] unused
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num [4] unused
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num [4] unused
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */
void mag_objectmagic(struct char_data * ch, struct obj_data * obj, char *argument)
{
  int i, k, spellnum, wear_pos, equipd = 0;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tch, &tobj);

  for (wear_pos=0;wear_pos < NUM_WEARS;wear_pos++)
    if (GET_EQ(ch, wear_pos))
      if (isname(buf, GET_EQ(ch, wear_pos)->name))
        equipd = wear_pos+1;  /*+1 cuz pos 0 is legal yet FALSE*/ 

  spellnum = GET_OBJ_VAL(obj, 3);

  if (equipd == WEAR_HOLD || (GET_OBJ_TYPE(obj) != ITEM_STAFF))
    switch (GET_OBJ_TYPE(obj)) {
      case ITEM_STAFF:
        act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
        if (obj->action_description)
          act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
        else
          act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

        if (GET_OBJ_VAL(obj, 2) <= 0) {
          act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
          act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
        } else {
          GET_OBJ_VAL(obj, 2)--;
          WAIT_STATE(ch, 1);
          for (tch = world[ch->in_room].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (ch == tch)
              continue;
            if (GET_OBJ_VAL(obj, 0))
              call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 0), CAST_STAFF);
            else
              call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), DEFAULT_STAFF_LVL, CAST_STAFF);
          }
        }
        break;
      case ITEM_WAND:
        if (k == FIND_CHAR_ROOM) {
          if (tch == ch) {
            act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
          } else {
            act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
            if (obj->action_description != NULL)
              act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
            else
              act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
          }
        } else
          if ((tobj != NULL) && (IS_SET(SINFO.targets, TAR_OBJ_INV) || IS_SET(SINFO.targets, TAR_OBJ_ROOM) || 
               IS_SET(SINFO.targets, TAR_OBJ_WORLD) || IS_SET(SINFO.targets, TAR_OBJ_EQUIP))) {
            act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
            if (obj->action_description != NULL)
              act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
            else
              act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
          } else {
            act("At what should $p be pointed?",FALSE, ch, obj, NULL, TO_CHAR);
            return;
          }

        if (GET_OBJ_VAL(obj, 2) <= 0) {
          act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
          act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
          return;
        }
        GET_OBJ_VAL(obj, 2)--;
        WAIT_STATE(ch, 1);
        if (GET_OBJ_VAL(obj, 0))
          call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 0), CAST_WAND);
        else
          call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3), DEFAULT_WAND_LVL, CAST_WAND);
        break;
      case ITEM_SCROLL:
        if ((GET_OBJ_VNUM(obj) == RECALLSCROLL) && (FIGHTING(ch))) {
          act("You can't concentrate enough!.", FALSE, ch, obj, NULL, TO_CHAR);
          return;
        }
        if (GET_POS(ch) == POS_SITTING) {
          send_to_char("You can't do this sitting!!\r\n", ch);
          return;
        }
   
        if (*arg) {
          if (!k) {
            act("There is nothing here to affect with $p.", FALSE, ch, obj, NULL, TO_CHAR);
            return;
          }
        } else
          tch = ch;
      
        act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
        if (obj->action_description)
          act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
        else
          act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

        WAIT_STATE(ch, 1);
        if (obj->carried_by) {            /* stops a crash if player dies while reciting */
          obj_from_char(obj);
          obj_to_room(obj, ch->in_room);
        }
        for (i = 1; i < 4; i++)
          if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), CAST_SCROLL)))
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
        act("You drink $p.", FALSE, ch, obj, NULL, TO_CHAR);
        if (obj->action_description)
          act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
        else
          act("$n drinks $p.", TRUE, ch, obj, NULL, TO_ROOM);

        WAIT_STATE(ch, 1);
        if (obj->carried_by) {            /* stops a crash if player dies while drinking */
          obj_from_char(obj);
          obj_to_room(obj, ch->in_room);
        }
        for (i = 1; i < 4; i++)
          if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), CAST_POTION)))
            break;

        if (obj != NULL)
          extract_obj(obj);
        break;
      default:
        log("SYSERR: Unknown object_type in mag_objectmagic");
        break;
      }
  else { /* its a staff or wand that's worn (castable eq) */
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
    else
      act("$n's uses $p which sparks blindingly, bathing you in its glow.", TRUE, ch, obj, 0, TO_ROOM);
      act("$p radiates an ethereal glow that lights the room.", FALSE, ch, obj, 0, TO_CHAR);
      if (GET_OBJ_VAL(obj, 2) <= 0) {
        act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
        act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
        GET_OBJ_VAL(obj, 2)--;
        WAIT_STATE(ch, 1);

        /* Level to cast spell at. */
        k = GET_OBJ_VAL(obj, 0) ? GET_OBJ_VAL(obj, 0) : DEFAULT_STAFF_LVL;

        if (HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, 3), MAG_MASSES | MAG_AREAS)) {
          call_magic(ch, NULL, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
        } else {
          for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (ch != tch)
              call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
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
        stc("You can only cast this spell upon yourself!\r\n", ch);
        return 0;
    }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF))
    {
        stc("You cannot cast this spell upon yourself!\r\n", ch);
        return 0;
    }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !IS_AFFECTED(ch, AFF_GROUP))
    {
        stc("You can't cast this spell if you're not in a group!\r\n",ch);
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
  struct char_data *tch = NULL, *pers;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0;
  struct event_data *event;

  if (ch->desc && ch->desc->original)
    pers = ch->desc->original;
  else
    pers = ch;

  if (IS_NPC(pers))
    return;

  /* Must be wielding a wand to cast a spell - Immortals excused */
  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (!GET_EQ(ch, WEAR_WIELD)) {
      send_to_char("You can't cast a spell without wielding a wand!\r\n", ch);
      return;
    } else if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) != ITEM_WAND) {
      send_to_char("You can't cast a spell without wielding a wand!\r\n", ch);
      return;
    }
  }

  /* get: blank, spell name, target name */
  strcpy(buf3, argument);       /* we may need this later for the event */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }

  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the magic symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }

  if (GET_SPELL(pers, spellnum) == '0') {
      send_to_char("You are unfamiliar with that spell.\r\n", ch);
      return;
  }

  if (!IS_OUTLAW(ch))
    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && SINFO.violent) {
      stc("This room just has such a peaceful, easy feeling.\r\n", ch);
      return;
    }

  if (!OUTSIDE(ch) && !SINFO.outdoors) {
    stc("You can only do this outdoors!\r\n", ch);
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

  } else {      /* if target string is empty */
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
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) && !SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n", IS_SET(SINFO.targets,
              TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ? "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char(OK, ch);
    say_spell(ch, spellnum, tch, tobj); 
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }

  if (!IS_NPC(ch) && tch)
    if (!IS_NPC(tch) && !tch->desc) {
      act("You can't do that while $N is linkless!", TRUE, ch, 0, tch, TO_CHAR);
      return;
    }

  if (tch) {
    if (!IS_NPC(tch) && !IS_OUTLAW(ch) && SINFO.violent) {
      sprintf(buf, "You cannot cast that on %s as you are not an Outlaw!\r\n",
        GET_NAME(tch));
      stc(buf, ch);
      return;
    }

    if (!IS_NPC(tch) && !IS_OUTLAW(tch) && SINFO.violent) {
      sprintf(buf, "You cannot cast that on %s as %s is not an Outlaw!\r\n",
        GET_NAME(tch), HSSH(tch));
      stc(buf, ch);
      return;
    }
  }

  mana = mag_manacost(pers, spellnum);
  if ((mana > 0) && (GET_MANA(pers) < mana) && (GET_LEVEL(pers) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    return;
  }

  if (GET_COMBAT_WAIT(ch)) {
    CREATE(event, struct event_data, 1);
    event->ch = ch;
    event->args = str_dup(buf3);
    event->cmd = cmd;
    event->subcmd = subcmd;
    event->count = GET_COMBAT_WAIT(ch) + DELAY(ch);
    event->fighting = FIGHTING(ch);
    event->command = do_cast;
    DELAY(ch) += calculate_speed(ch);
    add_event(event);
    return;
  }

  if (FIGHTING(ch))
    GET_COMBAT_WAIT(ch) += calculate_speed(ch);

  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (!skill_success(ch, spellnum, 0)) {
    WAIT_STATE(ch, 1);
    if (!tch || !skill_message(0, pers, tch, spellnum))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(pers) = MAX(0, (GET_MANA(pers) - (mana >> 1)));
    if (SINFO.violent && tch && IS_NPC(tch) && !FIGHTING(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum)) {
      improve_skill(ch, spellnum);
      WAIT_STATE(ch, 1);
      if (tch && !IS_NPC(tch))
        SET_BIT(PLR_FLAGS(tch), PLR_CRASH);
      if (mana > 0)
        GET_MANA(pers) = MAX(0, (GET_MANA(pers) - mana));
    }
  }
}

/* Assign the spells on boot up */
void spello(int spl, char *name, int max_mana, int min_mana,
  int mana_change, int minpos, int targets, int violent, int routines,
  char *wearoff, int outdoors)

{

  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
  spell_info[spl].outdoors = outdoors;
  spell_info[spl].name = strdup(name);
  spell_info[spl].wear_off_msg = wearoff;
}


void unused_spell(int spl)
{

  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
  spell_info[spl].outdoors = 0;
  spell_info[spl].name = unused_spellname;
}

#define skillo(skill, name) spello(skill, name, 0, 0, 0, 0, 0, 0, 0, NULL, TRUE);


/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * spellname: The name of the spell.
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
 * outdoors: TRUE or FALSE, depending on if this is considered an outdoor
 * spell and should not be cast in INDOORS rooms. Should be set on any spell
 * that can be cast outdoors.
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
  for (i = 0; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above */

  /* Rejuvenation Group */
  spello(SPELL_CURE_LIGHT, "cure light", 50, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_POINTS,
  NULL, TRUE);

  spello(SPELL_REMOVE_CURSE, "remove curse", 60, 40, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS,
  NULL, TRUE);

  spello(SPELL_REMOVE_POISON, "remove poison", 60, 40, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, 
  FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS,
  NULL, TRUE);

  spello(SPELL_SOBRIETY, "sobriety", 50, 40, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_HEAL, "heal", 150, 100, 10, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS,
  NULL, TRUE);

  spello(SPELL_CURE_BLIND, "cure blind", 50, 40, 5, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS,
  NULL, TRUE);

  spello(SPELL_MASS_HEAL, "mass heal", 300, 200, 10, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS,
  NULL, TRUE);

  spello(SPELL_INVIGORATE, "invigorate", 300, 200, 10, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_POINTS,
  NULL, TRUE);

  spello(SPELL_VITALITY, "vitality", 500, 400, 20, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_POINTS,
  NULL, TRUE);

  spello(SPELL_GROUP_HEAL, "group heal", 700, 500, 20, POS_FIGHTING,
  TAR_IGNORE, FALSE, MAG_GROUPS,
  NULL, TRUE);


  /* Detection Group */

  spello(SPELL_DETECT_ALIGN, "detect alignment", 40, 20, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "You feel less aware.", TRUE);

  spello(SPELL_DETECT_MAGIC, "detect magic", 40, 20, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "The detect magic wears off.", TRUE);

  spello(SPELL_INFRAVISION, "infravision", 70, 40, 1, POS_STANDING,
  TAR_CHAR_ROOM , FALSE, MAG_AFFECTS,
  "Your night vision seems to fade.", TRUE);

  spello(SPELL_DETECT_INVIS, "detect invisibility", 150, 100, 5, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "Your eyes stop tingling.", TRUE);

  spello(SPELL_SENSE_LIFE, "sense life", 150, 100, 5, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "You feel less aware of your surroundings.", TRUE);

  spello(SPELL_DETECT_RACE, "detect race", 100, 80, 10, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "Your race detection wears off.", TRUE);

  spello(SPELL_DETECT_POISON, "detect poison", 300, 200, 10, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
  "The detect poison wears off.", TRUE);

  spello(SPELL_MINDSIGHT, "mindsight", 500, 400, 20, POS_STANDING,
  TAR_CHAR_WORLD, FALSE, MAG_MANUAL,
  NULL, TRUE);


  /* Protection Group */

  spello(SPELL_BLESS, "bless", 50, 40, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS,
  "You feel less righteous.", TRUE);

  spello(SPELL_PROT_FROM_EVIL, "protection from evil", 150, 100, 5, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "You feel less protected.", TRUE);

  spello(SPELL_PROT_FROM_GOOD, "protection from good", 150, 100, 5, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "You feel less protected.", TRUE);

  spello(SPELL_SANCTUARY, "sanctuary", 300, 200, 10, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "The aura around your body fades.", TRUE);

  spello(SPELL_METALSKIN, "metalskin", 300, 200, 10, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "Your skin softens.", TRUE);

  spello(SPELL_HOLY_SHIELD, "holy shield", 500, 400, 10, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_GROUPS,
  NULL, TRUE);

  spello(SPELL_INVULNERABILITY, "invulnerability", 700, 500, 20, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "The globe of invulnerability around you vanishes.", TRUE);

  spello(SPELL_GROUP_PROT_FROM_GOOD, "group protection from good", 700, 500, 20, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_GROUP_PROT_FROM_EVIL, "group protection from evil", 700, 500, 20, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, TRUE);

  /* Psychic Group */

  spello(SPELL_MINDPOKE, "mind poke", 70, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_MINDATTACK , "mind attack", 100, 80, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_MINDBLAST, "mind blast", 150, 100, 5, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_SOUL_LEECH, "soul leech", 300, 200, 10, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_MENTAL_LAPSE, "mental lapse", 500, 400, 20, POS_STANDING,
  TAR_CHAR_WORLD, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_MIND_BAR, "mind bar", 700, 500, 20, POS_STANDING,
  TAR_CHAR_ROOM, TRUE, MAG_AFFECTS,
  "Your mental capacity returns to normal.", TRUE);

  spello(SPELL_SOULTRAP, "soul trap", 700, 500, 20, POS_STANDING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_MANUAL,
  NULL, TRUE);


  /* Malediction Group */

  spello(SPELL_POISON, "poison", 50, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV | TAR_FIGHT_VICT,
  TRUE, MAG_AFFECTS | MAG_ALTER_OBJS,
  "You feel less sick.", TRUE);

  spello(SPELL_CURSE, "curse", 70, 40, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS,
  "You feel more optimistic.", TRUE);

  spello(SPELL_BURNING_HANDS, "burning hands", 70, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_MAGIC_MISSILE, "magic missile", 100, 80, 5, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_COLOR_SPRAY, "color spray", 150, 100, 5, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_ENTRAP, "entrapment", 150, 100, 5, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_BLINDNESS, "blindness", 300, 200, 5, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS,
  "You feel a cloak of blindness dissolve.", TRUE);

  spello(SPELL_HARM, "harm", 400, 300, 10, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_FIREBALL, "fireball", 250, 200, 10, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_ENERGY_DRAIN, "energy drain", 450, 400, 10, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_ACID_BLAST, "acid blast", 500, 400, 20, POS_FIGHTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, TRUE);

  spello(SPELL_DISRUPT, "ray of disruption", 700, 500, 20, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE); 

  spello(SPELL_DISINTEGRATE, "disintegrate", 700, 500, 20, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE); 


  /* Necromancy Group */

  spello(SPELL_DISPEL_EVIL, "dispel evil", 70, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_DISPEL_GOOD, "dispel good", 70, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE);

  spello(SPELL_ANIMATE_DEAD, "animate dead", 100, 80, 1, POS_STANDING,
  TAR_OBJ_ROOM, FALSE, MAG_SUMMONS,
  NULL, TRUE);

  spello(SPELL_CLONE, "clone", 150, 100, 5, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_SUMMONS,
  NULL, TRUE);

  spello(SPELL_DOMINATE, "dominate", 150, 100, 5, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL,
  "Your free will returns.", TRUE);

  spello(SPELL_MASS_DOMINATE, "mass dominate", 400, 300, 10, POS_STANDING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, TRUE);

  spello(SPELL_SYPHON, "syphon energy", 300, 200, 10, POS_STANDING,
  TAR_OBJ_ROOM | TAR_OBJ_INV, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_CONJURE_ELEMENTAL, "conjure elemental", 700, 500, 20, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL,  TRUE);

  spello(SPELL_DIVINE_INT, "divine intervention", 700, 500, 20, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, TRUE);


  /* Environmental Group */

  spello(SPELL_CALL_LIGHTNING, "call lightning", 70, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, FALSE);

  spello(SPELL_INVISIBLE, "invisibility", 100, 80, 10, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, 
  FALSE, MAG_AFFECTS | MAG_ALTER_OBJS,
  "You feel yourself exposed.", TRUE);

  spello(SPELL_WATERWALK, "waterwalk", 100, 80, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "Your feet seem less boyant.", TRUE);

  spello(SPELL_WATER_BREATHE, "waterbreathe", 100, 80, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "You feel your breathing return to normal.", TRUE);

  spello(SPELL_FLY, "fly", 200, 150, 5, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "Your feet float back down to the ground.", TRUE);

  spello(SPELL_HASTE, "haste", 200, 150, 10, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "You cease moving rapidly.", TRUE);

  spello(SPELL_SLOW, "slow", 200, 150, 5, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "The world begins to pick up speed again.", TRUE);

  spello(SPELL_CONTROL_WEATHER, "control weather", 300, 200, 10, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, FALSE);

  spello(SPELL_GROUP_INVIS, "group invisibility", 400, 300, 10, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_GROUPS,
  NULL, TRUE);

  spello(SPELL_EARTHQUAKE, "earthquake", 400, 350, 10, POS_FIGHTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, TRUE);

  spello(SPELL_METEOR_SWARM, "meteor swarm", 700, 500, 20, POS_STANDING,
  TAR_IGNORE, TRUE, MAG_MANUAL,
  NULL, FALSE);

  spello(SPELL_HELLFIRE, "hellfire", 700, 500, 20, POS_FIGHTING,
  TAR_IGNORE, TRUE, MAG_MANUAL,
  NULL, TRUE); 

  spello(SPELL_FLAMESTRIKE, "flamestrike", 700, 500, 20, POS_STANDING,
  TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS,
  "The flames around you subside.", FALSE);

  spello(SPELL_SMOKESCREEN, "smokescreen", 500, 400, 20, POS_FIGHTING,
  TAR_IGNORE, TRUE, MAG_MASSES,
  NULL, TRUE);
  
  spello(SPELL_CALLIOPE, "calliope", 500, 400, 20, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL,
  NULL, TRUE);


  /* Astral Group */

  spello(SPELL_WORD_OF_RECALL, "word of recall", 70, 40, 1, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_SUMMON, "summon", 100, 80, 1, POS_STANDING,
  TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_GATE, "gate", 150, 100, 5, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, FALSE);

  spello(SPELL_GROUP_RECALL, "group recall", 400, 350, 10, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_GROUPS,
  NULL, TRUE);

  spello(SPELL_DREAM_TRAVEL, "dream travel", 500, 400, 20, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  NULL, TRUE);


  /* Miscellaneous Group */

  spello(SPELL_CREATE_FOOD, "create food", 50, 40, 1, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_CREATIONS,
  NULL, TRUE);

  spello(SPELL_CREATE_WATER, "create water", 50, 40, 1, POS_STANDING,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_SLEEP, "sleep", 150, 100, 5, POS_STANDING,
  TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS,
  "You feel less tired.", TRUE);


  spello(SPELL_STRENGTH, "strength", 300, 200, 10, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "You feel weaker", TRUE);

  spello(SPELL_INTELLECT, "word of intellect", 300, 200, 10, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "The secrets of the world seem less clear.", TRUE);

  spello(SPELL_ENCHANT_WEAPON, "enchant weapon", 700, 500, 20, POS_STANDING,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_ENCHANT_ARMOR, "enchant armor", 700, 500, 20, POS_STANDING,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL,
  NULL,  TRUE);


  /* Leftovers */
  /* ----------------------------------- */

  spello(SPELL_ARMOR, "armor", 30, 15, 3, POS_FIGHTING,
  TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
  "You feel less protected.", TRUE);

  spello(SPELL_TELEPORT, "teleport", 60, 50, 3, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_CHARM, "charm person", 75, 50, 2, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL,
  "You feel more self-confident.", TRUE);

  spello(SPELL_LIGHTNING_BOLT, "lightning bolt", 54, 34, 4, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, FALSE);

  spello(SPELL_LOCATE_OBJECT, "locate object", 25, 20, 1, POS_STANDING,
  TAR_OBJ_WORLD, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_COC, "circle of summoning", 90, 70, 1, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_LYCANTHROPY, "lycanthropy", 1, 1, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
  NULL, TRUE);
  
  spello(SPELL_VAMPIRISM, "vampirism", 1, 1, 1, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
  NULL, TRUE);
  
  spello(SPELL_IDENTIFY, "identify", 125, 100, 10, POS_STANDING,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_PSYSHIELD, "psychic shield", 30, 20, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "Your feel your energy shield dissipate.", TRUE);

  spello(SPELL_ADRENALINE, "adrenaline", 35, 30, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "Your adrenaline returns to its normal production rate.", TRUE);
                                                         
  spello(SPELL_LESSPERCEPT, "lesser perception", 40, 30, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY , FALSE, MAG_AFFECTS,
  "Your perception returns to normal.", TRUE);

  spello(SPELL_GREATPERCEPT, "greater perception", 65, 45, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY , FALSE, MAG_AFFECTS,
  "Your perception returns to normal.", TRUE);

  spello(SPELL_CHANGE_DENSITY, "change density", 70, 55, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "Your molecular density returns to normal.", TRUE);

  spello(SPELL_CELL_ADJUSTMENT, "cell adjustment", 85, 75, 1, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_POINTS,
  NULL, TRUE);

  spello(SPELL_ZEN, "zen", 70, 60, 4, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL,
  "Your trance ends.", TRUE);

  spello(SPELL_MIRROR_IMAGE, "mirror image", 150, 130, 5, POS_STANDING,
  TAR_IGNORE, FALSE, MAG_MANUAL,
  NULL, TRUE);

  spello(SPELL_TRANSPARENCY, "transparency", 35, 25, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "Your skin fades back into view.", TRUE);

  spello(SPELL_KNOW_ALIGN, "know align", 20, 20, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
  "Your perception of others emotions ebbs.", TRUE);

  spello(SPELL_LAY_HANDS, "lay hands", 90, 90, 1, POS_STANDING,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_POINTS,
  NULL, TRUE);

  spello(SPELL_PSIBLAST, "psiblast", 180, 150, 10, POS_FIGHTING,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
  NULL, TRUE); 

  spello(SPELL_RESTORE_MANA, "restore mana", 125, 100, 10, POS_STANDING,
  TAR_CHAR_ROOM, FALSE, MAG_POINTS,
  NULL, TRUE);

  /*
   * These spells are currently not used, not implemented, and not castable.
   * Values for the 'breath' spells are filled in assuming a dragon's breath.
   */

  spello(SPELL_FIRE_BREATH, "fire breath", 0, 0, 0, POS_SITTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, FALSE);

  spello(SPELL_GAS_BREATH, "gas breath", 0, 0, 0, POS_SITTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, FALSE);

  spello(SPELL_FROST_BREATH, "frost breath", 0, 0, 0, POS_SITTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, FALSE);

  spello(SPELL_ACID_BREATH, "acid breath", 0, 0, 0, POS_SITTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, FALSE);

  spello(SPELL_LIGHTNING_BREATH, "lightning breath", 0, 0, 0, POS_SITTING,
  TAR_IGNORE, TRUE, MAG_AREAS,
  NULL, FALSE);

  spello(SPELL_PARALYSE, "paralyse", 0, 0, 0, POS_SITTING,
	 TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel your mobility return.", TRUE);

  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  skillo(SKILL_BACKSTAB, "backstab");
  skillo(SKILL_BASH, "bash");
  skillo(SKILL_HIDE, "hide");
  skillo(SKILL_KICK, "kick");
  skillo(SKILL_PICK_LOCK, "pick lock");
  skillo(SKILL_PUNCH, "punch");
  skillo(SKILL_RESCUE, "rescue");
  skillo(SKILL_SNEAK, "sneak");
  skillo(SKILL_STEAL, "steal");
  skillo(SKILL_TRACK, "track");
  skillo(SKILL_HEADBUTT, "headbutt");
  skillo(SKILL_BEARHUG, "bearhug");
  skillo(SKILL_CUTTHROAT, "cutthroat");
  skillo(SKILL_CHARGE, "charge");
  skillo(SKILL_SCRIBE, "scribe");
  skillo(SKILL_SLUG, "slug");
  skillo(SKILL_TRIP, "trip");
  skillo(SKILL_SMACKHEADS, "smackheads");
  skillo(SKILL_PEEK, "peek");
  skillo(SKILL_SUBDUE, "subdue");
  skillo(SKILL_REFUGE, "refuge");
  skillo(SKILL_SKIN,   "skin");
  skillo(SKILL_STRIKE, "strike of revenge");
  skillo(SKILL_SERPENT_KICK, "serpent kick");
  skillo(SKILL_ESCAPE, "escape");
  skillo(SKILL_KK_RIN, "kuji-kiri rin");
  skillo(SKILL_KK_KYO, "kuji-kiri kyo");
  skillo(SKILL_KK_TOH, "kuji-kiri toh");
  skillo(SKILL_KK_KAI, "kuji-kiri kai");
  skillo(SKILL_KK_JIN, "kuji-kiri jin");
  skillo(SKILL_KK_RETSU, "kuji-kiri retsu");
  skillo(SKILL_KK_ZAI, "kuji-kiri zai");
  skillo(SKILL_KK_ZHEN, "kuji-kiri zhen");
  skillo(SKILL_KK_SHA, "kuji-kiri sha");
  skillo(SKILL_APPRAISE, "appraise");
  skillo(SKILL_FLESH_ALTER, "flesh alter");
  skillo(SKILL_COMPARE, "compare");
  skillo(SKILL_PALM, "palm");
  skillo(SKILL_BERSERK, "berserk");
  skillo(SKILL_PARRY, "parry");
  skillo(SKILL_CIRCLE, "circle");
  skillo(SKILL_GROINRIP, "groinrip");
  skillo(SKILL_SCROUNGE, "scrounge");
  skillo(SKILL_FIRST_AID, "first aid");
  skillo(SKILL_DISARM, "disarm");
  skillo(SKILL_MINDLINK, "mindlink");
  skillo(SKILL_DETECT, "search");
  skillo(SKILL_SHADOW, "shadow");
  skillo(SKILL_DISEMBOWEL, "disembowel");
  skillo(SKILL_TURN, "turn");
  skillo(SKILL_EVASION, "evasion");
  skillo(SKILL_SLEEPER, "sleeper");
  skillo(SKILL_NECKBREAK, "neckbreak");
  skillo(SKILL_AMBUSH, "ambush");
  skillo(SKILL_SCOUT, "scout");
  skillo(SKILL_ALCHEMY, "alchemy");
  skillo(SKILL_RETREAT, "retreat");
}

