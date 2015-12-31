/* ************************************************************************
*   File: alias.c                  A utility to CircleMUD *
*  Usage: writing/reading player's aliases                                *
*                                                                         *
*  Code done by Jeremy Hess and Chad Thompson                 *
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

/* $Id: alias.c 1487 2008-05-22 01:36:10Z jravn $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"

void write_aliases(struct char_data *ch)
{
  FILE *file;
  char fn[127],*buf;
  struct alias *temp;
  int length;

  get_filename(GET_NAME(ch),fn,ALIAS_FILE);
  unlink(fn);
  if( !GET_ALIASES(ch) )
    return;

  file = fopen(fn,"wt");

  temp = GET_ALIASES(ch);

  while( temp )
  {
    length = strlen(temp->alias);
    fprintf(file,"%d\n",length);
    fprintf(file,"%s\n",temp->alias);
    buf = str_dup(temp->replacement);
    while( *++buf == ' ' );
    length = strlen(buf);
    fprintf(file,"%d\n",length);
    fprintf(file,"%s\n",buf);
    fprintf(file,"%d\n",temp->type);
    temp = temp->next;
  }

  fclose(file);
}

void read_aliases(struct char_data *ch)
{
  FILE *file;
  char fn[127];
  struct alias *t2;
  int length;
  char temp_buf[127],buf[127];

  get_filename(GET_NAME(ch),fn,ALIAS_FILE);

  file = fopen(fn,"r");

  if( !file )
    return;

  CREATE(GET_ALIASES(ch),struct alias,1);
  t2 = GET_ALIASES(ch);
  do
  {
    fscanf(file,"%d\n",&length);
    fgets(buf,length+1,file);
    t2->alias=str_dup(buf);
    fscanf(file,"%d\n",&length);
    fgets(buf,length+1,file);
    strcpy(temp_buf," ");
    strcat(temp_buf,buf);
    t2->replacement=str_dup(temp_buf);
    fscanf(file,"%d\n",&length);
    t2->type = length;
    if( !feof(file) ){
      CREATE(t2->next,struct alias,1);
      t2 = t2->next;
    }
  } while( !feof(file) );

  fclose(file);
}

