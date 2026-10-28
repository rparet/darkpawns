/***************
 *  CircleMUD  *
 ***************/

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

/* $Id: house.h 1111 2003-01-02 23:39:26Z mcooper $ */

#ifndef _HOUSE_H
#define _HOUSE_H

#define MAX_HOUSES	100
#define MAX_GUESTS	50

#define HOUSE_PRIVATE	0

struct house_control_rec {
   long vnum;			/* vnum of this house		*/
   sh_int atrium;		/* vnum of atrium		*/
   sh_int exit_num;		/* direction of house's exit	*/
   time_t built_on;		/* date this house was built	*/
   int mode;			/* mode of ownership		*/
   long owner;			/* idnum of house's owner	*/
   int num_of_guests;		/* how many guests for house	*/
   long guests[MAX_GUESTS];	/* idnums of house's guests	*/
   time_t last_payment;		/* date of last house payment   */
   sh_int key;                  /* vnum of this house's key     */
   long spare1;
   long spare2;
   long spare3;
   long spare4;
   long spare5;
   long spare6;
   long spare7;
};
   
#define TOROOM(room, dir) (world[room].dir_option[dir] ? \
			    world[room].dir_option[dir]->to_room : NOWHERE)

void	House_listrent(struct char_data * ch, struct obj_file_elem object);
void	House_boot(void);
void	House_save_all(void);
int	House_can_enter(struct char_data *ch, sh_int house);
void	House_crashsave(long vnum);

#endif /* _HOUSE_H */
