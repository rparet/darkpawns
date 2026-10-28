/* ************************************************************************
*   File: objsave.c                                     Part of CircleMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
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

/* $Id: objsave.c 1276 2006-04-03 04:58:16Z rparet $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

/* External variables */
extern struct str_app_type str_app[];

/* Local functions */
void Mob_crashsave(struct char_data * mob);
long gc_objcount = 0;

/* Extern functions */
void sql_add_obj(struct obj_file_elem object);
void sql_add_container(struct obj_data *obj);
void sql_heap_table(struct char_data *ch);
void sql_remove_heap(struct char_data *ch);
void sql_rent_write(long id, ubyte code);
void sql_write_notes(char *name, char *arg, struct char_data *vict);

int Obj_to_store_from(struct obj_data *obj, int locate)
{
  int i, rnum = GET_OBJ_RNUM(obj);
  struct obj_file_elem object;
  struct obj_data *temp;

  /* Checking this here allows correct container weight handling */
  if (IS_OBJ_STAT(obj, ITEM_NOPERSIST))
    return (TRUE);

  /* Wipe the object clean */
  clear_sql_object(&object);

  /* Determine which table this object should be saved in, 'houseobjs' or 'mobobjs' */
  if (obj->in_room != NOWHERE)
    object.house = GET_ROOM_VNUM(obj->in_room);
  else if (obj->in_obj) {
    temp = obj->in_obj;
    while (temp->in_obj)          /* Let's go back to the base object */
      temp = temp->in_obj;
    if (temp->in_room != NOWHERE) /* The object is sitting in a room */
      object.house = GET_ROOM_VNUM(temp->in_room);
    else if (temp->carried_by)    /* The object is either worn, or in an inventory */
      object.owner = temp->carried_by;
    else
      object.owner = temp->worn_by;
  } else if (obj->carried_by)
    object.owner = obj->carried_by;
  else if (obj->worn_by)
    object.owner = obj->worn_by;

  /* Numerical values */
  object.id = obj->id;
  object.item_number = GET_OBJ_VNUM(obj);
  object.locate = locate;

  if (IS_CORPSE(obj)) {
    if (!obj->contains)
      return (FALSE);	      /* We don't need to save it if it has nothing */
    object.locate = -2;       /* ensures we can reload the corpse with eq */
    object.weight = GET_OBJ_WEIGHT(obj);
    object.timer = GET_OBJ_TIMER(obj);
    for (i = 0; i < 5; i++)
      object.value[i] = GET_OBJ_VAL(obj, i);
    strcpy(object.name, obj->name);
    strcpy(object.desc, obj->description);
    strcpy(object.shortd, obj->short_description);
    object.altered = TRUE;
    sql_add_obj(object);
    return (TRUE);
  }

  /* If this object is within another, we'll store its container details */
  if (locate && obj->in_obj)
    sql_add_container(obj);

  if (rnum == NOTHING)
    return (FALSE);

  /* Determine if this object differs from the proto */
  if ((GET_OBJ_WEIGHT(obj) != obj_proto[rnum].obj_flags.weight) ||
      (GET_OBJ_TIMER(obj) != obj_proto[rnum].obj_flags.timer) ||
      (GET_OBJ_EXTRA(obj) != obj_proto[rnum].obj_flags.extra_flags) ||
      (GET_OBJ_WEAR(obj) != obj_proto[rnum].obj_flags.wear_flags) ||
      (GET_OBJ_BITV(obj) != obj_proto[rnum].obj_flags.bitvector) ||
      (GET_OBJ_COST(obj) != obj_proto[rnum].obj_flags.cost) ||
      (obj->name != obj_proto[rnum].name) ||
      (obj->description != obj_proto[rnum].description) ||
      (obj->short_description != obj_proto[rnum].short_description))
    object.altered = TRUE;

  for (i = 0; i < 5; i++)
    if (GET_OBJ_VAL(obj, i) != obj_proto[rnum].obj_flags.value[i])
      object.altered = TRUE;

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (obj->affected[i].location != obj_proto[rnum].affected[i].location)
      object.altered = TRUE;

  for (i = 0; i < MAX_OBJ_REQS; i++)
    if (obj->reqs[i].location != obj_proto[rnum].reqs[i].location)
      object.altered = TRUE;

  if (obj->ex_description)
    if (obj->ex_description != obj_proto[rnum].ex_description)
      object.altered = TRUE;

  if (obj->action_description)
    if (obj->action_description != obj_proto[rnum].action_description)
      object.altered = TRUE;

  /* If we've found a difference, we need to store all of the variables */
  if (object.altered) {
    object.weight = GET_OBJ_WEIGHT(obj);
    object.timer = GET_OBJ_TIMER(obj);
    object.extra_flags = GET_OBJ_EXTRA(obj);
    object.wear_flags = GET_OBJ_WEAR(obj);
    object.bitvector = GET_OBJ_BITV(obj);
    object.cost = GET_OBJ_COST(obj);

    for (i = 0; i < 5; i++)
      object.value[i] = GET_OBJ_VAL(obj, i);

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      object.affected[i] = obj->affected[i];

    for (i = 0; i < MAX_OBJ_REQS; i++)
      object.reqs[i] = obj->reqs[i];

    if (obj->name != obj_proto[rnum].name)
      strcpy (object.name, obj->name);

    if (obj->description != obj_proto[rnum].description)
      strcpy (object.desc, obj->description);

    if (obj->short_description != obj_proto[rnum].short_description)
      strcpy (object.shortd, obj->short_description);

    if (obj->ex_description != obj_proto[rnum].ex_description)
      object.extra_descr = obj->ex_description;

    if (obj->action_description != obj_proto[rnum].action_description)
      object.action_descr = obj->action_description;
  }

  sql_add_obj(object);
  return (TRUE);
}


struct obj_data *Obj_from_store_to(struct obj_file_elem object)
{
  struct obj_data *obj;
  int i;

  if (real_object(object.item_number) > NOTHING)
    obj = read_object(object.item_number, VIRTUAL, object.id);
  else if (strstr(object.name, "corpse")) {
    /* We've found a corpse, let's restore it */
    obj = create_obj();
    GET_OBJ_TYPE(obj) = ITEM_CONTAINER;
    FREE(obj->id);
    obj->id = object.id;
  } else
    return (NULL);

  if (!object.altered)
    return (obj);

  /* Numerical Values */
  GET_OBJ_WEIGHT(obj) = object.weight;
  GET_OBJ_TIMER(obj) = object.timer;
  GET_OBJ_COST(obj) = object.cost;

  for (i = 0; i < 5; i++)
    GET_OBJ_VAL(obj, i) = object.value[i];

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    obj->affected[i] = object.affected[i];

  for (i = 0; i < MAX_OBJ_REQS; i++)
    obj->reqs[i] = object.reqs[i];


  /* Bitvector values */
  GET_OBJ_EXTRA(obj) = object.extra_flags;
  GET_OBJ_WEAR(obj) = object.wear_flags;
  obj->obj_flags.bitvector = object.bitvector;


  /* String Values */
  if (*object.name)
    obj->name = str_dup(object.name);
    
  if (*object.desc)
    obj->description = str_dup(object.desc);
    
  if (*object.shortd)
    obj->short_description = str_dup(object.shortd);

  /* Extra descriptions */
  if (object.extra_descr)
    obj->ex_description = object.extra_descr;

  /* Action descriptions */
  if (object.action_descr)
    obj->action_description = object.action_descr;

  return (obj);
}

int Crash_save(struct obj_data *obj, sh_int locate)
{
  struct obj_data *tmp;
  int result;

  if (obj) {
    Crash_save(obj->next_content, locate);
    Crash_save(obj->contains, GET_OBJ_VNUM(obj));
    result = Obj_to_store_from(obj, locate);

    if (!result) {
      sprintf(buf, "SYSERR: Negative RESULT for '%s' in Crash_save.", obj->short_description);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      return (FALSE);
    }

    for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj) {
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
      if(GET_OBJ_WEIGHT(tmp) < 1)
        GET_OBJ_WEIGHT(tmp) = 1;
    }
  }
  return (TRUE);
}

void Crash_clean_file(long id)
{
  struct rent_info rent;
  struct char_data *cbuf;
  char filetype[20];
  extern ubyte rent_file_timeout, crash_file_timeout;
  struct rent_info sql_rent_read(long id);

  rent = sql_rent_read(id);
  if ((rent.rentcode == RENT_CRASH) || (rent.rentcode == RENT_FORCED)) {
    if (rent.time < time(0) - (crash_file_timeout * SECS_PER_REAL_DAY))
      if ((cbuf = load_char(get_name_by_id(id)))) {
        if (GET_LEVEL(cbuf) < LVL_IMMORT) {   /* Don't wipe Immortal rent files */
          sql_Crash_delete_file(id);
          switch (rent.rentcode) {
            case RENT_CRASH:
              strcpy(filetype, "crash");
              break;
            case RENT_FORCED:
              strcpy(filetype, "forced rent");
              break;
            default:
              strcpy(filetype, "UNKNOWN!");
              break;
          }
          sql_rent_write(id, RENT_INACTIVE);
          sprintf(buf, "  Deleting %s's %s file.", get_name_by_id(id), filetype);
          log(buf);
        }
        free_char(cbuf);
      }
    /* Must retrieve rented items w/in 30 days */
  } else if (rent.rentcode == RENT_RENTED) {
    if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY))
      if ((cbuf = load_char(get_name_by_id(id)))) {
        if (GET_LEVEL(cbuf) < LVL_IMMORT) {  /* Don't wipe Immortal rent files */
          sql_Crash_delete_file(id);
          sql_rent_write(id, RENT_INACTIVE);
          sprintf(buf, "  Deleting %s's rent file.", get_name_by_id(id));
          log(buf);
        }
        free_char(cbuf);
      }
    /* Must log on w/in another 30 days */
  } else if (rent.rentcode == RENT_INACTIVE)
    if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY)) {
      if ((cbuf = load_char(get_name_by_id(id)))) {
        sprintf(buf, "  Deleting inactive player %s.", get_name_by_id(id));
        log(buf);
        sql_delete_char(cbuf);
      }
      free_char(cbuf);
    }
}

void update_obj_file(void)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    Crash_clean_file((player_table + i)->id);
  return;
}


void Crash_listrent(struct char_data * ch, struct obj_file_elem object)
{
  struct obj_data *obj;
  *buf = '\0';

  if ((obj = Obj_from_store_to(object))) {
    sprintf(buf, "%s [%5d] (%5d%%) <%2d> %s\r\n", buf,
      GET_OBJ_VNUM(obj), GET_OBJ_LOAD(obj), object.locate, obj->short_description);
    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_UNLOAD);   /* to prevent it being deleted */
    extract_obj(obj);
  }

  page_string( ch->desc, buf, 1 );
}

/* so this is gonna be the auto equip (hopefully) */
void auto_equip(struct char_data *ch, struct obj_data *obj, int locate)
{
  int j;

  /* If it was broken on death, we don't want to continue wearing it */
  if (OBJ_FLAGGED(obj, ITEM_BROKEN) && locate > 0)
    locate = 0;

  if (locate > 0) { /* was worn */
    switch (j = locate-1) {
    case WEAR_LIGHT:
      break;
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
      if (!CAN_WEAR(obj,ITEM_WEAR_FINGER)) /* not fitting :( */
	locate = 0;
      break;
    case WEAR_NECK_1:
    case WEAR_NECK_2:
      if (!CAN_WEAR(obj,ITEM_WEAR_NECK))
	locate = 0;
      break;
    case WEAR_BODY:
      if (!CAN_WEAR(obj,ITEM_WEAR_BODY))
	locate = 0;
      break;
    case WEAR_HEAD:
      if (!CAN_WEAR(obj,ITEM_WEAR_HEAD))
	locate = 0;
      break;
    case WEAR_LEGS:
      if (!CAN_WEAR(obj,ITEM_WEAR_LEGS))
	locate = 0;
      break;
    case WEAR_FEET:
      if (!CAN_WEAR(obj,ITEM_WEAR_FEET))
	locate = 0;
      break;
    case WEAR_HANDS:
      if (!CAN_WEAR(obj,ITEM_WEAR_HANDS))
	locate = 0;
      break;
    case WEAR_ARMS:
      if (!CAN_WEAR(obj,ITEM_WEAR_ARMS))
	locate = 0;
      break;
    case WEAR_SHIELD:
      if (!CAN_WEAR(obj,ITEM_WEAR_SHIELD))
	locate = 0;
      break;
    case WEAR_ABOUT:
      if (!CAN_WEAR(obj,ITEM_WEAR_ABOUT))
	locate = 0;
      break;
    case WEAR_WAIST:
      if (!CAN_WEAR(obj,ITEM_WEAR_WAIST))
	locate = 0;
      break;
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
      if (!CAN_WEAR(obj,ITEM_WEAR_WRIST))
	locate = 0;
      break;
    case WEAR_WIELD:
      if (!CAN_WEAR(obj,ITEM_WEAR_WIELD))
	locate = 0;
      break;
    case WEAR_HOLD:
      if (!CAN_WEAR(obj,ITEM_WEAR_HOLD))
	locate = 0;
      break;
    case WEAR_ABLEGS:
      if (!CAN_WEAR(obj, ITEM_WEAR_ABLEGS))
         locate = 0;
      break;
    case WEAR_FACE:
      if (!CAN_WEAR(obj, ITEM_WEAR_FACE))
         locate = 0;
      break;
    case WEAR_HOVER:
      if (!CAN_WEAR(obj, ITEM_WEAR_HOVER))
         locate = 0;
      break;
    default:
      locate = 0;
    }

    if (locate > 0)
    {
      if (!GET_EQ(ch,j)) 
      {
        /* check ch's alignment to prevent $M from being zapped through auto-equip */
	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	    (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	    (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
	  locate = 0;
	else
	  equip_char(ch, obj, j);
      }
    }  else  /* oops - saved player with double equipment[j]? */
	locate = 0;
  }
  if (locate <= 0)
    obj_to_char(obj, ch);
}

void Crash_load(struct char_data * ch, struct obj_file_elem object)
{
  struct obj_data *obj, *obj1, *obj2;
  int locate, j;
  int sql_check_container(struct obj_data *obj, struct obj_data *cont);

  if ((obj = Obj_from_store_to(object))) {
    locate = object.locate;

    if (locate <= NUM_WEARS)
      auto_equip(ch, obj, locate);
    else {
      /* Assume it's in the inventory, look there first */
      for (obj1 = ch->carrying; obj1; obj1 = obj1->next_content)
        if (GET_OBJ_VNUM(obj1) == locate)
          if (sql_check_container(obj, obj1)) {
            obj_to_obj(obj, obj1);
            return;
          }

      /* Let's try a sub-level container (ie. container within container) */
      for (obj1 = ch->carrying; obj1; obj1 = obj1->next_content)
        for (obj2 = obj1->contains; obj2; obj2 = obj2->next_content)
          if (GET_OBJ_VNUM(obj2) == locate)
            if (sql_check_container(obj, obj2)) {
              obj_to_obj(obj, obj2);
              return;
            }

      /* Must be wearing it, put it in there instead */
      for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j)) {
          if (GET_OBJ_VNUM(GET_EQ(ch, j)) == locate)          /* Let's just see if it's the container itself */
            if (sql_check_container(obj, GET_EQ(ch, j))) {
              obj_to_obj(obj, GET_EQ(ch, j));
              return;
            }

          if (GET_EQ(ch, j)->contains)                        /* How about a container within a container */
            for (obj1 = GET_EQ(ch, j)->contains; obj1; obj1 = obj1->next_content)
              if (GET_OBJ_VNUM(obj1) == locate)
                if (sql_check_container(obj, obj1)) {
                  obj_to_obj(obj, obj1);
                  return;
                }
        }

      sprintf(buf, "Unable to locate container for '%s' during crash_load. Location was %d.",
        obj->short_description, locate);
      log(buf);
      /* sprintf(buf, "There was a problem loading '%s' from your inventory, contact a staff member.",
        obj->short_description);
      sql_write_notes("SYSTEM", buf, ch); */
      obj_to_char(obj, ch);   /* No container? Let's just give it to them */
    }
  }

  /* turn this into a crash file by re-writing the control block */
  sql_rent_write(GET_IDNUM(ch), RENT_CRASH);
}


void Crash_restore_weight(struct obj_data * obj)
{
  if (obj) {
    Crash_restore_weight(obj->contains);
    Crash_restore_weight(obj->next_content);
    if (obj->in_obj)
      GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
  }
}



void Crash_extract_objs(struct obj_data * obj)
{
  if (obj) {
    Crash_extract_objs(obj->contains);
    Crash_extract_objs(obj->next_content);
    extract_obj(obj);
  }
}


int Crash_is_unrentable(struct obj_data * obj)
{
  if (!obj)
    return 0;

  if (IS_OBJ_STAT(obj, ITEM_NORENT) || GET_OBJ_RNUM(obj) <= NOTHING ||
      GET_OBJ_TYPE(obj) == ITEM_KEY)
    return 1;

  return 0;
}


void Crash_extract_norents(struct obj_data * obj)
{
  if (obj) {
    Crash_extract_norents(obj->contains);
    Crash_extract_norents(obj->next_content);
    if (Crash_is_unrentable(obj))
      extract_obj(obj);
  }
}


/* get norent items from eq to inventory and
   extract norents out of worn containers */
void Crash_extract_norents_from_equipped(struct char_data * ch)
{
  int j;

  for (j = 0;j < NUM_WEARS;j++) {
    if (GET_EQ(ch,j)) {
      if (IS_OBJ_STAT(GET_EQ(ch,j), ITEM_NORENT) ||
	  GET_OBJ_RNUM(GET_EQ(ch,j)) <= NOTHING ||
	  GET_OBJ_TYPE(GET_EQ(ch,j)) == ITEM_KEY)
	obj_to_char(unequip_char(ch,j),ch);
      else
	Crash_extract_norents(GET_EQ(ch,j));
    }
  }
}


void Crash_crashsave(struct char_data * ch)
{
  int j;

  if (IS_NPC(ch))
    return;

  sql_rent_write(GET_IDNUM(ch), RENT_CRASH);

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), j+1))
        return;
      Crash_restore_weight(GET_EQ(ch,j));
    }

  if (!Crash_save(ch->carrying, 0))
    return;
  Crash_restore_weight(ch->carrying);

  REMOVE_BIT(PLR_FLAGS(ch), PLR_CRASH);
}


void Crash_rentsave(struct char_data * ch)
{
  int j;

  if (IS_NPC(ch))
    return;

  Crash_extract_norents_from_equipped(ch);
  Crash_extract_norents(ch->carrying);

  Crash_crashsave(ch);    /* This ensures the player has been saved firstly */
  sql_rent_write(GET_IDNUM(ch), RENT_RENTED);
  sql_heap_table(ch);

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), j+1))
        return;
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }

  if (!Crash_save(ch->carrying, 0))
    return;

  Crash_extract_objs(ch->carrying);
  sql_remove_heap(ch);
}


void Crash_cryosave(struct char_data * ch)
{
  int j;

  if (IS_NPC(ch))
    return;

  Crash_extract_norents_from_equipped(ch);
  Crash_extract_norents(ch->carrying);

  Crash_crashsave(ch);    /* This ensures the player has been saved firstly */
  sql_rent_write(GET_IDNUM(ch), RENT_CRYO);
  sql_heap_table(ch);

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), j+1))
        return;
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }

  if (!Crash_save(ch->carrying, 0))
    return;

  Crash_extract_objs(ch->carrying);
  SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
  sql_remove_heap(ch);
}


void Crash_save_all(void)
{
  struct descriptor_data *d;
  struct char_data *mob;

  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
	Crash_crashsave(d->character);
	save_char(d->character, NOWHERE);
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_CRASH);
      }
    }
  }

  for (mob = character_list; mob; mob = mob->next)
    if (IS_NPC(mob) && MOB_FLAGGED(mob, MOB_CRASH)) {
      Mob_crashsave(mob);
      REMOVE_BIT(MOB_FLAGS(mob), MOB_CRASH);
    }
}


void Mob_crashsave(struct char_data * mob)
{
  int j;

  if (!IS_NPC(mob)) {
    mudlog("-E- PC struct passed to Mob_crashsave!", BRF, LVL_IMMORT, TRUE);
    return; 
  }

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(mob,j)) {
      if (!Crash_save(GET_EQ(mob,j), j+1))
        return;
      Crash_restore_weight(GET_EQ(mob,j));
    }
 
  if (mob->carrying) {
    if (!Crash_save(mob->carrying, 0)) 
      return;
    Crash_restore_weight(mob->carrying);
  }
} 


void Mob_crashload(struct char_data * ch, struct obj_file_elem object)
{
  struct obj_data *obj, *obj1, *obj2;
  int locate, j;
  int sql_check_container(struct obj_data *obj, struct obj_data *cont);

  if (!IS_NPC(ch)) {
    mudlog("-E- PC struct passed to Mob_crashload!", BRF, LVL_IMMORT, TRUE);
    return;
  }

  if ((obj = Obj_from_store_to(object))) {
    locate = object.locate;

    if (locate <= NUM_WEARS)
      auto_equip(ch, obj, locate);
    else {
      /* Assume it's in the inventory, look there first */
      for (obj1 = ch->carrying; obj1; obj1 = obj1->next_content)
        if (GET_OBJ_VNUM(obj1) == locate)
          if (sql_check_container(obj, obj1)) {
            obj_to_obj(obj, obj1);
            return;
          }

      /* Let's try a sub-level container (ie. container within container */
      for (obj1 = ch->carrying; obj1; obj1 = obj1->next_content)
        for (obj2 = obj1->contains; obj2; obj2 = obj2->next_content)
          if (GET_OBJ_VNUM(obj2) == locate)
            if (sql_check_container(obj, obj2)) {
              obj_to_obj(obj, obj2);
              return;
            }

      /* Must be wearing it, put it in there instead */
      for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j))
          if (GET_OBJ_VNUM(GET_EQ(ch, j)) == locate)
            if (sql_check_container(obj, GET_EQ(ch, j))) {
              obj_to_obj(obj, GET_EQ(ch, j));
              return;
            }

      sprintf(buf, "Unable to locate container for '%s' during mob_crashload.",
        obj->short_description);
      log(buf);
      extract_obj(obj);
    }
  }
  return;
}

void garbage_collection()
{
  int i;
  struct obj_data *obj, *next_obj;

  for (obj = object_list; obj ; obj = next_obj) {
    next_obj = obj->next;

    if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) /* !take objs, like fountains, boards */
      continue;

    if (obj->in_room != NOWHERE && obj->garbage >= 2) {
      extract_obj(obj);
      gc_objcount++;
    } else if (obj->in_room != NOWHERE)
      (obj->garbage)++;
  }

  /* Clear out any DTs */
  for (i = 0; i < top_of_world; i++)
    if (ROOM_FLAGGED(i, ROOM_DEATH))
      if (world[i].contents)
        for (obj = world[i].contents; obj; obj = next_obj) {
          next_obj = obj->next_content;
          extract_obj(obj);
        }
}
