/* ************************************************************************
*  File: notes.c                                       Part of CircleMUD  *
*  Usage: handling of player notes made by staff                          *
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

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "interpreter.h"
#include "comm.h"
#include "handler.h"
#include <mysql/mysql.h>

/* Local functions */
void free_notes(struct notes_data *b);

/* External functions */
MYSQL_RES *sql_query(MYSQL *db, const char *query, bool retval);

/* External Variables */
extern MYSQL *darkpawns;


void sql_write_notes(char *name, char *arg, struct char_data *vict)
{
  struct notes_data *temp;
  extern char *replace_quotes(char *text);

  if (!*arg)
    return;

  CREATE(temp, struct notes_data, 1);
  temp->note = str_dup(arg);               /* Contents of the note */
  temp->date = time(0);                    /* What date was the note recorded */
  if (!strcmp(name, "SYSTEM"))
    temp->note_by = 0;
  else
    temp->note_by = get_id_by_name(name);  /* Who wrote the note */


  /* Notes table format [id, note, date, note by] */
  sprintf(buf1, "%ld, \"%s\", %ld, %ld",
    GET_IDNUM(vict),
    replace_quotes(temp->note),
    temp->date,
    temp->note_by);
  sprintf(buf2, "REPLACE notes VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
  free_notes(temp);
}


bool sql_read_notes(struct char_data *ch, struct notes_data **notes)
{   
  int found = 0;
  struct notes_data *temp;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT COUNT(*) FROM notes WHERE id = %ld", GET_IDNUM(ch));
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      found = atoi(entry[0]);

  mysql_free_result(table);

  if (found) {
    CREATE(*notes, struct notes_data, 1);
    temp = *notes;

    sprintf(buf1, "SELECT * FROM notes WHERE id = %ld", GET_IDNUM(ch));
    if ((table = sql_query(darkpawns, buf1, TRUE)))
      while ((entry = mysql_fetch_row(table))) {
        found--;
        temp->note = str_dup(entry[1]);
        temp->date = atoi(entry[2]);
        temp->note_by = atoi(entry[3]);
        if (found) {
          CREATE(temp->next, struct notes_data, 1);
          temp = temp->next;
        }
      }
    mysql_free_result(table);
    return (TRUE);
  }
  return (FALSE);
} 


void sql_remove_note(struct char_data *ch, time_t date)
{
  sprintf(buf1, "DELETE FROM notes WHERE id = %ld AND date = %ld", GET_IDNUM(ch), date);
  sql_query(darkpawns, buf1, FALSE);
}


bool remove_note(struct char_data *ch, int num, struct notes_data **notes)
{
  int j = 0;
  struct notes_data *temp;

  temp = *notes;

  while (temp)
    if (num == ++j) {
      sql_remove_note(ch, temp->date);
      return (TRUE);
    } else
      temp = temp->next;

  return (FALSE);
}


void free_notes(struct notes_data *b)
{
  struct notes_data *nl;

  while (b) {
    nl = b;
    b = b->next;
    if (nl->note)
      FREE(nl->note);
    FREE(nl);
  }
}

void system_notes(struct char_data *ch, char *text, int type)
{
  int i;
  struct char_data *vict, *cbuf = NULL;
  extern int top_of_p_table ;
  extern struct player_index_element *player_table;

  if (!*text) {
    send_to_char("We must have something to write!\r\n", ch);
    return;
  }

  for (i = 0; i <= top_of_p_table; i++) {
    if (!(vict = get_player_vis(ch, (player_table + i)->name, FIND_CHAR_WORLD))) {
      if ((cbuf = load_char((player_table + i)->name)))
        vict = cbuf;
      else {
        log("SYSERR: Error attempting to locate player for system note.");
        break;
      }
    }
    
    if (!PLR_FLAGGED(vict, PLR_DELETED))
      switch(type) {
        case 0:     /* all */
          sql_write_notes("SYSTEM", buf, vict);
          break;
        case 1:     /* immortals */
          if (GET_LEVEL(vict) >= LVL_IMMORT)
            sql_write_notes("SYSTEM", buf, vict);
          break;
        case 2:     /* players */
          if (GET_LEVEL(vict) < LVL_IMMORT)
            sql_write_notes("SYSTEM", buf, vict);
          break;
      }

    if (!cbuf) {
      free_char(cbuf);            /* Free after every character loaded */
      cbuf = NULL;
    }
  }
}

ACMD(do_notes)
{
  int i, j = 0;
  bool exists = FALSE;
  struct char_data *vict = NULL, *cbuf = NULL;
  struct notes_data *notes = NULL, *nl;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], note_date[MAX_INPUT_LENGTH], *timestr;

  const char *groups[] = {
    "all",
    "immortals",
    "players",
    "\n"
  };

  struct notes_struct {
    const char *cmd;
    const char level;
  } fields[] = {
    { "nothing", 0 },            /* 0 */
    { "list",    LVL_IMMORT },
    { "add",     LVL_IMMORT },
    { "delete",  LVL_IMMORT },
    { "system",  LVL_GRGOD  },
    { "\n",      0}
  };

  if (IS_NPC(ch))
    return;

  skip_spaces(&argument);

  if (!*argument) {
    strcpy(buf, "Notes options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
          sprintf(buf + strlen(buf), "%-15s%s", fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  half_chop(argument, field, buf);
  half_chop(buf, name, buf);

  if (!*name || !*field) {
    send_to_char("Usage: notes <field> <player/group> <text>\r\n", ch);
    return;
  }

  for (i = 0; *(fields[i].cmd) != '\n'; i++)
    if (!strncmp(field, fields[i].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[i].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }

  if (!str_cmp(field, "system")) {         /* Is this a system message? */
    for (j = 0; *groups[j] != '\n'; j++)
      if (!strncmp(name, groups[j], strlen(name))) {
        exists = TRUE;
        break;
      }

    if (!exists) {
      send_to_char("That is not a valid group.\r\n", ch);
      return;
    }
  } else if (!(vict = get_player_vis(ch, name, FIND_CHAR_WORLD))) {
    if ((cbuf = load_char(name))) {
      vict = cbuf;      
      exists = TRUE;
    } else {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }

  switch(i) {
    case 1:             /* List character VICT notes */
      if (!sql_read_notes(vict, &notes))
        send_to_char("No notes on this character.\r\n", ch);
      else {
        nl = notes;     /* Copy the list over for FREE-ing later */
        j = 0;
        while (notes) {
          timestr = asctime(localtime(&(notes->date)));
          *(timestr + 16) = 0;
          strcpy(note_date, timestr);

          sprintf(buf, "%2d. (%s) %-10s - %s\r\n",
            ++j, get_name_by_id(notes->note_by) ? get_name_by_id(notes->note_by) : "SYSTEM",
            note_date, notes->note);
          send_to_char(buf, ch);
          notes = notes->next;
        }
        notes = nl;
      }
      break;

    case 2:             /* Add notes to character VICT */
      if (!*buf) {
        send_to_char("We must have something to write!\r\n", ch);
        return;
      }

      sql_write_notes(GET_NAME(ch), buf, vict);
      send_to_char("Note added.\r\n", ch);
      sprintf(buf, "(GC) Note on %s added by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;

    case 3:             /* Remove notes from character VICT */
      if (!sql_read_notes(vict, &notes))
        send_to_char("No notes on this character.\r\n", ch);
      else if (!remove_note(vict, atoi(buf), &notes))
        send_to_char("No such note exists.\r\n", ch);
      else {
        send_to_char("Note deleted.\r\n", ch);
        sprintf(buf1, "(GC) Note on %s deleted by %s.", GET_NAME(vict), GET_NAME(ch));
        mudlog(buf1, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      }
      break;

    case 4:             /* Send a SYSTEM note to the specified group */
      system_notes(ch, buf, j);
      send_to_char("System notes sent to appropriate players.\r\n", ch);
      return;

    default:
      send_to_char("Sorry, I don't understand that.\r\n", ch);
      break;
  }

  free_notes(notes);
  free_char(cbuf);
}

