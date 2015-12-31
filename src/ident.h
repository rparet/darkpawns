/* ************************************************************************
*  File: ident.h                                              *
*                                                                         *
*  Usage: Header file containing stuff required for rfc 931/1413 ident    *
*         lookups                                                         *
*                                                                         *
*  Written by Eric Green (egreen@cypronet.com)                *
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

/* $Id: ident.h 1309 2006-08-23 22:20:34Z jravn $ */

#ifndef _IDENT_H
#define _IDENT_H

void ident_start(struct descriptor_data *d, long addr);
void ident_check(struct descriptor_data *d, int pulse);
int waiting_for_ident(struct descriptor_data *d);


extern int ident;

#endif /* _IDENT_H */
