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

/* $Id: mobprog.h 1456 2008-05-17 17:06:18Z jravn $ */

#ifndef _MOBPROG_H
#define _MOBPROG_H

/* dogs growl or lick depending on align of char entering room,
   bark, pee, sleep, sniff, etc... every so often.                 */
#define IS_DOG(ch)       ((GET_MOB_VNUM((ch)) == 8063) || \
                          (GET_MOB_VNUM((ch)) == 8065))

/* janitors complain, whistle, smoke, spit etc... every so often   */
#define IS_JANITOR(ch)   ((GET_MOB_VNUM((ch)) == 8061))

/* blacksmith can make things into other things... ie dragon scales
   into dragon scale armor... for a price
*/
#define IS_DEMON(ch) ((GET_MOB_VNUM((ch)) == 14401))

/* mercenaries will follow you... for a price... */
#define IS_MERCENARY(ch) ((GET_MOB_VNUM((ch)) == 3063))

/* whores sell sex :) */
#define IS_WHORE(ch) ((GET_MOB_SPEC(ch) == prostitute))

#endif /* _MOBPROG_H */
