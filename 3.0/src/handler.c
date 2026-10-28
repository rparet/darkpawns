/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
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

/* $Id: handler.c 1297 2006-06-09 05:20:08Z rparet $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"
#include "events.h"
#include "house.h"

/* local vars */
int extractions_pending = 0;

/* external vars */
extern void death_cry(struct char_data *ch);

/* external functions */
extern int circle_check (struct char_data *ch);
void free_char(struct char_data * ch);
void stop_fighting(struct char_data * ch);
void remove_follower(struct char_data * ch);
void clearMemory(struct char_data * ch);
void tattoo_af(struct char_data *ch, bool add);
struct char_data *HUNTING(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);
int has_light(struct char_data *ch);
void sql_remove_obj(struct obj_data *obj);


char *fname(char *namelist)
{
  static char holder[30];
  register char *point;

  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;

  *point = '\0';

  return (holder);
}

int isname(char *str, char *namelist)
{
  register char *curname, *curstr;

  curname = namelist;
  if (curname == NULL) return 0;
  if (str == NULL) return 0;

  for (;;) {
    for (curstr = str;; curstr++, curname++) {
      if (!*curstr && !isalpha(*curname))
	return (1);

      if (!*curname)
	return (0);

      if (!*curstr || *curname == ' ')
	break;

      if (LOWER(*curstr) != LOWER(*curname))
	break;
    }

    /* skip to next name */

    for (; isalpha(*curname); curname++);
    if (!*curname)
      return (0);
    curname++;			/* first char of new name */
  }
}

#define WHITESPACE " \t"
/* unused, but works */
int 
isname_with_abbrevs(char *str, char *namelist)
{
  register char *newlist, *curtok;
  if (namelist)
  {
   newlist = strdup(namelist);

   for(curtok = strtok(newlist, WHITESPACE); 
       curtok; 
       curtok = strtok(NULL, WHITESPACE))
         if(curtok && is_abbrev(str, curtok))
          {
            FREE(newlist); 
            return 1;
          }
   FREE(newlist);
  }
  return 0;
}

void aff_apply_modify(struct char_data *ch, byte loc, sh_int mod, char *msg)
{
   int maxabil;

   maxabil = (IS_NPC(ch) ? 25 : 18);  /* not used but why remove it? */
   switch(loc) {
     case APPLY_NONE:   break;
     case APPLY_STR:    GET_STR(ch) += mod;
                        break;
     case APPLY_DEX:    GET_DEX(ch) += mod;
                        break;
     case APPLY_INT:    GET_INT(ch) += mod;
                        break;
     case APPLY_WIS:    GET_WIS(ch) += mod;
                        break;
     case APPLY_CON:    GET_CON(ch) += mod;
                        break;
     case APPLY_CHA:    GET_CHA(ch) += mod;
                        break;
     case APPLY_DODGEROLL: GET_DODGEROLL(ch) += mod;
                           break;
     case APPLY_LEVEL:  /* GET_LEVEL(ch) += mod */
                        break;
     case APPLY_AGE:    ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
                        break;
     case APPLY_CHAR_WEIGHT:
                        GET_WEIGHT(ch) += mod;
                        break;
     case APPLY_MANA:   GET_MAX_MANA(ch) += mod;
                        break;
     case APPLY_HIT:    GET_MAX_HIT(ch) += mod;
                        break;
     case APPLY_MOVE:   GET_MAX_MOVE(ch) += mod;
                        break;
     case APPLY_GOLD:   break;
     case APPLY_MANA_REGEN:
                        break;
     case APPLY_MOVE_REGEN:
                        break;
     case APPLY_HIT_REGEN:
			break;
     case APPLY_EXP:    break;
     case APPLY_AC:     GET_AC(ch) += mod;
                        break;
     case APPLY_HITROLL:
                        GET_HITROLL(ch) += mod;
                        break;
     case APPLY_DAMROLL:
                        GET_DAMROLL(ch) += mod;
                        break;
     case APPLY_SAVING_PARA:
                        GET_SAVE(ch, SAVING_PARA) += mod;
                        break;
     case APPLY_SAVING_ROD:
                        GET_SAVE(ch, SAVING_ROD) += mod;
                        break;
     case APPLY_SAVING_PETRI:
                        GET_SAVE(ch, SAVING_PETRI) += mod;
                        break;
     case APPLY_SAVING_BREATH:
                        GET_SAVE(ch, SAVING_BREATH) += mod;
                        break;
     case APPLY_SAVING_SPELL:
                        GET_SAVE(ch, SAVING_SPELL) += mod;
                        break;
     case APPLY_SPELL:
	if(IS_NPC(ch))
		break;
	else
	{
   			struct master_affected_type *aff;
			if (mod > 0)
			{
			  int found = 0; 
  			  if (ch->affected)
    			    for (aff = ch->affected; aff; aff = aff->next)
      				if (aff->bitvector)
				  if (aff->bitvector == mod)
				    found = 1;
			  if (!found)
			    SET_BIT(AFF_FLAGS(ch), mod);
			}
			else
			{
			  int found = 0; 
  			  if (ch->affected)
    			    for (aff = ch->affected; aff; aff = aff->next)
      				if (aff->bitvector)
				  if (aff->bitvector == -mod)
				    found = 1;
			  if (!found)
			    REMOVE_BIT(AFF_FLAGS(ch), -mod);
			}
	} /*!IS_NPC*/

			break;

     default:           sprintf(buf, "SYSERR: Unknown apply (%d) adjust "
                                "attempt (handler.c, %s).", loc, msg);
                        log(buf);
                        break;
   } /* switch */
}

void
affect_modify(struct char_data * ch, byte loc, sh_int mod, bitvector_t bitv, bool add)
{
  if (add)
    SET_BIT(AFF_FLAGS(ch), bitv);
  else
    {
      REMOVE_BIT(AFF_FLAGS(ch), bitv);
      mod = -mod;
    }

  aff_apply_modify(ch, loc, mod, "affect_modify");
}

/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data * ch)
{
  struct master_affected_type *af;
  int i, j;

  /* first, substract */
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
  }

  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

  tattoo_af(ch, FALSE);
  vitae_af(ch, FALSE);

  ch->aff_abils = ch->real_abils;

  /* now add it all again */
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i))
      for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, GET_EQ(ch, i)->affected[j].location,
		      GET_EQ(ch, i)->affected[j].modifier,
		      GET_EQ(ch, i)->obj_flags.bitvector, TRUE);
  }

  for (af = ch->affected; af; af = af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

  tattoo_af(ch, TRUE);
  vitae_af(ch, TRUE);

  /* for (i = 0; i < MAX_SKILLS; i++) {
    if (GET_SKILL(ch, i))
      GET_SKILL(ch, i) = calculate_skill_value(ch, i);
  }   -- this seems to be stuffing up skills */

  /* Make certain values are between 0..666, not < 0 and not > 666! */

  i = 666;

  GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), i));
  GET_INT(ch) = MAX(0, MIN(GET_INT(ch), i));
  GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), i));
  GET_CON(ch) = MAX(0, MIN(GET_CON(ch), i));
  GET_STR(ch) = MAX(0, MIN(GET_STR(ch), i));
 
  if (GET_ALIGNMENT(ch)>1000)
	GET_ALIGNMENT(ch) = 1000;
  if (GET_ALIGNMENT(ch)<-1000)
	GET_ALIGNMENT(ch) = -1000;
}


/* Serapis Wed Jun 10 10:07:12 1998 */
void master_affect_to_char(struct char_data *ch, struct affected_type *af,
			   by_types_enum by_type, int obj_num)
{
  struct master_affected_type *affected_alloc;

  CREATE(affected_alloc, struct master_affected_type, 1);

  affected_alloc->type = af->type;
  affected_alloc->duration = af->duration;
  affected_alloc->location=af->location;
  affected_alloc->modifier=af->modifier;
  affected_alloc->bitvector=af->bitvector;
  affected_alloc->by_type = by_type;
  affected_alloc->obj_num = obj_num;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;

  affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  affect_total(ch);
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data * ch, struct affected_type * af)
{
   master_affect_to_char(ch, af, BY_SPELL, 0);
}

void affect_to_char2(struct char_data *ch, struct master_affected_type *af)
{
  struct master_affected_type *affected_alloc;

  CREATE(affected_alloc, struct master_affected_type, 1);

  *affected_alloc = *af;
  affected_alloc->by_type = BY_SPELL;
  affected_alloc->obj_num = 0;
  
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;

  affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  affect_total(ch);
}


/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void affect_remove(struct char_data * ch, struct master_affected_type * af, bool erase)
{
  struct master_affected_type *temp; /* used in REMOVE_FROM_LIST macro */
  void sql_remove_affect(struct char_data *ch, struct master_affected_type *af);

  assert(ch->affected);

  affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
  if (erase)
    sql_remove_affect(ch, af);
  REMOVE_FROM_LIST(af, ch->affected, next);
  FREE(af);
  affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data * ch, int type)
{
  struct master_affected_type *hjp, *next;

  for (hjp = ch->affected; hjp; hjp = next) {
    next = hjp->next;
    if (hjp->type == type)
      affect_remove(ch, hjp, TRUE);
  }
}



/*
 * Return if a char is affected by a spell (SPELL_XXX), NULL indicates
 * not affected
 */
bool affected_by_spell(struct char_data * ch, int type)
{
  struct master_affected_type *hjp;

  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if (hjp->type == type && hjp->by_type == BY_SPELL)
      return TRUE;

  return FALSE;
}

void affect_join(struct char_data * ch, struct master_affected_type * af,
		             bool avg_dur, bool add_mod, bool avg_mod)
{
  struct master_affected_type *hjp;
  bool found = FALSE;

  for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
    if ((hjp->type == af->type) && (hjp->location == af->location)) {
      if (avg_dur)
	      af->duration >>= 1;

      if (add_mod)
	      af->modifier += hjp->modifier;
      if (avg_mod)
	      af->modifier >>= 1;

      affect_remove(ch, hjp, TRUE);
      affect_to_char2(ch, af);
      found = TRUE;
    }
  }
  if (!found)
    affect_to_char2(ch, af);
}


/* move a player out of a room */
void char_from_room(struct char_data * ch)
{
  struct char_data *temp, *i;
  
  if (ch == NULL) {
    log("SYSERR: NULL character in handler.c, char_from_room");
    exit(1);
  }

  if (ch->in_room == NOWHERE) {
    log("SYSERR: NOWHERE in handler.c, char_from_room");
    exit(1);
  }

  if (FIGHTING(ch) != NULL)
  {
    for (i = world[ch->in_room].people; i; i = i->next_in_room) /* stop everyone in the room from fighting ch */
      if (FIGHTING(i) == ch)
        stop_fighting(i);
    stop_fighting(ch);
  }

  if(has_light(ch) && world[ch->in_room].light)  /* If you've got a working light eq'd */
    world[ch->in_room].light--;

  REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
  ch->in_room = NOWHERE;
  ch->next_in_room = NULL;

  circle_check(ch); /* circle of summoning */
}


/* place a character in a room */
void char_to_room(struct char_data * ch, long room)
{
  if (!ch || room < 0 || room > top_of_world)
    log("SYSERR: Illegal value(s) passed to char_to_room");
  else {
    ch->next_in_room = world[room].people;
    world[room].people = ch;
    ch->in_room = room;

    if (has_light(ch))  /* if char has a light eq'd */
      world[room].light++;

    /* stop fighting, if we left */
    /* note: this should have been done already in char_from_room */
    if (FIGHTING(ch) && IN_ROOM(ch) != IN_ROOM(FIGHTING(ch))) {
      if (FIGHTING(FIGHTING(ch)) == ch) /* if the person you were fighting was fighting you */
        stop_fighting(FIGHTING(ch));
      stop_fighting(ch);
    }

    circle_check(ch); /* circle of summoning */

    if ((ROOM_FLAGGED(ch->in_room, ROOM_DEATH)) && (GET_LEVEL(ch) < LVL_IMMORT || IS_NPC(ch))) {
      look_at_room(ch, 0);
      log_death_trap(ch);
      GET_LOADROOM(ch) = NOWHERE;
      death_cry(ch); 
      extract_char(ch);
    }

    if (IS_NPC(ch))
      SET_BIT(MOB_FLAGS(ch), MOB_MOVED);
  }
}


/* give an object to a char   */
void obj_to_char(struct obj_data * object, struct char_data * ch)
{
  if (object && ch) {
    object->next_content = ch->carrying;
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;

    /* set flag for crash-save system */
    if (!IS_NPC(ch))
       SET_BIT(PLR_FLAGS(ch), PLR_CRASH);
    else
      SET_BIT(MOB_FLAGS(ch), MOB_CRASH);

  } else
    log("SYSERR: NULL obj (%p) or char (%p) passed to obj_to_char", object, ch);
}


/* take an object from a char */
void obj_from_char(struct obj_data * object)
{
  struct obj_data *temp;

  if (object == NULL) {
    log("SYSERR: NULL object passed to obj_from_char");
    return;
  }

  if (object->carried_by == NULL) {
    log("SYSERR: Invalid character in obj_from_char");
    return;
  }

  REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

  IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(object->carried_by)--;
  object->carried_by = NULL;
  object->next_content = NULL;
  if (!OBJ_FLAGGED(object, ITEM_UNLOAD)) {
    if (object->contains)
      for (temp = object->contains; temp; temp = temp->next_content)
        sql_remove_obj(temp);
    sql_remove_obj(object);
  }
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data * ch, int eq_pos)
{
  int factor;

  assert(GET_EQ(ch, eq_pos));

  if (!(GET_OBJ_TYPE(GET_EQ(ch, eq_pos)) == ITEM_ARMOR))
    return 0;

  switch (eq_pos) {

  case WEAR_BODY:
    factor = 3;
    break;			/* 30% */
  case WEAR_HEAD:
    factor = 2;
    break;			/* 20% */
  case WEAR_LEGS:
    factor = 2;
    break;			/* 20% */
  default:
    factor = 1;
    break;			/* all others 10% */
  }

  return (factor * GET_OBJ_VAL(GET_EQ(ch, eq_pos), 0));
}

#define PESTILENCE_VNUM 7907

void
check_for_bad_stats(struct char_data * ch)
{
	char bad_char = 0;

	if (GET_STR(ch) == 0) bad_char = 's';
	if (GET_INT(ch) == 0) bad_char = 'i';
	if (GET_WIS(ch) == 0) bad_char = 'w';
	if (GET_CHA(ch) == 0) bad_char = 'c';
	if (GET_DEX(ch) == 0) bad_char = 'd';
	
	switch (bad_char)
	{
	case 0: break;
	case 's': 
		send_to_char("You are too weak to fight!\n\r", ch);
		break;
	case 'i':
		send_to_char("You are too dumb to do much of anything!\n\r",ch);
		break;
	case 'w':
		send_to_char("You are too dumb to do much of anything!\n\r",ch);
		break;
	case 'c':
		send_to_char("The world hates you!\n\r", ch);
		if (!HUNTING( get_char_num(real_mobile(PESTILENCE_VNUM))))
		  set_hunting( get_char_num(real_mobile(PESTILENCE_VNUM)),ch);
		break;
	case 'd':
		send_to_char("You trip over your own feet and hit your "
				"head!\n\r",ch);
		damage(ch, ch, 40, TYPE_SUFFERING);  
		break;
	default: 
	    log("error (handler.c): illegal stat in check_for_bad_stats()");
		break;
	}
}


void 
equip_char(struct char_data * ch, struct obj_data * obj, int pos)
{
  int j;
  char mybuf[256];

  assert(pos >= 0 && pos < NUM_WEARS);

  if (GET_EQ(ch, pos)) {
    log("SYSERR: Char is already equipped: %s, %s", GET_NAME(ch), obj->short_description);
    extract_obj(obj);
    return;
  }
  if (obj->carried_by) {
    log("SYSERR: EQUIP: Obj is carried_by when equip.");
    return;
  }
  if (obj->in_room != NOWHERE) {
    log("SYSERR: EQUIP: Obj is in_room when equip.");
    return;
  }
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) )
  {
      act("You are zapped by $p and instantly let go of it.", 
		FALSE, ch, obj, 0, TO_CHAR);
      act("$n is zapped by $p and instantly lets go of it.", 
		FALSE, ch, obj, 0, TO_ROOM);
      obj_to_char(obj, ch);	/* changed to drop in inventory instead of
				 * ground */
      return;
  }

  if (IS_OBJ_STAT(obj, ITEM_TAKE_NAME))
  {
      int nr = GET_OBJ_RNUM(obj);
      sprintf(mybuf, "%s", obj->name);
      if (obj->short_description &&
	  obj->short_description != obj_proto[nr].short_description)
	FREE(obj->short_description);
      obj->short_description = tprintf("%s's %s", GET_NAME(ch), mybuf);
  }

  GET_EQ(ch, pos) = obj;
  obj->worn_by = ch;
  obj->worn_on = pos;

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) += apply_ac(ch, pos);

  if (ch->in_room != NOWHERE) {
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2) != 0)	/* if light is on, or infinite */
	world[ch->in_room].light++;
  }

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, TRUE);

  affect_total(ch);
  check_for_bad_stats(ch);
}



struct obj_data *unequip_char(struct char_data * ch, int pos)
{
  int j;
  struct obj_data *obj;

  assert(pos >= 0 && pos < NUM_WEARS);
  assert(GET_EQ(ch, pos));

  obj = GET_EQ(ch, pos);
  obj->worn_by = NULL;
  obj->worn_on = -1;

  if (IS_OBJ_STAT(obj, ITEM_TAKE_NAME))
  {
	if (obj->short_description)
	  FREE(obj->short_description);
	obj->short_description = tprintf("%s %s", AN(obj->name), obj->name);
  }

  if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) -= apply_ac(ch, pos);

  if (ch->in_room != NOWHERE) {
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      if (GET_OBJ_VAL(obj, 2) != 0)	/* if light is on, or infinite */
        if (world[ch->in_room].light)
          world[ch->in_room].light--;
  }

  GET_EQ(ch, pos) = NULL;

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, FALSE);

  affect_total(ch);

  return (obj);
}


int get_number(char **name)
{
  int i;
  char *ppos;
  char number[MAX_INPUT_LENGTH];

  *number = '\0';

  if ((ppos = strchr(*name, '.'))) {
    *ppos++ = '\0';
    strcpy(number, *name);
    strcpy(*name, ppos);

    for (i = 0; *(number + i); i++)
      if (!isdigit(*(number + i)))
	return 0;

    return (atoi(number));
  }
  return 1;
}

/* basically this function returns true if the PC has a ITEM_LIGHT
   worn anywhere, and false if not -rparet 01271998 */

int has_light(struct char_data *ch)
{
  int pos = 0;
  for(pos = 0; pos < NUM_WEARS; pos++) {
    if(GET_EQ(ch,pos))
      if (GET_OBJ_TYPE(GET_EQ(ch, pos)) == ITEM_LIGHT)
        if (GET_OBJ_VAL(GET_EQ(ch, pos), 2) != 0) /* Light is on, or infinite */
          return TRUE;
  }
  return FALSE;
}


/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data * list)
{
  struct obj_data *i;

  for (i = list; i; i = i->next_content)
    if (GET_OBJ_RNUM(i) == num)
      return i;

  return NULL;
}



/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
  struct obj_data *i;

  for (i = object_list; i; i = i->next)
    if (GET_OBJ_RNUM(i) == nr)
      return i;

  return NULL;
}



/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, long room)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = world[room].people; i && (j <= number); i = i->next_in_room)
    if (isname_with_abbrevs(tmp, i->player.name))
      if (++j == number)
	return i;

  return NULL;
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (IS_NPC(i))
      if (GET_MOB_RNUM(i) == nr)
        return i;

  return NULL;
}



/* put an object in a room */
void obj_to_room(struct obj_data * object, long room)
{
  if (!object || room < 0 || room > top_of_world) {
    log("SYSERR: Illegal value(s) passed to obj_to_room");
  }
  else {
    object->next_content = world[room].contents;
    world[room].contents = object;
    if (GET_OBJ_TYPE(object) == ITEM_LIGHT)
      world[room].light++;
    object->in_room = room;
    object->carried_by = NULL;
    object->garbage = 0;   /* garbage collection */
    SET_BIT(ROOM_FLAGS(room), ROOM_HOUSE_CRASH);
  }
}


/* Take an object from a room */
void 
obj_from_room(struct obj_data * object)
{
  struct obj_data *temp;

  if (!object || object->in_room == NOWHERE) {
    log("SYSERR: NULL object or obj not in a room passed to obj_from_room");
    return;
  }

  REMOVE_FROM_LIST(object, world[object->in_room].contents, next_content);

  if ((GET_OBJ_TYPE(object) == ITEM_LIGHT) && world[object->in_room].light)
    world[object->in_room].light--;
  object->in_room = NOWHERE;
  object->next_content = NULL;
  object->garbage = 0;  /* reset garbage collection counter */
  if (!OBJ_FLAGGED(object, ITEM_UNLOAD)) {
    if (object->contains)
      for (temp = object->contains; temp; temp = temp->next_content)
        sql_remove_obj(temp);
    sql_remove_obj(object);
  }
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data * obj, struct obj_data * obj_to)
{
  struct obj_data *tmp_obj;

  if (!obj || !obj_to || obj == obj_to) {
    log("SYSERR: NULL object or same source and target obj passed to obj_to_obj");
    return;
  }

  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;

  if (IN_ROOM(obj_to) != NOWHERE)
    SET_BIT(ROOM_FLAGS(IN_ROOM(obj_to)), ROOM_HOUSE_CRASH);

  for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

  /* top level object.  Subtract weight from inventory if necessary. */
  GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
  if (tmp_obj->carried_by)
    IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
}


/* remove an object from an object */
void obj_from_obj(struct obj_data * obj)
{
  struct obj_data *temp, *obj_from;

  if (obj->in_obj == NULL) {
    log("error (handler.c): trying to illegally extract obj from obj");
    return;
  }
  obj_from = obj->in_obj;
  REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

  /* Subtract weight from containers container */
  for (temp = obj->in_obj; temp->in_obj; temp = temp->in_obj)
    {
      GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
      if (GET_OBJ_WEIGHT(temp)<1) /*sanity check Serapis 970616*/
	GET_OBJ_WEIGHT(temp)=1;
    }

  /* Subtract weight from char that carries the object */
  GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
  if (GET_OBJ_WEIGHT(temp)<1) /*sanity check Serapis 970616*/
    GET_OBJ_WEIGHT(temp)=1;
  if (temp->carried_by)
    IS_CARRYING_W(temp->carried_by) -= GET_OBJ_WEIGHT(obj);

  obj->in_obj = NULL;
  obj->next_content = NULL;
  if (!OBJ_FLAGGED(obj, ITEM_UNLOAD))
    sql_remove_obj(obj);
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data * list, struct char_data * ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}


/* Extract an object from the world */
void 
extract_obj(struct obj_data * obj)
{
  struct obj_data *temp;

  if (obj->worn_by != NULL)
    if (unequip_char(obj->worn_by, obj->worn_on) != obj)
      log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->in_obj)
    obj_from_obj(obj);

  /* Get rid of the contents of the object, as well. */
  while (obj->contains)
    extract_obj(obj->contains);

  REMOVE_FROM_LIST(obj, object_list, next);

  /* This is done in each obj_from_XXXX, but just in case we missed something */
  if (!OBJ_FLAGGED(obj, ITEM_UNLOAD))
    sql_remove_obj(obj);
  free_obj(obj);
}



void update_object(struct obj_data * obj, int use)
{
  if ((GET_OBJ_TIMER(obj) > 0) && (GET_OBJ_TYPE(obj) != ITEM_KEY))
    GET_OBJ_TIMER(obj) -= use;
  if (obj->contains)
    update_object(obj->contains, use);
  if (obj->next_content)
    update_object(obj->next_content, use);
}


void update_char_objects(struct char_data * ch)
{
  int i;
  int j;
  int affec = FALSE;

  if (GET_EQ(ch, WEAR_HOLD) != NULL)
    if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_LIGHT)
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2) > 0) {
	i = --GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2);
	if (i == 1) {
	  act("Your light begins to flicker and fade.", 
		FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light begins to flicker and fade.", 
		FALSE, ch, 0, 0, TO_ROOM);
	} else if (i == 0) {
	  act("Your light sputters out and dies.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n's light sputters out and dies.", FALSE, ch, 0, 0, TO_ROOM);
          if (world[ch->in_room].light)
       	    world[ch->in_room].light--;
 	  for (j = 0; j < MAX_OBJ_AFFECT; j++)
             if (GET_EQ(ch, WEAR_HOLD)->affected[j].location!=APPLY_NONE)
		affec = TRUE;
	  if (!affec)
	  {
	    send_to_char("It crumbles into dust.\r\n", ch);
	    extract_obj(unequip_char(ch, WEAR_HOLD));
	  }
	}
      }

  /* depricated - timers are handled by limits.c - mcooper 20051118
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      update_object(GET_EQ(ch, i), 2);

  if (ch->carrying)
    update_object(ch->carrying, 1); */
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char_final(struct char_data * ch)
{
  struct char_data *k, *temp;
  struct descriptor_data *t_desc;
  struct obj_data *obj;
  int i, freed = 0;

  extern struct char_data *combat_list;

  ACMD(do_guard);
  ACMD(do_return);
  void sql_remove_mob(struct char_data *mob);
  void die_follower(struct char_data * ch);

  if (!IS_NPC(ch) && !ch->desc) {
    for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
      if (t_desc->original == ch)
	do_return(t_desc->character, "", 0, 0);
  }
  if (ch->in_room == NOWHERE) {
    log("SYSERR: NOWHERE extracting char. (handler.c, extract_char)");
    exit(1);
  }
  if (ch->followers || ch->master)
    die_follower(ch);

  /* stop any guarding */
  for (k = character_list; k; k = k->next)
    if (GUARDING(k) == ch)
      do_guard(k, "self", 0, 0);
  if (ch->guarded_by)
    do_guard(ch->guarded_by, "self", 0, 0);

  /* Forget snooping, if applicable */
  if (ch->desc) {
    if (ch->desc->snooping) {
      ch->desc->snooping->snoop_by = NULL;
      ch->desc->snooping = NULL;
    }
    if (ch->desc->snoop_by) {
      SEND_TO_Q("Your victim is no longer among us.\r\n",
		ch->desc->snoop_by);
      ch->desc->snoop_by->snooping = NULL;
      ch->desc->snoop_by = NULL;
    }
  }
  /* transfer objects to room, if any */
  while (ch->carrying) {
    obj = ch->carrying;
    if (OBJ_FLAGGED(obj, ITEM_UNLOAD)) {
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
    } else
      extract_obj(obj);
  }

  /* transfer equipment to room, if any */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i)) {
      if (OBJ_FLAGGED(GET_EQ(ch, i), ITEM_UNLOAD))
        obj_to_room(unequip_char(ch, i), ch->in_room);
      else
        extract_obj(GET_EQ(ch, i));
    }

  if (FIGHTING(ch))
    stop_fighting(ch);

  for (k = combat_list; k; k = temp) {
    temp = k->next_fighting;
    if (FIGHTING(k) == ch)
      stop_fighting(k);
  }

  char_from_room(ch);

  /* pull the char from the list */
  REMOVE_FROM_LIST(ch, character_list, next);

  if (ch->desc && ch->desc->original)
    do_return(ch, "", 0, 0);

  if (IS_NPC(ch)) {
    clearMemory(ch);
    if (!MOB_FLAGGED(ch, MOB_UNLOAD))
      sql_remove_mob(ch);
    free_char(ch);
    freed = 1;
  } else
    save_char(ch, NOWHERE);

  if (!freed && ch->desc != NULL) {
    STATE(ch->desc) = CON_RMOTD;
    SEND_TO_Q("Press RETURN to continue.\r\n", ch->desc);
  } else {  /* if a player gets purged from within the game */
    if (!freed)
      free_char(ch);
  }
}

/*
 * Q: Why do we do this?
 * A: Because trying to iterate over the character
 *    list with 'ch = ch->next' does bad things if
 *    the current character happens to die. The
 *    trivial workaround of 'vict = next_vict'
 *    doesn't work if the _next_ person in the list
 *    gets killed, for example, by an area spell.
 *
 * Q: Why do we leave them on the character_list?
 * A: Because code doing 'vict = vict->next' would
 *    get really confused otherwise.
 */
void extract_char(struct char_data *ch)
{
  if (IS_NPC(ch)) {
    if (IS_SET(MOB_FLAGS(ch), MOB_EXTRACT))  /* Already set */
      return;
    else
      SET_BIT(MOB_FLAGS(ch), MOB_EXTRACT);
  } else {
    if (IS_SET(PLR_FLAGS(ch), PLR_EXTRACT)) /* Already set */
      return;
    else
      SET_BIT(PLR_FLAGS(ch), PLR_EXTRACT);
  }

  extractions_pending++;
}

/*
 * I'm not particularly pleased with the MOB/PLR
 * hoops that have to be jumped through but it
 * hardly calls for a completely new variable.
 * Ideally it would be its own list, but that
 * would change the '->next' pointer, potentially
 * confusing some code. Ugh. -gg 3/15/2001
 *
 * NOTE: This doesn't handle recursive extractions.
 */
void extract_pending_chars(void)
{
  struct char_data *vict, *next_vict, *prev_vict;

  if (extractions_pending < 0)
    log("SYSERR: Negative (%d) extractions pending.", extractions_pending);

  for (vict = character_list, prev_vict = NULL; vict && extractions_pending; vict = next_vict) {
    next_vict = vict->next;

    if (MOB_FLAGGED(vict, MOB_EXTRACT))
      REMOVE_BIT(MOB_FLAGS(vict), MOB_EXTRACT);
    else if (PLR_FLAGGED(vict, PLR_EXTRACT))
      REMOVE_BIT(PLR_FLAGS(vict), PLR_EXTRACT);
    else {
      /* Last non-free'd character to continue chain from. */
      prev_vict = vict;
      continue;
    }

    extract_char_final(vict);
    extractions_pending--;

    if (prev_vict)
      prev_vict->next = next_vict;
    else
      character_list = next_vict;
  }

  if (extractions_pending > 0)
    log("SYSERR: Couldn't find %d extractions as counted.", extractions_pending);

  extractions_pending = 0;
}

/* ***********************************************************************
* Here follows high-level versions of some earlier routines, ie functions*
* which incorporate the actual player-data                               *.
*********************************************************************** */


struct char_data *get_player_vis(struct char_data * ch, char *name, int inroom)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (!IS_NPC(i) && (!inroom || i->in_room == ch->in_room) &&
	!str_cmp(i->player.name, name) && CAN_SEE(ch, i))
      return i;

  return NULL;
}


struct char_data *
get_char_room_vis(struct char_data * ch, char *name)
{
  struct char_data *i = NULL;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* JE 7/18/94 :-) :-) */
  if (!str_cmp(name, "self") || !str_cmp(name, "me"))
    return ch;

  /* 0.<name> means PC with name */
  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_vis(ch, tmp, 1);

  for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
    if (isname_with_abbrevs(tmp, i->player.name))
      if (CAN_SEE(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}


struct char_data *get_char_vis(struct char_data * ch, char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* check the room first */
  if ((i = get_char_room_vis(ch, name)) != NULL)
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_vis(ch, tmp, 0);

  for (i = character_list; i && (j <= number); i = i->next)
    if (isname(tmp, i->player.name) && CAN_SEE(ch, i))
      if (++j == number)
	return i;

  return NULL;
}



struct obj_data *get_obj_in_list_vis(struct char_data * ch, char *name,
				              struct obj_data * list)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  for (i = list; i && (j <= number); i = i->next_content)
    if (isname_with_abbrevs(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i) || GET_OBJ_TYPE(i) == ITEM_LIGHT)
	if (++j == number)
	  return i;

  return NULL;
}




/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data * ch, char *name)
{
  struct obj_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
    return i;

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
    return i;

  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return NULL;

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; i && (j <= number); i = i->next)
    if (isname_with_abbrevs(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i))
	if (++j == number)
	  return i;

  return NULL;
}



struct obj_data *get_object_in_equip_vis(struct char_data * ch,
		           char *arg, struct obj_data * equipment[], int *j)
{
  for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	if (isname_with_abbrevs(arg, equipment[(*j)]->name))
	  return (equipment[(*j)]);

  return NULL;
}


char *money_desc(int amount)
{
  static char buf[128];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }
  if (amount == 1)
    strcpy(buf, "a gold coin");
  else if (amount <= 10)
    strcpy(buf, "a tiny pile of gold coins");
  else if (amount <= 20)
    strcpy(buf, "a handful of gold coins");
  else if (amount <= 75)
    strcpy(buf, "a little pile of gold coins");
  else if (amount <= 200)
    strcpy(buf, "a small pile of gold coins");
  else if (amount <= 1000)
    strcpy(buf, "a pile of gold coins");
  else if (amount <= 5000)
    strcpy(buf, "a big pile of gold coins");
  else if (amount <= 10000)
    strcpy(buf, "a large heap of gold coins");
  else if (amount <= 20000)
    strcpy(buf, "a huge mound of gold coins");
  else if (amount <= 75000)
    strcpy(buf, "an enormous mound of gold coins");
  else if (amount <= 150000)
    strcpy(buf, "a small mountain of gold coins");
  else if (amount <= 250000)
    strcpy(buf, "a mountain of gold coins");
  else if (amount <= 500000)
    strcpy(buf, "a huge mountain of gold coins");
  else if (amount <= 1000000)
    strcpy(buf, "an enormous mountain of gold coins");
  else
    strcpy(buf, "an absolutely colossal mountain of gold coins");

  return buf;
}


struct obj_data *create_money(int amount)
{
  struct obj_data *obj;
  struct extra_descr_data *new_descr;
  char buf[200];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money.");
    return NULL;
  }

  if (!(obj = read_object(1224, VIRTUAL, NULL))) { /* Read the coin proto */
    sprintf(buf, "SYSERR: Unable to create money prototype.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return NULL;
  }

  CREATE(new_descr, struct extra_descr_data, 1);

  if (amount == 1) {
    obj->name = str_dup("coin gold");
    obj->short_description = str_dup("a gold coin");
    obj->description = str_dup("One miserable gold coin is lying here.");
    new_descr->keyword = str_dup("coin gold");
    new_descr->description = str_dup("It's just one miserable little gold coin.");
  } else {
    obj->name = str_dup("coins gold");
    obj->short_description = str_dup(money_desc(amount));
    sprintf(buf, "%s is lying here.", money_desc(amount));
    obj->description = str_dup(CAP(buf));

    new_descr->keyword = str_dup("coins gold");
    if (amount < 10) {
      sprintf(buf, "There are %d coins.", amount);
      new_descr->description = str_dup(buf);
    } else if (amount < 100) {
      sprintf(buf, "There are about %d coins.", 10 * (amount / 10));
      new_descr->description = str_dup(buf);
    } else if (amount < 1000) {
      sprintf(buf, "It looks to be about %d coins.", 100 * (amount / 100));
      new_descr->description = str_dup(buf);
    } else if (amount < 100000) {
      sprintf(buf, "You guess there are, maybe, %d coins.",
	      1000 * ((amount / 1000) + number(0, (amount / 1000))));
      new_descr->description = str_dup(buf);
    } else
      new_descr->description = str_dup("There are a LOT of coins.");
  }

  new_descr->next = NULL;
  obj->ex_description = new_descr;

  GET_OBJ_VAL(obj, 0) = amount;
  GET_OBJ_COST(obj) = amount;

  return obj;
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int 
generic_find(char *arg, bitvector_t bitvector, struct char_data * ch,
		     struct char_data ** tar_ch, struct obj_data ** tar_obj)
{
  int i, found;
  char name[256];

  one_argument(arg, name);

  if (!*name)
    return (0);

  *tar_ch = NULL;
  *tar_obj = NULL;

  if (IS_SET(bitvector, FIND_CHAR_ROOM)) {	/* Find person in room */
    if ((*tar_ch = get_char_room_vis(ch, name))) {
      return (FIND_CHAR_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*tar_ch = get_char_vis(ch, name))) {
      return (FIND_CHAR_WORLD);
    }
  }

  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
      return (FIND_OBJ_INV);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
      if (GET_EQ(ch, i) && isname_with_abbrevs(name, GET_EQ(ch, i)->name)== 1) {
	      *tar_obj = GET_EQ(ch, i);
	      found = TRUE;
      }
    if (found) {
      return (FIND_OBJ_EQUIP);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) {
      return (FIND_OBJ_ROOM);
    }
  }
  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*tar_obj = get_obj_vis(ch, name))) {
      return (FIND_OBJ_WORLD);
    }
  }
  return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg)
{
  if (!strcmp(arg, "all"))
    return FIND_ALL;
  else if (!strncmp(arg, "all.", 4)) {
    strcpy(arg, arg + 4);
    return FIND_ALLDOT;
  } else
    return FIND_INDIV;
}

struct char_data *get_player_hidden(struct char_data * ch, 
					char *name, int inroom)
{
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    if (!IS_NPC(i) && (!inroom || i->in_room == ch->in_room) &&
	!str_cmp(i->player.name, name) && 
	(CAN_SEE(ch, i)||IS_AFFECTED(i, AFF_HIDE)))
      return i;

  return NULL;
}


struct char_data *get_char_room_hidden(struct char_data * ch, char *name)
{
  struct char_data *i;
  int j = 0, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;

  /* JE 7/18/94 :-) :-) */
  if (!str_cmp(name, "self") || !str_cmp(name, "me"))
    return ch;

  /* 0.<name> means PC with name */
  strcpy(tmp, name);
  if (!(number = get_number(&tmp)))
    return get_player_hidden(ch, tmp, 1);

  for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room)
    if (isname_with_abbrevs(tmp, i->player.name))
      if (CAN_SEE(ch, i)||IS_AFFECTED(i, AFF_HIDE))
	if (++j == number)
	  return i;

  return NULL;
}

