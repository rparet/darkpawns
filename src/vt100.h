/* ************************************************************************
*   File: vt100.h                                       Part of CircleMUD *
*  Usage: header file for vt100 codes                                     *
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

/* $Id: vt100.h 2 1999-01-01 22:49:11Z rparet $ */

#ifndef _VT100_H
#define _VT100_H

#define VT_INITSEQ    "\033[1;24r"
#define VT_CURSPOS    "\033[%d;%dH" 
#define VT_CURSRIG    "\033[%dC"   
#define VT_CURSLEF    "\033[%dD"    
#define VT_HOMECLR    "\033[2J\033[0;0H"
#define VT_CTEOTCR    "\033[K"     
#define VT_CLENSEQ    "\033[r\033[2J" 
#define VT_INDUPSC    "\033M"
#define VT_INDDOSC    "\033D"
#define VT_SETSCRL    "\033[%d;24r"
#define VT_INVERTT    "\033[0;1;7m"
#define VT_BOLDTEX    "\033[0;1m"
#define VT_NORMALT    "\033[0m"
#define VT_MARGSET    "\033[%d;%dr"
#define VT_CURSAVE    "\0337"
#define VT_CURREST    "\0338"

/* infobar defines  */
#define INFO_HIT           1
#define INFO_MANA          2
#define INFO_MOVE          4
#define INFO_EXP           8
#define INFO_GOLD          16

#endif /* _VT100_H */
