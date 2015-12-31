/*
 * Originally written by: Michael Scott -- Manx.
 * Last known e-mail address: scottm@workcomm.net
 *
 * XXX: This needs Oasis-ifying.
 */

/* $Id: /local/dp2.2/src/db.c 1285 2008-04-23T02:27:13.368876Z jsravn  $ */

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "db.h"
#include "olc.h"
#include "improved-edit.h"
#include "file-edit.h"

extern char *credits;
extern char *news;
extern char *motd;
extern char *imotd;
extern char *help;
extern char *info;
extern char *background;
extern char *handbook;
extern char *policies;
extern char *future;
extern char *wizlist;
extern char *immlist;

ACMD(do_tedit)
{
  int l, i = 0;
  char field[MAX_INPUT_LENGTH];

  struct {
    char *cmd;
    char level;
    char **buffer;
    int  size;
    char *filename;
  } fields[] = {
    /* edit the lvls to your own needs */
    { "credits",    LVL_IMPL,   &credits,   2400,   CREDITS_FILE},
    { "news",   LVL_GOD,    &news,      8192,   NEWS_FILE},
    { "motd",   LVL_GOD,    &motd,      2400,   MOTD_FILE},
    { "imotd",  LVL_IMMORT, &imotd,     2400,   IMOTD_FILE},
    { "help",       LVL_GOD,    &help,      2400,   HELP_PAGE_FILE},
    { "info",   LVL_GOD,    &info,      8192,   INFO_FILE},
    { "background", LVL_GRGOD,  &background,    8192,   BACKGROUND_FILE},
    { "handbook",   LVL_GRGOD,  &handbook,  8192,   HANDBOOK_FILE},
    { "policies",   LVL_IMPL,   &policies,  8192,   POLICIES_FILE},
    { "future", LVL_GRGOD,  &future,        8192,   FUTURE_FILE},
    { "wizlist",    LVL_IMPL,   &wizlist,   2400,   WIZLIST_FILE},
    { "immlist",    LVL_IMPL,   &immlist,   2400,   IMMLIST_FILE},
    { "\n",     0,      NULL,       0,  NULL }
  };

  if (ch->desc == NULL)
    return;

  one_argument(argument, field);

  if (!*field) {
    send_to_char("Files available to be edited:\r\n", ch);
    for (l = 0; *fields[l].cmd != '\n'; l++) {
      if (GET_LEVEL(ch) >= fields[l].level) {
        sprintf(buf, "%-11.11s ", fields[l].cmd);
    send_to_char(buf, ch);
    if (!(++i % 6))
      send_to_char("\r\n", ch);
      }
    }
    if (i % 6)
      send_to_char("\r\n", ch);
    if (i == 0)
      send_to_char("None.\r\n", ch);
    return;
  }
  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (*fields[l].cmd == '\n') {
    send_to_char("Invalid text editor option.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }

  general_file_edit(ch, fields[l].filename, fields[l].buffer, fields[l].size);
}
