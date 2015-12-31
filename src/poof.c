/* ==========================================================================
   FILE   : poof.c
   HISTORY: dlkarnes 960911 (Serapis)
            based on code done by Jeremy Hess and Chad Thompson
   ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"

void
write_poofs(struct char_data *ch)
{
  FILE *file;
  char fn[127];
  char *poofin = POOFIN(ch);
  char *poofout = POOFOUT(ch);
  char default_poofin[50];
  char default_poofout[50];
  int length;

  /* truncate those bad boys so we dont crash */
  if (poofin)
   if (strlen(poofin)>120)
    poofin[120] = '\0';
  if (poofout)
   if (strlen(poofout)>120)
    poofout[120] = '\0';
  /*
   * set the default poofs
   */
  strcpy(default_poofin, "rides in on your mom.");
  strcpy(default_poofout, "rides out on your mom.");

  get_filename(GET_NAME(ch), fn, POOF_FILE);
  unlink(fn);
  if( !poofin && !poofout )
    return;

  file = fopen(fn,"wt");

  if( poofin )
  {
    length = strlen(poofin);
    fprintf(file,"%d\n",length);
    fprintf(file,"%s\n",poofin);
  }
  else
    {
      length = strlen(default_poofin);
      fprintf(file,"%d\n",length);
      fprintf(file,"%s\n",default_poofin);
    }

  if( poofout )
  {
    length = strlen(poofout);
    fprintf(file,"%d\n",length);
    fprintf(file,"%s\n",poofout);
  }
  else
    {
      length = strlen(default_poofout);
      fprintf(file,"%d\n",length);
      fprintf(file,"%s\n",default_poofout);
    }

  fclose(file);
}

void
read_poofs(struct char_data *ch)
{
  FILE *file;
  char fn[127];
  int length;
  char buf[127];

  get_filename(GET_NAME(ch),fn,POOF_FILE);

  file = fopen (fn,"r");

  if( !file )
    return;

  fscanf (file,"%d\n",&length);
  fgets (buf,length+1,file);
  POOFIN(ch) = str_dup(buf);
  fscanf (file,"%d\n",&length);
  fgets (buf,length+1,file);
  POOFOUT(ch) = str_dup(buf);

  fclose(file);
}

