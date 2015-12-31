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

/* $Id: oc.h 129 2001-05-03 11:35:35Z rparet $ */

#ifndef _OC_H
#define _OC_H

/* object clumping stuff */
struct howmany {
  int vnum;
  int count;
  int extras;
  int weight;
  char text[512];
  struct howmany *next;
};
struct howmany *oc_get_node (void);
struct howmany *oc_add_front (struct howmany *new, struct howmany *list);
int oc_onlist (int vnum, int *extras, int weight, char *text, struct howmany *list);
void oc_dispose_list (struct howmany *list);
void oc_show_list (struct howmany *list, struct char_data *ch,
                   int show_weight, int show_wide, int show_indent,
                   int show_header);

#endif /* _OC_H */
