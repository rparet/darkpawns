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

/* $Id: config.c 1487 2008-05-22 01:36:10Z jravn $ */

#define __CONFIG_C__

#include "config.h"
#include "sysdep.h"

#include "structs.h"

#define TRUE    1
#define YES 1
#define FALSE   0
#define NO  0

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
int level_can_shout = 2;

/* number of movement points it costs to holler */
int holler_move_cost = 20;

/* exp change limits */
int max_exp_gain = 100000;  /* max gainable per kill */
int max_exp_loss = 500000;  /* max losable per death */

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int max_npc_corpse_time = 5;
int max_pc_corpse_time = 10;

/* should items in death traps automatically be junked? */
int dts_are_dumps = YES;

/* "okay" etc. */
char *OK = "Okay.\r\n";
char *NOPERSON = "No-one by that name here.\r\n";
char *NOEFFECT = "Nothing seems to happen.\r\n";

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,
 * your objects are saved at no cost, as in Merc-type MUDs.)
 */
int free_rent = YES;

/* maximum number of items players are allowed to rent */
int max_obj_save = 30;

/* receptionist's surcharge on top of item costs */
int min_rent_cost = 100;

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.
 */
int auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
int autosave_time = 10;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int crash_file_timeout = 10;

/* Lifetime of normal rent files in days */
int rent_file_timeout = 30;


/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that mortals should enter at */
/* This is the Kir Drax'in hometown start point */
int mortal_start_room = 8004;

int kiroshi_start_room = 18201;
int alaozar_start_room = 21258;


/* virtual number of room that immorts should enter at by default */
int immort_start_room = 1204;

/* virtual number of room that frozen players should enter at */
int frozen_start_room = 1202;

/*
 * virtual numbers of donation rooms.  note: you must change code in
 * do_drop of act.item.c if you change the number of non-NOWHERE
 * donation rooms.
 */
int donation_room_1 = 8053;
int donation_room_2 = 18204;    /* unused - room for expansion */
int donation_room_3 = NOWHERE;  /* unused - room for expansion */


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
int MAX_PLAYERS = 75;

/* maximum size of bug, typo and idea files in bytes (to prevent bombing) */
int max_filesize = 50000;

/* maximum number of password attempts before disconnection */
int max_bad_pws = 3;

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

int nameserver_is_slow = YES;


char *MENU =
"\n\r"
"Welcome to Dark Pawns!\n\r"
"0) Exit from Dark Pawns.\n\r"
"1) Enter the game.\r\n"
"2) Enter description.\r\n"
"3) Read the background story.\r\n"
"4) Change password.\r\n"
"5) Delete this character.\r\n"
"\r\n"
"   Make your choice: ";

char *SHORT_GREETINGS =

"\n\r\n\r"
"         (_____)           (_)    (_____)\n\r"
"   _     /  __ \\           | |    |  __ \\                            _\n\r"
"  ;*;   /| |  | | __ _ _ __| | __ | |__) |_ _(_      _)_ __ (___)   ;*;\n\r"
"   =    /| |  | |/ _` | '__| |/ / |  ___/ _` \\ \\ /\\ / / '_ \\/ __|    =\n\r"
" .***.  /| |__| | (_| | |  |   <  | |  | (_| |\\ V  V /| | | \\__ \\  .***.\n\r"
" ~~~~~  /|_____/ \\__,_|_|  |_|\\_\\ |||   \\__,_| \\_/\\_/ |_| |_|___/  ~~~~~\n\r"
"        /                         |||\n\r"
"                             |||\n\r"
"                 `.'\n\r"
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
"\r\n"
"   As of 10-17-2008 there has been a pwipe.  Enjoy your new adventures!\r\n"
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
int use_autowiz = YES;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead.) */
int min_wizlist_lev = LVL_IMMORT+1;


/* If YES, the mud will attempt to find the remote user name for each
   player connecting to the mud.  This can also be toggled in the game
   with the "ident" command. */
int ident = NO;

/* Log file names ... */
const char *LOGNAME = NULL;
/* const char *LOGNAME = "log/syslog";  -- useful for Windows users */
