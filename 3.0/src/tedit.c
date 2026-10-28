/* ************************************************************************
*   File: tedit.c                                       Part of CircleMUD *
*  Usage: Treasure editing system code.                                   *
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

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "olc.h"
#include "db.h"
#include "handler.h"

/* External variables */
extern int top_of_treasure;
extern struct tlist_data *treasure_list;

/* Function prototypes */
void tedit_disp_types(struct char_data *ch);
void tedit_new_list(struct char_data *ch, char *type);
void tedit_setup(struct descriptor_data *d, int tlist_num);
void tedit_save_internally(struct descriptor_data *d);
void tedit_save_to_disk(struct descriptor_data *d);
void tedit_delete_list(struct descriptor_data *d);
void tedit_disp_menu(struct descriptor_data *d);
void tedit_parse(struct descriptor_data * d, char *arg);

/*------------------------------------------------------------------------*\
  Utility and exported functions
\*------------------------------------------------------------------------*/

void tedit_disp_types(struct char_data *ch)
{
  int i;

  sprintf(buf, "Treasure Lists:\r\n");
  for (i = 0; i < top_of_treasure; i++)
    sprintf(buf + strlen(buf), "  %2d - %-20s (%2d items)\r\n",
      i+1, treasure_list[i].type, treasure_list[i].num_of_items);

  send_to_char(buf, ch);
}
/*------------------------------------------------------------------------*/

void tedit_new_list(struct char_data *ch, char *type)
{
  int i, j;
  struct tlist_data *tlist;

  /*. Make a new list in memory.*/
  CREATE(tlist, struct tlist_data, top_of_treasure + 1);

  type[0] = UPPER(type[0]);
  for (i = 0; i < top_of_treasure + 1; i++) {
    if (i == top_of_treasure) {           /* Add our new list here */
      tlist[i].type = str_dup(type);
      tlist[i].num_of_items = 0;
      CREATE(tlist[i].vnum, long, 1);
    } else {                              /* Copy exiting list */
      tlist[i].type = strdup(treasure_list[i].type);
      tlist[i].num_of_items = treasure_list[i].num_of_items;
      CREATE(tlist[i].vnum, long, tlist[i].num_of_items);
      for (j = 0; j < tlist[i].num_of_items; j++)
        tlist[i].vnum[j] = treasure_list[i].vnum[j];
    }
  }

  if (top_of_treasure) {                  /* A treasure list already exists, FREE it */
    for (i = 0; i < top_of_treasure ; i++)
      FREE(treasure_list[i].vnum);

    FREE(treasure_list);
  }

  treasure_list = tlist;

  sprintf(buf, "OLC: %s creates a new treasure list, '%s'", GET_NAME(ch), type);
  mudlog(buf, BRF, LVL_BUILDER, TRUE);
  send_to_char("Treasure list created.\r\n", ch);
  top_of_treasure++;
  olc_add_to_save_list(0, OLC_SAVE_TREASURE);
  return;
}
/*------------------------------------------------------------------------*/

void tedit_setup(struct descriptor_data *d, int tlist_num)
{
  int i;
  struct tlist_data *tlist;

  /* Allocate list */
  CREATE(tlist, struct tlist_data, 1);
  tlist->num_of_items = treasure_list[tlist_num].num_of_items;

  /* If this a new list, no need to allocate vnum space */
  if (tlist->num_of_items) {
    CREATE(tlist->vnum, long, tlist->num_of_items);

    /* Copy vnums over */
    for (i = 0; i < tlist->num_of_items; i++)
      tlist->vnum[i] = treasure_list[tlist_num].vnum[i];
  }

  /* Copy string over */
  if (treasure_list[tlist_num].type)
    tlist->type = str_dup(treasure_list[tlist_num].type);

  /* Attatch new list to players descriptor */
  OLC_TLIST(d) = tlist;
  OLC_VAL(d) = 0;
  tedit_disp_menu(d);
}

/*------------------------------------------------------------------------*/

void tedit_save_internally(struct descriptor_data *d)
{
  int i, j, len, found = FALSE;

  /* Locate the treasure list being edited */
  for (i = 0; i < top_of_treasure; i++) {
    len = strlen(OLC_TLIST(d)->type);
    if (!strncmp(OLC_TLIST(d)->type, treasure_list[i].type, len)) {
      found = TRUE;
      break;
    }
  }

  /* Was there a problem finding it? */
  if (!found) {
    sprintf(buf, "SYSERR: OLC: Cannot locate treasure list '%s' in memory!",
      OLC_TLIST(d)->type);
    mudlog(buf, BRF, LVL_BUILDER, TRUE);
    return;
  }

  /* Found it, free the original listing and implement the new one */
  treasure_list[i].num_of_items = OLC_TLIST(d)->num_of_items;
  FREE(treasure_list[i].vnum);
  CREATE(treasure_list[i].vnum, long, OLC_TLIST(d)->num_of_items);
  for (j = 0; j < OLC_TLIST(d)->num_of_items; j++)
    treasure_list[i].vnum[j] = OLC_TLIST(d)->vnum[j];

  olc_add_to_save_list(0, OLC_SAVE_TREASURE);
}
/*------------------------------------------------------------------------*/

void tedit_save_to_disk(struct descriptor_data *d)
{
  int i, j;
  FILE *fp;

  sprintf(buf, "%s/treasure", OBJ_PREFIX);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Cannot open treasure list file!", BRF, LVL_BUILDER, TRUE);
    return;
  }

  /* If there are no items, we don't want to do anything */
  if (top_of_treasure) {

    /* write the total number of treasure lists */
    fprintf(fp, "%d\n", top_of_treasure);

    /* write the treasure list types and number of items within */
    for (i = 0; i < top_of_treasure; i++)
      fprintf(fp, 
        "%s\n"
        "%d\n",
        treasure_list[i].type,
        treasure_list[i].num_of_items
       );

    /* write treasure list vnums based on type */
    for (i = 0; i < top_of_treasure; i++)
      for (j = 0; j < treasure_list[i].num_of_items; j++)
        fprintf(fp, "%ld\n", treasure_list[i].vnum[j]);
  }

  /* close the file */
  fclose(fp);
  olc_remove_from_save_list(0, OLC_SAVE_TREASURE);
}
/*------------------------------------------------------------------------*/

void tedit_delete_list(struct descriptor_data *d)
{
  int i, j, k, len, found = FALSE;
  struct tlist_data *tlist = NULL;

  /* Locate the treasure list to be deleted */
  for (i = 0; i < top_of_treasure; i++) {
    len = strlen(OLC_TLIST(d)->type);
    if (!strncmp(OLC_TLIST(d)->type, treasure_list[i].type, len)) {
      found = TRUE;
      break;
    }
  }

  /* Was there a problem finding it? */
  if (!found) {
    sprintf(buf, "SYSERR: OLC: Cannot delete treasure list '%s' from memory!",
      OLC_TLIST(d)->type);
    mudlog(buf, BRF, LVL_BUILDER, TRUE);
    return;
  }

  /* We only need to do this if it isn't the last item in the list */
  if ((top_of_treasure - 1)) {

    /* Found it, shift following treasure lists along and free last pointer */
    CREATE(tlist, struct tlist_data, top_of_treasure - 1);
    for (i = 0, k = 0; i < top_of_treasure; i++) {
      len = strlen(OLC_TLIST(d)->type);
      if (strncmp(OLC_TLIST(d)->type, treasure_list[i].type, len)) {
        tlist[k].type = strdup(treasure_list[i].type);
        tlist[k].num_of_items = treasure_list[i].num_of_items;
        CREATE(tlist[k].vnum, long, tlist[k].num_of_items);
        for (j = 0; j < tlist[k].num_of_items; j++)
          tlist[k].vnum[j] = treasure_list[i].vnum[j];
        k++;
      }
    }
  }

  FREE(treasure_list);
  top_of_treasure--;
  treasure_list = tlist;
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/* display main menu */
void tedit_disp_menu(struct descriptor_data *d)
{
  int i;
  struct tlist_data *tlist;
  struct obj_data *obj;

  tlist = OLC_TLIST(d);
  get_char_cols(d->character);

  /* Build menu title */
  sprintf(buf,"\033[H\033[J"
    "Treasure List Menu - %s%s%s\r\n", cyn, tlist->type, nrm);
  send_to_char(buf, d->character);

  /* Build treasure list items */
  for (i = 0; i < tlist->num_of_items; i++) {
    obj = read_object(tlist->vnum[i], VIRTUAL, NULL);
    sprintf(buf, 
      "  [%s%5ld%s] %s%s%s\r\n",

      cyn, tlist->vnum[i], nrm,
      yel, obj->short_description, nrm);
    extract_obj(obj);
    send_to_char(buf, d->character);
  }

  /* Build menu options */
  sprintf(buf, "\r\n"
    "%sA%s) Add a new item to this list\r\n"
    "%sR%s) Remove an item from this list\r\n"
    "%sD%s) Delete this list entirely\r\n"
	  "%sQ%s) Quit\r\n"
	  "Enter choice : ",

    grn, nrm, grn, nrm, grn, nrm, grn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = TEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/

void tedit_parse(struct descriptor_data * d, char *arg)
{
  int i, j, number, items;
  long *vnum = 0;

  switch (OLC_MODE(d)) {
    case TEDIT_CONFIRM_SAVESTRING:
      switch (*arg) {
        case 'y':
        case 'Y':
          send_to_char("Saving treasure list to memory.\r\n", d->character);
          tedit_save_internally(d);
          sprintf(buf, "OLC: %s edits treasure list '%s'",
            GET_NAME(d->character), OLC_TLIST(d)->type);
          mudlog(buf, CMP, LVL_BUILDER, TRUE);
          cleanup_olc(d, CLEANUP_ALL);
          return;
        case 'n':
        case 'N':
          /*. Cleanup all .*/
          cleanup_olc(d, CLEANUP_ALL);
          return;
        default:
          send_to_char("Invalid choice!\r\n", d->character);
          send_to_char("Do you wish to save this treasure list internally?\r\n", d->character);
          return;
      }

    case TEDIT_CONFIRM_DELETE_LIST:
      switch (*arg) {
        case 'y':
        case 'Y':
          send_to_char("Deleting this treasure list.\r\n", d->character);
          tedit_delete_list(d);
          sprintf(buf, "OLC: %s deletes treasure list '%s'",
            GET_NAME(d->character), OLC_TLIST(d)->type);
          mudlog(buf, CMP, LVL_BUILDER, TRUE);
          olc_add_to_save_list(0, OLC_SAVE_TREASURE);
          cleanup_olc(d, CLEANUP_ALL);
          return;
        case 'n':
        case 'N':
          tedit_disp_menu(d);
          return;
        default:
          tedit_disp_menu(d);
          return;
      }
      
    case TEDIT_ADD_OBJECT:
      number = atoi(arg);
      if (!number) {                            /* Exit if 0 entered */
        tedit_disp_menu(d);
        return;
      } else
        if (real_object(number) < 0) {          /* Does this object exist? */
          send_to_char("That object does not exist!\r\n", d->character);
          send_to_char("Enter the object vnum to add (0 for none): ", d->character);
          return;
        }

      items = OLC_TLIST(d)->num_of_items++;
      if (items) {                              /* Existing vnums, let's add them */
        CREATE(vnum, long, items);
        for (i = 0; i < items; i++)
          vnum[i] = OLC_TLIST(d)->vnum[i];

        FREE(OLC_TLIST(d)->vnum);               /* Free existing pointer */
      }

      CREATE(OLC_TLIST(d)->vnum, long, items + 1);  /* Copy data back to pointer */
      for (i = 0; i < items; i++)
        OLC_TLIST(d)->vnum[i] = vnum[i];

      OLC_TLIST(d)->vnum[items] = (long)number;
      FREE(vnum);
      break;

    case TEDIT_REMOVE_OBJECT:
      number = atoi(arg);
      if (number) {                             /* Search only if 0 not entered */
        items = OLC_TLIST(d)->num_of_items;
        for (i = 0, j = 0; i < items; i++)
          if (OLC_TLIST(d)->vnum[i] != number)
            j++;

        if (j != items) {                       /* Was the item found? */
          CREATE(vnum, long, items - 1);
          for (i = 0, j = 0; i < items; i++)
            if (OLC_TLIST(d)->vnum[i] != number)
              vnum[j++] = OLC_TLIST(d)->vnum[i];

          FREE(OLC_TLIST(d)->vnum);
          OLC_TLIST(d)->num_of_items--;
          CREATE(OLC_TLIST(d)->vnum, long, OLC_TLIST(d)->num_of_items);
          for (i = 0; i < OLC_TLIST(d)->num_of_items; i++)
            OLC_TLIST(d)->vnum[i] = vnum[i];

          FREE(vnum);
        }
      }

      break;

    case TEDIT_MAIN_MENU:
      /* throw us out to whichever edit mode based on user input */
      switch (toupper(*arg)) {
        case 'q':
        case 'Q':
          if (OLC_VAL(d)) { 
            /*. Something has been modified .*/
            send_to_char("Do you wish to save this treasure list internally? : ",
              d->character);
            OLC_MODE(d) = TEDIT_CONFIRM_SAVESTRING;
          } else 
            cleanup_olc(d, CLEANUP_ALL);
          return;
        case 'a':
        case 'A':
          send_to_char("Enter the object vnum to add (0 for none): ", d->character);
          OLC_MODE(d) = TEDIT_ADD_OBJECT;
          break;
        case 'r':
        case 'R':
          send_to_char("Enter the object vnum to remove (0 for none): ", d->character);
          OLC_MODE(d) = TEDIT_REMOVE_OBJECT;
          break;
        case 'd':
        case 'D':
          send_to_char("Are you sure you wish to delete this list? : ", d->character);
          OLC_MODE(d) = TEDIT_CONFIRM_DELETE_LIST;
          return;
        default:
          tedit_disp_menu(d);
          break;
      }
      
      return;     /* end of TEDIT_MAIN_MENU */

    default:
      mudlog("SYSERR: OLC: Reached default case in tedit_parse()!",
        BRF, LVL_BUILDER, TRUE);
      break;
    }

  /*. If we get here, we have changed something .*/
  OLC_VAL(d) = 1; /*. Has changed flag .*/
  tedit_disp_menu(d);
}
