/* ************************************************************************
*  File: mail.c                                        Part of CircleMUD  *
*  Usage: Internal funcs and player spec-procs of mud-mail system         *
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
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "constants.h"
#include <mysql/mysql.h>

/* External functions */
MYSQL_RES *sql_query(MYSQL *db, const char *query, bool retval);

/* External Variables */
extern MYSQL *darkpawns;
extern int no_mail;

/* Determine the total number of letters in the table */
void sql_count_mail(void)
{
  ush_int found = 0;

  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT COUNT(*) FROM mail");
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      found = atoi(entry[0]);

  mysql_free_result(table);
  log("  Mail table read -- %d messages.", found);
}

/*
 * A simple little function which tells you if the guy has mail or not.
 */
int sql_has_mail(long recipient)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM mail WHERE id = %ld", recipient);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      return (TRUE);

  return (FALSE);
}


void sql_write_mail(long to, long from, char *message)
{
  extern char *replace_quotes(char *text);

  if (from < 0 || to < 0 || !*message) {
    log("SYSERR: Mail system -- non-fatal error. (from == %ld, to == %ld)", from, to);
    return;
  }

  /* Table format [id from, id to, date, message] */
  sprintf(buf1, "%ld, %ld, %ld, \"%s\"",
    from,
    to,
    time(0),
    replace_quotes(message));
  sprintf(buf2, "REPLACE mail VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_delete_mail(long id)
{
  sprintf(buf1, "DELETE FROM mail WHERE id = %ld", id);
  sql_query(darkpawns, buf1, FALSE);
}


/* Create the mail action description (ie. header and text) so that it can be
read by the player */
char *create_mail(MYSQL_ROW mail)
{
  time_t date = atoi(mail[2]);

  sprintf(buf, " * * * * Dark Pawns Mail System * * * *\r\n"
	  "Date: %s\r\n"
	  "  To: %s\r\n"
	  "From: %s\r\n\r\n"
    "%s\r\n",
    asctime(localtime(&date)),
    get_name_by_id(atoi(mail[1])) ? get_name_by_id(atoi(mail[1])) : "Unknown",
    get_name_by_id(atoi(mail[0])) ? get_name_by_id(atoi(mail[0])) : "Unknown",
    mail[3]);

  return (buf);
}

/****************************************************************
* Below is the spec_proc for a postmaster using the above       *
* routines.  Written by Jeremy Elson (jelson@server.cs.jhu.edu) *
****************************************************************/

void postmaster_send_mail(struct char_data * ch, struct char_data *mailman, char *arg)
{
  long recipient;
  char buf[256], **write;

  if (GET_LEVEL(ch) < MIN_MAIL_LEVEL) {
    sprintf(buf, "$n tells you, 'Sorry, you have to be level %d to send mail!'",
	    MIN_MAIL_LEVEL);
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  one_argument(arg, buf);

  if (!*buf) {			/* you'll get no argument from me! */
    act("$n tells you, 'You need to specify an addressee!'", FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  if (GET_GOLD(ch) < STAMP_PRICE) {
    sprintf(buf, "$n tells you, 'A stamp costs %d coins.'\r\n"
	    "$n tells you, '...which I see you can't afford.'", STAMP_PRICE);
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  if ((recipient = get_id_by_name(buf)) < 0) {
    act("$n tells you, 'No one by that name is registered here!'",
      FALSE, mailman, 0, ch, TO_VICT);
    return;
  }
  act("$n starts to write some mail.", TRUE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "$n tells you, 'I'll take %d coins for the stamp.'\r\n"
    "$n tells you, 'Write your message, use @ on a new line when done.'",
	  STAMP_PRICE);

  act(buf, FALSE, mailman, 0, ch, TO_VICT);
  GET_GOLD(ch) -= STAMP_PRICE;
  SET_BIT(PLR_FLAGS(ch), PLR_MAILING);  /* string_write() sets writing. */

  /* Start writing! */
  CREATE(write, char *, 1);
  string_write(ch->desc, write, MAX_MAIL_SIZE, recipient, NULL);
}


void postmaster_check_mail(struct char_data * ch, struct char_data *mailman)
{
  if (sql_has_mail(GET_IDNUM(ch)))
    sprintf(buf, "$n tells you, 'You have mail waiting.'");
  else
    sprintf(buf, "$n tells you, 'Sorry, you don't have any mail waiting.'");
  act(buf, FALSE, mailman, 0, ch, TO_VICT);
}


void postmaster_receive_mail(struct char_data * ch, struct char_data *mailman)
{
  struct obj_data *obj;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if (!sql_has_mail(GET_IDNUM(ch))) {
    sprintf(buf, "$n tells you, 'Sorry, you don't have any mail waiting.'");
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
    return;
  }

  sprintf(buf1, "SELECT * FROM mail WHERE id = %ld", GET_IDNUM(ch));
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table))) {
      obj = create_obj();
      obj->item_number = NOTHING;
      obj->name = str_dup("mail paper letter");
      obj->short_description = str_dup("a piece of mail");
      obj->description = str_dup("Someone has left a piece of mail here.");

      GET_OBJ_TYPE(obj) = ITEM_NOTE;
      SET_BIT(GET_OBJ_WEAR(obj), ITEM_WEAR_TAKE);
      SET_BIT(GET_OBJ_WEAR(obj), ITEM_WEAR_HOLD);
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NOPERSIST);
      GET_OBJ_WEIGHT(obj) = 1;
      GET_OBJ_COST(obj) = 30;
      GET_OBJ_LOAD(obj) = 100;
      obj->action_description = str_dup(create_mail(entry));
      obj_to_char(obj, ch);

      act("$n gives you a piece of mail.", FALSE, mailman, 0, ch, TO_VICT);
      act("$N gives $n a piece of mail.", FALSE, ch, 0, mailman, TO_ROOM);
    }

  sql_delete_mail(GET_IDNUM(ch));
  mysql_free_result(table);
}

SPECIAL(postmaster)
{
  if (!ch->desc || IS_NPC(ch))
    return (0);			/* so mobs don't get caught here */

  if (!(CMD_IS("mail") || CMD_IS("check") || CMD_IS("receive"))) 
    return (0);

  if (no_mail) {
    send_to_char("Sorry, the mail system is having technical difficulties.\r\n", ch);
    return (0);
  }

  if (CMD_IS("mail")) {
    postmaster_send_mail(ch, (struct char_data *)me, argument);
    return (1);
  } else if (CMD_IS("check")) {
    postmaster_check_mail(ch, (struct char_data *)me);
    return (1);
  } else if (CMD_IS("receive")) {
    postmaster_receive_mail(ch, (struct char_data *)me);
    return (1);
  } else
    return (0);
}
