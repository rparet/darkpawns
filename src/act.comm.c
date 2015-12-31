/************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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

/* $Id: act.comm.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "clan.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern char *races[];

char *delete_ansi_controls(char *string);


/*
 * Returns a rakshasan translation for the input string "said"
 * Serapis 10-20-96
 */
static char *
speak_rakshasan( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable rak_syls[] = {
      { " ", " " },
      { "are", "nec"   },
      { "and", "arrl"    },
      { "be", "fess" },
      { "how", "ciss" },
      { "what", "rriit" },
      { "is", "garr" },
      { "ou", "owwl" },
      { "where", "kaal" },
      { "me", "phis" },
      { "dwarf",  "dwarf" },
      { "elf",  "elf" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "llirr" },
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "elven" },
      { "dwarven", "dwarven" },
      { "god", "kashka" },
      { "God", "Kashka" },
      { "who", "rukkaturl" },
      { "ck", "k", },
      { "cks", "th" },
      { "the ", "(growl) " },
      { "A", "A" }, { "B", "B" }, { "C", "Q" }, { "D", "E" }, { "E", "Ii" },
      { "F", "Y" }, { "G", "O" }, { "H", "P" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "Rr" }, { "M", "W" }, { "N", "Rr" }, { "O", "A" },
      { "P", "Ss" }, { "Q", "D" }, { "R", "F" }, { "S", "G" }, { "T", "H" },
      { "U", "Ii" }, { "V", "Z" }, { "W", "X" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" }, { "e", "ii" },
      { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "rr" }, { "m", "w" }, { "n", "rr" }, { "o", "a" },
      { "p", "ss" }, { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
      { "u", "ii" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(rak_syls[j].org); j++)
   if (strncmp(rak_syls[j].org,heard+offs,strlen(rak_syls[j].org)) == 0) {
      strcat(buf, rak_syls[j].new);
      if (strlen(rak_syls[j].org))
         offs += strlen(rak_syls[j].org);
      else
         ++offs;
   }
      if ( (!*rak_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

static char *
speak_elven( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable elf_syls[] = {
      { " ", " " },
      { "are", "est"   },
      { "and", "et"    },
      { "be", "deleste" },
      { "how", "quad" },
      { "what", "quod" },
      { "is", "est" },
      { "ou", "estra" },
      { "where", "este" },
      { "me", "ego" },
      { "dwarf",  "dwarf" },
      { "elf",  "elvinisti" },
      { "Elf",  "Elvinisti" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "beligant" },
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "elvenesti" },
      { "Elven", "Elvenesti" },
      { "dwarven", "dwarven" },
      { "god", "deus" },
      { "God", "Deorum" },
      { "who", "quelsteno" },
      { "ck", "llin", },
      { "cks", "llins" },
      { "the ", "a " },
      { "A", "A" }, { "B", "B" }, { "C", "Q" }, { "D", "E" }, { "E", "I" },
      { "F", "Y" }, { "G", "O" }, { "H", "P" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "R" }, { "M", "W" }, { "N", "V" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "L" }, { "S", "R" }, { "T", "H" },
      { "U", "I" }, { "V", "Z" }, { "W", "X" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" }, { "e", "i" },
      { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "l" }, { "o", "a" },
      { "p", "ss" }, { "q", "d" }, { "r", "l" }, { "s", "r" }, { "t", "h" },
      { "u", "i" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(elf_syls[j].org); j++)
   if (strncmp(elf_syls[j].org,heard+offs,strlen(elf_syls[j].org)) == 0) {
      strcat(buf, elf_syls[j].new);
      if (strlen(elf_syls[j].org))
         offs += strlen(elf_syls[j].org);
      else
         ++offs;
   }
      if ( (!*elf_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}


static char *
speak_kender( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable ken_syls[] = {
      { " ", " " },
      { "are", "ese"   },
      { "and", "ete"    },
      { "be", "este" },
      { "how", "angti" },
      { "what", "astem" },
      { "is", "en" },
      { "ou", "a" },
      { "where", "tu'ke" },
      { "me", "ki'ga" },
      { "dwarf",  "dwarf" },
      { "elf",  "elf" },
      { "Elf",  "Elvinisti" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "beligant" },
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "elvenesti" },
      { "Elven", "Elvenesti" },
      { "dwarven", "dwarven" },
      { "god", "deus" },
      { "God", "Deorum" },
      { "who", "quelsteno" },
      { "ck", "llin", },
      { "cks", "llins" },
      { "the ", "a " },
      { "A", "A" }, { "B", "B" }, { "C", "Q" }, { "D", "E" }, { "E", "I" },
      { "F", "Y" }, { "G", "O" }, { "H", "P" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "R" }, { "M", "W" }, { "N", "V" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "L" }, { "S", "R" }, { "T", "H" },
      { "U", "I" }, { "V", "Z" }, { "W", "X" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" }, { "e", "i" },
      { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "l" }, { "o", "a" },
      { "p", "ss" }, { "q", "d" }, { "r", "l" }, { "s", "r" }, { "t", "h" },
      { "u", "i" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(ken_syls[j].org); j++)
   if (strncmp(ken_syls[j].org,heard+offs,strlen(ken_syls[j].org)) == 0) {
      strcat(buf, ken_syls[j].new);
      if (strlen(ken_syls[j].org))
         offs += strlen(ken_syls[j].org);
      else
         ++offs;
   }
      if ( (!*ken_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

static char *
speak_dwarven( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable dwa_syls[] = {
      { " ", " " },
      { "are", "icht"   },
      { "and", "ent"    },
      { "be", "ki" },
      { "how", "var" },
      { "what", "war" },
      { "is", "ict" },
      { "ou", "agen" },
      { "where", "hung" },
      { "me", "mein" },
      { "dwarf",  "dwarf" },
      { "Dwarf",  "Dwarf" },
      { "elf",  "eli" },
      { "Elf",  "Eli" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "k'ne"},
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "eli" },
      { "Elven", "Eli" },
      { "dwarven", "dwarven" },
      { "god", "g'du" },
      { "God", "G'du" },
      { "who", "b'ir" },
      { "ck", "k", },
      { "cks", "ks" },
      { "the ", "t'el " },
      { "A", "A" }, { "B", "B" }, { "C", "'" }, { "D", "E" }, { "E", "I" },
      { "F", "Y" }, { "G", "O" }, { "H", "P" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "R" }, { "M", "W" }, { "N", "V" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "L" }, { "S", "R" }, { "T", "H" },
      { "U", "I" }, { "V", "Z" }, { "W", "'" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "'" }, { "d", "e" }, { "e", "i" },
      { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "l" }, { "o", "a" },
      { "p", "'s" }, { "q", "d" }, { "r", "l" }, { "s", "r" }, { "t", "h" },
      { "u", "i" }, { "v", "z" }, { "w", "'" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(dwa_syls[j].org); j++)
   if (strncmp(dwa_syls[j].org,heard+offs,strlen(dwa_syls[j].org)) == 0) {
      strcat(buf, dwa_syls[j].new);
      if (strlen(dwa_syls[j].org))
         offs += strlen(dwa_syls[j].org);
      else
         ++offs;
   }
      if ( (!*dwa_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

static char *
speak_minotaur( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable min_syls[] = {
      { " ", " " },
      { "are", "era"   },
      { "and", "ef"    },
      { "be", "f'let" },
      { "how", "hi'fen" },
      { "what", "f'akal" },
      { "is", "ge'tur" },
      { "ou", "affah" },
      { "where", "f'akan" },
      { "me", "kill'tur" },
      { "dwarf",  "dwarf" },
      { "elf",  "elvinisti" },
      { "Elf",  "Elvinisti" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "f'else" },
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "elvenesti" },
      { "Elven", "Elvenesti" },
      { "dwarven", "dwarven" },
      { "god", "fel'kur" },
      { "God", "Fel'kur" },
      { "who", "f'il" },
      { "ck", "'f", },
      { "cks", "'fs" },
      { "the ", "(growl) " },
      { "A", "A" }, { "B", "B" }, { "C", "F" }, { "D", "E" }, { "E", "I" },
      { "F", "Y" }, { "G", "O" }, { "H", "P" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "R" }, { "M", "W" }, { "N", "V" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "L" }, { "S", "R" }, { "T", "H" },
      { "U", "I" }, { "V", "Z" }, { "W", "F" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "f" }, { "d", "e" }, { "e", "i" },
      { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "l" }, { "o", "a" },
      { "p", "ff" }, { "q", "d" }, { "r", "l" }, { "s", "r" }, { "t", "h" },
      { "u", "i" }, { "v", "z" }, { "w", "f" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(min_syls[j].org); j++)
   if (strncmp(min_syls[j].org,heard+offs,strlen(min_syls[j].org)) == 0) {
      strcat(buf, min_syls[j].new);
      if (strlen(min_syls[j].org))
         offs += strlen(min_syls[j].org);
      else
         ++offs;
   }
      if ( (!*min_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

static char *
speak_ssaur( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable ssa_syls[] = {
      { " ", " " },
      { "are", "era"   },
      { "and", "ef"    },
      { "be", "f'ess" },
      { "how", "hi'fen" },
      { "what", "f'esal" },
      { "is", "ge'tur" },
      { "ou", "affah" },
      { "where", "f'akan" },
      { "me", "kiss'tur" },
      { "dwarf",  "dwarf" },
      { "elf",  "elvinisti" },
      { "Elf",  "Elvinisti" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "f'else" },
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "elvenesti" },
      { "Elven", "Elvenesti" },
      { "dwarven", "dwarven" },
      { "god", "fel'kur" },
      { "God", "Fel'kur" },
      { "who", "f'il" },
      { "ck", "'f", },
      { "cks", "'fs" },
      { "the ", "(growl) " },
      { "A", "A" }, { "B", "B" }, { "C", "S" }, { "D", "E" }, { "E", "I" },
      { "F", "Y" }, { "G", "O" }, { "H", "F" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "R" }, { "M", "W" }, { "N", "S" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "L" }, { "S", "R" }, { "T", "H" },
      { "U", "I" }, { "V", "Z" }, { "W", "F" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "s" }, { "d", "e" }, { "e", "i" },
      { "f", "y" }, { "g", "o" }, { "h", "f" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "s" }, { "o", "a" },
      { "p", "ff" }, { "q", "d" }, { "r", "l" }, { "s", "r" }, { "t", "h" },
      { "u", "i" }, { "v", "z" }, { "w", "f" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(ssa_syls[j].org); j++)
   if (strncmp(ssa_syls[j].org,heard+offs,strlen(ssa_syls[j].org)) == 0) {
      strcat(buf, ssa_syls[j].new);
      if (strlen(ssa_syls[j].org))
         offs += strlen(ssa_syls[j].org);
      else
         ++offs;
   }
      if ( (!*ssa_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

static char *
speak_human( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable hum_syls[] = {
      { " ", " " },
      { "are", "ar"   },
      { "and", "yet"    },
      { "be", "be" },
      { "how", "keen" },
      { "what", "forsuth" },
      { "is", "ist" },
      { "ou", "e" },
      { "where", "withal" },
      { "me", "mine" },
      { "dwarf",  "dwarf" },
      { "elf",  "elvinisti" },
      { "Elf",  "Elvinisti" },
      { "fucking",  "fucking" },
      { "serapis",  "Serapis" },
      { "Serapis",  "Serapis" },
      { "kill",  "todeth" },
      { "kender", "kenderkin" },
      { "centaur", "centaur" },
      { "rakshasa", "rakshasa" },
      { "Rakshasa", "Rakshasa" },
      { "human", "human" },
      { "elven", "elvenesti" },
      { "Elven", "Elvenesti" },
      { "dwarven", "dwarven" },
      { "god", "yihew" },
      { "God", "Yihew" },
      { "who", "wih" },
      { "ck", "keth", },
      { "cks", "keths" },
      { "the ", "doth " },
      { "A", "A" }, { "B", "B" }, { "C", "K" }, { "D", "L" }, { "E", "I" },
      { "F", "P" }, { "G", "G" }, { "H", "Th" }, { "I", "U" }, { "J", "G" },
      { "K", "K" }, { "L", "R" }, { "M", "W" }, { "N", "V" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "L" }, { "S", "R" }, { "T", "Th" },
      { "U", "I" }, { "V", "Z" }, { "W", "X" }, { "X", "N" }, { "Y", "Y" },
      { "Z", "K" },
      { "a", "a" }, { "b", "b" }, { "c", "k" }, { "d", "l" }, { "e", "i" },
      { "f", "p" }, { "g", "g" }, { "h", "th" }, { "i", "u" }, { "j", "g" },
      { "k", "k" }, { "l", "r" }, { "m", "w" }, { "n", "v" }, { "o", "a" },
      { "p", "s" }, { "q", "d" }, { "r", "l" }, { "s", "r" }, { "t", "th" },
      { "u", "i" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, { "y", "y" },
      { "z", "k" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(hum_syls[j].org); j++)
   if (strncmp(hum_syls[j].org,heard+offs,strlen(hum_syls[j].org)) == 0) {
      strcat(buf, hum_syls[j].new);
      if (strlen(hum_syls[j].org))
         offs += strlen(hum_syls[j].org);
      else
         ++offs;
   }
      if ( (!*hum_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

ACMD(do_race_say)
{
   int  i;
   struct char_data *tch = NULL;
   char *race_speak = NULL;

   for (i = 0; *(argument + i) == ' '; i++)
      ;

  /* check for stupid players */
  if (GET_WIS(ch) == 0 || GET_INT(ch) == 0)
  {
    stc("You are too stupid to communicate with language!\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
  {
    stc("You cannot race-say!\r\n", ch);
    return;
  }

   if (*(argument + i))
   {
   if (GET_RACE(ch) == RACE_RAKSHASA)
     race_speak = speak_rakshasan(argument+i);
   else if (GET_RACE(ch) == RACE_ELF)
     race_speak = speak_elven(argument+i);
   else if (GET_RACE(ch) == RACE_HUMAN)
     race_speak = speak_human(argument+i);
   else if (GET_RACE(ch) == RACE_DWARF)
     race_speak = speak_dwarven(argument+i);
   else if (GET_RACE(ch) == RACE_KENDER)
     race_speak = speak_kender(argument+i);
   else if (GET_RACE(ch) == RACE_MINOTAUR)
     race_speak = speak_minotaur(argument+i);
   else if (GET_RACE(ch) == RACE_SSAUR)
     race_speak = speak_ssaur(argument+i);
   else
  return;
   }

   if (!*(argument + i))
      send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
   else
   {
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
      {
  if (GET_RACE(ch) != GET_RACE(tch) && GET_LEVEL(tch) < LVL_IMMORT &&
      !IS_NPC(tch))
    {
      switch ( argument[strlen(argument) - 1] )
        {
        case '!':
    sprintf (buf,"%s exclaims, '%s'\r\n", GET_NAME(ch),race_speak);
    break;
        case '?':
    sprintf (buf, "%s asks, '%s'\r\n", GET_NAME(ch),race_speak);
    break;
        case '.':
    sprintf (buf, "%s states, '%s'\r\n", GET_NAME(ch),race_speak);
    break;
        default:
    sprintf(buf, "%s says, '%s'\r\n", GET_NAME(ch),race_speak);
        }
            if(AWAKE(tch))
        send_to_char(buf, tch);
    }
  else
    {
      switch ( argument[strlen(argument) - 1] )
        {
        case '!':
    sprintf (buf, "%s exclaims, '(In %s) %s'\r\n",
       GET_NAME(ch),races[GET_RACE(ch)], argument + i);
    break;
        case '?':
    sprintf (buf, "%s asks, '(In %s) %s'\r\n",
       GET_NAME(ch),races[GET_RACE(ch)], argument + i);
    break;
        case '.':
    sprintf (buf, "%s states, '(In %s) %s'\r\n",
       GET_NAME(ch),races[GET_RACE(ch)], argument + i);
    break;
        default:
    sprintf(buf, "%s says, '(In %s) %s'\r\n",
      GET_NAME(ch),races[GET_RACE(ch)], argument + i);
        }
      if ((tch != ch) && (AWAKE(tch)))
        send_to_char(buf, tch);
    }
      }
      if (!PRF_FLAGGED(ch, PRF_NOREPEAT))
      {
   switch ( argument[strlen(argument) - 1] )
     {
     case '!':
       sprintf (buf, "You exclaim, '(In %s) %s'\r\n",
          races[GET_RACE(ch)], argument + i);
       break;
     case '?':
       sprintf (buf, "You ask, '(In %s) %s'\r\n",
          races[GET_RACE(ch)], argument + i);
       break;
     case '.':
       sprintf (buf, "You state, '(In %s) %s'\r\n",
          races[GET_RACE(ch)], argument + i);
       break;
     default:
       sprintf(buf, "You say, '(In %s) %s'\r\n",
         races[GET_RACE(ch)], argument + i);

     }
   send_to_char(buf, ch);
      }
      else
  send_to_char("Ok.\n\r", ch);
  }
  if (race_speak)
    FREE(race_speak);
}

static char *speak_drunk( char *said );

ACMD(do_say)
{
  int  i;
  char *drunk_speak = NULL;

  for (i = 0; *(argument + i) == ' '; i++)
    ;
  /* check for stupid players */
  if (GET_WIS(ch) == 0 || GET_INT(ch) == 0)
  {
    stc("You are too stupid to communicate with language!\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
  {
    stc("You cannot speak!\r\n", ch);
    return;
  }

  if (!*(argument + i))
    send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
  else
    {
      if (GET_COND(ch, DRUNK)>10)
     drunk_speak = speak_drunk(argument+i);

      switch ( argument[strlen(argument) - 1] )
  {
  case '!': sprintf (buf, "$n exclaims, '%s'",
         drunk_speak ? drunk_speak :argument + i);
  break;
  case '?': sprintf (buf, "$n asks, '%s'",
         drunk_speak ? drunk_speak :argument + i);
  break;
  case '.': sprintf (buf, "$n states, '%s'",
         drunk_speak ? drunk_speak :argument + i);
  break;
  default: sprintf(buf, "$n says, '%s'",
       drunk_speak ? drunk_speak :argument + i);
  }
      delete_ansi_controls(buf);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      if (drunk_speak)
  FREE(drunk_speak);
      if (!PRF_FLAGGED(ch, PRF_NOREPEAT))
  {
    switch ( argument[strlen(argument) - 1] )
      {
      case '!': sprintf (buf, "You exclaim '%s'\n\r", argument + i);
        break;
      case '?': sprintf (buf, "You ask '%s'\n\r", argument + i);
        break;
      case '.': sprintf (buf, "You state '%s'\n\r", argument + i);
        break;
      default: sprintf(buf, "You say '%s'\n\r", argument + i);
      }
          delete_ansi_controls(buf);
    send_to_char(buf, ch);
  } else
    send_to_char("Ok.\n\r", ch);
    }
}


ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are not the member of a group!\r\n", ch);
    return;
  }
  if (!*argument)
    send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
  else {
    if (ch->master)
      k = ch->master;
    else
      k = ch;

    sprintf(buf, "$n tells the group, '%s'", argument);
    delete_ansi_controls(buf);

    if (IS_AFFECTED(k, AFF_GROUP) && (k != ch))
     {
      send_to_char(CCWHT(k, C_CMP), k);
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
      send_to_char(CCNRM(k, C_CMP), k);
     }
    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch))
  {
      send_to_char(CCWHT(f->follower, C_CMP), f->follower);
       act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);
         send_to_char(CCNRM(f->follower, C_CMP), f->follower);
  }

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You tell the group, '%s'", argument);
      delete_ansi_controls(buf);
      send_to_char(CCWHT(ch, C_SPR), ch);
      act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
      send_to_char(CCNRM(ch, C_NRM), ch);
    }
  }
}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  send_to_char(CCRED(vict, C_NRM), vict);
  sprintf(buf, "$n tells you, '%s'", arg);
  delete_ansi_controls(buf);
  act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(vict, C_NRM), vict);

  if (PRF_FLAGGED(vict, PRF_AFK))
    act( "$E is AFK right now, $E may not hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    send_to_char(CCRED(ch, C_CMP), ch);
    sprintf(buf, "You tell $N, '%s'", arg);
    delete_ansi_controls(buf);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    send_to_char(CCNRM(ch, C_CMP), ch);
  }

  GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (IS_NPC(vict) && (!vict->desc))
    send_to_char(NOPERSON, ch);
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (PRF_FLAGGED(ch, PRF_NOTELL) && (GET_LEVEL(ch) < LVL_IMMORT))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOSHOUT))
    stc("You cannot tell anyone anything!\r\n", ch);
  else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)  /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.",
  FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if ((PRF_FLAGGED(vict, PRF_NOTELL) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
           && (GET_LEVEL(ch) < LVL_IMMORT))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else
    perform_tell(ch, vict, buf2);
}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */

    while ( (tch != NULL) &&
            (GET_IDNUM(tch) != GET_LAST_TELL(ch)) )
      tch = tch->next;

    if ( tch == NULL ||(IS_NPC(tch) && (!tch->desc)) )
      send_to_char("They are no longer playing.\r\n", ch);
    else if (PLR_FLAGGED(tch, PLR_WRITING))
      send_to_char("They are writing now, try later.\r\n", ch);
    else if (PLR_FLAGGED(ch, PLR_NOSHOUT))
      stc("You cannot tell anyone anything!\r\n", ch);
    else if (PRF_FLAGGED(tch, PRF_NOTELL))
      stc("They can't hear you.\r\n", ch);
    else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
      send_to_char("The walls seem to absorb your words.\r\n", ch);
    else if (!IS_NPC(tch) && !tch->desc)  /* linkless */
      act("$E's linkless at the moment.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else if (PRF_FLAGGED(tch, PRF_NOTELL) || ROOM_FLAGGED(tch->in_room, ROOM_SOUNDPROOF))
      act("$E can't hear you.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
    else
      perform_tell(ch, tch, argument);
  }
}


ACMD(do_spec_comm)
{
  struct char_data *vict;
  char *action_sing, *action_plur, *action_others;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
  {
    stc("Sorry, you cannot do that.\r\n", ch);
    return;
  }

  if (subcmd == SCMD_WHISPER) {
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
  } else {
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
    send_to_char(buf, ch);
  } else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (vict == ch)
    send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else {
    sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
    delete_ansi_controls(buf);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000  /* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper = 0, *pen = 0;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
  {
    stc("You cannot write anything!\r\n", ch);
    return;
  }

  if (!*papername) {    /* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {    /* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", penname);
      send_to_char(buf, ch);
      return;
    }
  } else {    /* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {  /* oops, a pen.. */
      pen = paper;
      paper = 0;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
        papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen)
      paper = GET_EQ(ch, WEAR_HOLD);
    else
      pen = GET_EQ(ch, WEAR_HOLD);
  }


  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
    /* we can write - hooray! */
    send_to_char("Write your note.  End with '@' on a new line.\r\n", ch);
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *tch;

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_GOD) {
  for (d = descriptor_list; d; d = d->next)
    if (!d->connected && d->character)
      act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
  send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    if (((tch = get_char_vis(ch, arg)) != NULL) && !IS_NPC(tch) && tch->desc) {
      act(buf, FALSE, ch, 0, tch, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
  send_to_char(OK, ch);
      else
  act(buf, FALSE, ch, 0, tch, TO_CHAR);
      return;
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  void update_review(struct char_data *speaker, char *gossip);
  extern int level_can_shout;
  extern int holler_move_cost;
  struct descriptor_data *i;
  char color_on[24];

  /* Array of flags which must _not_ be set in order for comm to be heard */
  static int channels[] =
  {
    0,
    PRF_DEAF,
    PRF_NOGOSS,
    PRF_NOAUCT,
    PRF_NOGRATZ,
    PRF_NONEWBIE,
    0
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string.
   */
  static char *com_msgs[][4] = {
    {"You cannot holler!!\r\n",
     "holler",
     "",
     KYEL},

    {"You cannot shout!!\r\n",
     "shout",
     "Turn off your noshout flag first!\r\n",
     KYEL},

    {"You cannot gossip!!\r\n",
     "gossip",
     "You aren't even on the channel!\r\n",
     KYEL},

    {"You cannot auction!!\r\n",
     "auction",
     "You aren't even on the channel!\r\n",
     KMAG},

    {"You cannot congratulate!\r\n",
     "congrat",
     "You aren't even on the channel!\r\n",
     KGRN},

    {"You cannot newbie!\r\n",
     "newbie",
     "You aren't even on the channel!\r\n",
     KYEL}
  };

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
    {
      send_to_char(com_msgs[subcmd][0], ch);
      return;
    }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
    {
      send_to_char("The walls seem to absorb your words.\r\n", ch);
      return;
    }
  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < level_can_shout && subcmd != SCMD_NEWBIE)
    {
      sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
        level_can_shout, com_msgs[subcmd][1]);
      send_to_char(buf1, ch);
      return;
    }
  /* check for stupid players */
  if (GET_WIS(ch) == 0 || GET_INT(ch) == 0)
    {
      send_to_char ("You are too stupid to communicate with language!\r\n",
        ch);
      return;
    }
  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd]))
    {
      send_to_char(com_msgs[subcmd][2], ch);
      return;
    }
  /* skip leading spaces */
  skip_spaces(&argument);

  /* make sure that there is something there to say! */
  if (!*argument)
    {
      sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
        com_msgs[subcmd][1], com_msgs[subcmd][1]);
      send_to_char(buf1, ch);
      return;
    }
  if (subcmd == SCMD_HOLLER)
    {
      if (GET_MOVE(ch) < holler_move_cost)
  {
    send_to_char("You're too exhausted to holler.\r\n", ch);
    return;
  }
      else
  GET_MOVE(ch) -= holler_move_cost;
    }
  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  /* first, set up strings to be given to the communicator */
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else
    {
      if (COLOR_LEV(ch) >= C_CMP)
  sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1],
    argument, KNRM);
      else
  sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
      delete_ansi_controls(buf1);
      act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
    }

  sprintf(buf, "$n %ss, '%s'", com_msgs[subcmd][1], argument);
  delete_ansi_controls(buf);
  if (subcmd == SCMD_GOSSIP)
    update_review(ch, argument);

  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next)
    {
      if (!i->connected && i != ch->desc && i->character &&
    !PRF_FLAGGED(i->character, channels[subcmd]) &&
    !PLR_FLAGGED(i->character, PLR_WRITING) &&
    !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF))
  {
    if (subcmd == SCMD_SHOUT &&
        ((world[ch->in_room].zone != world[i->character->in_room].zone)
         || GET_POS(i->character) < POS_RESTING) )
      continue;

    if (COLOR_LEV(i->character) >= C_NRM)
      send_to_char(color_on, i->character);
    act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
    if (COLOR_LEV(i->character) >= C_NRM)
      send_to_char(KNRM, i->character);
  }
    }
}


ACMD(do_qcomm)
{
  struct descriptor_data *i;

  if (!PRF_FLAGGED(ch, PRF_QUEST))
    {
      send_to_char("You aren't even part of the quest!\r\n", ch);
      return;
    }

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
  {
    stc("You cannot quest-say!\r\n", ch);
    return;
  }

  skip_spaces(&argument);

  if (!*argument)
    {
      sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
        CMD_NAME);
      CAP(buf);
      send_to_char(buf, ch);
    }
  else
    {
      delete_ansi_controls(argument);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
  send_to_char(OK, ch);
      else
  {
    if (subcmd == SCMD_QSAY)
      sprintf(buf, "&WYou quest-say, '%s'&n", argument);
    else
      strcpy(buf, argument);
    act(buf, FALSE, ch, 0, argument, TO_CHAR);
  }

      if (subcmd == SCMD_QSAY)
  sprintf(buf, "&W$n quest-says, '%s'&n", argument);
      else
  strcpy(buf, argument);

      for (i = descriptor_list; i; i = i->next)
  if (!i->connected && i != ch->desc && !PLR_FLAGGED(i->character, PLR_WRITING) &&
      PRF_FLAGGED(i->character, PRF_QUEST))
          {
      act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
          }
    }
}



ACMD(do_think)
{
  int  i;
  for (i = 0; *(argument + i) == ' '; i++)
    ;
  if (!*(argument + i))
    {
      sprintf(buf, "$n thinks about life, the universe, and everything.");
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You think about life, the universe, and everything.\n\r",
                   ch);
    }
  else
    {
      if (PLR_FLAGGED(ch, PLR_NOSHOUT) || GET_INT(ch) == 0)
      {
       stc("You try to think aloud, but cannot!\r\n", ch);
       return;
      }
      sprintf(buf, "$n thinks . o O ( %s )", argument + i);
      delete_ansi_controls(buf);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      if (!PRF_FLAGGED(ch, PRF_NOREPEAT))
  {
    sprintf(buf, "You think . o O ( %s )\n\r", argument + i);
    send_to_char(buf, ch);
  }
      else
  send_to_char("Ok.\n\r", ch);
    }
}

static char *
speak_drunk( char *said )
{
   char *output = NULL;
   char heard[MAX_INPUT_LENGTH];
   int j, offs;

   struct syllable {
      char  org[10];
      char  new[10];
   };

   struct syllable drunk_syls[] = {
      { " ", " " },
      { "are", "arsh"   },
      { "and", "andsh"    },
      { "how", "howsh" },
      { "what", "wha'" },
      { "is", "ish" },
      { "where", "whersh" },
      { "kill",  "murderize" },
      { "ck", "shkin", },
      { "the ", "th' " },
      { "A", "A" }, { "B", "B" }, { "C", "C" }, { "D", "D" }, { "E", "E" },
      { "F", "F" }, { "G", "G" }, { "H", "H" }, { "I", "I" }, { "J", "J" },
      { "K", "K" }, { "L", "L" }, { "M", "M" }, { "N", "N" }, { "O", "O" },
      { "P", "P" }, { "Q", "Q" }, { "R", "R" }, { "S", "SH" }, { "T", "Th" },
      { "U", "u" }, { "V", "V" }, { "W", "W" }, { "X", "X" }, { "Y", "Y" },
      { "Z", "Z" },
      { "a", "a" }, { "b", "b" }, { "c", "c" }, { "d", "d" }, { "e", "e" },
      { "f", "f" }, { "g", "g" }, { "h", "h" }, { "i", "i" }, { "j", "j" },
      { "k", "k" }, { "l", "l" }, { "m", "m" }, { "n", "n" }, { "o", "o" },
      { "p", "p" }, { "q", "q" }, { "r", "r" }, { "s", "sh" }, { "t", "th" },
      { "u", "u" }, { "v", "v" }, { "w", "w" }, { "x", "x" }, { "y", "y" },
      { "z", "z" }, { "", "" }
   };

   strcpy(buf, "");
   strcpy(heard, said);

   offs = 0;

   while (*(heard + offs)) {
      for (j = 0; *(drunk_syls[j].org); j++)
   if (strncmp(drunk_syls[j].org,
         heard+offs,strlen(drunk_syls[j].org)) == 0) {
      strcat(buf, drunk_syls[j].new);
      if (strlen(drunk_syls[j].org))
         offs += strlen(drunk_syls[j].org);
      else
         ++offs;
   }
      if ( (!*drunk_syls[j].org) && *(heard+offs) )
  {
  strncat (buf, heard+offs, 1);
  offs++;
  }
   }

   output = str_dup(buf);
   return(output);
}

ACMD (do_ctell)
{
  struct descriptor_data *i;
  int minlev=1, c=0;
  char level_string[6]="\0\0\0\0\0\0";

  skip_spaces (&argument);

  /*
   * The syntax of ctell for imms is different then for morts
   * mort: ctell <bla bla bla>    imms: ctell <clan_num> <bla bla bla>
   * Imms cannot actually see ctells but they can send them
   */
  if (GET_LEVEL(ch) >= LVL_IMMORT)
    {
      c = atoi (argument);
      if ((c <= 0) || (c > num_of_clans))
  {
    send_to_char ("There is no clan with that number.\r\n", ch);
    return;
  }
      while ((*argument != ' ') && (*argument != '\0'))
  argument++;
      while (*argument == ' ') argument++;
    }
  else if((c=GET_CLAN(ch))==0 || GET_CLAN_RANK(ch)==0)
    {
      send_to_char ("You're not part of a clan.\r\n", ch);
      return;
    }

  if (PRF_FLAGGED(ch, PRF_NOCTELL))
    {
      stc("You aren't currently on your clan channel.\r\n", ch);
      return;
    }

  if (PLR_FLAGGED(ch, PLR_NOSHOUT))
  {
    stc("You cannot clan-tell anything!\r\n", ch);
    return;
  }

  skip_spaces (&argument);

  if (!*argument){
    send_to_char ("What do you want to tell your clan?\r\n", ch);
    return;
  }

  if (*argument == '#')
    {
      one_argument(argument + 1, buf1);

      if (!is_number(buf1))
      {
        stc("Try entering in a number.\r\n", ch);
        return;
      }

      half_chop(argument+1, buf1, argument);
      minlev = atoi (buf1);
      if (minlev > clan[c].ranks)
  {
    send_to_char ("No one has a clan rank high enough to hear you!\r\n", ch);
    return;
  }

      skip_spaces(&argument);
      if (!*argument)
      {
        stc("What do you want to tell them?\r\n", ch);
        return;
      }
      sprintf (level_string, " (%d) ", minlev);
    }

  if (PRF_FLAGGED(ch,PRF_NOREPEAT)) {
    sprintf (buf1, "%s", OK);
    send_to_char (buf1, ch);
  }
  else {
    sprintf (buf1, "You tell your clan%s, '%s'\r\n",level_string, argument);
    delete_ansi_controls(buf1);
    stc(CCCYN(ch, C_CMP), ch);
    stc(buf1, ch);
    stc(CCNRM(ch, C_CMP), ch);
  }

  for (i = descriptor_list; i; i=i->next)
    {
     if (!i->connected) {
      if (i->character->player_specials->saved.clan == c)
  {
    if (i->character->player_specials->saved.clan_rank >= minlev)
      {
        if (strcmp (i->character->player.name, ch->player.name))
    {
                if (!PRF_FLAGGED(i->character, PRF_NOCTELL))
                  {
      sprintf (buf, "%s tells your clan%s, '%s'\r\n",
         (!CAN_SEE(i->character, ch) ?
          "Someone" : ch->player.name),
         level_string, argument);
                  delete_ansi_controls(buf);
      stc(CCCYN(i->character, C_CMP), i->character);
                  stc(buf, i->character);
                  stc(CCNRM(i->character, C_CMP),i->character);
                 }
    }
      }
  }
    }
  }
  return;
}
