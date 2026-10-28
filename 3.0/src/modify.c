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

/* $Id: modify.c 1191 2005-04-29 13:42:12Z mcooper $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "boards.h"
#include "olc.h"

/* External variables */
extern struct spell_info_type spell_info[];
extern const char *unused_spellname;	/* spell_parser.c */

/* External functions */
void show_string(struct descriptor_data *d, char *input);
int old_search_block(char *argument, int begin, int length, char **list, int mode);
char *clean_up (char *in);
char * find_exdesc(char *word, struct extra_descr_data * list);
void sql_write_mail(long to, long from, char *message);

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

void 
string_write(struct descriptor_data *d, char **writeto, size_t len, 
                  long mail_to, void *data)
{
  if (d->character && !IS_NPC(d->character))
    SET_BIT(PLR_FLAGS(d->character), PLR_WRITING);

  if (data)  /* currently no support for special data */
    mudlog("SYSERR: string_write: I don't understand special data.",
           BRF, LVL_IMMORT, TRUE);

  d->str = writeto;
  d->max_str = len;
  d->mail_to = mail_to;
}

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
  int terminator = 0;

  /* determine if this is the terminal string, and truncate if so */
  /* changed to only accept '@' at the beginning of line - J. Elson 1/17/94 */

  delete_doubledollar(str);

  if ((terminator = (*str == '@')))
    *str = '\0';

  if (!(*d->str)) {
    if (strlen(str) + 3 > d->max_str) {
      send_to_char("String too long - Truncated.\r\n",
		   d->character);
      strcpy(&str[d->max_str - 3], "\r\n");
      terminator = 1;
    }
    CREATE(*d->str, char, strlen(str) + 3);
    strcpy(*d->str, str);
  } else {
    if (strlen(str) + strlen(*d->str) + 3 > d->max_str) {
      send_to_char("String too long.  Last line skipped.\r\n", d->character);
      terminator = 1;
    } else {
      RECREATE((*d->str), char, (strlen(*d->str) + strlen(str) + 3));
      if (!(*d->str)) {
	perror("string_add");
	exit(1);
      }
      strcat(*d->str, str);
    }
  }

  if (terminator) {
    /*. OLC Edits .*/
    extern void oedit_disp_menu(struct descriptor_data *d);
    extern void oedit_disp_extradesc_menu(struct descriptor_data *d);
    extern void redit_disp_menu(struct descriptor_data *d);
    extern void redit_disp_extradesc_menu(struct descriptor_data *d);
    extern void redit_disp_exit_menu(struct descriptor_data *d);
    extern void medit_disp_menu(struct descriptor_data *d);

   
    /*  d->str = str_dup(clean_up(*d->str)); */

    if (STATE(d) == CON_MEDIT) 
    {  if (OLC_MOB(d)->player.description)
         FREE(OLC_MOB(d)->player.description);
       OLC_MOB(d)->player.description = str_dup(*d->str);
/*       OLC_MOB(d)->player.description = str_dup(clean_up(OLC_MOB(d)->player.description)); */
       FREE(*d->str);
       FREE(d->str);
       medit_disp_menu(d);
    }
    if (STATE(d) == CON_OEDIT) {
      switch(OLC_MODE(d)) {
      case OEDIT_ACTDESC:
	if (OLC_OBJ(d)->action_description)
	  FREE(OLC_OBJ(d)->action_description);
	if(strlen(*d->str)>3)
         OLC_OBJ(d)->action_description = str_dup(*d->str);
	else
	 OLC_OBJ(d)->action_description = NULL;
 	FREE(*d->str);
 	FREE(d->str);
 	oedit_disp_menu(d);
 	break;
      case OEDIT_EXTRADESC_DESCRIPTION:
 	if (OLC_DESC(d)->description)
 	  FREE(OLC_DESC(d)->description);
 	OLC_DESC(d)->description = str_dup(*d->str);
 	FREE(*d->str);
 	FREE(d->str);
 	oedit_disp_extradesc_menu(d);
      }
    }
    if (STATE(d) == CON_REDIT) {
      switch(OLC_MODE(d)) {
      case REDIT_DESC:
	if (OLC_ROOM(d)->description)
 	  FREE(OLC_ROOM(d)->description);
 	OLC_ROOM(d)->description = str_dup(*d->str);
 	FREE(*d->str);
 	FREE(d->str);
 	redit_disp_menu(d);
 	break;
      case REDIT_EXTRADESC_DESCRIPTION:
 	if (OLC_DESC(d)->description)
 	  FREE(OLC_DESC(d)->description);
 	OLC_DESC(d)->description = str_dup(*d->str);
 	FREE(*d->str);
 	FREE(d->str);
 	redit_disp_extradesc_menu(d);
 	break;
      case REDIT_EXIT_DESCRIPTION:
 	if (OLC_EXIT(d)->general_description)
 	  FREE(OLC_EXIT(d)->general_description);
 	OLC_EXIT(d)->general_description = str_dup(*d->str);
 	FREE(*d->str);
 	FREE(d->str);
 	redit_disp_exit_menu(d);
 	break;
      }
    } /*. End OLC edits .*/

    if (!d->connected && (PLR_FLAGGED(d->character, PLR_MAILING))) {
      sql_write_mail(d->mail_to, GET_IDNUM(d->character), *d->str);
      d->mail_to = 0;
      FREE(*d->str);
      FREE(d->str);
      SEND_TO_Q("Message sent!\r\n", d);
      if (!IS_NPC(d->character)) {
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING);
      }
    }
    d->str = NULL;

    if (d->mail_to >= BOARD_MAGIC) {
      Board_save_board(d->mail_to - BOARD_MAGIC);
      d->mail_to = 0;
    }
    if (!d->connected && d->character && !IS_NPC(d->character))
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
  } else if (strlen(*d->str) <= (d->max_str - 3)) /* 3 = \r\n\0 */
    strcat(*d->str, "\r\n");
}



/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

ACMD(do_skillset)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], buf2[128];
  char buf[MAX_INPUT_LENGTH];
  int group = 0, value, i, qend;

  extern const char *skill_groups[];

  argument = one_argument(argument, name);

  if (!*name) {			/* no arguments. print an informative text */
    send_to_char("Syntax: skillset <name> '<group>' <value>\r\n", ch);
    strcpy(buf, "Group being one of the following:\r\n");
    for (qend = 0, i = 1; strcmp(skill_groups[i], "\n") != 0; i++) {
      sprintf(buf + strlen(buf), "%18s", skill_groups[i]);
      if (qend++ % 4 == 3) {
	      strcat(buf, "\r\n");
	      send_to_char(buf, ch);
	      *buf = '\0';
      }
    }
    if (*buf)
      send_to_char(buf, ch);
    send_to_char("\r\n", ch);
    return;
  }

  if (!(vict = get_char_vis(ch, name))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  skip_spaces(&argument);

  /* If there is no chars in argument */
  if (!*argument) {
    send_to_char("Group name expected.\n\r", ch);
    return;
  }
  if (*argument != '\'') {
    send_to_char("Group must be enclosed in: ''\n\r", ch);
    return;
  }
  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    send_to_char("Group must be enclosed in: ''\n\r", ch);
    return;
  }
  strcpy(buf, (argument + 1));
  buf[qend - 1] = '\0';
  for (i = 1; strcmp(skill_groups[i], "\n") != 0; i++) {
    strcpy(buf2, skill_groups[i]);
    if (!str_cmp(buf, buf2)) {
      group = i;
      break;
    }
  }

  if (!group) {
    send_to_char("Unrecognized group.\n\r", ch);
    return;
  }
  argument += qend + 1;		/* skip to next parameter */
  argument = one_argument(argument, buf2);

  if (!*buf2) {
    send_to_char("Learned value expected.\n\r", ch);
    return;
  }
  value = atoi(buf2);
  if (value < 0) {
    send_to_char("Minimum value for learned is 0.\n\r", ch);
    return;
  }

  if (IS_NPC(vict)) {
    send_to_char("You can't set NPC skills.\n\r", ch);
    return;
  }
  sprintf(buf2, "%s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict), 
    skill_groups[group], value);
  mudlog(buf2, BRF, -1, TRUE);

  SET_SKILL(vict, group, value);

  sprintf(buf2, "You change %s's %s to %d.\r\n", GET_NAME(vict), skill_groups[group], value);
  send_to_char(buf2, ch);
}



/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

#define PAGE_LENGTH     22
#define PAGE_WIDTH      80

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
      if (*str == '\r')
	col = 1;
      /* Newline puts us on the next line. */
      else if (*str == '\n')
	line++;

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
#define TP_OBJ	  1
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
void	quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
   char	buf[MAX_STRING_LENGTH];

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
   extern struct obj_data *obj_proto;      /*. db.c        .*/
   char	name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
   int	field, type;
   struct char_data *mob;
   struct obj_data *obj;
   struct extra_descr_data *ed, *tmp, *ex_desc = NULL, *new_descr = NULL;

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
   } else /* type == TP_OBJ */	 {

      /* locate the object */
      if (!(obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
	 send_to_char("You aren't carrying anything by that name.\r\n", ch);
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
	    send_to_char("You have to supply a keyword.\r\n", ch);
	    return;
	 }
         /* since we are going to modify the ex_description of the object, let's
            allocate some space in memory for it independant of the space used
            by obj_proto.
         */

         if (obj->ex_description == obj_proto[GET_OBJ_RNUM(obj)].ex_description)
           for (ed = obj->ex_description; ed; ed = ed->next)
           {
             CREATE(new_descr, struct extra_descr_data, 1);
             new_descr->keyword = str_dup(ed->keyword);
             new_descr->description = str_dup(ed->description);
             new_descr->next = ex_desc;
             ex_desc = new_descr;
           }

	 /* try to locate extra description */
	 for (ed = ex_desc; ; ed = ed->next)
	    if (!ed) /* the field was not found. create a new one. */ {
	       CREATE(ed, struct extra_descr_data, 1);
	       ed->next = ex_desc;
	       ex_desc = ed;
	       CREATE(ed->keyword, char, strlen(string) + 1);
	       strcpy(ed->keyword, string);
	       ed->description = 0;
	       ch->desc->str = &ed->description;
	       send_to_char("New field.\r\n", ch);
	       break;
	    }
	    else if (!str_cmp(ed->keyword, string)) /* the field exists */ {
	       FREE(ed->description);
	       ed->description = 0;
	       ch->desc->str = &ed->description;
	       send_to_char("Modifying description.\r\n", ch);
	       break;
	    }
         obj->ex_description = ex_desc;   /* assign the new list */
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
	    if (!find_exdesc(string, ed)) 
            {
	       send_to_char("No field with that keyword.\r\n", ch);
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
   } else /* there was no string. enter string mode */	 {
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
