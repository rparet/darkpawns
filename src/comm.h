/* ************************************************************************
*   File: comm.h                                        Part of CircleMUD *
*  Usage: header file: prototypes of public communication functions       *
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

/* $Id: comm.h 1519 2008-06-14 20:40:26Z jravn $ */

#ifndef _COMM_H
#define _COMM_H

#define NUM_RESERVED_DESCS  8

/* comm.c */
int dp_main(int argc, char *argv[]);
void    send_to_all(char *messg);
void    send_to_char(const char *messg, struct char_data *ch);
void    send_to_room(char *messg, int room);
void    send_to_outdoor(char *messg);
void    perform_to_all(char *messg, struct char_data *ch);
void    close_socket(struct descriptor_data *d);

void    perform_act(char *orig, struct char_data *ch, struct obj_data *obj,
            void *vict_obj, struct char_data *to);

void    act(char *str, int hide_invisible, struct char_data *ch,
struct obj_data *obj, void *vict_obj, int type);

#define TO_ROOM     1
#define TO_VICT     2
#define TO_NOTVICT  3
#define TO_CHAR     4
#define TO_SLEEP    128 /* to char, even if sleeping */

int  write_to_descriptor(int desc, const char *txt, struct compr *comp);
void write_to_q(char *txt, struct txt_q *queue, int aliased);
size_t  write_to_output(struct descriptor_data *d, const char *txt, ...);
size_t  vwrite_to_output(struct descriptor_data *d, const char *format, va_list args);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
void string_write(struct descriptor_data *d, char **txt, size_t len,
   long mailto, void *data);

#define PAGE_LENGTH 22
#define PAGE_WIDTH  78

#define SEND_TO_Q(messg, desc)  write_to_output((desc), "%s", (messg))

#define USING_SMALL(d)  ((d)->output == (d)->small_outbuf)
#define USING_LARGE(d)  (!USING_SMALL(d))

typedef RETSIGTYPE sigfunc(int);

#endif /* _COMM_H */
