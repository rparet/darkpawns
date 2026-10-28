/* ************************************************************************e
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
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

/* $Id: db.c 1330 2007-08-05 01:18:21Z jravn $ */

#define __DB_C__

#include <math.h>
#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "interpreter.h"
#include "house.h"
#include "constants.h"
#include "olc.h"
#include <mysql/mysql.h>

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world = NULL;	/* array of rooms		 */
long top_of_world = 0;		/* ref to top element of world	 */

struct char_data *character_list = NULL;	/* global linked list of
						 * chars	 */
struct index_data *mob_index;	/* index table for mobile file	 */
struct char_data *mob_proto;	/* prototypes for mobs		 */
int top_of_mobt = 0;		/* top of mobile index table	 */

struct obj_data *object_list = NULL;	/* global linked list of objs	 */
struct index_data *obj_index;	/* index table for object file	 */
struct obj_data *obj_proto;	/* prototypes for objs		 */
int top_of_objt = 0;		/* top of object index table	 */

struct tlist_data *treasure_list = NULL; /* index table for treasure objects */
int top_of_treasure = 0;                 /* top of treasure item table       */

struct zone_data *zone_table;	/* zone table			 */
int top_of_zone_table = 0;	/* top element of zone tab	 */
struct message_list fight_messages[MAX_MESSAGES];/* fighting messages	 */

struct player_index_element *player_table = NULL;/* index to plr file	 */
MYSQL *darkpawns = NULL;        /* SQL database for the mud */
int top_of_p_table = 0;		/* ref to top of table		 */
long top_idnum = 0;		/* highest idnum in use		 */
long *unused_ids = NULL;  	/* List of unused id's in the ptable */
ush_int max_play = 1;		/* maximum number of players recorded */

int no_mail = 0;		/* mail disabled?		 */
int mini_mud = 0;		/* mini-mud mode?		 */
int no_rent_check = 0;		/* skip rent check on boot?	 */
const char *sql_database = "main"; /* SQL database to use?          */
time_t boot_time = 0;		/* time of mud boot		 */
int game_restrict = 0;		/* level of game restriction	 */
time_t shops_time = 0;          /* time shops were last cleared */
sh_int r_mortal_start_room[NUM_HOMETOWNS];	/* rnum of mortal start room	 */
sh_int r_immort_start_room;			/* rnum of immort start room	 */
sh_int r_frozen_start_room;			/* rnum of frozen start room	 */
sh_int start_room;		/* rnum of initial char creation room */
bool socials = FALSE;

char *credits = NULL;		/* game credits			 */
char *news = NULL;		/* mud news			 */
char *motd = NULL;		/* message of the day - mortals */
char *motd_mxp = NULL;          /* message of the day - mortals (MXP) */
char *imotd = NULL;		/* message of the day - immorts */
char *help = NULL;		/* help screen			 */
char *info = NULL;		/* info page			 */
char *wizlist = NULL;		/* list of higher gods		 */
char *immlist = NULL;		/* list of peon gods		 */
char *background = NULL;	/* background story		 */
char *handbook = NULL;		/* handbook for new immortals	 */
char *policies = NULL;		/* policies page		 */
char *creation = NULL;          /* creation background story	 */
char *future = NULL;            /* future command */
char *mxp = NULL;		/* MXP element setup 		 */

struct help_index_element *help_table = 0;	/* the help table	 */
int top_of_helpt = 0;		/* top of help index table	 */

struct dns_entry *dns_cache[DNS_HASH_NUM]; /* for dns caching * dnsmod */

struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;	/* the infomation about the weather */
struct player_special_data dummy_mob;	/* dummy spec area for mobs	 */
struct reset_q_type reset_q;	/* queue of zones to be reset	 */

struct review_t review_goss[25];     /* list of last 25 gossips */
struct review_t review_auct[25];     /* list of last 25 auctions */
struct review_t review_wizn[25];     /* list of last 25 wiznets */

struct ban_list_element *ban_list = NULL;   /* Ban listing */
bool daylight_saving_time;      /* daylight savings or not */
int boot_lua();                 /* scripts.c */

struct event_data *pending_events = NULL;   /* global pending commands */
int num_events = 0;                         /* number of pending commands */

/* local functions */
int check_object_spell_number(struct obj_data *obj, int val);
int check_object_level(struct obj_data *obj, int val);
int check_object(struct obj_data *);
void setup_dir(FILE * fl, long room, int dir);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode, char *filename);
void parse_room(FILE * fl, long virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
char *parse_object(FILE * obj_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_help(FILE *fl);
void load_treasure(void);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
void sql_remove_deleted_players(void);
void char_to_store(struct char_data * ch, struct char_file_u * st);
void store_to_char(struct char_file_u * st, struct char_data * ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
int file_to_string_alloc(char *name, char **buf);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(int zone, int cmd_no, char *message);
void reset_time(void);
void boot_dns(void); /* dnsmod */
void save_dns_cache(void); /* dnsmod */
int get_host_from_cache(struct dns_entry *dnsd); /* dnsmod */
void add_dns_host(struct dns_entry *dnsd, char *hostname); /* dnsmod */
long get_ptable_by_name(char *name);
void get_one_line(FILE *fl, char *buf);
void load_spell_scroll(struct char_data *mob);
void unload_room_descs(int zone);

/* external functions */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void mag_assign_spells(void);
void boot_social_messages(void);
void update_obj_file(void);	/* In objsave.c */
void sort_commands(void);
void sort_spells(void);
void Read_Invalid_List(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
int hsort(const void *a, const void *b);
void tattoo_af(struct char_data * ch, bool add);
void init_clans(void);
void load_compile_time();
extern int isname_with_abbrevs(char *str, char *namelist);
extern int is_shopkeeper(struct char_data * chChar);
extern int give_multiple_items(struct char_data * chChar, struct obj_data *object);
void stonekeep_init(void);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
long treasure_item(int type);
void boot_mysql(void);
void sql_build_player_index(void);
void sql_read_mud_date(void);
void sql_shop_boot(void);
void sql_integrate_shops(void);
void sql_load_banned(void);
void sql_count_mail(void);
void sql_load_mobs(int zone);
void sql_load_objs(int zone);

/* external vars */
extern int no_specials;
extern const char *unused_spellname;
extern char *hometowns[];
extern int scheck;

#define READ_SIZE 256

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* this is necessary for the autowiz system */
void
reboot_wizlists(void)
{
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
}


ACMD(do_reboot)
{
  int i;

  one_argument(argument, arg);

  if (!str_cmp(arg, "all") || *arg == '*')
    {
      file_to_string_alloc(WIZLIST_FILE, &wizlist);
      file_to_string_alloc(IMMLIST_FILE, &immlist);
      file_to_string_alloc(NEWS_FILE, &news);
      file_to_string_alloc(CREDITS_FILE, &credits);
      file_to_string_alloc(MOTD_FILE, &motd);
      file_to_string_alloc(MOTD_FILE_MXP, &motd_mxp);
      file_to_string_alloc(IMOTD_FILE, &imotd);
      file_to_string_alloc(HELP_PAGE_FILE, &help);
      file_to_string_alloc(INFO_FILE, &info);
      file_to_string_alloc(POLICIES_FILE, &policies);
      file_to_string_alloc(CREATION_FILE, &creation);
      file_to_string_alloc(HANDBOOK_FILE, &handbook);
      file_to_string_alloc(BACKGROUND_FILE, &background);
      file_to_string_alloc(FUTURE_FILE, &future); 
      file_to_string_alloc(MXP_FILE, &mxp); 
   }
  else if (!str_cmp(arg, "wizlist"))
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
  else if (!str_cmp(arg, "immlist"))
    file_to_string_alloc(IMMLIST_FILE, &immlist);
  else if (!str_cmp(arg, "news"))
    file_to_string_alloc(NEWS_FILE, &news);
  else if (!str_cmp(arg, "credits"))
    file_to_string_alloc(CREDITS_FILE, &credits);
  else if (!str_cmp(arg, "motd")) {
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(MOTD_FILE_MXP, &motd_mxp);
  } else if (!str_cmp(arg, "imotd"))
    file_to_string_alloc(IMOTD_FILE, &imotd);
  else if (!str_cmp(arg, "help"))
    file_to_string_alloc(HELP_PAGE_FILE, &help);
  else if (!str_cmp(arg, "info"))
    file_to_string_alloc(INFO_FILE, &info);
  else if (!str_cmp(arg, "policy"))
    file_to_string_alloc(POLICIES_FILE, &policies);
  else if (!str_cmp(arg, "creation"))
    file_to_string_alloc(CREATION_FILE, &creation);
  else if (!str_cmp(arg, "handbook"))
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
  else if (!str_cmp(arg, "background"))
    file_to_string_alloc(BACKGROUND_FILE, &background);
  else if (!str_cmp(arg, "future"))
    file_to_string_alloc(FUTURE_FILE, &future);
  else if (!str_cmp(arg, "mxp"))
    file_to_string_alloc(MXP_FILE, &mxp);
  else if (!str_cmp(arg, "xhelp"))
    {
      if (help_table)
	{
	  for (i = 0; i <= top_of_helpt; i++)
	    {
	      if (help_table[i].keyword)
		FREE(help_table[i].keyword);
	      if (help_table[i].entry && !help_table[i].duplicate)
		FREE(help_table[i].entry);
	    }
	  FREE(help_table);
	}
      top_of_helpt = 0;
      index_boot(DB_BOOT_HLP);
    }
  else
    {
      send_to_char("Unknown reload option.\r\n", ch);
      return;
    }

  send_to_char(OK, ch);
}

static void init_review_strings(void)
{
  int count;
  for (count = 0; count < 25; count++) {
    strcpy(review_goss[count].string, "\0");
    strcpy(review_goss[count].name,   "\0");
    review_goss[count].invis = 0;

    strcpy(review_auct[count].string, "\0");
    strcpy(review_auct[count].name,   "\0");
    review_auct[count].invis = 0;

    strcpy(review_wizn[count].string, "\0");
    strcpy(review_wizn[count].name,   "\0");
    review_wizn[count].invis = 0;
  }
}

void
boot_world(void)
{
  log("Loading zone table.");
  index_boot(DB_BOOT_ZON);

  log("Loading rooms.");
  index_boot(DB_BOOT_WLD);

  log("Renumbering rooms.");
  renum_world();

  log("Checking start rooms.");
  check_start_rooms();

  log("Loading mobs and generating index.");
  index_boot(DB_BOOT_MOB);

  log("Loading objs and generating index.");
  index_boot(DB_BOOT_OBJ);

  log("Loading treasure items and generating index.");
  load_treasure();

  log("Renumbering zone table.");
  renum_zone_table();

  log("Initializing the review strings.");
  init_review_strings();

  if (!no_specials) {
    log("Loading shops.");
    index_boot(DB_BOOT_SHP);
  }
}

/* body of the booting system */
void
boot_db(void)
{
  int i;

  log("Boot db -- BEGIN.");

  log("Booting SQL database");
  boot_mysql();

  log("Resetting the game time:");
  reset_time();
  check_dst(1);
  load_compile_time();

  log("Reading news, credits, help, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(MOTD_FILE_MXP, &motd_mxp);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(CREATION_FILE, &creation);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);
  file_to_string_alloc(FUTURE_FILE, &future);
  file_to_string_alloc(MXP_FILE, &mxp);

  boot_world();

  log("Loading help entries.");
  index_boot(DB_BOOT_HLP);

  log("Generating player index.");
  sql_build_player_index();
  sql_remove_deleted_players();

  log("Loading fight messages.");
  load_messages();

  log("Loading social messages.");
  boot_social_messages();

  log("Assigning function pointers:");

  if (!no_specials)
    {
      log("   Mobiles.");
      assign_mobiles();
      log("   Shopkeepers.");
      assign_the_shopkeepers();
      log("   Objects.");
      assign_objects();
      log("   Rooms.");
      assign_rooms();
    }
  log("   Spells.");
  mag_assign_spells();

  log("Sorting command list and spells.");
  sort_commands();
  sort_spells();

  log("Booting mail system.");
  sql_count_mail();

  log("Reading banned site and invalid-name list.");
  sql_load_banned();
  Read_Invalid_List();

  log("Booting DNS cache.");
  boot_dns();
 
  if (!no_rent_check)
    {
      log("Deleting timed-out crash and rent files:");
      update_obj_file();
      log("Done.");
    }

/*  log("Loading persistant world.");
  for (i = 0; i <= top_of_zone_table; i++) {
    SET_BIT(ZONE_FLAGS(i), ZONE_UNLOAD);
    sql_load_mobs(i);  / Load everything first, prevents duplication /
    sql_load_objs(i);
    reset_zone(i);
  }   -- going to try and NOT load eveything before start, not needed?  mcooper 20050514 */
  
  log("Marking zones for unloading.");
  for (i = 0; i <= top_of_zone_table; i++)
    SET_BIT(ZONE_FLAGS(i), ZONE_UNLOAD);

  reset_q.head = reset_q.tail = NULL;

  log("Booting Scripts.");
  boot_lua();

  log("Integrating player shops.");
  sql_integrate_shops();

  log("Loading shop control variables.");
  sql_shop_boot();

  boot_time = time(0);

  log("Boot db -- DONE.");
  stonekeep_init();
}

/* reset the time in the game from file */
void
reset_time(void)
{
  long beginning_of_time = 650336715;
  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);
  sql_read_mud_date();

  if (time_info.hours <= 4)
    weather_info.sunlight = SUN_DARK;
  else if (time_info.hours == 5)
    weather_info.sunlight = SUN_RISE;
  else if (time_info.hours <= 20)
    weather_info.sunlight = SUN_LIGHT;
  else if (time_info.hours == 21)
    weather_info.sunlight = SUN_SET;
  else
    weather_info.sunlight = SUN_DARK;

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
	  time_info.day, time_info.month, time_info.year);
  log(buf);

  time_info.moon = MOON_THREE_EMPTY;
  if (time_info.day < 33) time_info.moon = MOON_HALF_EMPTY;  
  if (time_info.day < 29) time_info.moon = MOON_QUARTER_EMPTY;
  if (time_info.day < 25)  time_info.moon = MOON_FULL;
  if (time_info.day < 21) time_info.moon = MOON_THREE_FULL;
  if (time_info.day < 16) time_info.moon = MOON_HALF_FULL;
  if (time_info.day < 11) time_info.moon = MOON_QUARTER_FULL;
  if (time_info.day < 5) time_info.moon = MOON_NEW;

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12))
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.pressure <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.pressure <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;
}



/*
 * Thanks to Andrey (andrey@alex-ua.com) for this bit of code, although I
 * did add the 'goto' and changed some "while()" into "do { } while()".
 *	-gg 6/24/98 (technically 6/25/98, but I care not.)
 */
int count_alias_records(FILE *fl)
{
  char key[READ_SIZE], next_key[READ_SIZE];
  char line[READ_SIZE], *scan;
  int total_keywords = 0;

  /* get the first keyword line */
  get_one_line(fl, key);

  while (*key != '$') {
    /* skip the text */
    do {
      get_one_line(fl, line);
      if (feof(fl))
	      goto ackeof;
    } while (*line != '#');

    /* now count keywords */
    scan = key;
    do {
      scan = one_word(scan, next_key);
      if (*next_key)
        ++total_keywords;
    } while (*next_key);

    /* get next keyword line (or $) */
    get_one_line(fl, key);

    if (feof(fl))
      goto ackeof;
  }

  return (total_keywords);

  /* No, they are not evil. -gg 6/24/98 */
ackeof:	
  log("SYSERR: Unexpected end of help file.");
  exit(1);	/* Some day we hope to handle these things better... */
}

/* function to count how many hash-mark delimited records exist in a file */
int
count_hash_records(FILE * fl)
{
  char buf[128];
  int count = 0;

  while (fgets(buf, 128, fl))
    if (*buf == '#')
      count++;

  return count;
}



void
index_boot(int mode)
{
  char *index_filename, *prefix = NULL;
  FILE *index, *db_file;
  int rec_count = 0;

  switch (mode)
    {
    case DB_BOOT_WLD:
      prefix = WLD_PREFIX;
      break;
    case DB_BOOT_MOB:
      prefix = MOB_PREFIX;
      break;
    case DB_BOOT_OBJ:
      prefix = OBJ_PREFIX;
      break;
    case DB_BOOT_ZON:
      prefix = ZON_PREFIX;
      break;
    case DB_BOOT_SHP:
      prefix = SHP_PREFIX;
      break;
    case DB_BOOT_HLP:
      prefix = HLP_PREFIX;
      break;
    default:
      log("SYSERR: Unknown subcommand to index_boot!");
      exit(1);
      break;
    }

  if (mini_mud)
    index_filename = MINDEX_FILE;
  else
    index_filename = INDEX_FILE;

  sprintf(buf2, "%s/%s", prefix, index_filename);

  if (!(index = fopen(buf2, "r")))
    {
      log("SYSERR: opening index file '%s': %s", buf2, strerror(errno));
      exit(1);
    }

  /* first, count the number of records in the file so we can CREATE */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      log("SYSERR: File '%s' listed in '%s/%s': %s",
        buf2, prefix, index_filename, strerror(errno));
      fscanf(index, "%s\n", buf1);
      continue;
    } else {
      if (mode == DB_BOOT_ZON)
	rec_count++;
      else if (mode == DB_BOOT_HLP)
	rec_count += count_alias_records(db_file);
      else
	rec_count += count_hash_records(db_file);
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* Exit if 0 records, unless this is shops */
  if (!rec_count)
    {
      if (mode == DB_BOOT_SHP)
	return;
      log("SYSERR: boot error - 0 records counted in %s/%s.", prefix, index_filename);
      exit(1);
    }

  rec_count++;

  switch (mode)
    {
    case DB_BOOT_WLD:
      CREATE(world, struct room_data, rec_count);
      break;
    case DB_BOOT_MOB:
      CREATE(mob_proto, struct char_data, rec_count);
      CREATE(mob_index, struct index_data, rec_count);
      break;
    case DB_BOOT_OBJ:
      CREATE(obj_proto, struct obj_data, rec_count);
      CREATE(obj_index, struct index_data, rec_count);
      break;
    case DB_BOOT_ZON:
      CREATE(zone_table, struct zone_data, rec_count);
      break;
    case DB_BOOT_HLP:
      CREATE(help_table, struct help_index_element, rec_count * 2);
      break;
    }

  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$')
    {
      sprintf(buf2, "%s/%s", prefix, buf1);
      if (!(db_file = fopen(buf2, "r")))
	{
          log("SYSERR: %s: %s", buf2, strerror(errno));
	  exit(1);
	}
      switch (mode)
	{
	case DB_BOOT_WLD:
	case DB_BOOT_OBJ:
	case DB_BOOT_MOB:
	  discrete_load(db_file, mode, buf2);
	  break;
	case DB_BOOT_ZON:
	  load_zones(db_file, buf2);
	  break;
	case DB_BOOT_HLP:
	  load_help(db_file);
	  break;
	case DB_BOOT_SHP:
	  boot_the_shops(db_file, buf2, rec_count);
	  break;
	}

      fclose(db_file);
      fscanf(index, "%s\n", buf1);
    }

  /* sort the help index */
  if (mode == DB_BOOT_HLP)
    {
      qsort(help_table, top_of_helpt, sizeof(struct help_index_element), hsort);
      top_of_helpt--;
    }

  fclose(index);
}


void discrete_load(FILE * fl, int mode, char *filename)
{
  int nr = -1, last = 0;
  char line[256];

  char *modes[] = {"world", "mob", "obj"};

  for (;;)
    {
      /*
       * we have to do special processing with the obj files because they have
       * no end-of-record marker :(
       */
      if (mode != DB_BOOT_OBJ || nr < 0)
        if (!get_line(fl, line)) {
	  if (nr == -1) {
	    log("SYSERR: %s file %s is empty!", modes[mode], filename);
	  } else {
	    log("SYSERR: Format error in %s after %s #%d\n"
	        "...expecting a new %s, but file ended!\n"
	        "(maybe the file is not terminated with '$'?)", filename,
	        modes[mode], nr, modes[mode]);
          }
	  exit(1);
        }

      if (*line == '$')
	return;

      if (*line == '#')
	{
	  last = nr;
	  if (sscanf(line, "#%d", &nr) != 1)
	    {
	      log("SYSERR: Format error after %s #%d", modes[mode], last);
	      exit(1);
	    }
	  if (nr >= 99999)
	    return;
	  else
	    switch (mode)
	      {
	      case DB_BOOT_WLD:
		parse_room(fl, nr);
		break;
	      case DB_BOOT_MOB:
		parse_mobile(fl, nr);
		break;
	      case DB_BOOT_OBJ:
		strcpy(line, parse_object(fl, nr));
		break;
	      }
	}
      else
	{
          log("SYSERR: Format error in %s file %s near %s #%d", modes[mode],
	      filename, modes[mode], nr);
          log("SYSERR: ... offending line: '%s'", line);
	  exit(1);
	}
    }
}


bitvector_t asciiflag_conv(char *flag)
{
  bitvector_t flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++)
    {
      if (islower(*p))
	flags |= 1LL << (*p - 'a');
      else if (isupper(*p))
	flags |= 1LL << (26 + (*p - 'A'));

      if (!isdigit(*p))
	is_number = 0;
    }

  if (is_number)
    flags = atol(flag);

  return flags;
}


/* load the rooms */
void
parse_room(FILE * fl, long virtual_nr)
{
  static int zone = 0;
  static long room_nr = 0;
  int t[10], i;
  char line[256], flags[128];
  struct extra_descr_data *new_descr;

  sprintf(buf2, "room #%ld", virtual_nr);

  if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1))
    {
      log("Room #%ld is below zone %d.\n", virtual_nr, zone);
      exit(1);
    }
  while (virtual_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table)
      {
	log("Room %ld is outside of any zone.\n", virtual_nr);
	exit(1);
      }
  world[room_nr].zone = zone;
  world[room_nr].number = virtual_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    log("SYSERR: Expecting roomflags/sector type of room #%ld but file ended!",
	virtual_nr);
    exit(1);
  }

  if (sscanf(line, " %d %s %d ", t, flags, t + 2) != 3) {
    log("SYSERR: Format error in roomflags/sector type of room #%ld",
	virtual_nr);
    exit(1);
  }

  /* t[0] is the zone number; ignored with the zone-file system */
  world[room_nr].room_flags = asciiflag_conv(flags);
  REMOVE_BIT(world[room_nr].room_flags, ROOM_SECRET_MARK);
  world[room_nr].sector_type = t[2];

  world[room_nr].func = NULL;
  world[room_nr].contents = NULL;
  world[room_nr].people = NULL;
  world[room_nr].light = 0;	/* Zero light sources */
  CREATE(world[room_nr].script, struct script_data, 1);
  world[room_nr].script->name = NULL;

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf, "Format error in room #%ld (expecting D/E/R/S)", virtual_nr);

  for (;;)
    {
      if (!get_line(fl, line))
	{
	  log(buf);
	  exit(1);
	}
      switch (*line)
	{
	case 'D':
	  setup_dir(fl, room_nr, atoi(line + 1));
	  break;
	case 'E':
	  CREATE(new_descr, struct extra_descr_data, 1);
	  new_descr->keyword = fread_string(fl, buf2);
          new_descr->link = atoi(fread_string(fl, buf2));
	  new_descr->description = fread_string(fl, buf2);
	  new_descr->next = world[room_nr].ex_description;
	  world[room_nr].ex_description = new_descr;
	  break;
        case 'R':
          sscanf((line + 1), " %s %d", flags, &i);
          GET_ROOM_SCRIPT(room_nr)->name = str_dup(flags);
          GET_ROOM_SCRIPT(room_nr)->lua_functions = i;
          break;
	case 'S':			/* end of room */
	  top_of_world = room_nr++;
	  return;
	  break;
	default:
	  log(buf);
	  exit(1);
	}
    }
}



/* read direction data */
void
setup_dir(FILE * fl, long room, int dir)
{
  int t[5];
  char line[256];

  sprintf(buf2, "room #%ld, direction D%d", world[room].number, dir);

  CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
  world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
  world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    log("SYSERR: Format error, %s", buf2);
    exit(1);
  }
  if (sscanf(line, " %d %d %d ", t, t + 1, t + 2) != 3) {
    log("SYSERR: Format error, %s", buf2);
    exit(1);
  }

  if (t[0] == 1)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
  else if (t[0] == 2)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else
    world[room].dir_option[dir]->exit_info = 0;

  world[room].dir_option[dir]->key = t[1];
  world[room].dir_option[dir]->to_room = t[2];
}


/* make sure the start rooms exist & resolve their vnums to rnums */
void
check_start_rooms(void)
{
  extern sh_int mortal_start_room[NUM_HOMETOWNS];
  extern sh_int immort_start_room;
  extern sh_int frozen_start_room;
  int i;

  for (i = 0; i < NUM_HOMETOWNS; i++)
    if ((r_mortal_start_room[i] = real_room(mortal_start_room[i])) < 0)
    {
      log("SYSERR: %s start room does not exist.  Change in config.c.", hometowns[i]);
      exit(1);
    }

  if ((r_immort_start_room = real_room(immort_start_room)) < 0)
    {
      if (!mini_mud)
	log("SYSERR:  Warning: Immort start room does not exist.  "
	    "Change in config.c.");
      r_immort_start_room = r_mortal_start_room[0];
    }
  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0)
    {
      if (!mini_mud)
	log("SYSERR:  Warning: Frozen start room does not exist.  "
	    "Change in config.c.");
      r_frozen_start_room = r_mortal_start_room[0];
    }
}


/* resolve all vnums into rnums in the world */
void
renum_world(void)
{
  register int door;
  register long room;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
	if (world[room].dir_option[door]->to_room != NOWHERE)
	  world[room].dir_option[door]->to_room =
	    real_room(world[room].dir_option[door]->to_room);
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void
renum_zone_table(void)
{
  int cmd_no, a, b, olda, oldb, oldc, zone;
  char buf[128];

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      a = b = 0;
      olda = ZCMD.arg1;
      oldb = ZCMD.arg2;
      oldc = ZCMD.arg3;

      switch (ZCMD.command) {
      case 'M':
	a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
	b = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'O':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (ZCMD.arg3 != NOWHERE)
	  b = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'E':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'P':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	b = ZCMD.arg3 = real_object(ZCMD.arg3);
	break;
      case 'L':
      case 'D':
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	break;
      case 'R': /* rem obj/mob from room */
        a = ZCMD.arg1 = real_room(ZCMD.arg1);
        if (ZCMD.arg2)
	  b = ZCMD.arg3 = real_object(ZCMD.arg3);
        else
          b = ZCMD.arg3 = real_mobile(ZCMD.arg3);
        break;
      }
      if (a < 0 || b < 0) {
        if (!mini_mud) {
	  sprintf(buf,  "Invalid vnum %d, cmd disabled", (a < 0) ? olda : ((b < 0) ? oldb : oldc));
          log_zone_error(zone, cmd_no, buf);
	}
        ZCMD.command = '*';
      }
    }
}


void
parse_simple_mob(FILE *mob_f, int i, int nr)
{
  int j, t[10], statmod;
  char line[256];

  mob_proto[i].real_abils.str = 5;
  mob_proto[i].real_abils.intel = 5;
  mob_proto[i].real_abils.wis = 5;
  mob_proto[i].real_abils.dex = 5;
  mob_proto[i].real_abils.con = 5;
  mob_proto[i].real_abils.cha = 5;

  if (!get_line(mob_f, line)) {
    log("SYSERR: Format error in mob #%d, file ended after S flag!", nr);
    exit(1);
  }

  if (sscanf(line, " %d %d %dd%d+%d %dd%d ",
	     t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6) != 7)
  {
    log("SYSERR: Format error in mob #%d, first line after S flag\n"
        "...expecting line of form '# # #d#+# #d#+#'", nr);
    exit(1);
  }

  GET_LEVEL(mob_proto + i) = t[0];
  /*
   * Serapis 132212ZMAY97 
   * Set mob stats based on level.
   */

  statmod = pow(GET_LEVEL(mob_proto + i), .70);
  mob_proto[i].real_abils.str += statmod;
  mob_proto[i].real_abils.intel += statmod;
  mob_proto[i].real_abils.wis += statmod;
  mob_proto[i].real_abils.dex += statmod;
  mob_proto[i].real_abils.con += statmod;
  mob_proto[i].real_abils.cha += statmod;

  mob_proto[i].points.hitroll = GET_LEVEL(mob_proto + i) / 3;
  mob_proto[i].points.dodgeroll = GET_LEVEL(mob_proto + i) / 10;
  mob_proto[i].points.damroll = (GET_LEVEL(mob_proto + i) > 10 ? ((GET_LEVEL(mob_proto + i) + 1) / 1.50 ):
                                ((GET_LEVEL(mob_proto + i) + 1) / 2));

  mob_proto[i].points.armor = t[1];

  /*  FIX come back and put in realistic mana/move stuff later -rparet */
  mob_proto[i].points.max_hit = 0;
  mob_proto[i].points.hit = 0;
  mob_proto[i].points.mana = 0;
  mob_proto[i].points.move = 0;
  mob_proto[i].points.max_mana = 50;
  mob_proto[i].points.max_move = 50;

  mob_proto[i].mob_specials.hpnumdice  = t[2];
  mob_proto[i].mob_specials.hpsizedice = t[3];
  mob_proto[i].mob_specials.hpbonus    = t[4];
  

  mob_proto[i].mob_specials.damnodice = t[5];
  mob_proto[i].mob_specials.damsizedice = t[6];

  if (!get_line(mob_f, line)) {
      log("SYSERR: Format error in mob #%d, second line after S flag\n"
	  "...expecting line of form '# #', but file ended!", nr);
      exit(1);
    }

  if (sscanf(line, " %d %d ", t, t + 1) != 2) {
    log("SYSERR: Format error in mob #%d, second line after S flag\n"
	"...expecting line of form '# #'", nr);
    exit(1);
  }

  GET_GOLD(mob_proto + i) = t[0];
  GET_EXP(mob_proto + i) = t[1];

  if (!get_line(mob_f, line)) {
    log("SYSERR: Format error in last line of mob #%d\n"
	"...expecting line of form '# # #', but file ended!", nr);
    exit(1);
  }

  if (sscanf(line, " %d %d %d %d ", t, t + 1, t + 2, t + 3) != 3) {
    log("SYSERR: Format error in last line of mob #%d\n"
	"...expecting line of form '# # #'", nr);
    exit(1);
  }

  mob_proto[i].char_specials.position = t[0];
  mob_proto[i].mob_specials.default_pos = t[1];
  mob_proto[i].player.sex = t[2];
  mob_proto[i].player.hometown = 0;

  mob_proto[i].player.weight = 200;

  for (j = 0; j < MAX_COND; j++)
    GET_COND(mob_proto + i, j) = -1;
  
  /*
   * these are now save applies; base save numbers for MOBs are now from
   * the warrior save table.
   */
  for (j = 0; j < MAX_SAVING; j++)
    GET_SAVE(mob_proto + i, j) = 0;

  mob_proto[i].mob_specials.race = RACE_HUMANOID;
  mob_proto[i].mob_specials.noise = NULL;
  mob_proto[i].mob_specials.sound = NULL;
  
  IS_PARRIED(mob_proto + i) = FALSE;
  COMBAT_STATE(mob_proto + i) = 0;
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void
interpret_espec(char *keyword, char *value, int i, int nr)
{
  int num_arg, matched = 0;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  two_arguments(value, arg1, arg2);
  num_arg = atoi(value);

  CASE("BareHandAttack")
    {
      RANGE(0, 99);
      mob_proto[i].mob_specials.attack_type = num_arg;
    }

  CASE("Str")
    {
      RANGE(3, 25);
      mob_proto[i].real_abils.str = num_arg;
    }

  CASE("Int")
    {
      RANGE(3, 25);
      mob_proto[i].real_abils.intel = num_arg;
    }

  CASE("Wis")
    {
      RANGE(3, 25);
      mob_proto[i].real_abils.wis = num_arg;
    }

  CASE("Dex")
    {
      RANGE(3, 25);
      mob_proto[i].real_abils.dex = num_arg;
    }

  CASE("Con")
    {
      RANGE(3, 25);
      mob_proto[i].real_abils.con = num_arg;
    }

  CASE("Cha")
    {
      RANGE(3, 25);
      mob_proto[i].real_abils.cha = num_arg;
    }

  CASE("Race")
    {
      RANGE(0, 99);
      mob_proto[i].mob_specials.race = num_arg;
    }
  
  CASE("Noise")
   {
      if (value)
        mob_proto[i].mob_specials.noise = str_dup(value);
   }
  
  CASE("Sound") {
    if (value)
      mob_proto[i].mob_specials.sound = str_dup(value);
  }

  CASE("Script")
   {
     if (arg1)
       mob_index[i].script->name = str_dup(arg1);
     
     if (arg2)
       mob_index[i].script->lua_functions = asciiflag_conv(arg2);    
   }

  if (!matched)
    {
      log("SYSERR: Warning: unrecognized espec keyword %s in mob #%d",
	   keyword, nr);
    }    
}

#undef CASE
#undef RANGE

void
parse_espec(char *buf, int i, int nr)
{
  char *ptr;

  if ((ptr = strchr(buf, ':')) != NULL)
    {
      *(ptr++) = '\0';
      while (isspace(*ptr))
	ptr++;
    }
  else
    ptr = "";

  interpret_espec(buf, ptr, i, nr);
}


void
parse_enhanced_mob(FILE *mob_f, int i, int nr)
{
  char line[256];

  parse_simple_mob(mob_f, i, nr);

  while (get_line(mob_f, line))
    {
      if (!strcmp(line, "E"))	/* end of the ehanced section */
	return;
      else if (*line == '#')
	{	/* we've hit the next mob, maybe? */
          log("SYSERR: Unterminated E section in mob #%d.", nr);
	  exit(1);
	}
      else
	parse_espec(line, i, nr);
    }

  log("SYSERR: Unexpected end of file reached after mob #%d.", nr);
  exit(1);
}


void
parse_mobile(FILE * mob_f, int nr)
{
  static int i = 0;
  int j, t[10];
  char line[256], *tmpptr, letter;
  char f1[128], f2[128];

  mob_index[i].virtual = nr;
  mob_index[i].func = NULL;
    
  CREATE(mob_index[i].script, struct script_data, 1);
  mob_index[i].script->name = NULL;

  mob_proto[i].player_specials = &dummy_mob;

  sprintf(buf2, "mob vnum %d", nr);

  /***** String data *** */
  mob_proto[i].player.name = fread_string(mob_f, buf2);
  if (!mob_proto[i].player.name)
  {
    log("SYSERR: Error at mob %d.", nr);
    mob_proto[i].player.name = str_dup("bug");
  }
  tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);
  mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
  mob_proto[i].player.description = fread_string(mob_f, buf2);
  mob_proto[i].player.title = NULL;

  /* *** Numeric data *** */
  if (!get_line(mob_f, line)) {
    log("SYSERR: Format error after string section of mob #%d\n"
	"...expecting line of form '# # # {S | E}', but file ended!", nr);
    exit(1);
  }

  if (sscanf(line, "%s %s %d %c", f1, f2, t + 2, &letter) != 4) {
    log("SYSERR: Format error after string section of mob #%d\n"
	"...expecting line of form '# # # {S | E}'", nr);
    exit(1);
  }

  MOB_FLAGS(mob_proto + i) = asciiflag_conv(f1);
  SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
  if (MOB_FLAGGED(mob_proto + i, MOB_EXTRACT)) {
    /* Rather bad to load mobiles with this bit already set. */
    log("SYSERR: Mob #%d has reserved bit EXTRACT set.", nr);
    REMOVE_BIT(MOB_FLAGS(mob_proto + i), MOB_EXTRACT);
  }

  AFF_FLAGS(mob_proto + i) = asciiflag_conv(f2);
  GET_ALIGNMENT(mob_proto + i) = t[2];

  /* AGGR_TO_ALIGN is ignored if the mob is AGGRESSIVE. */
  if (MOB_FLAGGED(mob_proto + i, MOB_AGGRESSIVE) && 
    MOB_FLAGGED(mob_proto + i, MOB_AGGR_GOOD | MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL))
    log("SYSERR: Mob #%d both AGGR and AGGR to an alignment.", nr);

  switch (letter)
    {
    case 'S':	/* Simple monsters */
      parse_simple_mob(mob_f, i, nr);
      break;
    case 'E':	/* Circle3 Enhanced monsters */
      parse_enhanced_mob(mob_f, i, nr);
      break;
      /* add new mob types here.. */
    default:
      log("SYSERR: Unsupported mob type '%c' in mob #%d.", letter, nr);
      exit(1);
    }

  /* Let's give the mob proto's access to all skills and spells */
  for (j = 0; j <= MAX_SKILL_GROUPS; j++) {
    mob_proto[i].skills.group_level[j] = 4;
    mob_proto[i].skills.group_skill[j] = 100;
  }

  mob_proto[i].aff_abils = mob_proto[i].real_abils;

  for (j = 0; j < NUM_WEARS; j++)
    mob_proto[i].equipment[j] = NULL;

  mob_proto[i].nr = i;
  mob_proto[i].desc = NULL;

  top_of_mobt = i++;
}




/* read all objects from obj file; generate index and prototypes */
char *
parse_object(FILE * obj_f, int nr)
{
  static int i = 0, retval;
  static char line[256];
  int t[10], j = 0, k = 0;
  char *tmpptr;
  char f1[256], f2[256], f3[256];
  struct extra_descr_data *new_descr;

  obj_index[i].virtual = nr;
  obj_index[i].func = NULL;

  CREATE(obj_index[i].script, struct script_data, 1);
  obj_index[i].script->name = NULL;
  obj_proto[i].in_room = NOWHERE;
  obj_proto[i].item_number = i;

  sprintf(buf2, "object #%d", nr);

  /* *** string data *** */
  if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL) {
    log("SYSERR: Null obj name or format error at or near %s", buf2);
    exit(1);
  }

  tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
  if (*tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);

  tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    *tmpptr = UPPER(*tmpptr);
  obj_proto[i].action_description = fread_string(obj_f, buf2);

  /* *** numeric data *** */
  if (!get_line(obj_f, line)) {
    log("SYSERR: Expecting first numeric line of %s, but file ended!", buf2);
    exit(1);
  }
  if ((retval = sscanf(line, " %d %s %s %s", t, f1, f2, f3)) != 4) {
    if (retval == 3)
      t[3] = 0;
    else {
      log("SYSERR: Format error in first numeric line (expecting 4 args, got %d), %s", retval, buf2);
      exit(1);
    }
  }
  obj_proto[i].obj_flags.type_flag = t[0];
  obj_proto[i].obj_flags.extra_flags = asciiflag_conv(f1);
  obj_proto[i].obj_flags.wear_flags = asciiflag_conv(f2);
  obj_proto[i].obj_flags.bitvector = asciiflag_conv(f3);
  
  if (!get_line(obj_f, line)) {
    log("SYSERR: Expecting second numeric line of %s, but file ended!", buf2);
    exit(1);
  }
  if ((retval = sscanf(line, "%d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4)) != 5) {
    log("SYSERR: Format error in second numeric line (expecting 5 args, got %d), %s", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.value[0] = t[0];
  obj_proto[i].obj_flags.value[1] = t[1];
  obj_proto[i].obj_flags.value[2] = t[2];
  obj_proto[i].obj_flags.value[3] = t[3];
  obj_proto[i].obj_flags.value[4] = t[4]; 

  if (!get_line(obj_f, line)) {
    log("SYSERR: Expecting third numeric line of %s, but file ended!", buf2);
    exit(1);
  }
  if ((retval = sscanf(line, "%d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5)) != 6) {
    log("SYSERR: Format error in third numeric line (expecting 6 args, got %d), %s", retval, buf2);
    exit(1);
  }

  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.speed = t[1];
  obj_proto[i].obj_flags.cost = t[2];
  obj_proto[i].obj_flags.percentage_load = t[3];
  obj_proto[i].obj_flags.timer = t[4];
  obj_proto[i].obj_flags.level = t[5];

  /* check to make sure that weight of containers exceeds curr. quantity */
/*  if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON ||
      obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN)
    {
      if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
	obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
    } -- depricated 20020129: mcooper */

  /* ***  begin extra descriptions and affect fields *** */

  /* initalise affect and requirements fields */

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    {
      obj_proto[i].affected[j].location = APPLY_NONE;
      obj_proto[i].affected[j].modifier = 0;
    }

  for (j = 0; j < MAX_OBJ_REQS; j++)
    {
      obj_proto[i].affected[j].location = REQ_NONE;
      obj_proto[i].affected[j].modifier = 0;
    }

  strcat(buf2, ", after numeric constants (expecting E/A/#xxx)");
  j = k = 0;

  for (;;)
  {
    if (!get_line(obj_f, line)) {
      log("SYSERR: Format error in %s", buf2);
      exit(1);
    }

      switch (*line)
	{
        case 'S':
          sscanf((line + 1), " %s %d", f1, &retval);
          obj_index[i].script->name = str_dup(f1);
          obj_index[i].script->lua_functions = retval;
          break;
	case 'E':
	  CREATE(new_descr, struct extra_descr_data, 1);
	  new_descr->keyword = fread_string(obj_f, buf2);
	  new_descr->description = fread_string(obj_f, buf2);
	  new_descr->next = obj_proto[i].ex_description;
	  obj_proto[i].ex_description = new_descr;
	  break;
	case 'A':
          if (j >= MAX_OBJ_AFFECT) {
	    log("SYSERR: Too many A fields (%d max), %s", MAX_OBJ_AFFECT, buf2);
	    exit(1);
          }
          if (!get_line(obj_f, line)) {
	    log("SYSERR: Format error in 'A' field, %s\n"
            "...expecting 2 numeric constants but file ended!", buf2);
            exit(1);
          }

          if ((retval = sscanf(line, " %d %d ", t, t + 1)) != 2) {
            log("SYSERR: Format error in 'A' field, %s\n"
	        "...expecting 2 numeric arguments, got %d\n"
	        "...offending line: '%s'", buf2, retval, line);
	    exit(1);
          }
	  obj_proto[i].affected[j].location = t[0];
	  obj_proto[i].affected[j].modifier = t[1];
	  j++;
	  break;
        case 'R':
          if (k >= MAX_OBJ_REQS) {
            log("Too many R fields (%d max), %s\n", MAX_OBJ_REQS, buf2);
            exit(1);
          }
          
          get_line(obj_f, line);
          sscanf(line, " %d %d ", t, t + 1);
          obj_proto[i].reqs[k].location = t[0];
          obj_proto[i].reqs[k].modifier = t[1];
          k++;
          break;
	case '$':
	case '#':
          check_object(&obj_proto[i]);
	  top_of_objt = i++;
	  return (line);
	default:
          log("SYSERR: Format error in (%c): %s", *line, buf2);
	  exit(1);
	}
    }
}


#define Z	zone_table[zone]

/* load the zone table and command tables */
void
load_zones(FILE * fl, char *zonename)
{
  static int zone = 0;
  int cmd_no, num_of_cmds = 0, line_num = 0, tmp, error;
  char *ptr, buf[256], zname[256], flags[128];

  strcpy(zname, zonename);

  /* Skip first 3 lines lest we mistake the zone name for a command. */
  for (tmp = 0; tmp < 3; tmp++)
    get_line(fl, buf);

  while (get_line(fl, buf))
    if ((strchr("MOETPDRL", buf[0]) && buf[1] == ' ') || (buf[0] == 'S' && buf[1] == '\0'))
      num_of_cmds++;

  rewind(fl);

  if (num_of_cmds == 0) {
    log("SYSERR: %s is empty!", zname);
    exit(1);
  } else
    CREATE(Z.cmd, struct reset_com, num_of_cmds);

  line_num += get_line(fl, buf);

  if (sscanf(buf, "#%d", &Z.number) != 1) {
    log("SYSERR: Format error in %s, line %d", zname, line_num);
    exit(1);
  }
  sprintf(buf2, "beginning of zone #%d", Z.number);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL)	/* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL)	/* take off the '~' if it's there */
    *ptr = '\0';
  Z.author = str_dup(buf);

  line_num += get_line(fl, buf);
  if (sscanf(buf, " %ld %d %d ", &Z.top, &Z.lifespan, &Z.reset_mode) != 3) {
    log("SYSERR: Format error in 3-constant line of %s", zname);
    exit(1);
  }

  line_num += get_line(fl, buf);
  if (sscanf(buf, " %d %s ", &Z.clime, flags) != 2)
    {
      log("Format error in 2-constant line of %s", zname);
      exit(0);  
    }

  Z.zflags = asciiflag_conv(flags);  

  cmd_no = 0;

  for (;;)
  {
    if ((tmp = get_line(fl, buf)) == 0) {
      log("SYSERR: Format error in %s - premature end of file", zname);
      exit(1);
    }

      line_num += tmp;
      ptr = buf;
      skip_spaces(&ptr);

      if ((ZCMD.command = *ptr) == '*')
	continue;

      ptr++;

      if (ZCMD.command == 'S' || ZCMD.command == '$')
	{
	  ZCMD.command = 'S';
	  break;
	}
      error = 0;
/*      if (strchr("MOEPD", ZCMD.command) == NULL)
	{	/ a 3-arg command /
	  if (sscanf(ptr, " %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2) != 3)
	    error = 1;
	}
      else 		-- no more 3-arg commands, mcooper 20020531 */
	
      if (sscanf(ptr, " %d %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2, &ZCMD.arg3) != 4)
        error = 1;

      ZCMD.if_flag = tmp;

      if (error) {
        log("SYSERR: Format error in %s, line %d: '%s'", zname, line_num, buf);
        exit(1);
      }
      ZCMD.line = line_num;
      cmd_no++;
    }

  if (num_of_cmds != cmd_no + 1) {
    log("SYSERR: Zone command count mismatch for %s. Estimated: %d, Actual: %d", zname, num_of_cmds, cmd_no + 1);
    exit(1);
  }

  top_of_zone_table = zone++;
}

#undef Z


void get_one_line(FILE *fl, char *buf)
{
  if (fgets(buf, READ_SIZE, fl) == NULL) {
    log("error reading help file: not terminated with $?");
    exit(1);
  }

  buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}

void load_treasure(void)
{
  int i, j;
  FILE *tfile;

  sprintf(buf, "%s/treasure", OBJ_PREFIX);

  if (!(tfile = fopen(buf, "r"))) {
    log("SYSERR: Unable to open %s", buf);
    exit(1);
  }

  if (!get_line(tfile, buf)) {    /* Read the number of treasure item types */
    CREATE(treasure_list, struct tlist_data, 1);
    CREATE(treasure_list[0].vnum, long, 1);
    return;
  }

  if (sscanf(buf, "%d", &top_of_treasure) != 1) {
    log("SYSERR: Format error in treasure file, cannot read number of treasure items");
    exit(1);
  }

  CREATE(treasure_list, struct tlist_data, top_of_treasure);
  for (i = 0; i < top_of_treasure; i++) {
    if (!get_line(tfile, buf)) {
      log("SYSERR: Format error in treasure listing, expecting string type");
      exit(1);
    }
    treasure_list[i].type = strdup(buf);

    if (!get_line(tfile, buf)) {
      log("SYSERR: Format error in treasure listing, expecting number of items");
      exit(1);
    }
    treasure_list[i].num_of_items = atoi(buf);
  }


  for (i = 0; i < top_of_treasure; i++) {
    CREATE(treasure_list[i].vnum, long, treasure_list[i].num_of_items);
    for (j = 0; j < treasure_list[i].num_of_items; j++) {
      if (!get_line(tfile, buf)) {
        log("SYSERR: Format error in treasure listing, expecting item vnum");
        exit(1);
      }
      treasure_list[i].vnum[j] = atoi(buf);
    }
  }
}

void load_help(FILE *fl)
{
  char key[READ_SIZE+1], next_key[READ_SIZE+1], entry[32384];
  char line[READ_SIZE+1], *scan;
  struct help_index_element el;

  /* get the first keyword line */
  get_one_line(fl, key);
  while (*key != '$') {
    /* read in the corresponding help entry */
    strcpy(entry, strcat(key, "\r\n"));
    get_one_line(fl, line);
    while (*line != '#') {
      strcat(entry, strcat(line, "\r\n"));
      get_one_line(fl, line);
    }

    /* now, add the entry to the index with each keyword on the keyword line */
    el.duplicate = 0;
    el.entry = str_dup(entry);
    scan = one_word(key, next_key);
    while (*next_key) {
      el.keyword = str_dup(next_key);
      help_table[top_of_helpt++] = el;
      el.duplicate++;
      scan = one_word(scan, next_key);
    }

    /* get next keyword line (or $) */
    get_one_line(fl, key);
  }
}


int
hsort(const void *a, const void *b)
{
  struct help_index_element *a1, *b1;

  a1 = (struct help_index_element *) a;
  b1 = (struct help_index_element *) b;

  return (str_cmp(a1->keyword, b1->keyword));
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */



int
vnum_mobile(char *searchname, struct char_data * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_mobt; nr++)
    {
      if (isname_with_abbrevs(searchname, mob_proto[nr].player.name))
	{
	  sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
		  mob_index[nr].virtual,
		  mob_proto[nr].player.short_descr);
          send_to_char(buf, ch);
	}
    }
  return (found);
}


int
vnum_object(char *searchname, struct char_data * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_objt; nr++)
    {
      if (isname_with_abbrevs(searchname, obj_proto[nr].name))
	{
	  sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
		  obj_index[nr].virtual,
		  obj_proto[nr].short_description);
          send_to_char(buf, ch);
	}
    }
  return (found);
}


/* create a character, and add it to the char list */
struct char_data *
create_char(void)
{
  struct char_data *ch;

  CREATE(ch, struct char_data, 1);
  ch->next = character_list;
  character_list = ch;

  return ch;
}


/* create a new mobile from a prototype */
struct char_data *
read_mobile(int nr, int type, char *id)
{
  int i;
  struct char_data *mob;
  uuid_t *uuid;
  char uuid_string[37];
  extern void aff_apply_modify(struct char_data *ch, byte loc, sh_int mod, char *msg);

  if (type == VIRTUAL)
    {
      if ((i = real_mobile(nr)) < 0)
	{
	  sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
	  log(buf);
          return (0);
	}
    }
  else
    i = nr;

  CREATE(mob, struct char_data, 1);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

  mob->points.max_hit = dice(GET_NHPD(mob), GET_SHPD(mob)) + GET_HPBONUS(mob);

  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  if (!id) {
    CREATE(uuid, uuid_t, 1);
    uuid_generate(*uuid);
    uuid_unparse(*uuid, uuid_string);
    GET_UUID(mob) = str_dup(uuid_string);
    FREE(uuid);
  } else
    GET_UUID(mob) = str_dup(id);

  if (GET_GOLD(mob))
  {
   /* set the gold to be gold+-(1 to 20%) */
   if (!number(0,1))                 
    GET_GOLD(mob)+=(number(1,20)*GET_GOLD(mob))/100;
   else
    GET_GOLD(mob)-=(number(1,20)*GET_GOLD(mob))/100;
  }
  GET_GOLD(mob) = MAX(0, GET_GOLD(mob));

  /* Apply INVULN and METALSKIN flags since they aren't applied elsewhere */
  if (AFF_FLAGS(mob)) {
    if (AFF_FLAGGED(mob, AFF_INVULN)) {
      aff_apply_modify(mob, APPLY_AC, (GET_LEVEL(mob) / 5) + 20, "read_mobile");
      aff_apply_modify(mob, APPLY_SAVING_SPELL, -(GET_LEVEL(mob) / 10), "read_mobile");
    } else if (AFF_FLAGGED(mob, AFF_METALSKIN))
      aff_apply_modify(mob, APPLY_AC, GET_LEVEL(mob) / 10, "read_mobile");
  }

  return mob;
}

/* dnsmod */
void boot_dns(void)
{
  int i = 0;
  char line[256], name[256];
  FILE *fl;
  struct dns_entry *dns;

  memset((char *) dns_cache, 0, sizeof(struct dns_entry *) * DNS_HASH_NUM);

  if(!(fl = fopen(DNS_FILE, "r"))) {
    log("No DNS cache!");
    return;
  }

  do {
    i = 0;
    get_line(fl, line);
    if(*line != '~') {
      CREATE(dns, struct dns_entry, 1);
      dns->name = NULL;
      dns->next = NULL;
      sscanf(line, "%d.%d.%d.%d %s", dns->ip, dns->ip + 1,
      dns->ip + 2, dns->ip + 3, name);
      dns->name = str_dup(name);
      i = (dns->ip[0] + dns->ip[1] + dns->ip[2]) % DNS_HASH_NUM;
      dns->next = dns_cache[i];
      dns_cache[i] = dns;
    }
  } while (!feof(fl) && *line != '~');
  fclose(fl);
}


/* dnsmod */
void save_dns_cache(void)
{
  int i;
  FILE *fl;
  struct dns_entry *dns;

  if(!(fl = fopen(DNS_FILE, "w"))) {
    log("SYSERR: Can't open dns cache file for write!");
    return;
  }

  for(i = 0; i < DNS_HASH_NUM; i++) {
    if(dns_cache[i]) {
      for(dns = dns_cache[i]; dns; dns = dns->next)
      fprintf(fl, "%d.%d.%d.%d %s\n", dns->ip[0], dns->ip[1],
        dns->ip[2], dns->ip[3], dns->name);
    }
  }
  fprintf(fl, "~\n");
  fclose(fl);
}


/* dnsmod */
int get_host_from_cache(struct dns_entry *dnsd)
{
  int i;
  struct dns_entry *d;
  char buf[256];

  i = (dnsd->ip[0] + dnsd->ip[1] + dnsd->ip[2]) % DNS_HASH_NUM;
  if(dns_cache[i]) {
    for(d = dns_cache[i]; d; d = d->next) {
      if(dnsd->ip[0] == d->ip[0] && dnsd->ip[1] == d->ip[1] &&
      dnsd->ip[2] == d->ip[2]) {
      if(d->ip[3] == -1) {
        sprintf(buf, "%d.%s", dnsd->ip[3], d->name);
        dnsd->name = str_dup(buf);
        return TRUE;
      } else if(dnsd->ip[3] == d->ip[3]) {
        dnsd->name = str_dup(d->name);
        return TRUE;
      }
      }
    }
  }
  return FALSE;
}


/* dnsmod */
void add_dns_host(struct dns_entry *dnsd, char *hostname)
{
  int i;
  struct dns_entry *d;

  i = (dnsd->ip[0] + dnsd->ip[1] + dnsd->ip[2]) % DNS_HASH_NUM;
  CREATE(d, struct dns_entry, 1);
  d->ip[0] = dnsd->ip[0];
  d->ip[1] = dnsd->ip[1];
  d->ip[2] = dnsd->ip[2];
  d->ip[3] = dnsd->ip[3];
  d->name = str_dup(hostname);
  d->next = dns_cache[i];
  dns_cache[i] = d;
  save_dns_cache();
}



/* create an object, and add it to the object list */
struct obj_data *
create_obj(void)
{
  struct obj_data *obj;
  uuid_t *uuid;
  char uuid_string[37];

  CREATE(obj, struct obj_data, 1);

  CREATE(uuid, uuid_t, 1);
  uuid_generate(*uuid);
  uuid_unparse(*uuid, uuid_string);   
  obj->id = str_dup(uuid_string);
  FREE(uuid);

  obj->next = object_list;
  object_list = obj;
  return obj;
}


/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type, char *id)
{
  struct obj_data *obj;
  int i;
  uuid_t *uuid;
  char uuid_string[37];

  if (nr < 0) {
    log("SYSERR: trying to create obj with negative num!");
    return NULL;
  }

  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
  	  sprintf(buf, "Object (V) %d does not exist in database.", nr);
	    log(buf);
      return NULL;
    }
  } else
    i = nr;

  CREATE(obj, struct obj_data, 1);
  *obj = obj_proto[i];
  if (!id) {
    CREATE(uuid, uuid_t, 1);
    uuid_generate(*uuid); 
    uuid_unparse(*uuid, uuid_string);
    obj->id = str_dup(uuid_string);
    FREE(uuid);
  } else
    obj->id = str_dup(id);

  obj->next = object_list;
  object_list = obj;

  return obj;
}


#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void
zone_update(void)
{
  int i;
  struct reset_q_element *update_u, *temp;
  static int timer = 0;

  /* jelson 10/22/92 */
  if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60)
    {
      /* one minute has passed */
      /*
       * NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
       * factor of 60
       */

      timer = 0;

      /* since one minute has passed, increment zone ages */
      for (i = 0; i <= top_of_zone_table; i++) {
        if (zone_table[i].age < zone_table[i].lifespan && zone_table[i].reset_mode &&
            ZONE_FLAGGED(i, ZONE_UNLOAD))
         (zone_table[i].age)++;

	  if (zone_table[i].age >= zone_table[i].lifespan &&
	      zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)
	    {
	      /* enqueue zone */

	      CREATE(update_u, struct reset_q_element, 1);

	      update_u->zone_to_reset = i;
	      update_u->next = 0;

	      if (!reset_q.head)
		reset_q.head = reset_q.tail = update_u;
	      else
		{
		  reset_q.tail->next = update_u;
		  reset_q.tail = update_u;
		}

	      zone_table[i].age = ZO_DEAD;
	    }
	}
    }	/* end - one minute has passed */


  /* dequeue zones (if possible) and reset */
  /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
  for (update_u = reset_q.head; update_u; update_u = update_u->next) {
    if (is_empty(update_u->zone_to_reset))
      unload_room_descs(update_u->zone_to_reset);
    else if (zone_table[update_u->zone_to_reset].reset_mode == 2)
      reset_zone(update_u->zone_to_reset);
    else
      continue;

    /* dequeue */
    if (update_u == reset_q.head)
      reset_q.head = reset_q.head->next;
    else {
      for (temp = reset_q.head; temp->next != update_u; temp = temp->next);
      if (!update_u->next)
 	      reset_q.tail = temp;

      temp->next = update_u->next;
    }
    FREE(update_u);
    break;
  }
}

void
log_zone_error(int zone, int cmd_no, char *message)
{
  char buf[256];

  sprintf(buf, "SYSERR: error in zone file: %s", message);
  mudlog(buf, NRM, LVL_GOD, TRUE);

  sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	  ZCMD.command, zone_table[zone].number, ZCMD.line);
  mudlog(buf, NRM, LVL_GOD, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }

/* ************************************************************************** 
   NAME       : percent_load()
   PARAMETERS : TYPE                NAME                     DESCRIPTION
                ----                ----                     -----------
                struct object_dat * obj                  pointer to an object
   PURPOSE    : Simulate a % chance of an item loading
   RETURNS    : TRUE if it should load
   HISTORY    : Created by dlkarnes 960811
   ************************************************************************ */
static bool 
percent_load(struct obj_data *obj)
{ 
  int num;

  if (GET_OBJ_LOAD(obj) < 100)
  {
    num = number(0,100);
    if (num > GET_OBJ_LOAD(obj)) 
    return (FALSE); 
  }
  return (TRUE);
} 


/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int cmd_no, tmp_cmd, to_room, i, last_cmd = 0, loop = 0;
  long vnum;
  struct char_data *mob = NULL;
  struct obj_data *obj, *obj_to;
  void Mob_crashsave(struct char_data * mob);
  void sql_remove_mob(struct char_data *mob);

  for (cmd_no = 0, tmp_cmd = 0; ZCMD.command != 'S'; cmd_no++) {
    if (ZCMD.if_flag && !last_cmd)
      continue;

    switch (ZCMD.command) {
      case '*':      /* ignore command */
        last_cmd = 0;
        break;

      case 'L':         /* Start/End Looping */
        if (!ZCMD.arg2) {
          tmp_cmd = cmd_no;
          loop = ZCMD.arg3;
          last_cmd = 1;
        } else
          if (--loop > 0)
            cmd_no = tmp_cmd;
        break;

      case 'M':        /* Read a mobile */
        i = in_world(mob_index[ZCMD.arg1].virtual, SCMD_MOB);

        if (i < ZCMD.arg2) {
          if ((mob = read_mobile(ZCMD.arg1, REAL, NULL))) {
            last_cmd = 1;
            char_to_room(mob, ZCMD.arg3);
            GET_LOADED_IN(mob) = world[ZCMD.arg3].number;

            /* Zone 79 random load mobs */
            if ((mob_index[ZCMD.arg1].virtual < 7999) && (mob_index[ZCMD.arg1].virtual > 7899)) {
              do {
                to_room = number(0, top_of_world);
              } while ((IS_SET(world[to_room].room_flags,ROOM_PRIVATE))
                 ||(IS_SET(world[to_room].room_flags,ROOM_GODROOM))
                 ||(IS_SET(world[to_room].room_flags,ROOM_DEATH))
                 ||(IS_SET(world[to_room].room_flags,ROOM_NOMOB))
                 ||(IS_SET(world[to_room].room_flags,ROOM_HOUSE))
                 ||(IS_SET(world[to_room].room_flags,ROOM_ATRIUM))
                 ||(world[to_room].sector_type == SECT_CITY)
                 ||(world[to_room].sector_type == SECT_WATER_NOSWIM)
                 ||(world[to_room].sector_type == SECT_WATER_SWIM)
                 ||(world[to_room].sector_type == SECT_UNDERWATER)
                 || ZONE_FLAGGED(world[to_room].zone, ZONE_LAW));
                      
              char_from_room(mob);
              char_to_room(mob, to_room);
              GET_LOADED_IN(mob) = world[to_room].number;

              /* Now unload them to stop them wandering around initially */
              if (!ZONE_FLAGGED(world[real_room(GET_LOADED_IN(mob))].zone, ZONE_UNLOAD)) {
                save_mob(mob);
                Mob_crashsave(mob);
                SET_BIT(MOB_FLAGS(mob), MOB_UNLOAD);
                extract_char(mob);
              }
              break;   /* prevent infinite loop -rparet */
            }
          
            if (MOB_FLAGGED(mob, MOB_RANDZON)) {
              do {
                to_room = number(0, top_of_world);
              } while ((IS_SET(world[to_room].room_flags,ROOM_PRIVATE))
                 ||(IS_SET(world[to_room].room_flags,ROOM_GODROOM))
                 ||(IS_SET(world[to_room].room_flags,ROOM_DEATH))
                 ||(IS_SET(world[to_room].room_flags,ROOM_NOMOB))  
                 ||(IS_SET(world[to_room].room_flags,ROOM_HOUSE))
                 ||(IS_SET(world[to_room].room_flags,ROOM_ATRIUM))
                 ||((world[to_room].sector_type == SECT_WATER_NOSWIM) &&
                   (!IS_AFFECTED(mob, AFF_WATERWALK))) /* stop drownings */
                 ||((world[to_room].sector_type == SECT_WATER_SWIM) &&
                   (!IS_AFFECTED(mob, AFF_WATERWALK))) /* stop drownings */
                 ||((world[to_room].sector_type == SECT_UNDERWATER) &&
                   (!IS_AFFECTED(mob, AFF_WATERBREATHE))) /* stop drownings */
                 ||(world[to_room].zone != zone));
              char_from_room(mob);
              char_to_room(mob, to_room);
              GET_LOADED_IN(mob) = world[to_room].number;
            }
          }
        } else
          last_cmd = 0;
        break;

      case 'O':      /* Read an object */
        i = in_world(obj_index[ZCMD.arg1].virtual, SCMD_OBJ);

        if (i < ZCMD.arg2) {
          if (ZCMD.arg3 >= 0) {
            obj = read_object(ZCMD.arg1, REAL, NULL);
            if (percent_load(obj)) {
              obj_to_room(obj, ZCMD.arg3);
              if (OBJ_FLAGGED(obj, ITEM_RANDZON)) {
                do {
                  to_room = number(0, top_of_world);
                } while ((IS_SET(world[to_room].room_flags,ROOM_PRIVATE))
                     ||(IS_SET(world[to_room].room_flags,ROOM_GODROOM))
                     ||(IS_SET(world[to_room].room_flags,ROOM_DEATH))
                     ||(world[to_room].zone != zone));
                obj_from_room(obj);
                obj_to_room(obj, to_room);
              }
              last_cmd = 1;
            } else
             extract_obj(obj);
          } else {
            obj = read_object(ZCMD.arg1, REAL, NULL);
            obj->in_room = NOWHERE;
            last_cmd = 1;
          }
        } else
          last_cmd = 0;
        break;

      case 'P':      /* Put an object in an object */
        i = in_world(obj_index[ZCMD.arg1].virtual, SCMD_OBJ);

        if (i < ZCMD.arg2) {
          obj = read_object(ZCMD.arg1, REAL, NULL);
          if (!(obj_to = get_obj_num(ZCMD.arg3))) {
            /* We'll assume it was meant for an object being held instead */
            extract_obj(obj);
            break;
          }
          if (percent_load(obj)) {
            obj_to_obj(obj, obj_to);
            last_cmd = 1;
          } else
            extract_obj(obj);
        } else
          last_cmd = 0;
        break;

      case 'E':       /* Equip/Give object to a mobile */
        if (!mob) {
          ZONE_ERROR("trying to equip non-existant mob, command disabled");
          ZCMD.command = '*';
          break;
        }

        if (ZCMD.arg3) {
          i = in_world(obj_index[ZCMD.arg1].virtual, SCMD_OBJ);

          if (i < ZCMD.arg2) {
            obj = read_object(ZCMD.arg1, REAL, NULL);
            if (percent_load(obj) || is_shopkeeper(mob)) {
              last_cmd = 1;
              obj_to_char(obj, mob);
              if (is_shopkeeper(mob))
                if (give_multiple_items(mob, obj))  /* Should items for sale be given */
                  while (in_world(obj_index[ZCMD.arg1].virtual, SCMD_OBJ) < ZCMD.arg2) {
                    obj = read_object(ZCMD.arg1, REAL, NULL);
                    obj_to_char(obj, mob);
                  }
            } else
              extract_obj(obj);
          } else
            last_cmd = 0;
        } else {
          i = in_world(obj_index[ZCMD.arg1].virtual, SCMD_OBJ);

          if (i < ZCMD.arg2) {
            obj = read_object(ZCMD.arg1, REAL, NULL);
            if (percent_load(obj)) {
              equip_char(mob, obj, find_eq_pos(mob, obj, NULL));
              last_cmd = 1;
            } else
              extract_obj(obj);
          } else
            last_cmd = 0;
        }

        break;

      case 'T':     /* Equip/Give a treasure item */
        if (!mob) {
          ZONE_ERROR("trying to equip non-existant mob, command disabled");
          ZCMD.command = '*';
          break;
        }

        if ((vnum = treasure_item(ZCMD.arg1)) > 0) {
          i = in_world(obj_index[ZCMD.arg1].virtual, SCMD_OBJ);

          if (i < ZCMD.arg2) {
            obj = read_object(vnum, VIRTUAL, NULL);
            if (percent_load(obj)) {
              last_cmd = 1;
              if (ZCMD.arg3)
                obj_to_char(obj, mob);
              else {      /* Allow multiple treasure item loads */
                i = find_eq_pos(mob, obj, NULL);
                while (GET_EQ(mob, i)) {
                  extract_obj(obj);
                  obj = read_object(treasure_item(ZCMD.arg1), VIRTUAL, NULL);
                  i = find_eq_pos(mob, obj, NULL);
                }
                equip_char(mob, obj, i);
              }
            } else
              extract_obj(obj);
          } else
            last_cmd = 0;
        } else {
          ZONE_ERROR("trying to equip an unknown treasure item, command disabled");
          ZCMD.command = '*';
          break;
        }

        break;

      case 'R':     /* Remove object/mobile from room */
        if (ZCMD.arg2) {
          if ((obj = get_obj_in_list_num(ZCMD.arg3, world[ZCMD.arg1].contents)) != NULL)
            extract_obj(obj);
        } else
         for (mob = world[ZCMD.arg1].people; mob; mob = mob->next_in_room)
          if ((GET_MOB_RNUM(mob) == ZCMD.arg3) && (!MOB_FLAGGED(mob, MOB_EXTRACT)) && !FIGHTING(mob)) {
            for (i = 0; i < NUM_WEARS; i++)                  /* remove any worn items */
              if (GET_EQ(mob, i))
                extract_obj(GET_EQ(mob, i));
            for (obj = mob->carrying; obj; obj = obj_to) {   /* remove any carried items */
              obj_to = obj->next_content;
              extract_obj(obj);
            }
            extract_char(mob);
            sql_remove_mob(mob);      /* Do this now so it's cleared immediately */
            break;
          }

        last_cmd = 1;
        break;

      case 'D':      /* Set state of door */
        if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS || 
           (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
          ZONE_ERROR("door does not exist, command disabled");
          ZCMD.command = '*';
        } else
          switch (ZCMD.arg3) {
            case 0:
              REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
              REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
              break;
            case 1:
              SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
              REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
              break;
            case 2:
              SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
              SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
            break;
          }
        last_cmd = 1;
        break;

      default:
        ZONE_ERROR("unknown cmd in reset table; cmd disabled");
        ZCMD.command = '*';
        break;
    }
  }

  zone_table[zone].age = 0;
}


/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int
is_empty(int zone_nr)
{
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected) {
      if (world[i->character->in_room].zone == zone_nr)
        return 0;
    } else {
      if (!i->character)
        continue;
      if (GET_LEVEL(i->character) >= LVL_IMMORT)
        if (world[i->character->in_room].zone == zone_nr)
          return 0;
    }

  return 1;
}





/*************************************************************************
*  stuff related to the save/load player system				 *
*********************************************************************** */

long get_ptable_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!str_cmp(player_table[i].name, arg))
      return (i);

  return (-1);
}


long
get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!str_cmp((player_table + i)->name, arg))
      return ((player_table + i)->id);

  return -1;
}


char *
get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);

  return NULL;
}

long get_next_id(void)
{
  int i, nr;
  extern long *unused_ids;

  for (i = 0; unused_ids[i] != -999; i++) 
    if (unused_ids[i] != NOBODY) {
      nr = unused_ids[i];
      unused_ids[i] = NOBODY;
      return (nr);
    }

  return (++top_idnum);
}

/* Load a char based on name. Character if exists, NULL if not */
struct char_data *load_char(char *name)
{
  int player_i;
  int find_name(char *name);
  struct char_file_u tmp_store;
  struct char_data *player;
  void sql_load_char(long id, struct char_file_u *st);

  if (!*name) {
    mudlog("SYSERR: NULL name passed to load_char.", BRF, LVL_IMMORT, TRUE);
    return (NULL);
  }

  if ((player_i = find_name(name)) >= 0) {
    CREATE(player, struct char_data, 1);                  /* Create our player */
    memset(&tmp_store, 0, sizeof(struct char_file_u));    /* Clear out the structure */
    sql_load_char(player_i, &tmp_store);                  /* Load player data from SQL */
    store_to_char(&tmp_store, player);                    /* Store player data to char struct */
    return (player);
  } else
    return (NULL);
}

/* Load an account based on name. Account if exists, NULL if not */
struct account_data *load_account(char *name)
{
  struct account_data *account;
  extern bool sql_load_account(char *name, struct account_data *account);
  int i;
 
  if (!*name)
    return (NULL);

  CREATE(account, struct account_data, 1);
  if (sql_load_account(name, account)) { 
    for (i = 0; i < MAX_CHARS; i++)
        if (account->chars[i] != 0 && get_name_by_id(account->chars[i]) == NULL)
          account->chars[i] = 0; 
    return (account);
  }
  free_account(account);
  return (NULL);
}

/* Player has removed a char from the account, modify it */
void account_modify(struct descriptor_data *d)
{
  int i, pos = 0;
  extern void sql_save_account(struct account_data *account);

  for (i = 0; i < MAX_CHARS; i++)
    if (d->account->chars[i] == GET_IDNUM(d->character))
      pos = i;

  /* Shift the remaining account chars along in the array */
  for (i = pos; i < (MAX_CHARS - 1); i++)
    d->account->chars[i] = d->account->chars[i + 1];

  /* Clear the last one */
  d->account->chars[--(d->account->num_chars)] = 0;
  sql_save_account(d->account);
}

/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
void save_char(struct char_data *ch, sh_int load_room)
{
  struct char_file_u tmp_store;
  void sql_save_char(struct char_file_u *st);

  if (IS_NPC(ch) || PLR_FLAGGED(ch, PLR_DELETED))
    return;

  memset(&tmp_store, 0, sizeof(struct char_file_u));    /* Clear out the structure */
  char_to_store(ch, &tmp_store);

  if (ch->desc) { 
    strncpy(tmp_store.host, ch->desc->host, HOST_LENGTH);
    tmp_store.host[HOST_LENGTH] = '\0';
  }
  else
   *tmp_store.host = '\0';


  /* changed so that when you save a character with NOWHERE, their load_room doesn't
     get updated. -rparet 20011026  
  */
  if (load_room != NOWHERE)
    tmp_store.player_specials_saved.load_room = GET_ROOM_VNUM(load_room);

  sql_save_char(&tmp_store);
}

void save_mob(struct char_data *mob)
{
  void sql_add_mob(struct char_data *mob);

  if (mob) {
    if (!IS_NPC(mob))
      return;
    sql_add_mob(mob);
  } else
    log("Invalid mobile passed to save_mob.");
}


/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, struct char_data * ch)
{
  void unmount(struct char_data *rider, struct char_data *mount);

  char *kills;
  struct descriptor_data *d;
  struct char_data *it = NULL;
  int i;

  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  GET_SEX(ch) = st->sex;
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.title = str_dup(st->title);
  ch->player.description = str_dup(st->description);
  ch->prompt = str_dup(st->prompt);

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  ch->player.weight = st->weight;

  ch->real_abils = st->real_abilities;
  ch->aff_abils = st->aff_abilities;
  ch->skills = st->skills;
  ch->points = st->points;
  ch->char_specials.saved = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;
  POOFIN(ch) = NULL;
  POOFOUT(ch) = NULL;

  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;

  i = 0;
  kills = strtok(st->race_kills, " ");
  while (kills && i < NUM_RACES) {
    GET_RACE_KILLS(ch, i++) = atoi(kills);
    kills = strtok(NULL, " ");
  }
  
  race_hate_sort(ch);

  ch->points.armor   = 0;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;
  ch->points.dodgeroll = 0;

  if (ch->player.name == NULL)
    CREATE(ch->player.name, char, strlen(st->name) + 1);
  strcpy(ch->player.name, st->name);

  /* give tattoo and vitae stat bonuses */
  tattoo_af(ch, TRUE);
  vitae_af(ch, TRUE);

  /* Add all spell effects */
  for (i = 0; i < MAX_AFFECT; i++)
    {
      if (st->affected[i].type)
	affect_to_char(ch, &st->affected[i]);
    }

  /*
   * If you're not poisoned and you've been away for more than an hour of
   * real time, we'll set your HMV back to full
   */

  if (!IS_AFFECTED(ch, AFF_POISON) &&
      (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR))
    {
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
    }

  /*
   * If your still mounted (due to a crash) unmount
   */
  if (IS_AFFECTED(ch, AFF_MOUNT))
	unmount(ch, NULL);
  /*
   * un-alter flesh
   */
  if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
    REMOVE_BIT(AFF_FLAGS(ch), AFF_FLESH_ALTER);
  /*
   * untag if IT is in the game, otherwise you're still it
   */
  for (d = descriptor_list; d; d = d->next)
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character))
      if (PLR_FLAGGED(d->character, PLR_IT))
        it = d->character;

  if (it && PLR_FLAGGED(ch, PLR_IT))
    REMOVE_BIT(PLR_FLAGS(ch), PLR_IT);

  /*
   * Enter the game active 
   */
   if (PRF_FLAGGED(ch, PRF_INACTIVE))
	REMOVE_BIT(PRF_FLAGS(ch), PRF_INACTIVE);
}


/* copy vital data from a players char-structure to the file structure */
void
char_to_store(struct char_data * ch, struct char_file_u * st)
{
  int i;
  struct master_affected_type *af;
  struct obj_data *char_eq[NUM_WEARS];

  /* Unaffect everything a character can be affected by */

  for (i = 0; i < NUM_WEARS; i++)
    {
      if (GET_EQ(ch, i))
	char_eq[i] = unequip_char(ch, i);
      else
	char_eq[i] = NULL;
    }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++)
    {
      if (af)
	{
	  st->affected[i].type = af->type;
	  st->affected[i].duration = af->duration;
	  st->affected[i].modifier = af->modifier;
	  st->affected[i].location = af->location;
	  st->affected[i].bitvector = af->bitvector;
	  st->affected[i].next = 0;
	  af = af->next;
	}
      else
	{
	  st->affected[i].type = 0;	/* Zero signifies not used */
	  st->affected[i].duration = 0;
	  st->affected[i].modifier = 0;
	  st->affected[i].location = 0;
	  st->affected[i].bitvector = 0;
	  st->affected[i].next = 0;
	}
    }


  /*
   * remove the affections so that the raw values are stored; otherwise the
   * effects are doubled when the char logs back in.
   */

  while (ch->affected)
    affect_remove(ch, ch->affected, TRUE);

  if ((i >= MAX_AFFECT) && af && af->next)
    log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  /* not sure if these removes below are necessary */

  /* remove tattooo and vitae bonuses */
  tattoo_af(ch, FALSE);
  vitae_af(ch, FALSE);

  /* remove ability bonuses */
  ch->aff_abils = ch->real_abils;
  st->skills = ch->skills;

  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->level = GET_LEVEL(ch);
  st->real_abilities = ch->real_abils;
  st->aff_abilities = ch->aff_abils;
  st->points = ch->points;
  st->char_specials_saved = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;

  st->points.armor   = 0;
  st->points.hitroll = 0;
  st->points.damroll = 0;
  st->points.dodgeroll = 0;

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';

  strcpy(st->name, GET_NAME(ch));

  if (ch->prompt)
    strcpy(st->prompt, ch->prompt);
  else
   *st->prompt = '\0';

  /* add tattoo, vitae, spell and eq affections back in now */
  tattoo_af(ch, TRUE);
  vitae_af(ch, TRUE);

  for (i = 0; i < MAX_AFFECT; i++)
    {
      if (st->affected[i].type)
	affect_to_char(ch, &st->affected[i]);
    }

  for (i = 0; i < NUM_WEARS; i++)
    {
      if (char_eq[i])
	equip_char(ch, char_eq[i], i);
    }

  *st->race_kills = '\0';
  for (i = 0; i < NUM_RACES; i++)
    sprintf(st->race_kills, "%s %d", st->race_kills, GET_RACE_KILLS(ch, i));

/*   affect_total(ch); unnecessary, I think !?! */
}				/* Char to store */



/* create a new entry in the in-memory index table for the player file */
void create_entry(char *name)
{
  int i, pos;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if ((pos = get_ptable_by_name(name)) == -1) {	/* new name */
    i = ++top_of_p_table + 1;

    RECREATE(player_table, struct player_index_element, i);
    pos = top_of_p_table;
  }

  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i))); i++);
}



/************************************************************************
*  funcs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *
fread_string(FILE * fl, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[512], *rslt;
  register char *point;
  int done = 0, length = 0, templength = 0;

  *buf = '\0';

  do {
    if (!fgets(tmp, 512, fl)) {
      log("SYSERR: fread_string: format error at or near %s", error);
      exit(1);
    }

    /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL)
      {
	*point = '\0';
	done = 1;
      }
    else
      {
	point = tmp + strlen(tmp) - 1;
	*(point++) = '\r';
	*(point++) = '\n';
	*point = '\0';
      }

    templength = strlen(tmp);

    if (length + templength >= MAX_STRING_LENGTH)
      {
	log("SYSERR: fread_string: string too large (db.c)");
	log(error);
	exit(1);
      }
    else
      {
	strcat(buf + length, tmp);
	length += templength;
      }
  } while (!done);

  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0)
    {
      CREATE(rslt, char, length + 1);
      strcpy(rslt, buf);
    }
  else
    rslt = NULL;

  return rslt;
}


/* release memory allocated for an account struct */
void free_account(struct account_data *account)
{
  if (!account)
    return;

  if (account->name)
    FREE(account->name);

  if (account->pwd)
    FREE(account->pwd);

  FREE(account);
}

/* release memory allocated for a char struct */
void free_char(struct char_data * ch)
{
  int i;
  struct alias *a;
  void free_alias(struct alias * a);
  void toggle_compression(struct descriptor_data *t);

  if (!ch)
    return;

  while (ch->affected)
    affect_remove(ch, ch->affected, FALSE);

  if (ch->prompt)
    FREE(ch->prompt);

  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {
    while ((a = GET_ALIASES(ch)) != NULL) {
      GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
      free_alias(a);
    }

    if (ch->player_specials->poofin)
      FREE(ch->player_specials->poofin);
    if (ch->player_specials->poofout)
      FREE(ch->player_specials->poofout);
    FREE(ch->player_specials);
    if (IS_NPC(ch))
      log("SYSERR: Mob had player_specials allocated!");
  }
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (ch->player.name)
      FREE(ch->player.name);
    if (ch->player.title)
      FREE(ch->player.title);
    if (ch->player.short_descr)
      FREE(ch->player.short_descr);
    if (ch->player.long_descr)
      FREE(ch->player.long_descr);
    if (ch->player.description)
      FREE(ch->player.description);
  } else if ((i = GET_MOB_RNUM(ch)) > -1) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
      FREE(ch->player.name);
    if (ch->player.title && ch->player.title != mob_proto[i].player.title)
      FREE(ch->player.title);
    if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
      FREE(ch->player.short_descr);
    if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
      FREE(ch->player.long_descr);
    if (ch->player.description && ch->player.description != mob_proto[i].player.description)
      FREE(ch->player.description);
    if (ch->mob_specials.noise && ch->mob_specials.noise != mob_proto[i].mob_specials.noise)
      FREE(ch->mob_specials.noise);
    if (ch->mob_specials.sound && ch->mob_specials.sound != mob_proto[i].mob_specials.sound)
      FREE(ch->mob_specials.sound);
    if (ch->mob_specials.id)
      FREE(ch->mob_specials.id);
  }

  /* Player's only - MCCP stuff */
  if (!IS_NPC(ch))
    if ((ch->desc) && IS_MCCP(ch))
      toggle_compression(ch->desc);

  if (ch->desc)
    ch->desc->character = NULL;

  FREE(ch);
}


/* release memory allocated for an obj struct */
void
free_obj(struct obj_data * obj)
{
  int nr;
  struct extra_descr_data *this, *next_one;

  if ((nr = GET_OBJ_RNUM(obj)) == -1)
    {
      if (obj->name)
	FREE(obj->name);
      if (obj->description)
	FREE(obj->description);
      if (obj->short_description)
	FREE(obj->short_description);
      if (obj->action_description)
	FREE(obj->action_description);
      if (obj->killer)
        FREE(obj->killer);
      if (obj->owner)
        FREE(obj->owner);
      if (obj->id)
        FREE(obj->id);
      if (obj->ex_description)
	for (this = obj->ex_description; this; this = next_one)
	  {
	    next_one = this->next;
	    if (this->keyword)
	      FREE(this->keyword);
	    if (this->description)
	      FREE(this->description);
	    FREE(this);
	  }
    }
  else
    {
      if (obj->id)
        FREE(obj->id);
      if (obj->name && obj->name != obj_proto[nr].name)
	FREE(obj->name);
      if (obj->description && obj->description != obj_proto[nr].description)
	FREE(obj->description);
      if (obj->short_description &&
	  obj->short_description != obj_proto[nr].short_description)
	FREE(obj->short_description);
      if (obj->action_description &&
	  obj->action_description != obj_proto[nr].action_description)
	FREE(obj->action_description);
      if (obj->killer)
        FREE(obj->killer);
      if (obj->owner)
        FREE(obj->owner);
      if (obj->ex_description &&
	  obj->ex_description != obj_proto[nr].ex_description)
	for (this = obj->ex_description; this; this = next_one)
	  {
	    next_one = this->next;
	    if (this->keyword)
	      FREE(this->keyword);
	    if (this->description)
	      FREE(this->description);
	    FREE(this);
	  }
    }

  FREE(obj);
}


/* releases memory allocated to MCCP compression struct */
void free_mccp(struct compr *comp)
{
  if (comp) {
    if (comp->stream) {
      deflateEnd(comp->stream);
      FREE(comp->stream);
    }
    if (comp->buff_in)
      FREE(comp->buff_in);
    if (comp->buff_out)
      FREE(comp->buff_out);
    FREE(comp);
  }
}


/* read contets of a text file, alloc space, point buf to it */
int
file_to_string_alloc(char *name, char **buf)
{
  char temp[(MAX_STRING_LENGTH * 2)];

  if (*buf)
    FREE(*buf);

  if (file_to_string(name, temp) < 0)
    {
      *buf = "";
      return -1;
    }
  else
    {
      *buf = str_dup(temp);
      return 0;
    }
}


/* read contents of a text file, and place in buf */
int
file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[READ_SIZE+3];

  *buf = '\0';

  if (!(fl = fopen(name, "r")))
    {
      sprintf(tmp, "Error reading %s", name);
      perror(tmp);
      return (-1);
    }
  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (!feof(fl))
    {
      if (strlen(buf) + strlen(tmp) < (MAX_STRING_LENGTH * 2))
        strcat(buf, tmp);
      else
      {
        sprintf(buf2, "SYSERR: %s: string too big (%d max)", name,
		       (MAX_STRING_LENGTH * 2));
	log(buf2);
        return (0);
      }
    }
  } while (!feof(fl));

  fclose(fl);

  return (0);
}



/* clear some of the the working variables of a char */
void
reset_char(struct char_data * ch)
{
  int i;

  for (i = 0; i < NUM_WEARS; i++)
    GET_EQ(ch, i) = NULL;

  ch->followers = NULL;
  ch->master = NULL;
  ch->in_room = NOWHERE;
  ch->carrying = NULL;
  ch->next = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room = NULL;
  FIGHTING(ch) = NULL;
  GET_WAS_IN(ch) = NOWHERE;
  ch->char_specials.position = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;

  if (GET_HIT(ch) > GET_MAX_HIT(ch))
    GET_HIT(ch) = GET_MAX_HIT(ch);
  if (GET_MOVE(ch) > GET_MAX_MOVE(ch))
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
  if (GET_MANA(ch) > GET_MAX_MANA(ch))
    GET_MANA(ch) = GET_MAX_MANA(ch);

  GET_LAST_TELL(ch) = NOBODY;
  GET_LAST_MOVED(ch) = 0;
  IS_PARRIED(ch) = FALSE;
  COMBAT_STATE(ch) = 0;
  GET_COMBAT_WAIT(ch) = 0;
  DELAY(ch) = 0;
}



/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void
clear_char(struct char_data * ch)
{
  ch->in_room = NOWHERE;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch) = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  FIGHTING(ch) = NULL;
  ch->affected = NULL;

  GET_AC(ch) = 0; /* no armor */
  GET_LEVEL(ch) = 0;    /* allows new players to be created correctly */
}

/* Since accounts are now the primary descriptor, it's possible to cross-link
data to other characters on the account in a single logged-on session. To avoid
this, we wipe the d->character clean every time we want to do something with a
new character, or prior to FREE-ing it so that MCCP is unaffected. */
void clear_desc(struct descriptor_data *d)
{
  if (!d->character)
    return;
  
  d->character->desc = NULL;  /* Stops MCCP being FREE'd for descriptor */
  free_char(d->character);
  d->character = NULL;
}

void
clear_object(struct obj_data * obj)
{

  obj->item_number = NOTHING;
  obj->in_room = NOWHERE;
}


void
clear_sql_object(struct obj_file_elem * object)
{
  memset((char *) object, 0, sizeof(struct obj_file_elem));

  object->item_number = NOTHING;
  strcpy(object->name, "\0");
  strcpy(object->desc, "\0");
  strcpy(object->shortd, "\0");
}

/* initialize a new character 
   called the first time a character is created
*/
/* FIX remove the magic numbers from this function if possible. */
void
init_char(struct char_data * ch)
{
  int i;

  /* create a player_special structure */
  if (!ch->player_specials)
    CREATE(ch->player_specials, struct player_special_data, 1);

  /* create the known-spells bitvector */
  for (i = 0; i < NUM_SPELLS; i++)
    ch->skills.known[i] = '0';

  /* *** if this is our first player --- he be God *** */

  if (top_of_p_table == 0)
    {
      GET_EXP(ch) = 7000000; 
      GET_LEVEL(ch) = LVL_IMPL;

    /* learn all spells */
    for (i = 0; i < NUM_SPELLS; i++)
      ch->skills.known[i] = '1';

      ch->points.max_hit  = 666;
      ch->points.max_mana = 666;
      ch->points.max_move = 666;
    }
  set_title(ch, NULL);

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.description = NULL;

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  ch->prompt = str_dup(DEFAULT_PROMPT);

  /* Set base level abilities */
  ch->real_abils.str = 5;
  ch->real_abils.dex = 5;
  ch->real_abils.con = 5;
  ch->real_abils.wis = 5;
  ch->real_abils.intel = 5;
  ch->real_abils.cha = 5;

  for (i = 0; i < MAX_SQUELCH; i++)
     GET_SQUELCH(ch, i) = 0;

  for (i = 0; i < 5; i++)
    GET_RACE_HATE(ch, i) = -1;

  /* make favors for sex */
  if (ch->player.sex == SEX_MALE)
      ch->player.weight = number(120, 180);
  else
      ch->player.weight = number(100, 160);

  ch->points.max_mana = 50;  
  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);  /* hp determined somewhere else ? */
  ch->points.max_move = 50;
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.armor = 0;

  if ((i = get_ptable_by_name(GET_NAME(ch))) != -1)
    player_table[i].id = GET_IDNUM(ch) = get_next_id();
  else
    log("SYSERR: init_char: Character '%s' not found in player table.", GET_NAME(ch));

  for (i = 1; i <= MAX_SKILL_GROUPS; i++) {
    GET_GROUP_LVL(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? 4 : 0);
    SET_SKILL(ch, i, (GET_LEVEL(ch) == LVL_IMPL ? 100 : 0));
  }

  ch->char_specials.saved.affected_by = 0;

  for (i = 0; i < MAX_SAVING; i++)
    GET_SAVE(ch, i) = 0;

  for (i = 0; i < MAX_COND; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : 48);
  GET_COND(ch, DRUNK) = 0;

  for (i = 0; i <= MAX_SKILL_GROUPS; i++)
    IMPROVE_TIME(ch, i) = 0;

  GET_LOADROOM(ch) = NOWHERE;
  GET_BINDROOM(ch) = NOWHERE;
  OUTLAW_TIME(ch) = 0;
  JAIL_TIME(ch) = 0;
  MOUNT_TIME(ch) = 0;
  GET_GOLD(ch) = 0;
  GET_STAT_PTS(ch) = 0;
  GET_GROUP_PTS(ch) = 0;
  GET_POS(ch) = POS_STANDING;
}



/* returns the real number of the room with given virtual number */
int
real_room(long virtual)
{
  long bot, top, mid;

  bot = 0;
  top = top_of_world;

  /* perform binary search on world-table */
  for (;;)
    {
      mid = (bot + top) / 2;

      if ((world + mid)->number == virtual)
	return mid;
      if (bot >= top)
	return NOWHERE;
      if ((world + mid)->number > virtual)
	top = mid - 1;
      else
	bot = mid + 1;
    }
}



/* returns the real number of the monster with given virtual number */
int
real_mobile(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_mobt;

  /* perform binary search on mob-table */
  for (;;)
    {
      mid = (bot + top) / 2;

      if ((mob_index + mid)->virtual == virtual)
	return (mid);
      if (bot >= top)
	return (-1);
      if ((mob_index + mid)->virtual > virtual)
	top = mid - 1;
      else
	bot = mid + 1;
    }
}



/* returns the real number of the object with given virtual number */
int
real_object(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;)
    {
      mid = (bot + top) / 2;

      if ((obj_index + mid)->virtual == virtual)
	return (mid);
      if (bot >= top)
	return (-1);
      if ((obj_index + mid)->virtual > virtual)
	top = mid - 1;
      else
	bot = mid + 1;
    }
}


/* As faction.c isn't used, this currently has no use - needs to be replaced with
   SQL code if it is to be implemented - 20030101  mcooper
void
save_char_file_u(struct char_file_u st)
{
  int player_i;
  int find_name(char *name);

  if((player_i = find_name(st.name)) >=0 )
    {
      fseek(player_fl, player_i * sizeof(struct char_file_u), SEEK_SET);
      fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
    }
} */

/* check if machine knows daylight saving time */
void check_dst(int check)
{
  struct tm *ltime;
  time_t chtime;

  chtime = time(0);
  ltime = localtime(&chtime);
  if (ltime->tm_isdst == 1)
    daylight_saving_time = 1;
  else if (ltime->tm_isdst == 0)
    daylight_saving_time = 0;
  else
    {
    if (check)
       log("Machine can't figure out if DST or not.");
    daylight_saving_time = 0;
    }
}


int check_object(struct obj_data *obj)
{
  int error = FALSE;

  if (GET_OBJ_WEIGHT(obj) <= 0 && (error = TRUE))
    log("SYSERR: Object #%d (%s) has negative or zero weight (%d).",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_WEIGHT(obj));

  if (GET_OBJ_LOAD(obj) < 0 && (error = TRUE))
    log("SYSERR: Object #%d (%s) has negative percentage load (%d).",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_LOAD(obj));

  sprintbit(GET_OBJ_WEAR(obj), wear_bits, buf);
  if (strstr(buf, "UNDEFINED") && (error = TRUE))
    log("SYSERR: Object #%d (%s) has unknown wear flags.",
	GET_OBJ_VNUM(obj), obj->short_description);

  sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
  if (strstr(buf, "UNDEFINED") && (error = TRUE))
    log("SYSERR: Object #%d (%s) has unknown extra flags.",
	GET_OBJ_VNUM(obj), obj->short_description);

  sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
  if (strstr(buf, "UNDEFINED") && (error = TRUE))
    log("SYSERR: Object #%d (%s) has unknown affection flags.",
	GET_OBJ_VNUM(obj), obj->short_description);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_FOUNTAIN:
    if (GET_OBJ_VAL(obj, 1) > GET_OBJ_VAL(obj, 0) && (error = TRUE))
      log("SYSERR: Object #%d (%s) contains (%ld) more than maximum (%ld).",
		GET_OBJ_VNUM(obj), obj->short_description,
		GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 0));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    error |= check_object_level(obj, 0);
    error |= check_object_spell_number(obj, 1);
    error |= check_object_spell_number(obj, 2);
    error |= check_object_spell_number(obj, 3);
    break;
  case ITEM_STAFF:
    error |= check_object_level(obj, 0);
    error |= check_object_spell_number(obj, 3);
    if (GET_OBJ_VAL(obj, 2) > GET_OBJ_VAL(obj, 1) && (error = TRUE))
      log("SYSERR: Object #%d (%s) has more charges (%ld) than maximum (%ld).",
	GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 1));
    break;
 }

  return (error);
}

int check_object_spell_number(struct obj_data *obj, int val)
{
  int error = FALSE;
  const char *spellname;

  if (GET_OBJ_VAL(obj, val) == -1)	/* i.e.: no spell */
    return (error);

  /*
   * Check for negative spells, spells beyond the top define, and any
   * spell which is actually a skill.
   */
  if (GET_OBJ_VAL(obj, val) < 0)
    error = TRUE;
  if (GET_OBJ_VAL(obj, val) > TOP_SPELL_DEFINE)
    error = TRUE;
  if (GET_OBJ_VAL(obj, val) > MAX_SPELLS && GET_OBJ_VAL(obj, val) <= MAX_SKILLS)
    error = TRUE;
  if (error)
    log("SYSERR: Object #%d (%s) has out of range spell #%ld.",
      GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));

  /*
   * This bug has been fixed, but if you don't like the special behavior...
   */
#if 0
  if (GET_OBJ_TYPE(obj) == ITEM_STAFF &&
    HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, val), MAG_AREAS | MAG_MASSES))
    log("... '%s' (#%d) uses %s spell '%s'.",
      obj->short_description, GET_OBJ_VNUM(obj),
      HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, val), MAG_AREAS) ? "area" : "mass",
      skill_name(GET_OBJ_VAL(obj, val)));
#endif

  if (scheck)		/* Spell names don't exist in syntax check mode. */
    return (error);

  /* Now check for unnamed spells. */
  spellname = skill_name(GET_OBJ_VAL(obj, val));

  if ((spellname == unused_spellname) && (error = TRUE))
    log("SYSERR: Object #%d (%s) uses '%s' spell #%ld.",
      GET_OBJ_VNUM(obj), obj->short_description, spellname, GET_OBJ_VAL(obj, val));

  return (error);
}

int check_object_level(struct obj_data *obj, int val)
{
  int error = FALSE;

  if ((GET_OBJ_VAL(obj, val) < 0 || GET_OBJ_VAL(obj, val) > 100) && (error = TRUE))
    log("SYSERR: Object #%d (%s) has out of range level #%ld.",
      GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));

  return (error);
}


void load_mob_items(struct char_data *mob)
{
  int i;
  bool loaded = FALSE;

  if (!mob || !IS_NPC(mob))
    return;

  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(mob, i)) {
      loaded = TRUE;
      break;
    }

  if ((mob->carrying) || loaded)
    load_spell_scroll(mob);
}

void load_spell_scroll(struct char_data *mob)
{
  ush_int spell = 0;
  ush_int perc_load = (GET_LEVEL(mob) / 2);
  int tree_idx = 0, x = 1;
  int spells_by_level[MAX_SPELLS+1];
  struct obj_data *obj;
  extern struct skill_tree_data skill_tree[];
  extern struct obj_data *create_spell_scroll(int spellnum, int group, int level, char *name);

  
  /* Percentage chance of load is based on the level of the mob and the type of spell scroll */
  /* Go through the tree and stick all the spells equal to or less than mob level into an array */
  while (*skill_tree[++tree_idx].name != '\n')
   if ((skill_tree[tree_idx].level <= GET_LEVEL(mob)) && (skill_tree[tree_idx].skill_number <= MAX_SPELLS)) { 
      spells_by_level[x] = tree_idx;
      x++; 
    }

  spell = spells_by_level[number(1, x-1)];

  if (!strcmp(skill_tree[spell].name, "\n")) {
    sprintf(buf, "SYSERR: Attempting invalid spell assign (%d) in load_spell_scroll.",
      spell);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  perc_load /= skill_tree[spell].level + 1;
  if (perc_load > number(0, 100)) {
    if (!(obj = create_spell_scroll(skill_tree[spell].skill_number, skill_tree[spell].group, skill_tree[spell].level, skill_tree[spell].name)))
      return;
    obj_to_char(obj, mob);
  }
}

void unload_room_descs(int zone)
{
  struct char_data *mob, *next_mob;
  struct obj_data *obj, *obj2, *next_obj;
  struct olc_save_info *entry;
  long counter, realcounter;
  int i;
  void Mob_crashsave(struct char_data * mob);

  /* Check to see if the zone has been saved in OLC - if not, don't reload it!! */
  if (olc_save_list)
    for (entry = olc_save_list; entry; entry = entry->next)
      if (entry->type != OLC_SAVE_TREASURE)
        if (zone_table[zone].number == entry->zone) {
          zone_table[zone].age = (ZO_DEAD - 1);
          return;
        }

  zone_table[zone].age = 0;
  REMOVE_BIT(ZONE_FLAGS(zone), ZONE_UNLOAD);

  for (counter = zone_table[zone].number * 100;
       counter <= zone_table[zone].top;
       counter++) {
    realcounter = real_room(counter);

    if (realcounter < 0)
      continue;

    /* FREE the room description */
    if (world[realcounter].description != NULL)
      FREE(world[realcounter].description);

    /* Set any mobs and their items for UNLOADING */
    if (world[realcounter].people)
      for (mob = world[realcounter].people; mob; mob = next_mob) {
        next_mob = mob->next_in_room;
        if (IS_NPC(mob) && !MOB_FLAGGED(mob, MOB_EXTRACT)) {
          save_mob(mob);
          Mob_crashsave(mob);
          SET_BIT(MOB_FLAGS(mob), MOB_UNLOAD);
          if (mob->carrying)
            for (obj = mob->carrying; obj; obj = next_obj) {
              next_obj = obj->next_content;
              while ((obj2 = obj->contains)) {   /* Allow for 2 levels of container */
                SET_BIT(GET_OBJ_EXTRA(obj2), ITEM_UNLOAD);
                while (obj2->contains) {
                  SET_BIT(GET_OBJ_EXTRA(obj2->contains), ITEM_UNLOAD);
                  extract_obj(obj2->contains);
                }
                extract_obj(obj2);
              }
              SET_BIT(GET_OBJ_EXTRA(obj), ITEM_UNLOAD);
              extract_obj(obj);
            }
          for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(mob, i)) {
              obj2 = GET_EQ(mob, i)->contains;
              while (obj2) {   /* Allow for 2 levels of container */
                SET_BIT(GET_OBJ_EXTRA(obj2), ITEM_UNLOAD);
                while (obj2->contains) {
                  SET_BIT(GET_OBJ_EXTRA(obj2->contains), ITEM_UNLOAD);
                  extract_obj(obj2->contains);
                }
                obj2 = obj2->next_content;
              }
              SET_BIT(GET_OBJ_EXTRA(GET_EQ(mob, i)), ITEM_UNLOAD);
              extract_obj(GET_EQ(mob, i));
            }
          extract_char(mob);
        }
      }

    /* Set any objs for UNLOADING */
    if (world[realcounter].contents) {
      House_crashsave(realcounter);
      for (obj = world[realcounter].contents; obj; obj = next_obj) {
        next_obj = obj->next_content;
        while ((obj2 = obj->contains)) {   /* Allow for 2 levels of container */
          SET_BIT(GET_OBJ_EXTRA(obj2), ITEM_UNLOAD);
          while (obj2->contains) {
            SET_BIT(GET_OBJ_EXTRA(obj2->contains), ITEM_UNLOAD);
            extract_obj(obj2->contains);
          }
          extract_obj(obj2);
        }
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_UNLOAD);
        extract_obj(obj);
      }
    }
  }
}

void parse_desc(FILE * fl, int virt_nr)
{
  char line[256];
  char *room_name;
  int real_nr = real_room(virt_nr);

  room_name = fread_string(fl, buf2);
  if (str_cmp(room_name, world[real_nr].name) && !ROOM_FLAGGED(real_nr, ROOM_NOUNLOAD)) { /* shrug */
    log("SYSERR: Read name does not equal room name.");
    exit(1);
  }

  FREE(room_name); /* fread_string allocates mem..so free it */
  if (world[real_nr].description == NULL)
    world[real_nr].description = fread_string(fl, buf2);
  else if (!ROOM_FLAGGED(real_nr, ROOM_NOUNLOAD)) {
    sprintf(buf, "SYSERR: Attempting to reload Non-NULL description for room %ld.",
      world[real_nr].number);
    log(buf);
    exit(1);
  }

  for (;;) {
    if (!get_line(fl, line)) {
      sprintf(buf, "SYSERR: Format error when reloading room descriptions for %d.wld\n",
        world[real_nr].zone);
      log(buf);
      exit(1);
    }
    if (*line == 'S') /* continue to the end of the record */
      if (*(line + 1) == '\0') /* make sure it IS the end of the record */
        return;
  }
}

void reload_room_descs(int zone_num)
{
  FILE *db_file;
  int nr = -1, last = 0;
  char line[256];
  struct olc_save_info *entry;
  void sql_load_mobs(int zone);
  void sql_load_objs(int zone);

  /* Check to see if the zone has been saved in OLC - if not, don't reload it!! */
  if (olc_save_list)
    for (entry = olc_save_list; entry; entry = entry->next)
      if (entry->type != OLC_SAVE_TREASURE)
        if (zone_table[zone_num].number == entry->zone)
          return;

  SET_BIT(ZONE_FLAGS(zone_num), ZONE_UNLOAD);
  sprintf(buf, "Reloading room descs for zone %d.", zone_table[zone_num].number);
  log(buf);

  sql_load_mobs(zone_num);
  sql_load_objs(zone_num);
  reset_zone(zone_num);

  zone_table[zone_num].age = 0;

  sprintf(buf1, "%d.wld", zone_table[zone_num].number);
  sprintf(buf2, "%s/%s", WLD_PREFIX, buf1);
  if (!(db_file = fopen(buf2, "r"))) {
    perror(buf2);
    exit(1);
  }

  for (;;) {
    if (!get_line(db_file, line)) {
	    fprintf(stderr, "Format error after world #%d\n", nr);
	    exit(1);
    }
    if (*line == '$')
      break;

    if (*line == '#') {
      last = nr;
      if (sscanf(line, "#%d", &nr) != 1) {
	      fprintf(stderr, "Format error after world #%d\n", last);
	      exit(1);
      }
      if (nr >= 99999)
	      break;
      else
	      parse_desc(db_file, nr);
    } else {
      fprintf(stderr, "Format error in world file near world #%d\n", nr);
      fprintf(stderr, "Offending line: '%s'\n", line);
      exit(1);
    }
  }

  fclose(db_file);
}
