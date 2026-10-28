/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
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

/* $Id: spec_assign.c 1277 2006-04-17 22:50:26Z rparet $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

/* External variables */
extern int mini_mud;

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  int rnum;
  if ((rnum = real_mobile(mob)) >= 0)
    mob_index[rnum].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
	    mob);
    log(buf);
  }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
	    obj);
    log(buf);
  }
}

void ASSIGNROOM(long room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%ld", room);
    log(buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  SPECIAL(postmaster);
  SPECIAL(stableboy);
  SPECIAL(remorter);
  SPECIAL(tattoo);
  SPECIAL(clerk);
  SPECIAL(mortician);
  SPECIAL(vitae_seller);
  SPECIAL(recharger);
  SPECIAL(roach);
  SPECIAL(wall_guard_ns);
  SPECIAL(repairer);
 
  /* Zone 0 stuff */
  ASSIGNMOB(23, roach);
  ASSIGNMOB(1200, postmaster);

  /* Amber/Arden */
  ASSIGNMOB(2766, tattoo);

  /* Xixieqi */
  ASSIGNMOB(4812, clerk);
  ASSIGNMOB(4821, stableboy);
  ASSIGNMOB(4827, postmaster);
  ASSIGNMOB(4834, repairer);
  ASSIGNMOB(4849, mortician);

  /* Mist Keep */
  ASSIGNMOB(5309, vitae_seller);
  ASSIGNMOB(5315, clerk);
  ASSIGNMOB(5341, postmaster);
  ASSIGNMOB(5342, mortician);
  ASSIGNMOB(5343, repairer);
  ASSIGNMOB(5344, stableboy);

  /*Random Load World*/
  ASSIGNMOB(7921, remorter);
  ASSIGNMOB(7922, recharger);
  
  /* Kir Drax'in */
  ASSIGNMOB(8000, repairer);
  ASSIGNMOB(8022, stableboy);
  ASSIGNMOB(8030, postmaster);
  ASSIGNMOB(8039, clerk);
  ASSIGNMOB(8060, wall_guard_ns);
  ASSIGNMOB(8086, tattoo);
  ASSIGNMOB(8095, mortician);

  /* City of Kir-Oshi */
  ASSIGNMOB(18210, stableboy);
  ASSIGNMOB(18213, tattoo);
  ASSIGNMOB(18228, clerk);
  ASSIGNMOB(18231, mortician);
  ASSIGNMOB(18232, repairer);
  ASSIGNMOB(18243, postmaster);

  /* City of Kir Morthis */
  ASSIGNMOB(21217, stableboy);
  ASSIGNMOB(21225, postmaster);
  ASSIGNMOB(21240, mortician);
  ASSIGNMOB(21244, tattoo);
  ASSIGNMOB(21246, vitae_seller);
  ASSIGNMOB(21253, repairer);
  ASSIGNMOB(21254, clerk);
}



/* assign special procedures to objects */
void assign_objects(void)
{
  SPECIAL(gen_board);
  SPECIAL(moon_gate);
  SPECIAL(deed_unassigned);
  SPECIAL(deed_assigned);

  /* Portals */
  ASSIGNOBJ(4001, moon_gate);   /* blue portal  */
  ASSIGNOBJ(4002, moon_gate);   /* red portal   */
  ASSIGNOBJ(4003, moon_gate);   /* green portal */

  /* Boards */
  ASSIGNOBJ(1210, gen_board);  /* customs house */
  ASSIGNOBJ(1211, gen_board);  /* chosen */
  ASSIGNOBJ(1212, gen_board);  /* social board   */
  ASSIGNOBJ(1213, gen_board);  /* freeze board   */
  ASSIGNOBJ(1214, gen_board);  /* immortal board */
  ASSIGNOBJ(1215, gen_board);  /* mortal board   */
  ASSIGNOBJ(4829, gen_board);  /* mortal board   */
  ASSIGNOBJ(5342, gen_board);  /* mortal board   */
  ASSIGNOBJ(18200, gen_board);  /* mortal board   */
  ASSIGNOBJ(21208, gen_board);  /* mortal board   */

  /* Player-owned Shop Items */
  ASSIGNOBJ(1222, deed_unassigned);
  ASSIGNOBJ(1223, deed_assigned);
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  /* extern ubyte dts_are_dumps; -- deprecated mcooper 20040915 */

  SPECIAL(enter_circle);
  SPECIAL(oro_quarters_room);
  SPECIAL(oro_study_room);
  SPECIAL(portal_room);
  SPECIAL(player_shop);

  /* Matt's spec procs - rooms (spec_procs3.c) */
  SPECIAL(fall_down);
  SPECIAL(fall_and_dropeq);

  /* Matt's spec procs - rooms (spec_procs4.c) */
  SPECIAL(stonekeep_puzzle);


  /* Stonekeep */
  ASSIGNROOM(1415, stonekeep_puzzle);
  ASSIGNROOM(1416, stonekeep_puzzle);
  ASSIGNROOM(1418, stonekeep_puzzle);
  ASSIGNROOM(1420, stonekeep_puzzle);
  ASSIGNROOM(1425, stonekeep_puzzle);
  ASSIGNROOM(1426, stonekeep_puzzle);
  ASSIGNROOM(1428, stonekeep_puzzle);
  ASSIGNROOM(1429, stonekeep_puzzle);
  ASSIGNROOM(1430, stonekeep_puzzle);
  ASSIGNROOM(1431, stonekeep_puzzle);
  ASSIGNROOM(1433, stonekeep_puzzle);
  ASSIGNROOM(1434, stonekeep_puzzle);

  /* Multi-zone Procs */
  ASSIGNROOM(2202, fall_down);
  ASSIGNROOM(19184, fall_and_dropeq);  

  /* Player-Owned Stores */
  ASSIGNROOM(8083, player_shop);
  ASSIGNROOM(8084, player_shop);
  ASSIGNROOM(8085, player_shop);
  ASSIGNROOM(8086, player_shop);
  ASSIGNROOM(8087, player_shop);
  ASSIGNROOM(8088, player_shop);
  ASSIGNROOM(8089, player_shop);
  ASSIGNROOM(8090, player_shop);
  ASSIGNROOM(5477, player_shop);
  ASSIGNROOM(5478, player_shop);
  ASSIGNROOM(5479, player_shop);
  ASSIGNROOM(5480, player_shop);
  ASSIGNROOM(5481, player_shop);
  ASSIGNROOM(5482, player_shop);
  ASSIGNROOM(5483, player_shop);
  ASSIGNROOM(5484, player_shop);
  ASSIGNROOM(18330, player_shop);
  ASSIGNROOM(18331, player_shop);
  ASSIGNROOM(18332, player_shop);
  ASSIGNROOM(18333, player_shop);
  ASSIGNROOM(4870, player_shop);
  ASSIGNROOM(4871, player_shop);
  ASSIGNROOM(4872, player_shop);
  ASSIGNROOM(4873, player_shop);
  ASSIGNROOM(21282, player_shop);
  ASSIGNROOM(21283, player_shop);
  ASSIGNROOM(21284, player_shop);
  ASSIGNROOM(21285, player_shop);
  ASSIGNROOM(21286, player_shop);
}

