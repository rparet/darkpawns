
#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"

/*
   This procedure has it's own file, because it must be compiled every
   time when linking to work correctly.
*/

time_t compile_time;
char *compile_time_str;

void load_compile_time( void )
{
  time_t ct;
  struct tm * now;
  char * mon;
  char buf[100];

  ct = time(0);
  compile_time = ct;

  now = localtime(&ct);

  switch(now->tm_mon) {
  case 0:
    mon = str_dup("January"); break;
  case 1:
    mon = str_dup("February"); break;
  case 2:
    mon = str_dup("March"); break;
  case 3:
    mon = str_dup("April"); break;
  case 4:
    mon = str_dup("May"); break;
  case 5:
    mon = str_dup("June"); break;
  case 6:
    mon = str_dup("July"); break;
  case 7:
    mon = str_dup("August"); break;
  case 8:
    mon = str_dup("September"); break;
  case 9:
    mon = str_dup("October"); break;
  case 10:
    mon = str_dup("November"); break;
  case 11:
    mon = str_dup("December"); break;
  default:
    mon = str_dup("Unknown"); break;
  }

  sprintf(buf, "%s %d, %d   ", mon, now->tm_mday, now->tm_year+1900);
  if (now->tm_hour >= 10)
     sprintf(buf, "%s%d:", buf, now->tm_hour);
  else
     sprintf(buf, "%s0%d:", buf, now->tm_hour);
  if (now->tm_min >= 10)
     sprintf(buf, "%s%d", buf, now->tm_min);
  else
     sprintf(buf, "%s0%d", buf, now->tm_min);
  compile_time_str = str_dup(buf);
  if (mon != NULL)
     FREE(mon);
}
