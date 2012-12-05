/**********************************************************
 *  dream.h -- dream code                                 *
 *                                                        *
 *  For Dark Pawns -- A CircleMUD derivative              *
 **********************************************************/

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

#ifndef _DREAM_H
#define _DREAM_H

void dream(struct char_data *i);
void dream_travel(struct char_data *ch, int subcmd);

struct dtravel_data {
   int subcmd;
   int room_num;
   char descrip[80];
};

#endif /* _DREAM_H */
