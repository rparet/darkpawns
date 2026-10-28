/* ************************************************************************
*   File: config.c                                      Part of CircleMUD *
*  Usage: Configuration of various aspects of CircleMUD operation         *
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

/* $Id: config.c 1211 2005-06-19 03:16:57Z mcooper $ */

#define __CONFIG_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"

#define TRUE	1
#define YES	1
#define FALSE	0
#define NO	0

/*
 * Below are several constants which you can change to alter certain aspects
 * of the way CircleMUD acts.  Since this is a .c file, all you have to do
 * to change one of the constants (assuming you keep your object files around)
 * is change the constant in this file and type 'make'.  Make will recompile
 * this file and relink; you don't have to wait for the whole thing to
 * recompile as you do if you change a header file.
 *
 * I realize that it would be slightly more efficient to have lots of
 * #defines strewn about, so that, for example, the autowiz code isn't
 * compiled at all if you don't want to use autowiz.  However, the actual
 * code for the various options is quite small, as is the computational time
 * in checking the option you've selected at run-time, so I've decided the
 * convenience of having all your options in this one file outweighs the
 * efficency of doing it the other way.
 *
 */

/****************************************************************************/
/****************************************************************************/


/* GAME PLAY OPTIONS */

/*
 * pk_allowed sets the tone of the entire game.  If pk_allowed is set to
 * NO, then players will not be allowed to kill, summon, charm, or sleep
 * other players, as well as a variety of other "asshole player" protections.
 * However, if you decide you want to have an all-out knock-down drag-out
 * PK Mud, just set pk_allowed to YES - and anything goes.
 */


/* minimum level a player must be to shout/holler/gossip/auction */
ubyte level_can_shout = 2;

/* number of movement points it costs to holler */
int holler_move_cost = 20;

/* number of movement points it costs to bury or dig for an item */
ubyte dig_bury_move_cost = 20;

/* exp change limits */
int max_exp_gain = 100000;	/* max gainable per kill */
int max_exp_loss = 500000;	/* max losable per death */

/* number of tics (usually 1 mins) before PC/NPC corpses decompose */
ubyte max_npc_corpse_time = 3;
ubyte max_pc_corpse_time = 10;

/* number of items to be left on a player corpse after death */
ubyte items_left_on_corpse = 3;

/* should items in death traps automatically be junked? */
/* ubyte dts_are_dumps = YES; -- deprecated mcooper 20040915 */

/* "okay" etc. */
char *OK = "Okay.\r\n";
char *NOPERSON = "No-one by that name here.\r\n";
char *NOEFFECT = "Nothing seems to happen.\r\n";

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.
 */
ubyte auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
ubyte autosave_time = 1;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
ubyte crash_file_timeout = 10;

/* Lifetime of normal rent files in days */
ubyte rent_file_timeout = 30;

/* Lifetime of shop items before being cleared out in days */
ubyte shop_empty_time = 30;

/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that mortals should enter at */
/* Kir Drax'in, Kir Oshi, Kir Morthis, Xixieqi, Mist Keep */
sh_int mortal_start_room[NUM_HOMETOWNS] = { 8008, 18201, 21258, 4806, 5338 };

/* virtual number of room that immorts should enter at by default */
sh_int immort_start_room = 1204;

/* virtual number of room that frozen players should enter at */
sh_int frozen_start_room = 1202;

/*
 * virtual numbers of donation rooms.  note: you must change code in
 * do_drop of act.item.c if you change the number of non-NOWHERE
 * donation rooms.
 */
sh_int donation_room_1 = 8036;
sh_int donation_room_2 = 18204;
sh_int donation_room_3 = 21264;
sh_int donation_room_4 = 5473;
sh_int donation_room_5 = 4893;
sh_int donation_room_6 = NOWHERE;

/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/*
 * This is the default port the game should run on if no port is given on
 * the command-line.  NOTE WELL: If you're using the 'autorun' script, the
 * port number there will override this setting.  Change the PORT= line in
 * instead of (or in addition to) changing this.
 */
int DFLT_PORT = 4300;

/* default directory to use as data directory */
char *DFLT_DIR = "lib";

/* maximum number of players allowed before game starts to turn people away */
ubyte MAX_PLAYERS = 75;

/* maximum size of bug, typo and idea files in bytes (to prevent bombing) */
int max_filesize = 50000;

/* maximum number of password attempts before disconnection */
ubyte max_bad_pws = 3;

/*
 * Some nameservers are very slow and cause the game to lag terribly every 
 * time someone logs in.  The lag is caused by the gethostbyaddr() function
 * which is responsible for resolving numeric IP addresses to alphabetic names.
 * Sometimes, nameservers can be so slow that the incredible lag caused by
 * gethostbyaddr() isn't worth the luxury of having names instead of numbers
 * for players' sitenames.
 *
 * If your nameserver is fast, set the variable below to NO.  If your
 * nameserver is slow, of it you would simply prefer to have numbers
 * instead of names for some other reason, set the variable to YES.
 *
 * You can experiment with the setting of nameserver_is_slow on-line using
 * the SLOWNS command from within the MUD.
 */

ubyte nameserver_is_slow = NO;


char *MENU_TITLE =
"\r\n"
"Welcome to Dark Pawns!\r\n"
"Lobby\r\n"
"-----\r\n";

/* Due to no select of ANSI yet, we must manually enter it */
char *MENU_ENDING =
"\x1B[0;36mA\x1B[0m)ccount options\r\n"
"\x1B[0;36mD\x1B[0m)elete a character\r\n"
"\x1B[0;36mE\x1B[0m)xit from Dark Pawns\r\n"
"\r\nMake your choice: ";

char *ACCOUNT_MENU =
"\r\n"
"Account\r\n"
"-------\r\n"
"\x1B[0;36mC\x1B[0m)hange account password\r\n"
"\x1B[0;36mD\x1B[0m)elete your entire account\r\n"
"\x1B[0;36mR\x1B[0m)eturn to the lobby\r\n"
"\r\nMake your choice: ";

char *SHORT_GREETINGS =

"\n\r\n\r"
"         (_____)           (_)    (_____)\n\r"
"   _     /  __ \\           | |    |  __ \\                            _\n\r"
"  ;*;   /| |  | | __ _ _ __| | __ | |__) |_ _(_      _)_ __ (___)   ;*;\n\r"
"   =    /| |  | |/ _` | '__| |/ / |  ___/ _` \\ \\ /\\ / / '_ \\/ __|    =\n\r"
" .***.  /| |__| | (_| | |  |   <  | |  | (_| |\\ V  V /| | | \\__ \\  .***.\n\r" 
" ~~~~~  /|_____/ \\__,_|_|  |_|\\_\\ |||   \\__,_| \\_/\\_/ |_| |_|___/  ~~~~~\n\r"
"        /                         |||\n\r"
"	                          |||\n\r"
"				  `.'\n\r"
"\n\r";

char *GREETINGS =
"\r\n\r\n"
"         (_____)           (_)    (_____)\r\n"
"   _     /  __ \\           | |    |  __ \\                            _\r\n"
"  ;*;   /| |  | | __ _ _ __| | __ | |__) |_ _(_      _)_ __ (___)   ;*;\r\n"
"   =    /| |  | |/ _` | '__| |/ / |  ___/ _` \\ \\ /\\ / / '_ \\/ __|    =\r\n"
" .***.  /| |__| | (_| | |  |   <  | |  | (_| |\\ V  V /| | | \\__ \\"
"  .***.\r\n"
" ~~~~~  /|_____/ \\__,_|_|  |_|\\_\\ |||   \\__,_| \\_/\\_/ |_| |_|___/  "
"~~~~~\r\n"
"                                  |||\r\n"
"                                  |||\r\n"
"                                  `.'\r\n"
"\r\n"
"             Based on CircleMUD 3.0 created by J. Elson and\r\n"
"            DikuMUD Gamma 0.0 created by K. Nyboe, T. Madsen,\r\n"
"                H. Staerfeldt, M. Seifert, and S. Hammer\r\n"
"\r\n\r\n";

char *WELC_MESSG =
"\r\n"
"Welcome to Dark Pawns! May your visit here be... Interesting."
"\r\n\r\n";

char *START_MESSG =
"Welcome to Dark Pawns, be sure and read HELP NEWBIE and HELP FAQ.\n\r"
"They will give you a good idea of how to start playing the game!\n\r";

/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/* Should the game automatically create a new wizlist/immlist every time
   someone immorts, or is promoted to a higher (or lower) god level? */
ubyte use_autowiz = YES;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead.) */
ubyte min_wizlist_lev = LVL_IMMORT+1;


/* If YES, the mud will attempt to find the remote user name for each
   player connecting to the mud.  This can also be toggled in the game
   with the "ident" command. */
ubyte ident = YES;

/* Log file names ... */
const char *LOGNAME = NULL;
/* const char *LOGNAME = "log/syslog";  -- useful for Windows users */
