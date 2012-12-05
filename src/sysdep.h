/* ************************************************************************
*   File: sysdep.h                                      Part of CircleMUD *
*  Usage: machine-specific defs based on values in config.h (from configure)*
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

/* $Id: sysdep.h 1487 2008-05-22 01:36:10Z jravn $ */

#ifndef _SYSDEP_H
#define _SYSDEP_H

/* Standard C headers */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <assert.h>

/* POSIX headers */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Other header files */

#include <sys/select.h>
#include <zlib.h>

#ifdef HAVE_CRYPT_H
  #include <crypt.h>
#endif

#endif /* _SYSDEP_H */
