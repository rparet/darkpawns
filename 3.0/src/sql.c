/* ************************************************************************
*  File: sql.c                                          Part of CircleMUD *
*  Usage: MySQL manipulation, database APIs                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996-1999 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "constants.h"
#include "interpreter.h"
#include "shop.h"
#include <mysql/mysql.h>

/* Shop handling defines */
#define SHOP_KEEPER(i)		(shop_index[(i)].keeper)
#define SHOP_OWNER(i)  		(shop_index[(i)].owner)
#define SHOP_BANK(i)            (shop_index[(i)].bankAccount)

/* Local functions */
MYSQL_RES *sql_query(MYSQL *db, const char *query, bool retval);
struct obj_file_elem sql_read_object(MYSQL_ROW entry);
sbyte sql_rent_code(struct char_data *ch);
struct rent_info sql_rent_read(long id);
void sql_save_account(struct account_data *account);

/* External functions */
void Mob_crashload(struct char_data * ch, struct obj_file_elem object);
void House_load(struct obj_file_elem object);
void Crash_load(struct char_data * ch, struct obj_file_elem object);
void House_listrent(struct char_data * ch, struct obj_file_elem object);
void Crash_listrent(struct char_data * ch, struct obj_file_elem object);
int real_zone(int number);
SPECIAL(shop_keeper);
struct account_data *load_account(char *name);

/* External Variables */
extern const char *sql_database;
extern MYSQL *darkpawns;
extern long top_idnum;
extern struct shop_data *shop_index;
extern struct ban_list_element *ban_list;
extern struct shop_cost_data shop_cost[];

void boot_mysql(void)
{
  /* Initialise the server, allocate memory for the database */
  if (!(darkpawns = mysql_init(NULL))) {
    log("[SQL]: Unable to allocate memory for SQL database.");
    exit(1);
  }

  /* Read server settings from file */
  if (mysql_options(darkpawns, MYSQL_READ_DEFAULT_FILE, "database/config.cnf"))
    log("[SQL] Invalid SQL option.");
  if (mysql_options(darkpawns, MYSQL_READ_DEFAULT_GROUP, sql_database))
    log("[SQL] Invalid SQL option.");
  /* Read server settings from file */

  if (!mysql_real_connect(darkpawns, NULL, NULL, NULL, NULL, 0, NULL, 0)) {
    log("[SQL]: Unable to connect to the SQL server.");
    log(mysql_error(darkpawns));
    exit(1);
  }
}


MYSQL_RES *sql_query(MYSQL *db, const char *query, bool retval)
{
  MYSQL_RES *result = NULL;

  if (mysql_real_query(db, query, strlen(query))) {
    sprintf(buf, "[SQL]: Database query failed: [%s]", query);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return (NULL);
  }

  if (mysql_field_count(db) > 0)
    if (!(result = mysql_store_result(db))) {
      sprintf(buf, "[SQL]: No result from database query: [%s]", query);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      return (NULL);
    }

  if (retval)
    return (result);
  else {
    mysql_free_result(result);
    return (NULL);
  }
}

void sql_remove_deleted_players(void)
{
  struct char_data *cbuf;
  struct account_data *account;
  int i, j, pos, count;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  for (i = 0, count = 0; i <= top_of_p_table; i++) {
    if ((cbuf = load_char((player_table + i)->name))) {
      if (PLR_FLAGGED(cbuf, PLR_DELETED)) {     /* Marked for deletion */
        count++;
        sql_Crash_delete_file(GET_IDNUM(cbuf));
        sql_delete_char(cbuf);

        sprintf(buf, "SELECT name FROM accounts WHERE char_one = %ld OR char_two = %ld OR char_three = %ld",
          GET_IDNUM(cbuf), GET_IDNUM(cbuf), GET_IDNUM(cbuf));
        if ((table = sql_query(darkpawns, buf, TRUE)))
          if ((entry = mysql_fetch_row(table)))
            if ((account = load_account(entry[0]))) {
              for (j = 0, pos = 0; j < MAX_CHARS; j++)
                if (account->chars[j] == GET_IDNUM(cbuf))
                  pos = j;

              /* Shift the remaining account chars along in the array */
              for (j = pos; j < (MAX_CHARS - 1); j++)
                account->chars[j] = account->chars[j + 1];

              /* Clear the last one */
              account->chars[--(account->num_chars)] = 0;
              sql_save_account(account);
              free_account(account);
            }

        if (table)
          mysql_free_result(table);
      }
      free_char(cbuf);
      cbuf = NULL;
    }
  }

  if (count) {
    sprintf(buf, "   %d players deleted.", count);
    log(buf);
  }
}

void sql_build_player_index(void)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;
  int players = 0, nr = -1, i, id, unused;
  extern long *unused_ids;

  /* Firstly, determine the number of entries in the player table */
  if ((table = sql_query(darkpawns, "SELECT * FROM players ORDER BY id", TRUE)))
    players = (int)mysql_num_rows(table);

  if (players) {
    sprintf(buf, "   %d players in database.", players);
    log(buf);
    CREATE(player_table, struct player_index_element, players);
  } else {
    player_table = NULL;
    top_of_p_table = -1;
    return;
  }

  while ((entry = mysql_fetch_row(table))) {
    nr++;
    CREATE(player_table[nr].name, char, strlen(entry[1]) + 1);
    for (i = 0; (*(player_table[nr].name + i) = LOWER(*(entry[1] + i))); i++);
    id = atoi(entry[0]);
    player_table[nr].id = id;
    top_idnum = MAX(top_idnum, player_table[nr].id);
  }

  top_of_p_table = nr;
  mysql_free_result(table);

  for (i = 0, nr = 1, unused = 0; i <= top_of_p_table ; i++, nr++) 
    if ((player_table + i)->id != nr) {
      unused += (player_table + i)->id - nr;
      nr = (player_table + i)->id;
    }
 
  /* We want to find all unused ids, so that we can use them on new char creation */
  CREATE(unused_ids, long, unused + 1);   /* an array of unused id's in the ptable */
  for (i = 0, nr = 1, unused = 0; i <= top_of_p_table ; i++, nr++) 
    if ((player_table + i)->id != nr)
      while ((player_table + i)->id != nr)
        unused_ids[unused++] = nr++;

  unused_ids[unused] = -999;              /* let's mark the end for search purposes */
}

void sql_load_char(long id, struct char_file_u *st)
{
  int i;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  /* Load basic details from the "players" table */
  sprintf(buf1, "SELECT * FROM players WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE))) {
    while ((entry = mysql_fetch_row(table))) {
      st->char_specials_saved.idnum = atoi(entry[0]);
      strcpy(st->name, entry[1]);
      if (entry[2])
        strcpy(st->description, entry[2]);
      if (entry[3])
        strcpy(st->title, entry[3]);
      if (entry[4])
        strcpy(st->prompt, entry[4]);
      st->birth = atoi(entry[5]);
      st->played = atoi(entry[6]);
      if (entry[7])
        strcpy(st->host, entry[7]);
      st->last_logon = atoi(entry[8]);
      if (entry[9])
        st->char_specials_saved.act = asciiflag_conv(entry[9]);
      if (entry[10])
        st->char_specials_saved.affected_by = asciiflag_conv(entry[10]);
      if (entry[11])
        st->player_specials_saved.pref = asciiflag_conv(entry[11]);
      st->player_specials_saved.load_room = atoi(entry[12]);
      st->points.gold = atoi(entry[13]);
      st->points.bank_gold = atoi(entry[14]);
      st->player_specials_saved.olc_zone = atoi(entry[15]);
    }
  }
  mysql_free_result(table);

  /* Load player preferences from the "player_prefs" table */
  sprintf(buf1, "SELECT * FROM player_prefs WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      st->player_specials_saved.infobar = atoi(entry[1]);
      st->player_specials_saved.size = atoi(entry[2]);
      st->player_specials_saved.religion = atoi(entry[3]);
      st->player_specials_saved.mount_vnum = atoi(entry[4]);
      st->player_specials_saved.stable = atoi(entry[5]);
      st->player_specials_saved.rank = atoi(entry[6]);
      st->player_specials_saved.patron = atoi(entry[7]);
      st->player_specials_saved.bind_room = atoi(entry[8]);
      st->player_specials_saved.wimp_level = atoi(entry[9]);
      st->player_specials_saved.freeze_level = atoi(entry[10]);
      st->player_specials_saved.invis_level = atoi(entry[11]);
      st->hometown = atoi(entry[12]);
    }
  mysql_free_result(table);

  /* Load player points/statistics from the "player_stats" table */
  sprintf(buf1, "SELECT * FROM player_stats WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      st->points.mana = atoi(entry[1]);
      st->points.max_mana = atoi(entry[2]);
      st->points.hit = atoi(entry[3]);
      st->points.max_hit = atoi(entry[4]);
      st->points.move = atoi(entry[5]);
      st->points.max_move = atoi(entry[6]);
      st->real_abilities.str = atoi(entry[7]);
      st->real_abilities.intel = atoi(entry[8]);
      st->real_abilities.wis = atoi(entry[9]);
      st->real_abilities.dex = atoi(entry[10]);
      st->real_abilities.con = atoi(entry[11]);
      st->real_abilities.cha = atoi(entry[12]);
      st->player_specials_saved.pkcount = atoi(entry[13]);
      st->player_specials_saved.killcount = atoi(entry[14]);
      st->player_specials_saved.deathcount = atoi(entry[15]);
      st->player_specials_saved.lastdeath = atoi(entry[16]);
      st->player_specials_saved.vitae = atoi(entry[17]);
      st->player_specials_saved.vitae_exp = atoi(entry[18]);
      st->points.exp = atoi(entry[19]);
      st->points.exp_queue = atoi(entry[20]);
      st->weight = atoi(entry[21]);
      st->char_specials_saved.alignment = atoi(entry[22]);
      st->sex = atoi(entry[23]);
      st->level = atoi(entry[24]);
      st->player_specials_saved.stat_points = atoi(entry[25]);
      st->player_specials_saved.group_points = atoi(entry[26]);
      st->player_specials_saved.tattoo.type = atoi(entry[27]);
      if (entry[28])
        st->player_specials_saved.tattoo.dyes = asciiflag_conv(entry[28]);
    }
  mysql_free_result(table);

  /* Load player race and racial hatreds from the "player_race" table */
  sprintf(buf1, "SELECT * FROM player_race WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      st->player_specials_saved.race = atoi(entry[1]);
      strcpy(st->race_kills, entry[2]);
    }
  mysql_free_result(table);

  /* Load player timer data from the "player_timers" table */
  sprintf(buf1, "SELECT * FROM player_timers WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      for (i = 0; i < MAX_COND; i++)
        st->player_specials_saved.timer.conditions[i] = atoi(entry[i + 1]);
      st->player_specials_saved.timer.outlaw = atoi(entry[MAX_COND + 1]);
      st->player_specials_saved.timer.jail = atoi(entry[MAX_COND + 2]);
      st->player_specials_saved.timer.mount = atoi(entry[MAX_COND + 3]);
    }
  mysql_free_result(table);

  /* Load player's squelch list from the "player_squelch" table */
  sprintf(buf1, "SELECT * FROM player_squelch WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table)))
      for (i = 0; i < MAX_SQUELCH; i++)
        st->player_specials_saved.squelch[i] = atoi(entry[i + 1]);
  mysql_free_result(table);

  /* Load player affects from the "player_affected" table */
  i = 0;
  sprintf(buf1, "SELECT * FROM player_affected WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      st->affected[i].type = atoi(entry[1]);
      st->affected[i].duration = atoi(entry[2]);
      st->affected[i].modifier = atoi(entry[3]);
      st->affected[i].location = atoi(entry[4]);
      if (entry[5])
        st->affected[i].bitvector = asciiflag_conv(entry[5]);
      i++;
    }
  mysql_free_result(table);

  /* Load player skill groups and levels from the "player_skills" table */
  sprintf(buf1, "SELECT * FROM player_skills WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      i = atoi(entry[1]);
      st->skills.group_level[i] = atoi(entry[2]);
      st->skills.group_skill[i] = atoi(entry[3]);
      st->player_specials_saved.timer.improve[i] = atoi(entry[4]);
    }
  mysql_free_result(table);

  /* Load player's known spells from the "player_spells" table */
  sprintf(buf1, "SELECT * FROM player_spells WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      strcpy(st->skills.known, entry[1]);

  mysql_free_result(table);
}

bool sql_load_account(char *name, struct account_data *account)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM accounts WHERE name = '%s'", name);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table))) {
      account->name = str_dup(name);
      account->pwd = str_dup(entry[1]);
      account->bad_pws = atoi(entry[2]);
      account->num_chars = atoi(entry[3]);
      account->chars[0] = atoi(entry[4]);
      account->chars[1] = atoi(entry[5]);
      account->chars[2] = atoi(entry[6]);
    }

  mysql_free_result(table);
  if (account->name)
    return (TRUE);

  return (FALSE);
}

struct account_data *sql_find_account(long id)
{
  struct account_data *account = NULL;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * from accounts WHERE char_one = %ld OR char_two = %ld OR char_three = %ld",
    id, id, id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table))) {
      CREATE(account, struct account_data, 1);
      account->name = str_dup(entry[0]);
      account->pwd = str_dup(entry[1]);
      account->bad_pws = atoi(entry[2]);
      account->num_chars = atoi(entry[3]);
      account->chars[0] = atoi(entry[4]);
      account->chars[1] = atoi(entry[5]);
      account->chars[2] = atoi(entry[6]);
    }

  mysql_free_result(table);
  return (account);
}

void sql_integrate_shops(void)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;
  struct shop_data *shop;
  int shop_nr, type;
  extern void create_shop(struct shop_data *shop, long copy_room, int type);

  sprintf(buf1, "SELECT * FROM player_shops");
  if ((table = sql_query(darkpawns, buf1, TRUE))) {
    if ((entry = mysql_fetch_row(table))) {
      shop_nr = atoi(entry[0]);
      shop = shop_index + shop_nr;

      SHOP_KEEPER(shop_nr) = atoi(entry[1]);
      shop->copy_room = atoi(entry[2]);
      type = atoi(entry[3]);
      shop->profit_buy = atoi(entry[4]);
      shop->profit_sell = atoi(entry[5]);
      shop->item_limit = atoi(entry[6]);
      shop->open1 = atoi(entry[7]);
      shop->close1 = atoi(entry[8]);

      FREE(shop->owner);
      shop->owner = str_dup(get_name_by_id(atoi(entry[9])));

      create_shop(shop, shop->copy_room, type);
    }
  }

  mysql_free_result(table);
}


void sql_load_shop(struct char_data *mob)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;
  int shop_nr;
  extern int top_shop;

  if (GET_MOB_SPEC(mob) != shop_keeper)       /* So we don't get miscellaneous other SPECs */
    return;

  for (shop_nr = 0; shop_nr <= top_shop; shop_nr++)
    if (SHOP_KEEPER(shop_nr) == mob->nr)
      break;

  sprintf(buf1, "SELECT * FROM shops WHERE uuid = '%s'", GET_UUID(mob));
  if ((table = sql_query(darkpawns, buf1, TRUE))) {
    if ((entry = mysql_fetch_row(table))) {
      GET_GOLD(mob) = atoi(entry[1]);
      switch (atoi(entry[2])) {
        case 0:
          break;
        case 1:
          SET_BIT(MOB_FLAGS(mob), MOB_WARN1);
          break;
        case 2:
          SET_BIT(MOB_FLAGS(mob), MOB_WARN2);
          break;
        default:
          sprintf(buf, "[SQL]: Invalid mob warning flag read from shop table.");
          mudlog(buf, BRF, LVL_IMMORT, TRUE);
          break;
      }
      GET_MOB_TIMER(mob) = atoi(entry[3]);
      SHOP_BANK(shop_nr) = atoi(entry[6]);
    }
  } else {
    sprintf(buf, "[SQL]: Shop keeper #%d cannot be found when attempting to load.",
      GET_MOB_VNUM(mob));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
  }
  mysql_free_result(table);
}


void sql_load_mobs(int zone)
{
  struct char_data *mob;
  struct obj_file_elem object;
  long bottom, top;
  char *alias, *short_desc, *long_desc;
  MYSQL_RES *mob_table, *mobsaves_table, *mobobj_table, *obj_table;
  MYSQL_ROW mob_entry, mobsaves_entry, mobobj_entry, obj_entry;
  int is_shopkeeper(struct char_data * chChar);

  bottom = zone_table[zone].number * 100;
  top = zone_table[zone].top;

  sprintf(buf1, "SELECT * FROM mobs WHERE room >= %ld AND room <= %ld", bottom, top);
  if ((mob_table = sql_query(darkpawns, buf1, TRUE))) {
    while ((mob_entry = mysql_fetch_row(mob_table))) {
      mob = read_mobile(atoi(mob_entry[1]), VIRTUAL, mob_entry[0]);
      if (mob) {
        char_to_room(mob, real_room(atoi(mob_entry[2])));
        GET_LOADED_IN(mob) = atoi(mob_entry[2]);
        REMOVE_BIT(MOB_FLAGS(mob), MOB_UNLOAD);
        GET_HIT(mob) = atoi(mob_entry[3]);
        if (GET_HIT(mob) > GET_MAX_HIT(mob))
          GET_MAX_HIT(mob) = GET_HIT(mob);

        /* Check for saved string details */
        sprintf(buf1, "SELECT * FROM mob_saves WHERE uuid = '%s'", GET_UUID(mob));
        if ((mobsaves_table = sql_query(darkpawns, buf1, TRUE)))
          while ((mobsaves_entry = mysql_fetch_row(mobsaves_table))) {
            alias = str_dup(mobsaves_entry[1]);
            short_desc = str_dup(mobsaves_entry[2]);
            long_desc = str_dup(mobsaves_entry[3]);
            mob->player.name = alias;
            mob->player.short_descr = short_desc;
            mob->player.long_descr = long_desc;
          }
        mysql_free_result(mobsaves_table);

        /* Select all objects from 'mobobjs' corresponding to the mob's uuid */
        sprintf(buf1, "SELECT * FROM mobobjs WHERE uuid = '%s' ORDER BY location,vnum", GET_UUID(mob));
        if ((mobobj_table = sql_query(darkpawns, buf1, TRUE)))
          while ((mobobj_entry = mysql_fetch_row(mobobj_table))) {
            clear_sql_object(&object);
            object.id = str_dup(mobobj_entry[1]);

            /* Now search for amended details in 'objs' */
            sprintf(buf1, "SELECT * from objs WHERE uuid = '%s'", object.id);
            if ((obj_table = sql_query(darkpawns, buf1, TRUE)))
              if ((obj_entry = mysql_fetch_row(obj_table))) {
                object = sql_read_object(obj_entry);
                object.altered = TRUE;
              }
            mysql_free_result(obj_table);
            object.item_number = atoi(mobobj_entry[2]);
            object.locate = atoi(mobobj_entry[3]);
            Mob_crashload(mob, object);
          }
        mysql_free_result(mobobj_table);

        if (is_shopkeeper(mob))
          sql_load_shop(mob);
      }
    }
  }
  mysql_free_result(mob_table);
}


void sql_load_objs(int zone)
{
  struct obj_file_elem object;
  MYSQL_RES *houseobj_table, *obj_table;
  MYSQL_ROW houseobj_entry, obj_entry;
  long bottom, top;

  bottom = zone_table[zone].number * 100;
  top = zone_table[zone].top;

  sprintf(buf1, "SELECT * FROM houseobjs WHERE room_vnum >= %ld AND room_vnum <= %ld"
    " ORDER BY location", bottom, top);
  if ((houseobj_table = sql_query(darkpawns, buf1, TRUE))) {
    while ((houseobj_entry = mysql_fetch_row(houseobj_table))) {
      clear_sql_object(&object);
      object.id = str_dup(houseobj_entry[1]);

      /* Now search for amended details in 'objs' */
      sprintf(buf1, "SELECT * from objs WHERE uuid = '%s'", object.id);
      if ((obj_table = sql_query(darkpawns, buf1, TRUE)))
        if ((obj_entry = mysql_fetch_row(obj_table))) {
          object = sql_read_object(obj_entry);
          object.altered = TRUE;
        }
      mysql_free_result(obj_table);
      object.house = atoi(houseobj_entry[0]);
      object.item_number = atoi(houseobj_entry[2]);
      object.locate = atoi(houseobj_entry[3]);
      House_load(object);
    }
  }
  mysql_free_result(houseobj_table);
}


void sql_load_banned(void)
{
  struct ban_list_element *next_node;
  MYSQL_RES *ban_table;
  MYSQL_ROW ban_entry;

  ban_list = 0;

  /* Load all of the banned site information */
  sprintf(buf1, "SELECT * FROM ban");
  if ((ban_table = sql_query(darkpawns, buf1, TRUE)))
    while ((ban_entry = mysql_fetch_row(ban_table))) {
      CREATE(next_node, struct ban_list_element, 1);
      strncpy(next_node->site, ban_entry[0], BANNED_SITE_LENGTH);
      next_node->site[BANNED_SITE_LENGTH] = '\0';
      next_node->type = atoi(ban_entry[1]);
      next_node->date = atoi(ban_entry[2]);
      next_node->length = atoi(ban_entry[3]);
      strncpy(next_node->account, ban_entry[4], MAX_NAME_LENGTH);
      next_node->account[MAX_NAME_LENGTH] = '\0';
      strncpy(next_node->name, ban_entry[5], MAX_NAME_LENGTH);
      next_node->name[MAX_NAME_LENGTH] = '\0';
      strncpy(next_node->comment, ban_entry[6], BANNED_COMMENT_LENGTH);
      next_node->comment[BANNED_COMMENT_LENGTH] = '\0';

      next_node->next = ban_list;
      ban_list = next_node;
    }
}


void sql_save_char(struct char_file_u *st)
{
  struct affected_type *af;
  char act[10], affects[10], prefs[20], bitvector[10];
  int i;

  extern char *replace_quotes(char *text);

  *buf = '\0';
  if (!st->name)
    sprintf(buf, "[SQL]: Attempting to insert a character into the players table without a name.");
  else if (!st->char_specials_saved.idnum)
    sprintf(buf, "[SQL]: Attempting to insert a character into the players table without an idnum.");

  if (*buf) {
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  if (st->char_specials_saved.act)
    sprintascii(act, st->char_specials_saved.act);
  else
    strcpy(act, "\0");
  if (st->char_specials_saved.affected_by)
    sprintascii(affects, st->char_specials_saved.affected_by);
  else
    strcpy(affects, "\0");
  if (st->player_specials_saved.pref)
    sprintascii(prefs, st->player_specials_saved.pref);
  else
    strcpy(prefs, "\0");

  /* Players table format [id, name, description, title, birth, played, host, last_logon,
  act_flags, affected_by, prefs, load_room, gold, bank gold, olc_zone] */
  sprintf(buf1, "%ld, '%s', %s, %s, '%s', %ld, %d, '%s', %ld, %s, %s, %s, %ld, "
    "%d, %d, %d",
    st->char_specials_saved.idnum,
    st->name,
    st->description ? tprintf("\"%s\"", st->description) : NULL,
    st->title ? tprintf("\"%s\"", replace_quotes(st->title)) : NULL,
    st->prompt,
    st->birth,
    st->played,
    st->host,
    st->last_logon,
    *act ? tprintf("'%s'", act) : NULL,
    *affects ? tprintf("'%s'", affects) : NULL,
    *prefs ? tprintf("'%s'", prefs) : NULL,
    st->player_specials_saved.load_room,
    st->points.gold,
    st->points.bank_gold,
    st->player_specials_saved.olc_zone);
  sprintf(buf2, "REPLACE players VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Player preferences table format [id, infobar, size, religion, mount_vnum, stable, rank,
  patron, bind_room, wimp_level, freeze_level, invis_level, hometown] */
  sprintf(buf1, "%ld, %d, %d, %d, %d, %ld, %d, %ld, %ld, %d, %d, %d, %d",
    st->char_specials_saved.idnum,
    st->player_specials_saved.infobar,
    st->player_specials_saved.size,
    st->player_specials_saved.religion,
    st->player_specials_saved.mount_vnum,
    st->player_specials_saved.stable,
    st->player_specials_saved.rank,
    st->player_specials_saved.patron,
    st->player_specials_saved.bind_room,
    st->player_specials_saved.wimp_level,
    st->player_specials_saved.freeze_level,
    st->player_specials_saved.invis_level,
    st->hometown);
  sprintf(buf2, "REPLACE player_prefs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Player points/statistics table format [id, mana, max mana, hit, max hit, move, max move,
  real str, intel, wis, dex, con, cha, pkcount, killcount, deathcount, last death, vitae,
  vitae exp, exp, exp queue, weight, alignment, sex, level, stat points, group points,
  tattoo type, dyes used */

  if (st->player_specials_saved.tattoo.dyes)
    sprintascii(bitvector, st->player_specials_saved.tattoo.dyes);
  else
    strcpy(bitvector, "\0");

  sprintf(buf1, "%ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %ld, "
    "%d, %ld, %ld, %ld, %d, %d, %d, %d, %d, %d, %d, %s",
    st->char_specials_saved.idnum,
    st->points.mana,
    st->points.max_mana,
    st->points.hit,
    st->points.max_hit,
    st->points.move,
    st->points.max_move,
    st->real_abilities.str,
    st->real_abilities.intel,
    st->real_abilities.wis,
    st->real_abilities.dex,
    st->real_abilities.con,
    st->real_abilities.cha,
    st->player_specials_saved.pkcount,
    st->player_specials_saved.killcount,
    st->player_specials_saved.deathcount,
    st->player_specials_saved.lastdeath,
    st->player_specials_saved.vitae,
    st->player_specials_saved.vitae_exp,
    st->points.exp,
    st->points.exp_queue,
    st->weight,
    st->char_specials_saved.alignment,
    st->sex,
    st->level,
    st->player_specials_saved.stat_points,
    st->player_specials_saved.group_points,
    st->player_specials_saved.tattoo.type,
    *bitvector ? tprintf("'%s'", bitvector) : NULL);
  sprintf(buf2, "REPLACE player_stats VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Player race and racial hatreds table format [id, race, race kills] */
  sprintf(buf1, "%ld, %d, '%s'",
    st->char_specials_saved.idnum,
    st->player_specials_saved.race,
    st->race_kills);
  sprintf(buf2, "REPLACE player_race VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Player timers table format [id, drunk, full, thirsty, tired, outlaw, jail, mount */
  *buf = '\0';
  for (i = 0; i < MAX_COND; i++) {
    sprintf(buf1, "%d, ", st->player_specials_saved.timer.conditions[i]);
    strcat(buf, buf1);
  }

  sprintf(buf1, "%ld, %s%d, %d, %ld",
    st->char_specials_saved.idnum,
    buf,
    st->player_specials_saved.timer.outlaw,
    st->player_specials_saved.timer.jail,
    st->player_specials_saved.timer.mount);
  sprintf(buf2, "REPLACE player_timers VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);


  /* Player's squelch table format [id, squelch id's 1 through 5] */
  *buf = '\0';
  for (i = 0; i < MAX_SQUELCH; i++) {
    sprintf(buf1, "%ld, ", st->player_specials_saved.squelch[i]);
    strcat(buf, buf1);
  }

  *(buf + strlen(buf) - 2) = '\0';
  sprintf(buf1, "%ld, %s", st->char_specials_saved.idnum, buf);
  sprintf(buf2, "REPLACE player_squelch VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Player's affections table format [id, type, duration, modifier, location, bitvector] */
  for (i = 0; i < MAX_AFFECT; i++)
    for (af = st->affected; af; af = af->next)
      if (af[i].type) {
        if (st->affected->bitvector > 0)
          sprintascii(bitvector, st->affected[i].bitvector);
        else
          strcpy(bitvector, "\0");
        sprintf(buf1, "%ld, %d, %d, %d, %d, %s",
          st->char_specials_saved.idnum,
          af[i].type,
          af[i].duration,
          af[i].modifier,
          af[i].location,
          *bitvector ? tprintf("'%s'", bitvector) : NULL);
        sprintf(buf2, "REPLACE player_affected VALUES (%s)", buf1);
        sql_query(darkpawns, buf2, FALSE);
      }

  /* Player's skills table format [id, group, group level, group skill] */
  for (i = 1; i <= MAX_SKILL_GROUPS; i++) {
    sprintf(buf1, "%ld, %d, %d, %d, %d",
      st->char_specials_saved.idnum,
      i,
      st->skills.group_level[i],
      st->skills.group_skill[i],
      st->player_specials_saved.timer.improve[i]);
    sprintf(buf2, "REPLACE player_skills VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }

  /* Player's known spells format table [id, spell number, known] */
  sprintf(buf1, "%ld, '%s'",
    st->char_specials_saved.idnum,
    st->skills.known);

  sprintf(buf2, "REPLACE player_spells VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_save_account(struct account_data *account)
{
  /* Accounts table format [name, password, bad passwords, num of chars, character ids 1 - 3] */
  sprintf(buf1, "'%s', '%s', %d, %d, %ld, %ld, %ld",
    account->name,
    account->pwd,
    account->bad_pws,
    account->num_chars,
    account->chars[0],
    account->chars[1],
    account->chars[2]);
  sprintf(buf2, "REPLACE accounts VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_save_shop(int shop_nr)
{
  struct char_data *mob;
  struct obj_data *deed = NULL;
  struct shop_data *shop;
  char id[36];
  bool owned;

  /* If the shop keeper isn't in the world, then we can't save him */
  if (!(mob = get_char_num(SHOP_KEEPER(shop_nr))))
    return;

  if (!IS_NPC(mob))
    return;

  *buf = '\0';
  if (!GET_UUID(mob))
    sprintf(buf, "[SQL]: Attempting to insert '%s' into shop table without a mob uuid.",
      GET_NAME(mob));

  if (*buf) {
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  owned = get_id_by_name(SHOP_OWNER(shop_nr));
  if (owned > 0)
    deed = get_obj_vis(mob, "deed");

  if (deed)
    strcpy(id, deed->id);
  else
    strcpy(id, "\0");

  /* Shops table format [uuid, gold, warned flag, timer, player owned, deed, bank] */
  sprintf(buf1, "'%s', %d, %d, %d, %d, %s, %d",
    GET_UUID(mob),
    GET_GOLD(mob),
    MOB_FLAGGED(mob, MOB_WARN1) ? 1 : (MOB_FLAGGED(mob, MOB_WARN2) ? 2 : 0),
    GET_MOB_TIMER(mob),
    owned > 0 ? TRUE : FALSE,
    *id ? tprintf("'%s'", id) : NULL,
    SHOP_BANK(shop_nr));

  sprintf(buf2, "REPLACE shops VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  if (owned > 0) {
    shop = shop_index + shop_nr;
    /* player_shops format [shop_nr, keeper, room, type, buy_profit, sell_profit, item_limit, open, close, id] */
    sprintf(buf1, "%d, %ld, %ld, %d, %4.2f, %4.2f, %d, %d, %d, %d",
      shop_nr,
      SHOP_KEEPER(shop_nr),
      shop->copy_room,
      shop->shop_type,
      shop->profit_buy,
      shop->profit_sell,
      shop->item_limit,
      shop->open1,
      shop->close1,
      get_id_by_name(shop->owner));

    sprintf(buf2, "REPLACE player_shops VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }
}


void sql_save_banned(void)
{
  struct ban_list_element *ban_node;

  /* Ban table format [site, type, date, length, account, name, comment] */
  for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
    sprintf(buf1, "'%s', %d, %ld, %d, '%s', '%s', '%s'",
      ban_node->site,
      ban_node->type,
      ban_node->date,
      ban_node->length,
      ban_node->account,
      ban_node->name,
      ban_node->comment);

    sprintf(buf2, "REPLACE ban VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }
}


void sql_delete_account(char *name)
{
  sprintf(buf1, "DELETE FROM accounts WHERE name = '%s'", name);
  sql_query(darkpawns, buf1, FALSE);
}


void sql_delete_ban(char *name, bool account)
{
  if (account)
    sprintf(buf1, "DELETE FROM ban WHERE account = '%s'", name);
  else
    sprintf(buf1, "DELETE FROM ban WHERE site = '%s'", name);
  sql_query(darkpawns, buf1, FALSE);
}


void sql_delete_char(struct char_data *ch)
{
  int i, pos;

 static char *name[] =
    { "player_affected", "player_prefs", "player_race", "player_rent", "player_skills",
      "player_squelch", "player_stats", "player_timers", "players", "plrobjs",
      "player_spells", "aliases", "notes", "poofs", "player_shops", "mail", "\n" };

  long get_ptable_by_name(char *name);

  for (i = 0; strcmp(name[i], "\n"); i++) {
    sprintf(buf1, "DELETE FROM %s WHERE id = %ld", name[i], GET_IDNUM(ch));
    sql_query(darkpawns, buf1, FALSE);
  }

  /* Now, we effectively remove the player from the player table */
  if ((pos = get_ptable_by_name(GET_NAME(ch))) != -1)
    strcpy(player_table[pos].name, "<deleted>");
}

void sql_add_plrobjs(struct obj_file_elem object)
{
  struct char_data *ch = object.owner;
  *buf = '\0';

  if (!object.id)
    sprintf(buf, "[SQL]: Attempting to insert '%s' into plrobjs table without an object uuid.",
      GET_NAME(ch));
  else if (!object.item_number)
    sprintf(buf, "[SQL]: Attempting to insert '%s' into plrobjs table without an object vnum.",
      GET_NAME(ch));

  if (*buf) {
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* Table format [plr id, obj uuid, obj vnum, obj location] */
  sprintf(buf1, "%ld, '%s', %d, %d", GET_IDNUM(ch), object.id, object.item_number, 
object.locate);
  sprintf(buf2, "REPLACE plrobjs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_add_mobobjs(struct obj_file_elem object)
{
  struct char_data *mob = object.owner;
  *buf = '\0';

  if (!GET_UUID(mob))
    sprintf(buf, "[SQL]: Attempting to insert '%s' into mobobjs table without a uuid.",
      mob->player.name);
  else if (!object.id)
    sprintf(buf, "[SQL]: Attempting to insert '%s' into mobobjs table without an object uuid.",
      mob->player.name);
  else if (!object.item_number)
    sprintf(buf, "[SQL]: Attempting to insert '%s' into mobobjs table without an object vnum.",
      mob->player.name);

  if (*buf) {
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* Table format [mob uuid, obj uuid, obj vnum, obj location] */
  sprintf(buf1, "'%s', '%s', %d, %d", GET_UUID(mob), object.id, object.item_number, 
object.locate);
  sprintf(buf2, "REPLACE mobobjs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_add_houseobjs(struct obj_file_elem object)
{
  long vnum = object.house;
  *buf = '\0';

  if (!vnum)
    sprintf(buf, "[SQL]: Attempting to insert object #%d into houseobjs table without a room vnum.",
      object.item_number);
  else if (!object.id)
    sprintf(buf, "[SQL]: Attempting to insert object #%d into houseobjs table without a uuid.",
      object.item_number);

  if (*buf) {
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* Table format [room vnum, obj uuid, obj vnum, obj location] */
  sprintf(buf1, "%ld, '%s', %d, %d", vnum, object.id, object.item_number, object.locate);
  sprintf(buf2, "REPLACE houseobjs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_add_mob(struct char_data *mob)
{
  bool altered = FALSE;
  long rnum = GET_MOB_RNUM(mob);

  *buf = '\0';

  if (!GET_UUID(mob))
    sprintf(buf, "[SQL]: Attempting to insert '%s' into mobile table without a uuid.",
      mob->player.name);
  else if (GET_MOB_VNUM(mob) == NOBODY)
    sprintf(buf, "[SQL]: Attempting to insert '%s' into mobile table without a vnum.",
      mob->player.name);
  else if (GET_ROOM_VNUM(mob->in_room) == NOWHERE)
    sprintf(buf, "[SQL]: Attempting to insert '%s' into mobile table without a room.",
      mob->player.name);

  if (*buf) {
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* Table format [uuid, mob vnum, mob's room vnum, mob hitpoints] */
  sprintf(buf1, "'%s', %d, %ld, %ld", GET_UUID(mob), GET_MOB_VNUM(mob),
    GET_ROOM_VNUM(mob->in_room), GET_HIT(mob));
  sprintf(buf2, "REPLACE mobs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Determine if mob is different than the proto in some aspect */
  if (mob->player.name != mob_proto[rnum].player.name)
    altered = TRUE;
  else if (mob->player.short_descr != mob_proto[rnum].player.short_descr)
    altered = TRUE;
  else if (mob->player.long_descr != mob_proto[rnum].player.long_descr)
    altered = TRUE;

  if (altered) {
    /* Table format [uuid, alias, short desc, long desc] */
    sprintf(buf1, "'%s', '%s', '%s', '%s'", GET_UUID(mob), mob->player.name,
      mob->player.short_descr, mob->player.long_descr);
    sprintf(buf2, "REPLACE mob_saves VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }
}


void sql_add_obj(struct obj_file_elem object)
{
  int i;
  struct extra_descr_data *temp;
  char extra[40], wear[40], bitvector[40];

  /* Determine where the object is to be stored, and make sure it's in the tables */
  if (object.house)
    sql_add_houseobjs(object);
  else if (object.owner) {
    if (IS_NPC(object.owner)) {
      sql_add_mob(object.owner);
      sql_add_mobobjs(object);
    } else
      sql_add_plrobjs(object);
  } else {
    sprintf(buf, "[SQL]: Attempting to insert object #%d into a table without a house or owner.",
      object.item_number);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* If the object isn't different from the proto, no need to store it */
  if (!object.altered)
    return;

  if (object.extra_flags)
    sprintascii(extra, object.extra_flags);
  else
    strcpy(extra, "\0");
  if (object.wear_flags)
    sprintascii(wear, object.wear_flags);
  else
    strcpy(wear, "\0");
  if (object.bitvector)
    sprintascii(bitvector, object.bitvector);
  else
    strcpy(bitvector, "\0");

  /* Main object table, 'objs' */
  sprintf(buf1,
    "'%s',"   /* object uuid */
    "%d,"     /* object weight */
    "%d,"     /* object timer */
    "%s,"     /* object extra flags */
    "%s,"     /* object wear flags */
    "%s,"     /* object bitvector flags */
    "%s,"     /* object name */
    "%s,"     /* object description */
    "%s",     /* object short description */

    object.id,
    object.weight,
    object.timer,
    *extra ? tprintf("'%s'", extra) : NULL,
    *wear ? tprintf("'%s'", wear) : NULL,
    *bitvector ? tprintf("'%s'", bitvector) : NULL,
    *object.name ? tprintf("\"%s\"", object.name) : NULL,
    *object.desc ? tprintf("\"%s\"", object.desc) : NULL,
    *object.shortd ? tprintf("\"%s\"", object.shortd) : NULL);
    
  sprintf(buf2, "REPLACE objs VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Object affects table, 'obj_affects' */
  /* Table format [uuid, location, modifier] */
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    sprintf(buf1, "'%s', %d, %d",
      object.id, object.affected[i].location, object.affected[i].modifier);
    sprintf(buf2, "REPLACE obj_affects VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }

  /* Object requirements table, 'obj_reqs' */
  /* Table format [uuid, location, modifier] */
  for (i = 0; i < MAX_OBJ_REQS; i++) {
    sprintf(buf1, "'%s', %d, %d",
      object.id, object.reqs[i].location, object.reqs[i].modifier);
    sprintf(buf2, "REPLACE obj_reqs VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }

  /* Object value table, 'obj_value' */
  /* Table format [uuid, val_zero, val_one, val_two, val_three, val_four, obj cost] */
  sprintf(buf1, "'%s', %ld, %ld, %ld, %ld, %ld, %ld", object.id, object.value[0], object.value[1],
    object.value[2], object.value[3], object.value[4], object.cost);
  sprintf(buf2, "REPLACE obj_value VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);

  /* Object extra description table, 'obj_extra_desc' */
  /* Table format [uuid, keyword, description] */
  for (temp = object.extra_descr; temp; temp = temp->next) {
    sprintf(buf1, "'%s', \"%s\", \"%s\"", object.id, temp->keyword, temp->description);
    sprintf(buf2, "REPLACE obj_extra_desc VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }

  /* Object action decription table, 'obj_action_desc' */
  /* Table format [uuid, description] */
  if (object.action_descr) {
    sprintf(buf1, "'%s', \"%s\"", object.id, object.action_descr);
    sprintf(buf2, "REPLACE obj_action_desc VALUES (%s)", buf1);
    sql_query(darkpawns, buf2, FALSE);
  }
}


void sql_add_container(struct obj_data *obj)
{
  /* Table format [object uuid, container uuid] */
  sprintf(buf1, "'%s', '%s'", obj->id, obj->in_obj->id);
  sprintf(buf2, "REPLACE obj_container VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_remove_mob(struct char_data *mob)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if (!GET_UUID(mob)) {
    sprintf(buf, "[SQL]: Attempting to delete '%s' from mobile table without a uuid.",
      mob->player.name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* Delete associated objects from the mobobjs and obj tables */
  sprintf(buf1, "SELECT obj_id FROM mobobjs WHERE uuid = '%s'", GET_UUID(mob));
  if ((table = sql_query(darkpawns, buf1, TRUE))) {
    while ((entry = mysql_fetch_row(table))) {
      sprintf(buf2, "DELETE FROM objs WHERE uuid = '%s'", entry[0]);
      sprintf(buf3, "DELETE FROM mobobjs WHERE uuid = '%s'", GET_UUID(mob));
      sql_query(darkpawns, buf2, FALSE);
      sql_query(darkpawns, buf3, FALSE);
    }
  }
  mysql_free_result(table);

  /* Delete from the mobs table */
  sprintf(buf1, "DELETE FROM mobs WHERE uuid = '%s'", GET_UUID(mob));
  sql_query(darkpawns, buf1, FALSE);

  /* Delete from the mob_saves table */
  sprintf(buf1, "DELETE FROM mob_saves WHERE uuid = '%s'", GET_UUID(mob));
  sql_query(darkpawns, buf1, FALSE);
}


void sql_remove_obj(struct obj_data *obj)
{
  int i;
  static char *name[] = 
    { "objs", "obj_value", "obj_affects", "obj_reqs", "obj_extra_desc", "obj_container",
      "obj_action_desc", "\n" };
  bool present = FALSE;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if (!obj->id) {
    sprintf(buf, "[SQL]: Attempting to delete '%s' from object table without a uuid.",
      obj->name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  /* Delete the object from the mobobjs table, if it exists */
  sprintf(buf1, "SELECT * FROM mobobjs WHERE obj_id = '%s'", obj->id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      sprintf(buf1, "DELETE FROM mobobjs WHERE obj_id = '%s'", obj->id);
      sql_query(darkpawns, buf1, FALSE);
      present = TRUE;
    }
  mysql_free_result(table);

  /* Delete the object from the houseobjs table, if it exists */
  sprintf(buf1, "SELECT * FROM houseobjs WHERE obj_id = '%s'", obj->id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      sprintf(buf1, "DELETE FROM houseobjs WHERE obj_id = '%s'", obj->id);
      sql_query(darkpawns, buf1, FALSE);
      present = TRUE;
    }
  mysql_free_result(table);

  /* Delete the object from the plrobjs table, if it exists */
  sprintf(buf1, "SELECT * FROM plrobjs WHERE obj_id = '%s'", obj->id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      sprintf(buf1, "DELETE FROM plrobjs WHERE obj_id = '%s'", obj->id);
      sql_query(darkpawns, buf1, FALSE);
      present = TRUE;
    }
  mysql_free_result(table);

  /* Was the object found? If so, remove it from all other tables */
  if (present)
    for (i = 0; strcmp(name[i], "\n"); i++) {
      sprintf(buf2, "DELETE FROM %s WHERE uuid = '%s'", name[i], obj->id);
      sql_query(darkpawns, buf2, FALSE);
    }

  /* Does the object contain anything? If so, remove its contents from the table */
  if (obj->contains) {
    sprintf(buf2, "DELETE FROM obj_container WHERE cont_id = '%s'", obj->id);
    sql_query(darkpawns, buf2, FALSE);
  }
}


void sql_remove_shop(struct char_data *keeper, int shop_nr)
{
  if (!GET_UUID(keeper)) {
    sprintf(buf, "[SQL]: Attempting to delete a shop from the table without a keeper.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  sprintf(buf1, "DELETE FROM shops WHERE uuid = '%s'", GET_UUID(keeper));
  sql_query(darkpawns, buf1, FALSE);

  if (shop_nr <= 0) {
    sprintf(buf, "[SQL]: Attempting to delete a shop from the table without a shop number.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  sprintf(buf1, "DELETE FROM player_shops WHERE shop_nr = %d", shop_nr);
  sql_query(darkpawns, buf1, FALSE);
}

void sql_remove_affect(struct char_data *ch, struct master_affected_type *af)
{
  if (!ch || !af) {
    sprintf(buf, "[SQL]: Attempting to delete a player affect without the char or affect.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  sprintf(buf1, "DELETE FROM player_affected WHERE id = %ld AND type = %d AND location = %d",
    GET_IDNUM(ch), (int)af->type, (int)af->location);
  sql_query(darkpawns, buf1, FALSE);
}


struct obj_file_elem sql_read_object(MYSQL_ROW entry)
{
  struct obj_file_elem object;
  struct extra_descr_data *new_descr;
  int i;
  MYSQL_RES *table;
  MYSQL_ROW field;

  /* Initialise the file element */
  clear_sql_object(&object);

  /* Reading from the 'objs' table */
  object.id = str_dup(entry[0]);
  object.weight = atoi(entry[1]);
  object.timer = atoi(entry[2]);
  if (entry[3])
    object.extra_flags = asciiflag_conv(entry[3]);
  if (entry[4])
    object.wear_flags = asciiflag_conv(entry[4]);
  if (entry[5])
    object.bitvector = asciiflag_conv(entry[5]);
  if (entry[6])
    strcpy(object.name, entry[6]);
  if (entry[7])
    strcpy(object.desc, entry[7]);
  if (entry[8])
    strcpy(object.shortd, entry[8]);

  /* Reading from the 'obj_value' table */
  sprintf(buf1, "SELECT * FROM obj_value WHERE uuid = '%s'", object.id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((field = mysql_fetch_row(table))) {
      for (i = 0; i < 5; i++)
        object.value[i] = atoi(field[i + 1]);
      object.cost = atoi(field[6]);
    }
  mysql_free_result(table);

  /* Reading from the 'obj_affects' table */
  i = 0;
  sprintf(buf1, "SELECT location, modifier FROM obj_affects WHERE uuid = '%s'", object.id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((field = mysql_fetch_row(table))) {
      object.affected[i].location = atoi(field[0]);
      object.affected[i].modifier = atoi(field[1]);
      i++;
    }
  mysql_free_result(table);

  /* Reading from the 'obj_reqs' table */
  i = 0;
  sprintf(buf1, "SELECT location, modifier FROM obj_reqs WHERE uuid = '%s'", object.id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((field = mysql_fetch_row(table))) {
      object.reqs[i].location = atoi(field[0]);
      object.reqs[i].modifier = atoi(field[1]);
      i++;
    }
  mysql_free_result(table);

  /* Reading from the 'obj_extra_desc' table */
  sprintf(buf1, "SELECT keyword, description FROM obj_extra_desc WHERE uuid = '%s'", object.id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((field = mysql_fetch_row(table))) {
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = str_dup(field[0]);
      new_descr->description = str_dup(field[1]);
      new_descr->next = object.extra_descr;
      object.extra_descr = new_descr;
    }
  mysql_free_result(table);

  /* Reading from the 'obj_action_desc' table */
  sprintf(buf1, "SELECT description FROM obj_action_desc WHERE uuid = '%s'", object.id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((field = mysql_fetch_row(table)))
      object.action_descr = str_dup(field[0]);
  mysql_free_result(table);

  return (object);
}


void sql_read_plrobjs(struct char_data *ch)
{
  struct obj_file_elem object;
  struct rent_info rent;
  MYSQL_RES *table, *obj_table;
  MYSQL_ROW entry, obj_entry;

  sprintf(buf1, "SELECT * from plrobjs WHERE id = %ld ORDER BY location", GET_IDNUM(ch));
  if ((table = sql_query(darkpawns, buf1, TRUE))) {
    rent = sql_rent_read(GET_IDNUM(ch));
    switch (rent.rentcode) {
      case RENT_RENTED:
        sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        break;
      case RENT_CRASH:
        sprintf(buf, "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        break;
      case RENT_CRYO:
        sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        break;
      case RENT_FORCED:
        sprintf(buf, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        break;
      case RENT_NEW:
      case RENT_INACTIVE:
        sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        break;
      default:
        sprintf(buf, "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
        break;
    }
    while ((entry = mysql_fetch_row(table))) {
      clear_sql_object(&object);
      object.id = entry[1];

      /* Now search for amended details in 'objs' */
      sprintf(buf1, "SELECT * from objs WHERE uuid = '%s'", object.id);
      if ((obj_table = sql_query(darkpawns, buf1, TRUE)))
        if ((obj_entry = mysql_fetch_row(obj_table))) {
          object = sql_read_object(obj_entry);
          object.altered = TRUE;
        }
      mysql_free_result(obj_table);
      object.item_number = atoi(entry[2]);
      object.locate = atoi(entry[3]);
      Crash_load(ch, object);
    }
  }
  mysql_free_result(table);
}


void sql_House_listrent(struct char_data *ch, long vnum)
{
  struct obj_file_elem object;
  bool records = FALSE;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM houseobjs WHERE room_vnum = %ld", vnum);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      clear_sql_object(&object);
      object.item_number = atoi(entry[2]);
      object.locate = atoi(entry[3]);
      House_listrent(ch, object);
      records = TRUE;
    }
  mysql_free_result(table);
  if (!records) {
    sprintf(buf, "No objects stored for house #%ld.\r\n", vnum);
    send_to_char(buf, ch);
  }
}


void sql_Crash_listrent(struct char_data *ch, char *name)
{
  struct obj_file_elem object;
  bool records = FALSE;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM plrobjs WHERE id = %ld", get_id_by_name(name));
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      clear_sql_object(&object);
      object.item_number = atoi(entry[2]);
      object.locate = atoi(entry[3]);
      records = TRUE;
      Crash_listrent(ch, object);
    }
  mysql_free_result(table);
  if (!records) {
    sprintf(buf, "%s has no rent file.\r\n", name);
    send_to_char(buf, ch);
  }
}


int sql_check_container(struct obj_data *obj, struct obj_data *cont)
{
  bool found = FALSE;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if ((!obj) || (!cont)) {
    sprintf(buf, "SYSERR: No object or container specified in sql_check_container.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return (FALSE);
  }

  sprintf(buf1, "SELECT cont_id FROM obj_container WHERE uuid = '%s'", obj->id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table)))
      if (!strcmp(cont->id, entry[0]))
        found = TRUE;

  mysql_free_result(table);
  if (found)
    return (TRUE);
  return (FALSE);
}

/* Count the number of mobs with vnum "virtual" in the database */
int sql_check_mob(long virtual)
{
  ush_int found = 0;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT COUNT(*) FROM mobs WHERE vnum = %d", virtual);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      found = atoi(entry[0]);


  mysql_free_result(table);
  return (found);
}

/* Count the number of objs with vnum "virtual" in the database (rooms only) */
int sql_check_obj(long virtual, char uuid[36])
{
  ush_int found = 0;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  /* House objects */
  sprintf(buf1, "SELECT COUNT(*) FROM houseobjs WHERE vnum = %d", virtual);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      found = atoi(entry[0]);


  mysql_free_result(table);
  table = NULL;

  /* Mob objects */
  sprintf(buf1, "SELECT COUNT(*) FROM mobobjs WHERE vnum = %d", virtual);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      found += atoi(entry[0]);

  mysql_free_result(table);
  return (found);
}

long sql_find_mob(char *searchname)
{
  int nr;
  long room_vnum = 0;
  MYSQL_RES *table = NULL;
  MYSQL_ROW entry;

  if (!*searchname)
    return 0;

  for (nr = 0; nr <= top_of_mobt; nr++)
    if (isname_with_abbrevs(searchname, mob_proto[nr].player.name)) {
      sprintf(buf1, "SELECT * FROM mobs where vnum = %d", mob_index[nr].virtual);
      if ((table = sql_query(darkpawns, buf1, TRUE)))
        if ((entry = mysql_fetch_row(table))) {
          room_vnum = atoi(entry[2]);
          break;
        }
    }

  if (table)
    mysql_free_result(table);
  return (room_vnum);
}

long sql_find_obj(char *searchname)
{
  long room_vnum = 0;
  char uuid[36];
  MYSQL_RES *table, *house_table;
  MYSQL_ROW entry, house_entry;

  sprintf(buf1, "SELECT * FROM objs WHERE name LIKE '%%%s%%'", searchname);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      strcpy(uuid, entry[0]);
      sprintf(buf2, "SELECT * FROM houseobjs WHERE obj_id = '%s'", uuid);
      if ((house_table = sql_query(darkpawns, buf2, TRUE)))
        if ((house_entry = mysql_fetch_row(house_table))) {
          room_vnum = atoi(house_entry[0]);
          mysql_free_result(house_table);
          break;
        }
      mysql_free_result(house_table);
    }

  mysql_free_result(table);
  return (room_vnum);
}

char *sql_do_where(char *searchname, int num)
{
  int nr;
  struct char_data *mob;
  struct obj_data *obj;
  bool found;
  MYSQL_RES *table = NULL;
  MYSQL_ROW entry;

  if (!*searchname)
    return 0;

  /* Find the mobs from the database first */
  *buf2 = '\0';
  for (nr = 0, found = FALSE; nr <= top_of_mobt; nr++) {
    if (num >= 80)
      break;

    if (isname_with_abbrevs(searchname, mob_proto[nr].player.name)) {
      sprintf(buf1, "SELECT * FROM mobs WHERE vnum = %ld", mob_index[nr].virtual);
      if ((table = sql_query(darkpawns, buf1, TRUE)))
        while ((entry = mysql_fetch_row(table))) {
          for (mob = character_list; mob; mob = mob->next) {
            if (!IS_MOB(mob))
              continue;

            if (!strcmp(GET_UUID(mob), entry[0]))
              found = TRUE;

            if (found)
              break;
          }

          if (!found)
            snprintf(buf2 + strlen(buf2), MAX_STRING_LENGTH, "&RM&n%3d. %-25s - [%5d] %s\r\n",
              ++num, mob_proto[nr].player.short_descr, atoi(entry[2]), world[real_room(atoi(entry[2]))].name);
        }
    }
  }

  if (table) {
    mysql_free_result(table);
    table = NULL;
  }

  /* Now find the house objs */
  for (nr = 0, found = FALSE; nr <= top_of_objt; nr++) {
    if (num >= 100)
      break;

    if (isname_with_abbrevs(searchname, obj_proto[nr].name)) {
      sprintf(buf1, "SELECT * FROM houseobjs WHERE vnum = %ld", obj_index[nr].virtual);
      if ((table = sql_query(darkpawns, buf1, TRUE)))
        while ((entry = mysql_fetch_row(table))) {
          for (obj = object_list; obj; obj = obj->next)
            if (!strcmp(obj->id, entry[1])) {
              found = TRUE;
              break;
            }

          if (!found)
            snprintf(buf2 + strlen(buf2), MAX_STRING_LENGTH, "&RO&n%3d. %-25s - [%5d] %s\r\n",
              ++num, obj_proto[nr].short_description, atoi(entry[0]), world[real_room(atoi(entry[0]))].name);
        }
    }
  }

  if (table) {
    mysql_free_result(table);
    table = NULL;
  }

  /* And lastly, objects carried by mobs */
  for (nr = 0, found = FALSE; nr <= top_of_objt; nr++) {
    if (num >= 120)
      break;

    if (isname_with_abbrevs(searchname, obj_proto[nr].name)) {
      sprintf(buf1, "SELECT mobs.vnum, mobobjs.obj_id FROM mobobjs LEFT JOIN mobs ON mobs.uuid = mobobjs.uuid"
        " WHERE mobobjs.vnum = %ld", obj_index[nr].virtual);
      if ((table = sql_query(darkpawns, buf1, TRUE)))
        while ((entry = mysql_fetch_row(table))) {
          for (obj = object_list; obj; obj = obj->next)
            if (!strcmp(obj->id, entry[1])) {
              found = TRUE;
              break;
            }

          if (!found)
            snprintf(buf2 + strlen(buf2), MAX_STRING_LENGTH, "&RO&n%3d. %-25s - carried by %s\r\n",
              ++num, obj_proto[nr].short_description, mob_proto[real_mobile(atoi(entry[0]))].player.short_descr);
        }
    }
  }

  if (table) {
    mysql_free_result(table);
    table = NULL;
  }

  return (buf2);
}

struct account_data *sql_check_account(char *name)
{
  struct account_data *account = NULL;
  int i;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM accounts WHERE name = '%s'", name);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table))) {
      CREATE(account, struct account_data, 1);
      account->name = str_dup(CAP(name));
      account->pwd = str_dup(entry[1]);
      account->bad_pws = atoi(entry[2]);
      account->num_chars = atoi(entry[3]);
      for (i = 0; i < MAX_CHARS; i++)
        account->chars[i] = atoi(entry[4 + i]);
    }

  mysql_free_result(table);
  return (account);
}

struct rent_info sql_rent_read(long id)
{
  struct rent_info rent;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  sprintf(buf1, "SELECT * FROM player_rent WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table))) {
      rent.rentcode = atoi(entry[1]);
      rent.time = atoi(entry[2]);
    }

  mysql_free_result(table);
  return (rent);
}


void sql_rent_write(long id, ubyte code)
{
  sprintf(buf1, "%ld, %d, %ld", id, code, time(0));
  sprintf(buf2, "REPLACE player_rent VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
}


void sql_shop_boot(void)
{
  int zone = 0, homet;
  bool owned;
  MYSQL_RES *shop_table, *mob_table;
  MYSQL_ROW shop_entry, mob_entry;

  sprintf(buf1, "SELECT uuid, owned FROM shops");
  if ((shop_table = sql_query(darkpawns, buf1, TRUE)))
    while ((shop_entry = mysql_fetch_row(shop_table)))
      if ((owned = atoi(shop_entry[1]))) {
        sprintf(buf1, "SELECT room FROM mobs WHERE uuid = '%s'", shop_entry[0]);
        if ((mob_table = sql_query(darkpawns, buf1, TRUE)))
          if ((mob_entry = mysql_fetch_row(mob_table)))
            zone = real_zone(atoi(mob_entry[0]));
        mysql_free_result(mob_table);
        switch (zone_table[zone].number) {
          case 80:  homet = 0; break;
          case 182: homet = 1; break;
          case 212: homet = 2; break;
          case 48:  homet = 3; break;
          case 53:  homet = 4; break;
          default:  homet = -1; break;
        }
        if (homet >= 0)
          shop_cost[homet].for_sale--;
      }
  mysql_free_result(shop_table);
}


void sql_optimize(void)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if ((table = sql_query(darkpawns, "SHOW TABLES", TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      sprintf(buf1, "OPTIMIZE TABLE %s", entry[0]);
      sql_query(darkpawns, buf1, FALSE);
    }
  mysql_free_result(table);
}


void sql_heap_table(struct char_data *ch)
{
  int i;
  static char *name[] =
    { "objs", "obj_value", "obj_affects", "obj_reqs", "obj_container", "\n" };
  static char *non_heap[] =
    { "obj_extra_desc", "obj_action_desc", "\n" };

  /* Create a temporary table, under the player's name, so that the objects can
  be re-saved after extracted from the game */
  sprintf(buf1, "CREATE TABLE %s TYPE=HEAP SELECT * FROM plrobjs WHERE id = %ld",
    GET_NAME(ch), GET_IDNUM(ch));
  sql_query(darkpawns, buf1, FALSE);

  /* Create temporary tables for all objects so that we save them too */
  for (i = 0; strcmp(name[i], "\n"); i++) {
    sprintf(buf1, "CREATE TABLE %s_%s TYPE=HEAP SELECT %s.* FROM %s, plrobjs WHERE"
      " (plrobjs.id = %ld AND plrobjs.obj_id = %s.uuid)",
      GET_NAME(ch), name[i], name[i], name[i], GET_IDNUM(ch), name[i]);
    sql_query(darkpawns, buf1, FALSE);
  }

  /* These tables cannot be created as type HEAP, hence they are separate */
  for (i = 0; strcmp(non_heap[i], "\n"); i++) {
    sprintf(buf1, "CREATE TABLE %s_%s SELECT %s.* FROM %s, plrobjs WHERE (plrobjs.id = "
      "%ld AND plrobjs.obj_id = %s.uuid)", GET_NAME(ch), non_heap[i], non_heap[i],
      non_heap[i], GET_IDNUM(ch), non_heap[i]);
    sql_query(darkpawns, buf1, FALSE);
  }
}


void sql_remove_heap(struct char_data *ch)
{
  int i;
  static char *name[] =
    { "objs", "obj_value", "obj_affects", "obj_reqs", "obj_extra_desc", "obj_container",
      "obj_action_desc", "\n" };

  /* Re-save the player's items into the default table now that the objects have
  been extracted from the game */
  sprintf(buf1, "REPLACE plrobjs (id, obj_id, vnum, location) SELECT * FROM %s",
    GET_NAME(ch));
  sprintf(buf2, "DROP TABLE %s", GET_NAME(ch));
  sql_query(darkpawns, buf1, FALSE);
  sql_query(darkpawns, buf2, FALSE);

  /* Re-save temporary objects into the default table */
  for (i = 0; strcmp(name[i], "\n"); i++) {
    sprintf(buf1, "REPLACE %s SELECT * FROM %s_%s", name[i], GET_NAME(ch), name[i]);
    sprintf(buf2, "DROP TABLE IF EXISTS %s_%s", GET_NAME(ch), name[i]);
    sql_query(darkpawns, buf1, FALSE);
    sql_query(darkpawns, buf2, FALSE);
  }
}


void sql_Crash_delete_file(long id)
{
  int i;
  MYSQL_RES *table;
  MYSQL_ROW entry;
  static char *name[] =
    { "objs", "obj_value", "obj_affects", "obj_reqs", "obj_extra_desc", "obj_container",
      "obj_action_desc", "\n" };
  
  sprintf(buf1, "SELECT * FROM plrobjs WHERE id = %ld", id);
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    while ((entry = mysql_fetch_row(table)))
      for (i = 0; strcmp(name[i], "\n"); i++) {
        sprintf(buf2, "DELETE FROM %s WHERE uuid = '%s'", name[i], entry[1]);
        sql_query(darkpawns, buf2, FALSE);
      }
  mysql_free_result(table);
  sprintf(buf1, "DELETE FROM plrobjs WHERE id = %ld", id);
  sql_query(darkpawns, buf1, FALSE);
}


void sql_write_mud_date(void)
{
  extern struct time_info_data time_info;
  extern int game_restrict;
  extern time_t shops_time;

  sprintf(buf1, "%d, %d, %d, %d, %ld",
    time_info.day, time_info.month, time_info.year, game_restrict, shops_time);
  sprintf(buf2, "DELETE FROM mud_date");
  sprintf(buf3, "INSERT INTO mud_date VALUES (%s)", buf1);
  sql_query(darkpawns, buf2, FALSE);
  sql_query(darkpawns, buf3, FALSE);
}

void sql_read_mud_date(void)
{
  extern struct time_info_data time_info;
  extern int game_restrict;
  extern time_t shops_time;
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if ((table = sql_query(darkpawns, "SELECT * FROM mud_date", TRUE)))
    if ((entry = mysql_fetch_row(table))) {
      time_info.day = atoi(entry[0]);
      time_info.month = atoi(entry[1]);
      time_info.year = atoi(entry[2]);
      game_restrict = atoi(entry[3]);
      shops_time = atoi(entry[4]);
    }
  mysql_free_result(table);
}

void sql_backup(void)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;

  return; 	/* until we can get this to work */

  if ((table = sql_query(darkpawns, "SHOW TABLES", TRUE)))
    while ((entry = mysql_fetch_row(table))) {
      sprintf(buf1, "SELECT * FROM %s INTO OUTFILE '%s%s.bak'",
        entry[0], BACKUP_DIR, entry[0]);
      sql_query(darkpawns, buf1, FALSE);
    }
}


void sql_erase(void)
{
  int i;
  static char *objs[] =
    { "objs", "obj_value", "obj_affects", "obj_reqs", "obj_extra_desc", "obj_container",
      "obj_action_desc", "\n" };
  static char *shop_mobs[] = { "mobobjs", "mobs", "mob_saves", "\n" };
  static char *others[] = { "houseobjs", "\n" };

  /* Player related - keep their objects and wipe the rest */
  for (i = 0; strcmp(objs[i], "\n"); i++) {
    sprintf(buf1, "CREATE TABLE %s_temp SELECT * FROM %s", objs[i], objs[i]);
    sprintf(buf2, "DELETE %s.* FROM %s LEFT JOIN plrobjs ON %s.uuid = plrobjs.obj_id WHERE "
      "plrobjs.obj_id IS NULL", objs[i], objs[i], objs[i]);
    sql_query(darkpawns, buf1, FALSE);
    sql_query(darkpawns, buf2, FALSE);
  }

  /* Let's get only player-owned shops to start with */
  sprintf(buf1, "DELETE FROM shops WHERE owned != 1");
  sql_query(darkpawns, buf1, FALSE);

  /* Now we want to go through and select only our shop keepers */
  for (i = 0; strcmp(shop_mobs[i], "\n"); i++) {
    sprintf(buf1, "DELETE %s.* FROM %s LEFT JOIN shops ON %s.uuid = shops.uuid WHERE "
      "shops.uuid IS NULL", shop_mobs[i], shop_mobs[i], shop_mobs[i]);
    sql_query(darkpawns, buf1, TRUE);
  }

  /* Now we keep any objects belonging to the shop keepers */
  for (i = 0; strcmp(objs[i], "\n"); i++) {
    sprintf(buf1, "REPLACE %s SELECT %s_temp.* FROM %s_temp LEFT JOIN mobobjs ON "
      "%s_temp.uuid = mobobjs.obj_id WHERE mobobjs.obj_id IS NOT NULL", objs[i], objs[i],
      objs[i], objs[i]);
    sprintf(buf2, "REPLACE %s SELECT %s_temp.* FROM %s_temp LEFT JOIN shops ON "
      "%s_temp.uuid = shops.deed WHERE shops.deed IS NOT NULL", objs[i], objs[i], objs[i],
      objs[i]);
    sprintf(buf3, "DROP TABLE %s_temp", objs[i]);
    sql_query(darkpawns, buf1, FALSE);
    sql_query(darkpawns, buf2, FALSE);
    sql_query(darkpawns, buf3, FALSE);
  }

  /* Finally, we need to keep the deeds */
  for (i = 0; strcmp(others[i], "\n"); i++) {
    sprintf(buf1, "DELETE %s.* FROM %s LEFT JOIN shops ON %s.obj_id = shops.deed WHERE shops.deed IS NULL",
      others[i], others[i], others[i]);
    sql_query(darkpawns, buf1, FALSE);
  }
}

void sql_listskills(struct char_data *ch, char *name)
{
  int i, j, skillnum;
  struct char_data *player;

  extern const char *skill_groups[];
  extern struct skill_tree_data skill_tree[];
  int find_skill_num(char *name);


  if (!(player = load_char(name))) {
    send_to_char("Unable to locate the specified player.\r\n", ch);
    return;
  }

  sprintf(buf, "%s's skills and spells:\r\n", GET_NAME(player));
  send_to_char(buf, ch);
  *buf = '\0';

  for (i = 1; i <= MAX_SKILL_GROUPS; i++)
    if (GET_GROUP_LVL(player, i)) {
      sprintf(buf + strlen(buf), "&W%-20s&n(Level: &g%d&n, Competency: &g%-3d%%&n )\r\n",
        skill_groups[i], GET_GROUP_LVL(player, i), GET_GROUP_SKILL(player, i));
      for (j = 1; strcmp(skill_tree[j].name, "\n"); j++) {
        if ((skill_tree[j].group == i) && (skill_tree[j].level <= GET_GROUP_LVL(player, i))) {
          skillnum = find_skill_num(skill_tree[j].name);
          if (skillnum > NUM_SPELLS)    /* It must be a skill, not a spell */
            sprintf(buf + strlen(buf), "  %-25s\r\n", skill_tree[j].name);
          else if (GET_SPELL(player, skillnum) !=  '0')
            sprintf(buf + strlen(buf), "  %-25s\r\n", skill_tree[j].name);
        }
      }
    }

  if (!*buf)
    send_to_char("  None.\r\n", ch);
  else
    page_string(ch->desc, buf, 1);
  free_char(player);
}

char *sql_get_host(struct char_data *ch)
{
  MYSQL_RES *table;
  MYSQL_ROW entry;

  if (!ch)
    return (NULL);

  *buf2 = '\0';
  sprintf(buf1, "SELECT host FROM players WHERE id = %ld", get_id_by_name(GET_NAME(ch)));
  if ((table = sql_query(darkpawns, buf1, TRUE)))
    if ((entry = mysql_fetch_row(table)))
      strcpy(buf2, entry[0]);

  mysql_free_result(table);
  return (buf2);
}
