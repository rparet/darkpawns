/* ************************************************************************
*   File: ban.c                                         Part of CircleMUD *
*  Usage: banning/unbanning/checking sites and player names               *
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

/* $Id: ban.c 1273 2005-12-20 05:19:42Z mcooper $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

/* External Functions */
void sql_save_banned(void);
void sql_delete_ban(char *name, bool account);

/* External Variables */
extern struct ban_list_element *ban_list;

/* functions in this file */
char wc_match(char *string, char *pattern);
char *ban_types[] = {
  "no",
  "new",
  "select",
  "all",
  "temp",
  "ERROR"
};

/* function depricated with introduction of SQL table - mcooper 09042004
void load_banned(void)
{
  FILE *fl;
  int i, date;
  char site_name[BANNED_SITE_LENGTH + 1], ban_type[100], comment[BANNED_COMMENT_LENGTH+1];
  char name[MAX_NAME_LENGTH + 1];
  struct ban_list_element *next_node;

  ban_list = 0;

  if (!(fl = fopen(BAN_FILE, "r"))) {
    perror("Unable to open banfile");
    return;
  }
  while (fscanf(fl, " %s %s %d %s ", ban_type, site_name, &date, name) == 4) {
    get_line(fl, comment);
    CREATE(next_node, struct ban_list_element, 1);
    strncpy(next_node->site, site_name, BANNED_SITE_LENGTH);
    next_node->site[BANNED_SITE_LENGTH] = '\0';
    strncpy(next_node->name, name, MAX_NAME_LENGTH);
    next_node->name[MAX_NAME_LENGTH] = '\0';
    strncpy(next_node->comment, comment, BANNED_COMMENT_LENGTH);
    next_node->comment[BANNED_COMMENT_LENGTH] = '\0';
    next_node->date = date;

    for (i = BAN_NOT; i <= BAN_ALL; i++)
      if (!strcmp(ban_type, ban_types[i]))
	next_node->type = i;

    next_node->next = ban_list;
    ban_list = next_node;
  }

  fclose(fl);
} */


int isbanned(char *hostname, bool site)
{
  int i = 0;
  long time_left;
  struct ban_list_element *banned_node, *temp;
  char *nextchar;

  if (!hostname || !*hostname)
    return (0);

  /* Was a site specified, or is it an account? */
  if (site) {
    for (nextchar = hostname; *nextchar; nextchar++)
      *nextchar = LOWER(*nextchar);

    for (banned_node = ban_list; banned_node; banned_node = banned_node->next)
      if (wc_match(hostname, banned_node->site))
        i = MAX(i, banned_node->type);
  } else {
    for (nextchar = hostname; *nextchar; nextchar++)
      *nextchar = LOWER(*nextchar);

    for (banned_node = ban_list; banned_node; banned_node = banned_node->next)
      if (wc_match(hostname, banned_node->account)) {
        time_left = (banned_node->date + (banned_node->length * SECS_PER_REAL_HOUR)) - time(0);
        if (time_left >= 0) {
          time_left = time_left / SECS_PER_REAL_HOUR;
          i = MAX(i, time_left+1);
        } else {
          sql_delete_ban(hostname, TRUE);
          REMOVE_FROM_LIST(banned_node, ban_list, next);
          sprintf(buf, "Temporary ban on account '%s' removed.", hostname);
          mudlog(buf, NRM, LVL_IMMORT, TRUE);
        }
      }
  }

  return i;
}

/* 
 wc_match returns TRUE if string matches pattern, and FALSE
 otherwise.  20020730 -rparet

 Adapted from code by Daniel Stenberg <daniel@haxx.se>
 which in turn was adapted from possibly the shorted implementation ever
 at 88 characters written by Staffan Ulfberg <staffanu@tcs07.nada.kth.se>:

 m(char*s,char*t){return*t-'*'?*s?*t=='?'|*s==*t&&m(s+1,t+1):!*t:m(s,t+1)||*s&&m(s+1,t);}
 
*/

char 
wc_match(char *string, char *pattern)
{
  for (; '*'^*pattern; ++pattern, ++string) {
    if (!*string)
      return (!*pattern);
    if (toupper(*string)^toupper(*pattern) && '?'^*pattern)
      return FALSE;
  }
  /* two-line patch to prevent *too* much recursiveness: */
  while('*' == pattern[1])
    pattern++;
  do {
    if ( wc_match(string, pattern + 1) )
      return TRUE;
  } while (*string++);
  return FALSE;
}

/* function depricated - mcooper 09042004
void _write_one_node(FILE * fp, struct ban_list_element * node)
{
  if (node) {
    _write_one_node(fp, node->next);
    fprintf(fp, "%s %s %ld %s %s\n", ban_types[node->type],
	    node->site, (long) node->date, node->name, node->comment);
  }
} */


/* function depricated - mcooper 09042004
void write_ban_list(void)
{
  FILE *fl;

  if (!(fl = fopen(BAN_FILE, "w"))) {
    perror("write_ban_list");
    return;
  }
  _write_one_node(fl, ban_list);  recursively write from end to start
  fclose(fl);
  return;
} */


#define BAN_LIST_FORMAT "%-20.20s  %-6.6s  %-9.9s  %-10.10s  %-30.30s\r\n"
ACMD(do_ban)
{
  char flag[MAX_INPUT_LENGTH], site[MAX_INPUT_LENGTH], *nextchar, *comment;
  int i;
  struct ban_list_element *ban_node;
  struct tm *ban_time;
  bool is_account = FALSE;
  struct account_data *account = NULL;
  struct account_data *sql_check_account(char *name);

  *buf = '\0';
  *buf2 = '\0';
  *buf3 = '\0';

  if (!*argument) {
    if (!ban_list) {
      send_to_char("No sites or accounts are banned.\r\n", ch);
      return;
    }
    sprintf(buf, BAN_LIST_FORMAT,
	    "Banned Site/Account",
	    "Type",
	    "Banned On",
	    "Banned By",
      "Comment");
    send_to_char(buf, ch);
    sprintf(buf, BAN_LIST_FORMAT,
	    "---------------------------------",
      "---------------------------------",
	    "---------------------------------",
	    "---------------------------------",
	    "---------------------------------");
    send_to_char(buf, ch);

    for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
      if (ban_node->date) {
        ban_time = localtime(&(ban_node->date));
        sprintf(buf2, "%d/%d/%d", ban_time->tm_mon, ban_time->tm_mday, ban_time->tm_year+1900);
      } else
	      sprintf(buf2, "Unknown");
      sprintf(buf, BAN_LIST_FORMAT,
        *(ban_node->site) ? ban_node->site : ban_node->account,
        ban_types[ban_node->type],
        buf2, ban_node->name,
        ban_node->comment);
      send_to_char(buf, ch);
    }
    return;
  }
  comment = two_arguments(argument, flag, site);
  skip_spaces(&comment);
  if (!*site || !*flag) {
    send_to_char("Usage: ban {all | select | new | <time period>} site/account comment\r\n", ch);
    return;
  }
  if (!(!str_cmp(flag, "select") || !str_cmp(flag, "all") || !str_cmp(flag, "new") ||
        is_number(flag))) {
    send_to_char("Flag must be ALL, SELECT, NEW, or a specified period of time.\r\n", ch);
    return;
  }
  for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
    if (!str_cmp(ban_node->site, site)) {
      send_to_char("That site has already been banned -- unban it to change the ban type.\r\n", ch);
      return;
    } else if (!str_cmp(ban_node->account, site)) {
      send_to_char("That account has already been banned -- unban it to change the ban type.\r\n", ch);
      return;
    }
  }

  /* If the "flag" was a number, it must be a time period and hence banning an account */
  if (is_number(flag))
    is_account = TRUE;

  if (is_account)
    if (!(account = sql_check_account(site))) {
      send_to_char("There is no such account.\r\n", ch);
      return;
    }

  CREATE(ban_node, struct ban_list_element, 1);

  /* Check to see if this an account or site ban */
  if (is_account) {
    strncpy(ban_node->account, site, MAX_NAME_LENGTH);
    for (nextchar = ban_node->account; *nextchar; nextchar++)
      *nextchar = LOWER(*nextchar);
    ban_node->account[0] = toupper(ban_node->account[0]);
    ban_node->account[MAX_NAME_LENGTH] = '\0';
  } else {
    strncpy(ban_node->site, site, BANNED_SITE_LENGTH);
    for (nextchar = ban_node->site; *nextchar; nextchar++)
      *nextchar = LOWER(*nextchar);
    ban_node->site[BANNED_SITE_LENGTH] = '\0';
  }

  strncpy(ban_node->name, GET_NAME(ch), MAX_NAME_LENGTH);
  ban_node->name[MAX_NAME_LENGTH] = '\0';
  strncpy(ban_node->comment, comment, BANNED_COMMENT_LENGTH);
  ban_node->comment[BANNED_COMMENT_LENGTH] = '\0';
  ban_node->date = time(0);

  if (is_account) {
    ban_node->length = atoi(flag);
    ban_node->type = BAN_TEMP;
  } else
    for (i = BAN_NEW; i <= BAN_ALL; i++)
      if (!str_cmp(flag, ban_types[i]))
        ban_node->type = i;

  ban_node->next = ban_list;
  ban_list = ban_node;

  if (is_account)
    sprintf(buf, "%s has banned account '%s' for %d %s.", GET_NAME(ch), site, atoi(flag),
    ((atoi(flag) > 1) ? "hours" : "hour"));
  else
    sprintf(buf, "%s has banned %s for %s players.", GET_NAME(ch), site, ban_types[ban_node->type]);
  mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
  send_to_char("Site/Account banned.\r\n", ch);
  sql_save_banned();
  free_account(account);
}

ACMD(do_unban)
{
  char site[80];
  struct ban_list_element *ban_node, *temp;
  bool found = FALSE, account = FALSE;

  one_argument(argument, site);
  if (!*site) {
    send_to_char("A site or account to unban might help.\r\n", ch);
    return;
  }
  ban_node = ban_list;
  while (ban_node && !found) {
    if (!str_cmp(ban_node->site, site))
      found = TRUE;
    else if (!str_cmp(ban_node->account, site)) {
      found = TRUE;
      account = TRUE;
    } else
      ban_node = ban_node->next;
  }

  if (!found) {
    send_to_char("That site/account is currently not banned.\r\n", ch);
    return;
  }
  REMOVE_FROM_LIST(ban_node, ban_list, next);
  if (account) {
    send_to_char("Account unbanned.\r\n", ch);
    sprintf(buf, "%s removed the temporary ban on account '%s'.", GET_NAME(ch), ban_node->account);
  } else {
    send_to_char("Site unbanned.\r\n", ch);
    sprintf(buf, "%s removed the %s-player ban on %s.",
      GET_NAME(ch), ban_types[ban_node->type], ban_node->site);
  }
  mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

  FREE(ban_node);
  sql_delete_ban(site, account);
}


/**************************************************************************
 *  Code to check for invalid names (i.e., profanity, etc.)		  *
 *  Written by Sharon P. Goza						  *
 **************************************************************************/

typedef char namestring[MAX_NAME_LENGTH];

namestring *invalid_list = NULL;
int num_invalid = 0;

int valid_name(char *newname)
{
  int i;
  struct descriptor_data *d;
  char tempname[MAX_INPUT_LENGTH];

  if (!*newname)
    return 1;

  /* check for multiple logins, or a character going through creation, etc. -rparet */

  for (d = descriptor_list; d; d = d->next)
     if (d->character && GET_NAME(d->character) && !str_cmp(GET_NAME(d->character), newname))
       return (STATE(d) == CON_PLAYING);


  /* return valid if list doesn't exist */
  if (!invalid_list || num_invalid < 1)
    return 1;

  /* change to lowercase */
  strcpy(tempname, newname);
  for (i = 0; tempname[i]; i++)
    tempname[i] = LOWER(tempname[i]);

  /* Does the desired name contain a string in the invalid list? */
  for (i = 0; i < num_invalid; i++)
    if (strstr(tempname, invalid_list[i]))
      return 0;

  return 1;
}


void Read_Invalid_List(void)
{
  FILE *fp;
  int i = 0;
  char string[80];

  if (!(fp = fopen(XNAME_FILE, "r"))) {
    perror("Unable to open invalid name file");
    return;
  }
  /* count how many records */
  while (fgets(string, MAX_NAME_LENGTH, fp) != NULL && strlen(string) > 1)
    num_invalid++;

  rewind(fp);

  CREATE(invalid_list, namestring, num_invalid);

  for (i = 0; i < num_invalid; i++) {
    fgets(invalid_list[i], MAX_NAME_LENGTH, fp);	/* read word */
    invalid_list[i][strlen(invalid_list[i]) - 1] = '\0'; /* cleave off \n */
  }

  fclose(fp);
}
