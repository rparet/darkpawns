/* ************************************************************************
*   File: boards.c                                      Part of CircleMUD *
*  Usage: handling of multiple bulletin boards                            *
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

/* $Id: boards.c 1487 2008-05-22 01:36:10Z jravn $ */

/* FEATURES & INSTALLATION INSTRUCTIONS ***********************************

Thhis board code has many improvements over the infamously buggy standard
Diku board code.  Features include:

- Arbitrary number of boards handled by one set of generalized routines.
  Adding a new board is as easy as adding another entry to an array.
- Safe removal of messages while other messages are being written.
- Does not allow messages to be removed by someone of a level less than
  the poster's level.


TO ADD A NEW BOARD, simply follow our easy 4-step program:

1 - Create a new board object in the object files

2 - Increase the NUM_OF_BOARDS constant in board.h

3 - Add a new line to the board_info array below.  The fields, in order, are:

	Board's virtual number.
	Min level one must be to look at this board or read messages on it.
	Min level one must be to post a message to the board.
	Min level one must be to remove other people's messages from this
		board (but you can always remove your own message).
	Filename of this board, in quotes.
	Last field must always be 0.

4 - In spec_assign.c, find the section which assigns the special procedure
    gen_board to the other bulletin boards, and add your new one in a
    similar fashion.

*/


#include "config.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "boards.h"
#include "interpreter.h"
#include "handler.h"
#include "improved-edit.h"

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;

int Board_display_msg(int board_type, struct char_data * ch, char *arg);
int Board_show_board(int board_type, struct char_data * ch, char *arg);
int Board_remove_msg(int board_type, struct char_data * ch, char *arg);
void Board_save_board(int board_type);
void Board_load_board(int board_type);
void Board_reset_board(int board_num);
void Board_write_message(int board_type, struct char_data * ch, char *arg);

/*
format:	vnum, read lvl, write lvl, remove lvl, filename, 0 at end
Be sure to also change NUM_OF_BOARDS in boards.h
*/
struct board_info_type board_info[NUM_OF_BOARDS] = {
  {8099, 0, 0, LVL_IMMORT, "etc/board.mort", 0},
  {8064, 0, 0, LVL_IMMORT, "etc/board.customs", 0},
  {8065, 0, 0, LVL_IMMORT, "etc/board/chosen", 0},
  {8098, LVL_IMMORT, LVL_IMMORT, LVL_GRGOD, "etc/board.immort", 0},
  {8096, LVL_IMMORT, LVL_IMMORT, LVL_GRGOD, "etc/board.social", 0},
  {8097, LVL_IMMORT, LVL_IMMORT, LVL_IMPL, "etc/board.freeze", 0},
  {19652, 0, 0, 0, "etc/board.trinity", 0},
  {19601, 0, 0, 0, "etc/board.neosunz", 0},
  {19627, 0, 0, 0, "etc/board.arithrix", 0},
  {19666, 0, 0, 0, "etc/board.silent_shadows", 0},
  {19677, 0, 0, 0, "etc/board.domination", 0},
  {19640, 0, 0, 0, "etc/board.tripnosis", 0},
};


char *msg_storage[INDEX_SIZE];
int msg_storage_taken[INDEX_SIZE];
int num_of_msgs[NUM_OF_BOARDS];
int CMD_READ, CMD_LOOK, CMD_EXAMINE, CMD_WRITE, CMD_REMOVE;
struct board_msginfo msg_index[NUM_OF_BOARDS][MAX_BOARD_MESSAGES];


int find_slot(void)
{
  int i;

  for (i = 0; i < INDEX_SIZE; i++)
    if (!msg_storage_taken[i]) {
      msg_storage_taken[i] = 1;
      return i;
    }
  return -1;
}


/* search the room ch is standing in to find which board he's looking at */
int find_board(struct char_data * ch)
{
  struct obj_data *obj;
  int i;

  for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
    for (i = 0; i < NUM_OF_BOARDS; i++)
      if (BOARD_RNUM(i) == GET_OBJ_RNUM(obj))
	return i;

  return -1;
}


void init_boards(void)
{
  int i, j, fatal_error = 0;
  char buf[256];

  for (i = 0; i < INDEX_SIZE; i++) {
    msg_storage[i] = 0;
    msg_storage_taken[i] = 0;
  }

  for (i = 0; i < NUM_OF_BOARDS; i++) {
    if ((BOARD_RNUM(i) = real_object(BOARD_VNUM(i))) == -1) {
      sprintf(buf, "SYSERR: Fatal board error: board vnum %d does not exist!",
	      BOARD_VNUM(i));
      log(buf);
      fatal_error = 1;
    }
    num_of_msgs[i] = 0;
    for (j = 0; j < MAX_BOARD_MESSAGES; j++) {
      memset((char *) &(msg_index[i][j]), 0, sizeof(struct board_msginfo));
      msg_index[i][j].slot_num = -1;
    }
    Board_load_board(i);
  }

  CMD_READ = find_command("read");
  CMD_WRITE = find_command("write");
  CMD_REMOVE = find_command("remove");
  CMD_LOOK = find_command("look");
  CMD_EXAMINE = find_command("examine");

  if (fatal_error)
    exit(1);
}


SPECIAL(gen_board)
{
  int board_type;
  static int loaded = 0;

  if (!loaded) {
    init_boards();
    loaded = 1;
  }
  if (!ch->desc)
    return 0;

  if (cmd != CMD_WRITE && cmd != CMD_LOOK && cmd != CMD_EXAMINE &&
      cmd != CMD_READ && cmd != CMD_REMOVE)
    return 0;

  if ((board_type = find_board(ch)) == -1) {
    log("SYSERR:  degenerate board!  (what the hell...)");
    return 0;
  }
  if (cmd == CMD_WRITE) {
    Board_write_message(board_type, ch, argument);
    return 1;
  } else if (cmd == CMD_LOOK || cmd == CMD_EXAMINE)
    return (Board_show_board(board_type, ch, argument));
  else if (cmd == CMD_READ)
    return (Board_display_msg(board_type, ch, argument));
  else if (cmd == CMD_REMOVE)
    return (Board_remove_msg(board_type, ch, argument));
  else
    return 0;
}


void Board_write_message(int board_type, struct char_data * ch, char *arg)
{
  char *tmstr;
  int len;
  time_t ct;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  if (GET_LEVEL(ch) < WRITE_LVL(board_type)) {
    send_to_char("You are not holy enough to write on this board.\r\n", ch);
    return;
  }
  if (num_of_msgs[board_type] >= MAX_BOARD_MESSAGES) {
    send_to_char("The board is full.\r\n", ch);
    return;
  }
  if ((NEW_MSG_INDEX(board_type).slot_num = find_slot()) == -1) {
    send_to_char("The board is malfunctioning - sorry.\r\n", ch);
    log("SYSERR: Board: failed to find empty slot on write.");
    return;
  }
  /* skip blanks */
  skip_spaces(&arg);
  delete_doubledollar(arg);

  /* JE 27 Oct 95 - Truncate headline at 80 chars if it's longer than that */
  arg[81] = '\0';

  if (!*arg) {
    send_to_char("We must have a headline!\r\n", ch);
    return;
  }
  ct = time(0);
  tmstr = (char *) asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  sprintf(buf2, "(%s)", GET_NAME(ch));
  sprintf(buf, "%6.10s %-12s :: %s", tmstr, buf2, arg);
  len = strlen(buf) + 1;
  CREATE(NEW_MSG_INDEX(board_type).heading, char, (sizeof(char) * len));
  if (!(NEW_MSG_INDEX(board_type).heading))
  {
    send_to_char("The board is malfunctioning - sorry.\r\n", ch);
    return;
  }
  strcpy(NEW_MSG_INDEX(board_type).heading, buf);
  NEW_MSG_INDEX(board_type).heading[len - 1] = '\0';
  NEW_MSG_INDEX(board_type).level = GET_LEVEL(ch);

  send_to_char("Write your message.\r\n", ch);
  send_editor_help(ch->desc);
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

  if (!IS_NPC(ch))
    SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);

  ch->desc->str = &(msg_storage[NEW_MSG_INDEX(board_type).slot_num]);
  ch->desc->max_str = MAX_MESSAGE_LENGTH;
  ch->desc->mail_to = board_type + BOARD_MAGIC;

  num_of_msgs[board_type]++;
}


int Board_show_board(int board_type, struct char_data * ch, char *arg)
{
  int i;
  char tmp[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];

  if (!ch->desc)
    return 0;

  one_argument(arg, tmp);

  if (!*tmp || !isname(tmp, "board bulletin"))
    return 0;

  if (GET_LEVEL(ch) < READ_LVL(board_type)) {
    send_to_char("You try but fail to understand the holy words.\r\n", ch);
    return 1;
  }
  act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);

  strcpy(buf,
	 "This is a bulletin board.  Usage: READ/REMOVE <messg #>, WRITE <header>.\r\n"
	 "You will need to look at the board to save your message.\r\n");
  if (!num_of_msgs[board_type])
    strcat(buf, "The board is empty.\r\n");
  else {
    sprintf(buf + strlen(buf), "There are %d messages on the board.\r\n",
	    num_of_msgs[board_type]);
    for (i = 0; i < num_of_msgs[board_type]; i++) {
/*    for (i = num_of_msgs[board_type] - 1; i >= 0; i--) { */
      if (MSG_HEADING(board_type, i))
	sprintf(buf + strlen(buf), "%-2d : %s\r\n", i + 1, MSG_HEADING(board_type, i));
      else {
	log("SYSERR: The board is fubar'd.");
	send_to_char("Sorry, the board isn't working.\r\n", ch);
	return 1;
      }
    }
  }
  page_string(ch->desc, buf, 1);

  return 1;
}


int Board_display_msg(int board_type, struct char_data * ch, char *arg)
{
  char number[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];
  int msg, ind;

  one_argument(arg, number);
  if (!*number)
    return 0;
  if (isname(number, "board bulletin"))	/* so "read board" works */
    return (Board_show_board(board_type, ch, arg));
  if (!isdigit(*number) || (!(msg = atoi(number))))
    return 0;

  if (GET_LEVEL(ch) < READ_LVL(board_type)) {
    send_to_char("You try but fail to understand the holy words.\r\n", ch);
    return 1;
  }
  if (!num_of_msgs[board_type]) {
    send_to_char("The board is empty!\r\n", ch);
    return (1);
  }
  if (msg < 1 || msg > num_of_msgs[board_type]) {
    send_to_char("That message exists only in your imagination.\r\n",
		 ch);
    return (1);
  }
  ind = msg - 1;
  if (MSG_SLOTNUM(board_type, ind) < 0 ||
      MSG_SLOTNUM(board_type, ind) >= INDEX_SIZE) {
    send_to_char("Sorry, the board is not working.\r\n", ch);
    log("SYSERR: Board is screwed up.");
    return 1;
  }
  if (!(MSG_HEADING(board_type, ind))) {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }
  if (!(msg_storage[MSG_SLOTNUM(board_type, ind)])) {
    send_to_char("That message seems to be empty.\r\n", ch);
    return 1;
  }
  sprintf(buffer, "Message %d : %s\r\n\r\n%s\r\n", msg,
	  MSG_HEADING(board_type, ind),
	  msg_storage[MSG_SLOTNUM(board_type, ind)]);

  page_string(ch->desc, buffer, 1);

  return 1;
}


int Board_remove_msg(int board_type, struct char_data * ch, char *arg)
{
  int ind, msg, slot_num;
  char number[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct descriptor_data *d;

  one_argument(arg, number);

  if (!*number || !isdigit(*number))
    return 0;
  if (!(msg = atoi(number)))
    return (0);

  if (!num_of_msgs[board_type]) {
    send_to_char("The board is empty!\r\n", ch);
    return 1;
  }
  if (msg < 1 || msg > num_of_msgs[board_type]) {
    send_to_char("That message exists only in your imagination.\r\n", ch);
    return 1;
  }
  ind = msg - 1;
  if (!MSG_HEADING(board_type, ind)) {
    send_to_char("That message appears to be screwed up.\r\n", ch);
    return 1;
  }
  sprintf(buf, "(%s)", GET_NAME(ch));
  if (GET_LEVEL(ch) < REMOVE_LVL(board_type) &&
      !(strstr(MSG_HEADING(board_type, ind), buf))) {
    send_to_char("You are not holy enough to remove other people's messages.\r\n", ch);
    return 1;
  }
  if (GET_LEVEL(ch) < MSG_LEVEL(board_type, ind)) {
    if (GET_LEVEL(ch) < LVL_IMPL-1)
    {
      send_to_char("You can't remove a message holier than yourself.\r\n",
                    ch);
      return 1;
    }
  }
  slot_num = MSG_SLOTNUM(board_type, ind);
  if (slot_num < 0 || slot_num >= INDEX_SIZE) {
    log("SYSERR: The board is seriously screwed up.");
    send_to_char("That message is majorly screwed up.\r\n", ch);
    return 1;
  }
  for (d = descriptor_list; d; d = d->next)
    if (!d->connected && d->str == &(msg_storage[slot_num])) {
      send_to_char("At least wait until the author is finished before removing it!\r\n", ch);
      return 1;
    }
  if (msg_storage[slot_num])
    FREE(msg_storage[slot_num]);
  msg_storage[slot_num] = 0;
  msg_storage_taken[slot_num] = 0;
  if (MSG_HEADING(board_type, ind))
    FREE(MSG_HEADING(board_type, ind));

  for (; ind < num_of_msgs[board_type] - 1; ind++) {
    MSG_HEADING(board_type, ind) = MSG_HEADING(board_type, ind + 1);
    MSG_SLOTNUM(board_type, ind) = MSG_SLOTNUM(board_type, ind + 1);
    MSG_LEVEL(board_type, ind) = MSG_LEVEL(board_type, ind + 1);
  }
  num_of_msgs[board_type]--;
  send_to_char("Message removed.\r\n", ch);
  sprintf(buf, "$n just removed message %d.", msg);
  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  Board_save_board(board_type);

  return 1;
}


void Board_save_board(int board_type)
{
  FILE *fl;
  int i;
  char *tmp1 = 0, *tmp2 = 0;

  if (!num_of_msgs[board_type]) {
    unlink(FILENAME(board_type));
    return;
  }
  if (!(fl = fopen(FILENAME(board_type), "wb"))) {
    perror("Error writing board");
    return;
  }
  fwrite(&(num_of_msgs[board_type]), sizeof(int), 1, fl);

  for (i = 0; i < num_of_msgs[board_type]; i++) {
    if ((tmp1 = MSG_HEADING(board_type, i)))
      msg_index[board_type][i].heading_len = strlen(tmp1) + 1;
    else
      msg_index[board_type][i].heading_len = 0;

    if (MSG_SLOTNUM(board_type, i) < 0 ||
	MSG_SLOTNUM(board_type, i) >= INDEX_SIZE ||
	(!(tmp2 = msg_storage[MSG_SLOTNUM(board_type, i)])))
      msg_index[board_type][i].message_len = 0;
    else
      msg_index[board_type][i].message_len = strlen(tmp2) + 1;

    fwrite(&(msg_index[board_type][i]), sizeof(struct board_msginfo), 1, fl);
    if (tmp1)
      fwrite(tmp1, sizeof(char), msg_index[board_type][i].heading_len, fl);
    if (tmp2)
      fwrite(tmp2, sizeof(char), msg_index[board_type][i].message_len, fl);
  }

  fclose(fl);
}


void Board_load_board(int board_type)
{
  FILE *fl;
  int i, len1 = 0, len2 = 0;
  char *tmp1 = NULL, *tmp2 = NULL;


  if (!(fl = fopen(FILENAME(board_type), "rb"))) {
    perror("Error reading board");
    return;
  }
  fread(&(num_of_msgs[board_type]), sizeof(int), 1, fl);
  if (num_of_msgs[board_type] < 1 || num_of_msgs[board_type] > MAX_BOARD_MESSAGES) {
    log("SYSERR: Board file corrupt.  Resetting.");
    Board_reset_board(board_type);
    return;
  }
  for (i = 0; i < num_of_msgs[board_type]; i++) {
    fread(&(msg_index[board_type][i]), sizeof(struct board_msginfo), 1, fl);
    if (!(len1 = msg_index[board_type][i].heading_len)) {
      log("SYSERR: Board file corrupt!  Resetting.");
      Board_reset_board(board_type);
      return;
    }
    CREATE(tmp1, char, (sizeof(char) * len1));
    if (!tmp1) {
      log("SYSERR: Error - CREATE failed for board header");
      exit(1);
    }
    fread(tmp1, sizeof(char), len1, fl);
    MSG_HEADING(board_type, i) = tmp1;

    if ((len2 = msg_index[board_type][i].message_len)) {
      if ((MSG_SLOTNUM(board_type, i) = find_slot()) == -1) {
	log("SYSERR: Out of slots booting board!  Resetting...");
	Board_reset_board(board_type);
	return;
      }
      CREATE(tmp2, char, (sizeof(char) * len2));
      if (!tmp2) {
	log("SYSERR: CREATE failed for board text");
	exit(1);
      }
      fread(tmp2, sizeof(char), len2, fl);
      msg_storage[MSG_SLOTNUM(board_type, i)] = tmp2;
    }
  }

  fclose(fl);
}


void Board_reset_board(int board_type)
{
  int i;

  for (i = 0; i < MAX_BOARD_MESSAGES; i++) {
    if (MSG_HEADING(board_type, i))
      FREE(MSG_HEADING(board_type, i));
    if (msg_storage[MSG_SLOTNUM(board_type, i)])
      FREE(msg_storage[MSG_SLOTNUM(board_type, i)]);
    msg_storage_taken[MSG_SLOTNUM(board_type, i)] = 0;
    memset((char *)&(msg_index[board_type][i]),0,sizeof(struct board_msginfo));
    msg_index[board_type][i].slot_num = -1;
  }
  num_of_msgs[board_type] = 0;
  unlink(FILENAME(board_type));
}
