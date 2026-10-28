/*************************************************************************
*  File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and constants                *
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

/* $Id: structs.h 1329 2007-06-29 04:53:09Z rparet $ */

#include <uuid/uuid.h>
#include <zlib.h>
#ifndef _STRUCTS_H
#define _STRUCTS_H

/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database	*/
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)


/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5


/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
/* If you add new flags, make sure you update NUM_ROOM_FLAGS in olc.h */
#define ROOM_DARK		(1LL << 0)   /* Dark			*/
#define ROOM_DEATH		(1LL << 1)   /* Death trap		*/
#define ROOM_NOMOB		(1LL << 2)   /* MOBs not allowed		*/
#define ROOM_INDOORS		(1LL << 3)   /* Indoors			*/
#define ROOM_PEACEFUL		(1LL << 4)   /* Violence not allowed	*/
#define ROOM_SOUNDPROOF		(1LL << 5)   /* Shouts, gossip blocked	*/
#define ROOM_NOTRACK		(1LL << 6)   /* Track won't go through	*/
#define ROOM_NOMAGIC		(1LL << 7)   /* Magic not allowed		*/
#define ROOM_TUNNEL		(1LL << 8)   /* room for only 1 pers	*/
#define ROOM_PRIVATE		(1LL << 9)   /* Can't teleport in		*/
#define ROOM_GODROOM		(1LL << 10)  /* LVL_GOD+ only allowed	*/
#define ROOM_HOUSE		(1LL << 11)  /* (R) Room is a house	*/
#define ROOM_HOUSE_CRASH	(1LL << 12)  /* (R) House needs saving	*/
#define ROOM_ATRIUM		(1LL << 13)  /* (R) The door to a house	*/
#define ROOM_OLC		(1LL << 14)  /* (R) Modifyable/!compress	*/
#define ROOM_BFS_MARK		(1LL << 15)  /* (R) breath-first srch mrk	*/
#define ROOM_NEUTRAL            (1LL << 16)  /* can't die in here 	*/
#define ROOM_BFR                (1LL << 17)  /* bad for recall 		*/
#define ROOM_REGENROOM          (1LL << 18)  /* hightened regeneration 	*/
#define ROOM_NO_WHO_ROOM        (1LL << 19)  /* residents don't show up  */
#define ROOM_SECRET_MARK     	(1LL << 20)  /* secret door has been detected */
#define ROOM_FLOW_NORTH		(1LL << 21)  /* chars in room are pushed N */
#define ROOM_FLOW_SOUTH		(1LL << 22)  /* chars in room are pushed S */
#define ROOM_FLOW_EAST		(1LL << 23)  /* chars in room are pushed E */
#define ROOM_FLOW_WEST		(1LL << 24)  /* chars in room are pushed W */
#define ROOM_FLOW_UP		(1LL << 25)  /* chars in room are pushed U */
#define ROOM_FLOW_DOWN		(1LL << 26)  /* chars in room are pushed D */
#define ROOM_ARENA              (1LL << 27)  /* Room is part of an arena */
#define ROOM_BIND		(1LL << 28)  /* chars can bind their souls here */
#define ROOM_SHOP_CLOSED	(1LL << 29)  /* (R) Shop is currently closed */
#define ROOM_NOUNLOAD   	(1LL << 30)  /* Do not unload this room from memory */

/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR		(1 << 0)   /* Exit is a door		*/
#define EX_CLOSED		(1 << 1)   /* The door is closed	*/
#define EX_LOCKED		(1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF		(1 << 3)   /* Lock can't be picked	*/

#define ZONE_BFR		(1LL << 0)
#define ZONE_TELEPORT		(1LL << 1)
#define ZONE_KIR_DRAXIN		(1LL << 2)
#define ZONE_KIR_MORTHIS	(1LL << 3)
#define ZONE_KIR_OSHI		(1LL << 4)
#define ZONE_UNLOAD		(1LL << 5)
#define ZONE_LAW                (1LL << 6)
#define NUM_ZFLAGS		7


/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0		   /* Indoors			*/
#define SECT_CITY            1		   /* In a city			*/
#define SECT_FIELD           2		   /* In a field		*/
#define SECT_FOREST          3		   /* In a forest		*/
#define SECT_HILLS           4		   /* In the hills		*/
#define SECT_MOUNTAIN        5		   /* On a mountain		*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_UNDERWATER	     8		   /* Underwater		*/
#define SECT_FLYING	     9		   /* Wheee!			*/
#define SECT_DESERT	     10            /* Lots of Sand              */
#define SECT_FIRE	     11            /* Elemental Plane of FIRE   */
#define SECT_EARTH	     12            /* Elemental Plane of EARTH  */
#define SECT_WIND	     13            /* Elemental Plane of WIND   */
#define SECT_WATER	     14            /* Elemental Plane of WATER  */
#define SECT_SWAMP           15            /* Swamp                     */
#define SECT_ROAD	     16		   /* Road			*/

/* char and mob-related defines *****************************************/


/* PC classes */
#define CLASS_UNDEFINED	  -1
#define CLASS_MAGIC_USER  0
#define CLASS_CLERIC      1
#define CLASS_THIEF       2
#define CLASS_WARRIOR     3
#define CLASS_MAGUS       4
#define CLASS_AVATAR      5
#define CLASS_ASSASSIN    6
#define CLASS_PALADIN     7
#define CLASS_NINJA       8
#define CLASS_PSIONIC     9
#define CLASS_RANGER      10
#define CLASS_MYSTIC      11


/* Races */
#define RACE_UNDEFINED  -1
#define RACE_KIR         0
#define RACE_XIXIAN	 1
#define RACE_ANTALI	 2
#define RACE_KENDER	 3
#define RACE_RAKSHASA    4
#define RACE_SSAUR	 5
#define RACE_AMPHIBIAN	 6
#define RACE_ANNELID	 7
#define RACE_ARACHNID	 8
#define RACE_AVIAN	 9
#define RACE_BAT	 10
#define RACE_CANINE	 11
#define RACE_DEMIGOD	 12
#define RACE_DEMON	 13
#define RACE_DRAGON	 14
#define RACE_ELEMENTAL	 15
#define RACE_FAERY	 16
#define RACE_FELINE	 17
#define RACE_FISH	 18
#define RACE_GIANT	 19
#define RACE_GOBLIN	 20
#define RACE_HUMANOID	 21
#define RACE_HYBRID	 22
#define RACE_INSECT	 23
#define RACE_LYCANTHROPE 24
#define RACE_CONSTRUCT	 25
#define RACE_MAMMAL	 26
#define RACE_OGRE	 27
#define RACE_ORC	 28
#define RACE_REPTILE	 29
#define RACE_RODENT	 30
#define RACE_TROLL	 31
#define RACE_UNDEAD	 32
#define RACE_UNGULATE	 33
#define RACE_URSINE	 34
#define RACE_VAMPIRE	 35
#define RACE_VEGETABLE	 36
#define RACE_GHOST       37
     
#define NUM_INTEL_RACES  23 /* defined in constants.c */
#define NUM_RACE_KILLS	 100
#define NUM_RACES        38

/* Tattoo types */
#define TATTOO_NONE     0
#define TATTOO_TRIBAL   1
#define TATTOO_SCORPION 2
#define TATTOO_MOON     3
#define TATTOO_SUN      4
#define TATTOO_DRAGON   5
#define TATTOO_SKULL    6
#define TATTOO_ROSE     7

#define NUM_TATTOOS     8
#define NUM_DYES	8


/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2


/* Positions */
#define POS_DEAD       0	/* dead			*/
#define POS_MORTALLYW  1	/* mortally wounded	*/
#define POS_INCAP      2	/* incapacitated	*/
#define POS_STUNNED    3	/* stunned		*/
#define POS_SLEEPING   4	/* sleeping		*/
#define POS_RESTING    5	/* resting		*/
#define POS_SITTING    6	/* sitting		*/
#define POS_FIGHTING   7	/* fighting		*/
#define POS_STANDING   8	/* standing		*/


/* Player flags: used by char_data.char_specials.act */
#define PLR_OUTLAW	(1LL << 0)   /* Player is a player-killer        */
#define PLR_EXTRACT	(1LL << 1)   /* Player makred for extraction    */
#define PLR_FROZEN	(1LL << 2)   /* Player is frozen		*/
#define PLR_DONTSET     (1LL << 3)   /* Don't EVER set (ISNPC bit)	*/
#define PLR_WRITING	(1LL << 4)   /* Player writing (board/mail/olc)	*/
#define PLR_MAILING	(1LL << 5)   /* Player is writing mail		*/
#define PLR_CRASH	(1LL << 6)   /* Player needs to be crash-saved	*/
#define PLR_SITEOK	(1LL << 7)   /* Player has been site-cleared	*/
#define PLR_NOSHOUT	(1LL << 8)   /* Player not allowed to shout/goss	*/
#define PLR_NOTITLE	(1LL << 9)   /* Player not allowed to set title	*/
#define PLR_DELETED	(1LL << 10)  /* Player deleted - space reusable	*/
#define PLR_LOADROOM	(1LL << 11)  /* Player uses nonstandard loadroom	*/
#define PLR_NOWIZLIST	(1LL << 12)  /* Player shouldn't be on wizlist	*/
#define PLR_NODELETE	(1LL << 13)  /* Player shouldn't be deleted	*/
#define PLR_INVSTART	(1LL << 14)  /* Player should enter game wizinvis	*/
#define PLR_CRYO	(1LL << 15)  /* Player is cryo-saved (purge prog)	*/
/* don't confuse below with "currently in the form of" */
#define PLR_WEREWOLF    (1LL << 16)  /* Player is a werewolf              */
#define PLR_VAMPIRE     (1LL << 17)  /* Player is a vampire               */
/* don't confuse above with "currently in the form of" */     
#define PLR_IT          (1LL << 18)  /* Player is IT!                     */
#define PLR_CHOSEN	(1LL << 19)  /* Player is a chosen of the gods    */
#define PLR_REMORT      (1LL << 20)  /* new player remort flag            */
#define PLR_MXP         (1LL << 21)  /* Player has selected to use MXP    */
#define PLR_MSP         (1LL << 22)  /* Player has selected to use MSP    */
#define PLR_DEBUG       (1LL << 23)  /* DEBUG mode selected for player    */


/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         (1LL << 0)  /* Mob has a callable spec-proc	*/
#define MOB_SENTINEL     (1LL << 1)  /* Mob should not move		*/
#define MOB_SCAVENGER    (1LL << 2)  /* Mob picks up stuff on the ground	*/
#define MOB_ISNPC        (1LL << 3)  /* (R) Automatically set on all Mobs	*/
#define MOB_AWARE	 (1LL << 4)  /* Mob can't be backstabbed		*/
#define MOB_AGGRESSIVE   (1LL << 5)  /* Mob hits players in the room	*/
#define MOB_STAY_ZONE    (1LL << 6)  /* Mob shouldn't wander out of zone	*/
#define MOB_WIMPY        (1LL << 7)  /* Mob flees if severely injured	*/
#define MOB_AGGR_EVIL	 (1LL << 8)  /* auto attack evil PC's		*/
#define MOB_AGGR_GOOD	 (1LL << 9)  /* auto attack good PC's		*/
#define MOB_AGGR_NEUTRAL (1LL << 10) /* auto attack neutral PC's		*/
#define MOB_MEMORY	 (1LL << 11) /* remember attackers if attacked	*/
#define MOB_HELPER	 (1LL << 12) /* attack PCs fighting other NPCs	*/
#define MOB_NOCHARM	 (1LL << 13) /* Mob can't be charmed		*/
#define MOB_NODISPLAY    (1LL << 14) /* Mob LDESC is not displayed - completely invisible */
#define MOB_NOSLEEP	 (1LL << 15) /* Mob can't be slept		*/
#define MOB_NOBASH	 (1LL << 16) /* Mob can't be bashed (e.g. trees)	*/
#define MOB_NOBLIND	 (1LL << 17) /* Mob can't be blinded		*/
#define MOB_HUNTER       (1LL << 18) /* mob hunts you if you flee after attking */
#define MOB_AGGR75	 (1LL << 19) /* mob is aggro to players levl 75+ */
#define MOB_RANDZON      (1LL << 20) /* mob loads randomly within thier zone */
#define MOB_MOUNTABLE    (1LL << 21) /* mob is rideable                   */
#define MOB_RARE         (1LL << 22) /* mob is a (very) rare load */     
#define MOB_LOOTS        (1LL << 23) /* this mob loots, but is not aggr24 */
#define MOB_OKGIVE       (1LL << 24) /* ok to give this mobile stuff */
#define MOB_STAY_TERR    (1LL << 25) /* mob remains in current terrain type */
#define MOB_UNIQUE       (1LL << 26) /* (R) Mob is UNIQUE, load only once */
#define MOB_MOVED	 (1LL << 27) /* (R) Mob needs to be saved in db */
#define MOB_CRASH	 (1LL << 28) /* (R) Mob needs to be crash-saved */
#define MOB_UNLOAD       (1LL << 29) /* (R) Mob needs to be unloaded */
#define MOB_WARN1        (1LL << 30) /* (R) PC owned shop, warning of low cash given */
#define MOB_WARN2        (1LL << 31) /* (R) PC owned shop, warning of no cash */
#define MOB_EXTRACT      (1LL << 32) /* (R) Mob marked for extraction */

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       (1LL << 0)  /* Room descs won't normally be shown	*/
#define PRF_COMPACT     (1LL << 1)  /* No extra CRLF pair before prompts	*/
#define PRF_DEAF	(1LL << 2)  /* Can't hear shouts			*/
#define PRF_NOTELL	(1LL << 3)  /* Can't receive tells		*/
#define PRF_DISPHP	(1LL << 4)  /* Display hit points in prompt	*/
#define PRF_DISPMANA	(1LL << 5)  /* Display mana points in prompt	*/
#define PRF_DISPMOVE	(1LL << 6)  /* Display move points in prompt	*/
#define PRF_AUTOEXIT	(1LL << 7)  /* Display exits in a room		*/
#define PRF_NOHASSLE	(1LL << 8)  /* Aggr mobs won't attack		*/
#define PRF_QUEST	(1LL << 9)  /* On quest				*/
#define PRF_SUMMONABLE	(1LL << 10) /* Can be summoned			*/
#define PRF_NOREPEAT	(1LL << 11) /* No repetition of comm commands	*/
#define PRF_HOLYLIGHT	(1LL << 12) /* Can see in dark			*/
#define PRF_COLOR_1	(1LL << 13) /* Color (low bit)			*/
#define PRF_COLOR_2	(1LL << 14) /* Color (high bit)			*/
#define PRF_NOWIZ	(1LL << 15) /* Can't hear wizline			*/
#define PRF_LOG1	(1LL << 16) /* On-line System Log (low bit)	*/
#define PRF_LOG2	(1LL << 17) /* On-line System Log (high bit)	*/
#define PRF_NOAUCT	(1LL << 18) /* Can't hear auction channel		*/
#define PRF_NOGOSS	(1LL << 19) /* Can't hear gossip channel		*/
#define PRF_NOGRATZ	(1LL << 20) /* Can't hear grats channel		*/
#define PRF_ROOMFLAGS	(1LL << 21) /* Can see room flags (ROOM_x)	*/
#define PRF_AFK         (1LL << 22) /* Player is Away from Keyboard       */
#define PRF_AUTOLOOT    (1LL << 23) /* Player loots corpse upon kill      */
#define PRF_AUTOGOLD    (1LL << 24) /* Player loots gold from corpse      */
#define PRF_AUTOSPLIT   (1LL << 25) /* Player splits up gold if in group  */
#define PRF_DISPTANK    (1LL << 26) /* Display tank status in prompt      */
#define PRF_DISPTARGET  (1LL << 27) /* Display target status in prompt    */
#define PRF_INACTIVE    (1LL << 29) /* Player is in chat mode             */
#define PRF_NOCTELL     (1LL << 30) /* Player doesn't want to hear ctells */
#define PRF_NOBROAD     (1LL << 31) /* Play doesn't want to hear broadcast */

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_BLIND             (1LL << 0)	   /* (R) Char is blind		*/
#define AFF_INVISIBLE         (1LL << 1)	   /* Char is invisible		*/
#define AFF_DETECT_ALIGN      (1LL << 2)	   /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      (1LL << 3)	   /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      (1LL << 4)	   /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        (1LL << 5)	   /* Char can sense hidden life*/
#define AFF_WATERWALK	      (1LL << 6)	   /* Char can walk on water	*/
#define AFF_SANCTUARY         (1LL << 7)	   /* Char protected by sanct.	*/
#define AFF_GROUP             (1LL << 8)	   /* (R) Char is grouped	*/
#define AFF_CURSE             (1LL << 9)	   /* Char is cursed		*/
#define AFF_INFRAVISION       (1LL << 10)   /* Char can see in dark	*/
#define AFF_POISON            (1LL << 11)   /* (R) Char is poisoned	*/
#define AFF_PROTECT_EVIL      (1LL << 12)   /* Char protected from evil  */
#define AFF_PROTECT_GOOD      (1LL << 13)   /* Char protected from good  */
#define AFF_SLEEP             (1LL << 14)   /* (R) Char magically asleep	*/
#define AFF_NOTRACK	      (1LL << 15)   /* Char can't be tracked	*/
#define AFF_FLESH_ALTER	      (1LL << 16)   /* Psionic body weaponry	*/
#define AFF_DODGE	      (1LL << 17)   /* Mob dodges/PC shadows     */
#define AFF_SNEAK             (1LL << 18)   /* Char can move quietly	*/
#define AFF_HIDE              (1LL << 19)   /* Char is hidden		*/
#define AFF_BERSERK	      (1LL << 20)   /* Berserk skill 	        */
#define AFF_CHARM             (1LL << 21)   /* Char is charmed		*/
#define AFF_FOLLOW            (1LL << 22)   /* Dunno if this is used     */
#define AFF_WIMPY	      (1LL << 23)   /* Mob is Wimpy              */
#define AFF_KUJI_KIRI         (1LL << 24)   /* kuji-kiri                 */
#define AFF_CUTTHROAT	      (1LL << 25)   /* BLEEDING so much!         */
#define AFF_FLY		      (1LL << 26)   /* I'm flying!               */
#define AFF_WEREWOLF          (1LL << 27)   /* PC is in werewolf form    */
#define AFF_VAMPIRE           (1LL << 28)   /* PC is in vampire form     */
#define AFF_MOUNT             (1LL << 29)   /* mob is mounted/a mount    */
#define AFF_INVULN            (1LL << 30)   /* invulnerability spell     */
#define AFF_FLAMING           (1LL << 31)   /* Char is on fire           */
#define AFF_NOTHING           (1LL << 32)   /* Char isn't affected (kludge) */
#define AFF_HASTE             (1LL << 33)   /* Char is hasted */
#define AFF_SLOW              (1LL << 34)   /* Char is slowed */
#define AFF_DREAM	      (1LL << 35)   /* Dream travel aff */
#define AFF_WATERBREATHE      (1LL << 36)   /* underwater action */
#define AFF_METALSKIN         (1LL << 37)   /* skin is metallic */
#define AFF_PARALYSE          (1LL << 38)   /* Char is paralysed */
#define AFF_SOULTRAP          (1LL << 39)   /* Char's soul is set for trapping */
#define AFF_DETECT_RACE       (1LL << 40)   /* Char can view a mob/PCs race */

/* Modes of connectedness: used by descriptor_data.state */
/* Make sure you update connected_types[] in constants.c if */
/* you change anything here */
#define CON_PLAYING 	    0	/* Playing - Nominal state  */
#define CON_CLOSE	    1	/* Disconnecting            */
#define CON_ACCOUNT         2   /* Name of this account     */
#define CON_ACCOUNT_CNFRM   3   /* Confirm name of this account */
#define CON_PASSWORD        4	/* Password for this account    */
#define CON_NEWPASSWD       5	/* Providing a new password	*/
#define CON_CNFPASSWD       6	/* Confirming the password	*/
#define CON_RMOTD           7	/* Press RETURN and enter menu  */
#define CON_MENU            8	/* Lobby menu                   */
#define CON_ACCOUNT_MENU    9   /* Account menu                 */
#define CON_CHPWD_GETOLD    10	/* Changing passwd: get old	*/
#define CON_CHPWD_GETNEW    11	/* Changing passwd: get new	*/
#define CON_CHPWD_VRFY      12	/* Verify new password		*/
#define CON_CHAR_DELCNF1    13	/* Delete confirmation 1 for chars  */
#define CON_CHAR_DELCNF2    14	/* Delete confirmation 2 for chars  */
#define CON_ACC_DELCNF1     15  /* Delete confirmation 1 for accounts */
#define CON_ACC_DELCNF2     16  /* Delete confirmation 2 for accounts */
#define CON_GET_NAME        17	/* Name of the character 	*/
#define CON_NAME_CNFRM      18	/* Confirming the character name*/
#define CON_QSEX            19	/* Sex of the character 	*/
#define CON_QRACE           20  /* Ask user for race            */
#define CON_COLOR           21  /* Ask user if they want color  */
#define CON_HOMETOWN        22  /* Ask user what hometown 	*/
#define CON_CREATE          23	/* Character creation - setup   */
#define CON_CREATE_Q1       24  /* Char creation question 1     */
#define CON_CREATE_Q2       25	/* Char creation question 2     */
#define CON_CREATE_Q3       26	/* Char creation question 3     */
#define CON_OEDIT	    27	/*. OLC mode - object edit     .*/
#define CON_REDIT	    28	/*. OLC mode - room edit       .*/
#define CON_ZEDIT	    29	/*. OLC mode - zone info edit  .*/
#define CON_MEDIT	    30	/*. OLC mode - mobile edit     .*/
#define CON_SEDIT	    31	/*. OLC mode - shop edit       .*/
#define CON_TEDIT	    32	/*. OLC mode - treasure edit   .*/

/* Descriptor bits: used by descriptor_data.bits  */
#define DESC_MSP	(1 << 0) 	/* Desc. supports MSP */
#define DESC_MXP	(1 << 1)	/* Desc. supports MXP */
#define DESC_MCCP	(1 << 2)	/* Desc. supports MCCP */


/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WEAR_WIELD     16
#define WEAR_HOLD      17
#define WEAR_UNUSED    18
#define WEAR_ABLEGS    19
#define WEAR_FACE      20
#define WEAR_HOVER     21

#define NUM_WEARS      22	/* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1		/* Item is a light source	*/
#define ITEM_SCROLL     2		/* Item is a scroll		*/
#define ITEM_WAND       3		/* Item is a wand		*/
#define ITEM_STAFF      4		/* Item is a staff		*/
#define ITEM_WEAPON     5		/* Item is a weapon		*/
#define ITEM_SOUL_GEM   6		/* Item is a soul gem  */
#define ITEM_NOTICE     7		/* Item is a notice - no action description */
#define ITEM_TREASURE   8		/* Item is a treasure, not gold	*/
#define ITEM_ARMOR      9		/* Item is armor		*/
#define ITEM_POTION    10 		/* Item is a potion		*/
#define ITEM_WORN      11		/* Item is simply worn		*/
#define ITEM_OTHER     12		/* Misc object			*/
#define ITEM_TRASH     13		/* Trash - shopkeeps won't buy	*/
#define ITEM_BOOK      14               /* Item is a book               */
#define ITEM_CONTAINER 15		/* Item is a container		*/
#define ITEM_NOTE      16		/* Item is note 		*/
#define ITEM_DRINKCON  17		/* Item is a drink container	*/
#define ITEM_KEY       18		/* Item is a key		*/
#define ITEM_FOOD      19		/* Item is food			*/
#define ITEM_MONEY     20		/* Item is money (gold)		*/
#define ITEM_PEN       21		/* Item is a pen		*/
#define ITEM_BOAT      22		/* Item is a boat		*/
#define ITEM_FOUNTAIN  23		/* Item is a fountain		*/


/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE		(1 << 0)  /* Item can be takes		*/
#define ITEM_WEAR_FINGER	(1 << 1)  /* Can be worn on finger	*/
#define ITEM_WEAR_NECK		(1 << 2)  /* Can be worn around neck 	*/
#define ITEM_WEAR_BODY		(1 << 3)  /* Can be worn on body 	*/
#define ITEM_WEAR_HEAD		(1 << 4)  /* Can be worn on head 	*/
#define ITEM_WEAR_LEGS		(1 << 5)  /* Can be worn on legs	*/
#define ITEM_WEAR_FEET		(1 << 6)  /* Can be worn on feet	*/
#define ITEM_WEAR_HANDS		(1 << 7)  /* Can be worn on hands	*/
#define ITEM_WEAR_ARMS		(1 << 8)  /* Can be worn on arms	*/
#define ITEM_WEAR_SHIELD	(1 << 9)  /* Can be used as a shield	*/
#define ITEM_WEAR_ABOUT		(1 << 10) /* Can be worn about body 	*/
#define ITEM_WEAR_WAIST 	(1 << 11) /* Can be worn around waist 	*/
#define ITEM_WEAR_WRIST		(1 << 12) /* Can be worn on wrist 	*/
#define ITEM_WEAR_WIELD		(1 << 13) /* Can be wielded		*/
#define ITEM_WEAR_HOLD		(1 << 14) /* Can be held		*/
#define ITEM_WEAR_UNUSED        (1 << 15) /* Can be thrown           32768 */
#define ITEM_WEAR_ABLEGS        (1 << 16) /* Can be worn about legs  65536 */
#define ITEM_WEAR_FACE          (1 << 17) /* Can be worn as a mask   131072 */
#define ITEM_WEAR_HOVER	        (1 << 18) /* Hovers near head        262144 */


/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW          	(1 << 0)	/* Item is glowing		*/
#define ITEM_HUM           	(1 << 1)	/* Item is humming		*/
#define ITEM_NORENT        	(1 << 2)	/* Item cannot be rented	*/
#define ITEM_NODONATE      	(1 << 3)	/* Item cannot be donated	*/
#define ITEM_NOINVIS	   	(1 << 4)	/* Item cannot be made invis	*/
#define ITEM_INVISIBLE    	(1 << 5)	/* Item is invisible		*/
#define ITEM_MAGIC         	(1 << 6)	/* Item is magical		*/
#define ITEM_NODROP        	(1 << 7)	/* Item is cursed: can't drop	*/
#define ITEM_BLESS         	(1 << 8)	/* Item is blessed		*/
#define ITEM_ANTI_GOOD     	(1 << 9)	/* Not usable by good people	*/
#define ITEM_ANTI_EVIL     	(1 << 10)	/* Not usable by evil people	*/
#define ITEM_ANTI_NEUTRAL  	(1 << 11)	/* Not usable by neutral people	*/
#define ITEM_NOPERSIST		(1 << 12)	/* Item not saved in world	*/
#define ITEM_RANDZON            (1 << 13)       /* Item randomly loads in zone  */
#define ITEM_BURIED             (1 << 14)	/* Item has been buried		*/
#define ITEM_TMP_PERSIST        (1 << 15)       /* Item temporarily saved       */
#define ITEM_NOSELL	   	(1 << 16)	/* Shopkeepers won't touch it	*/
#define ITEM_TAKE_NAME     	(1 << 17)	/* takes on the user's name     */
#define ITEM_BROKEN	   	(1 << 24)	/* Item has been broken         */
#define ITEM_UNLOAD		(1 << 25) /* (R) Item needs to be unloaded */
 
/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0	/* No effect			*/
#define APPLY_STR               1	/* Apply to strength		*/
#define APPLY_DEX               2	/* Apply to dexterity		*/
#define APPLY_INT               3	/* Apply to intelligence	*/
#define APPLY_WIS               4	/* Apply to wisdom		*/
#define APPLY_CON               5	/* Apply to constitution	*/
#define APPLY_CHA		6	/* Apply to charisma		*/
#define APPLY_DODGEROLL         7 	/* Apply to dodgeroll		*/
#define APPLY_LEVEL             8	/* Reserved			*/
#define APPLY_AGE               9	/* Apply to age			*/
#define APPLY_CHAR_WEIGHT      10	/* Apply to weight		*/
#define APPLY_UNUSED1	       11
#define APPLY_MANA             12	/* Apply to max mana		*/
#define APPLY_HIT              13	/* Apply to max hit points	*/
#define APPLY_MOVE             14	/* Apply to max move points	*/
#define APPLY_GOLD             15	/* Reserved			*/
#define APPLY_EXP              16	/* Reserved			*/
#define APPLY_AC               17	/* Apply to Armor Class		*/
#define APPLY_HITROLL          18	/* Apply to hitroll		*/
#define APPLY_DAMROLL          19	/* Apply to damage roll		*/
#define APPLY_SAVING_PARA      20	/* Apply to save throw: paralz	*/
#define APPLY_SAVING_ROD       21	/* Apply to save throw: rods	*/
#define APPLY_SAVING_PETRI     22	/* Apply to save throw: petrif	*/
#define APPLY_SAVING_BREATH    23	/* Apply to save throw: breath	*/
#define APPLY_SAVING_SPELL     24	/* Apply to save throw: spells	*/
#define APPLY_UNUSED           25
#define APPLY_HIT_REGEN	       26       /* regen even faster while sleeping */
#define APPLY_MANA_REGEN       27       /* regen even faster while sleeping */
#define APPLY_MOVE_REGEN       28       /* regen even faster while sleeping */
#define APPLY_SPELL            29       /* affect the user with a spell */     


#define NUM_REQS 		8

/* requirements for objects, used with R fields */
#define REQ_NONE     0
#define REQ_SWORD    1
#define REQ_AXE      2
#define REQ_STAFF    3
#define REQ_SPEAR    4
#define REQ_MACE     5
#define REQ_DAGGER   6
#define REQ_WHIP     7


/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)	/* Container can be closed	*/
#define CONT_PICKPROOF      (1 << 1)	/* Container is pickproof	*/
#define CONT_CLOSED         (1 << 2)	/* Container is closed		*/
#define CONT_LOCKED         (1 << 3)	/* Container is locked		*/


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15

#define WEAPON_NONE     0
#define WEAPON_SWORD    1
#define WEAPON_AXE      2
#define WEAPON_STAFF    3
#define WEAPON_SPEAR    4
#define WEAPON_MACE     5
#define WEAPON_DAGGER   6
#define WEAPON_WHIP     7

#define MAX_WEAPON_TYPES 8   /* should be types + 1 */
#define MAX_CRAFT        9    

/* script related defines *******************************************/

/* used by script_data.lua_functions */
/* Mobile flags */
#define MS_NONE         (1 << 0)
#define MS_BRIBE	(1 << 1)
#define MS_GREET	(1 << 2)
#define MS_ONGIVE	(1 << 3)
#define MS_SOUND	(1 << 4)
#define MS_DEATH        (1 << 5)
#define MS_ONPULSE_ALL  (1 << 6)
#define MS_ONPULSE_PC	(1 << 7)
#define MS_FIGHTING	(1 << 8)
#define MS_ONCMD	(1 << 9)

/* room flags */
#define RS_NONE         (1 << 0)
#define RS_ENTER        (1 << 1)
#define RS_ONPULSE      (1 << 2)
#define RS_ONDROP       (1 << 3)
#define RS_ONGET        (1 << 4)
#define RS_ONCMD	(1 << 5)

/* Objects */
#define OS_NONE         (1 << 0)
#define OS_ONCMD        (1 << 1)
#define OS_ONPULSE      (1 << 2)


/* Lua types, used by run_script */
#define LT_MOB		"mob"
#define LT_OBJ		"obj"
#define LT_ROOM		"room"


/* other miscellaneous defines *******************************************/

#define PM_ARRAY_MAX    4
#define PR_ARRAY_MAX    4
#define AF_ARRAY_MAX    4
#define TW_ARRAY_MAX    4
#define EF_ARRAY_MAX    4
#define LF_ARRAY_MAX    4

#define MAX_SQUELCH     5   /* maximum number of people in your squelch-list */

/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2
#define TIRED	     3
#define MAX_COND     4

/* Sun state for weather_data */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_NEW	5
#define RENT_INACTIVE   6

/* Review codes */
#define SCMD_REVGOSS    0
#define SCMD_REVAUCT    1
#define SCMD_REVWIZN    2

/* other #defined constants **********************************************/
#define MOON_NEW           0
#define MOON_QUARTER_FULL  1
#define MOON_HALF_FULL     2
#define MOON_THREE_FULL    3
#define MOON_FULL          4
#define MOON_QUARTER_EMPTY 5
#define MOON_HALF_EMPTY    6
#define MOON_THREE_EMPTY   7

/* Mail associated defines */
/* minimum level a player must be to send mail	*/
#define MIN_MAIL_LEVEL 2

/* # of gold coins required to send mail	*/
#define STAMP_PRICE 25

/* Maximum size of mail in bytes (arbitrary)	*/
#define MAX_MAIL_SIZE 4096


/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 */
#define LVL_IMPL	106
#define LEVEL_IMPL      LVL_IMPL
#define LVL_GRGOD	105
#define LEVEL_GRGOD	LVL_GRGOD
#define LVL_HIGOD       104
#define LEVEL_HIGOD     LVL_HIGOD
#define LVL_LEGEND	103
#define LEVEL_LEGEND	LVL_LEGEND
#define LVL_GOD		102
#define LEVEL_GOD	LVL_GOD
#define LVL_IMMORT	101
#define LEVEL_IMMORT	LVL_IMMORT

#define LVL_FREEZE	LVL_GRGOD
#define LEVEL_FREEZE	LVL_FREEZE

#define NUM_HOMETOWNS	5	/* number of hometowns */
#define NUM_OF_DIRS	6	/* number of directions in a room (nsewud) */
#define MAGIC_NUMBER	(0x06)	/* Arbitrary number that won't be in a string */

#define OPT_USEC	100000	/* 10 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC

#define PULSE_ZONE      (10 RL_SEC)
#define PULSE_MOBILE    (4 RL_SEC)
#define PULSE_VIOLENCE  ((1 RL_SEC) / 2)
#define PULSE_AUTOSAVE	(60 RL_SEC)
#define PULSE_IDLEPWD	(15 RL_SEC)
#define PULSE_SANITY	(30 RL_SEC)
#define PULSE_USAGE	(5 * 60 RL_SEC)	 /* 5 mins */
#define PULSE_CLEANUP   (60 * 60 RL_SEC) /* 1 hour */
#define PULSE_SHOPS     (30 * 60 RL_SEC) /* 30 mins */
#define PULSE_BACKUP    (24 * 60 * 60 RL_SEC) /* 1 day */

#define MAX_SOCK_BUF	   (12 * 1024)  /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH  512 		/* Max length of prompt */
#define GARBAGE_SPACE	   32           /* Space for ** OVERFLOW** etc */
#define SMALL_BUFSIZE 	   1024         /* Static output buffer size */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE 	(MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define MAX_STRING_LENGTH	8192   /* should be 8192 */
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		70
#define MAX_NAME_LENGTH		20  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH	80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH		50  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_CHARS		3
#define IDENT_LENGTH		9
#define EXDSCR_LENGTH		240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS		200 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILL_GROUPS        13
#define NUM_SPELLS		109
#define MAX_SPELLS              130
#define MAX_OBJ_AFFECT		6 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_OBJ_REQS		6 /* used in obj_file_elem *DO*NOT*CHANGE* */

/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef char			bool;

#ifndef CIRCLE_WINDOWS
typedef char			byte;
#endif

typedef long	room_num;
typedef sh_int	obj_num;

/*
 * Bitvector type for 32 bit unsigned long bitvectors.
 * 'unsigned long long' will give you at least 64 bits if you have GCC.
 * 'unsigned long int' will give you at least 32 bits.
 *
 * Since we don't want to break the pfiles, you'll have to search throughout
 * the code for "bitvector_t" and change them yourself if you'd like this
 * extra flexibility.
 *
 * added 20010528 - mcooper
 */
typedef unsigned long long bitvector_t;


/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   long link;                     /* Description link to another room */
   struct extra_descr_data *next; /* Next in list                     */
};

/* review structure */
struct review_t {
  ubyte invis; /* invis level */
  char name[MAX_STRING_LENGTH];
  char string[MAX_STRING_LENGTH];
};

/* object-related structures ******************************************/


/* object flags; used in obj_data */
struct obj_flag_data {
   long value[5];			/* Values of the item (see list)    */
   byte type_flag;			/* Type of item			    */
   bitvector_t wear_flags;		/* Where you can wear it	    */
   bitvector_t extra_flags;		/* If it hums, glows, etc.	    */
   ush_int weight;			/* Weight                           */
   ubyte speed;             		/* Speed (in rounds) for weapons    */
   ubyte level;             		/* Item level                       */
   ush_int cost;			/* Value when sold (gp.)            */
   byte percentage_load;		/* Perc. chance item will load	    */
   sh_int timer;			/* Timer for object                 */
   bitvector_t bitvector;		/* To set chars bits                */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sh_int modifier;    /* How much it changes by              */
};

struct obj_req_type {
   byte location; 
   sbyte modifier;
};

struct tlist_data {
  char *type;                  /* Treasure list type                 */
  ubyte num_of_items;          /* Number of items in this list       */
  long *vnum;                  /* Vnum list of the items in the list */
};

/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_num item_number;		/* Where in data-base			*/
   room_num in_room;		/* In what room -1 when conta/carr	*/
   char *id;              	/* UUID id for objects              */

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */
   struct obj_req_type reqs[MAX_OBJ_REQS];  /* requirements */

   char	*name;                    /* Title of object :get etc.        */
   char	*description;		  /* When in room                     */
   char	*short_description;       /* when worn/carry/in cont.         */
   char	*action_description;      /* What to write when used          */
   char *owner;			  /* name of person who owns object   */
   char *killer;	          /* who killed this person (corpse)  */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;	  /* Worn by?			      */
   sh_int worn_on;		  /* Worn where?		      */
   sbyte  garbage;                /* how many garbage ticks           */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */
struct obj_file_elem {
   char *id;
   obj_num item_number;
   sh_int locate;    /* the wear location or vnum of the container it sits in */

   sh_int weight;
   ubyte timer;
   long cost;
   long value[5];
   bool altered;

   bitvector_t extra_flags;
   bitvector_t wear_flags;
   bitvector_t bitvector;

   struct obj_affected_type affected[MAX_OBJ_AFFECT];
   struct obj_req_type reqs[MAX_OBJ_REQS];   

   /* new data added to store do_string objects */
   char name[128];
   char shortd[128];
   char desc[256];
   struct extra_descr_data *extra_descr;
   char *action_descr;

   /* Determine whether the object is stored under a mob or house */
   struct char_data *owner;
   long house;
};


/* header block for SQL rent table */
struct rent_info {
  long time;
  ubyte rentcode;
};
/* ======================================================================= */


/* room-related structures ************************************************/


struct room_direction_data {
   char	*general_description;       /* When look DIR.			*/

   char	*keyword;		/* for open/close			*/

   sh_int exit_info;		/* Exit info				*/
   obj_num key;			/* Key's number (-1 for no key)		*/
   long to_room;		/* Where direction leads (NOWHERE)	*/
};


/* ================== Memory Structure for room ======================= */
struct room_data {
   long number;			/* Rooms number	(vnum)		      */
   sh_int zone;                 /* Room zone (for resetting)          */
   int	sector_type;            /* sector type (move/hide)            */
   char	*name;                  /* Rooms name 'You are ...'           */
   char	*description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   long room_flags;             /* DEATH,DARK ... etc                 */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);
   struct script_data *script;

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
};
/* ====================================================================== */


/* char-related structures ************************************************/

/* tattoo structure for characters */
struct tattoo_data {
  ubyte type;           /* type of tattoo */
  bitvector_t dyes;     /* dyes used */
};

/* dice structure for mobs */
struct mob_dice_struct {
  int ndd;      /* number of damage dice */
  int sdd;      /* size of damage dice   */
};

/* memory structure for characters */
struct memory_rec_struct {
   long	id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   int hours, day, month, moon;
   sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   int	played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char	*name;	       /* PC / NPC s name (kill ...  )         */
   char	*short_descr;  /* for NPC 'actions'                    */
   char	*long_descr;   /* for 'look'			       */
   char	*description;  /* Extra descriptions                   */
   char	*title;        /* PC / NPC's title                     */
   byte sex;           /* PC / NPC's sex                       */
   byte level;         /* PC / NPC's level                     */
   ubyte hometown;      /* PCs Hometown                         */
   struct time_data time;  /* PC's AGE in days                 */
   ubyte weight;       /* PC / NPC's weight                    */
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sh_int str;
   sh_int intel;
   sh_int wis;
   sh_int dex;
   sh_int con;
   sh_int cha;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   sh_int mana;
   sh_int max_mana;     /* Max move for PC/NPC			   */
   sh_int hit;
   sh_int max_hit;      /* Max hit for PC/NPC                      */
   sh_int move;
   sh_int max_move;     /* Max move for PC/NPC                     */

   sh_int armor;        /* Armor Rating */
   int	gold;           /* Money carried                           */
   int	bank_gold;	/* Gold the char has in a bank account	   */
   long	exp;            /* The experience of the player            */
   long exp_queue;      /* The unassigned exp of the player        */

   sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
   sbyte damroll;       /* Any bonus or penalty to the damage roll */
   sbyte dodgeroll;     /* Any bonus or penalty to the dodgeroll   */
};


/* 
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   sh_int alignment;		/* +/- 1000 for alignments              */
   long	idnum;			/* player's idnum; -1 for mobiles	*/
   bitvector_t act;		/* act flag for NPC's; player flag for PC's */

   bitvector_t affected_by;	/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
  struct char_data *fighting;	/* Opponent				*/
  struct char_data *hunting;	/* Mobile hunted by this char		*/
  struct char_data *guarding;   /* Mob/PC that is being guarded */

  struct obj_data *hunted_obj;  /* Objected being hunted		*/
  long hunting_id;		/* Char id of player hunted 		*/
  
  byte position;		/* Standing, fighting, sleeping, etc.	*/
  
  ush_int carry_weight;		/* Carried weight			*/
  sh_int carry_items;		/* Number of items carried		*/

  ush_int timer;                /* idle timer */
  bool parried;                 /* Am i going to be parried next round? */
  ubyte combat_state;           /* combat state bitvector */
  sh_int delay;                 /* delay before next skill usage */

  struct char_special_data_saved saved; /* constants saved in plrfile	*/
};

struct skill_data {
  ubyte group_level[MAX_SKILL_GROUPS+1];
  ubyte group_skill[MAX_SKILL_GROUPS+1];
  char known[MAX_SPELLS+1];
};

/* Timers needed only by PCs, not NPCs and stored in the pfile */
struct player_timer_data {
  sbyte conditions[4];                /* Drunk, full, thirsty, tired      */
  ubyte outlaw;                       /* When can the PC toggle outlaw?   */
  ubyte jail;                         /* When can the PC leave jail?      */
  ubyte improve[MAX_SKILL_GROUPS+1];  /* When can the PC improve a skill? */
  long mount;                         /* Time when the mount was stabled  */
};

/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   ush_int wimp_level;		/* Below this # of hit points, flee!	*/
   ubyte freeze_level;		/* Level of god who froze char, if any	*/
   ubyte invis_level;		/* level of invisibility		*/
   room_num load_room;		/* Which room to place char in		*/
   bitvector_t pref;		/* preference flags for PC's.		*/
   struct player_timer_data timer;  /* PC timers                	*/

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   ush_int pkcount;                /* Character's num of pks               */
   ush_int killcount;              /* Character's num of mob kills         */
   ush_int deathcount;             /* Character's num of deaths            */
   ubyte race;		        /* Character's race                     */
   ubyte rank;                  /* character's rank in social structure */
   long patron;                 /* who is the character's patron (by idnum) */
   sh_int race_hate[5];		/* 5 racial hatreds, based on races killed  */
   ubyte olc_zone;
   struct tattoo_data tattoo;
   bool infobar;                 /* status of infobar */
   ubyte size;                    /* screen size */
   ubyte religion;                /* Character's religion */
   int mount_vnum;		/* vnum of the mount			*/
   ubyte vitae;			/* # of vitae penalty points accrued      */
   long squelch[MAX_SQUELCH];   /* character's squelch list               */
   long lastdeath;              /* when was the last time they died?      */
   long	stable;                 /* vnum of the stable where mount is kept */
   long	bind_room;              /* room the char is bound to */
   long	vitae_exp;		/* exp required to subtract a vitae point */
   ubyte stat_points;     /* number of points accrued to purchase stat increases */
   ubyte group_points;    /* number of points accrued to purchase skill groups   */
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   char	*poofin;			/* Description on arrival of a god.     */
   char	*poofout;			/* Description upon a god's exit.       */
   struct alias *aliases;		/* Character's aliases			*/
   long last_tell;			/* idnum of last tell from		*/
   void *last_olc_targ;			/* olc control				*/
   int last_olc_mode;			/* olc control				*/
   ush_int race_kills[NUM_RACES];	/* Number of kills per race */
   time_t last_moved;			/* last time (in seconds) the character moved */
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   char *id;		    /* mobile uuid */
   int	attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   sh_int damnodice;        /* The number of damage dice's	       */
   sh_int damsizedice;      /* The size of the damage dice's           */
   int  race;		    /* NPC Race type                           */
   char *noise;             /* text "sound" the mob makes              */
   char *sound;             /* sound file the mob broadcasts           */
   room_num loaded_in;      /* the number of the room the mob loaded in (vnum) */
   sh_int hpnumdice;        /* number of HP dice */
   sh_int hpsizedice;       /* size of the HP dice */
   sh_int hpbonus;          /* mobile hp bonus */
   memory_rec *memory;	    /* List of attackers to remember	       */
   sh_int timer;            /* a timer (used for shopkeepers)          */
};

/* Serapis APPLY_SPELL Wed Jun 10 09:54:40 1998 -- Begin*/
typedef enum {
   BY_SPELL,
   BY_OBJ,
   BY_RACE
} by_types_enum;   

struct master_affected_type {
   sh_int type;			/* The type of spell that caused this      */
   sh_int duration;      	/* For how long its effects will last      */
   sh_int modifier;      	/* This is added to apropriate ability     */
   byte location;	        /* Tells which ability to change(APPLY_XXX)*/
   bitvector_t bitvector;       /* Tells which bits to set (AFF_XXX)       */
   by_types_enum by_type; 	/* What caused this affection? */
   int obj_num;           	/* If caused by an obj, which obj? */

   struct master_affected_type *next;
};
/* Serapis APPLY_SPELL Wed Jun 10 09:54:40 1998 -- End*/


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sh_int modifier;      /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   bitvector_t bitvector; /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data {
   sh_int nr;                            /* Mob's rnum			  */
   room_num in_room;                     /* Location (real room number)	  */
   room_num was_in_room;		 /* location for linkdead people  */
   int wait;			         /* wait for how many loops	  */
   int combat_wait;                      /* for round-based combat        */

   struct char_player_data player;       /* Normal data                   */
   struct char_ability_data real_abils;	 /* Abilities without modifiers   */
   struct char_ability_data aff_abils;	 /* Abils with spells/stones/etc  */
   struct skill_data skills;             /* skill/spell groups            */

   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;	/* PC/NPC specials	  */
   struct player_special_data *player_specials; /* PC specials		  */
   struct mob_special_data mob_specials;	/* NPC specials		  */

   struct master_affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   struct char_data *next_in_room;     /* For room->people - list         */
   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */

   struct char_data *guarded_by;       /* List of guardians       */
   struct char_data *next_guarding;    /* For guardian list       */

   struct follow_type *followers;        /* List of chars followers       */
   struct char_data *master;             /* Who is char following?        */

   char   *prompt;                       /* ch prompt */
};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
   /* char_player_data */
   char	name[MAX_NAME_LENGTH+1];
   char	description[EXDSCR_LENGTH];
   char	title[MAX_TITLE_LENGTH+1];
   char prompt[MAX_PROMPT_LENGTH+1];
   byte sex;
   byte level;
   ubyte hometown;
   time_t birth;   /* Time of birth of character     */
   int	played;    /* Number of secs played in total */
   ubyte weight;
   char race_kills[256];      /* Race kill string for SQL storage */

   struct skill_data skills;
   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_ability_data real_abilities;
   struct char_ability_data aff_abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];

   time_t last_logon;		/* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];	/* host of last logon */
};
/* ====================================================================== */
/* descriptor-related structures ******************************************/


struct txt_block {
   char	*text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};


struct compr {
  ubyte state; /* 0 - off. 1 - waiting for response. 2 - compress2 on */
  Bytef *buff_out;
  uInt total_out; /* size of input buffer */
  uInt size_out; /* size of data in output buffer */

  Bytef *buff_in;
  uInt total_in; /* size of input buffer */
  uInt size_in; /* size of data in input buffer */

  z_streamp stream;
};


struct account_data {
  char *name;                 /* the name of the account */
  char *pwd;                  /* the account password */
  ubyte num_chars;            /* the number of characters in this account */
  ubyte bad_pws;              /* the number of bad pw attempts this login */
  long chars[MAX_CHARS];      /* ID nums of the account characters */
};

struct descriptor_data {
   socket_t descriptor;		/* file descriptor for socket		*/
   char	host[HOST_LENGTH+1];	/* hostname				*/
   byte close_me;		/* flag: the desc. should be closed     */
   ubyte idle_tics;		/* tics idle at password prompt		*/
   ubyte connected;		/* mode of 'connectedness'		*/
   ush_int desc_num;		/* unique num assigned to desc		*/
   time_t login_time;		/* when the person connected		*/
   char *showstr_head;		/* for keeping track of an internal str	*/
   char **showstr_vector;	/* for paging through texts		*/
   ubyte showstr_count;		/* number of pages to page through	*/
   ubyte showstr_page;		/* which page are we currently showing?	*/
   char	**str;			/* for the modify-str system		*/
   size_t max_str;		/*		-			*/
   long	mail_to;		/* name for mail system			*/
   ubyte has_prompt;		/* is the user at a prompt?             */
   char	inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
   char	last_input[MAX_INPUT_LENGTH]; /* the last input			*/
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
   char *output;		/* ptr to the current output buffer	*/
   int  bufptr;			/* ptr to end of current output		*/
   int	bufspace;		/* space left in the output buffer	*/
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;		/* q of unprocessed input		*/
   struct char_data *character;	/* linked to char			*/
   struct char_data *original;	/* original char if switched		*/
   struct descriptor_data *snooping; /* Who is this char snooping	*/
   struct descriptor_data *snoop_by; /* And who is snooping this char	*/
   struct descriptor_data *next; /* link to next descriptor		*/
   struct olc_data *olc;	     	/*. OLC info - defined in olc.h   .*/
   long bits;                    	/* descriptor bits                     */
   struct account_data *account;	/* account information */
   struct compr *comp;           /* compression data */
};


/* other miscellaneous structures ***************************************/

/* skill tree pre-requisites, etc. */

struct skill_tree_data {
  char *name;
  ubyte group;
  ubyte level;
  ush_int skill_number;
};

struct npc_race_data {
  char *race;
  ush_int attack_type;
  bitvector_t aff_flags;
  bitvector_t npc_flags;
  sh_int speed_mod;   /* -5 to +5 */
};

struct msg_type {
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;	/* messages when death			*/
   struct msg_type miss_msg;	/* messages when miss			*/
   struct msg_type hit_msg;	/* messages when hit			*/
   struct msg_type god_msg;	/* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   double todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


struct weather_data {
   int	pressure;	/* How is the pressure ( Mb ) */
   int	change;	/* How fast and what way does it change. */
   int	sky;	/* How is the sky. */
   int	sunlight;	/* And how much sun. */
};


/* element in monster and object index-tables   */
struct index_data {
   int	virtual;    /* virtual number of this mob/obj           */
   SPECIAL(*func);
   struct script_data *script;
};

struct tat_data {
  char *type;
  char *effects;
};


/* for saving game time */
struct time_write {
     int year, month, day;
   };

/* for scripts */
struct script_data
{
  const char *name;
  ush_int lua_functions;
};


/* notes.c */
struct notes_data {
  char *note;
  time_t date;
  long note_by;
  struct notes_data *next;
};

struct combat_cmd {
   void (*command)
   (struct char_data *ch, char * argument, int cmd, int subcmd);
   struct char_data *ch;
   char *argument;
   int cmd;
   int subcmd;

};


#endif /* _STRUCTS_H */

