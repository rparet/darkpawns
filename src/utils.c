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

/* $Id: utils.c 1525 2008-06-26 21:54:40Z jravn $ */

#include <stdarg.h>
#include <regex.h>

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "random.h"

extern struct time_data time_info;
extern struct room_data *world;
extern int intelligent_races[];
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern int is_shopkeeper(struct char_data * chChar);
extern int top_of_world;

struct char_data *HUNTING(struct char_data *ch);

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return (int)(uniform() * (to - from + 1)) + from;
}

/* simulates dice roll */
int dice(int num, int size)
{
  int sum = 0;

  if (size <= 0 || num <= 0)
    return 0;

  while (num--)
    sum += number(1, size);

  return sum;
}

/* returns a float from a uniform distribution in [0, 1) */
float uniform() {
  return prng_uniform();
}

int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}

/* Create a duplicate of a string. Returns null if source is null. */
char *str_dup(const char *source)
{
  if (source)
    return strdup(source);
  else
    return NULL;
}

/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
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
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
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

  sprintf(buf, "%s hit death trap #%d (%s)", GET_NAME(ch),
      world[ch->in_room].number, world[ch->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
}

/*
 * New variable argument log() function.  Works the same as the old for
 * previously written code but is very nice for new code.
 */
void basic_mud_log(const char *format, ...)
{
  va_list args;
  time_t ct = time(0);
  char *time_s = asctime(localtime(&ct));

  if (logfile == NULL)
    puts("SYSERR: Using log() before stream was initialized!");
  if (format == NULL)
    format = "SYSERR: log() received a NULL format.";

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
void mudlog(char *str, char type, int level, byte file)
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

void sprintbit(long bitvector, char *names[], char *result)
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

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24; /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY); /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;
  now.moon = 0;

  return now;
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35; /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);    /* 0..XX? years */

  now.moon = 0;

  return now;
}

struct time_info_data age(struct char_data *ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;    /* All players start at 17 */

  return player_age;
}

struct time_info_data playing_time(struct char_data *ch)
{
  struct time_info_data pt;

  time_t secs = (time(0) - ch->player.time.logon) + ch->player.time.played;

  pt.year = 0;
  pt.month = 0;
  pt.moon = 0;

  pt.day = secs / SECS_PER_REAL_DAY;
  secs -= pt.day * SECS_PER_REAL_DAY;

  pt.hours = secs / SECS_PER_REAL_HOUR;

  return pt;
}

bool is_veteran(struct char_data *ch)
{
  return playing_time(ch).day >= 30 && GET_KILLS(ch) >= 10000;
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
    REMOVE_BIT_AR(AFF_FLAGS(rider), AFF_MOUNT);
  if (mount && IS_MOUNTED(mount))
    REMOVE_BIT_AR(AFF_FLAGS(mount), AFF_MOUNT);
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
          REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_DODGE);
          shadowing = TRUE;
        }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
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

  if (ch->master->followers->follower == ch) {  /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    FREE(k);
  } else {          /* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    FREE(j);
  }

  ch->master = NULL;
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_CHARM);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
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
    if (*temp)
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

  switch (mode) {
  case POOF_FILE:
    prefix = "plrpoof";
    suffix = "poof";
    break;

  case ALIAS_FILE:
    prefix = "plralias";
    suffix = "alias";
    break;
  case CRASH_FILE:
    prefix = "plrobjs";
    suffix = "objs";
    break;
  case ETEXT_FILE:
    prefix = "plrtext";
    suffix = "text";
    break;
  default:
    return 0;
    break;
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
        case 'h': return RACE_HUMAN;
        case 'e': return RACE_ELF;
        case 'd': return RACE_DWARF;
        case 'k': return RACE_KENDER;
        case 'r': return RACE_RAKSHASA;
        case 'm': return RACE_MINOTAUR;
        case 's': return RACE_SSAUR;
        default: break;
    }
    return RACE_UNDEFINED;
}

struct char_data *
get_mount(struct char_data *ch)
{
  struct follow_type *j, *f;

  if (!ch || !IS_MOUNTED(ch))
    return (NULL);

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
  if (!ch || (HUNTING(ch)&&HUNTING(ch)==vict) )
    return;

  (ch)->char_specials.hunting_id = 0;
  (ch)->char_specials.hunting = NULL;

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
  else
    return(get_char_by_id((ch)->char_specials.hunting_id));
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

/* strlcpy function ported from Circle 3.1 - needed by sprintnbit */
/*
 * A 'strlcpy' function in the same fashion as 'strdup' below.
 *
 * This copies up to totalsize - 1 bytes from the source string, placing
 * them and a trailing NUL into the destination string.
 *
 * Returns the total length of the string it tried to copy, not including
 * the trailing NUL.  So a '>= totalsize' test says it was truncated.
 * (Note that you may have _expected_ truncation because you only wanted
 * a few characters from the source string.)
 */

/*
 size_t strlcpy(char *dest, const char *source, size_t totalsize)
{
  strncpy(dest, source, totalsize - 1);
  dest[totalsize - 1] = '\0';
  return strlen(source);
}

*/

/* sprintnbit function ported from Circle 3.1 (called sprintbit in Circle3.1) */
/*
 * If you don't have a 'const' array, just cast it as such.  It's safer
 * to cast a non-const array as const than to cast a const one as non-const.
 * Doesn't really matter since this function doesn't change the array though.
 */
size_t sprintnbit(bitvector_t bitvector, const char *names[], char *result, size_t reslen)
{
  size_t len = 0;
  int nlen;
  long nr;

  *result = '\0';

  for (nr = 0; bitvector && len < reslen; bitvector >>= 1) {
    if (IS_SET(bitvector, 1)) {
      nlen = snprintf(result + len, reslen - len, "%s ", *names[nr] != '\n' ? names[nr] : "UNDEFINED");
      if (len + nlen >= reslen || nlen < 0)
        break;
      len += nlen;
    }

    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    len = strlcpy(result, "NOBITS ", reslen);

  return (len);
}

void sprintbitarray(int bitvector[], char *names[], int maxar, char *result)
{
  int i;
  size_t len;
  char tmp[MAX_STRING_LENGTH/4];

  *result = '\0';
  len = 0;

  for (i = 0; i < maxar; i++) {
    len = sprintnbit((bitvector_t)bitvector[i], (const char**)(names + i*32), tmp, MAX_STRING_LENGTH/4);
    if (strcmp(tmp, "NOBITS ")) {
      strcat(result, tmp);
    }
  }

  if (!*result)
    strncpy(result, "NOBITS ", MAX_STRING_LENGTH);
}

/* old sprintbitarray function */
/* void sprintbitarray(int bitvector[], char *names[], int maxar, char *result) */
/* { */
/*    int nr, teller, found = FALSE; */

/*    *result = '\0'; */

/*    for(teller = 0; teller < maxar && !found; teller++) */
/*       for(nr = 0; nr < 32 && !found; nr++) { */
/*          if(IS_SET_AR(bitvector, (teller*32)+nr)) */
/*     if((*names)[(teller*32)+nr] != '\n') { */
/*       if((*names)[(teller*32)+nr] != '\0') { */
/*                   strcat(result, names[(teller *32)+nr]); */
/*                   strcat(result, " "); */
/*                } */
/*             } else { */
/*                strcat(result, "UNDEFINED "); */
/*             } */
/*          if((*names)[(teller*32)+nr] == '\n') */
/*             found = TRUE; */
/*       } */

/*    if(!*result) */
/*       strcpy(result, "NOBITS "); */
/* } */


/* check to see if there is a circle of summoning in the room.
   Also, decrement the counter due to movment in the room or use.
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
  log("SYSERR: Assertion failed at %s:%d!", who, line);

#if defined(CIRCLE_UNIX)
  /* These would be duplicated otherwise... */
  fflush(stdout);
  fflush(stderr);
  fflush(logfile);

  /*
   * Kill the child so the debugger or script doesn't think the MUD
   * crashed.  The 'autorun' script would otherwise run it again.
   */
  if (fork() == 0)
    abort();
#endif
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

/* Regular expression pattern checking */
bool matches(const char *s, const char *regex)
{
  int result;
  regex_t r;

  regcomp(&r, regex, REG_EXTENDED);

  result = !regexec(&r, s, 0, NULL, 0);

  regfree(&r);

  return result;
}
