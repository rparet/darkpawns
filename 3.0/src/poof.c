/* ==========================================================================
   FILE   : poof.c
   HISTORY: dlkarnes 960911 (Serapis)
            based on code done by Jeremy Hess and Chad Thompson
   ========================================================================= */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include <mysql/mysql.h>

/* External functions */
MYSQL_RES *sql_query(MYSQL *db, const char *query, bool retval);

/* External Variables */
extern MYSQL *darkpawns;

void sql_write_poofs(struct char_data *ch)
{
  char *poofin = POOFIN(ch);
  char *poofout = POOFOUT(ch);

  /* truncate those bad boys so we dont crash */
  if (poofin)
    if (strlen(poofin) > 120)
      poofin[120] = '\0';
  if (poofout)
    if (strlen(poofout) > 120)
      poofout[120] = '\0';
  
  /* Poofs table format [id, poofin, poofout] */
  sprintf(buf1, "%ld, %s, %s",
    GET_IDNUM(ch),
    poofin ? tprintf("\"%s\"", poofin) : NULL,
    poofout ? tprintf("\"%s\"", poofout) : NULL);
  sprintf(buf2, "REPLACE poofs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_read_poofs(struct char_data *ch)
{   
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM poofs WHERE id = %ld", GET_IDNUM(ch));
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      if (entry[1])
        POOFIN(ch) = str_dup(entry[1]);
      if (entry[2])
        POOFOUT(ch) = str_dup(entry[2]);
    }
  mysql_free_result(table);
} 
