/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - olc.h 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

/* $Id: olc.h 1257 2005-12-08 10:53:55Z mcooper $ */

#ifndef _OLC_H
#define _OLC_H

/*. Macros, defines, structs and globals for the OLC suite .*/

/*. CONFIG DEFINES - these should be in structs.h, but here is easier.*/

#define NUM_ROOM_FLAGS 		31
#define NUM_ROOM_SECTORS	17

#define NUM_MOB_FLAGS		27
#define NUM_AFF_FLAGS		41
#define NUM_ATTACK_TYPES	15

#define NUM_ITEM_TYPES		24
#define NUM_ITEM_FLAGS		25
#define NUM_ITEM_WEARS 		19
#define NUM_APPLIES		30
#define NUM_LIQ_TYPES 		16
#define NUM_POSITIONS		15

#define NUM_GENDERS		3
#define NUM_SHOP_FLAGS 		2
#define NUM_TRADERS 		3

#define LVL_BUILDER		LVL_IMMORT
#define LVL_SET_BUILD		LVL_GOD+1

#define NUM_MSCRIPT_FLAGS	10
#define NUM_RSCRIPT_FLAGS	6
#define NUM_OSCRIPT_FLAGS	3

/*. Utils exported from olc.c .*/
void strip_string(char *);
void cleanup_olc(struct descriptor_data *d, byte cleanup_type);
void get_char_cols(struct char_data *ch);
void olc_add_to_save_list(int zone, char type);
void olc_remove_from_save_list(int zone, char type);


/*. OLC structs .*/

struct olc_data {
  int mode;
  int zone_num;
  long number;
  int value;
  struct char_data *mob;
  struct room_data *room;
  struct obj_data *obj;
  struct zone_data *zone;
  struct shop_data *shop;
  struct tlist_data *tlist;
  struct extra_descr_data *desc;
};

struct olc_save_info {
  int zone;
  char type;
  struct olc_save_info *next;
};


/*. Exported globals .*/
#ifdef _RV_OLC_
char *nrm, *grn, *cyn, *yel;
struct olc_save_info *olc_save_list = NULL;
#else
extern char *nrm, *grn, *cyn, *yel;
extern struct olc_save_info *olc_save_list;
#endif


/*. Descriptor access macros .*/
#define OLC_MODE(d) 	((d)->olc->mode) 	/*. Parse input mode	.*/
#define OLC_NUM(d) 	((d)->olc->number)	/*. Room/Obj VNUM 	.*/
#define OLC_VAL(d) 	((d)->olc->value)  	/*. Scratch variable	.*/
#define OLC_ZNUM(d) 	((d)->olc->zone_num) 	/*. Real zone number	.*/
#define OLC_ROOM(d) 	((d)->olc->room)	/*. Room structure	.*/
#define OLC_OBJ(d) 	((d)->olc->obj)	  	/*. Object structure	.*/
#define OLC_ZONE(d)     ((d)->olc->zone)	/*. Zone structure	.*/
#define OLC_MOB(d)	((d)->olc->mob)	  	/*. Mob structure	.*/
#define OLC_SHOP(d) 	((d)->olc->shop)	/*. Shop structure	.*/
#define OLC_TLIST(d)  ((d)->olc->tlist)         /*. Treasure listing    .*/
#define OLC_DESC(d) 	((d)->olc->desc)	/*. Extra description	.*/

/*. Other macros .*/

#define OLC_EXIT(d)	(OLC_ROOM(d)->dir_option[OLC_VAL(d)])
#define GET_OLC_ZONE(c)	((c)->player_specials->saved.olc_zone)

/*. Cleanup types .*/
#define CLEANUP_ALL			1	/*. Free the whole lot  .*/
#define CLEANUP_STRUCTS 		2	/*. Don't free strings  .*/

/*. Add/Remove save list types	.*/
#define OLC_SAVE_ROOM			0
#define OLC_SAVE_OBJ			1
#define OLC_SAVE_ZONE			2
#define OLC_SAVE_MOB			3
#define OLC_SAVE_SHOP			4
#define OLC_SAVE_TREASURE		5

/* Submodes of OEDIT connectedness */
#define OEDIT_MAIN_MENU              	1
#define OEDIT_EDIT_NAMELIST          	2
#define OEDIT_SHORTDESC              	3
#define OEDIT_LONGDESC               	4
#define OEDIT_ACTDESC                	5
#define OEDIT_TYPE                   	6
#define OEDIT_EXTRAS                 	7
#define OEDIT_WEAR                  	8
#define OEDIT_WEIGHT                	9
#define OEDIT_COST                  	10
#define OEDIT_PERCENTLOAD            	11
#define OEDIT_TIMER                 	12
#define OEDIT_VALUE_0			13
#define OEDIT_VALUE_1               	14
#define OEDIT_VALUE_2               	15
#define OEDIT_VALUE_3               	16
#define OEDIT_VALUE_4               	17
#define OEDIT_APPLY                 	18
#define OEDIT_APPLYMOD              	19
#define OEDIT_EXTRADESC_KEY         	20
#define OEDIT_CONFIRM_SAVEDB        	21
#define OEDIT_CONFIRM_SAVESTRING    	22
#define OEDIT_PROMPT_APPLY          	23
#define OEDIT_EXTRADESC_DESCRIPTION 	24
#define OEDIT_EXTRADESC_MENU        	25
#define OEDIT_CHAR_BITS                	26
#define OEDIT_APPLYREQS			27
#define OEDIT_PROMPT_REQS		28
#define OEDIT_REQS			29
#define OEDIT_SCRIPT_NAME		30
#define OEDIT_SCRIPT_FLAGS		31
#define OEDIT_SCRIPT_MENU		32
#define OEDIT_SPEED                     33
#define OEDIT_LEVEL                     34

/* Submodes of REDIT connectedness */
#define REDIT_MAIN_MENU 		1
#define REDIT_NAME 			2
#define REDIT_DESC 			3
#define REDIT_FLAGS 			4
#define REDIT_SECTOR 			5
#define REDIT_EXIT_MENU 		6
#define REDIT_CONFIRM_SAVEDB 		7
#define REDIT_CONFIRM_SAVESTRING 	8
#define REDIT_EXIT_NUMBER 		9
#define REDIT_EXIT_DESCRIPTION 		10
#define REDIT_EXIT_KEYWORD 		11
#define REDIT_EXIT_KEY 			12
#define REDIT_EXIT_DOORFLAGS 		13
#define REDIT_EXTRADESC_MENU 		14
#define REDIT_EXTRADESC_KEY 		15
#define REDIT_EXTRADESC_DESCRIPTION 	16
#define REDIT_COPY                      17
#define REDIT_SCRIPT_MENU		18
#define REDIT_SCRIPT_NAME		19
#define REDIT_SCRIPT_FLAGS		20
#define REDIT_LINK_DESCRIPTION          21

/*. Submodes of ZEDIT connectedness 	.*/
#define ZEDIT_MAIN_MENU              	0
#define ZEDIT_DELETE_ENTRY		1
#define ZEDIT_NEW_ENTRY			2
#define ZEDIT_CHANGE_ENTRY		3
#define ZEDIT_COMMAND_TYPE		4
#define ZEDIT_IF_FLAG			5
#define ZEDIT_ARG1			6
#define ZEDIT_ARG2			7
#define ZEDIT_ARG3			8
#define ZEDIT_ZONE_NAME			9
#define ZEDIT_ZONE_LIFE			10
#define ZEDIT_ZONE_TOP			11
#define ZEDIT_ZONE_RESET		12
#define ZEDIT_CONFIRM_SAVESTRING	13
#define ZEDIT_ZONE_AUTHOR		14
#define ZEDIT_ZONE_CLIME		15
#define ZEDIT_ZONE_FLAGS		16

/*. Submodes of MEDIT connectedness 	.*/
#define MEDIT_MAIN_MENU              	0
#define MEDIT_ALIAS			1
#define MEDIT_S_DESC			2
#define MEDIT_L_DESC			3
#define MEDIT_D_DESC			4
#define MEDIT_NPC_FLAGS			5
#define MEDIT_AFF_FLAGS			6
#define MEDIT_CONFIRM_SAVESTRING	7
#define MEDIT_NOISE			8
#define MEDIT_SCRIPT_MENU               9
/*. Numerical responses .*/
#define MEDIT_NUMERICAL_RESPONSE	10
#define MEDIT_SEX			11
#define MEDIT_NDD			14
#define MEDIT_SDD			15
#define MEDIT_NUM_HP_DICE		16
#define MEDIT_SIZE_HP_DICE		17
#define MEDIT_ADD_HP			18
#define MEDIT_AC			19
#define MEDIT_EXP			20
#define MEDIT_GOLD			21
#define MEDIT_POS			22
#define MEDIT_DEFAULT_POS		23
#define MEDIT_ATTACK			24
#define MEDIT_LEVEL			25
#define MEDIT_ALIGNMENT			26
#define MEDIT_RACE                      27
#define MEDIT_SCRIPT_NAME		28
#define MEDIT_SCRIPT_FLAGS	        29
#define MEDIT_SOUND                     30
#define MEDIT_RACE_DEFAULTS             31

/*. Submodes of SEDIT connectedness 	.*/
#define SEDIT_MAIN_MENU              	0
#define SEDIT_CONFIRM_SAVESTRING	1
#define SEDIT_NOITEM1			2
#define SEDIT_NOITEM2			3
#define SEDIT_NOCASH1			4
#define SEDIT_NOCASH2			5
#define SEDIT_NOBUY			6
#define SEDIT_BUY			7
#define SEDIT_SELL			8
#define SEDIT_PRODUCTS_MENU		11
#define SEDIT_ROOMS_MENU		12
#define SEDIT_NAMELIST_MENU		13
#define SEDIT_NAMELIST			14
/*. Numerical responses .*/
#define SEDIT_NUMERICAL_RESPONSE	20
#define SEDIT_OPEN1			21
#define SEDIT_OPEN2			22
#define SEDIT_CLOSE1			23
#define SEDIT_CLOSE2			24
#define SEDIT_KEEPER			25
#define SEDIT_BUY_PROFIT		26
#define SEDIT_SELL_PROFIT		27
#define SEDIT_ITEM_LIMIT		28
#define SEDIT_TYPE_MENU			29
#define SEDIT_DELETE_TYPE		30
#define SEDIT_DELETE_PRODUCT		31
#define SEDIT_NEW_PRODUCT		32
#define SEDIT_DELETE_ROOM		33
#define SEDIT_NEW_ROOM			34
#define SEDIT_SHOP_FLAGS		35
#define SEDIT_NOTRADE			36

/* Submodes of TEDIT connectedness */
#define TEDIT_MAIN_MENU           1
#define TEDIT_CONFIRM_SAVESTRING  2
#define TEDIT_TREASURE_TYPE       3
#define TEDIT_ADD_OBJECT          4
#define TEDIT_REMOVE_OBJECT       5
#define TEDIT_CONFIRM_DELETE_LIST 6

/*. Limit info .*/
#define MAX_ROOM_NAME	75
#define MAX_MOB_NAME	50
#define MAX_OBJ_NAME	50
#define MAX_ROOM_DESC	1024
#define MAX_EXIT_DESC	256
#define MAX_EXTRA_DESC  1024
#define MAX_MOB_DESC	1024
#define MAX_OBJ_DESC	1024

#endif /* _OLC_H */
