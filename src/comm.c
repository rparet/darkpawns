/* ************************************************************************
*   File: comm.c                                        Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
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

/* $Id: comm.c 1524 2008-06-20 20:26:02Z jravn $ */

#include "config.h"
#include "sysdep.h"
#include "vt100.h"

#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include "events.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "house.h"
#include "olc.h"
#include "ident.h"
#include "screen.h"
#include "random.h"
#include "spells.h"
#include "scripts.h"

#include <arpa/telnet.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

/* MCCP compression global defines */
#define COMPRESS2 86

/* externs */
extern int game_restrict;
extern int mini_mud;
extern int no_rent_check;
extern FILE *player_fl;
extern int DFLT_PORT;
extern char *DFLT_DIR;
extern const char *LOGNAME;
extern int MAX_PLAYERS;
extern int MAX_DESCRIPTORS_AVAILABLE;
extern struct char_data *character_list;

extern struct time_info_data time_info;     /* In db.c */
extern char help[];

/* local globals */
struct descriptor_data *descriptor_list = NULL;     /* master desc list */
struct txt_block *bufpool = 0;  /* pool of large output buffers */
int buf_largecount = 0;     /* # of large buffers which exist */
int buf_overflows = 0;      /* # of overflows of output */
int buf_switches = 0;       /* # of switches from small to large buf */
int circle_shutdown = 0;    /* clean shutdown */
int circle_reboot = 0;      /* reboot the game after a shutdown */
int no_specials = 0;        /* Suppress ass. of special routines */
int max_players = 0;        /* max descriptors available */
int tics = 0;           /* for extern checkpointing */
int scheck = 0;         /* for syntax checking mode */
extern int nameserver_is_slow;  /* see config.c */
extern int auto_save;       /* see config.c */
extern int autosave_time;   /* see config.c */
struct timeval null_time;   /* zero-valued time structure */
int port;           /* port mud is running on */
unsigned long pulse = 0;        /* number of pulses since game began */
FILE *logfile = NULL;           /* where to send the log messages. */

/* functions in this file */
int get_from_q(struct txt_q *queue, char *dest, int *aliased);
void do_broadcast(char *str, struct char_data *ch, struct obj_data *obj,
                  struct obj_data *vict_obj, int hide_invisible);
void write_mud_date_to_file(void);
void init_game(int port);
void signal_setup(void);
void game_loop(int mother_desc);
int init_socket(int port);
int new_descriptor(int s);
int get_max_players(void);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval a, struct timeval b);
struct timeval timeadd(struct timeval a, struct timeval b);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
int perform_subst(struct descriptor_data *t, char *orig, char *subst);
int perform_alias(struct descriptor_data *d, char *orig);
void record_usage(void);
char *make_prompt(struct descriptor_data *point);
void check_idle_passwords(void);
void heartbeat(int pulse);
int set_sendbuf(int s);
void hunt_items(void); /* new_cmds2.c */
void setup_log(const char *filename, int fd);
int open_logfile(const char *filename, FILE *stderr_fp);


/* extern fcnts */
void boot_db(void);
void boot_world(void);
void zone_update(void);
void affect_update(void);   /* In magic.c */
void point_update(void);    /* In limits.c */
void mobile_activity(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void show_string(struct descriptor_data *d, char *input);
int isbanned(char *hostname);
void weather_and_time(int mode);
void init_whod(int port);
void close_whod (void);
void whod_loop(void);
void save_clans(void);
void InfoBarUpdate(struct char_data *ch, int update);
void extract_pending_chars(void);

#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif

/* Implementation of required zlib helper functions. */
void *z_alloc(void *opaque, uInt items, uInt size)
{
  return calloc(items, size);
}

void z_free(void *opaque, void *address)
{
  free(address);
}

/* *********************************************************************
*  main game loop and related stuff                                    *
********************************************************************* */

int dp_main(int argc, char *argv[])
{
  char buf[MAX_STRING_LENGTH];
  int pos = 1;
  char *dir;

  port = DFLT_PORT;
  dir = DFLT_DIR;

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
    case 'd':
      if (*(argv[pos] + 2))
    dir = argv[pos] + 2;
      else if (++pos < argc)
    dir = argv[pos];
      else {
    puts("SYSERR: Directory arg expected after option -d.");
    exit(1);
      }
      break;
    case 'm':
      mini_mud = 1;
      no_rent_check = 1;
      puts("Running in minimized mode & with no rent check.");
      break;
    case 'c':
      scheck = 1;
      puts("Syntax check mode enabled.");
      break;
    case 'q':
      no_rent_check = 1;
      puts("Quick boot mode -- rent check supressed.");
      break;
    case 'r':
      game_restrict = 1;
      puts("Restricting game -- no new players allowed.");
      break;
    case 's':
      no_specials = 1;
      puts("Suppressing assignment of special routines.");
      break;
    default:
      sprintf(buf, "SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
      puts(buf);
      break;
    }
    pos++;
  }

  if (pos < argc) {
    if (!isdigit(*argv[pos])) {
      fprintf(stderr, "Usage: %s [-c] [-m] [-q] [-r] [-s] [-l] [-d pathname] [port #]\n", argv[0]);
      exit(1);
    } else if ((port = atoi(argv[pos])) <= 1024) {
      fprintf(stderr, "SYSERR: Illegal port number.\n");
      exit(1);
    }
  }

  /* All arguments have been parsed, try to open log file. */
  setup_log(LOGNAME, STDERR_FILENO);

  if (chdir(dir) < 0) {
    perror("SYSERR: Fatal error changing to data directory");
    exit(1);
  }
  sprintf(buf, "Using %s as data directory.", dir);
  log(buf);

  if (scheck) {
    boot_world();
    log("Done.");
    exit(0);
  } else {
    sprintf(buf, "Running game on port %d.", port);
    log(buf);
    init_game(port);
  }

  return 0;
}



/* Init sockets, run game, and cleanup sockets */
void init_game(int port)
{
  int mother_desc;

  prng_seed(time(0));

  log("Finding player limit.");
  max_players = get_max_players();

  event_init();
  boot_db();

  log("Opening mother connection.");
  mother_desc = init_socket(port);

  init_whod(port); /* Serapis 960808 */

  log("Signal trapping.");
  signal_setup();

  log("Entering game loop.");

  game_loop(mother_desc);

  log("Closing all sockets.");
  while (descriptor_list)
    close_socket(descriptor_list);

  close(mother_desc);

  save_clans(); /* Oro 990402 */
  close_whod(); /* Serapis 960808 */
  write_mud_date_to_file();

  fclose(player_fl);

  if (circle_reboot) {
    log("Rebooting.");
    exit(52);           /* what's so great about HHGTTG, anyhow? */
  }
  log("Normal termination of game.");
}



/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
int init_socket(int port)
{
  int s, opt;
  struct sockaddr_in sa;

  /*
   * Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so ths point is (hopefully) moot.
   */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("SYSERR: Error creating socket");
    exit(1);
  }

#if defined(SO_REUSEADDR)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }
#endif

   set_sendbuf(s);

#if defined(SO_LINGER)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0) {
      perror("SYSERR: setsockopt LINGER");
      exit(1);
    }
  }
#endif

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("SYSERR: bind");
    close(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5);
  return s;
}


int get_max_players(void)
{
  int max_descs = 0;
  char *method;

/*
 * First, we'll try using getrlimit/setrlimit.  This will probably work
 * on most systems.
 */
#if defined (RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#if !defined(RLIMIT_NOFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
  {
    struct rlimit limit;
    /* find the limit of file descs */
    method = "rlimit";
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("SYSERR: calling getrlimit");
      exit(1);
    }
    /* set the current to the maximum */
#ifdef OPEN_MAX
limit.rlim_max = limit.rlim_max > OPEN_MAX? OPEN_MAX : limit.rlim_max;
#endif
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("SYSERR: calling setrlimit");
      exit(1);
    }
#ifdef RLIM_INFINITY
    if (limit.rlim_max == RLIM_INFINITY)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else
      max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#else
    max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
  }

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
  method = "OPEN_MAX";
  max_descs = OPEN_MAX;     /* Uh oh.. rlimit didn't work, but we have
                 * OPEN_MAX */
#elif defined (POSIX)
  /*
   * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
   * use the POSIX sysconf() function.  (See Stevens' _Advanced Programming
   * in the UNIX Environment_).
   */
  method = "POSIX sysconf";
  errno = 0;
  if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
    if (errno == 0)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else {
      perror("SYSERR: Error calling sysconf");
      exit(1);
    }
  }
#else
  /* if everything has failed, we'll just take a guess */
  max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
#endif

  /* now calculate max _players_ based on max descs */
  max_descs = MIN(MAX_PLAYERS, max_descs - NUM_RESERVED_DESCS);

  if (max_descs <= 0) {
    sprintf(buf, "SYSERR: Non-positive max player limit! (Set at %d using %s).",
        max_descs, method);
    log(buf);
    exit(1);
  }
  sprintf(buf, "Setting player limit to %d using %s.", max_descs, method);
  log(buf);
  return max_descs;
}



/*
 * game_loop contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for accepting new
 * new connections, polling existing connections for input, dequeueing
 * output and sending it out to players, and calling "heartbeat" functions
 * such as mobile_activity().
 */
void game_loop(int mother_desc)
{
  fd_set input_set, output_set, exc_set, null_set;
  struct timeval last_time, before_sleep, opt_time, process_time, now, timeout;
  char comm[MAX_INPUT_LENGTH];
  struct descriptor_data *d, *next_d;
  int missed_pulses, maxdesc, aliased;
/*  int pulse = 0, missed_pulses, maxdesc, aliased; */


  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  FD_ZERO(&null_set);

  gettimeofday(&last_time, (struct timezone *) 0);

  /* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. */
  while (!circle_shutdown) {

    /* Serapis mod 100896 so whod works with no players.

       Sleep if we don't have any connections
    if (descriptor_list == NULL) {
      log("No connections.  Going to sleep.");
      FD_ZERO(&input_set);
      FD_SET(mother_desc, &input_set);
      if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
    if (errno == EINTR)
      log("Waking up to process signal.");
    else
      perror("SYSERR: Select coma");
      } else
    log("New connection.  Waking up.");
      gettimeofday(&last_time, (struct timezone *) 0);
    }
    */

    /* Set up the input, output, and exception sets for select(). */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);

    maxdesc = mother_desc;
    for (d = descriptor_list; d; d = d->next) {
      if (d->descriptor > maxdesc)
        maxdesc = d->descriptor;
      FD_SET(d->descriptor, &input_set);
      FD_SET(d->descriptor, &output_set);
      FD_SET(d->descriptor, &exc_set);
    }

    /*
     * At this point, we have completed all input, output and heartbeat
     * activity from the previous iteration, so we have to put ourselves
     * to sleep until the next 0.1 second tick.  The first step is to
     * calculate how long we took processing the previous iteration.
     */

    gettimeofday(&before_sleep, (struct timezone *) 0); /* current time */
    process_time = timediff(before_sleep, last_time);

    /*
     * If we were asleep for more than one pass, count missed pulses and sleep
     * until we're resynchronized with the next upcoming pulse.
     */
    if (process_time.tv_sec == 0 && process_time.tv_usec < OPT_USEC) {
      missed_pulses = 0;
    } else {
      missed_pulses = process_time.tv_sec * PASSES_PER_SEC;
      missed_pulses += process_time.tv_usec / OPT_USEC;
      process_time.tv_sec = 0;
      process_time.tv_usec = process_time.tv_usec % OPT_USEC;
    }

    /* Calculate the time we should wake up */
    last_time = timeadd(before_sleep, timediff(opt_time, process_time));

    /* Now keep sleeping until that time has come */
    gettimeofday(&now, (struct timezone *) 0);
    timeout = timediff(last_time, now);

    /* go to sleep */
    do {
      if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
    if (errno != EINTR) {
      perror("SYSERR: Select sleep");
      exit(1);
    }
      }
      gettimeofday(&now, (struct timezone *) 0);
      timeout = timediff(last_time, now);
    } while (timeout.tv_usec || timeout.tv_sec);

    whod_loop(); /* Serapis 960808 */

    /* poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("SYSERR: Select poll");
      return;
    }
    /* If there are new connections waiting, accept them. */
    if (FD_ISSET(mother_desc, &input_set))
      new_descriptor(mother_desc);

    /* kick out the freaky folks in the exception set */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &exc_set) || d->close_me) {
    FD_CLR(d->descriptor, &input_set);
    FD_CLR(d->descriptor, &output_set);
    close_socket(d);
      }
    }

    /* process descriptors with input pending */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &input_set))
    if (process_input(d) < 0)
      close_socket(d);
    }

    /* process descriptors with ident pending */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;

      if (waiting_for_ident(d))
    ident_check(d, pulse);
    }


    /* process commands we just read from process_input */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;

      if (!waiting_for_ident(d) && ((d->character ? --(d->character->wait) : 0) <= 0) &&
      get_from_q(&d->input, comm, &aliased)) {
    if (d->character) {
      /* reset the idle timer & pull char back from void if necessary */
      d->character->char_specials.timer = 0;
      if (!d->connected && GET_WAS_IN(d->character) != NOWHERE) {
        if (d->character->in_room != NOWHERE)
          char_from_room(d->character);
        char_to_room(d->character, GET_WAS_IN(d->character));
        GET_WAS_IN(d->character) = NOWHERE;
        act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
      }
    }
    if (d->character) {
      d->character->wait = 1;
    }
    d->has_prompt = 0;

    if (d->str)     /* writing boards, mail, etc.     */
      string_add(d, comm);
    else if (d->showstr_count)  /* reading something w/ pager     */
      show_string(d, comm);
    else if (d->connected != CON_PLAYING)   /* in menus, etc. */
      nanny(d, comm);
    else {          /* else: we're playing normally */
      if (aliased)      /* to prevent recursive aliases */
        d->has_prompt = 1;
      else {
        if (perform_alias(d, comm))     /* run it through aliasing system */
          get_from_q(&d->input, comm, &aliased);
      }
      command_interpreter(d->character, comm);  /* send it to interpreter */
    }
      }
    }

    /* send queued output out to the operating system (ultimately to user) */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (*(d->output) && FD_ISSET(d->descriptor, &output_set)) {
    if (process_output(d) < 0)
      close_socket(d);
    else
      d->has_prompt = 1;
      }
    }

    /* give each descriptor an appropriate prompt */
    for (d = descriptor_list; d; d = d->next) {
      if (!d->has_prompt) {
        write_to_descriptor(d->descriptor, make_prompt(d), d->comp);
        d->has_prompt = 1;
      }
    }

    /* kick out folks in the CON_CLOSE state */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (STATE(d) == CON_CLOSE)
    close_socket(d);
    }

    /*
     * Now, we execute as many pulses as necessary--just one if we haven't
     * missed any pulses, or make up for lost time if we missed a few
     * pulses by sleeping for too long.
     */
    missed_pulses++;

    if (missed_pulses <= 0) {
      log("SYSERR: **BAD** MISSED_PULSES NONPOSITIVE, TIME GOING BACKWARDS!!");
      missed_pulses = 1;
    }

    /* If we missed more than 30 seconds worth of pulses, forget it */
    if (missed_pulses > (30 * PASSES_PER_SEC)) {
      log("SYSERR: Missed more than 30 seconds worth of pulses");
      missed_pulses = 30 * PASSES_PER_SEC;
    }

    /* Now execute the heartbeat functions */
    while (missed_pulses--)
      heartbeat(++pulse);


    /* Roll pulse over after 10 hours */
/*    if (pulse >= (600 * 60 * PASSES_PER_SEC))
      pulse = 0;
*/
    /* Update tics for deadlock protection (UNIX only) */
    tics++;
  }
}

void room_activity(void)
{
  register struct char_data *ch, *next_ch;
  int was_in;

  void flow_room(struct char_data *ch);
  void raw_kill(struct char_data * ch, struct char_data * killer, int attacktype);
  void loud_mobs(struct char_data * ch);

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (!ch->in_room)
      continue;

    if (!IS_NPC(ch) && GET_ROOM_SCRIPT(ch->in_room) &&
        ROOM_SCRIPT_FLAGGED(ch->in_room, RS_ONPULSE))
      run_script(ch, ch, NULL, &world[ch->in_room], NULL, "onpulse", LT_ROOM);

    if (!PRF_FLAGGED(ch, PRF_NOHASSLE) && IS_AFFECTED(ch, AFF_FLAMING))
      damage(ch, ch, 15, SPELL_FLAMESTRIKE);

    if ((SECT(ch->in_room) == SECT_UNDERWATER) && !IS_AFFECTED(ch, AFF_WATERBREATHE)
        && !PRF_FLAGGED(ch, PRF_NOHASSLE))
      damage(ch, ch, 25, SPELL_DROWNING);

    if (SECT(ch->in_room) == SECT_WATER_NOSWIM)
      if (!IS_AFFECTED(ch, AFF_WATERWALK) && !IS_AFFECTED(ch, AFF_FLY) && !PRF_FLAGGED(ch, PRF_NOHASSLE))
        if (!has_boat(ch))
          damage(ch, ch, 25, SPELL_DROWNING);

    if (!IS_NPC(ch)) {
      if (GET_ROOM_SPEC(ch->in_room) != NULL)
        if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, 0, 0))
          return;

      if (world[ch->in_room].sector_type == SECT_FLYING && !IS_FLYING(ch)) {
        send_to_char("You fall from the sky...\r\n", ch);
        act ("$n falls from the sky...", TRUE, ch, NULL, NULL,TO_ROOM);
        if (CAN_GO(ch, DOWN)) {
          was_in = ch->in_room;
          char_from_room(ch);
          char_to_room(ch, world[was_in].dir_option[DOWN]->to_room);
          look_at_room(ch, 0);
        } else {
          char_from_room(ch);
          char_to_room(ch, real_room(5));  /* fall from sky DT */
          return;
        }
      }
      if (ch && ch->in_room > 0 && ROOM_FLOWS(ch->in_room) && !number(0,1))
        flow_room(ch);
    } else
      loud_mobs(ch);

    /* I admit, I'm cheating by putting this here, but it is a quick
       solution to a nefarious problem. */
    if ((GET_CON(ch) <= 0)  && (GET_LEVEL(ch) < LVL_IMMORT) && !IS_NPC(ch))
    {
      stc("You are too unhealthy to live!\r\n", ch);
      act("Suddenly, $n croaks before your very eyes!", TRUE, ch, 0, 0, TO_ROOM);
      GET_HIT(ch) = 1;  /* for when they re-enter the game */
      raw_kill(ch, NULL, TYPE_UNDEFINED);
    }
  }
}

void object_activity(void)
{
  extern struct obj_data *object_list;
  struct obj_data *obj, *next_obj;
  char location[10];

  for (obj = object_list; obj; obj = next_obj) {
    next_obj = obj->next;
    if (GET_OBJ_RNUM(obj) != NOTHING) {

      /* DEBUGGING */

      if ((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) || (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) || IS_CORPSE(obj))
        continue;

      if (GET_OBJ_WEIGHT(obj) != obj_proto[GET_OBJ_RNUM(obj)].obj_flags.weight) {
        if (!obj->contains) {
          if (obj->carried_by)
            sprintf(location, "carried");
          else if (obj->in_room)
            sprintf(location, "in room");
          else if (obj->worn_by)
            sprintf(location, "worn by");
          else
            sprintf(location, "unknown");
          sprintf(buf, "SYSERR: Object '%s' weight incorrect, location '%s'", obj->short_description, location);
          mudlog(buf, BRF, LVL_IMMORT, TRUE);
          GET_OBJ_WEIGHT(obj) = obj_proto[GET_OBJ_RNUM(obj)].obj_flags.weight;
        }
      }

      if (GET_OBJ_SCRIPT(obj) && OBJ_SCRIPT_FLAGGED(obj, OS_ONPULSE)) {
        if (obj->carried_by)
          run_script(NULL, NULL, obj, &world[obj->carried_by->in_room], NULL, "onpulse", LT_OBJ);
        else
          run_script(NULL, NULL, obj, &world[obj->in_room], NULL, "onpulse", LT_OBJ);
      }
    }
  }
}


void heartbeat(int pulse)
{
  static int mins_since_crashsave = 0;

  event_process();
  extract_pending_chars();

  if (!(pulse % PULSE_ZONE))
    zone_update();

  if (!(pulse % (15 * PASSES_PER_SEC)))     /* 15 seconds */
    {
      check_idle_passwords();
    }

  if (!(pulse % PULSE_MOBILE))
    {
      mobile_activity();
      room_activity();
      object_activity();
    }

  if (!(pulse % PULSE_VIOLENCE))
    perform_violence();

  if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) {
    weather_and_time(1);
    affect_update();
    point_update();
    hunt_items();
    fflush(player_fl);
  }
  if (auto_save && !(pulse % (60 * PASSES_PER_SEC))) {  /* 1 minute */
    if (++mins_since_crashsave >= autosave_time) {
      mins_since_crashsave = 0;
      Crash_save_all();
     /* House_save_all();  temporarily removed -rparet */
    }
  }
  if (!(pulse % (5 * 60 * PASSES_PER_SEC))) /* 5 minutes */
    record_usage();

  if (!(pulse % (60 * 60 * PASSES_PER_SEC)))    /* 60 minutes */
    write_mud_date_to_file();
}


/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

/*
 *  new code to calculate time differences, which works on systems
 *  for which tv_usec is unsigned (and thus comparisons for something
 *  being < 0 fail).  Based on code submitted by ss@sirocco.cup.hp.com.
 */

/*
 * code to return the time difference between a and b (a-b).
 * always returns a nonnegative value (floors at 0).
 */
struct timeval timediff(struct timeval a, struct timeval b)
{
  struct timeval rslt;

  if (a.tv_sec < b.tv_sec)
    return null_time;
  else if (a.tv_sec == b.tv_sec) {
    if (a.tv_usec < b.tv_usec)
      return null_time;
    else {
      rslt.tv_sec = 0;
      rslt.tv_usec = a.tv_usec - b.tv_usec;
      return rslt;
    }
  } else {          /* a->tv_sec > b->tv_sec */
    rslt.tv_sec = a.tv_sec - b.tv_sec;
    if (a.tv_usec < b.tv_usec) {
      rslt.tv_usec = a.tv_usec + 1000000 - b.tv_usec;
      rslt.tv_sec--;
    } else
      rslt.tv_usec = a.tv_usec - b.tv_usec;
    return rslt;
  }
}

/* add 2 timevals */
struct timeval timeadd(struct timeval a, struct timeval b)
{
  struct timeval rslt;

  rslt.tv_sec = a.tv_sec + b.tv_sec;
  rslt.tv_usec = a.tv_usec + b.tv_usec;

  while (rslt.tv_usec >= 1000000) {
    rslt.tv_usec -= 1000000;
    rslt.tv_sec++;
  }

  return rslt;
}


void record_usage(void)
{
  int sockets_connected = 0, sockets_playing = 0;
  struct descriptor_data *d;
  char buf[256];

  for (d = descriptor_list; d; d = d->next) {
    sockets_connected++;
    if (!d->connected)
      sockets_playing++;
  }

  sprintf(buf, "nusage: %-3d sockets connected, %-3d sockets playing",
      sockets_connected, sockets_playing);
  log(buf);

#ifdef RUSAGE
  {
    struct rusage ru;

    getrusage(0, &ru);
    sprintf(buf, "rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
        ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
    log(buf);
  }
#endif

}



/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  SEND_TO_Q(off_string, d);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) TELOPT_NAOFFD,
    (char) TELOPT_NAOCRD,
    (char) 0,
  };

  SEND_TO_Q(on_string, d);
}


static char *
get_status ( int percent )
{
  char *status;

  if (percent >= 100)
    status = str_dup("(excellent)");
  else if (percent >= 90)
    status = str_dup("(few scratches)");
  else if (percent >= 75)
    status = str_dup("(small wounds)");
  else if (percent >= 50)
    status = str_dup("(quite a few wounds)");
  else if (percent >= 30)
    status = str_dup("(big nasty wounds)");
  else if (percent >= 15)
    status = str_dup("(pretty hurt)");
  else if (percent >= 0)
    status = str_dup("(awful)");
  else
    status = str_dup("(nearly dead)");

  return(status);
}


static char *
target_status ( struct char_data *ch )
{
  struct char_data *target = FIGHTING(ch);
  int percent = -1;

  if (!ch || !target)
    return( str_dup("") );

  if (GET_MAX_HIT(target) > 0)
    percent = (100 * GET_HIT(target)) / GET_MAX_HIT(target);

  return( get_status(percent) );
}


static char *
tank_status ( struct char_data *ch )
{
  struct char_data *tank = NULL;
  int percent = -1;

  if ( !ch || !FIGHTING(ch) || !(tank = FIGHTING(FIGHTING(ch))) )
    return( str_dup("") );

  if (GET_MAX_HIT(tank) > 0)
    percent = (100 * GET_HIT(tank)) / GET_MAX_HIT(tank);

  return( get_status(percent) );
}


char *make_prompt(struct descriptor_data *d)
{
  static char prompt[256];

  /* note: prompt is truncated at MAX_PROMPT_LENGTH chars (structs.h) */

  struct char_data *ch = d->character;
  int count, max_count, update = FALSE;
  float percent;


  if (d->str)
    strcpy(prompt, "] ");
  else
    if (!d->connected && d->showstr_count)
      {
    sprintf(prompt, "\r%s[ %sReturn%s to continue, (%sq%s)uit,"
        " (%sr%s)efresh, (%sb%s)ack, or page "
        "number (%s%d%s/%s%d%s) ]%s",
        CCCYN(d->character, C_CMP),
        CCRED(d->character, C_CMP), CCCYN(d->character, C_CMP),
        CCRED(d->character, C_CMP), CCCYN(d->character, C_CMP),
        CCRED(d->character, C_CMP), CCCYN(d->character, C_CMP),
        CCRED(d->character, C_CMP), CCCYN(d->character, C_CMP),
        CCRED(d->character, C_CMP), d->showstr_page,
            CCCYN(d->character, C_CMP),
        CCRED(d->character, C_CMP), d->showstr_count,
            CCCYN(d->character, C_CMP),
        CCNRM(d->character, C_CMP));
      }
    else if (!d->connected)
      {
    char *status = NULL;

    *prompt = '\0';

    if (GET_INVIS_LEV(d->character))
      sprintf(prompt, "i%d ", GET_INVIS_LEV(d->character));

    if (GET_INFOBAR(ch) == INFOBAR_OFF)
    {
      /*
      if (PRF_FLAGGED(d->character, PRF_DISPHP))
        sprintf(prompt, "%s%s%d%sH ", prompt, CCRED(d->character, C_CMP),
          GET_HIT(d->character), CCNRM(d->character, C_CMP));

       if (PRF_FLAGGED(d->character, PRF_DISPMANA))
        sprintf(prompt, "%s%s%d%sM ", prompt, CCRED(d->character, C_CMP),
          GET_MANA(d->character), CCNRM(d->character, C_CMP));

       if (PRF_FLAGGED(d->character, PRF_DISPMOVE))
        sprintf(prompt, "%s%s%d%sV ", prompt, CCRED(d->character, C_CMP),
          GET_MOVE(d->character), CCNRM(d->character, C_CMP));
       */

         if (PRF_FLAGGED(ch, PRF_DISPHP)) {
          count = GET_HIT(ch);
          max_count = GET_MAX_HIT(ch);
          percent = (float)count / (float)max_count;
          if (percent >= 0.75)
             sprintf(prompt, "%s%s", prompt, CCGRN(ch, C_CMP));
          else if (percent >= 0.33)
             sprintf(prompt, "%s%s", prompt, CCYEL(ch, C_CMP));
          else
             sprintf(prompt, "%s%s", prompt, CCRED(ch, C_CMP));
          sprintf(prompt, "%s%d%sH ", prompt, count, CCNRM(ch, C_CMP));
         }
         if (PRF_FLAGGED(ch, PRF_DISPMANA)) {
          count = GET_MANA(ch);
          max_count = GET_MAX_MANA(ch);
          percent = (float)count / (float)max_count;
          if (percent >= 0.75)
             sprintf(prompt, "%s%s", prompt, CCGRN(ch, C_CMP));
          else if (percent >= 0.33)
             sprintf(prompt, "%s%s", prompt, CCYEL(ch, C_CMP));
          else
             sprintf(prompt, "%s%s", prompt, CCRED(ch, C_CMP));
          sprintf(prompt, "%s%d%sM ", prompt, count, CCNRM(ch, C_CMP));
         }
         if (PRF_FLAGGED(ch, PRF_DISPMOVE)) {
          count = GET_MOVE(ch);
          max_count = GET_MAX_MOVE(ch);
          percent = (float)count / (float)max_count;
          if (percent >= 0.75)
             sprintf(prompt, "%s%s", prompt, CCGRN(ch, C_CMP));
          else if (percent >= 0.33)
             sprintf(prompt, "%s%s", prompt, CCYEL(ch, C_CMP));
          else
             sprintf(prompt, "%s%s", prompt, CCRED(ch, C_CMP));
          sprintf(prompt, "%s%d%sV ", prompt, count, CCNRM(ch, C_CMP));
         }

       if (PRF_FLAGGED(d->character, PRF_DISPTARGET))
       {
        status = target_status(d->character);
        if (strcmp(status, ""))
          {
        char name[80];
        one_argument(PERS(FIGHTING(d->character), d->character), name);
        name[0] = UPPER(name[0]);
        sprintf(prompt, "%s%s%s:%s%s ", prompt,
            CCRED(d->character, C_CMP),
            name, status,
            CCNRM(d->character, C_CMP));
          }
        FREE(status);
       }

       if (PRF_FLAGGED(d->character, PRF_DISPTANK))
       {
        status = tank_status(d->character);
        if (strcmp(status, ""))
          sprintf(prompt, "%s%s%s:%s%s ", prompt,
              CCRED(d->character, C_CMP),
              PERS(FIGHTING(FIGHTING(d->character)), d->character),
              status,
              CCNRM(d->character, C_CMP));
        FREE(status);
       }

       if (PRF_FLAGGED(d->character, PRF_AFK))
         sprintf(prompt, "%sAFK%s ", CCRED(d->character, C_CMP),
          CCNRM(d->character, C_CMP));

       if (PRF_FLAGGED(d->character, PRF_INACTIVE))
         sprintf(prompt, "%sINACTIVE%s ", CCRED(d->character, C_CMP),
          CCNRM(d->character, C_CMP));

        strcat(prompt, "> ");
    }
    else
    {
         if (GET_MOVE(ch) != GET_LASTMOVE(ch)) {
          SET_BIT(update, INFO_MOVE);
          GET_LASTMOVE(ch) = GET_MOVE(ch);
         }
         if (GET_MAX_MOVE(ch) != GET_LASTMAXMOVE(ch)) {
           SET_BIT(update, INFO_MOVE);
           GET_LASTMAXMOVE(ch) = GET_MAX_MOVE(ch);
         }
         if (GET_MANA(ch) != GET_LASTMANA(ch)) {
          SET_BIT(update, INFO_MANA);
          GET_LASTMANA(ch) = GET_MANA(ch);
         }
         if (GET_MAX_MANA(ch) != GET_LASTMAXMANA(ch)) {
          SET_BIT(update, INFO_MANA);
          GET_LASTMAXMANA(ch) = GET_MAX_MANA(ch);
         }
         if (GET_HIT(ch) != GET_LASTHIT(ch)) {
          SET_BIT(update, INFO_HIT);
          GET_LASTHIT(ch) = GET_HIT(ch);
         }
         if (GET_MAX_HIT(ch) != GET_LASTMAXHIT(ch)) {
          SET_BIT(update, INFO_HIT);
          GET_LASTMAXHIT(ch) = GET_MAX_HIT(ch);
         }
         if (GET_GOLD(ch) != GET_LASTGOLD(ch)) {
          SET_BIT(update, INFO_GOLD);
          GET_LASTGOLD(ch) = GET_GOLD(ch);
         }
         if (GET_EXP(ch) != GET_LASTEXP(ch)) {
          SET_BIT(update, INFO_EXP);
          GET_LASTEXP(ch) = GET_EXP(ch);
         }
         if (update)
          InfoBarUpdate(ch, update);
          if (PRF_FLAGGED(d->character, PRF_AFK))
          {
             sprintf(prompt, "%sAFK%s ", CCRED(d->character, C_CMP),
                  CCNRM(d->character, C_CMP));
             strcat(prompt, "> ");
      }
      else
             sprintf(prompt, "> ");
    }
     } else
       *prompt = '\0';

     return prompt;
}


void write_to_q(char *txt, struct txt_q *queue, int aliased)
{
  struct txt_block *new;

  CREATE(new, struct txt_block, 1);
  CREATE(new->text, char, strlen(txt) + 1);
  strcpy(new->text, txt);
  new->aliased = aliased;

  /* queue empty? */
  if (!queue->head) {
    new->next = NULL;
    queue->head = queue->tail = new;
  } else {
    queue->tail->next = new;
    queue->tail = new;
    new->next = NULL;
  }
}



int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
  struct txt_block *tmp;

  /* queue empty? */
  if (!queue->head)
    return 0;

  tmp = queue->head;
  strcpy(dest, queue->head->text);
  *aliased = queue->head->aliased;
  queue->head = queue->head->next;

  FREE(tmp->text);
  FREE(tmp);

  return 1;
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
  int dummy;

  if (d->large_outbuf) {
    d->large_outbuf->next = bufpool;
    bufpool = d->large_outbuf;
  }
  while (get_from_q(&d->input, buf2, &dummy));
}

#if defined(SO_SNDBUF)
/* Sets the kernel's send buffer size for the descriptor */
int set_sendbuf(int s)
{
  int opt = MAX_SOCK_BUF;

  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt SNDBUF");
    return -1;
  }

#if 0
  if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt RCVBUF");
    return -1;
  }
#endif

  return 0;
}
#endif

#define COLOR_ON(ch) (t->character && (!IS_NPC(ch) ? (PRF_FLAGGED((ch), PRF_COLOR_1) ? 1 : 0) + \
                                       (PRF_FLAGGED((ch), PRF_COLOR_2) ? 2 : 0) : 0))

/* Expand built-in color shortcuts, e.g. &r -> red */
size_t color_expansion(char *txt, size_t size)
{
  int wantsize = 0, j;
  const char *i = NULL;
  char orig_buf[MAX_STRING_LENGTH];
  char *buf = orig_buf, *orig_txt = txt;

#define A "\x1B["
  const char *ANSI[] = { "&", A"0m",A"0;30m",A"0;34m",A"0;32m",A"0;36m",A"0;31m",
                         A"0;35m",A"0;33m",A"0;37m",A"1;30m",A"1;34m",A"1;32m",A"1;36m",A"1;31m",
                         A"1;35m",A"1;33m",A"1;37m", "!"};
#undef A

  const char *OUR = "&ndbgcrmywDBGCRMYW";

  while (*txt) {
    if (*txt == '&') {
      txt++;

      for (j = 0; OUR[j]; j++) {
        if (*txt == OUR[j]) {
          i = ANSI[j];
          break;
        }
      }

      if (OUR[j]) {
        while (*i) {
          *buf = *i;
          i++;
          wantsize++;
          if (wantsize < size)
            buf++;
        }
      } else {
        txt--;
        *buf = '&';
        wantsize++;
        if (wantsize < size)
          buf++;
      }
    } else {
      *buf = *txt;
      wantsize++;
      if (wantsize < size)
        buf++;
    }

    txt++;
  }

  *buf = '\0';
  wantsize++;
  strncpy(orig_txt, orig_buf, wantsize < size ? wantsize : size);

  return wantsize - 1; /* Don't include the trailing '\0' in size */
}

/* Add a new string to a player's output queue. For outside use. */
size_t write_to_output(struct descriptor_data *t, const char *txt, ...)
{
  va_list args;
  size_t left;

  va_start(args, txt);
  left = vwrite_to_output(t, txt, args);
  va_end(args);

  return left;
}


/* Add a new string to a player's output queue. */
size_t vwrite_to_output(struct descriptor_data *t, const char *format, va_list args)
{
  const char *text_overflow = "\r\nOVERFLOW\r\n";
  static char txt[MAX_STRING_LENGTH];
  size_t wantsize;
  int size;

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufspace == 0)
    return (0);

  wantsize = size = vsnprintf(txt, sizeof(txt), format, args);

  /* Only attempt color expansion if it doesn't overflow */
  if (COLOR_ON(t->character) && size >= 0 && wantsize < sizeof(txt))
    wantsize = size = color_expansion(txt, sizeof(txt));

  /* If exceeding the size of the buffer, truncate it for the overflow message */
  if (size < 0 || wantsize >= sizeof(txt)) {
    size = sizeof(txt) - 1;
    strcpy(txt + size - strlen(text_overflow), text_overflow);  /* strcpy: OK */
  }

  /*
   * If the text is too big to fit into even a large buffer, truncate
   * the new text to make it fit.  (This will switch to the overflow
   * state automatically because t->bufspace will end up 0.)
   */
  if (size + t->bufptr + 1 > LARGE_BUFSIZE) {
    size = LARGE_BUFSIZE - t->bufptr - 1;
    txt[size] = '\0';
    buf_overflows++;
  }

  /*
   * If we have enough space, just write to buffer and that's it! If the
   * text just barely fits, then it's switched to a large buffer instead.
   */
  if (t->bufspace > size) {
    strcpy(t->output + t->bufptr, txt); /* strcpy: OK (size checked above) */
    t->bufspace -= size;
    t->bufptr += size;
    return (t->bufspace);
  }

  buf_switches++;

  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {          /* else create a new one */
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output); /* strcpy: OK (size checked previously) */
  t->output = t->large_outbuf->text;    /* make big buffer primary */
  strcat(t->output, txt);   /* strcat: OK (size checked) */

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;

  return (t->bufspace);
}

/* ******************************************************************
*  socket handling                                                  *
****************************************************************** */

#if defined(SO_SNDBUF)
/* Sets the kernel's send buffer size for the descriptor */
int sent_sendbuf(int s)
{
  int opt = MAX_SOCK_BUF;

  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt SNDBUF");
    return -1;
  }

#if 0
  if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt RCVBUF");
    return -1;
  }
#endif

  return 0;
}
#endif

int new_descriptor(int s)
{
  int desc;
  int sockets_connected = 0;
  unsigned long addr;
  socklen_t addrlen;
  struct dns_entry dns;
  char buf[256];
  static int last_desc = 0; /* last descriptor number */
  struct descriptor_data *newd;
  struct sockaddr_in peer;
  struct hostent *from;
  char wildhost[HOST_LENGTH+1];
  char double_wild[HOST_LENGTH+1];
  extern char *GREETINGS;

  int get_host_from_cache(struct dns_entry *dnsd);
  void add_dns_host(struct dns_entry *dnsd, char *hostname);

  /* Initial compression negotiation string. */
  const char COMPRESS_OFFER[] =
  {
    (char) IAC,
    (char) WILL,
    (char) COMPRESS2,
    (char) 0,
  };

  /* accept the new connection */
  addrlen = sizeof(peer);
  if ((desc = accept(s, (struct sockaddr *) &peer, &addrlen)) < 0) {
    perror("accept");
    return -1;
  }
  /* keep it from blocking */
  nonblock(desc);

  /* set the send buffer size if available on the system */
#if defined (SO_SNDBUF)
  if (set_sendbuf(desc) < 0) {
     close(desc);
     return 0;
  }
#endif

  /* make sure we have room for it */
  for (newd = descriptor_list; newd; newd = newd->next) {
    sockets_connected++;
    if (newd->ident_sock != -1)
      sockets_connected++;
  }

  if (sockets_connected >= max_players) {
    write_to_descriptor(desc, "Sorry, Dark Pawns is full right now... please try again later!\r\n", NULL);
    close(desc);
    return 0;
  }
  /* create a new descriptor */
  CREATE(newd, struct descriptor_data, 1);

  /* find the numeric site address */
  addr = ntohl(peer.sin_addr.s_addr);
  dns.ip[0] = (int) ((addr & 0xFF000000) >> 24);
  dns.ip[1] = (int) ((addr & 0x00FF0000) >> 16);
  dns.ip[2] = (int) ((addr & 0x0000FF00) >> 8);
  dns.ip[3] = (int) ((addr & 0x000000FF));
  dns.name = NULL;
  dns.next = NULL;

  if(!get_host_from_cache(&dns)) { /* cache lookup failed */
     /* find the sitename */
     if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr,
                      sizeof(peer.sin_addr), AF_INET))) {

        /* resolution failed */
        if (!nameserver_is_slow)
          perror("gethostbyaddr");

        sprintf(newd->host, "%03u.%03u.%03u.%03u",
               (int) ((addr & 0xFF000000) >> 24),
               (int) ((addr & 0x00FF0000) >> 16),
               (int) ((addr & 0x0000FF00) >> 8),
           (int) ((addr & 0x000000FF)));

        sprintf(wildhost, "%03u.%03u.%03u.*",
               (int) ((addr & 0xFF000000) >> 24),
               (int) ((addr & 0x00FF0000) >> 16),
               (int) ((addr & 0x0000FF00) >> 8));

        sprintf(double_wild, "%03u.%03u.*.*",
               (int) ((addr & 0xFF000000) >> 24),
               (int) ((addr & 0x00FF0000) >> 16));


        if (!nameserver_is_slow) {
           sprintf(buf, "DNS lookup failed on %s.", newd->host);
           mudlog(buf, CMP, LVL_GOD, TRUE);
        }

     } else {
       strncpy(newd->host, from->h_name, HOST_LENGTH);
       *(newd->host + HOST_LENGTH) = '\0';
       add_dns_host(&dns, newd->host);
     }
  } else {
     strncpy(newd->host, dns.name, HOST_LENGTH);
  }

  /* determine if the site is banned */
  if ((isbanned(newd->host) == BAN_ALL) ||
      (isbanned(wildhost) == BAN_ALL)   ||
      (isbanned(double_wild) == BAN_ALL))
  {
    write_to_descriptor(desc, "Sorry, your site is banned.\r\n", NULL);
    close(desc);
    sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
    mudlog(buf2, CMP, LVL_GOD, TRUE);
    FREE(newd);
    return 0;
  }
#if 0
  /* Log new connections - probably unnecessary, but you may want it */
  sprintf(buf2, "New connection from [%s]", newd->host);
  mudlog(buf2, CMP, LVL_GOD, FALSE);
#endif

  /* initialize descriptor data */
  newd->descriptor = desc;
  newd->connected = CON_GET_NAME;
  newd->peer_port = peer.sin_port;
  newd->idle_tics = 0;
  newd->output = newd->small_outbuf;
  newd->bufspace = SMALL_BUFSIZE - 1;
  newd->next = descriptor_list;
  newd->login_time = time(0);

  CREATE(newd->comp, struct compr, 1);
  newd->comp->state = 1; /* waiting for compression request from client */
  newd->comp->stream = NULL;

  if (++last_desc == 1000)
    last_desc = 1;
  newd->desc_num = last_desc;

  /* prepend to list */
  descriptor_list = newd;

  SEND_TO_Q(COMPRESS_OFFER, newd);

  SEND_TO_Q(GREETINGS, newd);

  ident_start(newd, peer.sin_addr.s_addr);

  return 0;
}


int process_output(struct descriptor_data *t)
{
  static char i[MAX_SOCK_BUF];
  static int result;

  /* we may need this \r\n for later -- see below */
  strcpy(i, "\r\n");

  /* now, append the 'real' output */
  strcpy(i + 2, t->output);

  /* if we're in the overflow state, notify the user */
  if (t->bufptr < 0)
    strcat(i, "**OVERFLOW**");

  /* add the extra CRLF if the person isn't in compact mode */
  if (!t->connected && t->character && !PRF_FLAGGED(t->character, PRF_COMPACT))
    strcat(i + 2, "\r\n");

  /* add a prompt */
  strncat(i+2, make_prompt(t), MAX_PROMPT_LENGTH);

  /*
   * now, send the output.  If this is an 'interruption', use the prepended
   * CRLF, otherwise send the straight output sans CRLF.
   */
  if (t->has_prompt)      /*  && !t->connected) */
    result = write_to_descriptor(t->descriptor, i, t->comp);
  else
    result = write_to_descriptor(t->descriptor, i + 2, t->comp);

  /* handle snooping: prepend "% " and send to snooper */
  if (t->snoop_by) {
    SEND_TO_Q("% ", t->snoop_by);
    SEND_TO_Q(t->output, t->snoop_by);
    SEND_TO_Q("%%", t->snoop_by);
  }
  /*
   * if we were using a large buffer, put the large buffer on the buffer pool
   * and switch back to the small one
   */
  if (t->large_outbuf) {
    t->large_outbuf->next = bufpool;
    bufpool = t->large_outbuf;
    t->large_outbuf = NULL;
    t->output = t->small_outbuf;
  }
  /* reset total bufspace back to that of a small buffer */
  t->bufspace = SMALL_BUFSIZE - 1;
  t->bufptr = 0;
  *(t->output) = '\0';

  return result;
}

static ssize_t write_compressed(int desc, const char *txt, size_t length,
                                struct compr *comp)
{
  ssize_t written = 0;
  int bytes_copied, compr_result, tmp;

  /* Copy data to input buffer */
  if (comp->size_in + length > comp->max_in)
    bytes_copied = comp->max_in - comp->size_in;
  else
    bytes_copied = length;

  strncpy((char *)(comp->buff_in + comp->size_in), txt, bytes_copied);
  comp->size_in += bytes_copied;

  /* Set up stream input */
  comp->stream->avail_in = comp->size_in;
  comp->stream->next_in = comp->buff_in;

  /* Compress, flushing the output buffer when it fills */
  do {
    /* Set up stream output */
    comp->stream->avail_out = comp->max_out - comp->size_out;
    comp->stream->next_out = comp->buff_out + comp->size_out;

    compr_result = deflate(comp->stream, Z_SYNC_FLUSH);

    if (compr_result == Z_OK && !(comp->stream->avail_out)) {
      /* Buffer is full, flush and keep deflating */
      compr_result = 1;
    } else if (compr_result < 0) {
      /* Fatal zlib error */
      written = 0;
      break;
    } else {
      /* All text successfully deflated */
      compr_result = 0;
    }

    /* Adjust output size */
    comp->size_out = comp->max_out - comp->stream->avail_out;

    /*
     * Flush compressed data in buff_out.
     * If problems are encountered, try re-sending all data.
     */
    tmp = 0;
    while (comp->size_out > 0) {
      written = write(desc, comp->buff_out + tmp, comp->size_out);
      if (written <= 0) {
        /* Unsuccessful write or socket error, exit method with error code */
        compr_result = 0;
        break;
      }
      comp->size_out -= written;
      tmp += written;
    }
  } while (compr_result);

  /* Remove from input buffer what got compressed */
  bytes_copied = comp->size_in - comp->stream->avail_in;
  if (bytes_copied > 0)
    strncpy((char *)comp->buff_in, (char *)comp->buff_in + bytes_copied,
            comp->size_in - bytes_copied);
  comp->size_in = comp->stream->avail_in;

  if (written > 0)
      written = bytes_copied;

  return written;
}

/* perform_socket_write for all Non-Windows platforms */
ssize_t perform_socket_write(int desc, const char *txt, size_t length, struct compr *comp)
{
  ssize_t result = 0;

  /* Handle MCCP zlib compression. */
  if (comp && comp->state == 2)
    result = write_compressed(desc, txt, length, comp);
  else
    result = write(desc, txt, length);

  if (result > 0) {
    /* Write was successful. */
    return (result);
  }

  if (result == 0) {
    /* This should never happen! */
    log("SYSERR: Huh??  write() returned 0???  Please report this!");
    return (-1);
  }

  /*
   * result < 0, so an error was encountered - is it transient?
   * Unfortunately, different systems use different constants to
   * indicate this.
   */

#ifdef EAGAIN       /* POSIX */
  if (errno == EAGAIN)
    return (0);
#endif

#ifdef EWOULDBLOCK  /* BSD */
  if (errno == EWOULDBLOCK)
    return (0);
#endif

#ifdef EDEADLK      /* Macintosh */
  if (errno == EDEADLK)
    return (0);
#endif

  /* Looks like the error was fatal.  Too bad. */
  return (-1);
}

/*
 * write_to_descriptor takes a descriptor, and text to write to the
 * descriptor.  It keeps calling the system-level write() until all
 * the text has been delivered to the OS, or until an error is
 * encountered.
 *
 * Returns:
 * >=0  If all is well and good.
 *  -1  If an error was encountered, so that the player should be cut off.
 */
int write_to_descriptor(int desc, const char *txt, struct compr *comp)
{
  ssize_t bytes_written;
  size_t total = strlen(txt), write_total = 0;

  while (total > 0) {
    bytes_written = perform_socket_write(desc, txt, total, comp);

    if (bytes_written < 0) {
      /* Fatal error.  Disconnect the player. */
      perror("SYSERR: Write to socket");
      return (-1);
    } else if (bytes_written == 0) {
      /* Temporary failure -- socket buffer full. */
      return (write_total);
    } else {
      txt += bytes_written;
      total -= bytes_written;
      write_total += bytes_written;
    }
  }

  return (write_total);
}

/*
 * ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 */
int process_input(struct descriptor_data *t)
{
  int buf_length, bytes_read, space_left, failed_subst;
  char *ptr, *read_point, *write_point, *nl_pos = NULL, *comp_pos;
  char tmp[MAX_INPUT_LENGTH + 8];

  const char COMPRESS_START[] =
  {
    (char) IAC,
    (char) SB,
    (char) COMPRESS2,
    (char) IAC,
    (char) SE,
    (char) 0
  };

  const char COMPRESS_REQUEST[] =
  {
    (char) IAC,
    (char) DO,
    (char) COMPRESS2,
    (char) 0
  };

  const char NOCOMPRESS_REQUEST[] =
  {
    (char) IAC,
    (char) DONT,
    (char) COMPRESS2,
    (char) 0
  };

  /* first, find the point where we left off reading data */
  buf_length = strlen(t->inbuf);
  read_point = t->inbuf + buf_length;
  space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

  do {
    if (space_left <= 0) {
      log("process_input: about to close connection: input overflow");
      return -1;
    }
    if ((bytes_read = read(t->descriptor, read_point, space_left)) < 0) {
#ifdef EWOULDBLOCK
      if (errno == EWOULDBLOCK)
    errno = EAGAIN;
#endif /* EWOULDBLOCK */
      if (errno != EAGAIN && errno != EINTR) {
    perror("process_input: about to lose connection");
    return -1;      /* some error condition was encountered on
                 * read */
      } else
    return 0;       /* the read would have blocked: just means no
                 * data there but everything's okay */
    } else if (bytes_read == 0) {
      log("EOF on socket read (connection broken by peer)");
      return -1;
    }
    /* at this point, we know we got some data from the read */

    *(read_point + bytes_read) = '\0';  /* terminate the string */

    /* Check for compression request. */
    if (t->comp->state == 1 && buf_length + bytes_read >= 3 ) {
      if ((comp_pos = strstr(t->inbuf, COMPRESS_REQUEST))) {
    /* Client requested compression. */
    /* Send start of the compression stream. */
    write_to_descriptor(t->descriptor, COMPRESS_START, NULL);

    /* Init the compression stream. */
    CREATE(t->comp->stream, z_stream, 1);
    t->comp->stream->zalloc = z_alloc;
    t->comp->stream->zfree = z_free;
    t->comp->stream->opaque = Z_NULL;
    deflateInit(t->comp->stream, Z_DEFAULT_COMPRESSION);

    /* Init the compression buffers. */
    CREATE(t->comp->buff_out, Bytef, SMALL_BUFSIZE);
    t->comp->max_out = SMALL_BUFSIZE;
    t->comp->size_out = 0;
    CREATE(t->comp->buff_in, Bytef, SMALL_BUFSIZE);
    t->comp->max_in = SMALL_BUFSIZE;
    t->comp->size_in = 0;

    /* Turn compression state on. */
    t->comp->state = 2;
    log("MCCP compression successfully negotiated.");
      } else if ((comp_pos = strstr(t->inbuf, NOCOMPRESS_REQUEST))) {
      /* Client requested no compression. */
      t->comp->state = 0;
      }

      /* Remove [no]compression request from descriptor's buffer. */
      if (comp_pos) {
    strncpy(comp_pos, comp_pos + 3, space_left - 3);
    bytes_read -= 3;
      }
    }

    /* search for a newline in the data we just read */
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
    nl_pos = ptr;

    read_point += bytes_read;
    space_left -= bytes_read;

/*
 * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
 * causing the MUD to hang when it encounters input not terminated by a
 * newline.  This was causing hangs at the Password: prompt, for example.
 * I attempt to compensate by always returning after the _first_ read, instead
 * of looping forever until a read returns -1.  This simulates non-blocking
 * I/O because the result is we never call read unless we know from select()
 * that data is ready (process_input is only called if select indicates that
 * this descriptor is in the read set).  JE 2/23/95.
 */
#if !defined(POSIX_NONBLOCK_BROKEN)
  } while (nl_pos == NULL);
#else
  } while (0);

  if (nl_pos == NULL)
    return 0;
#endif /* POSIX_NONBLOCK_BROKEN */

  /*
   * okay, at this point we have at least one newline in the string; now we
   * can copy the formatted data to a new array for further processing.
   */

  read_point = t->inbuf;

  while (nl_pos != NULL) {
    write_point = tmp;
    space_left = MAX_INPUT_LENGTH - 1;

    for (ptr = read_point; (space_left > 0) && (ptr < nl_pos); ptr++) {
      if (*ptr == '\b') {   /* handle backspacing */
    if (write_point > tmp) {
      if (*(--write_point) == '$') {
        write_point--;
        space_left += 2;
      } else
        space_left++;
    }
      } else if (isascii(*ptr) && isprint(*ptr)) {
    if ((*(write_point++) = *ptr) == '$') {     /* copy one character */
      *(write_point++) = '$';   /* if it's a $, double it */
      space_left -= 2;
    } else
      space_left--;
      }
    }

    *write_point = '\0';

    if ((space_left <= 0) && (ptr < nl_pos)) {
      char buffer[MAX_INPUT_LENGTH + 64];

      sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
      if (write_to_descriptor(t->descriptor, buffer, t->comp) < 0)
    return -1;
    }
    if (t->snoop_by) {
      SEND_TO_Q("% ", t->snoop_by);
      SEND_TO_Q(tmp, t->snoop_by);
      SEND_TO_Q("\r\n", t->snoop_by);
    }
    failed_subst = 0;

    if (*tmp == '!')
      strcpy(tmp, t->last_input);
    else if (*tmp == '^') {
      if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
    strcpy(t->last_input, tmp);
    } else
      strcpy(t->last_input, tmp);

    if (!failed_subst)
      write_to_q(tmp, &t->input, 0);

    /* find the end of this line */
    while (ISNEWL(*nl_pos))
      nl_pos++;

    /* see if there's another newline in the input buffer */
    read_point = ptr = nl_pos;
    for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
    nl_pos = ptr;
  }

  /* now move the rest of the buffer up to the beginning for the next pass */
  write_point = t->inbuf;
  while (*read_point)
    *(write_point++) = *(read_point++);
  *write_point = '\0';

  return 1;
}



/*
 * perform substitution for the '^..^' csh-esque syntax
 * orig is the orig string (i.e. the one being modified.
 * subst contains the substition string, i.e. "^telm^tell"
 */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
  char new[MAX_INPUT_LENGTH + 5];

  char *first, *second, *strpos;

  /*
   * first is the position of the beginning of the first string (the one
   * to be replaced
   */
  first = subst + 1;

  /* now find the second '^' */
  if (!(second = strchr(first, '^'))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* terminate "first" at the position of the '^' and make 'second' point
   * to the beginning of the second string */
  *(second++) = '\0';

  /* now, see if the contents of the first string appear in the original */
  if (!(strpos = strstr(orig, first))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);
    return 1;
  }
  /* now, we construct the new string for output. */

  /* first, everything in the original, up to the string to be replaced */
  strncpy(new, orig, (strpos - orig));
  new[(strpos - orig)] = '\0';

  /* now, the replacement string */
  strncat(new, second, (MAX_INPUT_LENGTH - strlen(new) - 1));

  /* now, if there's anything left in the original after the string to
   * replaced, copy that too. */
  if (((strpos - orig) + strlen(first)) < strlen(orig))
    strncat(new, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(new) - 1));

  /* terminate the string in case of an overflow from strncat */
  new[MAX_INPUT_LENGTH - 1] = '\0';
  strcpy(subst, new);

  return 0;
}



void close_socket(struct descriptor_data *d)
{
  char buf[128];
  struct descriptor_data *temp;
  long target_idnum = -1;

  close(d->descriptor);
  flush_queues(d);

  if (d->ident_sock != -1)
    close(d->ident_sock);

  /* Forget snooping */
  if (d->snooping)
    d->snooping->snoop_by = NULL;

  if (d->snoop_by) {
    SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
    d->snoop_by->snooping = NULL;
  }

  /*. Kill any OLC stuff .*/
  switch(d->connected)
  { case CON_OEDIT:
    case CON_REDIT:
    case CON_ZEDIT:
    case CON_MEDIT:
    case CON_SEDIT:
    case CON_TEDIT:
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      break;
  }

  if (d->character) {
    if (PLR_FLAGGED(d->character, PLR_MAILING) && d->str)
    {
      if (*(d->str))
        FREE(*(d->str));
      FREE(d->str);
    }
    target_idnum = GET_IDNUM(d->character);
    if (d->connected == CON_PLAYING) {
      save_char(d->character, NOWHERE);
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
      d->character->desc = NULL;
    } else {
      sprintf(buf, "Losing player: %s.",
          GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
      mudlog(buf, CMP, LVL_IMMORT, TRUE);
      free_char(d->character);
    }
  } else
    mudlog("Losing descriptor without char.", CMP, LVL_IMMORT, TRUE);

  /* JE 2/22/95 -- part of my unending quest to make switch stable */
  if (d->original && d->original->desc)
    d->original->desc = NULL;

  REMOVE_FROM_LIST(d, descriptor_list, next);

  if (d->showstr_head)
    FREE(d->showstr_head);
  if (d->showstr_count)
    FREE(d->showstr_vector);

  /* Free compression structures. */
  if (d->comp->stream) {
    deflateEnd(d->comp->stream);
    free(d->comp->stream);
    free(d->comp->buff_out);
    free(d->comp->buff_in);
  }
  if (d->comp)
    free(d->comp);

  FREE(d);
}



void check_idle_passwords(void)
{
  struct descriptor_data *d, *next_d;

  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME && STATE(d) != CON_MENU)
      continue;
    if (!d->idle_tics) {
      d->idle_tics++;
      continue;
    } else {
      echo_on(d);
      SEND_TO_Q("\r\nTimed out... goodbye.\r\n", d);
      STATE(d) = CON_CLOSE;
    }
  }
}



/*
 * I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.)
 */

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(int s)
{
  int flags;

  flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
    perror("Fatal error executing nonblock (comm.c)");
    exit(1);
  }
}


/* ******************************************************************
*  signal-handling functions (formerly signals.c)                   *
****************************************************************** */


RETSIGTYPE checkpointing()
{
  if (!tics) {
    log("SYSERR: CHECKPOINT shutdown: tics not updated");
    abort();
  } else
    tics = 0;
}


RETSIGTYPE reread_wizlists()
{
  void reboot_wizlists(void);

  mudlog("Signal received - rereading wizlists.", CMP, LVL_IMMORT, TRUE);
  reboot_wizlists();
}


RETSIGTYPE unrestrict_game()
{
  extern struct ban_list_element *ban_list;
  extern int num_invalid;

  mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
     BRF, LVL_IMMORT, TRUE);
  ban_list = NULL;
  game_restrict = 0;
  num_invalid = 0;
}


RETSIGTYPE hupsig()
{
  log("Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
  exit(0);          /* perhaps something more elegant should
                 * substituted */
}


/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 *
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 *
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
 */

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc *my_signal(int signo, sigfunc * func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif

  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
#endif              /* NeXT */


void signal_setup(void)
{
  struct itimerval itime;
  struct timeval interval;

  /* user signal 1: reread wizlists.  Used by autowiz system. */
  my_signal(SIGUSR1, reread_wizlists);

  /*
   * user signal 2: unrestrict game.  Used for emergencies if you lock
   * yourself out of the MUD somehow.  (Duh...)
   */
  my_signal(SIGUSR2, unrestrict_game);

  /*
   * set up the deadlock-protection so that the MUD aborts itself if it gets
   * caught in an infinite loop for more than 3 minutes.  Doesn't work with
   * OS/2.
   */
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  my_signal(SIGVTALRM, checkpointing);

  /* just to be on the safe side: */
  my_signal(SIGHUP, hupsig);
  my_signal(SIGINT, hupsig);
  my_signal(SIGTERM, hupsig);
  my_signal(SIGPIPE, SIG_IGN);
  my_signal(SIGALRM, SIG_IGN);
}

/* ****************************************************************
*       Public routines for system-to-player-communication        *
**************************************************************** */

void send_to_char(const char *messg, struct char_data *ch)
{
  if (ch->desc && messg)
    SEND_TO_Q(messg, ch->desc);
}


void send_to_all(char *messg)
{
  struct descriptor_data *i;

  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
    SEND_TO_Q(messg, i);
}


void send_to_outdoor(char *messg)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character && AWAKE(i->character) &&
    OUTSIDE(i->character))
      SEND_TO_Q(messg, i);
}

void send_to_room(char *messg, int room)
{
  struct char_data *i;

  if (messg)
    for (i = world[room].people; i; i = i->next_in_room)
      if (i->desc && (GET_POS(i) > POS_SLEEPING))
    SEND_TO_Q(messg, i->desc);
}

void
send_to_zone(char *messg, struct char_data *ch)
{
  struct descriptor_data *i;

  if (messg && ch)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && AWAKE(i->character) &&
      (ch->in_room != (i->character)->in_room) &&
      (world[ch->in_room].zone == world[(i->character)->in_room].zone))
    SEND_TO_Q(messg, i);
}


char *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) {i = ACTNULL;} else {i = (expression);}


/* higher-level communication: the act() function */
void perform_act(char *orig, struct char_data *ch, struct obj_data *obj,
         void *vict_obj, struct char_data *to)
{
  register char *i = NULL, *buf;
  static char lbuf[MAX_STRING_LENGTH];

  buf = lbuf;

  for (;;) {
    if (*orig == '$') {
      switch (*(++orig)) {
      case 'n':
    i = PERS(ch, to);
    break;
      case 'N':
    CHECK_NULL(vict_obj, PERS((struct char_data *) vict_obj, to));
    break;
      case 'm':
    i = HMHR(ch);
    break;
      case 'M':
    CHECK_NULL(vict_obj, HMHR((struct char_data *) vict_obj));
    break;
      case 's':
    i = HSHR(ch);
    break;
      case 'S':
    CHECK_NULL(vict_obj, HSHR((struct char_data *) vict_obj));
    break;
      case 'e':
    i = HSSH(ch);
    break;
      case 'E':
    CHECK_NULL(vict_obj, HSSH((struct char_data *) vict_obj));
    break;
      case 'o':
    CHECK_NULL(obj, OBJN(obj, to));
    break;
      case 'O':
    CHECK_NULL(vict_obj, OBJN((struct obj_data *) vict_obj, to));
    break;
      case 'p':
    CHECK_NULL(obj, OBJS(obj, to));
    break;
      case 'P':
    CHECK_NULL(vict_obj, OBJS((struct obj_data *) vict_obj, to));
    break;
      case 'a':
    CHECK_NULL(obj, SANA(obj));
    break;
      case 'A':
    CHECK_NULL(vict_obj, SANA((struct obj_data *) vict_obj));
    break;
      case 'T':
    CHECK_NULL(vict_obj, (char *) vict_obj);
    break;
      case 'F':
    CHECK_NULL(vict_obj, fname((char *) vict_obj));
    break;
      case '$':
    i = "$";
    break;
      default:
    log("SYSERR: Illegal $-code to act():");
    strcpy(buf1, "SYSERR: ");
    strcat(buf1, orig);
    log(buf1);
    break;
      }
      while ((*buf = *(i++)))
    buf++;
      orig++;
    } else if (!(*(buf++) = *(orig++)))
      break;
  }

  *(--buf) = '\r';
  *(++buf) = '\n';
  *(++buf) = '\0';

  SEND_TO_Q(CAP(lbuf), to->desc);
}

static bool SENDOK(struct char_data *ch, bool to_sleep) {
  return (ch)->desc && (AWAKE(ch) || to_sleep)
    && !PLR_FLAGGED((ch), PLR_WRITING);
}

void act(char *str, int hide_invisible, struct char_data *ch,
     struct obj_data *obj, void *vict_obj, int type)
{
  struct char_data *to;
  int sleep;

  if (!str || !*str)
    return;

  /*
   * Warning: the following TO_SLEEP code is a hack.
   *
   * I wanted to be able to tell act to deliver a message regardless of sleep
   * without adding an additional argument.  TO_SLEEP is 128 (a single bit
   * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
   * command.  It's not legal to combine TO_x's with each other otherwise.
   */

  /* check if TO_SLEEP is there, and remove it if it is. */
  if ((sleep = (type & TO_SLEEP)))
    type &= ~TO_SLEEP;

  if (type == TO_CHAR) {
    if (ch && SENDOK(ch, sleep))
      perform_act(str, ch, obj, vict_obj, ch);
    return;
  }
  if (type == TO_VICT) {
    if ((to = (struct char_data *) vict_obj) && SENDOK(to, sleep))
      perform_act(str, ch, obj, vict_obj, to);
    return;
  }
  /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

  if (ch && ch->in_room != NOWHERE)
    to = world[ch->in_room].people;
  else if (obj && obj->in_room != NOWHERE)
    to = world[obj->in_room].people;
  else {
    log("SYSERR: no valid target to act()!");
    mudlog("SYSERR: no valid target to act()!", CMP, LVL_IMMORT, TRUE);
    return;
  }

  if (ch && ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
    do_broadcast(str, ch, obj, vict_obj, hide_invisible);

  for (; to; to = to->next_in_room)
    if (SENDOK(to, sleep) && !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
    (to != ch) && (type == TO_ROOM || (to != vict_obj)))
      perform_act(str, ch, obj, vict_obj, to);
}


void do_broadcast(char *str, struct char_data *ch, struct obj_data *obj,
          struct obj_data *vict_obj, int hide_invisible)
{
  struct char_data *to;
  static char buf[128];

  sprintf(buf, "&RBroadcast: %s&n", str);

  for (to = character_list; to; to = to->next)
     {
       if (PRF_FLAGGED(to, PRF_NOBROAD))
         continue;
       if (SENDOK(to, TRUE) && !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
          (to != ch))
       {
         perform_act(buf, ch, obj, vict_obj, to);
       }
     }
}

/* Prefer the file over the descriptor. */
void
setup_log(const char *filename, int fd)
{
  FILE *s_fp;

#if defined(__MWERKS__) || defined(__GNUC__)
  s_fp = stderr;
#else
  if ((s_fp = fdopen(STDERR_FILENO, "w")) == NULL) {
    puts("SYSERR: Error opening stderr, trying stdout.");

    if ((s_fp = fdopen(STDOUT_FILENO, "w")) == NULL) {
      puts("SYSERR: Error opening stdout, trying a file.");

      /* If we don't have a file, try a default. */
      if (filename == NULL || *filename == '\0')
        filename = "log/syslog";
    }
  }
#endif

  if (filename == NULL || *filename == '\0') {
    /* No filename, set us up with the descriptor we just opened. */
    logfile = s_fp;
    puts("Using file descriptor for logging.");
    return;
  }

  /* We honor the default filename first. */
  if (open_logfile(filename, s_fp))
    return;

  /* Well, that failed but we want it logged to a file so try a default. */
  if (open_logfile("log/syslog", s_fp))
    return;

  /* Ok, one last shot at a file. */
  if (open_logfile("syslog", s_fp))
    return;

  /* Erp, that didn't work either, just die. */
  puts("SYSERR: Couldn't open anything to log to, giving up.");
  exit(1);
}

int
open_logfile(const char *filename, FILE *stderr_fp)
{
  if (stderr_fp)        /* freopen() the descriptor. */
    logfile = freopen(filename, "w", stderr_fp);
  else
    logfile = fopen(filename, "w");

  if (logfile) {
    printf("Using log file '%s'%s.\n",
                filename, stderr_fp ? " with redirection" : "");
    return TRUE;
  }

  printf("SYSERR: Error opening file '%s': %s\n", filename, strerror(errno));
  return FALSE;
}


void
write_mud_date_to_file()
{
  FILE *f;

  f = fopen("etc/date_record", "w");

  sprintf(buf, "%d %d %d", time_info.year, time_info.month, time_info.day);
  fwrite(buf, sizeof(char), strlen(buf), f);

  fclose(f);

  log("Saving mud time.");
}
