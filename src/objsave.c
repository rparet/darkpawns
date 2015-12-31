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

/* $Id: objsave.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

/* these factors should be unique integers */
#define RENT_FACTOR 	1
#define CRYO_FACTOR 	4

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int min_rent_cost;

/* Extern functions */
ACMD(do_action);

SPECIAL(receptionist);
SPECIAL(cryogenicist);


struct obj_data *Obj_from_store_to(struct obj_file_elem object, int *locate)
{
  struct obj_data *obj;
  int j, taeller;

  if (real_object(object.item_number) > -1) {
    obj = read_object(object.item_number, VIRTUAL);
    *locate = (int) object.locate;
    GET_OBJ_VAL(obj, 0) = object.value[0];
    GET_OBJ_VAL(obj, 1) = object.value[1];
    GET_OBJ_VAL(obj, 2) = object.value[2];
    GET_OBJ_VAL(obj, 3) = object.value[3];
    for(taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
      obj->obj_flags.extra_flags[taeller] = object.extra_flags[taeller];
    GET_OBJ_WEIGHT(obj) = object.weight;
    GET_OBJ_TIMER(obj) = object.timer;
    for(taeller = 0; taeller < AF_ARRAY_MAX; taeller++)
      obj->obj_flags.bitvector[taeller] = object.bitvector[taeller];

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
      obj->affected[j] = object.affected[j];

  /* do_string stuff */
    if (object.name != NULL)
      obj->name = str_dup (object.name);

    if (object.desc != NULL)
      obj->description = str_dup(object.desc);

    if (object.shortd != NULL)
      obj->short_description = str_dup(object.shortd);
  /*****end do_string stuff*****/


    return obj;
  } else
    return NULL;
}


/* this function used in house.c */
struct obj_data *Obj_from_store(struct obj_file_elem object)
{
  int locate;

  return Obj_from_store_to(object, &locate);
}


int Obj_to_store_from(struct obj_data * obj, FILE * fl, int locate)
{
  int j, taeller;
  struct obj_file_elem object;

  object.item_number = GET_OBJ_VNUM(obj);
  object.locate = (int) locate; /* where worn or inventory? */
  object.value[0] = GET_OBJ_VAL(obj, 0);
  object.value[1] = GET_OBJ_VAL(obj, 1);
  object.value[2] = GET_OBJ_VAL(obj, 2);
  object.value[3] = GET_OBJ_VAL(obj, 3);
  for(taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
    object.extra_flags[taeller] = obj->obj_flags.extra_flags[taeller];
  object.weight = GET_OBJ_WEIGHT(obj);
  object.timer = GET_OBJ_TIMER(obj);
  for(taeller = 0; taeller < AF_ARRAY_MAX; taeller++)
    object.bitvector[taeller] = obj->obj_flags.bitvector[taeller];
  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    object.affected[j] = obj->affected[j];

  /* do_string stuff */
  if (obj->name)
    strcpy (object.name, obj->name);
  else
    strcpy (object.name, "bug\0");
  if (obj->description)
    strcpy (object.desc, obj->description);
  else
    strcpy (object.desc,"\0");

  if (obj->short_description)
    strcpy (object.shortd, obj->short_description);
  else
    strcpy (object.shortd, "\0");
  /*****end do_string stuff*****/

  if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
    perror("Error writing object in Obj_to_store");
    return 0;
  }
  return 1;
}


int Obj_to_store(struct obj_data * obj, FILE * fl)
{
  return Obj_to_store_from(obj, fl, 0);
}

int Crash_delete_file(char *name)
{
  char filename[50];
  FILE *fl;

  if (!get_filename(name, filename, CRASH_FILE))
    return 0;
  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) {	/* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
      perror(buf1);
    }
    return 0;
  }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
      perror(buf1);
    }
  }
  return (1);
}


int Crash_delete_crashfile(struct char_data * ch)
{
  char fname[MAX_INPUT_LENGTH];
  struct rent_info rent;
  FILE *fl;

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 0;
  if (!(fl = fopen(fname, "rb"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: checking for crash file %s (3)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

  if (rent.rentcode == RENT_CRASH)
    Crash_delete_file(GET_NAME(ch));

  return 1;
}


int
Crash_clean_file(char *name)
{
  char fname[MAX_STRING_LENGTH], filetype[20];
  struct rent_info rent;
  extern int rent_file_timeout, crash_file_timeout;
  FILE *fl;

  if (!get_filename(name, fname, CRASH_FILE))
    return 0;
  /*
   * open for write so that permission problems will be flagged now, at boot
   * time.
   */
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: OPENING OBJECT FILE %s (4)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

  if ((rent.rentcode == RENT_CRASH) ||
      (rent.rentcode == RENT_FORCED) || (rent.rentcode == RENT_TIMEDOUT)) {
    if (rent.time < time(0) - (crash_file_timeout * SECS_PER_REAL_DAY)) {
      Crash_delete_file(name);
      switch (rent.rentcode) {
      case RENT_CRASH:
	strcpy(filetype, "crash");
	break;
      case RENT_FORCED:
	strcpy(filetype, "forced rent");
	break;
      case RENT_TIMEDOUT:
	strcpy(filetype, "idlesave");
	break;
      default:
	strcpy(filetype, "UNKNOWN!");
	break;
      }
      sprintf(buf, "    Deleting %s's %s file.", name, filetype);
      log(buf);
      return 1;
    }
    /* Must retrieve rented items w/in 30 days */
  } else if (rent.rentcode == RENT_RENTED)
    if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY)) {
      Crash_delete_file(name);
      sprintf(buf, "    Deleting %s's rent file.", name);
      log(buf);
      return 1;
    }
  return (0);
}



void
update_obj_file(void)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    Crash_clean_file((player_table + i)->name);
  return;
}



void Crash_listrent(struct char_data * ch, char *name)
{
  FILE *fl;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct obj_data *obj;
  struct rent_info rent;

  if (!get_filename(name, fname, CRASH_FILE))
    return;
  if (!(fl = fopen(fname, "rb"))) {
    sprintf(buf, "%s has no rent file.\r\n", name);
    send_to_char(buf, ch);
    return;
  }
  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  switch (rent.rentcode) {
  case RENT_RENTED:
    strcat(buf, "Rent\r\n");
    break;
  case RENT_CRASH:
    strcat(buf, "Crash\r\n");
    break;
  case RENT_CRYO:
    strcat(buf, "Cryo\r\n");
    break;
  case RENT_TIMEDOUT:
  case RENT_FORCED:
    strcat(buf, "TimedOut\r\n");
    break;
  default:
    strcat(buf, "Undef\r\n");
    break;
  }
  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      fclose(fl);
      return;
    }
    if (!feof(fl))
      if (real_object(object.item_number) > -1) {
	obj = read_object(object.item_number, VIRTUAL);
	sprintf(buf, "%s [%5d] (%.2fau) <%2d> %-20s\r\n", buf,
		object.item_number, GET_OBJ_LOAD(obj),
		object.locate, obj->short_description);
	extract_obj(obj);
      }
  }
  page_string( ch->desc, buf, 1 );
  fclose(fl);
}



int Crash_write_rentcode(struct char_data * ch, FILE * fl, struct rent_info * rent)
{
  if (fwrite(rent, sizeof(struct rent_info), 1, fl) < 1) {
    perror("Writing rent code.");
    return 0;
  }
  return 1;
}



/* so this is gonna be the auto equip (hopefully) */
void auto_equip(struct char_data *ch, struct obj_data *obj, int locate)
{
  int j;

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
      if (!CAN_WEAR(obj,ITEM_WEAR_HOLD) &&
	  !(IS_WARRIOR(ch) &&
	    CAN_WEAR(obj,ITEM_WEAR_WIELD) && GET_OBJ_TYPE(obj) == ITEM_WEAPON))
	locate = 0;
      break;
    case WEAR_THROW:
      if (!CAN_WEAR(obj, ITEM_WEAR_THROW))
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
    if (locate > 0) {
      if (!GET_EQ(ch,j)) {
/* check ch's alignment to prevent $M from being zapped through auto-equip */
	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	    (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	    (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
	  locate = 0;
	else
	  equip_char(ch, obj, j);
      }
      else  /* oops - saved player with double equipment[j]? */
	locate = 0;
    }
  }
  if (locate <= 0)
    obj_to_char(obj, ch);
}


#define MAX_BAG_ROW 5
/* should be enough - who would carry a bag in a bag in a bag in a
   bag in a bag in a bag ?!? */

int Crash_load(struct char_data * ch)
/* return values:
	0 - successful load, keep char in rent room.
	1 - load failure or load of crash items -- put char in temple.
	2 - rented equipment lost (no $)
*/
{
  void Crash_crashsave(struct char_data * ch);

  FILE *fl;
  char fname[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct rent_info rent;
  int cost, orig_rent_code;
  float num_of_days;
  struct obj_data *obj;
  int locate, j;
  struct obj_data *obj1;
  struct obj_data *cont_row[MAX_BAG_ROW];

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 1;
  if (!(fl = fopen(fname, "r+b")))
    {
      if (errno != ENOENT)
	{	/* if it fails, NOT because of no file */
	  sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
	  perror(buf1);
	  send_to_char("\r\n********************* NOTICE "
		       "*********************\r\n"
		       "There was a problem loading your objects from disk.\r\n"
		       "Contact a God for assistance.\r\n", ch);
	}
      sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      return 1;
    }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if (rent.rentcode == RENT_RENTED || rent.rentcode == RENT_TIMEDOUT)
    {
      num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
      cost = (int) (rent.net_cost_per_diem * num_of_days);
      if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch))
	{
	  fclose(fl);
	  sprintf(buf, "%s entering game, rented equipment lost (no $).",
		  GET_NAME(ch));
	  mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
	  Crash_crashsave(ch);
	  return 2;
	}
      else
	{
	  GET_BANK_GOLD(ch) -= MAX(cost - GET_GOLD(ch), 0);
	  GET_GOLD(ch) = MAX(GET_GOLD(ch) - cost, 0);
	  save_char(ch, NOWHERE);
	}
    }
  switch (orig_rent_code = rent.rentcode)
    {
    case RENT_RENTED:
      sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      break;
    case RENT_CRASH:
      sprintf(buf, "%s retrieving crash-saved items and entering game.",
	      GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      break;
    case RENT_CRYO:
      sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      break;
    case RENT_FORCED:
    case RENT_TIMEDOUT:
      sprintf(buf, "%s retrieving force-saved items and entering game.",
	      GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      break;
    default:
      sprintf(buf, "WARNING: %s entering game with undefined rent code.",
	      GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      break;
    }

  for (j = 0;j < MAX_BAG_ROW;j++)
    cont_row[j] = NULL; /* empty all cont lists (you never know ...) */

  while (!feof(fl))
    {
      fread(&object, sizeof(struct obj_file_elem), 1, fl);
      if (ferror(fl))
	{
	  perror("Reading crash file: Crash_load.");
	  fclose(fl);
	  return 1;
	}
      if (!feof(fl))
	if ((obj = Obj_from_store_to(object, &locate))) {
	  auto_equip(ch, obj, locate);

	  /*
	    what to do with a new loaded item:

	    if there's a list with <locate> less than 1 below this:
	    (equipped items are assumed to have <locate>==0 here) then its
	    container has disappeared from the file   *gasp*
	    -> put all the list back to ch's inventory

	    if there's a list of contents with <locate> 1 below this:
	    check if it's a container
	    - if so: get it from ch, fill it, and give it back to ch
	      (this way the container has its correct weight before
	       modifying ch)
	    - if not: the container is missing -> put all the list to ch's
	      inventory

	    for items with negative <locate>:
	    if there's already a list of contents with the same <locate>
	    put obj to it if not, start a new list

	    Confused? Well maybe you can think of some better text to be put
	    here ...

	    since <locate> for contents is < 0 the list indices are switched to
	    non-negative
	    */

	  if (locate > 0) { /* item equipped */
	    for (j = MAX_BAG_ROW-1;j > 0;j--)
	      if (cont_row[j]) { /* no container -> back to ch's inventory */
		for (;cont_row[j];cont_row[j] = obj1) {
		  obj1 = cont_row[j]->next_content;
		  obj_to_char(cont_row[j], ch);
		}
		cont_row[j] = NULL;
	      }
	    if (cont_row[0]) { /* content list existing */
	      if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
		/* rem item ; fill ; equip again */
		obj = unequip_char(ch, locate-1);
		obj->contains = NULL; /* should be empty - but who knows */
		for (;cont_row[0];cont_row[0] = obj1) {
		  obj1 = cont_row[0]->next_content;
		  obj_to_obj(cont_row[0], obj);
		}
		equip_char(ch, obj, locate-1);
	      } else { /* object isn't container -> empty content list */
		for (;cont_row[0];cont_row[0] = obj1) {
		  obj1 = cont_row[0]->next_content;
		  obj_to_char(cont_row[0], ch);
		}
		cont_row[0] = NULL;
	      }
	    }
	  } else { /* locate <= 0 */
	    for (j = MAX_BAG_ROW-1;j > -locate;j--)
	      if (cont_row[j]) { /* no container -> back to ch's inventory */
		for (;cont_row[j];cont_row[j] = obj1) {
		  obj1 = cont_row[j]->next_content;
		  obj_to_char(cont_row[j], ch);
		}
		cont_row[j] = NULL;
	      }

	    if (j == -locate && cont_row[j]) { /* content list existing */
	      if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
		/* take item ; fill ; give to char again */
		obj_from_char(obj);
		obj->contains = NULL;
		for (;cont_row[j];cont_row[j] = obj1) {
		  obj1 = cont_row[j]->next_content;
		  obj_to_obj(cont_row[j], obj);
		}
		obj_to_char(obj, ch); /* add to inv first ... */
	      } else { /* object isn't container -> empty content list */
		for (;cont_row[j];cont_row[j] = obj1) {
		  obj1 = cont_row[j]->next_content;
		  obj_to_char(cont_row[j], ch);
		}
		cont_row[j] = NULL;
	      }
	    }

	    if (locate < 0 && locate >= -MAX_BAG_ROW) {
	      /* let obj be part of content list
		 but put it at the list's end thus having the items
		 in the same order as before renting */
	      obj_from_char(obj);
	      if ((obj1 = cont_row[-locate-1])) {
		while (obj1->next_content)
		  obj1 = obj1->next_content;
		obj1->next_content = obj;
	      } else
		cont_row[-locate-1] = obj;
	    }
	  }
	}
    }

  /* turn this into a crash file by re-writing the control block */
  rent.rentcode = RENT_CRASH;
  rent.time = time(0);
  rewind(fl);
  Crash_write_rentcode(ch, fl, &rent);

  fclose(fl);

  if ((orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO))
    return 0;
  else
    return 1;
}


int Crash_save(struct obj_data * obj, FILE * fp, int locate)
{
  struct obj_data *tmp;
  int result;

  if (obj) {
    Crash_save(obj->next_content, fp, locate);
    Crash_save(obj->contains, fp, MIN(0,locate)-1);
    result = Obj_to_store_from(obj, fp, locate);

    for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
    {
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
      if(GET_OBJ_WEIGHT(tmp)<1) /*sanity check - Serapis 970616*/
	GET_OBJ_WEIGHT(tmp) = 1;
    }

    if (!result)
      return 0;
  }
  return TRUE;
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

  if (IS_OBJ_STAT(obj, ITEM_NORENT) || GET_OBJ_LOAD(obj) < 0 ||
      GET_OBJ_RNUM(obj) <= NOTHING || GET_OBJ_TYPE(obj) == ITEM_KEY)
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
	  GET_OBJ_LOAD(GET_EQ(ch,j)) < 0 ||
	  GET_OBJ_RNUM(GET_EQ(ch,j)) <= NOTHING ||
	  GET_OBJ_TYPE(GET_EQ(ch,j)) == ITEM_KEY)
	obj_to_char(unequip_char(ch,j),ch);
      else
	Crash_extract_norents(GET_EQ(ch,j));
    }
  }
}


void Crash_extract_expensive(struct obj_data * obj)
{
  struct obj_data *tobj, *max;

  max = obj;
  for (tobj = obj; tobj; tobj = tobj->next_content)
    if (GET_OBJ_LOAD(tobj) > GET_OBJ_LOAD(max))
      max = tobj;
  extract_obj(max);
}



void Crash_calculate_rent(struct obj_data * obj, int *cost)
{
  if (obj) {
    *cost += MAX(0, GET_OBJ_LOAD(obj));
    Crash_calculate_rent(obj->contains, cost);
    Crash_calculate_rent(obj->next_content, cost);
  }
}


void Crash_crashsave(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  rent.rentcode = RENT_CRASH;
  rent.time = time(0);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
    }

  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  Crash_restore_weight(ch->carrying);

  fclose(fp);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_CRASH);
}


void Crash_idlesave(struct char_data * ch)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  int cost, cost_eq;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  Crash_extract_norents_from_equipped(ch);

  Crash_extract_norents(ch->carrying);

  cost = 0;
  Crash_calculate_rent(ch->carrying, &cost);

  cost_eq = 0;
  for (j = 0; j < NUM_WEARS; j++)
    Crash_calculate_rent(GET_EQ(ch,j), &cost_eq);

  cost <<= 1;			/* forcerent cost is 2x normal rent */
  cost_eq <<= 1;

  if (cost+cost_eq > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
    for (j = 0; j < NUM_WEARS; j++) /* unequip player with low money */
      if (GET_EQ(ch,j))
	obj_to_char(unequip_char(ch, j), ch);
    cost += cost_eq;
    cost_eq = 0;

    while ((cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) && ch->carrying) {
      Crash_extract_expensive(ch->carrying);
      cost = 0;
      Crash_calculate_rent(ch->carrying, &cost);
      cost <<= 1;
    }
  }

  if (!ch->carrying) {
    for (j = 0; j < NUM_WEARS && !(GET_EQ(ch,j)); j++)
      ;
    if (j == NUM_WEARS) { /* no eq nor inv */
      fclose(fp);
      Crash_delete_file(GET_NAME(ch));
      return;
    }
  }
  rent.net_cost_per_diem = cost;

  rent.rentcode = RENT_TIMEDOUT;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }

  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void
Crash_rentsave(struct char_data * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  Crash_extract_norents_from_equipped(ch);

  Crash_extract_norents(ch->carrying);

  rent.net_cost_per_diem = cost;
  rent.rentcode = RENT_RENTED;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void Crash_cryosave(struct char_data * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  Crash_extract_norents_from_equipped(ch);

  Crash_extract_norents(ch->carrying);

  GET_GOLD(ch) = MAX(0, GET_GOLD(ch) - cost);

  rent.rentcode = RENT_CRYO;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  rent.net_cost_per_diem = 0;
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch,j)) {
      if (!Crash_save(GET_EQ(ch,j), fp, j+1)) {
	fclose(fp);
	return;
      }
      Crash_restore_weight(GET_EQ(ch,j));
      Crash_extract_objs(GET_EQ(ch,j));
    }
  if (!Crash_save(ch->carrying, fp, 0)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
  SET_BIT_AR(PLR_FLAGS(ch), PLR_CRYO);
}


/* ************************************************************************
* Routines used for the receptionist					  *
************************************************************************* */

void Crash_rent_deadline(struct char_data * ch, struct char_data * recep,
			      long cost)
{
  long rent_deadline;

  if (!cost)
    return;

  rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / cost);
  sprintf(buf,
      "$n tells you, 'You can rent for %ld day%s with the gold you have\r\n"
	  "on hand and in the bank.'\r\n",
	  rent_deadline, (rent_deadline > 1) ? "s" : "");
  act(buf, FALSE, recep, 0, ch, TO_VICT);
}

int Crash_report_unrentables(struct char_data * ch, struct char_data * recep,
			         struct obj_data * obj)
{
  char buf[128];
  int has_norents = 0;

  if (obj) {
    if (Crash_is_unrentable(obj)) {
      has_norents = 1;
      sprintf(buf, "$n tells you, 'You cannot store %s.'", OBJS(obj, ch));
      act(buf, FALSE, recep, 0, ch, TO_VICT);
    }
    has_norents += Crash_report_unrentables(ch, recep, obj->contains);
    has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
  }
  return (has_norents);
}



void Crash_report_rent(struct char_data * ch, struct char_data * recep,
		            struct obj_data * obj, long *cost, long *nitems, int display, int factor)
{
  static char buf[256];

  if (obj) {
    if (!Crash_is_unrentable(obj)) {
      (*nitems)++;
      *cost += MAX(0, (GET_OBJ_LOAD(obj) * factor));
      if (display) {
	sprintf(buf, "$n tells you, '%5d coins for %s..'",
		(((int)GET_OBJ_LOAD(obj)) * factor), OBJS(obj, ch));
	act(buf, FALSE, recep, 0, ch, TO_VICT);
      }
    }
    Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor);
    Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
  }
}



int Crash_offer_rent(struct char_data * ch, struct char_data * receptionist,
		         int display, int factor)
{
  extern int max_obj_save;	/* change in config.c */
  char buf[MAX_INPUT_LENGTH];
  int i;
  long totalcost = 0, numitems = 0, norent = 0;

  norent = Crash_report_unrentables(ch, receptionist, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    norent += Crash_report_unrentables(ch, receptionist, GET_EQ(ch,i));

  if (norent)
    return 0;

  totalcost = min_rent_cost * factor;

  Crash_report_rent(ch, receptionist, ch->carrying, &totalcost, &numitems, display, factor);

  for (i = 0; i < NUM_WEARS; i++)
    Crash_report_rent(ch, receptionist, GET_EQ(ch,i), &totalcost, &numitems, display, factor);

  if (!numitems) {
    act("$n tells you, 'But you are not carrying anything!  Just quit!'",
	FALSE, receptionist, 0, ch, TO_VICT);
    return (0);
  }
  if (numitems > max_obj_save) {
    sprintf(buf, "$n tells you, 'Sorry, but I cannot store more than %d items.'",
	    max_obj_save);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    return (0);
  }
  if (display) {
    sprintf(buf, "$n tells you, 'Plus, my %d coin fee..'",
	    min_rent_cost * factor);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    sprintf(buf, "$n tells you, 'For a total of %ld coins%s.'",
	    totalcost, (factor == RENT_FACTOR ? " per day" : ""));
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    if (totalcost > GET_GOLD(ch)) {
      act("$n tells you, '...which I see you can't afford.'",
	  FALSE, receptionist, 0, ch, TO_VICT);
      return (0);
    } else if (factor == RENT_FACTOR)
      Crash_rent_deadline(ch, receptionist, totalcost);
  }
  return (totalcost);
}



int gen_receptionist(struct char_data * ch, struct char_data * recep,
		         int cmd, char *arg, int mode)
{
  int cost = 0;
  extern int free_rent;
  int save_room;
  char *action_table[] = {"smile", "dance", "sigh", "blush", "burp",
  "cough", "fart", "twiddle", "yawn"};

  if (!ch->desc || IS_NPC(ch))
    return FALSE;

  if (!cmd && !number(0, 5)) {
    do_action(recep, "", find_command(action_table[number(0, 8)]), 0);
    return FALSE;
  }
  if (!CMD_IS("offer") && !CMD_IS("rent"))
    return FALSE;
  if (!AWAKE(recep)) {
    send_to_char("She is unable to talk to you...\r\n", ch);
    return TRUE;
  }
  if (!CAN_SEE(recep, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return TRUE;
  }
  if (free_rent) {
    act("$n tells you, 'Rent is free here.  Just quit, and your objects will be saved!'",
	FALSE, recep, 0, ch, TO_VICT);
    return 1;
  }
  if (CMD_IS("rent")) {
    if (!(cost = Crash_offer_rent(ch, recep, FALSE, mode)))
      return TRUE;
    if (mode == RENT_FACTOR)
      sprintf(buf, "$n tells you, 'Rent will cost you %d gold coins per day.'", cost);
    else if (mode == CRYO_FACTOR)
      sprintf(buf, "$n tells you, 'It will cost you %d gold coins to be frozen.'", cost);
    act(buf, FALSE, recep, 0, ch, TO_VICT);
    if (cost > GET_GOLD(ch)) {
      act("$n tells you, '...which I see you can't afford.'",
	  FALSE, recep, 0, ch, TO_VICT);
      return TRUE;
    }
    if (cost && (mode == RENT_FACTOR))
      Crash_rent_deadline(ch, recep, cost);

    if (mode == RENT_FACTOR) {
      act("$n stores your belongings and helps you into your private chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_rentsave(ch, cost);
      sprintf(buf, "%s has rented (%d/day, %d tot.)", GET_NAME(ch),
	      cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
    } else {			/* cryo */
      act("$n stores your belongings and helps you into your private chamber.\r\n"
	  "A white mist appears in the room, chilling you to the bone...\r\n"
	  "You begin to lose consciousness...",
	  FALSE, recep, 0, ch, TO_VICT);
      Crash_cryosave(ch, cost);
      sprintf(buf, "%s has cryo-rented.", GET_NAME(ch));
      SET_BIT_AR(PLR_FLAGS(ch), PLR_CRYO);
    }

    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
    save_room = ch->in_room;
    extract_char(ch);
    save_char(ch, save_room);
  } else {
    Crash_offer_rent(ch, recep, TRUE, mode);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  }
  return TRUE;
}


SPECIAL(receptionist)
{
  return (gen_receptionist(ch, me, cmd, argument, RENT_FACTOR));
}


SPECIAL(cryogenicist)
{
  return (gen_receptionist(ch, me, cmd, argument, CRYO_FACTOR));
}


void Crash_save_all(void)
{
  struct descriptor_data *d;
  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
	Crash_crashsave(d->character);
	save_char(d->character, NOWHERE);
	REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRASH);
      }
    }
  }
}


/* Blow away old alias files */
int Alias_delete_file(char *name)
{
  char filename[50];
  FILE *fl;

  if (!get_filename(name, filename, ALIAS_FILE))
    return 0;
  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) {      /* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting alias file %s (1)", filename);
      perror(buf1);
    }
    return 0;
  }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) {      /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting alias file %s (2)", filename);
      perror(buf1);
    }
  }
  return (1);
}
