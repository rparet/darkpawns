/*************************************************************************
*   File: structs.h                                     Part of CircleMUD *
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

/* $Id: structs.h 1508 2008-05-23 05:49:33Z jravn $ */

#ifndef _STRUCTS_H
#define _STRUCTS_H

/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database */
#define NOTHING    -1    /* nil reference for objects       */
#define NOBODY     -1    /* nil reference for mobiles       */

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
#define ROOM_DARK       0   /* Dark         */
#define ROOM_DEATH      1   /* Death trap       */
#define ROOM_NOMOB      2   /* MOBs not allowed     */
#define ROOM_INDOORS        3   /* Indoors          */
#define ROOM_PEACEFUL       4   /* Violence not allowed */
#define ROOM_SOUNDPROOF     5   /* Shouts, gossip blocked   */
#define ROOM_NOTRACK        6   /* Track won't go through   */
#define ROOM_NOMAGIC        7   /* Magic not allowed        */
#define ROOM_TUNNEL     8   /* room for only 1 pers */
#define ROOM_PRIVATE        9   /* Can't teleport in        */
#define ROOM_GODROOM        10  /* LVL_GOD+ only allowed    */
#define ROOM_HOUSE      11  /* (R) Room is a house  */
#define ROOM_HOUSE_CRASH    12  /* (R) House needs saving   */
#define ROOM_ATRIUM     13  /* (R) The door to a house  */
#define ROOM_OLC        14  /* (R) Modifyable/!compress */
#define ROOM_BFS_MARK       15  /* (R) breath-first srch mrk    */
#define ROOM_NEUTRAL            16  /* can't die in here */
#define ROOM_BFR                17  /* bad for recall */
#define ROOM_REGENROOM          18  /* hightened regeneration */
#define ROOM_NO_WHO_ROOM        19  /* residents don't show up  */
#define ROOM_SECRET_MARK        20  /* secret door has been detected */
#define ROOM_FLOW_NORTH     21  /* chars in room are pushed N */
#define ROOM_FLOW_SOUTH     22  /* chars in room are pushed S */
#define ROOM_FLOW_EAST      23  /* chars in room are pushed E */
#define ROOM_FLOW_WEST      24  /* chars in room are pushed W */
#define ROOM_FLOW_UP        25  /* chars in room are pushed U */
#define ROOM_FLOW_DOWN      26  /* chars in room are pushed D */
#define ROOM_ARENA              27  /* Room is part of an arena */

/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR       (1 << 0)   /* Exit is a door        */
#define EX_CLOSED       (1 << 1)   /* The door is closed    */
#define EX_LOCKED       (1 << 2)   /* The door is locked    */
#define EX_PICKPROOF        (1 << 3)   /* Lock can't be picked  */


/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0         /* Indoors           */
#define SECT_CITY            1         /* In a city         */
#define SECT_FIELD           2         /* In a field        */
#define SECT_FOREST          3         /* In a forest       */
#define SECT_HILLS           4         /* In the hills      */
#define SECT_MOUNTAIN        5         /* On a mountain     */
#define SECT_WATER_SWIM      6         /* Swimmable water       */
#define SECT_WATER_NOSWIM    7         /* Water - need a boat   */
#define SECT_UNDERWATER      8         /* Underwater        */
#define SECT_FLYING      9         /* Wheee!            */
#define SECT_DESERT      10            /* Lots of Sand              */
#define SECT_FIRE        11            /* Elemental Plane of FIRE   */
#define SECT_EARTH       12            /* Elemental Plane of EARTH  */
#define SECT_WIND        13            /* Elemental Plane of WIND   */
#define SECT_WATER       14            /* Elemental Plane of WATER  */
#define SECT_SWAMP           15            /* Swamp                     */

/* char and mob-related defines *****************************************/


/* PC classes */
#define CLASS_UNDEFINED   -1
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

#define NUM_CLASSES   12  /* This must be the number of classes!! */

/* NPC Races */
#define RACE_NPCHUMAN    0
#define RACE_NPCELF  1
#define RACE_NPCDWARF    2
#define RACE_NPCKENDER   3
#define RACE_NPCCENTAUR  4
#define RACE_NPCRAKSHASA 5
#define RACE_TROLL       6
#define RACE_LYCANTHROPE 7
#define RACE_VAMPIRE     8
#define RACE_UNDEAD      9
#define RACE_DRAGON      10
#define RACE_DEMON       11
#define RACE_HORSE       12
#define RACE_REPTILE     13
#define RACE_ARACHNID    14
#define RACE_RODENT      15
#define RACE_OTHER       16
#define RACE_VEGGIE      17
#define RACE_GIANT       18
#define RACE_DEMIGOD     19
#define RACE_OGRE        20
#define RACE_INSECT      21
#define RACE_MAMMAL      22
#define RACE_FISH        23
#define RACE_AVIAN       24
#define RACE_MAGICAL     25
#define RACE_AMPHIBIAN   26
#define RACE_HUMANOID    27
#define RACE_FAERY       28
#define RACE_NPCSSAUR    29
#define RACE_NPCMINOTAUR 30

#define NUM_MOB_RACES    31
#define NUM_INTEL_RACES  18 /* defined in constants.c */



/* Tattoo types */
#define TATTOO_NONE     0
#define TATTOO_DRAGON   1
#define TATTOO_TRIBAL   2
#define TATTOO_SKULL    3
#define TATTOO_TIGER    4
#define TATTOO_WORM     5
#define TATTOO_EYE      6
#define TATTOO_SWORDS   7
#define TATTOO_EAGLE    8
#define TATTOO_HEART    9
#define TATTOO_STAR     10
#define TATTOO_SHIP     11
#define TATTOO_SPIDER   12
#define TATTOO_JYHAD    13
#define TATTOO_MOM      14
#define TATTOO_ANGEL    15
#define TATTOO_FOX      16
#define TATTOO_OWL      17

#define NUM_TATTOOS     18

/* PC Races */
#define RACE_UNDEFINED  -1
#define RACE_HUMAN  0
#define RACE_ELF    1
#define RACE_DWARF  2
#define RACE_KENDER 3
#define RACE_MINOTAUR   4
#define RACE_RAKSHASA   5
#define RACE_SSAUR      6

#define NUM_RACES   7

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2


/* Positions */
#define POS_DEAD       0    /* dead         */
#define POS_MORTALLYW  1    /* mortally wounded */
#define POS_INCAP      2    /* incapacitated    */
#define POS_STUNNED    3    /* stunned      */
#define POS_SLEEPING   4    /* sleeping     */
#define POS_RESTING    5    /* resting      */
#define POS_SITTING    6    /* sitting      */
#define POS_FIGHTING   7    /* fighting     */
#define POS_STANDING   8    /* standing     */


/* Player flags: used by char_data.char_specials.act */
#define PLR_OUTLAW  0   /* Player is a player-killer        */
#define PLR_OPEN    1   /* Player is a player-thief     */
#define PLR_FROZEN  2   /* Player is frozen         */
#define PLR_DONTSET     3   /* Don't EVER set (ISNPC bit)   */
#define PLR_WRITING 4   /* Player writing (board/mail/olc)  */
#define PLR_MAILING 5   /* Player is writing mail       */
#define PLR_CRASH   6   /* Player needs to be crash-saved   */
#define PLR_SITEOK  7   /* Player has been site-cleared */
#define PLR_NOSHOUT 8   /* Player not allowed to shout/goss */
#define PLR_NOTITLE 9   /* Player not allowed to set title  */
#define PLR_DELETED 10  /* Player deleted - space reusable  */
#define PLR_LOADROOM    11  /* Player uses nonstandard loadroom */
#define PLR_NOWIZLIST   12  /* Player shouldn't be on wizlist   */
#define PLR_NODELETE    13  /* Player shouldn't be deleted  */
#define PLR_INVSTART    14  /* Player should enter game wizinvis    */
#define PLR_CRYO    15  /* Player is cryo-saved (purge prog)    */
/* don't confuse below with "currently in the form of" */
#define PLR_WEREWOLF    16  /* Player is a werewolf              */
#define PLR_VAMPIRE     17  /* Player is a vampire               */
/* don't confuse above with "currently in the form of" */
#define PLR_IT          18  /* Player is IT!                     */
#define PLR_CHOSEN  19  /* Player is a chosen of the gods    */
#define PLR_REMORT      20  /* new player remort flag            */
#define PLR_EXTRACT     21  /* Player marked for extraction      */

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         0  /* Mob has a callable spec-proc */
#define MOB_SENTINEL     1  /* Mob should not move      */
#define MOB_SCAVENGER    2  /* Mob picks up stuff on the ground */
#define MOB_ISNPC        3  /* (R) Automatically set on all Mobs    */
#define MOB_AWARE    4  /* Mob can't be backstabbed     */
#define MOB_AGGRESSIVE   5  /* Mob hits players in the room */
#define MOB_STAY_ZONE    6  /* Mob shouldn't wander out of zone */
#define MOB_WIMPY        7  /* Mob flees if severely injured    */
#define MOB_AGGR_EVIL    8  /* auto attack evil PC's        */
#define MOB_AGGR_GOOD    9  /* auto attack good PC's        */
#define MOB_AGGR_NEUTRAL 10 /* auto attack neutral PC's     */
#define MOB_MEMORY   11 /* remember attackers if attacked   */
#define MOB_HELPER   12 /* attack PCs fighting other NPCs   */
#define MOB_NOCHARM  13 /* Mob can't be charmed     */
#define MOB_NOSUMMON     14 /* Mob can't be summoned        */
#define MOB_NOSLEEP  15 /* Mob can't be slept       */
#define MOB_NOBASH   16 /* Mob can't be bashed (e.g. trees) */
#define MOB_NOBLIND  17 /* Mob can't be blinded     */
#define MOB_HUNTER       18 /* mob hunts you if you flee after attking */
#define MOB_AGGR24   19 /* mob is aggro to players levl 24+ */
#define MOB_RANDZON      20 /* mob loads randomly within thier zone */
#define MOB_MOUNTABLE    21 /* mob is rideable                   */
#define MOB_RARE         22 /* mob is a (very) rare load */
#define MOB_LOOTS        23 /* this mob loots, but is not aggr24 */
#define MOB_OKGIVE       24 /* ok to give this mobile stuff */
#define MOB_EXTRACT      25 /* (R) Mob marked for extraction */

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       0  /* Room descs won't normally be shown    */
#define PRF_COMPACT     1  /* No extra CRLF pair before prompts */
#define PRF_DEAF    2  /* Can't hear shouts         */
#define PRF_NOTELL  3  /* Can't receive tells       */
#define PRF_DISPHP  4  /* Display hit points in prompt  */
#define PRF_DISPMANA    5  /* Display mana points in prompt */
#define PRF_DISPMOVE    6  /* Display move points in prompt */
#define PRF_AUTOEXIT    7  /* Display exits in a room       */
#define PRF_NOHASSLE    8  /* Aggr mobs won't attack        */
#define PRF_QUEST   9  /* On quest              */
#define PRF_SUMMONABLE  10 /* Can be summoned           */
#define PRF_NOREPEAT    11 /* No repetition of comm commands    */
#define PRF_HOLYLIGHT   12 /* Can see in dark           */
#define PRF_COLOR_1 13 /* Color (low bit)           */
#define PRF_COLOR_2 14 /* Color (high bit)          */
#define PRF_NOWIZ   15 /* Can't hear wizline            */
#define PRF_LOG1    16 /* On-line System Log (low bit)  */
#define PRF_LOG2    17 /* On-line System Log (high bit) */
#define PRF_NOAUCT  18 /* Can't hear auction channel        */
#define PRF_NOGOSS  19 /* Can't hear gossip channel     */
#define PRF_NOGRATZ 20 /* Can't hear grats channel      */
#define PRF_ROOMFLAGS   21 /* Can see room flags (ROOM_x)   */
#define PRF_AFK         22 /* Player is Away from Keyboard       */
#define PRF_AUTOLOOT    23 /* Player loots corpse upon kill      */
#define PRF_AUTOGOLD    24 /* Player loots gold from corpse      */
#define PRF_AUTOSPLIT   25 /* Player splits up gold if in group  */
#define PRF_DISPTANK    26 /* Display tank status in prompt      */
#define PRF_DISPTARGET  27 /* Display target status in prompt    */
#define PRF_NONEWBIE    28 /* Can't hear newbie channel          */
#define PRF_INACTIVE    29 /* Player is in chat mode             */
#define PRF_NOCTELL     30 /* Player doesn't want to hear ctells */
#define PRF_NOBROAD     31 /* Play doesn't want to hear broadcast */

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_BLIND             0    /* (R) Char is blind     */
#define AFF_INVISIBLE         1    /* Char is invisible     */
#define AFF_DETECT_ALIGN      2    /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      3    /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      4    /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        5    /* Char can sense hidden life*/
#define AFF_WATERWALK         6    /* Char can walk on water    */
#define AFF_SANCTUARY         7    /* Char protected by sanct.  */
#define AFF_GROUP             8    /* (R) Char is grouped   */
#define AFF_CURSE             9    /* Char is cursed        */
#define AFF_INFRAVISION       10   /* Char can see in dark  */
#define AFF_POISON            11   /* (R) Char is poisoned  */
#define AFF_PROTECT_EVIL      12   /* Char protected from evil  */
#define AFF_PROTECT_GOOD      13   /* Char protected from good  */
#define AFF_SLEEP             14   /* (R) Char magically asleep */
#define AFF_NOTRACK       15   /* Char can't be tracked */
#define AFF_FLESH_ALTER       16   /* Psionic body weaponry */
#define AFF_DODGE         17   /* Mob dodges/PC shadows     */
#define AFF_SNEAK             18   /* Char can move quietly */
#define AFF_HIDE              19   /* Char is hidden        */
#define AFF_BERSERK       20   /* Berserk skill             */
#define AFF_CHARM             21   /* Char is charmed       */
#define AFF_FOLLOW            22   /* Dunno if this is used     */
#define AFF_WIMPY         23   /* Mob is Wimpy              */
#define AFF_KUJI_KIRI         24   /* kuji-kiri                 */
#define AFF_CUTTHROAT         25   /* BLEEDING so much!         */
#define AFF_FLY           26   /* I'm flying!               */
#define AFF_WEREWOLF          27   /* PC is in werewolf form    */
#define AFF_VAMPIRE           28   /* PC is in vampire form     */
#define AFF_MOUNT             29   /* mob is mounted/a mount    */
#define AFF_INVULN            30   /* invulnerability spell     */
#define AFF_FLAMING           31   /* Char is on fire           */
#define AFF_NOTHING           32   /* Char isn't affected (kludge) */
#define AFF_HASTE             33   /* Char is hasted */
#define AFF_SLOW              34   /* Char is slowed */
#define AFF_DREAM         35   /* Dream travel aff */
#define AFF_WATERBREATHE      36   /* underwater action */
#define AFF_METALSKIN         37   /* skin is metallic */
#define AFF_ROBBED        38   /* player has been robbed recently */

/* Modes of connectedness: used by descriptor_data.state */
/* Make sure you update connected_types[] in constants.c if */
/* you change anything here */
#define CON_PLAYING  0      /* Playing - Nominal state  */
#define CON_CLOSE    1      /* Disconnecting        */
#define CON_GET_NAME     2      /* By what name ..?     */
#define CON_NAME_CNFRM   3      /* Did I get that right, x? */
#define CON_PASSWORD     4      /* Password:            */
#define CON_NEWPASSWD    5      /* Give me a password for x */
#define CON_CNFPASSWD    6      /* Please retype password:  */
#define CON_QSEX     7      /* Sex?             */
#define CON_QCLASS   8      /* Class?           */
#define CON_RMOTD    9      /* PRESS RETURN after MOTD  */
#define CON_MENU     10     /* Your choice: (main menu) */
#define CON_EXDESC   11     /* Enter a new description: */
#define CON_CHPWD_GETOLD 12     /* Changing passwd: get old */
#define CON_CHPWD_GETNEW 13     /* Changing passwd: get new */
#define CON_CHPWD_VRFY   14     /* Verify new password      */
#define CON_DELCNF1  15     /* Delete confirmation 1    */
#define CON_DELCNF2  16     /* Delete confirmation 2    */
#define CON_OEDIT    17     /*. OLC mode - object edit     .*/
#define CON_REDIT    18     /*. OLC mode - room edit       .*/
#define CON_ZEDIT    19     /*. OLC mode - zone info edit  .*/
#define CON_MEDIT    20     /*. OLC mode - mobile edit     .*/
#define CON_SEDIT    21     /*. OLC mode - shop edit       .*/
#define CON_IDCONING     22     /* waiting for ident connection */
#define CON_IDCONED  23     /* ident connection complete    */
#define CON_IDREADING    24     /* waiting to read ident sock   */
#define CON_IDREAD   25     /* ident results read           */
#define CON_ASKNAME  26     /* Ask user for name            */
#define CON_QRACE        27             /* Ask user for race            */
#define CON_COLOR        28             /* Ask user if they want color  */
#define CON_HOMETOWN     29             /* Ask user what hometown */
#define CON_ROLLABL1     30
#define CON_ROLLABL2     31
#define CON_TEDIT    32             /* OLC mode - text editor   */

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
#define WEAR_THROW     18
#define WEAR_ABLEGS    19
#define WEAR_FACE      20
#define WEAR_HOVER     21

#define NUM_WEARS      22   /* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1       /* Item is a light source   */
#define ITEM_SCROLL     2       /* Item is a scroll     */
#define ITEM_WAND       3       /* Item is a wand       */
#define ITEM_STAFF      4       /* Item is a staff      */
#define ITEM_WEAPON     5       /* Item is a weapon     */
#define ITEM_FIREWEAPON 6       /* Unimplemented        */
#define ITEM_MISSILE    7       /* Unimplemented        */
#define ITEM_TREASURE   8       /* Item is a treasure, not gold */
#define ITEM_ARMOR      9       /* Item is armor        */
#define ITEM_POTION    10       /* Item is a potion     */
#define ITEM_WORN      11       /* Unimplemented        */
#define ITEM_OTHER     12       /* Misc object          */
#define ITEM_TRASH     13       /* Trash - shopkeeps won't buy  */
#define ITEM_TRAP      14       /* Unimplemented        */
#define ITEM_CONTAINER 15       /* Item is a container      */
#define ITEM_NOTE      16       /* Item is note         */
#define ITEM_DRINKCON  17       /* Item is a drink container    */
#define ITEM_KEY       18       /* Item is a key        */
#define ITEM_FOOD      19       /* Item is food         */
#define ITEM_MONEY     20       /* Item is money (gold)     */
#define ITEM_PEN       21       /* Item is a pen        */
#define ITEM_BOAT      22       /* Item is a boat       */
#define ITEM_FOUNTAIN  23       /* Item is a fountain       */


/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE      0  /* Item can be takes     */
#define ITEM_WEAR_FINGER    1  /* Can be worn on finger */
#define ITEM_WEAR_NECK      2  /* Can be worn around neck   */
#define ITEM_WEAR_BODY      3  /* Can be worn on body   */
#define ITEM_WEAR_HEAD      4  /* Can be worn on head   */
#define ITEM_WEAR_LEGS      5  /* Can be worn on legs   */
#define ITEM_WEAR_FEET      6  /* Can be worn on feet   */
#define ITEM_WEAR_HANDS     7  /* Can be worn on hands  */
#define ITEM_WEAR_ARMS      8  /* Can be worn on arms   */
#define ITEM_WEAR_SHIELD    9  /* Can be used as a shield   */
#define ITEM_WEAR_ABOUT     10 /* Can be worn about body    */
#define ITEM_WEAR_WAIST     11 /* Can be worn around waist  */
#define ITEM_WEAR_WRIST     12 /* Can be worn on wrist  */
#define ITEM_WEAR_WIELD     13 /* Can be wielded        */
#define ITEM_WEAR_HOLD      14 /* Can be held       */
#define ITEM_WEAR_THROW         15 /* Can be thrown           32768 */
#define ITEM_WEAR_ABLEGS        16 /* Can be worn about legs  65536 */
#define ITEM_WEAR_FACE          17 /* Can be worn as a mask   131072 */
#define ITEM_WEAR_HOVER         18 /* Hovers near head        262144 */


/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW           0   /* Item is glowing      */
#define ITEM_HUM            1   /* Item is humming      */
#define ITEM_NORENT         2   /* Item cannot be rented    */
#define ITEM_NODONATE       3   /* Item cannot be donated   */
#define ITEM_NOINVIS        4   /* Item cannot be made invis    */
#define ITEM_INVISIBLE      5   /* Item is invisible        */
#define ITEM_MAGIC          6   /* Item is magical      */
#define ITEM_NODROP         7   /* Item is cursed: can't drop   */
#define ITEM_BLESS          8   /* Item is blessed      */
#define ITEM_ANTI_GOOD      9   /* Not usable by good people    */
#define ITEM_ANTI_EVIL      10  /* Not usable by evil people    */
#define ITEM_ANTI_NEUTRAL   11  /* Not usable by neutral people */
#define ITEM_ANTI_MAGIC_USER    12  /* Not usable by mages      */
#define ITEM_ANTI_CLERIC    13  /* Not usable by clerics    */
#define ITEM_ANTI_THIEF     14  /* Not usable by thieves    */
#define ITEM_ANTI_WARRIOR   15  /* Not usable by warriors   */
#define ITEM_NOSELL     16  /* Shopkeepers won't touch it   */
#define ITEM_TAKE_NAME      17  /* takes on the user's name     */
#define ITEM_ANTI_PSIONIC   18  /* not useable by psionics      */
#define ITEM_ANTI_NINJA     19  /* not useable by ninja         */
#define ITEM_ANTI_PALADIN   20
#define ITEM_ANTI_MAGUS     21
#define ITEM_ANTI_ASSASSIN  22
#define ITEM_ANTI_AVATAR    23
#define ITEM_RARE       24  /* Item is RARE (boot only)     */
#define ITEM_NOLOCATE           25      /* Item is unlocatable          */
#define ITEM_ANTI_RANGER        26      /* not useable by rangers       */
#define ITEM_ANTI_MYSTIC        27      /* not useable by mystics       */
#define ITEM_TWO_HANDED     28  /* for two-handed weapons ONLY  */

/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0   /* No effect            */
#define APPLY_STR               1   /* Apply to strength        */
#define APPLY_DEX               2   /* Apply to dexterity       */
#define APPLY_INT               3   /* Apply to intelligence    */
#define APPLY_WIS               4   /* Apply to wisdom      */
#define APPLY_CON               5   /* Apply to constitution    */
#define APPLY_CHA       6   /* Apply to charisma        */
#define APPLY_CLASS             7   /* Reserved         */
#define APPLY_LEVEL             8   /* Reserved         */
#define APPLY_AGE               9   /* Apply to age         */
#define APPLY_CHAR_WEIGHT      10   /* Apply to weight      */
#define APPLY_CHAR_HEIGHT      11   /* Apply to height      */
#define APPLY_MANA             12   /* Apply to max mana        */
#define APPLY_HIT              13   /* Apply to max hit points  */
#define APPLY_MOVE             14   /* Apply to max move points */
#define APPLY_GOLD             15   /* Reserved         */
#define APPLY_EXP              16   /* Reserved         */
#define APPLY_AC               17   /* Apply to Armor Class     */
#define APPLY_HITROLL          18   /* Apply to hitroll     */
#define APPLY_DAMROLL          19   /* Apply to damage roll     */
#define APPLY_SAVING_PARA      20   /* Apply to save throw: paralz  */
#define APPLY_SAVING_ROD       21   /* Apply to save throw: rods    */
#define APPLY_SAVING_PETRI     22   /* Apply to save throw: petrif  */
#define APPLY_SAVING_BREATH    23   /* Apply to save throw: breath  */
#define APPLY_SAVING_SPELL     24   /* Apply to save throw: spells  */
#define APPLY_RACE_HATE        25       /* Actually a flag :)           */
#define APPLY_HIT_REGEN        26       /* regen even faster while sleeping */
#define APPLY_MANA_REGEN       27       /* regen even faster while sleeping */
#define APPLY_MOVE_REGEN       28       /* regen even faster while sleeping */
#define APPLY_SPELL            29       /* affect the user with a spell */

/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)    /* Container can be closed  */
#define CONT_PICKPROOF      (1 << 1)    /* Container is pickproof   */
#define CONT_CLOSED         (1 << 2)    /* Container is closed      */
#define CONT_LOCKED         (1 << 3)    /* Container is locked      */


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


/* other miscellaneous defines *******************************************/

#define RF_ARRAY_MAX    4
#define PM_ARRAY_MAX    4
#define PR_ARRAY_MAX    4
#define AF_ARRAY_MAX    4
#define TW_ARRAY_MAX    4
#define EF_ARRAY_MAX    4

/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2


/* Sun state for weather_data */
#define SUN_DARK    0
#define SUN_RISE    1
#define SUN_LIGHT   2
#define SUN_SET     3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS   0
#define SKY_CLOUDY  1
#define SKY_RAINING 2
#define SKY_LIGHTNING   3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


/* other #defined constants **********************************************/
#define MOON_NEW           0
#define MOON_QUARTER_FULL  1
#define MOON_HALF_FULL     2
#define MOON_THREE_FULL    3
#define MOON_FULL          4
#define MOON_QUARTER_EMPTY 5
#define MOON_HALF_EMPTY    6
#define MOON_THREE_EMPTY   7

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 */
#define LVL_IMPL    40
#define LEVEL_IMPL      LVL_IMPL
#define LVL_GRGOD   38
#define LEVEL_GRGOD LVL_GRGOD
#define LVL_HIGOD       36
#define LEVEL_HIGOD     LVL_HIGOD
#define LVL_LEGEND  35
#define LEVEL_LEGEND    LVL_LEGEND
#define LVL_GOD     34
#define LEVEL_GOD   LVL_GOD
#define LVL_IMMORT  31
#define LEVEL_IMMORT    LVL_IMMORT

#define LVL_FREEZE  LVL_GRGOD
#define LEVEL_FREEZE    LVL_FREEZE

#define NUM_OF_DIRS 6   /* number of directions in a room (nsewud) */

#define OPT_USEC    100000  /* 10 passes per second */
#define PASSES_PER_SEC  (1000000 / OPT_USEC)
#define RL_SEC      * PASSES_PER_SEC

#define PULSE_ZONE      (10 RL_SEC)
#define PULSE_MOBILE    (4 RL_SEC)
#define PULSE_VIOLENCE  (2 RL_SEC)

#define MAX_SOCK_BUF       (12 * 1024)  /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH  192 /* Max length of prompt */
#define GARBAGE_SPACE      32           /* Space for ** OVERFLOW** etc */
#define SMALL_BUFSIZE      1024         /* Static output buffer size */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE   (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define MAX_STRING_LENGTH   8192   /* should be 8192 */
#define MAX_INPUT_LENGTH    256 /* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH    512 /* Max size of *raw* input */
#define MAX_MESSAGES        70
#define MAX_NAME_LENGTH     20  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH      10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH    80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH     30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define IDENT_LENGTH        9
#define EXDSCR_LENGTH       240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE      3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS      200 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT      32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT      6 /* Used in obj_file_elem *DO*NOT*CHANGE* */


/* script related defines *******************************************/

/* used by script_data.lua_functions */
/* Mobile flags */
#define MS_NONE         (1 << 0)
#define MS_BRIBE    (1 << 1)
#define MS_GREET    (1 << 2)
#define MS_ONGIVE   (1 << 3)
#define MS_SOUND    (1 << 4)
#define MS_DEATH        (1 << 5)
#define MS_ONPULSE_ALL  (1 << 6)
#define MS_ONPULSE_PC   (1 << 7)
#define MS_FIGHTING (1 << 8)
#define MS_ONCMD    (1 << 9)

/* room flags */
#define RS_NONE         (1 << 0)
#define RS_ENTER        (1 << 1)
#define RS_ONPULSE      (1 << 2)
#define RS_ONDROP       (1 << 3)
#define RS_ONGET        (1 << 4)
#define RS_ONCMD    (1 << 5)

/* Objects */
#define OS_NONE         (1 << 0)
#define OS_ONCMD        (1 << 1)
#define OS_ONPULSE      (1 << 2)


/* Lua types, used by run_script */
#define LT_MOB      "mob"
#define LT_OBJ      "obj"
#define LT_ROOM     "room"

/**********************************************************************
* Structures                                                          *
**********************************************************************/

typedef unsigned long int       bitvector_t;

typedef signed char     sbyte;
typedef unsigned char       ubyte;
typedef signed short int    sh_int;
typedef unsigned short int  ush_int;
typedef char            byte;

typedef int room_num;

/* BEWARE: obj_num is used by obj_file_elem. Do not change! */
typedef sh_int  obj_num;

/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char *keyword;                 /* Keyword in look/examine          */
   char *description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/


/* object flags; used in obj_data */
struct obj_flag_data {
   int  value[4];           /* Values of the item (see list)    */
   byte type_flag;          /* Type of item             */
   int  wear_flags[TW_ARRAY_MAX];   /* Where you can wear it        */
   int  extra_flags[EF_ARRAY_MAX];  /* If it hums, glows, etc.      */
   int  weight;             /* Weigt what else                  */
   int  cost;               /* Value when sold (gp.)            */
   float load;                  /* Percent chance to load           */
   int  timer;              /* Timer for object                 */
   int  bitvector[AF_ARRAY_MAX];    /* To set chars bits                */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};


/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_num item_number;     /* Where in data-base           */
   room_num in_room;        /* In what room -1 when conta/carr  */

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   char *name;                    /* Title of object :get etc.        */
   char *description;         /* When in room                     */
   char *short_description;       /* when worn/carry/in cont.         */
   char *action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;     /* Worn by?                 */
   sh_int worn_on;        /* Worn where?              */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files        */
struct obj_file_elem {
   obj_num item_number;
   sh_int locate;  /* that's the (1+)wear-location (when equipped) or
              (20+)index in obj file (if it's in a container) BK */
   int  value[4];
   int  extra_flags[EF_ARRAY_MAX];
   int  weight;
   int  timer;
   int  bitvector[AF_ARRAY_MAX];
   struct obj_affected_type affected[MAX_OBJ_AFFECT];

   /* new data added to store do_string objects */
   char name[128];
   char shortd[128];
   char desc[256];

};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
   int  time;
   int  rentcode;
   int  net_cost_per_diem;
   int  gold;
   int  account;
   int  nitems;
   int  spare0;
   int  spare1;
   int  spare2;
   int  spare3;
   int  spare4;
   int  spare5;
   int  spare6;
   int  spare7;
};
/* ======================================================================= */


/* room-related structures ************************************************/


struct room_direction_data {
   char *general_description;       /* When look DIR.           */

   char *keyword;       /* for open/close           */

   sh_int exit_info;        /* Exit info                */
   obj_num key;         /* Key's number (-1 for no key)     */
   room_num to_room;        /* Where direction leads (NOWHERE)  */
};


/* ================== Memory Structure for room ======================= */
struct room_data {
   room_num number;     /* Rooms number (vnum)            */
   sh_int zone;                 /* Room zone (for resetting)          */
   int  sector_type;            /* sector type (move/hide)            */
   char *name;                  /* Rooms name 'You are ...'           */
   char *description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   int room_flags[RF_ARRAY_MAX];/* DEATH,DARK ... etc                 */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);
   struct script_data *script;

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
};
/* ====================================================================== */


/* char-related structures ************************************************/


/* memory structure for characters */
struct memory_rec_struct {
   long id;
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
   int played;      /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char *name;         /* PC / NPC s name (kill ...  )         */
   char *short_descr;  /* for NPC 'actions'                    */
   char *long_descr;   /* for 'look'                   */
   char *description;  /* Extra descriptions                   */
   char *title;        /* PC / NPC's title                     */
   byte sex;           /* PC / NPC's sex                       */
   byte class;         /* PC / NPC's class             */
   byte level;         /* PC / NPC's level                     */
   int  hometown;      /* PC s Hometown (zone)                 */
   struct time_data time;  /* PC's AGE in days                 */
   ubyte weight;       /* PC / NPC's weight                    */
   ubyte height;       /* PC / NPC's height                    */
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   sh_int mana;
   sh_int max_mana;     /* Max move for PC/NPC             */
   sh_int hit;
   sh_int max_hit;      /* Max hit for PC/NPC                      */
   sh_int move;
   sh_int max_move;     /* Max move for PC/NPC                     */

   sh_int armor;        /* Internal -100..100, external -10..10 AC */
   int  gold;           /* Money carried                           */
   int  bank_gold;  /* Gold the char has in a bank account     */
   int  exp;            /* The experience of the player            */

   sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
   sbyte damroll;       /* Any bonus or penalty to the damage roll */
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
   int  alignment;      /* +-1000 for alignments                */
   long idnum;          /* player's idnum; -1 for mobiles   */
   int act[PM_ARRAY_MAX];   /* act flag for NPC's; player flag for PC's */

   int affected_by[AF_ARRAY_MAX];/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)      */
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
  struct char_data *fighting;   /* Opponent             */
  struct char_data *hunting;    /* Mobile hunted by this char       */
  long hunting_id;      /* Char id of player hunted         */

  byte position;        /* Standing, fighting, sleeping, etc.   */

  int  carry_weight;        /* Carried weight           */
  byte carry_items;     /* Number of items carried      */
  int  timer;           /* Timer for update         */

  long race_hate[5];            /* 5 races you're allowed to hate :)    */

  bool parried;                 /* Am i going to be parried next round? */

  int  jailtimer;               /* how long until out of jail           */

  struct char_special_data_saved saved; /* constants saved in plrfile   */
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
   byte skills[MAX_SKILLS+1];   /* array of skills plus skill 0     */
   byte PADDING0;       /* used to be spells_to_learn       */
   bool talks[MAX_TONGUE];  /* PC s Tongues 0 for NPC       */
   int  wimp_level;     /* Below this # of hit points, flee!    */
   byte freeze_level;       /* Level of god who froze char, if any  */
   sh_int invis_level;      /* level of invisibility        */
   sh_int load_room;        /* Which room to place char in      */
   int pref[PR_ARRAY_MAX];  /* preference flags for PC's.       */
   ubyte bad_pws;       /* number of bad password attemps   */
   sbyte conditions[3];         /* Drunk, full, thirsty         */

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   long pkcount;                /* Character's num of pks               */
   long killcount;              /* Character's num of mob kills         */
   long deathcount;             /* Character's num f deaths             */
   int race;                /* Character's race                     */
   ubyte orig_con;      /* Characters con before con loss       */
   ubyte clan_rank;
   int spells_to_learn;     /* How many can you learn yet this level*/
   int olc_zone;
   int tattoo;
   int tattimer;
   int infobar;                 /* status of infobar */
   int size;                    /* screen size */
   int clan;
   int spare13;
   int spare14;
   int mount_vnum;      /* vnum of the mount            */
   int mount_cost_day;      /* cost to rent mount per day       */
   long lastdeath;              /* when was the last time they died?    */
   long mount_rent;     /* time when mount was rented       */
   long spare19;
   long spare20;
   long spare21;
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

   char *poofin;        /* Description on arrival of a god.     */
   char *poofout;       /* Description upon a god's exit.       */
   struct alias *aliases;   /* Character's aliases          */
   long last_tell;      /* idnum of last tell from      */
   void *last_olc_targ;     /* olc control              */
   int last_olc_mode;       /* olc control              */
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   byte last_direction;     /* The last direction the monster went     */
   int  attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   memory_rec *memory;      /* List of attackers to remember           */
   byte damnodice;          /* The number of damage dice's         */
   byte damsizedice;        /* The size of the damage dice's           */
   int wait_state;      /* Wait state for bashed mobs          */
   int  race;
   char *noise;             /* sound the mob makes                     */
};

/* Serapis APPLY_SPELL Wed Jun 10 09:54:40 1998 -- Begin*/
typedef enum {
   BY_SPELL,
   BY_OBJ,
   BY_RACE
} by_types_enum;

struct master_affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long bitvector;       /* Tells which bits to set (AFF_XXX)       */
   by_types_enum by_type; /* What caused this affection? */
   int obj_num;           /* If caused by an obj, which obj? */

   struct master_affected_type *next;
};
/* Serapis APPLY_SPELL Wed Jun 10 09:54:40 1998 -- End*/


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long bitvector;       /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};

struct last_checked {
   int mana;
   int maxmana;
   int hit;
   int maxhit;
   int move;
   int maxmove;
   int exp;
   int gold;
};

/* ================== Structure for player/non-player ===================== */
struct char_data {
   int pfilepos;             /* playerfile pos        */
   sh_int nr;                            /* Mob's rnum            */
   room_num in_room;                     /* Location (real room number)   */
   room_num was_in_room;         /* location for linkdead people  */
   int wait;                     /* wait for how many loops   */

   struct char_player_data player;       /* Normal data                   */
   struct char_ability_data real_abils;  /* Abilities without modifiers   */
   struct char_ability_data aff_abils;   /* Abils with spells/stones/etc  */
   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;  /* PC/NPC specials    */
   struct player_special_data *player_specials; /* PC specials        */
   struct mob_special_data mob_specials;    /* NPC specials       */

   struct master_affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   struct char_data *next_in_room;     /* For room->people - list         */
   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */

   struct follow_type *followers;        /* List of chars followers       */
   struct char_data *master;             /* Who is char following?        */

   struct last_checked last;             /* For updating displays         */

   struct event *action;
};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile       */
struct char_file_u {
   /* char_player_data */
   char name[MAX_NAME_LENGTH+1];
   char description[EXDSCR_LENGTH];
   char title[MAX_TITLE_LENGTH+1];
   byte sex;
   byte class;
   byte level;
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   int  played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;

   char pwd[MAX_PWD_LENGTH+1];    /* character's password */

   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];

   time_t last_logon;       /* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];    /* host of last logon */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char *text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};

/* Compression state structure */
struct compr {
  int state;                    /* 0 - disabled, 1 - waiting, 2 - enabled */

  Bytef *buff_out;              /* output buffer */
  int max_out;                  /* max size of input buffer */
  int size_out;                 /* size of data in output buffer */

  Bytef *buff_in;               /* input buffer */
  int max_in;                   /* max size of input buffer */
  int size_in;                  /* size of data in input buffer */

  z_streamp stream;             /* zlib stream */
};

struct descriptor_data {
   int descriptor;      /* file descriptor for socket       */
   int ident_sock;      /* socket used for ident process        */
   unsigned short peer_port;    /* port of peer                     */
   char host[HOST_LENGTH+1];    /* hostname             */
   byte close_me;       /* flag: the desc. should be closed     */
   byte bad_pws;        /* number of bad pw attemps this login  */
   byte idle_tics;      /* tics idle at password prompt     */
   int  connected;      /* mode of 'connectedness'      */
   int  desc_num;       /* unique num assigned to desc      */
   time_t login_time;       /* when the person connected        */
   char *showstr_head;      /* for keeping track of an internal str */
   char **showstr_vector;   /* for paging through texts     */
   int  showstr_count;      /* number of pages to page through  */
   int  showstr_page;       /* which page are we currently showing? */
   char **str;          /* for the modify-str system        */
   char *backstr;               /* backup string for modify-str system  */
   size_t max_str;      /*      -           */
   long mail_to;        /* name for mail system         */
   int  has_prompt;     /* is the user at a prompt?             */
   char inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input       */
   char last_input[MAX_INPUT_LENGTH]; /* the last input         */
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer     */
   char *output;        /* ptr to the current output buffer */
   int  bufptr;         /* ptr to end of current output     */
   int  bufspace;       /* space left in the output buffer  */
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;      /* q of unprocessed input       */
   struct char_data *character; /* linked to char           */
   struct char_data *original;  /* original char if switched        */
   struct descriptor_data *snooping; /* Who is this char snooping   */
   struct descriptor_data *snoop_by; /* And who is snooping this char   */
   struct descriptor_data *next; /* link to next descriptor     */
   struct olc_data *olc;         /*. OLC info - defined in olc.h   .*/
   struct compr *comp;          /* compression info                     */
};


/* other miscellaneous structures ***************************************/


struct msg_type {
   char *attacker_msg;  /* message to attacker */
   char *victim_msg;    /* message to victim   */
   char *room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg; /* messages when death          */
   struct msg_type miss_msg;    /* messages when miss           */
   struct msg_type hit_msg; /* messages when hit            */
   struct msg_type god_msg; /* messages when hit on god     */
   struct message_type *next;   /* to next messages of this kind.   */
};


struct message_list {
   int  a_type;         /* Attack type              */
   int  number_of_attacks;  /* How many attack messages to chose from. */
   struct message_type *msg;    /* List of messages.            */
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
   sh_int todam;    /* Damage Bonus/Penalty                */
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
   int  pressure;   /* How is the pressure ( Mb ) */
   int  change; /* How fast and what way does it change. */
   int  sky;    /* How is the sky. */
   int  sunlight;   /* And how much sun. */
};


struct title_type {
   char *title_m;
   char *title_f;
   int  exp;
};


/* element in monster and object index-tables   */
struct index_data {
   int  virtual;    /* virtual number of this mob/obj           */
   int  number;     /* number of existing units of this mob/obj */
   SPECIAL(*func);
   struct script_data *script;
};

/* review structure */
struct review_t {
  int invis; /* invis level */
  char name[MAX_STRING_LENGTH];
  char string[MAX_STRING_LENGTH];
};

struct tat_data {
        int tattoo_num;
        int price;
        char descrip[80];
        char effects[80];
};


/*
 * Field Objects
 */
typedef enum {
   FO_AFFECT,
   FO_DAMAGE,
   FO_SOLID
} fo_types_enum;

typedef struct field_object_data {
   int obj_vnum;
   fo_types_enum fo_type;
   char wear_off_msg[256];
   int worn_off_obj_num;
}field_object_data_t;

/* for saving game time */
struct time_write {
     int year, month, day;
   };


/* in constants.c */
#define NUM_FOS 3


/* for scripts */
struct script_data
{
  const char *name;
  ush_int lua_functions;
};

#endif /* _STRUCTS_H */

