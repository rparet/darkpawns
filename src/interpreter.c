/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
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

/* $Id: interpreter.c 1504 2008-05-23 01:06:35Z jravn $ */

#define __INTERPRETER_C__ 1
#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "olc.h"
#include "clan.h"
#include "improved-edit.h"
#include "scripts.h"
#include "tedit.h"

extern const char *titles[NUM_CLASSES];
extern char *abil_names[];
extern char *motd;
extern char *imotd;
extern char *background;
extern char *MENU;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int game_restrict;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;

/* external functions */
void roll_real_abils(struct char_data *ch); 
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
void init_char(struct char_data *ch);
int create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
void oedit_parse(struct descriptor_data *d, char *arg);
void redit_parse(struct descriptor_data *d, char *arg);
void zedit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void read_aliases(struct char_data *ch);
void read_poofs(struct char_data *ch);
void InfoBarOn(struct char_data *ch);

/* prototypes for all do_x functions. */
ACMD(do_abils);
ACMD(do_action);
ACMD(do_advance);
ACMD(do_afk);
ACMD(do_alias);
ACMD(do_ambush);
ACMD(do_assist);
ACMD(do_auto);
ACMD(do_at);
ACMD(do_appraise);
ACMD(adjust_mobs);
ACMD(do_backstab);
ACMD(do_ban);
ACMD(do_bearhug);
ACMD(do_behead);
ACMD(do_berserk);
ACMD(do_bash);
ACMD(do_bite);
ACMD(do_carve);
ACMD(do_cast);
ACMD(do_charge);
ACMD(do_checkload);
ACMD(do_circle);
ACMD(do_clan);
ACMD(do_ctell);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_compare);
ACMD(do_consider);
ACMD(do_cutthroat);
ACMD(do_credits);
ACMD(do_date);
ACMD(do_dark);
ACMD(do_dc);
ACMD(do_detect);
ACMD(do_diagnose);
ACMD(do_dig);
ACMD(do_disarm);
ACMD(do_disembowel);
ACMD(do_dismount);
ACMD(do_display);
ACMD(do_dns);
ACMD(do_drink);
ACMD(do_dragon_kick);
ACMD(do_drop);
ACMD(do_dream);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_first_aid);
ACMD(do_flee);
ACMD(do_flesh_alter);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_groinrip);
ACMD(do_coins);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_hcontrol);
ACMD(do_headbutt);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_home);
ACMD(do_house);
ACMD(do_idlist);
ACMD(do_inactive);
ACMD(do_info);
ACMD(do_infobar);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_kuji_kiri);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_lines);
ACMD(do_load);
ACMD(do_look);
ACMD(do_luaedit);
ACMD(do_map);
ACMD(do_mindlink);
ACMD(do_mlist);
ACMD(do_mold);
ACMD(do_move);
ACMD(do_neckbreak);
ACMD(do_newbie);
ACMD(do_not_here);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_olist);
ACMD(do_order);
ACMD(do_otouch);
ACMD(do_page);
ACMD(do_parry);
ACMD(do_palm);
ACMD(do_peek);
ACMD(do_poofset);
ACMD(do_point);
ACMD(do_pour);
ACMD(do_practice);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_race_say);
ACMD(do_reboot);
ACMD(do_recall);
ACMD(do_remove);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_retreat);
ACMD(do_return);
ACMD(do_review);
ACMD(do_ride);
ACMD(do_rlist);
ACMD(do_roll);
ACMD(do_save);
ACMD(do_say);
ACMD(do_score);
ACMD(do_scout);
ACMD(do_scrounge);
ACMD(do_send);
ACMD(do_serpent_kick);
ACMD(do_set);
ACMD(do_sethunt);
ACMD(do_shadow);
ACMD(do_sharpen);
ACMD(do_shoot);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skills);
ACMD(do_skillset);
ACMD(do_sleep);
ACMD(do_sleeper);
ACMD(do_slug);
ACMD(do_smackheads);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_spike);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_stealth);
ACMD(do_strike);
ACMD(do_string);
ACMD(do_subdue);
ACMD(do_switch);
ACMD(do_sysfile);
ACMD(do_syslog);
ACMD(do_tag);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_think);
ACMD(do_time);
ACMD(do_tick);
ACMD(do_tiger_punch);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_transform);
ACMD(do_trip);
ACMD(do_turn);
ACMD(do_unban);
ACMD(do_ungroup);
ACMD(do_use);
ACMD(do_users);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_wake);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_whois);
ACMD(do_whod);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_yank);
ACMD(do_zlist);
ACMD(do_zreset);


/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

const struct command_info cmd_info[] = {
  { "RESERVED", 0, 0, 0, 0 },	/* this must be first -- for specprocs */

  /* directions must come before other commands but after RESERVED */
  { "north"    , POS_STANDING, do_move     , 0, SCMD_NORTH },
  { "east"     , POS_STANDING, do_move     , 0, SCMD_EAST },
  { "south"    , POS_STANDING, do_move     , 0, SCMD_SOUTH },
  { "west"     , POS_STANDING, do_move     , 0, SCMD_WEST },
  { "up"       , POS_STANDING, do_move     , 0, SCMD_UP },
  { "down"     , POS_STANDING, do_move     , 0, SCMD_DOWN },

  /* now, the main list */
  { "abilities", POS_SLEEPING, do_abils    , 0, 0 },
  { "at"       , POS_DEAD    , do_at       , LVL_GRGOD, 0 },
  { "advance"  , POS_DEAD    , do_advance  , LVL_GRGOD, 0 },
  { "admobs"   , POS_DEAD    , adjust_mobs, LVL_IMPL-1, 0},
  { "afk"      , POS_DEAD    , do_afk      , 0, 0 },
  { "aid"      , POS_STANDING, do_first_aid, 0, 0 },
  { "alias"    , POS_DEAD    , do_alias    , 0, 0 },
  { "alter"    , POS_FIGHTING, do_flesh_alter, 0, 0 },
  { "accuse"   , POS_SITTING , do_action   , 0, 0 },
  { "agree"    , POS_RESTING , do_action   , 0, 0 },
  { "ambush"   , POS_STANDING, do_ambush   , 0, 0 },
  { "apologize", POS_RESTING , do_action   , 0, 0 },
  { "applaud"  , POS_RESTING , do_action   , 0, 0 },
  { "anguish"  , POS_RESTING , do_action   , 0, 0 },
  { "appraise" , POS_RESTING , do_appraise , 0, 0 },
  { "assist"   , POS_FIGHTING, do_assist   , 1, 0 },
  { "ask"      , POS_RESTING , do_spec_comm, 0, SCMD_ASK },
  { "auction"  , POS_SLEEPING, do_gen_comm , 0, SCMD_AUCTION },
  { "auto"     , POS_DEAD    , do_auto     , 0, 0 },

  { "bounce"   , POS_STANDING, do_action   , 0, 0 },
  { "backstab" , POS_STANDING, do_backstab , 1, 0 },
  { "bah"      , POS_RESTING , do_action   , 0, 0 },
  { "ban"      , POS_DEAD    , do_ban      , LVL_GRGOD, 0 },
  { "balance"  , POS_STANDING, do_not_here , 1, 0 },
  { "bash"     , POS_FIGHTING, do_bash     , 1, 0 },
  { "bearhug"  , POS_FIGHTING, do_bearhug  , 1, 0 },
  { "beckon"   , POS_RESTING , do_action   , 0, 0 },
  { "behead"   , POS_STANDING, do_behead   , 0, 0 },
  { "beg"      , POS_RESTING , do_action   , 0, 0 },
  { "bellow"   , POS_RESTING , do_action   , 0, 0 },
  { "berserk"  , POS_FIGHTING, do_berserk  , 1, 0 },
  { "bird"     , POS_RESTING , do_action   , 0, 0 },
  { "bitch"    , POS_RESTING , do_action   , 0, 0 },
  { "bite"     , POS_RESTING , do_bite     , 0, 0 },
  { "blame"    , POS_RESTING , do_action   , 0, 0 },
  { "bleed"    , POS_RESTING , do_action   , 0, 0 },
  { "bless"    , POS_RESTING , do_action   , 0, 0 },
  { "blink"    , POS_RESTING , do_action   , 0, 0 },
  { "blush"    , POS_RESTING , do_action   , 0, 0 },
  { "bonk"     , POS_RESTING , do_action   , 0, 0 },
  { "boink"    , POS_RESTING , do_action   , 0, 0 },
  { "boggle"   , POS_RESTING , do_action   , 0, 0 },
  { "bow"      , POS_STANDING, do_action   , 0, 0 },
  { "brief"    , POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF },
  { "brb"      , POS_RESTING , do_action   , 0, 0 },
  { "burp"     , POS_RESTING , do_action   , 0, 0 },
  { "buy"      , POS_STANDING, do_not_here , 0, 0 },
  { "bug"      , POS_DEAD    , do_gen_write, 0, SCMD_BUG },

  { "cackle"   , POS_RESTING , do_action   , 0, 0 },
  { "cast"     , POS_SITTING , do_cast     , 1, 0 },
  { "carve"    , POS_STANDING, do_carve    , 0, 0 },
  { "chuckle"  , POS_RESTING , do_action   , 0, 0 },
  { "chide"    , POS_RESTING , do_action   , 0, 0 },
  { "charge"   , POS_FIGHTING, do_charge   , 1, 0 },
  { "circle"   , POS_FIGHTING, do_circle   , 1, 0 },
  { "cheer"    , POS_RESTING , do_action   , 0, 0 },
  { "check"    , POS_STANDING, do_not_here , 1, 0 },
  { "checkload", POS_DEAD    , do_checkload, LVL_IMMORT, 0 },
  { "close"    , POS_SITTING , do_gen_door , 0, SCMD_CLOSE },
  { "clan"     , POS_SLEEPING, do_clan     , 1, 0 },
  { "clap"     , POS_RESTING , do_action   , 0, 0 },
  { "clear"    , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "cls"      , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "consider" , POS_RESTING , do_consider , 0, 0 },
  { "color"    , POS_DEAD    , do_color    , 0, 0 },
  { "collect"  , POS_RESTING , do_not_here , 0, 0 },
  { "comfort"  , POS_RESTING , do_action   , 0, 0 },
  { "comb"     , POS_RESTING , do_action   , 0, 0 },
  { "commands" , POS_DEAD    , do_commands , 0, SCMD_COMMANDS },
  { "compare"  , POS_STANDING, do_compare  , 0, 0 },
  { "compact"  , POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT },
  { "cough"    , POS_RESTING , do_action   , 0, 0 },
  { "credits"  , POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS },
  { "cringe"   , POS_RESTING , do_action   , 0, 0 },
  { "cry"      , POS_RESTING , do_action   , 0, 0 },
  { "ctell"    , POS_SLEEPING, do_ctell    , 0, 0 },
  { "cuddle"   , POS_RESTING , do_action   , 0, 0 },
  { "curse"    , POS_RESTING , do_action   , 0, 0 },
  { "curtsey"  , POS_STANDING, do_action   , 0, 0 },
  { "cutthroat", POS_FIGHTING, do_cutthroat, 1, 0 },

  { "dance"    , POS_STANDING, do_action   , 0, 0 },
  { "date"     , POS_DEAD    , do_date     , LVL_IMMORT, SCMD_DATE },
  { "daydream" , POS_SLEEPING, do_action   , 0, 0 },
  { "dark"     , POS_DEAD    , do_dark     , LVL_IMMORT, 0 },
  { "dc"       , POS_DEAD    , do_dc       , LVL_IMMORT+1, 0 },
  { "deposit"  , POS_STANDING, do_not_here , 1, 0 },
  { "search"   , POS_STANDING, do_detect   , 0, 0 },
  { "diagnose" , POS_RESTING , do_diagnose , 0, 0 },
  { "dig"      , POS_RESTING , do_dig      , LVL_BUILDER, 0 },
  { "disarm"   , POS_FIGHTING, do_disarm   , 0, 0 },
  { "disembowel", POS_FIGHTING, do_disembowel, 0, 0 },
  { "dismount" , POS_FIGHTING, do_dismount , 0, 0 },
  { "display"  , POS_DEAD    , do_display  , 0, 0 },
  { "dns"      , POS_DEAD    , do_dns      , LVL_IMPL-1, 0 },
  { "doh"      , POS_RESTING , do_action   , 0, 0 },
  { "donate"   , POS_RESTING , do_drop     , 0, SCMD_DONATE },
  { "drink"    , POS_RESTING , do_drink    , 0, SCMD_DRINK },
  { "dragon"   , POS_FIGHTING, do_dragon_kick, 0, 0 },
  { "dream"    , POS_SLEEPING, do_dream    , 0, 0 },
  { "drop"     , POS_RESTING , do_drop     , 0, SCMD_DROP },
  { "drool"    , POS_RESTING , do_action   , 0, 0 },

  { "eat"      , POS_RESTING , do_eat      , 0, SCMD_EAT },
  { "echo"     , POS_SLEEPING, do_echo     , LVL_IMMORT, SCMD_ECHO },
  { "emote"    , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { ":"        , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { "embrace"  , POS_STANDING, do_action   , 0, 0 },
  { "enter"    , POS_STANDING, do_enter    , 0, 0 },
  { "equipment", POS_SLEEPING, do_equipment, 0, 0 },
  { "escape"   , POS_FIGHTING, do_retreat  , 0, 0 },
  { "exits"    , POS_RESTING , do_exits    , 0, 0 },
  { "examine"  , POS_RESTING , do_examine  , 0, 0 },

  { "force"    , POS_SLEEPING, do_force    , LVL_GOD, 0 },
  { "fade"     , POS_RESTING , do_action   , 0, 0 },
  { "faint"    , POS_RESTING , do_action   , 0, 0 },
  { "farewell" , POS_RESTING , do_action   , 0, 0 },
  { "fart"     , POS_RESTING , do_action   , 0, 0 },
  { "fill"     , POS_STANDING, do_pour     , 0, SCMD_FILL },
  { "finger"   , POS_DEAD    , do_whois    , 0, 0 },
  { "flee"     , POS_FIGHTING, do_flee     , 1, 0 },
  { "flesh"    , POS_FIGHTING, do_flesh_alter, 0, 0 },
  { "flex"     , POS_RESTING , do_action   , 0, 0 },
  { "flip"     , POS_STANDING, do_action   , 0, 0 },
  { "flirt"    , POS_RESTING , do_action   , 0, 0 },
  { "follow"   , POS_RESTING , do_follow   , 0, 0 },
  { "fondle"   , POS_RESTING , do_action   , 0, 0 },
  { "freeze"   , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_FREEZE },
  { "french"   , POS_RESTING , do_action   , 0, 0 },
  { "frown"    , POS_RESTING , do_action   , 0, 0 },
  { "fume"     , POS_RESTING , do_action   , 0, 0 },
  { "future"   , POS_DEAD    , do_gen_ps   , 0, SCMD_FUTURE },
  { "fwap"     , POS_RESTING , do_action   , 0, 0 },

  { "get"      , POS_RESTING , do_get      , 0, 0 },
  { "gag"      , POS_RESTING , do_action   , 0, 0 },
  { "gasp"     , POS_RESTING , do_action   , 0, 0 },
  { "gecho"    , POS_DEAD    , do_gecho    , LVL_GOD, 0 },
  { "give"     , POS_RESTING , do_give     , 0, 0 },
  { "giggle"   , POS_RESTING , do_action   , 0, 0 },
  { "glare"    , POS_RESTING , do_action   , 0, 0 },
  { "glance"   , POS_RESTING , do_diagnose , 0, 0 },
  { "goto"     , POS_SLEEPING, do_goto     , LVL_IMMORT, 0 },
  { "gossip"   , POS_SLEEPING, do_gen_comm , 0, SCMD_GOSSIP },
  { "gold"     , POS_RESTING , do_coins    , 0, 0 },
  { "goose"    , POS_RESTING , do_action   , 0, 0 },
  { "group"    , POS_SLEEPING, do_group    , 1, 0 },
  { "grab"     , POS_RESTING , do_grab     , 0, 0 },
  { "grats"    , POS_SLEEPING, do_gen_comm , 0, SCMD_GRATZ },
  { "greet"    , POS_RESTING , do_action   , 0, 0 },
  { "grin"     , POS_RESTING , do_action   , 0, 0 },
  { "grimace"  , POS_RESTING , do_action   , 0, 0 },
  { "groan"    , POS_RESTING , do_action   , 0, 0 },
  { "groinrip" , POS_FIGHTING, do_groinrip , 0, 0 },
  { "grope"    , POS_RESTING , do_action   , 0, 0 },
  { "grovel"   , POS_RESTING , do_action   , 0, 0 },
  { "growl"    , POS_RESTING , do_action   , 0, 0 },
  { "grumble"  , POS_RESTING , do_action   , 0, 0 },
  { "gsay"     , POS_SLEEPING, do_gsay     , 0, 0 },
  { "gtell"    , POS_SLEEPING, do_gsay     , 0, 0 },

  { "help"     , POS_DEAD    , do_help     , 0, 0 },
  { "headbutt" , POS_FIGHTING, do_headbutt , 1, 0 },
  { "?"        , POS_DEAD    , do_help     , 0, 0 },
  { "heh"      , POS_RESTING , do_action   , 0, 0 },
  { "handbook" , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_HANDBOOK },
  { "hcontrol" , POS_DEAD    , do_hcontrol , LVL_GRGOD, 0 },
  { "hiccup"   , POS_RESTING , do_action   , 0, 0 },
  { "hide"     , POS_RESTING , do_hide     , 1, 0 },
  { "highfive" , POS_RESTING , do_action   , 0, 0 },
  { "hit"      , POS_FIGHTING, do_hit      , 0, SCMD_HIT },
  { "hire"     , POS_STANDING, do_not_here , 0, 0 },
  { "hold"     , POS_RESTING , do_grab     , 1, 0 },
  { "holler"   , POS_RESTING , do_gen_comm , 1, SCMD_HOLLER },
  { "holylight", POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_HOLYLIGHT },
  { "home"     , POS_RESTING , do_home     , LVL_IMMORT, 0 },
  { "hop"      , POS_RESTING , do_action   , 0, 0 },
  { "house"    , POS_RESTING , do_house    , 0, 0 },
  { "howl"     , POS_RESTING , do_action   , 0, 0 },
  { "hug"      , POS_RESTING , do_action   , 0, 0 },
  { "hum"      , POS_RESTING , do_action   , 0, 0 },
  { "hump"     , POS_RESTING , do_action   , 0, 0 },
  { "hush"     , POS_RESTING , do_action   , 0, 0 },

  { "inventory", POS_DEAD    , do_inventory, 0, 0 },
  { "idea"     , POS_DEAD    , do_gen_write, 0, SCMD_IDEA },
  { "ident"    , POS_DEAD    , do_gen_tog  , LVL_IMPL-1, SCMD_IDENT },
  { "idlist"   , POS_DEAD    , do_idlist   , LVL_GRGOD, 0 },
  { "imotd"    , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_IMOTD },
  { "immlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST },
  { "inactive" , POS_SLEEPING, do_inactive , 0, 0 },
  { "info"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO },
  { "infobar"  , POS_DEAD    , do_infobar  , 0, 0 },
  { "insult"   , POS_RESTING , do_insult   , 0, 0 },
  { "invis"    , POS_DEAD    , do_invis    , LVL_IMMORT, 0 },

  { "junk"     , POS_RESTING , do_drop     , 0, SCMD_JUNK },
  { "jeer"     , POS_RESTING , do_action   , 0, 0 },
  { "jin"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_JIN },

  { "kill"     , POS_FIGHTING, do_kill     , 0, 0 },
  { "kabuki"   , POS_RESTING , do_hide     , 1, SCMD_KABUKI },
  { "kai"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_KAI },
  { "kick"     , POS_FIGHTING, do_kick     , 1, 0 },
  { "kiss"     , POS_RESTING , do_action   , 0, 0 },
  { "kyo"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_KYO },
  
  { "look"     , POS_RESTING , do_look     , 0, SCMD_LOOK },
  { "laugh"    , POS_RESTING , do_action   , 0, 0 },
  { "lambada"  , POS_STANDING, do_action   , 0, 0 },
  { "last"     , POS_DEAD    , do_last     , LVL_GOD-1, 0 },
  { "leave"    , POS_STANDING, do_leave    , 0, 0 },
  { "leer"     , POS_RESTING , do_action   , 0, 0 },
  { "levels"   , POS_DEAD    , do_levels   , 0, 0 },
  { "list"     , POS_STANDING, do_not_here , 0, 0 },
  { "listen"   , POS_RESTING , do_action   , 0, 0 },
  { "lick"     , POS_RESTING , do_action   , 0, 0 },
  { "lines"    , POS_DEAD    , do_lines    , 0, 0 },
  { "lock"     , POS_SITTING , do_gen_door , 0, SCMD_LOCK },
  { "load"     , POS_DEAD    , do_load     , LVL_IMMORT, 0 },
  { "love"     , POS_RESTING , do_action   , 0, 0 },
  { "luaedit"  , POS_DEAD    , do_luaedit  , LVL_BUILDER, LVL_HIGOD },

  { "map"      , POS_SLEEPING, do_map      , 0, 0},        
  { "mindlink" , POS_STANDING, do_mindlink , 0, 0 },
  { "moan"     , POS_RESTING , do_action   , 0, 0 },
  { "mold"     , POS_RESTING , do_mold     , LVL_IMMORT, 0 },
  { "mosh"     , POS_RESTING , do_action   , 0, 0 },
  { "medit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_MEDIT},
  { "mlist"    , POS_DEAD    , do_mlist    , LVL_BUILDER, 0},
  { "motd"     , POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD },
  { "mail"     , POS_STANDING, do_not_here , 1, 0 },
  { "massage"  , POS_RESTING , do_action   , 0, 0 },
  { "mount"    , POS_STANDING, do_ride     , 0, 0 },
  { "mute"     , POS_DEAD    , do_wizutil  , 1, SCMD_SQUELCH },
  { "muhaha"   , POS_RESTING , do_action   , 0, 0 },
  { "mumble"   , POS_RESTING , do_action   , 0, 0 },
  { "murder"   , POS_FIGHTING, do_hit      , 0, SCMD_MURDER },

  { "neckbreak", POS_FIGHTING, do_neckbreak, 0, 0 },
  { "news"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS },
  { "newbie"   , POS_SLEEPING, do_gen_comm , 0, SCMD_NEWBIE },
  { "nibble"   , POS_RESTING , do_action   , 0, 0 },
  { "nod"      , POS_RESTING , do_action   , 0, 0 },
  { "noauction", POS_DEAD    , do_gen_tog  , 0, SCMD_NOAUCTION },
  { "nobroadcast", POS_DEAD  , do_gen_tog  , 0, SCMD_NOBROAD },
  { "noctell"  , POS_DEAD    , do_gen_tog  , 0, SCMD_NOCTELL },
  { "nogossip" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP },
  { "nograts"  , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGRATZ },
  { "nohassle" , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOHASSLE },
  { "nonewbie" , POS_DEAD    , do_gen_tog  , 0, SCMD_NONEWBIE },
  { "norepeat" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT },
  { "noshout"  , POS_SLEEPING, do_gen_tog  , 1, SCMD_DEAF },
  { "nosummon" , POS_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON },
  { "notell"   , POS_DEAD    , do_gen_tog  , 1, SCMD_NOTELL },
  { "notitle"  , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_NOTITLE },
  { "noogie"   , POS_RESTING , do_action   , 0, 0 },
  { "nowiz"    , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOWIZ },
  { "nudge"    , POS_RESTING , do_action   , 0, 0 },
  { "nuzzle"   , POS_RESTING , do_action   , 0, 0 },

  { "order"    , POS_RESTING , do_order    , 1, 0 },
  { "orgasm"   , POS_RESTING , do_otouch   , LVL_IMMORT, 0 },
  { "offer"    , POS_STANDING, do_not_here , 1, 0 },
  { "open"     , POS_SITTING , do_gen_door , 0, SCMD_OPEN },
  { "olc"      , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SAVEINFO },
  { "olist"    , POS_DEAD    , do_olist    , LVL_BUILDER, 0},
  { "oedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_OEDIT},

  { "put"      , POS_RESTING , do_put      , 0, 0 },
  { "pace"     , POS_RESTING , do_action   , 0, 0 },
  { "pant"     , POS_RESTING , do_action   , 0, 0 },
  { "pat"      , POS_RESTING , do_action   , 0, 0 },
  { "page"     , POS_DEAD    , do_page     , LVL_IMMORT, 0 },
  { "palm"     , POS_STANDING, do_palm     , 0, 0 },
  { "parry"    , POS_DEAD    , do_parry    , 0, 0 },
  { "pardon"   , POS_DEAD    , do_wizutil  , 1, SCMD_PARDON },
  { "peek"     , POS_RESTING , do_peek     , 0, 0 },
  { "peer"     , POS_RESTING , do_action   , 0, 0 },
  { "pick"     , POS_STANDING, do_gen_door , 1, SCMD_PICK },
  { "pinch"    , POS_RESTING , do_action   , 0, 0 },
  { "players"  , POS_DEAD,     do_gen_ps   , LVL_GRGOD, SCMD_PLAYER_LIST },
  { "plot"     , POS_RESTING , do_action   , 0, 0 },
  { "piss"     , POS_RESTING , do_action   , 0, 0 },
  { "point"    , POS_RESTING , do_point    , 0, 0 },
  { "poke"     , POS_RESTING , do_action   , 0, 0 },
  { "policy"   , POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES },
  { "ponder"   , POS_RESTING , do_action   , 0, 0 },
  { "poofin"   , POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFIN },
  { "poofout"  , POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFOUT },
  { "pose"     , POS_STANDING, do_action   , 0, 0 },
  { "pour"     , POS_STANDING, do_pour     , 0, SCMD_POUR },
  { "pout"     , POS_RESTING , do_action   , 0, 0 },
  { "practice" , POS_RESTING , do_practice , 1, 0 },
  { "prompt"   , POS_DEAD    , do_display  , 0, 0 },
  { "pray"     , POS_SITTING , do_action   , 0, 0 },
  { "puke"     , POS_RESTING , do_action   , 0, 0 },
  { "punch"    , POS_RESTING , do_action   , 0, 0 },
  { "purr"     , POS_RESTING , do_action   , 0, 0 },
  { "purge"    , POS_DEAD    , do_purge    , LVL_IMMORT+1, 0 },

  { "quaff"    , POS_SITTING , do_use      , 0, SCMD_QUAFF },
  { "qecho"    , POS_DEAD    , do_qcomm    , LVL_IMMORT, SCMD_QECHO },
  { "quest"    , POS_DEAD    , do_gen_tog  , 0, SCMD_QUEST },
  { "qui"      , POS_DEAD    , do_quit     , 0, 0 },
  { "quit"     , POS_DEAD    , do_quit     , 0, SCMD_QUIT },
  { "qsay"     , POS_SLEEPING, do_qcomm    , 0, SCMD_QSAY },

  { "rest"     , POS_RESTING , do_rest     , 0, 0 },
  { "report"   , POS_RESTING , do_report   , 0, 0 },
  { "raise"     , POS_RESTING , do_action   , 0, 0 },
  { "read"     , POS_RESTING , do_look     , 0, SCMD_READ },
  { "reel"     , POS_RESTING , do_action   , 0, 0 },
  { "reload"   , POS_DEAD    , do_reboot   , LVL_IMPL-1, 0 },
  { "recite"   , POS_RESTING , do_use      , 0, SCMD_RECITE },
  { "recharge" , POS_STANDING, do_not_here , 0, 0 },
  { "recall"   , POS_RESTING , do_recall   , 0, 0 },
  { "receive"  , POS_STANDING, do_not_here , 1, 0 },
  { "remove"   , POS_RESTING , do_remove   , 0, 0 },
  { "remort"   , POS_STANDING, do_not_here , 0, 0 },
  { "ren"      , POS_RESTING , do_action   , 0, 0 },
  { "rent"     , POS_STANDING, do_not_here , 1, 0 },
  { "reply"    , POS_DEAD    , do_reply    , 0, 0 },
  { "."        , POS_SLEEPING, do_reply    , 0, 0 },
  { "reroll"   , POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_REROLL },
  { "rescue"   , POS_FIGHTING, do_rescue   , 1, 0 },
  { "restore"  , POS_DEAD    , do_restore  , LVL_GOD-1, 0 },
  { "retreat"  , POS_FIGHTING, do_retreat  , 0, 0 },
  { "retrieve" , POS_STANDING, do_not_here,  1, 0 },
  { "retsu"    , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_RETSU },
  { "return"   , POS_DEAD    , do_return   , 0, 0 },
  { "redit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_REDIT},
  { "reallyquit",POS_DEAD    , do_quit     , 0, SCMD_REALLY_QUIT },
  { "review"   , POS_DEAD    , do_review   , 0, 0 },
  { "ride"     , POS_STANDING, do_ride     , 0, 0 },
  { "rin"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_RIN },
  { "rlist"    , POS_DEAD    , do_rlist    , LVL_BUILDER, 0},
  { "roar"     , POS_RESTING , do_action   , 0, 0 },
  { "rofl"     , POS_RESTING , do_action   , 0, 0 },
  { "roll"     , POS_DEAD    , do_roll     , 0, 0 },
  /*  { "rolleyes" , POS_RESTING , do_action   , 0, 0 },*/
  { "roomflags", POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_ROOMFLAGS },
  { "rsay"     , POS_RESTING , do_race_say , 0, 0 },
  { "ruffle"   , POS_STANDING, do_action   , 0, 0 },

  { "say"      , POS_RESTING , do_say      , 0, 0 },
  { "'"        , POS_RESTING , do_say      , 0, 0 },
  { "save"     , POS_SLEEPING, do_save     , 0, 0 },
  { "salute"   , POS_RESTING , do_action   , 0, 0 },
  { "score"    , POS_DEAD    , do_score    , 0, 0 },
  { "scoff"    , POS_RESTING , do_action   , 0, 0 },
  { "scout"    , POS_STANDING, do_scout    , 0, 0 },
  { "scream"   , POS_RESTING , do_action   , 0, 0 },
  { "scratch"  , POS_RESTING , do_action   , 0, 0 },
  { "scrounge" , POS_STANDING, do_scrounge , 0, 0 },
  { "sell"     , POS_STANDING, do_not_here , 0, 0 },
  { "send"     , POS_SLEEPING, do_send     , LVL_GOD, 0 },
  { "set"      , POS_DEAD    , do_set      , LVL_GOD, 0 },
  { "sethunt"  , POS_DEAD    , do_sethunt  , LVL_GRGOD, 0 },
  { "sedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SEDIT},
  { "serpent"  , POS_FIGHTING, do_serpent_kick, 1, 0 },
  { "sha"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_SHA },
  { "shadow"   , POS_STANDING, do_follow   , 0, TRUE },
  { "shame"    , POS_RESTING , do_action   , 0, 0 },
  { "shishkabob",POS_RESTING , do_action   , 0, 0 },
  { "shout"    , POS_RESTING , do_gen_comm , 0, SCMD_SHOUT },
  { "shake"    , POS_RESTING , do_action   , 0, 0 },
  { "sharpen"  , POS_RESTING , do_sharpen  , 0, 0 },
  { "shiver"   , POS_RESTING , do_action   , 0, 0 },
  { "show"     , POS_DEAD    , do_show     , LVL_IMMORT, 0 },
  { "shoot"    , POS_STANDING, do_shoot    , 0, 0 },
  { "shrug"    , POS_RESTING , do_action   , 0, 0 },
  { "shudder"  , POS_RESTING , do_action   , 0, 0 },
  { "shutdow"  , POS_DEAD    , do_shutdown , LVL_IMPL-1, 0 },
  { "shutdown" , POS_DEAD    , do_shutdown , LVL_IMPL-1, SCMD_SHUTDOWN },
  { "sigh"     , POS_RESTING , do_action   , 0, 0 },
  { "sing"     , POS_RESTING , do_action   , 0, 0 },
  { "sip"      , POS_RESTING , do_drink    , 0, SCMD_SIP },
  { "sit"      , POS_RESTING , do_sit      , 0, 0 },
  { "skillset" , POS_SLEEPING, do_skillset , LVL_GRGOD, 0 },
  { "sleep"    , POS_SLEEPING, do_sleep    , 0, 0 },
  { "sleeper"  , POS_STANDING, do_sleeper  , 0, 0 },
  { "slap"     , POS_RESTING , do_action   , 0, 0 },
  { "slowns"   , POS_DEAD    , do_gen_tog  , LVL_IMPL-1, SCMD_SLOWNS },
  { "slug"     , POS_FIGHTING, do_slug     , 1, 0 },
  { "smile"    , POS_RESTING , do_action   , 0, 0 },
  { "smackheads" , POS_FIGHTING, do_smackheads    , 1, 0 },
  { "smell"    , POS_RESTING , do_action   , 0, 0 },
  { "smirk"    , POS_RESTING , do_action   , 0, 0 },
  { "sneeze"   , POS_RESTING , do_action   , 0, 0 },
  { "sneer"    , POS_RESTING , do_action   , 0, 0 },
  { "snicker"  , POS_RESTING , do_action   , 0, 0 },
  { "snap"     , POS_RESTING , do_action   , 0, 0 },
  { "snarl"    , POS_RESTING , do_action   , 0, 0 },
  { "sneak"    , POS_STANDING, do_sneak    , 1, 0 },
  { "sniff"    , POS_RESTING , do_action   , 0, 0 },
  { "snore"    , POS_SLEEPING, do_action   , 0, 0 },
  { "snowball" , POS_STANDING, do_action   , LVL_IMMORT, 0 },
  { "snoop"    , POS_DEAD    , do_snoop    , LVL_GOD, 0 },
  { "snuggle"  , POS_RESTING , do_action   , 0, 0 },
  { "socials"  , POS_DEAD    , do_commands , 0, SCMD_SOCIALS },
  { "split"    , POS_SITTING , do_split    , 1, 0 },
  { "spackle"  , POS_RESTING , do_action   , 0, 0 },
  { "spank"    , POS_RESTING , do_action   , 0, 0 },
  { "spew"     , POS_RESTING , do_action   , 0, 0 },
  { "spike"    , POS_STANDING, do_spike    , 0, SCMD_SPIKE },
  { "spit"     , POS_RESTING , do_action   , 0, 0 },
  { "squeeze"  , POS_RESTING , do_action   , 0, 0 },
  { "stand"    , POS_RESTING , do_stand    , 0, 0 },
  { "stake"    , POS_STANDING, do_spike    , 0, SCMD_STAKE },
  { "stable"   , POS_RESTING,  do_not_here , 0, 0 },
  { "stare"    , POS_RESTING , do_action   , 0, 0 },
  { "startle"  , POS_RESTING , do_action   , 0, 0 },
  { "stat"     , POS_DEAD    , do_stat     , LVL_IMMORT, 0 },
  { "steal"    , POS_STANDING, do_steal    , 1, 0 },
  { "stealth"  , POS_STANDING, do_stealth  , 1, 0 },
  { "steam"    , POS_RESTING , do_action   , 0, 0 },
  { "stimpy"   , POS_RESTING , do_action   , 0, 0 },
  { "stretch"  , POS_RESTING , do_action   , 0, 0 },
  { "strike"   , POS_FIGHTING, do_strike   , 1, 0 },
  { "string"   , POS_RESTING , do_string   , LVL_IMMORT+1, 0 },
  { "stroke"   , POS_RESTING , do_action   , 0, 0 },
  { "strut"    , POS_STANDING, do_action   , 0, 0 },
  { "subdue"   , POS_STANDING, do_subdue   , 0, 0 },        
  { "sulk"     , POS_RESTING , do_action   , 0, 0 },
  { "sweat"    , POS_RESTING , do_action   , 0, 0 },
  { "switch"   , POS_DEAD    , do_switch   , LVL_IMMORT+1, 0 },
  { "sysfile"  , POS_DEAD    , do_sysfile  , LVL_GOD, 0 },
  { "syslog"   , POS_DEAD    , do_syslog   , LVL_IMMORT, 0 },

  { "tell"     , POS_MORTALLYW,do_tell     , 0, 0 },
  { "tackle"   , POS_RESTING , do_action   , 0, 0 },
  { "tag"      , POS_RESTING , do_tag      , 0, 0 },
  { "take"     , POS_RESTING , do_get      , 0, 0 },
  { "tango"    , POS_STANDING, do_action   , 0, 0 },
  { "tap"      , POS_RESTING , do_action   , 0, 0 },
  { "taunt"    , POS_RESTING , do_action   , 0, 0 },
  { "taste"    , POS_RESTING , do_eat      , 0, SCMD_TASTE },
  { "teleport" , POS_DEAD    , do_teleport , LVL_GOD, 0 },
  { "tedit"    , POS_DEAD    , do_tedit    , LVL_GRGOD, 0 },
  { "thank"    , POS_RESTING , do_action   , 0, 0 },
  { "think"    , POS_RESTING , do_think    , 0, 0 },
  { "thpbt"    , POS_RESTING , do_action   , 0, 0 },
  { "thaw"     , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_THAW },
  { "threaten" , POS_RESTING , do_action   , 0, 0 },
  { "throttle" , POS_RESTING , do_action   , 0, 0 },
  { "thumbsup" , POS_RESTING , do_action   , 0, 0 },
  { "thunk"    , POS_RESTING , do_action   , 0, 0 },
  { "title"    , POS_DEAD    , do_title    , 0, 0 },
  { "tick"     , POS_DEAD    , do_tick     , LVL_IMPL-1, 0 },
  { "tickle"   , POS_RESTING , do_action   , 0, 0 },
  { "tiger"    , POS_FIGHTING, do_tiger_punch, 0, 0 },
  { "tilt"     , POS_RESTING , do_action   , 0, 0 },
  { "time"     , POS_DEAD    , do_time     , 0, 0 },
  { "toggle"   , POS_DEAD    , do_toggle   , 0, 0 },
  { "todo"     , POS_DEAD    , do_gen_write, LVL_GOD-1, SCMD_TODO },
  { "toh"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_TOH },
  { "track"    , POS_STANDING, do_track    , 0, 0 },
  { "trip"     , POS_FIGHTING, do_trip     , 1, 0 },
  { "transfer" , POS_SLEEPING, do_trans    , LVL_IMMORT+1, 0 },
  { "transform", POS_RESTING , do_transform, 1, 0 },
  { "tug"      , POS_RESTING , do_action   , 0, 0 },
  { "turn"     , POS_STANDING, do_turn     , 0, 0 },
  { "twiddle"  , POS_RESTING , do_action   , 0, 0 },
  { "typo"     , POS_DEAD    , do_gen_write, 0, SCMD_TYPO },

  { "unlock"   , POS_SITTING , do_gen_door , 0, SCMD_UNLOCK },
  { "ungroup"  , POS_DEAD    , do_ungroup  , 0, 0 },
  { "unban"    , POS_DEAD    , do_unban    , LVL_GRGOD, 0 },
  { "unaffect" , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_UNAFFECT },
  { "uptime"   , POS_DEAD    , do_date     , LVL_IMMORT, SCMD_UPTIME },
  { "use"      , POS_SITTING , do_use      , 1, SCMD_USE },
  { "users"    , POS_DEAD    , do_users    , LVL_IMMORT, 0 },

  { "value"    , POS_STANDING, do_not_here , 0, 0 },
  { "version"  , POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION },
  { "violate"  , POS_RESTING , do_action   , 0, 0 },
  { "visible"  , POS_RESTING , do_visible  , 1, 0 },
  { "vnum"     , POS_DEAD    , do_vnum     , LVL_IMMORT, 0 },
  { "vstat"    , POS_DEAD    , do_vstat    , LVL_IMMORT, 0 },

  { "wake"     , POS_SLEEPING, do_wake     , 0, 0 },
  { "waltz"    , POS_RESTING , do_action   , 0, 0 },
  { "wave"     , POS_RESTING , do_action   , 0, 0 },
  { "wear"     , POS_RESTING , do_wear     , 0, 0 },
  { "weather"  , POS_RESTING , do_weather  , 0, 0 },
  { "wedgie"   , POS_RESTING , do_action   , 0, 0 },
  { "wee"      , POS_RESTING , do_action   , 0, 0 },
  { "weep"     , POS_RESTING , do_action   , 0, 0 },
  { "who"      , POS_DEAD    , do_who      , 0, 0 },
  { "whap"     , POS_RESTING , do_action   , 0, 0 },
  { "whimper"  , POS_RESTING , do_action   , 0, 0 },
  { "whoami"   , POS_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI },
  { "whod  "   , POS_DEAD    , do_whod     , LVL_IMMORT, 0  },
  { "whois"    , POS_DEAD    , do_whois    , 0, 0 },
  { "where"    , POS_RESTING , do_where    , LVL_IMMORT, 0 },
  { "whisper"  , POS_RESTING , do_spec_comm, 0, SCMD_WHISPER },
  { "whine"    , POS_RESTING , do_action   , 0, 0 },
  { "whistle"  , POS_RESTING , do_action   , 0, 0 },
  { "wield"    , POS_RESTING , do_wield    , 0, 0 },
  { "will"     , POS_SITTING , do_cast     , 1, 0 },
  { "wiggle"   , POS_STANDING, do_action   , 0, 0 },
  { "wimpy"    , POS_DEAD    , do_wimpy    , 0, 0 },
  { "wink"     , POS_RESTING , do_action   , 0, 0 },
  { "withdraw" , POS_STANDING, do_not_here , 1, 0 },
  { "wiznet"   , POS_DEAD    , do_wiznet   , 0, 0 },
  { ";"        , POS_DEAD    , do_wiznet   , 0, 0 },
  { "wizhelp"  , POS_SLEEPING, do_commands , LVL_IMMORT, SCMD_WIZHELP },
  { "wizlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST },
  { "wizlock"  , POS_DEAD    , do_wizlock  , LVL_IMPL-1, 0 },
  { "wnewbie"  , POS_DEAD    , do_newbie   , LVL_IMMORT, 0 },
  { "worship"  , POS_RESTING , do_action   , 0, 0 },
  { "write"    , POS_STANDING, do_write    , 1, 0 },

  { "yank"     , POS_STANDING, do_yank     , 0, 0 },
  { "yawn"     , POS_RESTING , do_action   , 0, 0 },
  { "yodel"    , POS_RESTING , do_action   , 0, 0 },
  { "yuball"   , POS_RESTING , do_action   , 0, 0 },

  { "zai"      , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_ZAI },
  { "zhen"     , POS_STANDING, do_kuji_kiri, 0, SKILL_KK_ZHEN },
  { "zedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_ZEDIT},
  { "zlist"    , POS_DEAD    , do_zlist    , LVL_BUILDER, 0 },
  { "zreset"   , POS_DEAD    , do_zreset   , LVL_BUILDER, 0 },

  { "\n", 0, 0, 0, 0 } };	/* this must be last */


char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

char *reserved[] =
{
  "a",
  "an",
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  extern int no_specials;
  char *line;

  if (!number(0,3))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  /* just drop to next line for hitting CR */
  skip_spaces(&argument);
  if (!*argument)
    return;

  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha(*argument)) {
    if (*argument == '\033[A')
      send_to_char("awesome!\r\n", ch);

    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument + 1;
  } else
    line = any_one_arg(argument, arg);

  /* otherwise, find the command */
  for (length = strlen(arg), cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strncmp(cmd_info[cmd].command, arg, length))
      if (GET_LEVEL(ch) >= cmd_info[cmd].minimum_level)
	break;

  if (*cmd_info[cmd].command == '\n')
    send_to_char("Huh?!?\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL-1)
    send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
  else if (cmd_info[cmd].command_pointer == NULL)
    send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC(ch) && cmd_info[cmd].minimum_level >= LVL_IMMORT)
    send_to_char("You can't use immortal commands while switched.\r\n", ch);
  else if (GET_POS(ch) < cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
      break;
    case POS_STUNNED:
      send_to_char("All you can do right now is think about the stars!\r\n", ch);
      break;
    case POS_SLEEPING:
      send_to_char("In your dreams, or what?\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("Maybe you should get on your feet first?\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
  } else if (no_specials || !special(ch, cmd, line))
    ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));
}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias *find_alias(struct alias *alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)	/* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
	return alias_list;

    alias_list = alias_list->next;
  }

  return NULL;
}


void free_alias(struct alias *a)
{
  if (a->alias)
    FREE(a->alias);
  if (a->replacement)
    FREE(a->replacement);
  FREE(a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
  char *repl;
  struct alias *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {	/* no argument specified -- list currently defined aliases */
    send_to_char("Currently defined aliases:\r\n", ch);
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
	sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
	send_to_char(buf, ch);
	a = a->next;
      }
    }
  } else {			/* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
      if (a == NULL)
	send_to_char("No such alias.\r\n", ch);
      else
	send_to_char("Alias deleted.\r\n", ch);
    } else {		/* otherwise, either add or redefine an alias */
      if (!str_cmp(arg, "alias")) {
	send_to_char("You can't alias 'alias'.\r\n", ch);
	return;
      }
      CREATE(a, struct alias, 1);
      a->alias = str_dup(arg);
      delete_doubledollar(repl);
      /* Serapis mod to avoid alias save crashing */
      if (strlen(repl)>120)
	{
	  send_to_char("Maximum alias length is 120 characters. Yours has "
		       "been truncated.\r\n",ch);
	  repl[120]='\0';
	}
      a->replacement = str_dup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
	a->type = ALIAS_COMPLEX;
      else
	a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      send_to_char("Alias added.\r\n", ch);
    }
  }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  temp = strtok(strcpy(buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
	strcpy(write_point, tokens[num]);
	write_point += strlen(tokens[num]);
      } else if (*temp == ALIAS_GLOB_CHAR) {
	strcpy(write_point, orig);
	write_point += strlen(orig);
      } else if ((*(write_point++) = *temp) == '$')	/* redouble $ for act safety */
	*(write_point++) = '$';
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias *a, *tmp;

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return 0;

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return 0;

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return 0;

  if (a->type == ALIAS_SIMPLE) {
    strcpy(orig, a->replacement);
    return 0;
  } else {
    perform_complex_alias(&d->input, ptr, a);
    return 1;
  }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, char **list, int exact)
{
  register int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;			/* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }

  return -1;
}


int is_number(char *str)
{
  while (*str)
    if (!isdigit(*(str++)))
      return 0;

  return 1;
}

/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}

/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
  char *read, *write;

  /* If the string has no dollar signs, return immediately */
  if ((write = strchr(string, '$')) == NULL)
    return string;

  /* Start from the location of the first dollar sign */
  read = write;

  while (*read)  /* until we reach the end of teh string... */
    if ((*(write++) = *(read++)) == '$') /* copy one char */
      if (*read == '$')
	read++; /* skip if we saw 2 $'s in a row */

  *write = '\0';

  return string;
}

/* Use this to control where your ansi markup language is used 
   Note that this will strip the & character from every piece of
   text you run through it, so use with caution. */

char *delete_ansi_controls(char *string)
{
  char *read, *write;

  if ((write = strchr(string, '&')) == NULL)
    return string;

  read = write;

  while (*read) 
  {
    if (*(read) != '&')
      *(write++) = *(read); 
    read++;
  }

  *write = '\0';

  return string;
}
  


int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while (*argument && !isspace(*argument)) {
      *(first_arg++) = LOWER(*argument);
      argument++;
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg); /* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 * 
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return 0;

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return 0;

  if (!*arg1)
    return 1;
  else
    return 0;
}

/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);
}

/* return first single-space delimited token in arg1; remainder of string in arg2 */
void one_space_half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);

  if (isspace(*temp))
    temp++;

  strcpy(arg2, temp);
}

/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
  int cmd;

  for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  /* special in room? */
  if (GET_ROOM_SPEC(ch->in_room) != NULL)
    if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return 1;

  /* script in room? */
  if (!IS_NPC(ch) && GET_ROOM_SCRIPT(ch->in_room) && ROOM_SCRIPT_FLAGGED(ch->in_room, RS_ONCMD)) {
    sprintf(buf, "%s%s", CMD_NAME, arg);
    if (run_script(ch, ch, NULL, &world[ch->in_room], buf, "oncmd", LT_ROOM))
      return 1;
  }

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++) {
    if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
      if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
	return 1;
    if (!IS_NPC(ch) && GET_EQ(ch, j) && GET_OBJ_RNUM(GET_EQ(ch, j)) != NOTHING)
      if (GET_OBJ_SCRIPT(GET_EQ(ch, j)) && OBJ_SCRIPT_FLAGGED(GET_EQ(ch, j), OS_ONCMD)) {
          sprintf(buf, "%s%s", CMD_NAME, arg);
          if (run_script(ch, ch, GET_EQ(ch, j), &world[ch->in_room], buf, "oncmd", LT_OBJ))
            return 1;
      }
  }

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content) {
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;
    if (!IS_NPC(ch) && GET_OBJ_RNUM(i) != NOTHING)
      if (GET_OBJ_SCRIPT(i) && OBJ_SCRIPT_FLAGGED(i, OS_ONCMD)) {
        sprintf(buf, "%s%s", CMD_NAME, arg);
        if (run_script(ch, ch, i, &world[ch->in_room], buf, "oncmd", LT_OBJ))
          return 1;
    }
  }

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room) {
    if (!MOB_FLAGGED(k, MOB_EXTRACT)) {
      if (GET_MOB_SPEC(k) != NULL)
        if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
          return 1;
      if (GET_MOB_SCRIPT(k) && MOB_SCRIPT_FLAGGED(k, MS_ONCMD) && !IS_NPC(ch)) {
        sprintf(buf, "%s%s", CMD_NAME, arg);
        if (run_script(ch, k, NULL, &world[ch->in_room], buf, "oncmd", LT_MOB))
          return 1;
      }
    }
  }

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;
    if (GET_OBJ_RNUM(i) != NOTHING) {
      if (GET_OBJ_SCRIPT(i) && OBJ_SCRIPT_FLAGGED(i, OS_ONCMD)) {
        sprintf(buf, "%s%s", CMD_NAME, arg);
        if (run_script(ch, ch, i, &world[ch->in_room], buf, "oncmd", LT_OBJ))
          return 1;
      }
    }
  }
  

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int 
find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return i;
  }

  return -1;
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha(*arg))
      return 1;

  if (!i)
    return 1;

  return 0;
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3

int
perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k)
    {
      next_k = k->next;

      if (k == d)
	continue;

      if (k->original && (GET_IDNUM(k->original) == id))
	{    /* switched char */
	  SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
	  STATE(k) = CON_CLOSE;
	  if (!target)
	    {
	      target = k->original;
	      mode = UNSWITCH;
	    }
	  if (k->character)
	    k->character->desc = NULL;
	  k->character = NULL;
	  k->original = NULL;
	}
      else if (k->character && (GET_IDNUM(k->character) == id))
	{
	  if (!target && STATE(k) == CON_PLAYING)
	    {
	      SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
	      target = k->character;
	      mode = USURP;
	    }
	  k->character->desc = NULL;
	  k->character = NULL;
	  k->original = NULL;
	  SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
	  STATE(k) = CON_CLOSE;
	}
    }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch)
    {
      next_ch = ch->next;

      if (IS_NPC(ch))
	continue;
      if (GET_IDNUM(ch) != id)
	continue;

      /* ignore chars with descriptors (already handled by above step) */
      if (ch->desc)
	continue;

      /* don't extract the target char we've found one already */
      if (ch == target)
	continue;

      /* we don't already have a target and found a candidate for switching */
      if (!target)
	{
	  target = ch;
	  mode = RECON;
	  continue;
	}

      /* we've found a duplicate - blow him away, dumping his eq in limbo. */
      if (ch->in_room != NOWHERE)
	char_from_room(ch);
      char_to_room(ch, 1);
      extract_char(ch);
    }

  /* no target for swicthing into was found - allow login to continue */
  if (!target)
    return 0;

  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
  STATE(d) = CON_PLAYING;

  switch (mode)
    {
    case RECON:
      SEND_TO_Q("Reconnecting.\r\n", d);
      if (has_mail(GET_IDNUM(d->character)))
        send_to_char("You have mail waiting.\r\n", d->character);
      act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
      break;
    case USURP:
      SEND_TO_Q("You take over your own body, already in use!\r\n", d);
      act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	  "$n's body has been taken over by a new spirit!",
	  TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
	      GET_NAME(d->character));
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
      break;
    case UNSWITCH:
      SEND_TO_Q("Reconnecting to unswitched char.", d);
      sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
      break;
    }

  return 1;
}

/* ************************************************************************** 
   NAME       : valid_user_class_choice()
   PARAMETERS : TYPE                NAME                     DESCRIPTION
                ----                ----                     -----------
		int                 race            user race
                int                 user_choice     numerical equiv of a class
		                                    or -1
   PURPOSE    : returns TRUE if the user picked a valid non-remort class
   RETURNS    : TRUE or FALSE
   HISTORY    : Created by dlkarnes 960808
   ************************************************************************ */
static int
valid_user_class_choice(int race, int user_choice)
{
  switch (user_choice)
    {
    case CLASS_NINJA:
      if (race != RACE_HUMAN)
	break;
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
    case CLASS_THIEF:
    case CLASS_WARRIOR:
    case CLASS_PSIONIC:
      return (TRUE);
    }
  return(FALSE);
}

/* deal with newcomers and other non-playing sockets */
void
nanny(struct descriptor_data *d, char *arg)
{
  char buf[128];
  char buf2[128];
  int player_i, load_result, new_char, clan_num;
  char tmp_name[MAX_INPUT_LENGTH];
  struct char_file_u tmp_store;
  extern int r_mortal_start_room;
  extern int r_immort_start_room;
  extern int r_frozen_start_room;
  extern const char *class_menu;
  extern const char *human_class_menu;
  extern const char *hometown_menu;
  extern const char *race_menu;
  extern const char *race_help;
  extern const char *help_elf;
  extern const char *help_dwarf;
  extern const char *help_human;
  extern const char *help_kender;
  extern const char *help_rakshasa;
  extern const char *help_ssaur;
  extern const char *help_minotaur;
  extern int max_bad_pws;
  int load_room;
  register int l;
  int load_char(char *name, struct char_file_u *char_element);
  int parse_class(char arg);
  
  skip_spaces(&arg);

  switch (STATE(d)) {

  /*. OLC states .*/
  case CON_OEDIT: 
    oedit_parse(d, arg);
    break;
  case CON_REDIT: 
    redit_parse(d, arg);
    break;
  case CON_ZEDIT: 
    zedit_parse(d, arg);
    break;
  case CON_MEDIT: 
    medit_parse(d, arg);
    break;
  case CON_SEDIT: 
    sedit_parse(d, arg);
    break;
  /*. End of OLC states .*/

  case CON_GET_NAME:		/* wait for input of name */
    if (d->character == NULL)
      {
	CREATE(d->character, struct char_data, 1);
	clear_char(d->character);
	CREATE(d->character->player_specials, struct player_special_data, 1);
	d->character->desc = d;
      }
    if (!*arg)
      close_socket(d);
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
	  strlen(tmp_name) > MAX_NAME_LENGTH || !Valid_Name(tmp_name) ||
	  fill_word(strcpy(buf, tmp_name)) || reserved_word(buf))
	{
	  SEND_TO_Q("Invalid name, please try another.\r\n"
		    "Name: ", d);
	  return;
	}
      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) 
      {
	store_to_char(&tmp_store, d->character);
	GET_PFILEPOS(d->character) = player_i;
	d->character->wait = 1;

	if (PLR_FLAGGED(d->character, PLR_DELETED)) 
        {
	  free_char(d->character);
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  CREATE(d->character->player_specials, struct player_special_data, 1);
	  d->character->desc = d;
          for (l = 0; *(tmp_name + l); l++)   /* convert to all lowercase */
            *(tmp_name + l) = LOWER(*(tmp_name + l));
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  GET_PFILEPOS(d->character) = player_i;
          strcpy((player_table + player_i)->name, tmp_name);  /* update the player table */
 	  SEND_TO_Q("Please remember to choose an appropriate fantasy-oriented name.\r\n", d);
	  sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NAME_CNFRM;
	}
	else
	  {
	    /* undo it just in case they are set */
	    REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
	    REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	    REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRYO);

	    SEND_TO_Q("Password: ", d);
	    echo_off(d);
	    d->idle_tics = 0;
	    STATE(d) = CON_PASSWORD;
	  }
      }
      else
	{
	  /* player unknown -- make new character */

	  if (!Valid_Name(tmp_name))
	    {
	      SEND_TO_Q("Invalid name, please try another.\r\n", d);
	      SEND_TO_Q("Name: ", d);
	      return;
	    }
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));

	  SEND_TO_Q("Please remember to choose an appropriate fantasy-oriented name.\r\n", d);
	  sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NAME_CNFRM;
	}
    }
    break;
  case CON_NAME_CNFRM:		/* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y')
      {
	if (isbanned(d->host) >= BAN_NEW)
	  {
	    sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
		    GET_NAME(d->character), d->host);
	    mudlog(buf, NRM, LVL_GOD, TRUE);
	    SEND_TO_Q("Sorry, new characters are not "
		      "allowed from your site!\r\n", d);
	    STATE(d) = CON_CLOSE;
	    return;
	  }
	if (game_restrict)
	  {
	    SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n",
		      d);
	    sprintf(buf, "Request for new char %s denied from [%s] (wizlock)",
		    GET_NAME(d->character), d->host);
	    mudlog(buf, NRM, LVL_GOD, TRUE);
	    STATE(d) = CON_CLOSE;
	    return;
	  }
	SEND_TO_Q("New character.\r\n", d);
	sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
	SEND_TO_Q(buf, d);
	echo_off(d);
	STATE(d) = CON_NEWPASSWD;
      }
    else if (*arg == 'n' || *arg == 'N')
      {
	SEND_TO_Q("Okay, what IS it, then? ", d);
	FREE(d->character->player.name);
	d->character->player.name = NULL;
	STATE(d) = CON_GET_NAME;
      }
    else
      {
	SEND_TO_Q("Please type Yes or No: ", d);
      }
    break;
  case CON_PASSWORD:		/* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */

    echo_on(d);    /* turn echo back on */

    if (!*arg)
      close_socket(d);
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)),
		  GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
	sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	mudlog(buf, BRF, LVL_GOD, TRUE);
	GET_BAD_PWS(d->character)++;
	save_char(d->character, NOWHERE);
	if (++(d->bad_pws) >= max_bad_pws) {	/* 3 strikes and you're out. */
	  SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
	  STATE(d) = CON_CLOSE;
	} else {
	  SEND_TO_Q("Wrong password.\r\nPassword: ", d);
	  echo_off(d);
	}
	return;
      }
      /* password was correct */
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;
      d->bad_pws = 0;

      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	SEND_TO_Q("Sorry, this char has not been cleared for login from "
		  "your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_GOD, TRUE);
	return;
      }
      if (GET_LEVEL(d->character) < game_restrict) {
	SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n",
		  d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_GOD, TRUE);
	return;
      }
      /* check and make sure no other copies of this player are logged in */
      if (perform_dupe_check(d))
	return;

      if (GET_LEVEL(d->character) >= LVL_IMMORT)
	SEND_TO_Q(imotd, d);
      else
	SEND_TO_Q(motd, d);

      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      if (PLR_FLAGGED(d->character, PLR_INVSTART))
	GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);

      if (load_result) {
	sprintf(buf, "\r\n\r\n\007\007\007"
		"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		CCRED(d->character, C_SPR), load_result,
		(load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	SEND_TO_Q(buf, d);
	GET_BAD_PWS(d->character) = 0;
      }
      SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_NAME(d->character)),
	    MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    SEND_TO_Q("\r\nPlease retype password: ", d);
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;

    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
		MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      if (STATE(d) == CON_CNFPASSWD)
	STATE(d) = CON_NEWPASSWD;
      else
	STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);

    if (STATE(d) == CON_CNFPASSWD)
      {
	SEND_TO_Q("Do you want ANSI color (Y/N)? ", d);
	STATE(d) = CON_COLOR;
      }
    else
      {
	save_char(d->character, NOWHERE);
	echo_on(d);
	SEND_TO_Q("\r\nDone.\n\r", d);
	SEND_TO_Q(MENU, d);
	STATE(d) = CON_MENU;
      }

    break;

  case CON_COLOR:
    switch (*arg)
      {
      case 'y':
      case 'Y':
	SET_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_1);
	SET_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_2);
	break;
      case 'n':
      case 'N':
	break;
      default:
	SEND_TO_Q("Please answer Y or N.\r\n"
		  "Do you want ANSI color (Y/N)? ", d);
	return;
      }
      SEND_TO_Q("What is your sex (M/F)? ", d);
      STATE(d) = CON_QSEX;
      break;
      
  case CON_QSEX:		/* query sex of new user         */
    switch (*arg)
      {
      case 'm':
      case 'M':
	d->character->player.sex = SEX_MALE;
	break;
      case 'f':
      case 'F':
	d->character->player.sex = SEX_FEMALE;
	break;
      default:
	SEND_TO_Q("That is not a sex..\r\n"
		  "What IS your sex? ", d);
	return;
	break;
      }

    SEND_TO_Q(race_menu, d);
    SEND_TO_Q("\r\nRace: ", d);
    STATE(d) = CON_QRACE;
    break;
   
  case CON_QRACE:		/* query race of new user      */ 
  {
    switch (*arg)
      {
      case '?':
	if (strlen(arg)>1)
	{
	  switch(arg[1])
	  {
	  case 'h': case 'H': SEND_TO_Q(help_human, d); break;
          case 'e': case 'E': SEND_TO_Q(help_elf, d); break;
          case 'd': case 'D': SEND_TO_Q(help_dwarf, d); break;
          case 'r': case 'R': SEND_TO_Q(help_rakshasa, d); break;
          case 's': case 'S': SEND_TO_Q(help_ssaur, d); break;
          case 'm': case 'M': SEND_TO_Q(help_minotaur, d); break;
          case 'k': case 'K': SEND_TO_Q(help_kender, d); break;
          default: SEND_TO_Q("That is not a race..\r\n",d); break;
	  }
    	  SEND_TO_Q(race_menu, d);
    	  SEND_TO_Q("\r\nRace: ", d);
	}
	else
	{
    	  SEND_TO_Q(race_help, d);
    	  SEND_TO_Q(race_menu, d);
    	  SEND_TO_Q("\r\nRace: ", d);
	}
	return;
      case 'h': case 'H': GET_PLR_RACE(d->character) = RACE_HUMAN; break;
      case 'e': case 'E': GET_PLR_RACE(d->character) = RACE_ELF; break;
      case 'd': case 'D': GET_PLR_RACE(d->character) = RACE_DWARF; break;
      case 'k': case 'K': GET_PLR_RACE(d->character) = RACE_KENDER; break;
      case 'm': case 'M': GET_PLR_RACE(d->character) = RACE_MINOTAUR; break;
      case 'r': case 'R': GET_PLR_RACE(d->character) = RACE_RAKSHASA; break;
      case 's': case 'S': GET_PLR_RACE(d->character) = RACE_SSAUR; break;
      default: SEND_TO_Q("That is not a race..\r\nWhat IS your race? ", d);
	return;
      } 
    if (GET_PLR_RACE(d->character) == RACE_HUMAN)
      SEND_TO_Q(human_class_menu, d);
    else
      SEND_TO_Q(class_menu, d);
    SEND_TO_Q("\r\nClass: ", d);
    STATE(d) = CON_QCLASS;
    break;
  }
  case CON_QCLASS:
    load_result = parse_class(*arg);
    if (!valid_user_class_choice(GET_PLR_RACE(d->character), load_result))
      {
	SEND_TO_Q("\r\nThat's not a class.\r\nClass: ", d);
	return;
      }
    else
      GET_CLASS(d->character) = load_result;

    SEND_TO_Q(hometown_menu, d);
    SEND_TO_Q("\r\nSelect: ", d);
    STATE(d) = CON_HOMETOWN;
    break;

  case CON_HOMETOWN:
    switch (*arg) {
      case 'k': case 'K': GET_HOME(d->character) = 1; break;
      case 'o': case 'O': GET_HOME(d->character) = 2; break;
      case 'a': case 'A': GET_HOME(d->character) = 3; break;
      default: SEND_TO_Q("Invalid choice!\r\nSelect: ", d);
      return;
    }
    STATE(d) = CON_ROLLABL1;

  case CON_ROLLABL1:
    SEND_TO_Q("\r\nYour ability scores:\r\n", d);
    roll_real_abils(d->character);
    sprintf(buf2,"  Str: %-13s Dex: %-13s Int: %-13s"
                 "\r\n  Wis: %-13s Con: %-13s Cha: %-13s\r\n",
              abil_names[GET_STR(d->character)], 
              abil_names[GET_DEX(d->character)],
              abil_names[GET_INT(d->character)], 
              abil_names[GET_WIS(d->character)],
              abil_names[GET_CON(d->character)], 
              abil_names[GET_CHA(d->character)]);
    SEND_TO_Q(buf2, d);
    SEND_TO_Q("\r\nPress 'Y' to keep these stats, and 'N' to"
              " reroll:", d);
    STATE(d) = CON_ROLLABL2;
    break;

  case CON_ROLLABL2:  
    switch (*arg) {
     case 'Y': case 'y': 
       if (GET_PFILEPOS(d->character) < 0)
         GET_PFILEPOS(d->character) = create_entry(GET_NAME(d->character));
       init_char(d->character);
       save_char(d->character, NOWHERE);
       SEND_TO_Q(motd, d);
       SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
       STATE(d) = CON_RMOTD;
       break;
     case 'N': case 'n':
      SEND_TO_Q("\r\nYour ability scores:\r\n", d);
      roll_real_abils(d->character);
      sprintf(buf2,"  Str: %-13s Dex: %-13s Int: %-13s"
                   "\r\n  Wis: %-13s Con: %-13s Cha: %-13s\r\n",
                abil_names[GET_STR(d->character)], 
                abil_names[GET_DEX(d->character)],
                abil_names[GET_INT(d->character)], 
                abil_names[GET_WIS(d->character)],
                abil_names[GET_CON(d->character)], 
                abil_names[GET_CHA(d->character)]);
      SEND_TO_Q(buf2, d);
      SEND_TO_Q("\r\nPress 'Y' to keep these stats, and 'N' to"
                " reroll:", d);
      return;
     default:
       SEND_TO_Q("Invalid choice! Select 'Y' or 'N':", d);
       return;
    }
 
    sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, LVL_IMMORT, TRUE);
    break;

    

  case CON_RMOTD:		/* read CR after printing motd   */
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_MENU;
    break;

  case CON_MENU:		/* get selection from main menu  */
    switch (*arg)
      {
      case '0':
        SEND_TO_Q("Goodbye.\r\n", d);
        STATE(d) = CON_CLOSE;
	break;

      case '1':
	new_char = FALSE;
	reset_char(d->character);
        if (!GET_CON(d->character))
        {
         SEND_TO_Q("You are too unhealthy to play!\r\n", d);
         STATE(d) = CON_CLOSE;
         break;
        }
	if (PLR_FLAGGED(d->character, PLR_INVSTART))
	  GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
	if ((load_result = Crash_load(d->character)))
	  d->character->in_room = NOWHERE;
	save_char(d->character, NOWHERE);
	send_to_char(WELC_MESSG, d->character);
	d->character->next = character_list;
	character_list = d->character;

	if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
	  load_room = real_room(load_room);

	/* If char was saved with NOWHERE, or real_room above failed... */
	if (load_room == NOWHERE) {
	  if (GET_LEVEL(d->character) >= LVL_IMMORT) {
	    load_room = r_immort_start_room;
	  } else {
	    load_room = r_mortal_start_room;
	  }
	}

	if (PLR_FLAGGED(d->character, PLR_FROZEN))
	  load_room = r_frozen_start_room;

	/* Serapis 100896 Dunno if this is a real bug, but heres the fix */	
	if (load_room < 0) 
	  load_room = r_mortal_start_room;	

	char_to_room(d->character, load_room);
	act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

	STATE(d) = CON_PLAYING;
	if (!GET_LEVEL(d->character)) {
	  new_char = TRUE;
	  do_start(d->character);
	}
	else
        {
	  if (GET_INFOBAR(d->character) == INFOBAR_ON)
            InfoBarOn(d->character);
	  look_at_room(d->character, 0);
        }
	if (has_mail(GET_IDNUM(d->character)))
	  send_to_char("You have mail waiting.\r\n", d->character);
	if (load_result == 2) {	/* rented items lost */
	  stc("\r\n\007You could not afford your rent!\r\n"
	      "Your possesions have been donated to the Salvation Army!\r\n",
	       d->character);
	}
	/* sanity check for con loss Serapis 970604*/
 	if (GET_ORIG_CON(d->character)<1)
		GET_ORIG_CON(d->character)=(d->character)->real_abils.con;
	d->has_prompt = 0;
	read_aliases(d->character);
	if (GET_LEVEL(d->character)>=LEVEL_IMMORT)
	  read_poofs(d->character);
	if (new_char)
	  {
	    char_from_room(d->character);
	    char_to_room(d->character, real_room(8099));
	    look_at_room(d->character, 0);
	  }
	break;

      case '2':
      if (d->character->player.description) {
	write_to_output(d, "Current description:\r\n%s", d->character->player.description);
	/*
	 * Don't free this now... so that the old description gets loaded
	 * as the current buffer in the editor.  Do setup the ABORT buffer
	 * here, however.
	 *
	 * free(d->character->player.description);
	 * d->character->player.description = NULL;
	 */
	d->backstr = strdup(d->character->player.description);
      }
      write_to_output(d, "Enter the new text you'd like others to see when they look at you.\r\n");
      send_editor_help(d);
      d->str = &d->character->player.description;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_EXDESC;
      break;

      case '3':
	page_string(d, background, 0);
	STATE(d) = CON_RMOTD;
	break;

      case '4':
	SEND_TO_Q("\r\nEnter your old password: ", d);
	echo_off(d);
	STATE(d) = CON_CHPWD_GETOLD;
	break;

      case '5':
	SEND_TO_Q("\r\nEnter your password for verification: ", d);
	echo_off(d);
	STATE(d) = CON_DELCNF1;
	break;

      default:
	SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
	SEND_TO_Q(MENU, d);
	break;
      }

    break;

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      echo_on(d);
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
      return;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    break;

  case CON_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    } else {
      SEND_TO_Q("\r\n", d);
      SEND_TO_Q(GET_NAME(d->character), d);
      SEND_TO_Q(":\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
		"ARE YOU ABSOLUTELY SURE?\r\n\r\n"
		"Please type \"yes\" to confirm: ", d);
      STATE(d) = CON_DELCNF2;
    }
    break;

  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
	SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
	SEND_TO_Q("Character not deleted.\r\n\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (GET_LEVEL(d->character) < LVL_GRGOD)
	SET_BIT_AR(PLR_FLAGS(d->character), PLR_DELETED);
      if ((clan_num = find_clan_by_id(GET_CLAN(d->character)))>0)
      {
        GET_CLAN(d->character) = 0;
        GET_CLAN_RANK(d->character) = 0;
        clan[clan_num].members--;
        clan[clan_num].power-=GET_LEVEL(d->character);
      }
      save_char(d->character, NOWHERE);
      Crash_delete_file(GET_NAME(d->character));
      Alias_delete_file(GET_NAME(d->character));
      sprintf(buf, "Character '%s' deleted!\r\n"
	      "Goodbye.\r\n", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
	      GET_LEVEL(d->character));
      mudlog(buf, NRM, LVL_GOD, TRUE);
      STATE(d) = CON_CLOSE;
      return;
    } else {
      SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }
    break;

  case CON_CLOSE:
    close_socket(d);
    break;

  default:
    log("SYSERR: Nanny: illegal state of con'ness; closing connection");
    close_socket(d);
    break;
  }
}
