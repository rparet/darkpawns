/*******************************************************
 *  oc.c   Object Stacking code from Scott G. (Raven)  *
 *******************************************************/

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

/* $Id: oc.c 1487 2008-05-22 01:36:10Z jravn $ */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>


#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "screen.h"
#include "oc.h"

extern struct obj_data *object_list;

struct howmany *oc_get_node (void) {
  struct howmany *item;

  CREATE(item, struct howmany, sizeof(struct howmany));
  if (item != NULL) {
    item->vnum = 0;
    item->count = 0;
    item->extras = 0;
    item->next = NULL;
    item->weight = 0;
    item->text[0] = 0;
  } else {
    log ("Error allocating space in object clumping (oc.c)");
  }
  return (item);
}

struct howmany *oc_add_front (struct howmany *new, struct howmany *list) {
  new->next = list;
  list = new;
  return(list);
}

int oc_onlist (int vnum, int *extras, int weight, char *text, struct howmany *list)
{
  struct howmany *temp;

  temp = list;
  while (temp != NULL)
  {
    if ((temp->vnum == vnum) && (temp->extras == *extras) &&
        !strcmp(text, temp->text) && (temp->weight == weight))
    {
      temp->count++;
      return TRUE;
    }
    temp = temp->next;
  }
  return FALSE;
}

void oc_show_list (struct howmany *list, struct char_data *ch,
                   int show_weight, int show_wide, int show_indent, int show_header) {

  /* struct obj_data *obj; */
  int weight;
  int number;
  int extras;
  int first_extra = 1;

    *buf = 0;
    if (show_header) {
    strcpy(buf, "\r\n Num  Item   ");
    if (show_wide && show_weight)
      strcat(buf, "                                                   Encumbrance\r\n");
    else if (show_weight)
      strcat(buf, "                 encumbrance\r\n");

    if (show_wide)
      strcat(buf, "-------------------------------------------------------------------------------\r\n");
    else
      strcat(buf, "----------------------------------------------\r\n");

    send_to_char(buf, ch);
  }

  while (list != NULL) {
    *buf = 0;
    weight = list->weight;
    number = list->count;
    extras = list->extras;

    if (number == 1) {
      if (show_indent) strcat(buf, "  1   ");
      if (show_wide) sprintf(buf, "%s%-63s", buf, list->text);
        else sprintf(buf, "%s%-33s", buf, list->text);
      if (show_weight) sprintf(buf, "%s%2d pt%s", buf, weight,
        ((weight == 1) ? "" : "s"));
    }

    else if (number > 1) {
      if (show_indent) sprintf(buf, " %2d   ", number);
        else sprintf(buf, "[%2d] ", number);
      if (show_wide) sprintf(buf, "%s%-63s", buf, list->text);
        else sprintf(buf, "%s%-33s", buf, list->text);
      if (show_weight) sprintf(buf, "%s%2d pt%s ea.", buf, weight,
        ((weight == 1) ? "" : "s"));
    }

    else
      strcpy(buf, "____THIS IS AN ERROR, PLEASE REPORT!____");


    strcat(buf, "\r\n");

    first_extra = 1;
    if (IS_SET(extras, (1 << ITEM_INVISIBLE))) {
      if (first_extra) strcat(buf, "          ");
      strcat(buf, "...it is invisible ");
      first_extra = 0;
    }
    if (IS_SET(extras, (1 << ITEM_BLESS)) &&
        IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
      if (first_extra) strcat(buf, "          ");
      strcat(buf, "...it glows blue");
      first_extra = 0;
    }
    if (IS_SET(extras, (1 << ITEM_MAGIC)) &&
        IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
      if (first_extra) strcat(buf, "          ");
      strcat(buf, "...it glows gold");
      first_extra = 0;
    }
    if (IS_SET(extras, (1 << ITEM_GLOW))) {
      if (first_extra) strcat(buf, "          ");
      strcat(buf, "...it glows white");
      first_extra = 0;
    }
    if (IS_SET(extras, (1 << ITEM_HUM))) {
      if (first_extra) strcat(buf, "          ");
      strcat(buf, "...it is humming");
      first_extra = 0;
    }

    if (!first_extra) strcat(buf, "\r\n");

    send_to_char(buf, ch);
    list = list->next;
  }
}

void oc_dispose_list (struct howmany *list) {
  struct howmany *temp;

  while (list != NULL) {
    temp = list;
    list = list->next;
    FREE(temp);
  }
}
