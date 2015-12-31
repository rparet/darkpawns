/* ************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996-99 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

/* $Id: modify.c 1512 2008-06-06 03:57:50Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "mail.h"
#include "boards.h"
#include "olc.h"
#include "improved-edit.h"
#include "tedit.h"
#include "file-edit.h"

/* Cleanup utilities exported from redit, medit, and oedit */
void redit_string_cleanup(struct descriptor_data *d, int action);
void medit_string_cleanup(struct descriptor_data *d, int action);
void oedit_string_cleanup(struct descriptor_data *d, int action);

/* local functions */
void show_string(struct descriptor_data *d, char *input);
int old_search_block(char *argument, int begin, int length, char **list,
                     int mode);
char *clean_up (char *in);
char *find_exdesc(char *word, struct extra_descr_data * list);
void playing_string_cleanup(struct descriptor_data *d, int action);
void exdesc_string_cleanup(struct descriptor_data *d, int action);

char *string_fields[] =
{
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};


/* maximum length for text field x+1 */
int length[] =
{
  15,
  60,
  256,
  240,
  60
};

void smash_tilde(char *str)
{
  /*
   * Erase any _line ending_ tildes inserted in the editor.
   * The load mechanism can't handle those, yet.
   * -- Welcor 04/2003
   */

   char *p = str;
   for (; *p; p++)
     if (*p == '~' && (*(p+1)=='\r' || *(p+1)=='\n' || *(p+1)=='\0'))
       *p=' ';
}

void
string_write(struct descriptor_data *d, char **writeto, size_t len,
                  long mail_to, void *data)
{
  if (d->character && !IS_NPC(d->character))
    SET_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);

  if (using_improved_editor)
    d->backstr = (char *)data;
  else if (data)
    free(data);

  d->str = writeto;
  d->max_str = len;
  d->mail_to = mail_to;
}

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
  int action;

  /* determine if this is the terminal string, and truncate if so */
  /* changed to only accept '@' at the beginning of line - J. Elson 1/17/94 */

  delete_doubledollar(str);
  smash_tilde(str);

  if ((action = (*str == '@')))
    *str = '\0';
  else
    if ((action = improved_editor_execute(d, str)) == STRINGADD_ACTION)
      return;

  if (action != STRINGADD_OK)
    /* Do nothing. */ ;
  else if (!(*d->str)) {
    if (strlen(str) + 3 > d->max_str) {
      send_to_char("String too long - Truncated.\r\n",
           d->character);
      strcpy(&str[d->max_str - 3], "\r\n");
      CREATE(*d->str, char, d->max_str);
      strcpy(*d->str, str);
      if (!using_improved_editor)
    action = STRINGADD_SAVE;
    } else {
      CREATE(*d->str, char, strlen(str) + 3);
      strcpy(*d->str, str);
    }
  } else {
    if (strlen(str) + strlen(*d->str) + 3 > d->max_str) {
      send_to_char("String too long.  Last line skipped.\r\n", d->character);
      if (!using_improved_editor)
    action = STRINGADD_SAVE;
      else if (action == STRINGADD_OK)
    action = STRINGADD_ACTION;
    } else {
      RECREATE((*d->str), char, (strlen(*d->str) + strlen(str) + 3));
      strcat(*d->str, str);
    }
  }

  /* Common cleanup */
  switch (action) {
  case STRINGADD_ABORT:
    switch (STATE(d)) {
    case CON_TEDIT:
    case CON_MEDIT:
    case CON_OEDIT:
    case CON_REDIT:
    case CON_EXDESC:
      FREE(*d->str);
      *d->str = d->backstr;
      d->backstr = NULL;
      d->str = NULL;
      break;
    default:
      log("SYSERR: string_add: Aborting write from unknown origin.");
      break;
    }
    break;
  case STRINGADD_SAVE:
    if (d->backstr)
      free(d->backstr);
    d->backstr = NULL;
    break;
  case STRINGADD_ACTION:
    break;
  }

  if (action == STRINGADD_SAVE || action == STRINGADD_ABORT) {
    int i;
    struct {
      int mode;
      void (*func)(struct descriptor_data *d, int action);
    } cleanup_modes[] = {
      { CON_MEDIT  , medit_string_cleanup },
      { CON_OEDIT  , oedit_string_cleanup },
      { CON_REDIT  , redit_string_cleanup },
      { CON_TEDIT  , tedit_string_cleanup },
      { CON_EXDESC , exdesc_string_cleanup },
      { CON_PLAYING, playing_string_cleanup },
      { -1, NULL }
    };

    for (i = 0; cleanup_modes[i].func; i++)
      if (STATE(d) == cleanup_modes[i].mode)
        (*cleanup_modes[i].func)(d, action);

    /* Common post cleanup code. */
    d->str = NULL;
    d->mail_to = 0;
    d->max_str = 0;

    if (d->character && !IS_NPC(d->character)) {
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
    }
  } else if (action != STRINGADD_ACTION && strlen(*d->str) + 3 <= d->max_str) /* 3 = \r\n\0 */
    strcat(*d->str, "\r\n");
}

void playing_string_cleanup(struct descriptor_data *d, int action)
{
  if (PLR_FLAGGED(d->character, PLR_MAILING)) {
    if (action == STRINGADD_SAVE && *d->str) {
      store_mail(d->mail_to, GET_IDNUM(d->character), *d->str);
      write_to_output(d, "Message sent!\r\n");
    } else
      write_to_output(d, "Mail aborted.\r\n");
      free(*d->str);
      free(d->str);
    }

  /*
   * We have no way of knowing which slot the post was sent to so we can only give the message...
   */
    if (d->mail_to >= BOARD_MAGIC) {
      Board_save_board(d->mail_to - BOARD_MAGIC);
      if (action == STRINGADD_ABORT)
        write_to_output(d, "Post not aborted, use REMOVE <post #>.\r\n");
    }
}

void exdesc_string_cleanup(struct descriptor_data *d, int action)
{
  extern char *MENU;

  if (action == STRINGADD_ABORT)
    write_to_output(d, "Description aborted.\r\n");

  write_to_output(d, "%s", MENU);
  STATE(d) = CON_MENU;
}

/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

ACMD(do_skillset)
{
  extern char *spells[];
  struct char_data *vict;
  char name[100], buf2[100], buf[100], help[MAX_STRING_LENGTH];
  int skill, value, i, qend;

  argument = one_argument(argument, name);

  if (!*name) {         /* no arguments. print an informative text */
    send_to_char("Syntax: skillset <name> '<skill>' <value>\r\n", ch);
    strcpy(help, "Skill being one of the following:\n\r");
    for (i = 0; *spells[i] != '\n'; i++) {
      if (*spells[i] == '!')
    continue;
      sprintf(help + strlen(help), "%18s", spells[i]);
      if (i % 4 == 3) {
    strcat(help, "\r\n");
    send_to_char(help, ch);
    *help = '\0';
      }
    }
    if (*help)
      send_to_char(help, ch);
    send_to_char("\n\r", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  skip_spaces(&argument);

  /* If there is no chars in argument */
  if (!*argument) {
    send_to_char("Skill name expected.\n\r", ch);
    return;
  }
  if (*argument != '\'') {
    send_to_char("Skill must be enclosed in: ''\n\r", ch);
    return;
  }
  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    send_to_char("Skill must be enclosed in: ''\n\r", ch);
    return;
  }
  strcpy(help, (argument + 1));
  help[qend - 1] = '\0';
  if ((skill = find_skill_num(help)) <= 0) {
    send_to_char("Unrecognized skill.\n\r", ch);
    return;
  }
  argument += qend + 1;     /* skip to next parameter */
  argument = one_argument(argument, buf);

  if (!*buf) {
    send_to_char("Learned value expected.\n\r", ch);
    return;
  }
  value = atoi(buf);
  if (value < 0) {
    send_to_char("Minimum value for learned is 0.\n\r", ch);
    return;
  }
  if (value > 100) {
    send_to_char("Max value for learned is 100.\n\r", ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char("You can't set NPC skills.\n\r", ch);
    return;
  }
  sprintf(buf2, "%s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict),
      spells[skill], value);
  mudlog(buf2, BRF, -1, TRUE);

  SET_SKILL(vict, skill, value);

  sprintf(buf2, "You change %s's %s to %d.\n\r", GET_NAME(vict),
      spells[skill], value);
  send_to_char(buf2, ch);
}



/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

/* Traverse down the string until the begining of the next page has been
 * reached.  Return NULL if this is the last page of the string.
 */
char *next_page(char *str)
{
  int col = 1, line = 1, spec_code = FALSE;

  for (;; str++) {
    /* If end of string, return NULL. */
    if (*str == '\0')
      return NULL;

    /* If we're at the start of the next page, return this fact. */
    else if (line > PAGE_LENGTH)
      return str;

    /* Check for the begining of an ANSI color code block. */
    else if (*str == '\x1B' && !spec_code)
      spec_code = TRUE;

    /* Check for the end of an ANSI color code block. */
    else if (*str == 'm' && spec_code)
      spec_code = FALSE;

    /* Check for everything else. */
    else if (!spec_code) {
      /* Carriage return puts us in column one. */
      if (*str == '\r') {
    col = 1;
      }
      /* Newline puts us on the next line. */
      else if (*str == '\n') {
    col = 1;
    line++;
      }

      /* We need to check here and see if we are over the page width,
       * and if so, compensate by going to the begining of the next line.
       */
      else if (col++ > PAGE_WIDTH) {
    col = 1;
    line++;
      }
    }
  }
}

/* Function that returns the number of pages in the string. */
int count_pages(char *str)
{
  int pages;

  for (pages = 1; (str = next_page(str)); pages++);
  return pages;
}


/* This function assigns all the pointers for showstr_vector for the
 * page_string function, after showstr_vector has been allocated and
 * showstr_count set.
 */
void
paginate_string(char *str, struct descriptor_data *d)
{
  int i;

  if (d->showstr_count)
    *(d->showstr_vector) = str;

  for (i = 1; i < d->showstr_count && str; i++)
    str = d->showstr_vector[i] = next_page(str);

  d->showstr_page = 0;
}


/* The call that gets the paging ball rolling... */
void
page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  if (!d)
    return;

  if (!str || !*str) {
    send_to_char("", d->character);
    return;
  }

  CREATE(d->showstr_vector, char *, d->showstr_count = count_pages(str));

  if (keep_internal) {
    d->showstr_head = str_dup(str);
    paginate_string(d->showstr_head, d);
  } else
    paginate_string(str, d);

  show_string(d, "");
}


/* The call that displays the next page. */
void
show_string(struct descriptor_data *d, char *input)
{
  char buffer[MAX_STRING_LENGTH];
  int diff;

  any_one_arg(input,buf);

  /* Q is for quit. :) */
  if (LOWER(*buf) == 'q')
  {
     FREE(d->showstr_vector);
     d->showstr_count = 0;
     if (d->showstr_head)
     {
        FREE(d->showstr_head);
        d->showstr_head = 0;
     }
     return;
  }
  /* R is for refresh, so back up one page internally so we can display
   * it again.
   */
  else if (LOWER(*buf) == 'r')
  {
     d->showstr_page = MAX(0, d->showstr_page - 1);
  }

  /* B is for back, so back up two pages internally so we can display the
   * correct page here.
   */
  else if (LOWER(*buf) == 'b')
  {
     d->showstr_page = MAX(0, d->showstr_page - 2);
  }

  /* Feature to 'goto' a page.  Just type the number of the page and you
   * are there!
   */
  else if (isdigit(*buf))
  {
     d->showstr_page = MAX(0, MIN(atoi(buf) - 1, d->showstr_count - 1));
  }

  else if (*buf)
  {
     send_to_char("Valid commands while paging are RETURN, Q, R, B, or a "
                  "numeric value.\r\n", d->character);
     return;
  }
  /* If we're displaying the last page, just send it to the character, and
   * then free up the space we used.
   */
  if (d->showstr_page + 1 >= d->showstr_count)
  {
     send_to_char(d->showstr_vector[d->showstr_page], d->character);
     FREE(d->showstr_vector);
     d->showstr_count = 0;
     if (d->showstr_head)
     {
        FREE(d->showstr_head);
        d->showstr_head = NULL;
     }
  }
  /* Or if we have more to show.... */
  else
  {
     strncpy(buffer, d->showstr_vector[d->showstr_page],
             diff = ((int) d->showstr_vector[d->showstr_page + 1])
             - ((int) d->showstr_vector[d->showstr_page]));
     buffer[diff] = '\0';
     send_to_char(buffer, d->character);
     d->showstr_page++;
  }
}

/* do_string stuff ***********************************************/

#define TP_MOB    0
#define TP_OBJ    1
#define TP_ERROR  2
extern struct obj_data *object_list;

/*search the entire world for an object, and return a pointer  */
struct obj_data *
get_obj(char *name)
{
   struct obj_data *i;
   int  j, number;
   char tmpname[MAX_INPUT_LENGTH];
   char *tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = object_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->name)) {
         if (j == number)
            return(i);
         j++;
      }

   return(0);
}


/* interpret an argument for do_string */
void    quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
   char buf[MAX_STRING_LENGTH];

   /* determine type */
   arg = one_argument(arg, buf);
   if (is_abbrev(buf, "mob"))
      *type = TP_MOB;
   else if (is_abbrev(buf, "obj"))
      *type = TP_OBJ;
   else {
      *type = TP_ERROR;
      return;
   }

   /* find name */
   arg = one_argument(arg, name);

   /* field name and number */
   arg = one_argument(arg, buf);
   if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
      return;

   /* string */
   for (; isspace(*arg); arg++)
      ;
   for (; (*string = *arg); arg++, string++)
      ;

   return;
}
/* modification of CREATED'ed strings in chars/objects */
ACMD(do_string)
{
   char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
   int  field, type;
   struct char_data *mob;
   struct obj_data *obj;
   struct extra_descr_data *ed, *tmp;

   if (IS_NPC(ch))
      return;

   quad_arg(argument, &type, name, &field, string);

   if (type == TP_ERROR) {
     send_to_char("Usage: string ('obj'|'mob') <name> <field> [<string>]", ch);
     return;
   }

   if (!field) {
      send_to_char("No field by that name. Try 'help string'.\n\r", ch);
      return;
   }

   if (type == TP_MOB) {

      /* locate the beast */
      if (!(mob = get_char_vis(ch, name))) {
     send_to_char("I don't know anyone by that name...\n\r", ch);
     return;
      }

      switch (field) {
      case 1:
     if (!IS_NPC(mob) && GET_LEVEL(ch) < LEVEL_IMPL-1) {
        send_to_char("You can't change that field for players.", ch);
        return;
     }
     ch->desc->str = &(mob->player.name);
     if (!IS_NPC(mob))
        send_to_char("WARNING: You have changed the name of a player.\n\r", ch);
     break;
      case 2:
     if (!IS_NPC(mob)) {
        send_to_char("That field is for monsters only.\n\r", ch);
        return;
     }
     ch->desc->str = &mob->player.short_descr;
     break;
      case 3:
     if (!IS_NPC(mob)) {
        send_to_char("That field is for monsters only.\n\r", ch);
        return;
     }
         strcpy(string + strlen(string), "\r\n");
     ch->desc->str = &mob->player.long_descr;
     break;
      case 4:
     ch->desc->str = &mob->player.description;
     break;
      case 5:
     if (IS_NPC(mob)) {
        send_to_char("Monsters have no titles.\n\r", ch);
        return;
     }
     ch->desc->str = &mob->player.title;
     break;
      default:
     send_to_char("That field is undefined for monsters.\n\r", ch);
     return;
     break;
      }
   } else /* type == TP_OBJ */   {

      /* locate the object */
      if (!(obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
     send_to_char("Nothing by that name..\n\r", ch);
     return;
      }

      switch (field) {
      case 1:
     ch->desc->str = &obj->name;
     break;
      case 2:
     ch->desc->str = &obj->short_description;
     break;
      case 3:
     ch->desc->str = &obj->description;
     break;
      case 4:
     if (!*string) {
        send_to_char("You have to supply a keyword.\n\r", ch);
        return;
     }
     /* try to locate extra description */
     for (ed = obj->ex_description; ; ed = ed->next)
        if (!ed) /* the field was not found. create a new one. */ {
           CREATE(ed , struct extra_descr_data, 1);
           ed->next = obj->ex_description;
           obj->ex_description = ed;
           CREATE(ed->keyword, char, strlen(string) + 1);
           strcpy(ed->keyword, string);
           ed->description = 0;
           ch->desc->str = &ed->description;
           send_to_char("New field.\n\r", ch);
           break;
        }
        else if (!str_cmp(ed->keyword, string)) /* the field exists */ {
           FREE(ed->description);
           ed->description = 0;
           ch->desc->str = &ed->description;
           send_to_char("Modifying description.\n\r", ch);
           break;
        }
     ch->desc->max_str = MAX_STRING_LENGTH;
     return; /* the stndrd (see below) procedure does not apply here */
     break;
      case 6: /* delete-description */
     if (!*string) {
        send_to_char("You must supply a field name.\n\r", ch);
        return;
     }
     /* try to locate field */
        ed = obj->ex_description;
        if (!find_exdesc(string, ed)) {
           send_to_char("No field with that keyword.\n\r", ch);
           return;
        }
        else if (isname_with_abbrevs(string, ed->keyword)) {
           FREE(ed->keyword);
           if (ed->description)
          FREE(ed->description);

           /* delete the entry in the desr list */
           if (ed == obj->ex_description)
          obj->ex_description = ed->next;
           else {
          for (tmp = obj->ex_description; tmp->next != ed;
              tmp = tmp->next)
             ;
          tmp->next = ed->next;
           }
           FREE(ed);

           send_to_char("Field deleted.\n\r", ch);
           return;
        }
     break;
      default:
     send_to_char(
         "That field is undefined for objects.\n\r", ch);
     return;
     break;
      }
   }
/*
   if (*ch->desc->str) {
      FREE(*ch->desc->str);
   }
*/
/*  obj->item_number = -1; */

   if (*string)   /* there was a string in the argument array */ {
      if (strlen(string) > length[field - 1]) {
     send_to_char("String too long - truncated.\n\r", ch);
     *(string + length[field - 1]) = '\0';
      }
      CREATE(*ch->desc->str, char, strlen(string) + 1);
      strcpy(*ch->desc->str, string);
/*      strcpy(*ch->desc->str + strlen(string), "\r\n"); */
      ch->desc->str = 0;
      send_to_char("Ok.\n\r", ch);
   } else /* there was no string. enter string mode */   {
      send_to_char("Enter string. terminate with '@'.\n\r", ch);
      *ch->desc->str = 0;
      ch->desc->max_str = length[field - 1];
   }
}


#define RETURN_CHAR '|'
char
*clean_up (char *in)
{
  char buf[MAX_STRING_LENGTH];
  int notdone, linelen, bufi, ini, wordlen, whitespacelen, exit, word,
    letter, endspacelen;
  int width = 78;

  notdone = 1;
  linelen = 0;
  bufi = 0;
  ini = 0;
  word = 0;
  strcpy (buf, in);

  while (notdone)  {
      wordlen = 0;
      whitespacelen = 0;
      endspacelen = 0;
      word++;

      while (buf[bufi] == ' ')
        {                       /* snarf initial whitespace */
          in[ini++] = buf[bufi++];
          whitespacelen++;
        }

      exit = 0;
      letter = 0;
      while (buf[bufi] != ' ' && !exit)
      {
          letter++;
          switch (buf[bufi])
            {
            case '\n':
            case '\r':
              if (letter > 1 && word > 1)
                {
                  in[ini++] = ' ';
                  whitespacelen++;
                }
              endspacelen++;
              bufi++;
              while (buf[bufi] == '\n' || buf[bufi] == '\r')
                {
                  bufi++;
                  endspacelen++;
                }
              exit = 1;
              break;
            case '\0':
              notdone = 0;
              linelen = 0;
              in[ini] = '\0';
              exit = 1;
              break;

            case RETURN_CHAR:
              if (letter > 1)
                {
                  exit = 1;
                }
              else
                {
                  in[ini++] = '\n';
                  in[ini++] = '\r';
                  bufi++;
                  word = 0;
                  linelen = 0;
                  endspacelen = 0;
                  exit = 1;
                }
              break;

            default:
              in[ini++] = buf[bufi++];
              wordlen++;
            }
        }
      linelen += (wordlen + whitespacelen);

      if (linelen >= width)
        {
          bufi -= (wordlen + endspacelen);
          ini -= (wordlen + whitespacelen);

          in[ini++] = '\n';
          in[ini++] = '\r';
          linelen = 0;
          word = 0;
        }
    }                           /* end outer while */
  strcpy(in, buf);
  return in;
}
