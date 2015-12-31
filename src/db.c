/* ************************************************************************
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

/* $Id: db.c 1511 2008-06-06 02:45:35Z jravn $ */

#define __DB_C__

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "scripts.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world = NULL; /* array of rooms        */
int top_of_world = 0;       /* ref to top element of world   */

struct char_data *character_list = NULL;    /* global linked list of
                         * chars     */
struct index_data *mob_index;   /* index table for mobile file   */
struct char_data *mob_proto;    /* prototypes for mobs       */
int top_of_mobt = 0;        /* top of mobile index table     */

struct obj_data *object_list = NULL;    /* global linked list of objs    */
struct index_data *obj_index;   /* index table for object file   */
struct obj_data *obj_proto; /* prototypes for objs       */
int top_of_objt = 0;        /* top of object index table     */

struct zone_data *zone_table;   /* zone table            */
int top_of_zone_table = 0;  /* top element of zone tab   */
struct message_list fight_messages[MAX_MESSAGES];/* fighting messages    */

struct player_index_element *player_table = NULL;/* index to plr file    */
FILE *player_fl = NULL;     /* file desc of player file  */
int top_of_p_table = 0;     /* ref to top of table       */
int top_of_p_file = 0;      /* ref of size of p file     */
long top_idnum = 0;     /* highest idnum in use      */

int no_mail = 0;        /* mail disabled?        */
int mini_mud = 0;       /* mini-mud mode?        */
int no_rent_check = 0;      /* skip rent check on boot?  */
time_t boot_time = 0;       /* time of mud boot      */
int game_restrict = 0;      /* level of game restriction     */
int r_mortal_start_room;    /* rnum of mortal start room     */
int r_immort_start_room;    /* rnum of immort start room     */
int r_frozen_start_room;    /* rnum of frozen start room     */
int r_kiroshi_start_room;    /* rnum of Kir-Oshi start room   */
int r_alaozar_start_room;    /* rnum of alaozar start room */

char *credits = NULL;       /* game credits          */
char *news = NULL;      /* mud news          */
char *motd = NULL;      /* message of the day - mortals */
char *imotd = NULL;     /* message of the day - immorts */
char *help = NULL;      /* help screen           */
char *info = NULL;      /* info page             */
char *wizlist = NULL;       /* list of higher gods       */
char *immlist = NULL;       /* list of peon gods         */
char *background = NULL;    /* background story      */
char *handbook = NULL;      /* handbook for new immortals    */
char *policies = NULL;      /* policies page         */
char *future = NULL;            /* future command */

struct help_index_element *help_table = 0;  /* the help table    */
int top_of_helpt = 0;       /* top of help index table   */

struct dns_entry *dns_cache[DNS_HASH_NUM]; /* for dns caching * dnsmod */

struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;   /* the infomation about the weather */
struct player_special_data dummy_mob;   /* dummy spec area for mobs  */
struct reset_q_type reset_q;    /* queue of zones to be reset    */

struct review_t review[25];     /* list of last 25 gossips */

bool daylight_saving_time;      /* daylight savings or not */

/* local functions */
void boot_world_files();
void setup_dir(FILE * fl, int room, int dir);
void read_mud_date_from_file(void);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode);
void parse_room(FILE * fl, int virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
char *parse_object(FILE * obj_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_help(FILE *fl);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
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
void clear_char(struct char_data * ch);
void boot_dns(void); /* dnsmod */
void save_dns_cache(void); /* dnsmod */
int get_host_from_cache(struct dns_entry *dnsd); /* dnsmod */
void add_dns_host(struct dns_entry *dnsd, char *hostname); /* dnsmod */

/* external functions */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void weather_and_time(int mode);
void mag_assign_spells(void);
void boot_social_messages(void);
void update_obj_file(void); /* In objsave.c */
void sort_commands(void);
void sort_spells(void);
void load_banned(void);
void Read_Invalid_List(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
int hsort(const void *a, const void *b);
void tattoo_af(struct char_data * ch, bool add);
void init_clans(void);
void load_compile_time();
extern int isname_with_abbrevs(char *str, char *namelist);

/* external vars */
extern int no_specials;

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
      file_to_string_alloc(IMOTD_FILE, &imotd);
      file_to_string_alloc(HELP_PAGE_FILE, &help);
      file_to_string_alloc(INFO_FILE, &info);
      file_to_string_alloc(POLICIES_FILE, &policies);
      file_to_string_alloc(HANDBOOK_FILE, &handbook);
      file_to_string_alloc(BACKGROUND_FILE, &background);
      file_to_string_alloc(FUTURE_FILE, &future);
   }
  else if (!str_cmp(arg, "wizlist"))
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
  else if (!str_cmp(arg, "immlist"))
    file_to_string_alloc(IMMLIST_FILE, &immlist);
  else if (!str_cmp(arg, "news"))
    file_to_string_alloc(NEWS_FILE, &news);
  else if (!str_cmp(arg, "credits"))
    file_to_string_alloc(CREDITS_FILE, &credits);
  else if (!str_cmp(arg, "motd"))
    file_to_string_alloc(MOTD_FILE, &motd);
  else if (!str_cmp(arg, "imotd"))
    file_to_string_alloc(IMOTD_FILE, &imotd);
  else if (!str_cmp(arg, "help"))
    file_to_string_alloc(HELP_PAGE_FILE, &help);
  else if (!str_cmp(arg, "info"))
    file_to_string_alloc(INFO_FILE, &info);
  else if (!str_cmp(arg, "policy"))
    file_to_string_alloc(POLICIES_FILE, &policies);
  else if (!str_cmp(arg, "handbook"))
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
  else if (!str_cmp(arg, "background"))
    file_to_string_alloc(BACKGROUND_FILE, &background);
  else if (!str_cmp(arg, "future"))
    file_to_string_alloc(FUTURE_FILE, &future);
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


static void
init_review_strings( void )
{
  int count;
  for (count = 0; count < 25; count++)
    {
      strcpy(review[count].string, "\0");
      strcpy(review[count].name,   "\0");
      review[count].invis = 0;
    }
}


void
boot_world(void)
{
  boot_world_files();
}

void boot_world_files()
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

  log("Renumbering zone table.");
  renum_zone_table();

  log("Initializing the review strings.");
  init_review_strings();

  if (!no_specials)
    {
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

  log("Resetting the game time:");
  reset_time();
  check_dst(1);
  load_compile_time();

  log("Reading news, credits, help, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);
  file_to_string_alloc(FUTURE_FILE, &future);

  boot_world();

  log("Loading help entries.");
  index_boot(DB_BOOT_HLP);

  log("Generating player index.");
  build_player_index();

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

  log("Assigning spell and skill levels.");
  init_spell_levels();

  log("Sorting command list and spells.");
  sort_commands();
  sort_spells();

  log("Booting mail system.");
  if (!scan_file())
    {
      log("    Mail boot failed -- Mail system disabled");
      no_mail = 1;
    }
  log("Reading banned site and invalid-name list.");
  load_banned();
  Read_Invalid_List();

  log("Booting DNS cache.");
  boot_dns();

  if (!no_rent_check)
    {
      log("Deleting timed-out crash and rent files:");
      update_obj_file();
      log("Done.");
    }
  for (i = 0; i <= top_of_zone_table; i++)
    {
      sprintf(buf2, "Resetting %s (rooms %d-%d).",
          zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0),
          zone_table[i].top);
      log(buf2);
      reset_zone(i);
    }

  reset_q.head = reset_q.tail = NULL;

  log("Booting Scripts.");
  boot_lua();

  if (!mini_mud)
    {
      log("Booting houses.");
      House_boot();
    }
  log("Booting clans.");
  init_clans();

  boot_time = time(0);

  log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void
reset_time(void)
{
  long beginning_of_time = 650336715;
  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);
  read_mud_date_from_file();

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



/* generate index table for the player file */
void
build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  if (!(player_fl = fopen(PLAYER_FILE, "r+b")))
    {
      if (errno != ENOENT)
    {
      perror("fatal error opening playerfile");
      exit(1);
    }
      else
    {
      log("No playerfile.  Creating a new one.");
      touch(PLAYER_FILE);
      if (!(player_fl = fopen(PLAYER_FILE, "r+b")))
        {
          perror("fatal error opening playerfile");
          exit(1);
        }
    }
    }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs)
    {
      sprintf(buf, "   %ld players in database.", recs);
      log(buf);
      CREATE(player_table, struct player_index_element, recs);
    }
  else
    {
      player_table = NULL;
      top_of_p_file = top_of_p_table = -1;
      return;
    }

  for (; !feof(player_fl);)
    {
      fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
      if (!feof(player_fl))
    {   /* new record */
      nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
           (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
      player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
    }
    }

  top_of_p_file = top_of_p_table = nr;
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
  char *index_filename, *prefix;
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
      sprintf(buf1, "Error opening index file '%s'", buf2);
      perror(buf1);
      exit(1);
    }

  /* first, count the number of records in the file so we can CREATE */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$')
    {
      sprintf(buf2, "%s/%s", prefix, buf1);
      if (!(db_file = fopen(buf2, "r"))) {
    perror(buf2);
    log("file listed in index not found");
    exit(1);
      }
      else
    {
      if (mode == DB_BOOT_ZON)
        rec_count++;
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
      log("SYSERR: boot error - 0 records counted");
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
      perror(buf2);
      exit(1);
    }
      switch (mode)
    {
    case DB_BOOT_WLD:
    case DB_BOOT_OBJ:
    case DB_BOOT_MOB:
      discrete_load(db_file, mode);
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


void
discrete_load(FILE * fl, int mode)
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
    if (!get_line(fl, line))
      {
        fprintf(stderr, "Format error after %s #%d\n", modes[mode], nr);
        exit(1);
      }
      if (*line == '$')
    return;

      if (*line == '#')
    {
      last = nr;
      if (sscanf(line, "#%d", &nr) != 1)
        {
          fprintf(stderr, "Format error after %s #%d\n", modes[mode], last);
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
      fprintf(stderr, "Format error in %s file near %s #%d\n",
          modes[mode], modes[mode], nr);
      fprintf(stderr, "Offending line: '%s'\n", line);
      exit(1);
    }
    }
}


long
asciiflag_conv(char *flag)
{
  long flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++)
    {
      if (islower(*p))
    flags |= 1 << (*p - 'a');
      else if (isupper(*p))
    flags |= 1 << (26 + (*p - 'A'));

      if (!isdigit(*p))
    is_number = 0;
    }

  if (is_number)
    flags = atol(flag);

  return flags;
}


/* load the rooms */
void
parse_room(FILE * fl, int virtual_nr)
{
  static int room_nr = 0, zone = 0;
  int t[10], i;
  char line[256], flags[128];
  char flags2[256], flags3[128];
  char flags4[256];
  struct extra_descr_data *new_descr;

  sprintf(buf2, "room #%d", virtual_nr);

  if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1))
    {
      fprintf(stderr, "Room #%d is below zone %d.\n", virtual_nr, zone);
      exit(1);
    }
  while (virtual_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table)
      {
    fprintf(stderr, "Room %d is outside of any zone.\n", virtual_nr);
    exit(1);
      }
  world[room_nr].zone = zone;
  world[room_nr].number = virtual_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);
  CREATE(world[room_nr].script, struct script_data, 1);
  world[room_nr].script->name = NULL;

  if (!get_line(fl, line) ||
      sscanf(line, " %d %s %s %s %s %d ", t, flags, flags2, flags3,
             flags4, t + 2) != 6)
  {
      fprintf(stderr, "Format error in room #%d\n", virtual_nr);
      exit(1);
  }
  /* t[0] is the zone number; ignored with the zone-file system */
  world[room_nr].room_flags[0] = asciiflag_conv(flags);
  world[room_nr].room_flags[1] = asciiflag_conv(flags2);
  world[room_nr].room_flags[2] = asciiflag_conv(flags3);
  world[room_nr].room_flags[3] = asciiflag_conv(flags4);
  world[room_nr].sector_type = t[2];

  world[room_nr].func = NULL;
  world[room_nr].contents = NULL;
  world[room_nr].people = NULL;
  world[room_nr].light = 0; /* Zero light sources */

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf, "Format error in room #%d (expecting D/E/S)", virtual_nr);

  for (;;)
    {
      if (!get_line(fl, line))
    {
      fprintf(stderr, "%s\n", buf);
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
      new_descr->description = fread_string(fl, buf2);
      new_descr->next = world[room_nr].ex_description;
      world[room_nr].ex_description = new_descr;
      break;
        case 'R':
          sscanf((line + 1), " %s %d", flags, &i);
          GET_ROOM_SCRIPT(room_nr)->name = str_dup(flags);
          GET_ROOM_SCRIPT(room_nr)->lua_functions = i;
          break;
    case 'S':           /* end of room */
      top_of_world = room_nr++;
      return;
      break;
    default:
      fprintf(stderr, "%s\n", buf);
      exit(1);
      break;
    }
    }
}



/* read direction data */
void
setup_dir(FILE * fl, int room, int dir)
{
  int t[5];
  char line[256];

  sprintf(buf2, "room #%d, direction D%d", world[room].number, dir);

  CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
  world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
  world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

  if (!get_line(fl, line))
    {
      fprintf(stderr, "Format error, %s\n", buf2);
      exit(1);
    }
  if (sscanf(line, " %d %d %d ", t, t + 1, t + 2) != 3)
    {
      fprintf(stderr, "Format error, %s\n", buf2);
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
  extern int mortal_start_room;
  extern int immort_start_room;
  extern int frozen_start_room;
  extern int kiroshi_start_room;
  extern int alaozar_start_room;

  if ((r_mortal_start_room = real_room(mortal_start_room)) < 0)
    {
      log("SYSERR: Kir Drax'in start room does not exist.  Change in config.c.");
      exit(1);
    }
  if ((r_kiroshi_start_room = real_room(kiroshi_start_room)) < 0)
    {
     if (!mini_mud)
      log("SYSERR: Kir-Oshi start room does not exist. Change in config.c");
      r_kiroshi_start_room = r_mortal_start_room;
    }
  if ((r_alaozar_start_room = real_room(alaozar_start_room)) < 0)
    {
     if (!mini_mud)
      log("SYSERR: Alaozar start room does not exist.  Change in config.c");
      r_alaozar_start_room = r_mortal_start_room;
    }
  if ((r_immort_start_room = real_room(immort_start_room)) < 0)
    {
      if (!mini_mud)
    log("SYSERR:  Warning: Immort start room does not exist.  "
        "Change in config.c.");
      r_immort_start_room = r_mortal_start_room;
    }
  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0)
    {
      if (!mini_mud)
    log("SYSERR:  Warning: Frozen start room does not exist.  "
        "Change in config.c.");
      r_frozen_start_room = r_mortal_start_room;
    }
}


/* resolve all vnums into rnums in the world */
void
renum_world(void)
{
  register int room, door;

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
  int zone, cmd_no, a, b;

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
      {
    a = b = 0;
    switch (ZCMD.command)
      {
      case 'M':
        a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
        b = ZCMD.arg3 = real_room(ZCMD.arg3);
        break;
      case 'O':
        a = ZCMD.arg1 = real_object(ZCMD.arg1);
        if (ZCMD.arg3 != NOWHERE)
          b = ZCMD.arg3 = real_room(ZCMD.arg3);
        break;
      case 'G':
        a = ZCMD.arg1 = real_object(ZCMD.arg1);
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

        /* Transform the legacy 'R' command format. */
        bool legacy_cmd = (ZCMD.arg3 == -1);
        if (legacy_cmd) {
          ZCMD.arg3 = ZCMD.arg2;
          ZCMD.arg2 = 1;
        }

        if (ZCMD.arg2)
          b = ZCMD.arg3 = real_object(ZCMD.arg3);
        else
          b = ZCMD.arg3 = real_mobile(ZCMD.arg3);
        break;
      }
    if (a < 0 || b < 0)
      {
        if (!mini_mud)
          log_zone_error(zone, cmd_no, "Invalid vnum, cmd disabled");
        ZCMD.command = '*';
      }
      }
}


void
parse_simple_mob(FILE *mob_f, int i, int nr)
{
  int j, t[10];
  char line[256];

  mob_proto[i].real_abils.str = 11;
  mob_proto[i].real_abils.intel = 11;
  mob_proto[i].real_abils.wis = 11;
  mob_proto[i].real_abils.dex = 11;
  mob_proto[i].real_abils.con = 11;
  mob_proto[i].real_abils.cha = 11;

  get_line(mob_f, line);
  if (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
         t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9)
    {
      fprintf(stderr, "Format error in mob #%d, first line after S flag\n"
          "...expecting line of form '# # # #d#+# #d#+#'\n", nr);
      exit(1);
    }
  GET_LEVEL(mob_proto + i) = t[0];
  /*
   * Serapis 132212ZMAY97
   * Set mob stats based on level.
   */
  if (GET_LEVEL(mob_proto + i)>15)
  {
     int statmod = GET_LEVEL(mob_proto + i)-15;
     mob_proto[i].real_abils.str += MIN(number(0,statmod), 7);
     mob_proto[i].real_abils.intel += MIN(number(0,statmod), 7);
     mob_proto[i].real_abils.wis += MIN(number(0,statmod), 7);
     mob_proto[i].real_abils.dex += MIN(number(0,statmod), 7);
     mob_proto[i].real_abils.con += MIN(number(0,statmod), 7);
     mob_proto[i].real_abils.cha += MIN(number(0,statmod), 7);
  }
  mob_proto[i].points.hitroll = 20 - t[1];
  mob_proto[i].points.armor = 10 * t[2];

  /* max hit = 0 is a flag that H, M, V is xdy+z */
  mob_proto[i].points.max_hit = 0;
  mob_proto[i].points.hit = t[3];
  mob_proto[i].points.mana = t[4];
  mob_proto[i].points.move = t[5];

  mob_proto[i].points.max_mana = 10;
  mob_proto[i].points.max_move = 50;

  mob_proto[i].mob_specials.damnodice = t[6];
  mob_proto[i].mob_specials.damsizedice = t[7];
  mob_proto[i].points.damroll = t[8];

  get_line(mob_f, line);
  sscanf(line, " %d %d ", t, t + 1);
  GET_GOLD(mob_proto + i) = t[0];

  GET_EXP(mob_proto + i) = t[1];

  get_line(mob_f, line);
  if (sscanf(line, " %d %d %d %d ", t, t + 1, t + 2, t + 3) != 3)
    {
      fprintf(stderr, "Format error in mob #%d, second line after S flag\n"
          "...expecting line of form '# # #'\n", nr);
    }

  mob_proto[i].char_specials.position = t[0];
  mob_proto[i].mob_specials.default_pos = t[1];
  mob_proto[i].player.sex = t[2];

  mob_proto[i].player.class = 0;
  mob_proto[i].player.weight = 200;
  mob_proto[i].player.height = 198;

  for (j = 0; j < 3; j++)
    GET_COND(mob_proto + i, j) = -1;

  /*
   * these are now save applies; base save numbers for MOBs are now from
   * the warrior save table.
   */
  for (j = 0; j < 5; j++)
    GET_SAVE(mob_proto + i, j) = 0;

  mob_proto[i].mob_specials.race = RACE_OTHER;
  mob_proto[i].mob_specials.noise = NULL;

  for (j = 0; j < 5; j++)
    GET_RACE_HATE(mob_proto + i, j) = -1;

  IS_PARRIED(mob_proto + i) = FALSE;
  GET_JAIL_TIMER(mob_proto +i) = FALSE;
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

  CASE("StrAdd")
    {
      RANGE(0, 100);
      mob_proto[i].real_abils.str_add = num_arg;
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


  CASE("Script")
   {
     mob_index[i].script->name = str_dup(arg1);
     mob_index[i].script->lua_functions = asciiflag_conv(arg2);
   }

  if (!matched)
    {
      fprintf(stderr, "Warning: unrecognized espec keyword %s in mob #%d\n",
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
      if (!strcmp(line, "E"))   /* end of the ehanced section */
    return;
      else if (*line == '#')
    {   /* we've hit the next mob, maybe? */
      fprintf(stderr, "Unterminated E section in mob #%d\n", nr);
      exit(1);
    }
      else
    parse_espec(line, i, nr);
    }

  fprintf(stderr, "Unexpected end of file reached after mob #%d\n", nr);
  exit(1);
}


void
parse_mobile(FILE * mob_f, int nr)
{
  static int i = 0;
  int j, t[10];
  char line[256], *tmpptr, letter;
  char f1[128], f2[128];
  char f3[128], f4[128];
  char f5[128], f6[128];
  char f7[128], f8[128];

  mob_index[i].virtual = nr;
  mob_index[i].number = 0;
  mob_index[i].func = NULL;

  clear_char(mob_proto + i);

  CREATE(mob_index[i].script, struct script_data, 1);
  mob_index[i].script->name = NULL;

  mob_proto[i].player_specials = &dummy_mob;

  sprintf(buf2, "mob vnum %d", nr);

  /***** String data *** */
  mob_proto[i].player.name = fread_string(mob_f, buf2);
  if (!mob_proto[i].player.name)
  {
    char mybuf[80];
    sprintf(mybuf,"SYSERR:Error at mob %d.", nr);
        log(mybuf);
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
  get_line(mob_f, line);
  sscanf(line, "%s %s %s %s %s %s %s %s %d %c", f1, f2, f3, f4,
               f5, f6, f7, f8, t + 2, &letter);
  MOB_FLAGS(mob_proto + i)[0] = asciiflag_conv(f1);
  MOB_FLAGS(mob_proto + i)[1] = asciiflag_conv(f2);
  MOB_FLAGS(mob_proto + i)[2] = asciiflag_conv(f3);
  MOB_FLAGS(mob_proto + i)[3] = asciiflag_conv(f4);
  SET_BIT_AR(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
  if (MOB_FLAGGED(mob_proto + i, MOB_EXTRACT)) {
    /* Rather bad to load mobiles with this bit already set. */
    log("SYSERR: Mob #%d has reserved bit EXTRACT set.", nr);
    REMOVE_BIT_AR(MOB_FLAGS(mob_proto + i), MOB_EXTRACT);
  }

  AFF_FLAGS(mob_proto + i)[0] = asciiflag_conv(f5);
  AFF_FLAGS(mob_proto + i)[1] = asciiflag_conv(f6);
  AFF_FLAGS(mob_proto + i)[2] = asciiflag_conv(f7);
  AFF_FLAGS(mob_proto + i)[3] = asciiflag_conv(f8);
  GET_ALIGNMENT(mob_proto + i) = t[2];

  switch (letter)
    {
    case 'S':   /* Simple monsters */
      parse_simple_mob(mob_f, i, nr);
      break;
    case 'E':   /* Circle3 Enhanced monsters */
      parse_enhanced_mob(mob_f, i, nr);
      break;
      /* add new mob types here.. */
    default:
      fprintf(stderr, "Unsupported mob type '%c' in mob #%d\n", letter, nr);
      exit(1);
      break;
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
  int t[10], j;
  float load = 0.0;
  char *tmpptr;
  char f1[256], f2[256];
  char f3[256], f4[256];
  char f5[256], f6[256];
  char f7[256], f8[256];
  struct extra_descr_data *new_descr;

  obj_index[i].virtual = nr;
  obj_index[i].number = 0;
  obj_index[i].func = NULL;

  clear_object(obj_proto + i);

  CREATE(obj_index[i].script, struct script_data, 1);
  obj_index[i].script->name = NULL;

  obj_proto[i].in_room = NOWHERE;
  obj_proto[i].item_number = i;

  sprintf(buf2, "object #%d", nr);

  /* *** string data *** */
  if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL)
    {
      fprintf(stderr, "Null obj name or format error at or near %s\n", buf2);
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
  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, " %d %s %s %s %s %s %s %s %s",
                  t, f1, f2, f3, f4, f5, f6, f7, f8)) != 9)
    {
      fprintf(stderr, "Format error in first numeric line "
          "(expecting 9 args, got %d), %s\n", retval, buf2);
      exit(1);
    }
  obj_proto[i].obj_flags.type_flag = t[0];
  obj_proto[i].obj_flags.extra_flags[0] = asciiflag_conv(f1);
  obj_proto[i].obj_flags.extra_flags[1] = asciiflag_conv(f2);
  obj_proto[i].obj_flags.extra_flags[2] = asciiflag_conv(f3);
  obj_proto[i].obj_flags.extra_flags[3] = asciiflag_conv(f4);
  obj_proto[i].obj_flags.wear_flags[0] = asciiflag_conv(f5);
  obj_proto[i].obj_flags.wear_flags[1] = asciiflag_conv(f6);
  obj_proto[i].obj_flags.wear_flags[2] = asciiflag_conv(f7);
  obj_proto[i].obj_flags.wear_flags[3] = asciiflag_conv(f8);

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4)
    {
      fprintf(stderr, "Format error in second numeric line "
          "(expecting 4 args, got %d), %s\n", retval, buf2);
      exit(1);
    }
  obj_proto[i].obj_flags.value[0] = t[0];
  obj_proto[i].obj_flags.value[1] = t[1];
  obj_proto[i].obj_flags.value[2] = t[2];
  obj_proto[i].obj_flags.value[3] = t[3];

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %f", t, t + 1, &load)) != 3) {
    fprintf(stderr, "Format error in third numeric line "
        "(expecting 3 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.cost = t[1];

  SET_OBJ_LOAD(&obj_proto[i], load);

  /* check to make sure that weight of containers exceeds curr. quantity */
  if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON ||
      obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN)
    {
      if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
    obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
    }

  /* *** extra descriptions and affect fields *** */

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    {
      obj_proto[i].affected[j].location = APPLY_NONE;
      obj_proto[i].affected[j].modifier = 0;
    }

  strcat(buf2, ", after numeric constants (expecting E/A/#xxx)");
  j = 0;

  for (;;)
    {
      if (!get_line(obj_f, line))
    {
      fprintf(stderr, "Format error in %s\n", buf2);
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
      if (j >= MAX_OBJ_AFFECT)
        {
          fprintf(stderr, "Too many A fields (%d max), %s\n",
              MAX_OBJ_AFFECT, buf2);
          exit(1);
        }
      get_line(obj_f, line);
      sscanf(line, " %d %d ", t, t + 1);
      obj_proto[i].affected[j].location = t[0];
      obj_proto[i].affected[j].modifier = t[1];
      j++;
      break;
    case '$':
    case '#':
      top_of_objt = i++;
      return line;
      break;
    default:
      fprintf(stderr, "Format error in %s\n", buf2);
      exit(1);
      break;
    }
    }
}


#define Z   zone_table[zone]

/* load the zone table and command tables */
void
load_zones(FILE * fl, char *zonename)
{
  static int zone = 0;
  int cmd_no = 0, num_of_cmds = 0, line_num = 0, tmp, error;
  char *ptr, buf[256], zname[256];

  strcpy(zname, zonename);

  while (get_line(fl, buf))
    num_of_cmds++;      /* this should be correct within 3 or so */
  rewind(fl);

  if (num_of_cmds == 0)
    {
      fprintf(stderr, "%s is empty!\n", zname);
      exit(0);
    }
  else
    CREATE(Z.cmd, struct reset_com, num_of_cmds);

  line_num += get_line(fl, buf);

  if (sscanf(buf, "#%d", &Z.number) != 1)
    {
      fprintf(stderr, "Format error in %s, line %d\n", zname, line_num);
      exit(0);
    }
  sprintf(buf2, "beginning of zone #%d", Z.number);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);
  if (sscanf(buf, " %d %d %d ", &Z.top, &Z.lifespan, &Z.reset_mode) != 3)
    {
      fprintf(stderr, "Format error in 3-constant line of %s", zname);
      exit(0);
    }
  cmd_no = 0;

  for (;;)
    {
      if ((tmp = get_line(fl, buf)) == 0)
    {
      fprintf(stderr, "Format error in %s - premature end of file\n",
          zname);
      exit(0);
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
      if (strchr("MOEPDRL", ZCMD.command) == NULL)
    {   /* a 3-arg command */
      if (sscanf(ptr, " %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2) != 3)
        error = 1;
    }
      else
    {
      if (sscanf(ptr, " %d %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2,
             &ZCMD.arg3) != 4)
        error = 1;
    }

      ZCMD.if_flag = tmp;

      if (error)
    {
      fprintf(stderr, "Format error in %s, line %d: '%s'\n",
          zname, line_num, buf);
      exit(0);
    }
      ZCMD.line = line_num;
      cmd_no++;
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
*  procedures for resetting, both play-time and boot-time        *
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
  clear_char(ch);
  ch->next = character_list;
  character_list = ch;

  return ch;
}


/* create a new mobile from a prototype */
struct char_data *
read_mobile(int nr, int type)
{
  int i;
  struct char_data *mob;

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
  clear_char(mob);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

  if (!mob->points.max_hit)
    {
      mob->points.max_hit = dice(mob->points.hit, mob->points.mana) +
    mob->points.move;
    }
  else
    mob->points.max_hit = number(mob->points.hit, mob->points.mana);

  mob->points.hit = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  mob_index[i].number++;

  if (GET_GOLD(mob))
  {
   /* set the gold to be gold+-(1 to 20%) */
   if (!number(0,1))
    GET_GOLD(mob)+=(number(1,20)*GET_GOLD(mob))/100;
   else
    GET_GOLD(mob)-=(number(1,20)*GET_GOLD(mob))/100;
  }
  GET_GOLD(mob) = MAX(0, GET_GOLD(mob));

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

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  obj->next = object_list;
  object_list = obj;

  return obj;
}

/* Initializes a rare object by modifying its AFFECT fields */
static void init_rare(struct obj_data *obj)
{
  int i, mod, loc;

  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    loc = obj->affected[i].location;
    if (loc && dice(1, 100) <= 20) {
      switch (loc) {
      case APPLY_DAMROLL:
      case APPLY_HITROLL:
        mod = 1;
        break;
      case APPLY_AC:
        mod = 5;
        break;
      default:
        mod = 0;
        break;
      }

      if (number(0, 1) == 0)
        mod *= -1;

      obj->affected[i].modifier += mod;
    }
  }
}

/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type)
{
   struct obj_data *obj;
   int i;

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
   }
   else
      i = nr;

   CREATE(obj, struct obj_data, 1);
   clear_object(obj);
   *obj = obj_proto[i];
   obj->next = object_list;
   object_list = obj;

   obj_index[i].number++;

   if (IS_OBJ_STAT(obj, ITEM_RARE))
     init_rare(obj);

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
  /*char buf[128];*/

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
      for (i = 0; i <= top_of_zone_table; i++)
    {
      if (zone_table[i].age < zone_table[i].lifespan &&
          zone_table[i].reset_mode)
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
    }   /* end - one minute has passed */


  /* dequeue zones (if possible) and reset */
  /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
  for (update_u = reset_q.head; update_u; update_u = update_u->next)
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
    is_empty(update_u->zone_to_reset))
      {
    reset_zone(update_u->zone_to_reset);
    /* Serapis 141303ZMAY97
    sprintf(buf, "Auto zone reset: %s",
        zone_table[update_u->zone_to_reset].name);
    mudlog(buf, CMP, LVL_GOD, FALSE);
    */
    /* dequeue */
    if (update_u == reset_q.head)
      reset_q.head = reset_q.head->next;
    else
      {
        for (temp = reset_q.head; temp->next != update_u;
         temp = temp->next);

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
    { log_zone_error(zone, cmd_no, message); }

/* Returns true if an object loads, false if it doesn't.
   The chance of returning true is based on its percent chance to load. */
static bool percent_load(struct obj_data *obj)
{
  if (GET_OBJ_LOAD(obj) > (uniform() * 100.0))
    return TRUE;
  else
    return FALSE;
}

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int cmd_no, to_room, i, last_cmd = 0, tmp_cmd = 0, loop = 0;
  struct char_data *mob = NULL;
  struct obj_data *obj, *obj_to;
  time_t mytime;
  extern time_t boot_time;

  mytime = time(0) - boot_time;

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
    {
      if (ZCMD.if_flag && !last_cmd)
    continue;

      /* if_flag commands should be dependent on the last command with no if_flag */
      if (!ZCMD.if_flag)
    last_cmd = 0;

      switch (ZCMD.command) {
    case '*':           /* ignore command */
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

    case 'M':           /* read a mobile */
      if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
        mob = read_mobile(ZCMD.arg1, REAL);
        char_to_room(mob, ZCMD.arg3);
        last_cmd = 1;

        /*zone79 randload mobs*/
        if ((mob_index[ZCMD.arg1].virtual < 7999) &&
        (mob_index[ZCMD.arg1].virtual > 7899) ) {
          do {
        to_room = number(0, top_of_world);
          }
          while( (IS_SET_AR(world[to_room].room_flags,ROOM_PRIVATE))
             ||(IS_SET_AR(world[to_room].room_flags,ROOM_GODROOM))
             ||(IS_SET_AR(world[to_room].room_flags,ROOM_DEATH))
             ||(IS_SET_AR(world[to_room].room_flags,ROOM_NOMOB))
             ||(IS_SET_AR(world[to_room].room_flags,ROOM_HOUSE))
             ||(IS_SET_AR(world[to_room].room_flags,ROOM_ATRIUM))
             ||(world[to_room].sector_type == SECT_CITY)
             || (zone_table[world[to_room].zone].number == 163) );

          char_from_room(mob);
          char_to_room(mob, to_room);
        }
        /* Random load within zone */
        if (MOB_FLAGGED(mob, MOB_RANDZON)) {
          do {
        to_room = number(0, top_of_world);
          }  while( (IS_SET_AR(world[to_room].room_flags,ROOM_PRIVATE))
            ||(IS_SET_AR(world[to_room].room_flags,ROOM_GODROOM))
            ||(IS_SET_AR(world[to_room].room_flags,ROOM_DEATH))
            ||(IS_SET_AR(world[to_room].room_flags,ROOM_NOMOB))
            ||(IS_SET_AR(world[to_room].room_flags,ROOM_HOUSE))
            ||(IS_SET_AR(world[to_room].room_flags,ROOM_ATRIUM))
            ||(world[to_room].zone != zone) );

          char_from_room(mob);
          char_to_room(mob, to_room);
        }
      }
      break;

    case 'O':           /* read an object */
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
            if (ZCMD.arg3 >= 0) {
              obj = read_object(ZCMD.arg1, REAL);

              if (percent_load(obj)) {
                obj_to_room(obj, ZCMD.arg3);
                last_cmd = 1;
              } else {
                extract_obj(obj);
              }
            }
            else
            {
              obj = read_object(ZCMD.arg1, REAL);
              obj->in_room = NOWHERE;
              last_cmd = 1;
            }
          }
      break;

    case 'P':           /* object to object */
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
        obj = read_object(ZCMD.arg1, REAL);
        if (!(obj_to = get_obj_num(ZCMD.arg3))) {
          ZONE_ERROR("target obj not found");
          break;
        }
        if (percent_load(obj)) {
          obj_to_obj(obj, obj_to);
          last_cmd = 1;
        } else
          extract_obj(obj);
      }
      break;

    case 'G':           /* obj_to_char */
      if (!mob) {
        ZONE_ERROR("attempt to give obj to non-existant mob");
        break;
      }
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
        obj = read_object(ZCMD.arg1, REAL);
        if (percent_load(obj)) {
          obj_to_char(obj, mob);
          last_cmd = 1;
        } else
          extract_obj(obj);
      }
      break;

    case 'E':           /* object to equipment list */
      if (!mob) {
          ZONE_ERROR("trying to equip non-existant mob");
          break;
          }
      if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
            if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS) {
              ZONE_ERROR("invalid equipment pos number");
            } else {
              obj = read_object(ZCMD.arg1, REAL);
              if (percent_load(obj)) {
                equip_char(mob, obj, ZCMD.arg3);
                last_cmd = 1;
              }
              else
                extract_obj(obj);
            }
          }
      break;

    case 'R': /* Remove object/mobile from room */
      if (ZCMD.arg2) {
        if ((obj = get_obj_in_list_num(ZCMD.arg3, world[ZCMD.arg1].contents)) != NULL) {
          obj_from_room(obj);
          extract_obj(obj);
          last_cmd = 1;
        }
      } else {
        for (mob = world[ZCMD.arg1].people; mob; mob = mob->next_in_room) {
          if ((GET_MOB_RNUM(mob) == ZCMD.arg3)
                  && !MOB_FLAGGED(mob, MOB_EXTRACT)
                  && !FIGHTING(mob)) {
        for (i = 0; i < NUM_WEARS; i++) /* remove any worn items */
          if (GET_EQ(mob, i))
            extract_obj(GET_EQ(mob, i));
        for (obj = mob->carrying; obj; obj = obj_to) { /* remove any carried items */
          obj_to = obj->next_content;
          extract_obj(obj);
        }
        extract_char(mob);
        last_cmd = 1;
        break;
          }
        }
      }
      break;

    case 'D':           /* set state of door */
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
          (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
          ZONE_ERROR("door does not exist");
      } else {
            if (ROOM_FLAGGED(ZCMD.arg1, ROOM_SECRET_MARK))
              REMOVE_BIT_AR(ROOM_FLAGS(ZCMD.arg1), ROOM_SECRET_MARK);
        switch (ZCMD.arg3) {
        case 0:
          REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
             EX_LOCKED);
          REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
             EX_CLOSED);
          break;
        case 1:
          SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
              EX_CLOSED);
          REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
             EX_LOCKED);
          break;
        case 2:
          SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
              EX_LOCKED);
          SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
              EX_CLOSED);
          break;
        }
        last_cmd = 1;
          }
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
    if (!i->connected)
      if (world[i->character->in_room].zone == zone_nr)
    return 0;

  return 1;
}





/*************************************************************************
*  stuff related to the save/load player system              *
*********************************************************************** */


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


/* Load a char, TRUE if loaded, FALSE if not */
int
load_char(char *name, struct char_file_u * char_element)
{
  int player_i;

  int find_name(char *name);

  if ((player_i = find_name(name)) >= 0)
    {
      fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)),
        SEEK_SET);
      fread(char_element, sizeof(struct char_file_u), 1, player_fl);
      return (player_i);
    }
  else
    return (-1);
}


/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
void
save_char(struct char_data * ch, int load_room)
{
  struct char_file_u st;

  if (IS_NPC(ch) || GET_PFILEPOS(ch) < 0)
    return;

  char_to_store(ch, &st);

  if (ch->desc)
  {
    strncpy(st.host, ch->desc->host, HOST_LENGTH);
    st.host[HOST_LENGTH] = '\0';
  }
  else
    *st.host = '\0';

  /* Vnums can be larger than sh_int, which unfortunately is what is used
     to store the load_room in the player file. We check for that case
     to prevent an error. */
  if (!PLR_FLAGGED(ch, PLR_LOADROOM))
    {
      if (load_room == NOWHERE)
    st.player_specials_saved.load_room = (sh_int)NOWHERE;
      else {
    extern int mortal_start_room;
    int load_room_vnum;
    load_room_vnum = world[real_room(load_room)].number;
    if (load_room_vnum != (sh_int)load_room_vnum) {
      log("SYSERR: Attemping to save player load room that is larger than sh_int size, value will be set to default.");
      load_room_vnum = mortal_start_room;
    }
    st.player_specials_saved.load_room = (sh_int)load_room_vnum;
      }
    }

  strcpy(st.pwd, GET_PASSWD(ch));

  fseek(player_fl, GET_PFILEPOS(ch) * sizeof(struct char_file_u), SEEK_SET);
  fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
}



/* copy data from the file structure to a char struct */
void
store_to_char(struct char_file_u * st, struct char_data * ch)
{
  void unmount(struct char_data *rider, struct char_data *mount);

  struct descriptor_data *d;
  struct char_data *it = NULL;
  int i;

  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.title = str_dup(st->title);
  ch->player.description = str_dup(st->description);

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->real_abils = st->abilities;
  ch->aff_abils = st->abilities;
  ch->points = st->points;
  ch->char_specials.saved = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;
  POOFIN(ch) = NULL;
  POOFOUT(ch) = NULL;

  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;

  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;

  for (i = 0; i < 5; i++)
    GET_RACE_HATE(ch, i) = -1;

  IS_PARRIED(ch) = FALSE;
  GET_JAIL_TIMER(ch) = FALSE;

  ch->points.armor = 100;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;

  if (ch->player.name == NULL)
    CREATE(ch->player.name, char, strlen(st->name) + 1);
  strcpy(ch->player.name, st->name);
  strcpy(ch->player.passwd, st->pwd);

  /*
   * giv tattoo stat bonuses
   */
  tattoo_af(ch, TRUE);

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
   * if you've been gone a day mud time, set your tatto timer back to 0
   */
  if ( (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR) )
     TAT_TIMER(ch)=0;

  /*
   * If your still mounted (due to a crash) unmount
   */
  if (IS_AFFECTED(ch, AFF_MOUNT))
    unmount(ch, NULL);
  /*
   * un-alter flesh
   */
  if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_FLESH_ALTER);
  /*
   * untag if IT is in the game, otherwise you're still it
   */
  for (d = descriptor_list; d; d = d->next)
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character))
      if (PLR_FLAGGED(d->character, PLR_IT))
        it = d->character;

  if (it && PLR_FLAGGED(ch, PLR_IT))
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IT);

  /*
   * Enter the game active
   */
   if (PRF_FLAGGED(ch, PRF_INACTIVE))
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_INACTIVE);
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
      st->affected[i].type = 0; /* Zero signifies not used */
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
    affect_remove(ch, ch->affected);

  if ((i >= MAX_AFFECT) && af && af->next)
    log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  /*
   * remove tattooo bonuses
   */
  tattoo_af(ch, FALSE);

  ch->aff_abils = ch->real_abils;

  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = GET_CLASS(ch);
  st->level = GET_LEVEL(ch);
  st->abilities = ch->real_abils;
  st->points = ch->points;
  st->char_specials_saved = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;

  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';

  strcpy(st->name, GET_NAME(ch));

  /* add tattoo, spell and eq affections back in now */
  tattoo_af(ch, TRUE);

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
/*   affect_total(ch); unnecessary, I think !?! */
}               /* Char to store */



void
save_etext(struct char_data * ch)
{
/* this will be really cool soon */

}


/* create a new entry in the in-memory index table for the player file */
int
create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1)
    {
      CREATE(player_table, struct player_index_element, 1);
      top_of_p_table = 0;
    }
  else
    {
       top_of_p_table++;
       RECREATE(player_table, struct player_index_element,(top_of_p_table+1));
       if (!player_table)
       {
          perror("create entry");
          exit(1);
       }
    }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);

  return (top_of_p_table);
}



/************************************************************************
*  funcs of a (more or less) general utility nature         *
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
    if (!fgets(tmp, 512, fl))
      {
    fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
        error);
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


/* release memory allocated for a char struct */
void
free_char(struct char_data * ch)
{
  int i;
  struct alias *a;
  void free_alias(struct alias * a);

  while (ch->affected)
    affect_remove(ch, ch->affected);

  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob)
    {
      while ((a = GET_ALIASES(ch)) != NULL)
    {
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
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1))
    {
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
    }
  else if ((i = GET_MOB_RNUM(ch)) > -1)
    {
      /* otherwise, free strings only if the string is not pointing at proto */
      if (ch->player.name && ch->player.name != mob_proto[i].player.name)
    FREE(ch->player.name);
      if (ch->player.title && ch->player.title != mob_proto[i].player.title)
    FREE(ch->player.title);
      if (ch->player.short_descr &&
      ch->player.short_descr != mob_proto[i].player.short_descr)
    FREE(ch->player.short_descr);
      if (ch->player.long_descr &&
      ch->player.long_descr != mob_proto[i].player.long_descr)
    FREE(ch->player.long_descr);
      if (ch->player.description &&
      ch->player.description != mob_proto[i].player.description)
    FREE(ch->player.description);
      if (ch->mob_specials.noise &&
      ch->mob_specials.noise != mob_proto[i].mob_specials.noise)
    FREE(ch->mob_specials.noise);
    }

  if ( (!IS_NPC(ch)) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1))
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


/* read contets of a text file, alloc space, point buf to it */
int
file_to_string_alloc(char *name, char **buf)
{
  char temp[MAX_STRING_LENGTH];

  if (*buf)
    FREE(*buf);

  if (file_to_string(name, temp) < 0)
    {
      *buf = NULL;
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
      log("Error opening %s", name);
      return (-1);
    }
  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (!feof(fl))
      {
    if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH)
      {
        sprintf(buf, "SYSERR: %s: string too big (%d max)", name,
            MAX_STRING_LENGTH);
        log(buf);
        *buf = '\0';
        return -1;
      }
    strcat(buf, tmp);
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

  GET_LAST_TELL(ch) = NOBODY;

  for (i = 0; i < 5; i++)
    GET_RACE_HATE(ch, i) = -1;

  IS_PARRIED(ch) = FALSE;
  GET_JAIL_TIMER(ch) = FALSE;
}



/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void
clear_char(struct char_data * ch)
{
  memset((char *) ch, 0, sizeof(struct char_data));

  ch->in_room = NOWHERE;
  GET_PFILEPOS(ch) = -1;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch) = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;

  GET_AC(ch) = 100;     /* Basic Armor */
  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;
}


void
clear_object(struct obj_data * obj)
{
  memset((char *) obj, 0, sizeof(struct obj_data));

  obj->item_number = NOTHING;
  obj->in_room = NOWHERE;
}




/* initialize a new character only if class is set */
void
init_char(struct char_data * ch)
{
  int i, taeller;

  /* create a player_special structure */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  /* *** if this is our first player --- he be God *** */

  if (top_of_p_table == 0)
    {
      GET_EXP(ch) = 7000000;
      GET_LEVEL(ch) = LVL_IMPL;

      ch->points.max_hit = 500;
      ch->points.max_mana = 100;
      ch->points.max_move = 82;
    }
  set_title(ch, NULL);

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.description = NULL;

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i < MAX_TONGUE; i++)
    GET_TALK(ch, i) = 0;

  /* make favors for sex */
  if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(120, 180);
      ch->player.height = number(160, 200);
    }
  else
    {
      ch->player.weight = number(100, 160);
      ch->player.height = number(150, 180);
    }

  ch->points.max_mana = 100;
  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.max_move = 82;
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.armor = 100;

  player_table[top_of_p_table].id = GET_IDNUM(ch) = ++top_idnum;

  for (i = 1; i <= MAX_SKILLS; i++)
    {
      if (GET_LEVEL(ch) < LVL_IMPL)
SET_SKILL(ch, i, 0);
      else
      SET_SKILL(ch, i, 100);
    }

  for(taeller=0; taeller < AF_ARRAY_MAX; taeller++)
     ch->char_specials.saved.affected_by[taeller] = 0;

  for (i = 0; i < 5; i++)
    GET_SAVE(ch, i) = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : 24);

  GET_LOADROOM(ch) = NOWHERE;
}



/* returns the real number of the room with given virtual number */
int
real_room(int virtual)
{
  int bot, top, mid;

  if (virtual == NOWHERE)
    return NOWHERE;

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
}

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


void
read_mud_date_from_file(void)
{
  FILE *f;
  struct time_write read_date;

  f = fopen("etc/date_record", "r");

  if (!f) {
    log("SYSERR: File etc/date_record not found, mud date will be reset to default!");
    return;
  }

  if (fscanf(f, "%d %d %d", &read_date.year, &read_date.month, &read_date.day) != 3) {
    log("SYSERR: File etc/date_record is corrupted, mud date will be reset to default!");
  } else {
    time_info.year = read_date.year;
    time_info.month = read_date.month;
    time_info.day   = read_date.day;
  }

  fclose(f);
}


