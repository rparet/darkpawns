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

/* $Id: interpreter.c 1330 2007-08-05 01:18:21Z jravn $ */

#define INTERPRETER_C
#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "screen.h"
#include "olc.h"
#include "faction.h"
#include "events.h"

extern char *motd;
extern char *motd_mxp;
extern char *imotd;
extern char *mxp;
extern char *background;
extern char *creation;
extern char *MENU_TITLE;
extern char *MENU_ENDING;
extern char *ACCOUNT_MENU;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern int game_restrict;
extern sh_int start_room;

/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
void create_start(int lifestyle, struct char_data *ch);
void create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
int isbanned(char *hostname, bool site);
int valid_name(char *newname);
void oedit_parse(struct descriptor_data *d, char *arg);
void redit_parse(struct descriptor_data *d, char *arg);
void zedit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void tedit_parse(struct descriptor_data *d, char *arg);
void sql_read_aliases(struct char_data *ch);
void sql_remove_alias(struct char_data *ch, char *alias);
void sql_read_poofs(struct char_data *ch);
void InfoBarOn(struct char_data *ch);
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);
int sql_has_mail(long recipient);
void sql_write_notes(char *name, char *arg, struct char_data *vict);

/* prototypes for all do_x functions. */
ACMD(skill_wrap);
ACMD(ambush_event);
ACMD(quit_event);

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
ACMD(do_bash);
ACMD(do_bearhug);
ACMD(do_behead);
ACMD(do_berserk);
ACMD(do_bind);
ACMD(do_bite);
ACMD(do_build);
ACMD(do_bury);
ACMD(do_carve);
ACMD(do_cast);
ACMD(do_charge);
ACMD(do_checkloadstatus);
ACMD(do_circle);
ACMD(do_clan);
ACMD(do_ctell);
ACMD(do_color);
ACMD(do_combine);
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
ACMD(do_draw);
ACMD(do_drop);
ACMD(do_dream);
ACMD(do_duck);
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
ACMD(do_guard);
ACMD(do_hcontrol);
ACMD(do_headbutt);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_home);
ACMD(do_house);
ACMD(do_idlist);
ACMD(do_idlist2);
ACMD(do_inactive);
ACMD(do_info);
ACMD(do_infobar);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_jump);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_kuji_kiri);
ACMD(do_last);
ACMD(do_learn);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_lines);
ACMD(do_load);
ACMD(do_look);
ACMD(do_map);
ACMD(do_mindlink);
ACMD(do_mlist);
ACMD(do_mold);
ACMD(do_move);
ACMD(do_neckbreak);
ACMD(do_newbie);
ACMD(do_not_here);
ACMD(do_notes);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_olist);
ACMD(do_order);
ACMD(do_otouch);
ACMD(do_outlaw);
ACMD(do_page);
ACMD(do_parry);
ACMD(do_palm);
ACMD(do_peek);
ACMD(do_poofset);
ACMD(do_point);
ACMD(do_pour);
ACMD(do_practice);
ACMD(do_prompt);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_race_say);
ACMD(do_reboot);
ACMD(do_recall);
ACMD(do_refuge);
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
ACMD(do_save);
ACMD(do_say);
ACMD(do_score);
ACMD(do_scout);
ACMD(do_scribe);
ACMD(do_scrounge);
ACMD(do_send);
ACMD(do_serpent_kick);
ACMD(do_set);
ACMD(do_sethunt);
ACMD(do_shadow);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_skin);
ACMD(do_slay);
ACMD(do_sleep);
ACMD(do_sleeper);
ACMD(do_slug);
ACMD(do_smackheads);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_scroll);
ACMD(do_spike);
ACMD(do_split);
ACMD(do_squelch);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_strike);
ACMD(do_string);
ACMD(do_subdue);
ACMD(do_swear);
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
ACMD(do_tlist);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_transform);
ACMD(do_tree);
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
  { "aid"      , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "alias"    , POS_DEAD    , do_alias    , 0, 0 },
  { "alter"    , POS_FIGHTING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "accuse"   , POS_SITTING , do_action   , 0, 0 },
  { "agree"    , POS_RESTING , do_action   , 0, 0 },
  { "ambush"   , POS_STANDING, skill_wrap  , 0, SCMD_COMBAT },
  { "apologize", POS_RESTING , do_action   , 0, 0 },
  { "applaud"  , POS_RESTING , do_action   , 0, 0 },
  { "anguish"  , POS_RESTING , do_action   , 0, 0 },
  { "appraise" , POS_RESTING , skill_wrap  , 0, SCMD_NONCOMBAT },
  { "assist"   , POS_FIGHTING, do_assist   , 1, 0 },
  { "auction"  , POS_SLEEPING, do_gen_comm , 0, SCMD_AUCTION },
  { "auto"     , POS_DEAD    , do_auto     , 0, 0 },

  { "bounce"   , POS_STANDING, do_action   , 0, 0 },
  { "backstab" , POS_STANDING, skill_wrap  , 0, SCMD_COMBAT },
  { "bah"      , POS_RESTING , do_action   , 0, 0 },
  { "ban"      , POS_DEAD    , do_ban      , LVL_GRGOD, 0 },
  { "balance"  , POS_STANDING, do_not_here , 1, 0 },
  { "bash"     , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "bearhug"  , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "beckon"   , POS_RESTING , do_action   , 0, 0 },
  { "behead"   , POS_STANDING, do_behead   , 0, 0 },
  { "beg"      , POS_RESTING , do_action   , 0, 0 },
  { "bellow"   , POS_RESTING , do_action   , 0, 0 },
  { "berserk"  , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "bind"     , POS_RESTING , do_bind     , 1, 0 },
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
  { "brb"      , POS_RESTING , do_action   , 0, 0 },
  { "build"    , POS_DEAD    , do_build    , LVL_BUILDER, 0 },
  { "burp"     , POS_RESTING , do_action   , 0, 0 },
  { "bury"     , POS_STANDING, do_bury     , 0, 0 },
  { "buy"      , POS_STANDING, do_not_here , 0, 0 },
  { "bug"      , POS_DEAD    , do_gen_write, 0, SCMD_BUG },

  { "cackle"   , POS_RESTING , do_action   , 0, 0 },
  { "cast"     , POS_SITTING , do_cast     , 1, 0 },
  { "carve"    , POS_STANDING, do_carve    , 0, 0 },
  { "chuckle"  , POS_RESTING , do_action   , 0, 0 },
  { "chide"    , POS_RESTING , do_action   , 0, 0 },
  { "charge"   , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "circle"   , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "cheer"    , POS_RESTING , do_action   , 0, 0 },
  { "check"    , POS_STANDING, do_not_here , 1, 0 }, 
  { "checkload", POS_DEAD    , do_checkloadstatus, LVL_IMMORT, 0 },
  { "close"    , POS_SITTING , do_gen_door , 0, SCMD_CLOSE },
  { "clap"     , POS_RESTING , do_action   , 0, 0 },
  { "cls"      , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "consider" , POS_RESTING , do_consider , 0, 0 }, 
  { "color"    , POS_DEAD    , do_color    , 0, 0 },
  { "collect"  , POS_RESTING , do_not_here , 0, 0 },
  { "comfort"  , POS_RESTING , do_action   , 0, 0 },
  { "comb"     , POS_RESTING , do_action   , 0, 0 }, 
  { "commands" , POS_DEAD    , do_commands , 0, SCMD_COMMANDS },
  { "compare"  , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "cough"    , POS_RESTING , do_action   , 0, 0 },
  { "credits"  , POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS },
  { "cringe"   , POS_RESTING , do_action   , 0, 0 },
  { "cry"      , POS_RESTING , do_action   , 0, 0 },
  { "cuddle"   , POS_RESTING , do_action   , 0, 0 },
  { "curse"    , POS_RESTING , do_action   , 0, 0 },
  { "curtsey"  , POS_STANDING, do_action   , 0, 0 },
  { "cutthroat", POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },

  { "dance"    , POS_STANDING, do_action   , 0, 0 },
  { "date"     , POS_DEAD    , do_date     , LVL_IMMORT, SCMD_DATE },
  { "daydream" , POS_SLEEPING, do_action   , 0, 0 }, 
  { "dark"     , POS_DEAD    , do_dark     , LVL_IMMORT, 0 },
  { "dc"       , POS_DEAD    , do_dc       , LVL_IMMORT+1, 0 },
  { "deposit"  , POS_STANDING, do_not_here , 1, 0 },
  { "search"   , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "diagnose" , POS_RESTING , do_diagnose , 0, 0 },
  { "dig"      , POS_STANDING, do_dig      , 0, 0 },
  { "disarm"   , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "disembowel", POS_FIGHTING, skill_wrap , 0, SCMD_COMBAT },
  { "dismount" , POS_STANDING, do_dismount , 0, 0 },
  { "display"  , POS_DEAD    , do_display  , 0, 0 },
  { "dns"      , POS_DEAD    , do_dns      , LVL_IMPL-1, 0 },
  { "doh"      , POS_RESTING , do_action   , 0, 0 },
  { "donate"   , POS_RESTING , do_drop     , 0, SCMD_DONATE },
  { "drink"    , POS_RESTING , do_drink    , 0, 0 },
  { "draw"     , POS_RESTING , do_draw     , 0, 0 },
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
  { "flesh"    , POS_FIGHTING, skill_wrap  , 0, SCMD_NONCOMBAT },
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
  { "goto"     , POS_SLEEPING, do_goto     , LVL_IMMORT, 0 },
  { "gossip"   , POS_SLEEPING, do_gen_comm , 0, SCMD_GOSSIP },
  { "gold"     , POS_SLEEPING, do_coins    , 0, 0 },
  { "goose"    , POS_RESTING , do_action   , 0, 0 }, 
  { "group"    , POS_SLEEPING, do_group    , 1, 0 },
  { "grats"    , POS_SLEEPING, do_gen_comm , 0, SCMD_GRATZ },
  { "greet"    , POS_RESTING , do_action   , 0, 0 },
  { "grin"     , POS_RESTING , do_action   , 0, 0 },
  { "grimace"  , POS_RESTING , do_action   , 0, 0 },
  { "groan"    , POS_RESTING , do_action   , 0, 0 },
  { "groinrip" , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "grope"    , POS_RESTING , do_action   , 0, 0 },
  { "grovel"   , POS_RESTING , do_action   , 0, 0 },
  { "growl"    , POS_RESTING , do_action   , 0, 0 },
  { "grumble"  , POS_RESTING , do_action   , 0, 0 },
  { "gsay"     , POS_SLEEPING, do_gsay     , 0, 0 },
  { "gtell"    , POS_SLEEPING, do_gsay     , 0, 0 },
  { "guard"    , POS_STANDING, do_guard    , 0, 0 }, 

  { "help"     , POS_DEAD    , do_help     , 0, 0 }, 
  { "headbutt" , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT }, 
  { "?"        , POS_DEAD    , do_help     , 0, 0 },
  { "heh"      , POS_RESTING , do_action   , 0, 0 },
  { "handbook" , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_HANDBOOK },
  { "hcontrol" , POS_DEAD    , do_hcontrol , LVL_GRGOD, 0 },
  { "hiccup"   , POS_RESTING , do_action   , 0, 0 },
  { "hide"     , POS_RESTING , skill_wrap  , 0, SCMD_NONCOMBAT },
  { "highfive" , POS_RESTING , do_action   , 0, 0 },
  { "hiss"     , POS_RESTING , do_action   , 0, 0 }, 
  { "hold"     , POS_RESTING , do_grab     , 1, 0 },
  { "holler"   , POS_RESTING , do_gen_comm , 1, SCMD_HOLLER }, 
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
  { "idlist"   , POS_DEAD    , do_idlist   , LVL_GRGOD, 0 },
  { "imotd"    , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_IMOTD },
  { "immlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST },
  { "inactive" , POS_SLEEPING, do_inactive , 0, 0 },
  /* { "info"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO }, */
  { "infobar"  , POS_DEAD    , do_infobar  , 0, 0 },
  { "insult"   , POS_RESTING , do_insult   , 0, 0 }, 
  { "invis"    , POS_DEAD    , do_invis    , LVL_IMMORT, 0 },

  { "junk"     , POS_RESTING , do_drop     , 0, SCMD_JUNK },
  { "jeer"     , POS_RESTING , do_action   , 0, 0 },

  { "kill"     , POS_FIGHTING, do_kill     , 0, 0 },
  { "kick"     , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "kiss"     , POS_RESTING , do_action   , 0, 0 },
  { "kneel"    , POS_STANDING, do_action   , 0, 0 },
  
  { "look"     , POS_RESTING , do_look     , 0, SCMD_LOOK },
  { "laugh"    , POS_RESTING , do_action   , 0, 0 },
  { "lambada"  , POS_STANDING, do_action   , 0, 0 },
  { "last"     , POS_DEAD    , do_last     , LVL_GOD-1, 0 },
  { "learn"    , POS_RESTING , do_learn    , 0, 0 }, 
  { "leave"    , POS_STANDING, do_leave    , 0, 0 },
  { "leer"     , POS_RESTING , do_action   , 0, 0 },
  { "levels"   , POS_DEAD    , do_levels   , LVL_IMPL, 0 }, 
  { "list"     , POS_STANDING, do_not_here , 0, 0 },
  { "listen"   , POS_RESTING , do_action   , 0, 0 },
  { "lick"     , POS_RESTING , do_action   , 0, 0 },
  { "lines"    , POS_DEAD    , do_lines    , 0, 0 }, 
  { "lock"     , POS_SITTING , do_gen_door , 0, SCMD_LOCK },
  { "load"     , POS_DEAD    , do_load     , LVL_IMMORT, 0 },
  { "love"     , POS_RESTING , do_action   , 0, 0 }, 

  { "map"      , POS_SLEEPING, do_map      , LVL_IMMORT, 0 },
  { "mindlink" , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "mix"      , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
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
  { "mutter"   , POS_RESTING , do_action   , 0, 0 },
  

  { "neckbreak", POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "news"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS },
  { "nibble"   , POS_RESTING , do_action   , 0, 0 },
  { "nod"      , POS_RESTING , do_action   , 0, 0 }, 
  { "nosummon" , POS_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON },
  { "notes"    , POS_DEAD    , do_notes    , LVL_IMMORT, 0 },
  { "notitle"  , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_NOTITLE },
  { "noogie"   , POS_RESTING , do_action   , 0, 0 },
  { "nudge"    , POS_RESTING , do_action   , 0, 0 },
  { "nuzzle"   , POS_RESTING , do_action   , 0, 0 },

  { "order"    , POS_RESTING , do_order    , 1, 0 },
  { "orgasm"   , POS_RESTING , do_otouch   , LVL_IMMORT, 0 }, 
  { "open"     , POS_SITTING , do_gen_door , 0, SCMD_OPEN },
  { "olc"      , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SAVEINFO },
  { "olist"    , POS_DEAD    , do_olist    , LVL_BUILDER, 0},
  { "oedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_OEDIT},
  { "outla"    , POS_SLEEPING, do_outlaw   , 0, 0 },
  { "outlaw"   , POS_SLEEPING, do_outlaw   , 0, SCMD_OUTLAW }, 

  { "put"      , POS_RESTING , do_put      , 0, 0 },
  { "pace"     , POS_RESTING , do_action   , 0, 0 },
  { "pant"     , POS_RESTING , do_action   , 0, 0 },
  { "pat"      , POS_RESTING , do_action   , 0, 0 },
  { "page"     , POS_DEAD    , do_page     , LVL_IMMORT, 0 },
  { "palm"     , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "parry"    , POS_DEAD    , skill_wrap  , 0, SCMD_COMBAT },
  { "pardon"   , POS_DEAD    , do_wizutil  , 1, SCMD_PARDON },
  { "peek"     , POS_RESTING , skill_wrap  , 0, SCMD_NONCOMBAT },
  { "peer"     , POS_RESTING , do_action   , 0, 0 },
  { "pick"     , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
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
  { "prompt"   , POS_DEAD    , do_prompt   , 0, 0 },
  { "pray"     , POS_SITTING , do_action   , 0, 0 },
  { "puke"     , POS_RESTING , do_action   , 0, 0 },
  { "punch"    , POS_RESTING , do_action   , 0, 0 },
  { "purr"     , POS_RESTING , do_action   , 0, 0 }, 
  { "purge"    , POS_DEAD    , do_purge    , LVL_IMMORT, 0 },
  { "push"     , POS_STANDING, do_not_here , 0, 0 },

  { "qecho"    , POS_DEAD    , do_qcomm    , LVL_IMMORT, SCMD_QECHO }, 
  { "qui"      , POS_DEAD    , do_quit     , 0, 0 },
  { "quit"     , POS_DEAD    , do_quit     , 0, SCMD_QUIT },
  { "qsay"     , POS_SLEEPING, do_qcomm    , 0, SCMD_QSAY },

  { "rest"     , POS_RESTING , do_rest     , 0, 0 }, 
  { "report"   , POS_RESTING , do_report   , 0, 0 },
  { "raise"    , POS_RESTING , do_action   , 0, 0 }, 
  { "read"     , POS_RESTING , do_look     , 0, SCMD_READ },
  { "reel"     , POS_RESTING , do_action   , 0, 0 }, 
  { "reload"   , POS_DEAD    , do_reboot   , LVL_IMPL-1, 0 },
  { "recite"   , POS_RESTING , do_use      , 0, SCMD_RECITE },
  { "recharge" , POS_STANDING, do_not_here , 0, 0 }, 
  { "recall"   , POS_RESTING , do_recall   , 0, 0 },
  { "receive"  , POS_STANDING, do_not_here , 1, 0 },
  { "refuge"   , POS_RESTING,  skill_wrap  , 0, SCMD_NONCOMBAT },
  { "remove"   , POS_RESTING , do_remove   , 0, 0 },
  { "remort"   , POS_STANDING, do_not_here , 0, 0 },
  { "ren"      , POS_RESTING , do_action   , 0, 0 },
  { "reply"    , POS_DEAD    , do_reply    , 0, 0 },
  { "."        , POS_SLEEPING, do_reply    , 0, 0 },
  { "rescue"   , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "restore"  , POS_DEAD    , do_restore  , LVL_GOD-1, 0 },
  { "retreat"  , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "retrieve" , POS_STANDING, do_not_here,  1, 0 },
  { "return"   , POS_DEAD    , do_return   , 0, 0 }, 
  { "redit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_REDIT},
  { "review"   , POS_DEAD    , do_review   , 0, 0 },
  { "rlist"    , POS_DEAD    , do_rlist    , LVL_BUILDER, 0},
  { "roar"     , POS_RESTING , do_action   , 0, 0 },
  { "rofl"     , POS_RESTING , do_action   , 0, 0 },
  { "roll"     , POS_RESTING , do_action   , 0, 0 },
  { "rsay"     , POS_RESTING , do_race_say , 0, 0 },
  { "ruffle"   , POS_STANDING, do_action   , 0, 0 }, 

  { "say"      , POS_RESTING , do_say      , 0, 0 },
  { "'"        , POS_RESTING , do_say      , 0, 0 },
  { "save"     , POS_SLEEPING, do_save     , 0, 0 },
  { "salute"   , POS_RESTING , do_action   , 0, 0 }, 
  { "score"    , POS_DEAD    , do_score    , 0, 0 },
  { "scoff"    , POS_RESTING , do_action   , 0, 0 },
  { "scout"    , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "scream"   , POS_RESTING , do_action   , 0, 0 },
  { "scratch"  , POS_RESTING , do_action   , 0, 0 },
  { "scribe"   , POS_RESTING , skill_wrap  , 0, SCMD_NONCOMBAT },
  { "scrounge" , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT }, 
  { "sell"     , POS_STANDING, do_not_here , 0, 0 },
  { "send"     , POS_SLEEPING, do_send     , LVL_GOD, 0 },
  { "set"      , POS_DEAD    , do_set      , LVL_GOD, 0 },
  { "sethunt"  , POS_DEAD    , do_sethunt  , LVL_GRGOD, 0 }, 
  { "sedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SEDIT},
  { "shadow"   , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "shame"    , POS_RESTING , do_action   , 0, 0 },
  { "shishkabob",POS_RESTING , do_action   , 0, 0 },
  { "shout"    , POS_RESTING , do_gen_comm , 0, SCMD_SHOUT },
  { "shake"    , POS_RESTING , do_action   , 0, 0 },
  { "shiver"   , POS_RESTING , do_action   , 0, 0 },
  { "shop"     , POS_STANDING, do_not_here , 0, 0 },
  { "show"     , POS_DEAD    , do_show     , LVL_IMMORT, 0 },
  { "shrug"    , POS_RESTING , do_action   , 0, 0 },
  { "shudder"  , POS_RESTING , do_action   , 0, 0 }, 
  { "shutdow"  , POS_DEAD    , do_shutdown , LVL_IMPL-1, 0 },
  { "shutdown" , POS_DEAD    , do_shutdown , LVL_IMPL-1, SCMD_SHUTDOWN },
  { "sigh"     , POS_RESTING , do_action   , 0, 0 },
  { "sing"     , POS_RESTING , do_action   , 0, 0 },
  { "sit"      , POS_RESTING , do_sit      , 0, 0 },
  { "skillset" , POS_SLEEPING, do_skillset , LVL_GRGOD, 0 },
  { "skin"     , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "sleep"    , POS_SLEEPING, do_sleep    , 0, 0 },
  { "sleeper"  , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "slap"     , POS_RESTING , do_action   , 0, 0 },
  { "slug"     , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "smile"    , POS_RESTING , do_action   , 0, 0 },
  { "smackheads" , POS_FIGHTING, skill_wrap, 0, SCMD_COMBAT },
  { "smell"    , POS_RESTING , do_action   , 0, 0 },
  { "smirk"    , POS_RESTING , do_action   , 0, 0 },
  { "sneeze"   , POS_RESTING , do_action   , 0, 0 },
  { "sneer"    , POS_RESTING , do_action   , 0, 0 },
  { "snicker"  , POS_RESTING , do_action   , 0, 0 },
  { "snap"     , POS_RESTING , do_action   , 0, 0 },
  { "snarl"    , POS_RESTING , do_action   , 0, 0 },
  { "sneak"    , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
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
  { "spell"    , POS_DEAD    , do_scroll   , LVL_IMMORT, 0 },
  { "spike"    , POS_STANDING, do_spike    , 0, SCMD_SPIKE },
  { "spit"     , POS_RESTING , do_action   , 0, 0 },
  { "squeeze"  , POS_RESTING , do_action   , 0, 0 },
  { "squelch"  , POS_DEAD    , do_squelch  , 0, 0 }, 
  { "stand"    , POS_RESTING , do_stand    , 0, 0 },
  { "stake"    , POS_STANDING, do_spike    , 0, SCMD_STAKE },
  { "stable"   , POS_RESTING,  do_not_here , 0, 0 },
  { "stare"    , POS_RESTING , do_action   , 0, 0 },
  { "startle"  , POS_RESTING , do_action   , 0, 0 }, 
  { "stat"     , POS_DEAD    , do_stat     , LVL_IMMORT, 0 },
  { "steal"    , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "steam"    , POS_RESTING , do_action   , 0, 0 },
  { "stimpy"   , POS_RESTING , do_action   , 0, 0 },
  { "stretch"  , POS_RESTING , do_action   , 0, 0 },
  { "string"   , POS_RESTING , do_string   , LVL_IMMORT+1, 0 },
  { "stroke"   , POS_RESTING , do_action   , 0, 0 },
  { "strut"    , POS_STANDING, do_action   , 0, 0 },
  { "subdue"   , POS_STANDING, skill_wrap  , 0, SCMD_COMBAT },
  { "sulk"     , POS_RESTING , do_action   , 0, 0 },
  /* { "swear"    , POS_STANDING, do_swear    , 0, 0 }, */
  { "sweat"    , POS_RESTING , do_action   , 0, 0 }, 
  { "switch"   , POS_DEAD    , do_switch   , LVL_IMMORT+1, 0 },
  { "sysfile"  , POS_DEAD    , do_sysfile  , LVL_GOD, 0 }, 
  { "syslog"   , POS_DEAD    , do_syslog   , LVL_IMMORT, 0 },

  { "tell"     , POS_MORTALLYW,do_tell     , 0, 0 },
  { "tackle"   , POS_RESTING , do_action   , 0, 0 },
  { "tag"      , POS_RESTING , do_tag      , 0, 0 },
  { "tango"    , POS_STANDING, do_action   , 0, 0 },
  { "tap"      , POS_RESTING , do_action   , 0, 0 },
  { "taunt"    , POS_RESTING , do_action   , 0, 0 }, 
  { "tedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_TEDIT},
  { "teleport" , POS_DEAD    , do_teleport , LVL_GOD, 0 },
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
  { "tiger"    , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT },
  { "tilt"     , POS_RESTING , do_action   , 0, 0 }, 
  { "time"     , POS_RESTING , do_time     , 0, 0 },
  { "tlist"    , POS_DEAD    , do_tlist    , LVL_BUILDER, 0 },
  { "toggle"   , POS_DEAD    , do_toggle   , 0, 0 },
  { "todo"     , POS_DEAD    , do_gen_write, LVL_GOD-1, SCMD_TODO },
  { "track"    , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
  { "trip"     , POS_FIGHTING, skill_wrap  , 0, SCMD_COMBAT }, 
  { "transfer" , POS_SLEEPING, do_trans    , LVL_IMMORT+1, 0 },
  { "transform", POS_RESTING , do_transform, 1, 0 },
  { "tree"     , POS_DEAD,     do_tree      ,LVL_IMMORT, 0 },
  { "tug"      , POS_RESTING , do_action   , 0, 0 },
  { "turn"     , POS_STANDING, skill_wrap  , 0, SCMD_NONCOMBAT },
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
  int cmd, length, i;
  extern int no_specials;
  char *line;
  bool item = FALSE;

  for (i = 0; i < NUM_WEARS; i++)	/* Perm HIDE objects should keep char hidden */
    if (GET_EQ(ch, i) && (GET_OBJ_BITV(GET_EQ(ch, i)) == AFF_HIDE)) {
      item = TRUE;
      break;
    }

  if (!number(0, 3) && !item) // FIX... "fades into existence..."?
    REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

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
  else if (AFF_FLAGGED(ch, AFF_PARALYSE) && GET_LEVEL(ch) < LVL_IMMORT)
    send_to_char("You are unable to do anything, you're paralysed!\r\n", ch);
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
      do_stand(ch, "", 0, 0);
      if (no_specials || !special(ch, cmd, line)) {
        ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));
      }
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
  } else if (no_specials || !special(ch, cmd, line)) {
    ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));
  }
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
      sql_remove_alias(ch, a->alias);
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
int 
search_block(char *arg, char **list, int exact)
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
char *
delete_doubledollar(char *string)
{
  char *read, *write;

  /* If the string has no dollar signs, return immediately */
  if ((write = strchr(string, '$')) == NULL)
    return string;

  /* Start from the location of the first dollar sign */
  read = write;

  while (*read)  /* until we reach the end of the string... */
    if ((*(write++) = *(read++)) == '$') /* copy one char */
      if (*read == '$')
	read++; /* skip if we saw 2 $'s in a row */

  *write = '\0';

  return string;
}

/* Use this to control where your ansi markup language is used 
   Note that this will strip the & character from every piece of
   text you run through it, so use with caution. */

char *
delete_ansi_controls(char *string)
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

/* Use this control to remove MSP commands from a string (ie. speech) */
char *delete_msp_controls(char *string)
{
  char *read, *write;

  /* If the string has no exclamation marks, return immediately */
  if ((write = strchr(string, '!')) == NULL)
    return string;

  read = write;

  while (*read)  /* until we reach the end of the string... */
    if ((*(write++) = *(read++)) == '!') /* copy one char */
      if (*read == '!')
        read++; /* skip if we saw 2 !'s in a row */

  *write = '\0';
  return string;
}
  
char *remove_mxp(struct char_data *ch, char *string)
{
  char *read, *write, *copy, replace[15][256];
  int i = 0, j;

  if (IS_MXP(ch))
    return string;

  /* If there is no MXP content, we can return the original string */
  if (!(write = strstr(string, "<send")))
    return string;

  copy = strdup(string);
  write = strchr(string, '\033');   /* Erase everything past the escape char */
  *write = '\0';

  read = strtok(copy, ">");
  for (;;) {
    if (!(read = strtok(NULL, "<")))
      break;
    strcpy(replace[i++], read);        /* copy non-mxp string */
    if (!(read = strtok(NULL, ">")))
      break;
  }

  /* Append non-mxp string components back onto original string */
  for (j = 0; j < i; j++)
    strcat(string, replace[j]);

  FREE(copy);
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


int account_char(struct descriptor_data *d)
{
  int i = 0;

  if (get_id_by_name(GET_NAME(d->character)) != NOTHING) {
    while (d->account->chars[i])
      if (d->account->chars[i++] == GET_IDNUM(d->character))
        return (FALSE);
    return (TRUE);
  }
  return (FALSE);
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
      if (GET_MOB_SPEC(k) && (GET_MOB_SPEC(k) (ch, k, cmd, arg)))
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
    if (GET_OBJ_RNUM(i) != NOTHING)
      if (GET_OBJ_SCRIPT(i) && OBJ_SCRIPT_FLAGGED(i, OS_ONCMD)) {
        sprintf(buf, "%s%s", CMD_NAME, arg);
        if (run_script(ch, ch, i, &world[ch->in_room], buf, "oncmd", LT_OBJ))
          return 1;
    }
  }

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return (player_table + i)->id;
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

int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0, load_result;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    /* If two people log on at the same time, we may not have account details yet */
    if (!k->account)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
      if (!target) {
        target = k->original;
        mode = UNSWITCH;
      }
      if (k->character)
        k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && (GET_IDNUM(k->character) == id)) {
      if (!target && STATE(k) == CON_PLAYING) {
        SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
        target = k->character;
        mode = USURP;
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
    } else if (!strcmp(k->account->name, d->account->name)) {   /* same account, different char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      k->close_me = 1;
      if (k->character)
        sprintf(buf, "Multiple login -- %s's link marked for closing.", GET_NAME(k->character));
      else
        sprintf(buf, "Multiple login -- %s's account, nil character.", GET_ACCOUNT(k));
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
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

  for (ch = character_list; ch; ch = next_ch) {
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
    if (!target) {
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
  load_result = GET_BAD_PWS(d);
  GET_BAD_PWS(d) = 0;
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
  REMOVE_BIT(AFF_FLAGS(d->character), AFF_GROUP);
  STATE(d) = CON_PLAYING;

  switch (mode) {
    case RECON:
      SEND_TO_Q("Reconnecting.\r\n", d);
      if (sql_has_mail(GET_IDNUM(d->character)))
        send_to_char("You have mail waiting.\r\n", d->character);
      act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
      if (!ZONE_FLAGGED(world[d->character->in_room].zone, ZONE_UNLOAD))
        reload_room_descs(world[d->character->in_room].zone);
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
      if (!ZONE_FLAGGED(world[d->character->in_room].zone, ZONE_UNLOAD))
        reload_room_descs(world[d->character->in_room].zone);
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
      break;
    case UNSWITCH:
      SEND_TO_Q("Reconnecting to unswitched character.\r\n", d);
      sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
      mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
      break;
    }

  if (load_result) {
    sprintf(buf, "\r\n\r\n\007\007\007%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
      CCRED(d->character, C_SPR), load_result, (load_result > 1) ? "S" : "",
      CCNRM(d->character, C_SPR));
    SEND_TO_Q(buf, d);
  }

  return 1;
}

/* deal with newcomers and other non-playing sockets */
void
nanny(struct descriptor_data *d, char *arg)
{
  char buf[256];
  int load_result, new_char, i = 0;
  char tmp_name[MAX_INPUT_LENGTH];
  extern sh_int r_mortal_start_room[NUM_HOMETOWNS];
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;
  extern const char *hometown_menu;
  extern const char *race_menu;
  extern int max_bad_pws;
  long load_room;

  struct account_data *load_account(char *name);
  void sql_save_account(struct account_data *account);
  void sql_delete_account(char *name);
  void account_modify(struct descriptor_data *d);
  long get_ptable_by_name(char *name);
  void toggle_compression(struct descriptor_data *t);
  
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
  case CON_TEDIT:
    tedit_parse(d, arg);
    break;
  /*. End of OLC states .*/

  case CON_ACCOUNT:
    if (!*arg)
      close_socket(d);
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
          strlen(tmp_name) > MAX_NAME_LENGTH || !valid_name(tmp_name) ||
          fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
        SEND_TO_Q("Invalid account name, please try another.\r\n", d);
        SEND_TO_Q("Account name: ", d);
        return;
      }

      /* determine if the account is banned */
      if ((i = isbanned(tmp_name, FALSE))) {
        sprintf(buf2, "Sorry, your account has been banned for %d %s.\r\n", i,
          ((i > 1) ? "hours" : "hour"));
        SEND_TO_Q(buf2, d);
        sprintf(buf2, "Connection attempt denied from account '%s'", tmp_name);
        mudlog(buf2, CMP, LVL_GOD, TRUE);
        STATE(d) = CON_CLOSE;
        break;
      }

      if ((d->account = load_account(tmp_name))) {
        SEND_TO_Q("Password: ", d);
        echo_off(d);
        STATE(d) = CON_PASSWORD;
      } else {
        CREATE(d->account, struct account_data, 1);
        GET_ACCOUNT(d) = str_dup(CAP(tmp_name));
        SEND_TO_Q("Please remember to choose an account name you will remember.\r\n", d);
        sprintf(buf, "Did I get that right, \"%s\" (Y/N)? ", tmp_name);
        SEND_TO_Q(buf, d);
        STATE(d) = CON_ACCOUNT_CNFRM;
      }
    }
    break;

  case CON_ACCOUNT_CNFRM:
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host, TRUE) >= BAN_NEW) {
        sprintf(buf, "Request for new account %s denied from [%s] (siteban)",
          GET_ACCOUNT(d), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        SEND_TO_Q("Sorry, new accounts are not allowed from your site!\r\n", d);
        STATE(d) = CON_CLOSE;
        return;
      }
      if (game_restrict) {
        SEND_TO_Q("Sorry, new accounts can't be created at the moment.\r\n", d);
        sprintf(buf, "Request for new account %s denied from [%s] (wizlock)", GET_ACCOUNT(d), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        STATE(d) = CON_CLOSE;
        return;
      }
      sprintf(buf, "Give me a password for \"%s\": ", GET_ACCOUNT(d));
      SEND_TO_Q(buf, d);
      echo_off(d);
      STATE(d) = CON_NEWPASSWD;
    } else if (UPPER(*arg) == 'N') {
      SEND_TO_Q("Okay, what IS it, then? ", d);
      FREE(GET_ACCOUNT(d));
      STATE(d) = CON_ACCOUNT;
    } else
      SEND_TO_Q("Please type Yes or No: ", d);
    break;

  case CON_GET_NAME:		/* wait for input of name - this is (or should be) a new character */
    if (!*arg) {
      SEND_TO_Q("Press RETURN to continue.\r\n", d);
      STATE(d) = CON_RMOTD;
    } else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
            strlen(tmp_name) > MAX_NAME_LENGTH || !valid_name(tmp_name) ||
            fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
        SEND_TO_Q("Invalid name, please try another.\r\nName: ", d);
        return;
      }
      
      clear_desc(d);
      if ((d->character = load_char(tmp_name))) {
        if (account_char(d))
          SEND_TO_Q("This name is already in use, please try another.\r\nName: ", d);
        else {
          /* We've attempted to create a char already in the account */
          SEND_TO_Q("This character already exists, simply select it from the menu.\r\n", d);
          STATE(d) = CON_RMOTD;
          SEND_TO_Q("*** PRESS RETURN: ", d);
        }
        return;
      } else {
        /* Player unknown -- make new character */
        CREATE(d->character, struct char_data, 1);
        d->character->player.name = str_dup(CAP(tmp_name));
        SEND_TO_Q("Please remember to choose an appropriate fantasy-oriented name.\r\n", d);
        sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
        SEND_TO_Q(buf, d);
        STATE(d) = CON_NAME_CNFRM;
      }
    }
    break;

  case CON_NAME_CNFRM:		/* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host, TRUE) >= BAN_NEW) {
        sprintf(buf, "Request for new char %s denied from [%s] (siteban)", GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
        STATE(d) = CON_CLOSE;
        return;
      }
      if (game_restrict) {
        SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
        sprintf(buf, "Request for new char %s denied from [%s] (wizlock)", GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        STATE(d) = CON_CLOSE;
        return;
      }

      /* Create the character here so that variables can be set properly */
      create_entry(GET_NAME(d->character));
      init_char(d->character);
      d->character->desc = d;

      SEND_TO_Q("Do you want ANSI color (Y/N)? ", d);
      STATE(d) = CON_COLOR;
    } else if (UPPER(*arg) == 'N') {
      SEND_TO_Q("Okay, what IS it, then? ", d);
      FREE(d->character->player.name);
      STATE(d) = CON_GET_NAME;
    } else
      SEND_TO_Q("Please type Yes or No: ", d);

    break;

  case CON_PASSWORD:		/* get pwd for known account */
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
      if (strn_cmp(CRYPT(arg, GET_PASSWD(d)), GET_PASSWD(d), MAX_PWD_LENGTH)) {
        sprintf(buf, "Bad PW: %s [%s]", GET_ACCOUNT(d), d->host);
        mudlog(buf, BRF, LVL_GOD, TRUE);
        GET_BAD_PWS(d)++;
        sql_save_account(d->account);
        if (GET_BAD_PWS(d) >= max_bad_pws) {	/* 3 strikes and you're out. */
          SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
          STATE(d) = CON_CLOSE;
        } else {
         SEND_TO_Q("Wrong password.\r\nPassword: ", d);
         echo_off(d);
        }
        return;
      }

      /* password was correct */
      if (isbanned(d->host, TRUE) == BAN_SELECT) {
        SEND_TO_Q("Sorry, this account has not been cleared for login from your site!\r\n", d);
        STATE(d) = CON_CLOSE;
        sprintf(buf, "Connection attempt for '%s' denied from %s", GET_ACCOUNT(d), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        return;
      }

      /* Check for bad passwords - allows dupe check to do the same */
      if ((load_result = GET_BAD_PWS(d))) {
        GET_BAD_PWS(d) = 0;
        sql_save_account(d->account);
      }

      if (load_result) {
        sprintf(buf, "\r\n\r\n\007\007\007\x1B[0;31m"
            "%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.\x1B[0m\r\n",
            load_result, (load_result > 1) ? "S" : "");
        SEND_TO_Q(buf, d);
      }

      sprintf(buf, "Account %s [%s] has connected.", GET_ACCOUNT(d), d->host);
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
      SEND_TO_Q("\r\n", d);
      if (IS_SET(d->bits, DESC_MXP))    /* Determine if we send the MXP motd */
        SEND_TO_Q(motd_mxp, d);
      else
        SEND_TO_Q(motd, d);
      SEND_TO_Q("\r\nPress RETURN to continue.\r\n", d);
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 || !str_cmp(arg, GET_ACCOUNT(d))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    if (GET_PASSWD(d))
      FREE(GET_PASSWD(d));

    GET_PASSWD(d) = str_dup(CRYPT(arg, GET_ACCOUNT(d)));

    SEND_TO_Q("\r\nPlease retype your password: ", d);
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;

    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d)), GET_PASSWD(d), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      if (STATE(d) == CON_CNFPASSWD)
        STATE(d) = CON_NEWPASSWD;
      else
        STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);

    sql_save_account(d->account);
    if (STATE(d) == CON_CNFPASSWD) {
      sprintf(buf, "%s [%s] new account.", GET_ACCOUNT(d), d->host);
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
      SEND_TO_Q("\r\nAccount created.\r\n", d);
      if (IS_SET(d->bits, DESC_MXP)) {    /* Determine if we send the MXP motd */
        SEND_TO_Q(motd_mxp, d);
        SEND_TO_Q("\033[1z<SUPPORT prompt>", d);
      } else {
        SEND_TO_Q(motd, d);
        SEND_TO_Q("\r\nPress RETURN to continue.\r\n", d);
      }
      STATE(d) = CON_RMOTD;
    } else {
      sprintf(buf, "Account %s [%s] password modified.", GET_ACCOUNT(d), d->host);
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
      SEND_TO_Q("\r\nDone.\r\n", d);
      SEND_TO_Q(ACCOUNT_MENU, d);
      STATE(d) = CON_ACCOUNT_MENU;
    }
    break;

  case CON_COLOR:
    switch (*arg) {
      case 'y':
      case 'Y':
        SET_BIT(PRF_FLAGS(d->character), PRF_COLOR_1);
        SET_BIT(PRF_FLAGS(d->character), PRF_COLOR_2);
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
    switch (*arg) {
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
    }
    SEND_TO_Q(race_menu, d);
    SEND_TO_Q("\r\nRace: ", d);
    STATE(d) = CON_QRACE;
    break;
   
  case CON_QRACE:		/* query race of new user      */ 
    switch (*arg) {
      case '?':
        do_help(d->character, "race", 0, 0);
        SEND_TO_Q(race_menu, d);
    	SEND_TO_Q("\r\nRace: ", d);
	return;
      case 'k': case 'K': d->character->player_specials->saved.race = RACE_KIR; break;
      case 'a': case 'A': d->character->player_specials->saved.race = RACE_ANTALI; break;
      case 'x': case 'X': d->character->player_specials->saved.race = RACE_XIXIAN; break;
      default:
        SEND_TO_Q("That is not a race..\r\nWhat IS your race? ", d);
        return;
    } 
    SEND_TO_Q(hometown_menu, d);
    SEND_TO_Q("\r\nSelect: ", d);
    STATE(d) = CON_HOMETOWN;
    break;

  case CON_HOMETOWN:
    switch (*arg) {
      case 'k': case 'K': GET_HOME(d->character) = 0; break;
      case 'o': case 'O': GET_HOME(d->character) = 1; break;
      case 'm': case 'M': GET_HOME(d->character) = 2; break;
      case 'x': case 'X': GET_HOME(d->character) = 3; break;
      case 't': case 'T': GET_HOME(d->character) = 4; break;
      default:
        SEND_TO_Q("Invalid choice!\r\nSelect: ", d);
        return;
    }
    SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
    STATE(d) = CON_CREATE;
    break;

  case CON_CREATE:  /* character creation setup */
  {
    struct char_data *creator;

    SEND_TO_Q(creation, d);

    creator = read_mobile(2, VIRTUAL, NULL);      /* Required to run the script */
    char_to_room(creator, 0);
    run_script(d->character, creator, NULL, NULL, NULL, "question1", LT_MOB);
    extract_char(creator);                        /* No longer required */

    SEND_TO_Q("\r\nYour choice: ", d);
    STATE(d) = CON_CREATE_Q1;
    break;
  }

  case CON_CREATE_Q1:   /* character creation question1 - attributes*/
  {
    struct char_data *creator;

    switch(*arg) {
      case 'a': case 'A':
        d->character->real_abils.str += 1;
        d->character->real_abils.dex += 1;
        d->character->real_abils.intel -= 1;
        d->character->real_abils.wis -= 1;
        GET_MAX_HIT(d->character) += 30;
        GET_MAX_MANA(d->character) -= 30;
        break;
      case 'b': case 'B':
        d->character->real_abils.intel += 1;
        d->character->real_abils.wis += 1;
        d->character->real_abils.str -= 1;
        d->character->real_abils.dex -= 1;
        GET_MAX_HIT(d->character) -= 30;
        GET_MAX_MANA(d->character) += 30;
        break;
      case 'c': case 'C':
        d->character->real_abils.dex += 1;
        d->character->real_abils.con += 1;
        d->character->real_abils.intel -= 1;
        d->character->real_abils.str -= 1;
        GET_MAX_HIT(d->character) += 10;
        GET_MAX_MANA(d->character) -= 10;
        GET_MAX_MOVE(d->character) += 30;
        break;
      default:
        SEND_TO_Q("Invalid choice!\r\nYour choice: ", d);
        return;
    }

    creator = read_mobile(2, VIRTUAL, NULL);      /* Required to run the script */
    char_to_room(creator, 0);
    run_script(d->character, creator, NULL, NULL, NULL, "question2", LT_MOB);
    extract_char(creator);                        /* No longer required */

    SEND_TO_Q("\r\nYour choice: ", d);
    STATE(d) = CON_CREATE_Q2;
    break;
  }
    
  case CON_CREATE_Q2:   /* character creation question2 - background*/
  {
    struct char_data *creator;

    switch(*arg) {
      case 'a': case 'A':
        GET_ALIGNMENT(d->character) += 500;
        break;
      case 'b': case 'B':
        GET_ALIGNMENT(d->character) -= 500;
        break;
      case 'c': case 'C':
        GET_ALIGNMENT(d->character) += 100;
        break;
      default:
        SEND_TO_Q("Invalid choice!\r\nYour choice: ", d);
        return;
    }

    creator = read_mobile(2, VIRTUAL, NULL);      /* Required to run the script */
    char_to_room(creator, 0);
    run_script(d->character, creator, NULL, NULL, NULL, "question3", LT_MOB);
    extract_char(creator);                        /* No longer required */

    SEND_TO_Q("\r\nYour choice: ", d);
    STATE(d) = CON_CREATE_Q3;
    break;
  }

  case CON_CREATE_Q3:   /* character creation question3 - lifestyle*/
  {
    int lifestyle;

    switch(*arg) {
      case 'a': case 'A':
        lifestyle = LIFE_POOR;
        start_room = 1210;
        break;
      case 'b': case 'B':
        lifestyle = LIFE_AVERAGE;
        start_room = 1211;
        break;
      case 'c': case 'C':
        lifestyle = LIFE_RICH;
        start_room = 1212;
        break;
      default:
        SEND_TO_Q("Invalid choice!\r\nYour choice: ", d);
        return;
    }

    create_start(lifestyle, d->character);
    d->character->aff_abils = d->character->real_abils;
    d->account->chars[d->account->num_chars++] = GET_IDNUM(d->character);
    sql_save_account(d->account);
    save_char(d->character, NOWHERE);
    SET_BIT(PRF_FLAGS(d->character), PRF_NOBROAD);	/* NOBROAD set by default on start */
    sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, LVL_IMMORT, TRUE);

    sprintf(buf, "You have been given a letter of introduction. Seek out the city clerk ");
    sprintf(buf, "%sto gain some equipment.\r\n", buf);
    sql_write_notes("SYSTEM", buf, d->character);

    SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
    STATE(d) = CON_RMOTD;
    break;
  } 

  case CON_RMOTD:		/* read CR after printing motd   */
    SEND_TO_Q(MENU_TITLE, d);
    for (i = 0; i < MAX_CHARS; i++) {
      sprintf(buf, "\x1B[0;36m%d\x1B[0m) %s\r\n", i+1,
        d->account->chars[i] ? CAP(get_name_by_id(d->account->chars[i])) : "New Character");
      SEND_TO_Q(buf, d);
    }
    SEND_TO_Q(MENU_ENDING, d);
    STATE(d) = CON_MENU;
    break;

  case CON_MENU:		/* get selection from main menu  */
  {
    struct notes_data *notes = NULL, *notes_head;
    long i = 0;

    bool sql_read_notes(struct char_data *ch, struct notes_data **notes);
    void sql_remove_note(struct char_data *ch, time_t date);
    void free_notes(struct notes_data *b);
    void sql_read_plrobjs(struct char_data *ch);

    /* Since we'll need the character anyway, let's get it ready here */
    if (!d->character) {
      CREATE(d->character, struct char_data, 1);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }

    d->idle_tics = 0;
    switch (*arg) {
      case '1':   /* Either adding a new character, or selecting one to play */
      case '2':
      case '3':
        if ((i = atoi(arg) - 1))
          if (!d->account->chars[i - 1]) {
            SEND_TO_Q("You must select the next available empty slot to create a character.\r\n", d);
            SEND_TO_Q("Make your choice: ", d);
            STATE(d) = CON_MENU;
            return;
          }

        if (atoi(arg) > 3) {  /* Since *arg only checks the first digit */
          SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
          SEND_TO_Q("Your choice: ", d);
          STATE(d) = CON_MENU;
          break;
        }

        if (!d->account->chars[i]) {
          SEND_TO_Q("By what name do you wish to be known? ", d);
          STATE(d) = CON_GET_NAME;
          break;
        }

        /* Let's load the selected char and check for deletion */
        clear_desc(d);
        if (!(d->character = load_char(get_name_by_id(d->account->chars[i])))) {
          SEND_TO_Q("\r\nThere was an error loading this character.\r\nMake your choice: ",d);
          return;
        }

        d->character->desc = d;

        if (PLR_FLAGGED(d->character, PLR_DELETED)) {
          SEND_TO_Q("This character has been deleted.\r\nMake your choice: ", d);
          free_char(d->character);
          return;
        }

        if (GET_LEVEL(d->character) < game_restrict) {
          SEND_TO_Q("The game is temporarily restricted for this character.. try again later.\r\n", d);
          SEND_TO_Q("Press RETURN to continue.\r\n", d);
          sprintf(buf, "Request for login denied for %s [%s] (wizlock)", GET_NAME(d->character), d->host);
          mudlog(buf, NRM, LVL_GOD, TRUE);
          STATE(d) = CON_RMOTD;                        
          break;
        }

        /* Let's remove MXP/MSP flags if they were saved */
        REMOVE_BIT(PLR_FLAGS(d->character), PLR_MXP);
        REMOVE_BIT(PLR_FLAGS(d->character), PLR_MSP);

        /* Let's set the MXP and MSP flags if able */
        if (IS_SET(d->bits, DESC_MXP))
          SET_BIT(PLR_FLAGS(d->character), PLR_MXP);
        if (IS_SET(d->bits, DESC_MSP))
          SET_BIT(PLR_FLAGS(d->character), PLR_MSP);

        /* check and make sure no other copies of this player are logged in */
        if (perform_dupe_check(d)) {
          if (IS_SET(d->bits, DESC_MCCP) && !d->comp)   /* Need to do this for reconnecting chars */
            toggle_compression(d);
          return;
        }

        if (IS_SET(d->bits, DESC_MCCP) && !d->comp)
          toggle_compression(d);

        /* undo it just in case they are set */
        REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING |PLR_MAILING |PLR_CRYO);
        REMOVE_BIT(AFF_FLAGS(d->character), AFF_GROUP);

        if (GET_LEVEL(d->character) >= LVL_IMMORT)
          SEND_TO_Q(imotd, d);

        if (IS_SET(d->bits, DESC_MXP))
	  SEND_TO_Q(mxp, d);

        new_char = FALSE;
        reset_char(d->character);
        if ((GET_REAL_CON(d->character) - GET_VITAE(d->character)) <= 0) {    /* See if they've killed themselves! */
          SEND_TO_Q("\r\nThis character is too unhealthy to play! Press RETURN to continue.\r\n", d);
          STATE(d) = CON_RMOTD;
          break;
        }

        if (PLR_FLAGGED(d->character, PLR_INVSTART))
          GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
        sql_read_plrobjs(d->character);
        send_to_char(WELC_MESSG, d->character);
        d->character->next = character_list;
        character_list = d->character;

        if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
          load_room = real_room(load_room);

        /* If char was saved with NOWHERE, or real_room above failed... */
        if (load_room == NOWHERE) {
          if (GET_LEVEL(d->character) >= LVL_IMMORT)
            load_room = r_immort_start_room;
          else
            load_room = r_mortal_start_room[GET_HOME(d->character)];
        }

        if (PLR_FLAGGED(d->character, PLR_FROZEN))
          load_room = r_frozen_start_room;
        
        /* remove afk/inactive flags upon entering the game */
        if (PRF_FLAGGED(d->character, PRF_INACTIVE))
          REMOVE_BIT(PRF_FLAGS(d->character), PRF_INACTIVE);
        if (PRF_FLAGGED(d->character, PRF_AFK))
          REMOVE_BIT(PRF_FLAGS(d->character), PRF_AFK);

        /* Serapis 100896 Dunno if this is a real bug, but heres the fix */	
        if (load_room < 0) 
          load_room = r_mortal_start_room[0];

        sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
        if (PLR_FLAGGED(d->character, PLR_INVSTART))
          GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
        mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);

        char_to_room(d->character, load_room);
        act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

   	STATE(d) = CON_PLAYING;
        if (!GET_LEVEL(d->character)) {
          new_char = TRUE;
          do_start(d->character);
        } else {
          if (GET_INFOBAR(d->character) == INFOBAR_ON)
            InfoBarOn(d->character);
          look_at_room(d->character, 0);
        }
        if (sql_has_mail(GET_IDNUM(d->character)))
          send_to_char("You have mail waiting.\r\n", d->character);

        /* Check for any SYSTEM notes to the player */
        if (sql_read_notes(d->character, &notes)) {
          notes_head = notes;
          while (notes) {
            if (!notes->note_by) {
              sprintf(buf, "\r\n%s(Notice:)%s &W%s&n\r\n",
                      CCGRN(d->character, C_NRM), CCNRM(d->character, C_NRM), notes->note);
              send_to_char(buf, d->character);
              sql_remove_note(d->character, notes->date);
            }
            notes = notes->next;
          }
          free_notes(notes_head);
        }

        /* sanity check for con loss Serapis 970604*/
         if (GET_REAL_CON(d->character) < 1)
           GET_REAL_CON(d->character) = (d->character)->real_abils.con;
        d->has_prompt = 0;
        sql_read_aliases(d->character);
        if (GET_LEVEL(d->character)>=LEVEL_IMMORT)
          sql_read_poofs(d->character);
        if (new_char) {
          char_from_room(d->character);
          char_to_room(d->character, real_room(start_room));
          look_at_room(d->character, 0);
        }
        affect_total(d->character);
        break;

      case 'a':   /* Progress to the account menu */
      case 'A':
        SEND_TO_Q(ACCOUNT_MENU, d);
        STATE(d) = CON_ACCOUNT_MENU;
        break;

      case 'd':   /* Remove a character from the account */
      case 'D':
        if (!d->account->num_chars) {
          SEND_TO_Q("You have no characters to delete! Press RETURN to continue.\r\n", d);
          STATE(d) = CON_RMOTD;
        } else {
          SEND_TO_Q("Enter the number of the character you wish to delete: ", d);
          STATE(d) = CON_CHAR_DELCNF1;
        }
        break;

      case 'e':   /* Exit from the game */
      case 'E':
        SEND_TO_Q("Goodbye.\r\n", d);
        STATE(d) = CON_CLOSE;
        break;

      default:
        SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
        SEND_TO_Q("Your choice: ", d);
        STATE(d) = CON_MENU;
        break;
    }
    break;
  }

  case CON_ACCOUNT_MENU:
    d->idle_tics = 0;
    switch(*arg) {
      case 'c':   /* Change account password */
      case 'C':
        SEND_TO_Q("\r\nEnter your old password: ", d);
        echo_off(d);
        STATE(d) = CON_CHPWD_GETOLD;
        break;

      case 'd':
      case 'D':
        SEND_TO_Q("\r\nEnter your password for verification: ", d);
        echo_off(d);
        STATE(d) = CON_ACC_DELCNF1;
        break;

      case 'r':
      case 'R':
        SEND_TO_Q(MENU_TITLE, d);
        for (i = 0; i < MAX_CHARS; i++) {
          sprintf(buf, "%d) %s\r\n", i+1,
            d->account->chars[i] ? CAP(get_name_by_id(d->account->chars[i])) : "New Character");
          SEND_TO_Q(buf, d);
        }
        SEND_TO_Q(MENU_ENDING, d);
        STATE(d) = CON_MENU;
        break;

      default:
        SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
        SEND_TO_Q("Your choice: ", d);
        STATE(d) = CON_ACCOUNT_MENU;
        break;
    }
    break;

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d)), GET_PASSWD(d), MAX_PWD_LENGTH)) {
      echo_on(d);
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(ACCOUNT_MENU, d);
      STATE(d) = CON_ACCOUNT_MENU;
      break;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    break;

  case CON_CHAR_DELCNF1:
    i = atoi(arg) - 1;

    clear_desc(d);
    if (!*arg || !is_number(arg) || (i < 0) || (i > (MAX_CHARS - 1)) || !d->account->chars[i]) {
      SEND_TO_Q("Invalid character specified. Press RETURN to continue.\r\n", d);
      STATE(d) = CON_RMOTD;
      return;
    } else if ((d->character = load_char(get_name_by_id(d->account->chars[i])))) {
      SEND_TO_Q("\r\n", d);
      snprintf(buf, sizeof(buf), "You are about to delete %s PERMANENTLY.\r\n"
        "Are you ABSOLUTELY SURE?\r\n\r\n"
        "Please type \"yes\" to confirm: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      STATE(d) = CON_CHAR_DELCNF2;
    } else {
      SEND_TO_Q("There was an error when attempting to load this character. Press RETURN to"
        " continue.\r\n", d);
      STATE(d) = CON_RMOTD;
      return;
    }
    break;

  case CON_CHAR_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
        SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
        SEND_TO_Q("Character not deleted. Press RETURN to continue\r\n", d);
        break;
      }
      account_modify(d);
      sql_Crash_delete_file(GET_IDNUM(d->character));
      sql_delete_char(d->character);
      sprintf(buf, "Character '%s' deleted!\r\nPress RETURN to continue.\r\n", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character), GET_LEVEL(d->character));
      mudlog(buf, NRM, LVL_GOD, TRUE);

      /* Now, we effectively remove the player from the player table */
      if ((i = get_ptable_by_name(GET_NAME(d->character))) != -1)
        strcpy(player_table[i].name, "<deleted>");
    } else
      SEND_TO_Q("\r\nCharacter not deleted. Press RETURN to continue.\r\n", d);

    clear_desc(d);
    STATE(d) = CON_RMOTD;
    break;

  case CON_ACC_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, GET_PASSWD(d)), GET_PASSWD(d), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(ACCOUNT_MENU, d);
      STATE(d) = CON_ACCOUNT_MENU;
      return;
    } else {  
      SEND_TO_Q("\r\n", d);
      snprintf(buf, sizeof(buf), "You are about to delete \"%s\" PERMANENTLY, including your "
        "characters.\r\nAre you ABSOLUTELY SURE?\r\n\r\n"
        "Please type \"yes\" to confirm: ", GET_ACCOUNT(d));
      SEND_TO_Q(buf, d);
      STATE(d) = CON_ACC_DELCNF2;
    }
    break;

  case CON_ACC_DELCNF2:
  {
    struct char_data *newchar;

    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      for (i = 0; i < MAX_CHARS; i++)
        if (d->account->chars[i]) {
          sql_Crash_delete_file(d->account->chars[i]);
          if ((newchar = load_char(get_name_by_id(d->account->chars[i])))) {
            sql_delete_char(newchar);
            free_char(newchar);
          }
        }
      sql_delete_account(GET_ACCOUNT(d));
      sprintf(buf, "Account %s [%s] self-deleted.", GET_ACCOUNT(d), d->host);
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
      SEND_TO_Q("Account deleted! Goodbye.", d);
      STATE(d) = CON_CLOSE;
    } else {
      SEND_TO_Q("\r\nAccount not deleted.\r\n", d);
      SEND_TO_Q(ACCOUNT_MENU, d);
      STATE(d) = CON_ACCOUNT_MENU;
    }
    break;
  }

  case CON_CLOSE:
    close_socket(d);
    break;

  default:
    log("SYSERR: Nanny: illegal state of con'ness; closing connection");
    close_socket(d);
    break;
  }
}

/* ********************************************************************************* */
/* Skill parser - checks a lot of the basics (fighting, mounted, etc) before calling
   the actual command. */

/* cmd, function, skill, outlaw cx, needed moves */
const struct combat_skill_table combat_table[] = {
  { "ambush"    , do_ambush,      SKILL_AMBUSH,       TRUE,   5 },
  { "backstab"  , do_backstab,    SKILL_BACKSTAB,     TRUE,   5 },
  { "bash"      , do_bash,        SKILL_BASH,         TRUE,   3 },
  { "bearhug"   , do_bearhug,     SKILL_BEARHUG,      TRUE,   4 },
  { "berserk"   , do_berserk,     SKILL_BERSERK,      TRUE,   5 },
  { "charge"    , do_charge,      SKILL_CHARGE,       TRUE,   5 },
  { "circle"    , do_circle,      SKILL_CIRCLE,       TRUE,   5 },
  { "cutthroat" , do_cutthroat,   SKILL_CUTTHROAT,    TRUE,   4 },
  { "disarm"    , do_disarm,      SKILL_DISARM,       TRUE,   6 },
  { "disembowel", do_disembowel,  SKILL_DISEMBOWEL,   TRUE,   6 },
  { "groinrip"  , do_groinrip,    SKILL_GROINRIP,     TRUE,   6 },
  { "headbutt"  , do_headbutt,    SKILL_HEADBUTT,     TRUE,   3 },
  { "kick"      , do_kick,        SKILL_KICK,         TRUE,   3 },
  { "neckbreak" , do_neckbreak,   SKILL_NECKBREAK,    TRUE,   6 },
  { "parry"     , do_parry,       SKILL_PARRY,        FALSE,  4 },
  { "rescue"    , do_rescue,      SKILL_RESCUE,       FALSE,  5 },
  { "retreat"   , do_retreat,     SKILL_RETREAT,      FALSE,  0 },
  { "slug"      , do_slug,        SKILL_SLUG,         TRUE,   4 },
  { "smackheads", do_smackheads,  SKILL_SMACKHEADS,   TRUE,   5 },
  { "subdue"    , do_subdue,      SKILL_SUBDUE,       TRUE,   5 },
  { "trip"      , do_trip,        SKILL_TRIP,         TRUE,   4 },
  { "\n"        , 0, 0, 0, 0 }
};

/* cmd, function, skill, subcmd, victim cx, mounted cx */
const struct normal_skill_table noncombat_table[] = {
  { "aid"       , do_first_aid,   SKILL_FIRST_AID,   0,         TRUE,  TRUE },
  { "appraise"  , do_appraise,    SKILL_APPRAISE,    0,         FALSE, FALSE },
  { "compare"   , do_compare,     SKILL_COMPARE,     0,         FALSE, FALSE },
  { "alter"     , do_flesh_alter, SKILL_FLESH_ALTER, 0,         FALSE, FALSE },
  { "flesh"     , do_flesh_alter, SKILL_FLESH_ALTER, 0,         FALSE, FALSE },
  { "hide"      , do_hide,        SKILL_HIDE,        0,         FALSE, TRUE },
  { "mindlink"  , do_mindlink,    SKILL_MINDLINK,    0,         TRUE,  FALSE },
  { "mix"       , do_combine,     SKILL_ALCHEMY,     SCMD_MIX,  FALSE, FALSE },
  { "palm"      , do_palm,        SKILL_PALM,        0,         FALSE, TRUE },  
  { "pick"      , do_gen_door,    SKILL_PICK_LOCK,   SCMD_PICK, FALSE, TRUE },
  { "peek"      , do_peek,        SKILL_PEEK,        0,         TRUE,  FALSE },
  { "refuge"    , do_refuge,      SKILL_REFUGE,      0,         FALSE, TRUE },
  { "scout"     , do_scout,       SKILL_SCOUT,       0,         FALSE, FALSE },
  { "scribe"    , do_scribe,      SKILL_SCRIBE,      0,         FALSE, FALSE },
  { "scrounge"  , do_scrounge,    SKILL_SCROUNGE,    0,         FALSE, TRUE },
  { "search"    , do_detect,      SKILL_DETECT,      0,         FALSE, FALSE },
  { "shadow"    , do_follow,      SKILL_SHADOW,      TRUE,      TRUE,  TRUE },
  { "skin"      , do_skin,        SKILL_SKIN,        0,         FALSE, TRUE },
  { "sleeper"   , do_sleeper,     SKILL_SLEEPER,     0,         TRUE,  TRUE },
  { "sneak"     , do_sneak,       SKILL_SNEAK,       0,         FALSE, TRUE },
  { "steal"     , do_steal,       SKILL_STEAL,       0,         TRUE,  TRUE },
  { "track"     , do_track,       SKILL_TRACK,       0,         FALSE, FALSE },
  { "turn"      , do_turn,        SKILL_TURN,        0,         FALSE, FALSE },
  { "\n"        , 0, 0, 0 }
};

/* Particular combat/non-combat skill functions */
void combat_skill(struct char_data *ch, char *argument, int cmd, int subcmd, int i)
{
  struct char_data *vict;
  int is_shopkeeper(struct char_data * chChar);
  int find_moves(int skill_number);
  struct event_data *event;

  if (*combat_table[i].cmdname == '\n') {
    mudlog("SYSERR: No valid command passed to skill wrapper", BRF, LVL_IMPL, TRUE);
    return;
  }

  if (!GET_SKILL(ch, combat_table[i].skill)) {
    send_to_char("You have no idea how!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  if (combat_table[i].outlaw) {
    one_argument(argument, arg);
    if (!(vict = get_char_room_vis(ch, arg))) {
      if (FIGHTING(ch) && FIGHTING(ch)->in_room == ch->in_room)
        vict = FIGHTING(ch);
      else {
        sprintf(buf, "%s who??\r\n", combat_table[i].cmdname);
        send_to_char(CAP(buf), ch);
        return;
      }
    }

    if (vict == ch) {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }

    if (GET_LEVEL(ch) < LVL_IMMORT) {
      if (!IS_NPC(ch) && !IS_NPC(vict) && !IS_OUTLAW(ch)) {
        act("You can not attack $M because you are not an Outlaw!", TRUE, ch, 0, vict, TO_CHAR);
        return;
      }

      if (!IS_NPC(ch) && !IS_NPC(vict) && !IS_OUTLAW(vict)) {
        act("You can not attack $M because $E is not an Outlaw!", TRUE, ch, 0, vict, TO_CHAR);
        return;
      }
    }

    if (is_shopkeeper(vict) && (GET_LEVEL(ch) < LVL_IMMORT)) {
      send_to_char("Ha ha... Don't think so.\r\n", ch);
      return;
    }
  }

  if (IS_MOUNTED(ch) && strcmp(combat_table[i].cmdname, "charge")) {
    send_to_char("Dismount first!\r\n", ch);
    return;
  }

  if ((find_moves(combat_table[i].skill) > GET_MOVE(ch)) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("You don't have the energy to do that!\r\n", ch);
    return;
  }

  if (combat_table[i].skill != SKILL_RETREAT)
    if (GET_COMBAT_WAIT(ch)) {
      CREATE(event, struct event_data, 1);
      event->ch = ch;
      event->args = str_dup(argument);
      event->cmd = cmd;
      event->subcmd = subcmd;
      event->count = GET_COMBAT_WAIT(ch) + DELAY(ch);
      event->fighting = FIGHTING(ch);
      event->command = combat_table[i].command_pointer;
      DELAY(ch) += (combat_table[i].delay * PULSE_VIOLENCE) + calculate_speed(ch);
      add_event(event);
      return;
    }

  ((*combat_table[i].command_pointer) (ch, argument, cmd, subcmd));
}

void noncombat_skill(struct char_data *ch, char *argument, int cmd, int subcmd, int i)
{
  struct char_data *vict;
  int is_shopkeeper(struct char_data * chChar);

  if (*noncombat_table[i].cmdname == '\n') {
    mudlog("SYSERR: No valid command passed to skill wrapper", BRF, LVL_IMPL, TRUE);
    return;
  }

  if (!GET_SKILL(ch, noncombat_table[i].skill)) {
    send_to_char("You have no idea how!\r\n", ch);
    return;
  }

  if (noncombat_table[i].victim) {
    one_argument(argument, arg);
    if (!(vict = get_char_room_vis(ch, arg))) {
      sprintf(buf, "%s who??\r\n", noncombat_table[i].cmdname);
      send_to_char(CAP(buf), ch);
      return;
    }

    if (vict == ch) {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }

    if (is_shopkeeper(vict) && (GET_LEVEL(ch) < LVL_IMMORT)) {
      send_to_char("Ha ha... Don't think so.\r\n", ch);
      return;
    }
  }

  if (noncombat_table[i].mounted)
    if (IS_MOUNTED(ch)) {
      send_to_char("Dismount first!\r\n", ch);
      return;
    }

  if (FIGHTING(ch)) {
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
    return;
  }

  ((*noncombat_table[i].command_pointer) (ch, argument, cmd, noncombat_table[i].subcmd));
}

/* this function wraps all combat skills */
ACMD(skill_wrap)
{
  int length, i;

  switch(subcmd) {
    case SCMD_COMBAT:
      for (length = strlen(cmd_info[cmd].command), i = 0; *combat_table[i].cmdname != '\n'; i++)
        if (!strncmp(cmd_info[cmd].command, combat_table[i].cmdname, length))
          break;

      combat_skill(ch, argument, cmd, subcmd, i);
      break;
    case SCMD_NONCOMBAT:
      for (length = strlen(cmd_info[cmd].command), i = 0; *noncombat_table[i].cmdname != '\n'; i++)
        if (!strncmp(cmd_info[cmd].command, noncombat_table[i].cmdname, length))
          break;

      noncombat_skill(ch, argument, cmd, subcmd, i);
      break;
    default:
      mudlog("SYSERR: Invalid subcmd to skill_wrap.", BRF, LVL_IMPL, TRUE);
      break;
  }
}

