/* ************************************************************************
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
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

/* $Id: utils.c 1273 2005-12-20 05:19:42Z mcooper $ */

#include <stdarg.h>

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "constants.h"

/* external functions */
extern struct time_data time_info;
extern int intelligent_races[];
SPECIAL(cityguard);
SPECIAL(shop_keeper);
SPECIAL(clerk);
SPECIAL(prostitute);

/* local functions */
struct char_data *HUNTING(struct char_data *ch);
int is_shopkeeper(struct char_data * chChar);
int is_citymob(struct char_data * chChar);
int is_teacher(struct char_data * chChar);

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((random() % (to - from + 1)) + from);
}


/* simulates dice roll */
int dice(int num, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  while (num-- > 0)
    sum += number(1, size); 

  return sum;
}


int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}



/* Create a duplicate of a string */
char *
str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1); 
  return (strcpy(new, source));
}



/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
    {
      if (chk < 0)
	return (-1);
      else
	return (1);
    }
  return (0);
}


/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk, i;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
    {
      if (chk < 0)
	return (-1);
      else
	return (1);
    }
  return (0);
}


/* log a death trap hit */
void log_death_trap(struct char_data * ch)
{
  char buf[150];
  extern struct room_data *world;

  sprintf(buf, "%s hit death trap #%ld (%s)", GET_NAME(ch),
	  world[ch->in_room].number, world[ch->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
}

/*
 * New variable argument log() function.  Works the same as the old for
 * previously written code but is very nice for new code.
 */
void 
basic_mud_log(const char *format, ...)
{
  va_list args;
  time_t ct = time(0);
  char *time_s = asctime(localtime(&ct));

  if (logfile == NULL)
    puts("-E- Using log() before stream was initialized!");
  if (format == NULL)
    format = "-E- log() received a NULL format.";

  fprintf(logfile, "%-19.19s :: ", time_s);

  va_start(args, format);
  vfprintf(logfile, format, args);
  va_end(args);

  fprintf(logfile, "\n");
  fflush(logfile);
}

/* variable argument logging function */
void alog (char const *str, ...)
{
   time_t ct;
   char *tmstr;
   va_list args;

   ct = time(0);
   tmstr = asctime(localtime(&ct));
   *(tmstr + strlen(tmstr) - 1) = '\0';

   va_start(args, str);
   fprintf(stderr, "%-19.19s :: ", tmstr);
   vfprintf(stderr, str, args);
   fprintf(stderr, "\n");
   va_end(args);
}
/* the "touch" command, essentially. */
int touch(char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    perror(path);
    return -1;
  } else {
    fclose(fl);
    return 0;
  }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void 
mudlog(char *str, char type, int level, byte file)
{
  char buf[MAX_STRING_LENGTH];
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i;
  char *tmp, tp;
  time_t ct;

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (file)
    fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING)) {
      tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
	    (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

      if ((GET_LEVEL(i->character) >= level) && (tp >= type)) {
	send_to_char(CCGRN(i->character, C_NRM), i->character);
	send_to_char(buf, i->character);
	send_to_char(CCNRM(i->character, C_NRM), i->character);
      }
    }
}



void 
sprintbit(bitvector_t bitvector, char *names[], char *result)
{
  long nr;

  *result = '\0';

  if (bitvector < 0) {
    strcpy(result, "<INVALID BITVECTOR>");
    return;
  }
  for (nr = 0; bitvector; bitvector >>= 1) {
    if (IS_SET(bitvector, 1)) {
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else
	strcat(result, "UNDEFINED ");
    }
    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcpy(result, "NOBITS ");
}



void sprinttype(int type, char *names[], char *result)
{
  int nr = 0;

  while (type && *names[nr] != '\n') {
    type--;
    nr++;
  }

  if (*names[nr] != '\n')
    strcpy(result, names[nr]);
  else
    strcpy(result, "UNDEFINED");
}


/* Replaces all instances of " in text with \" for SQL storage purposes */
char *replace_quotes(char *text)
{
  int i, count;
  char *ptr;
  static char new_text[MAX_STRING_LENGTH+1];

  if (strchr(text, '"')) {
    ptr = text;
    for (i = 0, count = 0; *ptr; *ptr++, i++)
      if (*ptr == '"') {
        new_text[i + count] = '\\';
        new_text[i + count + 1] = text[i];
        count++;
      } else
        new_text[i + count] = text[i];
    new_text[i + count] = '\0';
  } else
    strcpy(new_text,text);

  return (new_text);
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17;	/* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

  return now;
}



struct time_info_data age(struct char_data * ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;	/* All players start at 17 */

  return player_age;
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}


void
unmount(struct char_data *rider, struct char_data *mount)
{
  if (rider && IS_MOUNTED(rider))
    REMOVE_BIT(AFF_FLAGS(rider), AFF_MOUNT);
  if (mount && IS_MOUNTED(mount))
    REMOVE_BIT(AFF_FLAGS(mount), AFF_MOUNT);
}

struct char_data *
get_rider(struct char_data *mount)
{
  if (mount && IS_NPC(mount) && IS_MOUNTED(mount))
    return (mount->master);
  return ((struct char_data *)NULL);
}


/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data * ch)
{
  struct follow_type *j, *k;
  int shadowing = FALSE;

  assert(ch->master);

  if (IS_SHADOWING(ch))
        {
          affect_from_char(ch, SKILL_SHADOW);
          REMOVE_BIT(AFF_FLAGS(ch), AFF_DODGE);
          shadowing = TRUE;
        }    

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    GET_MOB_TIMER(ch) = 6;			/* 6 hours before timer expires */
    if (affected_by_spell(ch, SPELL_CHARM))
      affect_from_char(ch, SPELL_CHARM);
  } else if (shadowing) {
    act("You stop shadowing $N.", FALSE, ch, 0, ch->master, TO_CHAR);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    if (CAN_SEE(ch->master, ch) && AWAKE(ch->master))
      act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }
  if (IS_NPC(ch) && IS_MOUNTED(ch))
    unmount(get_rider(ch), ch);

  if (ch->master->followers->follower == ch) {	/* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    FREE(k);
  } else {			/* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    FREE(j);
  }

  ch->master = NULL;
  REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM);
  REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}


/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leaderbut no message will be sent */
void
add_follower_quiet(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;
}

/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data * ch, struct char_data * leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch) && AWAKE(leader))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (temp && *temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl))
    return 0;
  else {
    strcpy(buf, temp);
    return lines;
  }
}


int get_filename(char *orig_name, char *filename, int mode)
{
  char *prefix, *middle, *suffix, *ptr, name[64];

  switch (mode) 
  {
  case ETEXT_FILE:
    prefix = "plrtext";
    suffix = "text";
    break;
  default:
    return 0;
  }

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
  return 1;
}


int num_pc_in_room(struct room_data *room)
{
  int i = 0;
  struct char_data *ch;

  for (ch = room->people; ch != NULL; ch = ch->next_in_room)
    if (!IS_NPC(ch))
      i++;

  return i;
}

/* serapis 110896 */
int
parse_race(char arg)
{
	switch(arg)
	{
		case 'k': return RACE_KIR; 
		case 'a': return RACE_ANTALI; 
		case 'x': return RACE_XIXIAN; 
		default: break;
	}
	return RACE_UNDEFINED; 
}

struct char_data *
get_mount(struct char_data *ch)
{
  struct follow_type *j, *f;

  if (!ch || !IS_MOUNTED(ch))
    return ((struct char_data *)NULL);

  for (f = ch->followers; f; f = j) {
    j = f->next;
    if (IS_MOB(f->follower) && IS_MOUNTED(f->follower))
      return(f->follower);
  }

  return ((struct char_data *)NULL);
}


struct char_data *
get_rider_in_room(struct char_data *mount)
{
  if (mount && IS_NPC(mount) && IS_MOUNTED(mount) && 
	mount->master->in_room == mount->in_room)
    return (mount->master);
  return ((struct char_data *)NULL);
}



int
riding_mount(struct char_data *rider, struct char_data *mount)
{
  if (rider == get_rider_in_room(mount) && mount == get_mount(rider))
    return(TRUE);
  else
    return(FALSE);
}

bool
are_grouped(struct char_data *ch1, struct char_data *ch2)
{
   struct char_data *k;
   struct follow_type *f;

   if (!IS_AFFECTED(ch1, AFF_GROUP) || !IS_AFFECTED(ch2, AFF_GROUP))
      return(FALSE);
   if (ch1->master)
      k = ch1->master;
   else
      k = ch1;
   if (k == ch2)
      return(TRUE);
   for ( f = k->followers; f; f = f->next)
      if (f->follower == ch2)
         return(TRUE);
   return FALSE;
} 

bool
is_intelligent(struct char_data *ch)
{
  int i;
  for (i=0; i<NUM_INTEL_RACES; i++) 
	if (GET_RACE(ch)==intelligent_races[i])
		return(TRUE);
  return(FALSE);
}

bool
can_speak(struct char_data *ch)
{
  return(is_intelligent(ch) );
}

struct char_data *
get_char_by_id(long id)
{
  struct descriptor_data *d = NULL;
  struct char_data *vict = NULL;

  for (d = descriptor_list; d; d = d->next)
  {
   if (!d->connected)
     vict = (d->original ? d->original : d->character);
   if (vict)
        if (GET_IDNUM(vict) == id)
          return(vict);
  }
  return(NULL);
}

void
set_hunting(struct char_data *ch, struct char_data *vict) 
{ 
  if (!ch || (ch==vict) )
    return;

  if (vict == NULL) /* vict has been set to NULL for whatever reason */
  {
    /* setting this to -1 is a signal to return to origin */
    (ch)->char_specials.hunting_id = -1;
    (ch)->char_specials.hunting = NULL; /* clean up */
    (ch)->char_specials.hunted_obj = NULL;
    return;
  }    

  (ch)->char_specials.hunting_id = 0;
  (ch)->char_specials.hunting = NULL;
  (ch)->char_specials.hunted_obj = NULL;

  if (vict)
  {
    if (IS_MOB(vict))
      (ch)->char_specials.hunting = vict;
    else if (GET_IDNUM(vict)>0)
    {
      char buf[150];
      sprintf(buf, "%s started hunting %s", GET_NAME(ch), GET_NAME(vict));
      mudlog(buf, CMP, LVL_IMMORT, FALSE);
      (ch)->char_specials.hunting_id = GET_IDNUM(vict);
    }
  }
}

struct char_data *
HUNTING(struct char_data *ch)
{
  if (!ch || !IS_NPC(ch))
    return NULL;
  if ((ch)->char_specials.hunting)
    return((ch)->char_specials.hunting);
  else if ((ch)->char_specials.hunting_id)
    return(get_char_by_id((ch)->char_specials.hunting_id));
  else
   return NULL;
}


int
num_followers(struct char_data *ch)
{
  int tot_members = 0;
  struct follow_type *f;
  
  if (ch->followers)
    for (f = ch->followers; f; f = f->next)
      tot_members++;

   return(tot_members);
}

static int parse_command (char *ptr, char **f)
{
   double nf;

   nf = strtod (ptr, f);

   if (*f == ptr)
      nf = 0;

   return ((int)nf);
}


static int 
total_bytes (char *fmt, va_list argptr)
{
   int len;
   int size, psize;
   char *ptr;
   char *f;
   char format[4];
   char buffer[128];
   int i;
   unsigned u;
   double d;

   len = strlen(fmt);
   ptr = strchr (fmt, '%');

   /* skip past any escaped % characters */
   while (ptr && *ptr && (*(ptr + 1) == '%'))
      ptr = strchr (ptr + 2, '%');

   while (ptr)
   {
      size = 0;
      psize = parse_command (++ptr, &f);
      sprintf (format, "%%%c", *f);

      switch (*f)
      {
      case 'd':
      case 'i':
         i = va_arg (argptr, int);
         sprintf (buffer, format, i);
         size = strlen (buffer);
         break;
      case '%':
         ptr++;
      case 'c':
         (void)va_arg (argptr, int);    /* char types are promoted to int */
         size = sizeof(char);
         break;
      case 'e':
      case 'E':
      case 'g':
      case 'f':
      case 'G':
         d = va_arg (argptr, double);
         sprintf (buffer, format, d);
         size = strlen (buffer);
         break;
      case 'u':
      case 'o':
      case 'x':
      case 'X':
         u = va_arg (argptr, unsigned int);
         sprintf (buffer, format, u);
         size = strlen (buffer);
         break;
      case 'p':
         i = (int) va_arg (argptr, void *);
         sprintf (buffer, format, i);
         size = strlen (buffer);
         break;
      case 's':
         f = va_arg (argptr, char *);
         size = f ? strlen (f) : 0;
         break;
      default:
         fprintf (stderr, "total_bytes: invalid format modifier <%c>\n", *f);
         len = -1;
         break;
      }

      if (len == -1)
         break;

      if (psize)
         len += psize;
      else
         len += size;

      ptr = strchr (ptr, '%');

      /* skip past any escaped % characters */
      while (ptr && *ptr && (*(ptr + 1) == '%'))
         ptr = strchr (ptr + 2, '%');
   }

   return (len);
}

/*
 * char_string = tprintf("%s blah %d\r\n", some_string, some_int;
 * char_string will be allocated and must be FREEd
 */
char *tprintf (char *fmt, ...)
{
   int bytes;
   va_list params;
   char *buffer = NULL;

   va_start (params, fmt);

   bytes = total_bytes (fmt, params);
   if (bytes > 0)
   {
      buffer = (char *) calloc (bytes + 1, sizeof(char));
      buffer[0] = '\0';
   }

   if (buffer)
      vsprintf(buffer, fmt, params);

   va_end (params);

   return buffer;
}

struct char_data *
is_playing(char *vict_name)
{
  extern struct descriptor_data
    *descriptor_list; struct descriptor_data *i, *next_i;

  for (i = descriptor_list; i; i = next_i)
    {
      next_i = i->next;
      if(i->connected == CON_PLAYING &&
	 !strcmp(i->character->player.name,CAP(vict_name)))
	return i->character;
    }
  return NULL;
}

/* With change to 64 bits, depricated 20010602 - mcooper */
/*
void 
sprintbitarray(int bitvector[], char *names[], int maxar, char *result) 
{
   int nr, teller, found = FALSE;
   *result = '\0';

   for(teller = 0; teller < maxar && !found; teller++)
      for(nr = 0; nr < 32 && !found; nr++) 
      {
         if(IS_SET_AR(bitvector, (teller*32)+nr))
         {
            if(*names[(teller*32)+nr] != '\n') 
            {
               if(*names[(teller*32)+nr] != '\0') 
               {
                  strcat(result, names[(teller *32)+nr]);
                  strcat(result, " ");
               }
            } else 
            {
               strcat(result, "UNDEFINED ");
            }
         }
         if(*names[(teller*32)+nr] == '\n')
            found = TRUE;
      }

   if(!*result)
      strcpy(result, "NOBITS ");
}
*/

int 
sprintascii(char *out, bitvector_t bits)
{
  int i, j = 0;
  /* 42 bits, don't just add letters to try to get more unless your bitvector_t is
     also as large. */
  char *flags="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQ";

  for (i = 0; flags[i] != '\0'; i++)
    if (bits & (1LL << i))
      out[j++] = flags[i];

  if (j == 0) /* Didn't write anything. */
    out[j++] = '0';

  /* NUL terminate the output string. */
  out[j++] = '\0';
  return j;
}


/* check to see if there is a circle of summoning in the room.
   Also, decrement the counter due to movement in the room or use.
   -rparet 5-07-98 */

int
circle_check(struct char_data *ch)
{
  struct obj_data *i = NULL;

  if (!ch || ch->in_room == NOWHERE)
    return FALSE;
  
  for (i = world[ch->in_room].contents; i; i = i->next_content) 
  {
     if(i && GET_OBJ_VNUM(i) == COC_VNUM) {
       GET_OBJ_TIMER(i)--;
       return TRUE;
     }
  }

  return FALSE;

}

bool ok_to_damage(struct char_data *ch, struct char_data *victim, int is_magic)
{
  if (ROOM_FLAGGED(victim->in_room, ROOM_PEACEFUL))
    return FALSE;

  if (is_magic && ROOM_FLAGGED(victim->in_room, ROOM_NOMAGIC))
    return FALSE;

  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return FALSE;

  if (is_shopkeeper(victim))
    return FALSE;

  if (!IS_NPC(ch) && !IS_NPC(victim) && GET_LEVEL(victim) <= 10)
    return FALSE;

  return TRUE;
}

/*
 * This function (derived from basic fork(); abort(); idea by Erwin S.
 * Andreasen) causes your MUD to dump core (assuming you can) but
 * continue running.  The core dump will allow post-mortem debugging
 * that is less severe than assert();  Don't call this directly as
 * core_dump_unix() but as simply 'core_dump()' so that it will be
 * excluded from systems not supporting them. (e.g. Windows '95).
 *
 * You still want to call abort() or exit(1) for
 * non-recoverable errors, of course...
 *
 * XXX: Wonder if flushing streams includes sockets?
 */
void core_dump_real(const char *who, int line)
{
  log("-E- Assertion failed at %s:%d!", who, line);

#if defined(CIRCLE_UNIX)
  /* These would be duplicated otherwise... */
  fflush(stdout);
  fflush(stderr);
  fflush(logfile);
  mysql_close(darkpawns);

  /* Everything, just in case, for the systems that support it. */  
  fflush(NULL);

  /*
   * Kill the child so the debugger or script doesn't think the MUD
   * crashed.  The 'autorun' script would otherwise run it again.
   */
  if (fork() == 0)
    abort();
#endif
}

/* returns true if target is in ch's squelch-list */

int 
check_squelch(struct char_data *ch, struct char_data *target)
{
  int counter;
 
  if (!ch || !target)
    return FALSE;

  if (IS_NPC(ch) || IS_NPC(target))
    return FALSE;

  for (counter = 0; counter < MAX_SQUELCH; counter++)
     if (GET_IDNUM(target) == GET_SQUELCH(ch, counter))
       return TRUE;

  return FALSE;
}

int is_shopkeeper(struct char_data * chChar)
{
  int ch_num;

  if (!IS_NPC(chChar))
    return (FALSE);

  if (GET_MOB_SPEC(chChar) == shop_keeper)
    return TRUE;

  if (GET_MOB_SPEC(chChar) == clerk)
    return TRUE;
  
  if (is_teacher(chChar))
    return TRUE;

  ch_num = GET_MOB_VNUM(chChar);
  switch (ch_num) { /* Mobs we don't want to race attack or be able to kill! */
    case 3:
    case 5330:
    case 8087:
    case 8088:
    case 18236:
    case 18237:
    case 21216:
      return (TRUE);
  }

  return (FALSE);
}

int
is_citymob(struct char_data * chChar)
{
  int ch_num;

  if (!IS_NPC(chChar))
    return (TRUE);

  ch_num = GET_MOB_VNUM(chChar);
  if ((ch_num >= 7900) && (ch_num < 7999))
    return (FALSE);

  return (TRUE);
}

int is_teacher(struct char_data * chChar)
{
  int ch_num;

  if (!IS_NPC(chChar))
    return (FALSE);

  ch_num = GET_MOB_VNUM(chChar);
  switch (ch_num) {      /* insert teachers here */
    case 4825:
    case 5307:
    case 7971:
    case 7972:
    case 7973:
    case 8024:
    case 18219:
    case 21214:
    case 21220:
      return (TRUE);
  }
  return (FALSE);
}

int
is_magical_mob(struct char_data *chChar)
{
  int race;

  if (!IS_NPC(chChar))
    return (FALSE);

  race = GET_RACE(chChar);
  switch (race) {
    case RACE_ELEMENTAL:
    case RACE_CONSTRUCT:
    case RACE_VAMPIRE:
    case RACE_GHOST:
      return (TRUE);
      break;
  }

  return (FALSE);
}

void race_hate_sort(struct char_data *ch)
{
  int i, j, high, race, count;
  struct race_hate {
    int race;
    int kills;
  } *list;

  CREATE(list, struct race_hate, NUM_RACES);

  /* Clear our current racial hatreds */
  for (i = 0; i < 5; i++)
    GET_RACE_HATE(ch, i) = -1;

  /* Compile a list of the races we currently have a hatred for */
  for (i = 0, j = 0; i < NUM_RACES; i++)
    if (GET_RACE_KILLS(ch, i) >= NUM_RACE_KILLS) {
      list[j].race = i;
      list[j++].kills = GET_RACE_KILLS(ch, i);
    }

  /* Search for the highest 5 racial hatreds */
  for (i = 0, high = 0, count = 0, race = 0; i < 5; i++) {
    j = -1;
    while (list[++j].kills)
      if (list[j].kills > high) {
        high = list[j].kills;
        race = list[j].race;
        count = j;
      }

    /* Now re-assign based on any changes */
    if (high) {
      GET_RACE_HATE(ch, i) = race;
      list[count].kills = -1;
      high = 0;
    }
  }

  FREE(list);
}

unsigned short uuid_hash(uuid_t *uuid)
{
  short               c0, c1;
  short               x, y;
  byte *next_uuid;

    /*
     * initialize counters 
     */
    c0 = c1 = 0;
    next_uuid = (byte *) uuid;


/*
     * For speed lets unroll the following loop:
     *
     *   for (i = 0; i < UUID_K_LENGTH; i++)
     *   {
     *       c0 = c0 + *next_uuid++;
     *       c1 = c1 + c0;
     *   }
     */
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
 
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
      
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    c0 = c0 + *next_uuid++;
    c1 = c1 + c0;
    
    /*
     *  Calculate the value for "First octet" of the hash
     */
    x = -c1 % 255;
    if (x < 0)   
    {   
        x = x + 255;
    }
    
    /*
     *  Calculate the value for "second octet" of the hash
     */
    y = (c1 - c0) % 255;   
    if (y < 0)   
    {   
        y = y + 255;
    }

    /*
     * return the pieces put together
     */
 return ((y * 256) + x);
}

int in_zone(int virtual, int zone, ubyte type)
{
  int number = 0, location = 0;
  struct char_data *mob;
  struct obj_data *obj, *temp;

  switch(type) {
    case SCMD_MOB:
      for (mob = character_list; mob; mob = mob->next) {
        if (!IS_NPC(mob) || GET_MOB_VNUM(mob) != virtual || MOB_FLAGGED(mob, MOB_EXTRACT))
          continue;

        if (world[mob->in_room].zone == zone)
          number++;
      }
      break;
    case SCMD_OBJ:
      for (obj = object_list; obj; obj = obj->next) {
        if (GET_OBJ_VNUM(obj) != virtual)
          continue;

        temp = obj;
        if (temp->in_obj)
          while (temp->in_obj)
            temp = temp->in_obj;

        if (temp->carried_by)
          location = world[temp->carried_by->in_room].zone;
        else if (temp->in_room != NOWHERE)
          location = world[temp->in_room].zone;
        else if (temp->worn_by)
          location = world[temp->worn_by->in_room].zone;

        if (location == zone)
          number++;
      }
    default:
      break;
  }

  return (number);
}

int in_world(int virtual, ubyte type)
{
  int number = 0;
  struct char_data *mob;
  struct obj_data *obj;
  int sql_check_mob(long virtual);
  int sql_check_obj(long virtual);

  switch(type) {
    case SCMD_MOB:
      number = sql_check_mob(virtual);
      for (mob = character_list; mob; mob = mob->next) {
        if (!IS_NPC(mob) || GET_MOB_VNUM(mob) != virtual)
          continue;

        number++;
      }

      break;
    case SCMD_OBJ:
      number = sql_check_obj(virtual);
      for (obj = object_list; obj; obj = obj->next) {
        if (GET_OBJ_VNUM(obj) != virtual)
          continue;

        number++;
      }

      break;
    default:
      break;
  }

  return (number);
}

int get_skill(struct char_data *ch, int skill)
{
  int group, group_lvl;
  extern int find_group(char *name);
  extern int find_group_level(char *name);

  group_lvl = find_group_level(skill_name(skill));
  group = find_group(skill_name(skill));

  if (group < 0)
    return 0;

  if (GET_GROUP_LVL(ch, group) >= group_lvl)
    return (GET_GROUP_SKILL(ch, group));
  else
    return 0;
}

bool skill_success(struct char_data *ch, int skill_num, int modifier)
{
  int competency, diceroll;

  if ((skill_num < 1) || (skill_num > MAX_SKILLS)) {
    mudlog("SYSERR: Invalid skill passed to skill_success.", BRF, LVL_IMMORT, TRUE);
    return (FALSE);
  }

  diceroll = number(1, 101);        /* 101% is a complete failure */
  diceroll += modifier;	            /* Any modifier */
  competency = GET_SKILL(ch, skill_num);

  if (GET_LEVEL(ch) >= LVL_IMMORT)  /* Immortals always successful */
    diceroll = 0;

  if (PLR_FLAGGED(ch, PLR_DEBUG)) {
    sprintf(buf, "&G[Skill/Spell] Competency [%d%%], Diceroll [%d], Modifier [%d]&n\r\n",
      competency, diceroll, modifier);
    send_to_char(buf, ch);
  }

  if (competency >= diceroll)       /* Success */
    return (TRUE);

  return (FALSE);
}

/* Create a spell scroll with the given parameters */
#define SPELL_SCROLL  1280
struct obj_data *create_spell_scroll(int spellnum, int group, int level, char *name)
{
  struct obj_data *scroll = read_object(SPELL_SCROLL, VIRTUAL, NULL);
  struct extra_descr_data *new_descr;
  extern const char *skill_groups[];

  if (!scroll) {
    mudlog("SYSERR: Unable to load default spell scroll object.", BRF, LVL_IMMORT, TRUE);
    return NULL;
  }

  GET_OBJ_VAL(scroll, 0) = spellnum;         /* Assign the spell */
  GET_OBJ_VAL(scroll, 1) = group;            /* Assign the group */
  GET_OBJ_VAL(scroll, 2) = level;            /* Assign the prerequisite */
  GET_OBJ_COST(scroll) = 500 * level;

  /* Create a new extra description to describe what the spell scroll does */
  CREATE(new_descr, struct extra_descr_data, 1);
  sprintf(buf, "%s\r\n\t\t&G...%s...\r\n", scroll->ex_description->description, name);
  sprintf(buf, "%s\t\t\tof the group %s&n", buf, skill_groups[group]);
  sprintf(buf1, "%s %s", scroll->name, name);

  new_descr->keyword = str_dup(scroll->ex_description->keyword);
  new_descr->description = str_dup(buf);
  scroll->ex_description = new_descr;
  scroll->name = str_dup(buf1);

  return (scroll);
}

int find_moves(int skill_number)
{
  int i;
  extern int needed_moves[];

  for (i = 0; needed_moves[i] > 0; i += 2)
    if (needed_moves[i] == skill_number)
      return (needed_moves[i + 1]);

  return 0;
}

bool check_dead(struct char_data *ch)
{
  if (GET_POS(ch) == POS_DEAD)
    return (TRUE);

  if (IS_NPC(ch)) {
    if (MOB_FLAGGED(ch, MOB_EXTRACT))
      return (TRUE);
  } else {
    if (PLR_FLAGGED(ch, PLR_EXTRACT))
      return (TRUE);
  }

  if (!ch || ch->in_room < 0 || ch->in_room > top_of_world)
    return (TRUE);

  return (FALSE);
}

void mob_combat_dice(struct char_data *mob)
{
  int type;

  if (!mob)
    return;

  if (GET_LEVEL(mob) <= 3)
    type = 0;
  else if (GET_LEVEL(mob) <= 9)
    type = 1;
  else if (GET_LEVEL(mob) <= 19)
    type = 2;
  else if (GET_LEVEL(mob) <= 29)
    type = 3;
  else if (GET_LEVEL(mob) <= 39)
    type = 4;
  else if (GET_LEVEL(mob) <= 49)
    type = 5;
  else if (GET_LEVEL(mob) <= 59)
    type = 6;
  else if (GET_LEVEL(mob) <= 69)
    type = 7;
  else if (GET_LEVEL(mob) <= 79)
    type = 8;
  else if (GET_LEVEL(mob) <= 83)
    type = 9;
  else if (GET_LEVEL(mob) <= 89)
    type = 10;
  else if (GET_LEVEL(mob) <= 93)
    type = 11;
  else if (GET_LEVEL(mob) <= 96)
    type = 12;
  else
    type = 13;

  GET_NDD(mob) = mob_dice[type].ndd;
  GET_SDD(mob) = mob_dice[type].sdd;
}
