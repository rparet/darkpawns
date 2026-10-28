/* ************************************************************************
*   File: alias.c				   A utility to CircleMUD *
*  Usage: writing/reading player's aliases                                *
*                                                                         *
*  Code done by Jeremy Hess and Chad Thompson				  *
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

/* $Id: alias.c 1201 2005-05-08 12:24:41Z mcooper $ */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "interpreter.h"
#include <mysql/mysql.h>

/* External functions */
MYSQL_RES *sql_query(MYSQL *db, const char *query, bool retval);

/* External Variables */
extern MYSQL *darkpawns;


void sql_write_aliases(struct char_data *ch)
{
  struct alias *temp;
  extern char *replace_quotes(char *text);

  if (!GET_ALIASES(ch))
    return;

  temp = GET_ALIASES(ch);

  /* Alias table format [id, alias, replacement, type] */
  while (temp) {
    sprintf(buf1, "%ld, \"%s\", \"%s\", %d",
      GET_IDNUM(ch),
      temp->alias,
      replace_quotes(temp->replacement),
      temp->type);
    sprintf(buf2, "REPLACE aliases VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
    temp = temp->next;
  }
}

  
void sql_read_aliases(struct char_data *ch)
{
  int found = 0;
  struct alias *temp;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT COUNT(*) FROM aliases WHERE id = %ld", GET_IDNUM(ch));
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      found = atoi(entry[0]);

  mysql_free_result(table);

  if (found) {
    CREATE(GET_ALIASES(ch), struct alias, 1);
    temp = GET_ALIASES(ch); 

    sprintf(buf1, "SELECT * FROM aliases WHERE id = %ld", GET_IDNUM(ch));
    if ((table = sql_query(darkpawns, buf1, TRUE)))
      while ((entry = mysql_fetch_row(table))) {
        found--;
        temp->alias = str_dup(entry[1]);
        temp->replacement = str_dup(entry[2]);
        temp->type = atoi(entry[3]);
        if (found) {
          CREATE(temp->next, struct alias, 1);
          temp = temp->next;
        }
      }
    mysql_free_result(table);
  }
} 


void sql_remove_alias(struct char_data *ch, char *alias)
{
  if (!alias) {
    sprintf(buf, "[SQL]: Attempting to remove an unknown alias from the table.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  sprintf(buf1, "DELETE FROM aliases WHERE id = %ld AND alias = \"%s\"", GET_IDNUM(ch), alias);
  sql_query(darkpawns, buf1, FALSE);
}
