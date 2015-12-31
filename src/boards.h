/* ************************************************************************
*   File: boards.h                                      Part of CircleMUD *
*  Usage: header file for bulletin boards                                 *
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

/* $Id: boards.h 1336 2008-02-23 00:37:54Z jravn $ */

#ifndef _BOARDS_H
#define _BOARDS_H

#define NUM_OF_BOARDS       12  /* change if needed! */
#define MAX_BOARD_MESSAGES  60      /* arbitrary -- change if needed */
#define MAX_MESSAGE_LENGTH  4096    /* arbitrary -- change if needed */

#define INDEX_SIZE     ((NUM_OF_BOARDS*MAX_BOARD_MESSAGES) + 5)

#define BOARD_MAGIC 1048575 /* arbitrary number - see modify.c */

struct board_msginfo {
   int  slot_num;     /* pos of message in "master index" */
   char *heading;     /* pointer to message's heading */
   int  level;        /* level of poster */
   int  heading_len;  /* size of header (for file write) */
   int  message_len;  /* size of message text (for file write) */
};

struct board_info_type {
   int  vnum;       /* vnum of this board */
   int  read_lvl;   /* min level to read messages on this board */
   int  write_lvl;  /* min level to write messages on this board */
   int  remove_lvl; /* min level to remove messages from this board */
   char filename[50];   /* file to save this board to */
   int  rnum;       /* rnum of this board */
};

#define BOARD_VNUM(i) (board_info[i].vnum)
#define READ_LVL(i) (board_info[i].read_lvl)
#define WRITE_LVL(i) (board_info[i].write_lvl)
#define REMOVE_LVL(i) (board_info[i].remove_lvl)
#define FILENAME(i) (board_info[i].filename)
#define BOARD_RNUM(i) (board_info[i].rnum)

#define NEW_MSG_INDEX(i) (msg_index[i][num_of_msgs[i]])
#define MSG_HEADING(i, j) (msg_index[i][j].heading)
#define MSG_SLOTNUM(i, j) (msg_index[i][j].slot_num)
#define MSG_LEVEL(i, j) (msg_index[i][j].level)

int Board_display_msg(int board_type, struct char_data *ch, char *arg);
int Board_show_board(int board_type, struct char_data *ch, char *arg);
int Board_remove_msg(int board_type, struct char_data *ch, char *arg);
void    Board_save_board(int board_type);
void    Board_load_board(int board_type);
void    Board_reset_board(int board_num);
void    Board_write_message(int board_type, struct char_data *ch, char *arg);

#endif /* _BOARDS_H */
