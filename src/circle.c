/*
 * Entry point to mud, has main().
 *
 * All parts of this code not covered by the copyright by the Trustees of the
 * Johns Hopkins University are Copyright (C) 1996, 97, 98 by the Dark Pawns
 * Coding Team.
 *
 * This includes all original code done for Dark Pawns MUD by other authors.
 * All code is the intellectual property of the author, and is used here by
 * permission.
 *
 * No original code may be duplicated, reused, or executed without the written
 * permission of the author. All rights reserved.
 *
 * See dp-team.txt or "help coding" online for members of the Dark Pawns Coding
 *  Team.
 */

/* $Id: /local/dp2.2/src/comm.c 1485 2008-05-23T01:06:27.235780Z jsravn  $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"

int main(int argc, char *argv[])
{
  return dp_main(argc, argv);
}
