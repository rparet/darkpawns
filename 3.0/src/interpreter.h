/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
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

/* $Id: interpreter.h 1330 2007-08-05 01:18:21Z jravn $ */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#define ACMD(name)  \
   void name(struct char_data *ch, char *argument, int cmd, int subcmd)

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (cmdnum >= 1 && cmdnum <= 6)

void	command_interpreter(struct char_data *ch, char *argument);
int	search_block(char *arg, char **list, int exact);
char	lower( char c );
char	*one_argument(char *argument, char *first_arg);
char	*one_word(char *argument, char *first_arg);
char	*any_one_arg(char *argument, char *first_arg);
char	*two_arguments(char *argument, char *first_arg, char *second_arg);
int	fill_word(char *argument);
void	half_chop(char *string, char *arg1, char *arg2);
void	nanny(struct descriptor_data *d, char *arg);
int	is_abbrev(char *arg1, char *arg2);
int	is_number(char *str);
int	find_command(char *command);
void	skip_spaces(char **string);
char	*delete_doubledollar(char *string);

struct command_info {
   char *command;
   byte minimum_position;
   void	(*command_pointer)
   (struct char_data *ch, char * argument, int cmd, int subcmd);
   sh_int minimum_level;
   int	subcmd;
};

/* necessary for CMD_IS macro */
#ifndef INTERPRETER_C
extern const struct command_info cmd_info[];
#endif

struct alias {
  char *alias;
  char *replacement;
  int type;
  struct alias *next;
};

struct combat_skill_table
{
  char *cmdname;
  void (*command_pointer)
  (struct char_data *ch, char * argument, int cmd, int subcmd);
  ubyte skill;
  bool outlaw;
  sh_int delay;
};

struct normal_skill_table
{
  char *cmdname;
  void (*command_pointer)
  (struct char_data *ch, char * argument, int cmd, int subcmd);
  ubyte skill;
  int subcmd;
  bool victim;
  bool mounted;
};

#define ALIAS_SIMPLE	0
#define ALIAS_COMPLEX	1

#define ALIAS_SEP_CHAR	';'
#define ALIAS_VAR_CHAR	'$'
#define ALIAS_GLOB_CHAR	'*'


/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH	1
#define SCMD_EAST	2
#define SCMD_SOUTH	3
#define SCMD_WEST	4
#define SCMD_UP		5
#define SCMD_DOWN	6

/* do_gen_ps */
#define SCMD_INFO         0
#define SCMD_HANDBOOK     1
#define SCMD_CREDITS      2
#define SCMD_NEWS         3
#define SCMD_WIZLIST      4
#define SCMD_POLICIES     5
#define SCMD_VERSION      6
#define SCMD_IMMLIST      7
#define SCMD_MOTD	  8
#define SCMD_IMOTD	  9
#define SCMD_CLEAR	  10
#define SCMD_WHOAMI	  11
#define SCMD_PLAYER_LIST  12
#define SCMD_FUTURE       13

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL	4
#define SCMD_NOAUCTION	5
#define SCMD_DEAF	6
#define SCMD_NOGOSSIP	7
#define SCMD_NOGRATZ	8
#define SCMD_NOWIZ	9
#define SCMD_QUEST	10
#define SCMD_ROOMFLAGS	11
#define SCMD_NOREPEAT	12
#define SCMD_HOLYLIGHT	13
#define SCMD_SLOWNS	14
#define SCMD_AUTOEXIT	15
#define SCMD_NOCTELL    16
#define SCMD_NOBROAD    17
#define SCMD_MXP	18
#define SCMD_NOSQL      19
#define SCMD_MSP        20
#define SCMD_MCCP       21


/* do_wizutil */
#define SCMD_REROLL	0
#define SCMD_PARDON     1
#define SCMD_NOTITLE    2
#define SCMD_SQUELCH    3
#define SCMD_FREEZE	4
#define SCMD_THAW	5
#define SCMD_UNAFFECT	6

/* do_spec_com */
#define SCMD_WHISPER	0

/* do_gen_com */
#define SCMD_HOLLER	0
#define SCMD_SHOUT	1
#define SCMD_GOSSIP	2
#define SCMD_AUCTION	3
#define SCMD_GRATZ	4

/* do_shutdown */
#define SCMD_SHUTDOW	0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUI	 0
#define SCMD_QUIT	 1

/* do_date */
#define SCMD_DATE	0
#define SCMD_UPTIME	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2
#define SCMD_TODO       3

/* do_look */
#define SCMD_LOOK	0
#define SCMD_READ	1

/* do_qcomm */
#define SCMD_QSAY	0
#define SCMD_QECHO	1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_poof */
#define SCMD_POOFIN	0
#define SCMD_POOFOUT	1

/* do_eat */
#define SCMD_EAT	0

/* do_use */
#define SCMD_USE	0
#define SCMD_RECITE	2

/* do_outlaw */
#define SCMD_OUTLAW     1

/* do_echo */
#define SCMD_ECHO	0
#define SCMD_EMOTE	1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4

/*. do_olc .*/
#define SCMD_OLC_REDIT  0
#define SCMD_OLC_OEDIT  1
#define SCMD_OLC_ZEDIT  2
#define SCMD_OLC_MEDIT  3
#define SCMD_OLC_SEDIT  4
#define SCMD_OLC_TEDIT  5
#define SCMD_OLC_SAVEINFO  6

/*. do_description .*/
#define SCMD_ROOM       0
#define SCMD_OBJ_EQ     1
#define SCMD_OBJ_INV    2
#define SCMD_OBJ_ROOM   3

/*. do_spike .*/
#define SCMD_SPIKE 0
#define SCMD_STAKE 1

/*. do_infobar .*/
#define INFOBAR_OFF     0
#define INFOBAR_ON      1 

/* do_combine */
#define SCMD_MIX	0

/* skill_wrap */
#define SCMD_COMBAT    0
#define SCMD_NONCOMBAT 1

#endif /* _INTERPRETER_H */
