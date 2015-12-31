/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
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

/* $Id: constants.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"

/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */
const char *phases[] =
{
    "not in the sky",
    "one-quarter full(waxing)",
    "half full(waxing)",
    "three-quarters full(waxing)",
    "full",
    "three-quarters full(waning)",
    "half full(waning)",
    "one-quarter full(waning)",
    "\n"
};

const char *hometowns[] =
{
    "!Bad Hometown - Tell a God!",
        "Kir Drax'in",
    "Kir-Oshi",
    "Alaozar",
    "Add a new hometown here!",
    "\n"
};

const char *abil_names[] =
{
  "terrible",
  "terrible",
  "awful",
  "awful",
  "bad",
  "bad",  /* 5 */
  "poor",
  "poor",
  "below average",
  "average",
  "average",   /* 10 */
  "decent",
  "decent",
  "good",
  "good",
  "very good",     /* 15 */
  "very good",
  "excellent",
  "excellent",
  "superior",
  "godlike",      /* 20 */
  "godlike",
  "godlike",
  "godlike",
  "godlike",
  "godlike",
  "\n"
};

const char *crowd_size[] =
{
  "no one",
  "someone",
  "a few people",
  "a few people",
  "a group of people",
  "a group of people", /* 5 */
  "a large group of people",
  "a large group of people",
  "a large group of people",
  "a large group of people",
  "a crowd of people",  /* 10 */
  "a crowd of people",
  "a crowd of people",
  "a large crowd of people",
  "a large crowd of people",
  "a large mob",       /* 15 */
  "\n"
};

/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};


const char *races[] =
{
  "Human",
  "Elven",
  "Dwarven",
  "Kenderkin",
  "Minotauran",
  "Rakshasan",
  "Ssauran",
  "\n"
};

const char *mob_races[] =
{
  "Human",
  "Elf",
  "Dwarf",
  "Kender",
  "Centaur",
  "Rakshasa",
  "Troll",
  "Lycanthrope",
  "Vampire",
  "Undead",
  "Dragon",
  "Demon",
  "Horse",
  "Reptile",
  "Arachnid",
  "Rodent",
  "Other",
  "Vegetable",
  "Giant",
  "Demi-god",
  "Ogre",
  "Insect",
  "Mammal",
  "Fish",
  "Avian",
  "Magical Construct",
  "Amphibian",
  "Humanoid",
  "Faery",
  "Ssaur",
  "Minotaur",
  "\n"
};

const char *tattoos[] =
{
  "None",
  "of a green dragon",
  "in a tribal design",
  "of a flaming skull",
  "of a leaping tiger",
  "of an ice worm",
  "of an open eye",
  "of crossed swords",
  "of a screaming eagle",
  "of a heart",
  "of a star",
  "of a ship",
  "of a spider",
  "of the symbol of the Jyhad",
  "of the word 'MOM'",
  "of an angel",
  "of a fox",
  "of an owl"
};

const char *race_menu =
"\r\n"
"Choose a race:\r\n"
"  [H]uman        [E]lven       [D]warven      [K]enderkin\r\n"
"  [M]inotaur     [R]akshasan   [S]sauran\r\n"
"  [?]Help on races in general\r\n"
"  [?<race abbreviation>] Help on a specific race (i.e ?D for help on dwarves)"
"\r\n";

const char *race_help =
"\r\n"
"Your race is pretty much class independant; it affects innate abilities "
"such\r\nas:\r\n"
"The type of terrain you see best in: \r\n"
"       RAKSHASA: desert              SSAUR: swamplands\r\n"
"       MINOTAUR & ELF: forest        DWARF: mountains\r\n"
"       KENDER & HUMAN: fairly good everywhere.\r\n"
"Magick resistance.: Elves and dwarves are a bit more hearty in this area.\r\n"
"Attitudes: Humans abound, so they are often suspicious of other races and\r\n"
"       give preferential treatment to their own kind.\r\n"
"Kender tend to 'acquire' other's objects unknowingly, and make excellent\r\n"
"       thieves. Only humans can belong to the ninja class.\r\n"
"Each race has its own language.\r\n";

const char *help_human =
"\r\n"
"Humans are the most common race on this world, and come in all sorts of shapes"
"\r\n"
"and sizes. The appearance of humans are not the only thing that varies about"
"\r\n"
"them, though, some are evil as sin, while others are good as good can be, but"
"\r\n"
"most you shall find on your adventures are neutral, and will just mind their"
"\r\n"
"own business and pay no attention to the affairs of adventurers. Also, humans"
"\r\n"
"are the only race that can become ninjas, the dangerous oriental mercenaries."
"\r\n"
"They adapt easily to most climes, allowing them to build cities in almost any"
"\r\n"
"location.\r\n";

const char *help_dwarf =
"\r\n"
"Dwarves are a noble race of demihumans who dwell under the earth, forging"
"\r\n"
"great cities and waging massive wars against the forces of chaos and evil."
"\r\n"
"Dwarves also have much in common with the rocks and gems they love to work,"
"\r\n"
"for they are both hard and unyielding. It's often been said that it's easier"
"\r\n"
"to make a stone weep than it is to change a dwarf's mind. Standing an average"
"\r\n"
"of four-and-a-half feet tall, dwarves tend to be stocky and muscular. They"
"\r\n"
"have ruddy cheeks and bright eyes. Their skin is typically deep tan or light"
"\r\n"
"brown. Their hair is usually black, grey, or brown, and worn long, though not"
"\r\n"
"long enough to impair vision in any way. They favor long beards and moustaches"
"\r\n"
"as well.\r\n";

const char *help_elf =
"\r\n"
"Though their lives span several human generations, elves appear at first"
"\r\n"
"glance to be frail when compared to man, due to their delicate and finely"
"\r\n"
"chiseled features. Elves have very pale complextions, which is odd because"
"\r\n"
"they spend a great deal of time outdoors. They tend to be slim, almost "
"\r\n"
"fragile. Though they are not as sturdy as humans, elves are much more agile."
"\r\n"
"Elves have learned that it is very important to understand the creatures, both"
"\r\n"
"good and evil, that share their forest homes.\r\n";

const char *help_kender =
"\r\n"
"Kender are small, kind, but somewhat annoying, elf-like beings that have"
"\r\n"
"recently spread across the globe. They do not seem to have any sort of kingdom"
"\r\n"
"and most are found just wandering throughout the lands, exploring. Although"
"\r\n"
"some are trained thieves, the whole of the kender race seems to have a knack"
"\r\n"
"for stealing, and occasionally, without even noticing it sometimes, they have"
"\r\n"
"been known to steal from friends and enemies alike. They act much like humans,"
"\r\n"
"but four things make a kender's personality drastically different from that of"
"\r\n"
"a typical human. Kender are utterly fearless, insatiably curious, unstoppably"
"\r\n"
"mobile and independant, and will pick up anything that is not nailed down."
"\r\n";

const char *help_minotaur =
"\r\n"
"Minotaurs are either cursed humans or the offspring of minotaurs and humans."
"\r\n"
"They are usually found dwelling in underground labyrinths, for they seem to"
"\r\n"
"have an innate ability to manuver in these places, and do not often lose their"
"\r\n"
"sense of direction. Minotaurs are huge, well over seven feet tall, and their"
"\r\n"
"broad bodies ripple with muscles. They have the head of a bull but the body of"
"\r\n"
"a human male, there have been accounts of female minotaurs, but they are rare."
"\r\n"
"The color of their fur ranges from brown to black, while their body coloring"
"\r\n"
"varies, as would a normal human's. Although they usually dwell in mazes"
"\r\n"
"beneath the earth, it is noted that they also see very well in forests.\r\n";

const char *help_rakshasa =
"\r\n"
"Rakshasas are a race of malevolent spirits encased in flesh that hunt and"
"\r\n"
"torment humanity. No one knows where these creatures originate, some say they"
"\r\n"
"are the embodiment of nightmares. The only way to describe their form is that"
"\r\n"
"they are humanoid tigers, with hands whose palms curve backward, away from the"
"\r\n"
"body. Most of the worlds rakshasa are evil, but recently many have decided to"
"\r\n"
"stop their tyrannical living and become adventurers, although they still"
"\r\n"
"retain their fondness towards the great sandy wastes of their homeland.\r\n";

const char *help_ssaur =
"\r\n"
"Ssaurs are a relatively new race in the world. They are a more evolved type of"
"\r\n"
"lizardman, and most are more intelligent than their aggressive ancestors, and"
"\r\n"
"for that are shunned from the lizardman tribes, and the few that are born into"
"\r\n"
"those tribes are cast out almost as soon as they are hatched. Other than the "
"\r\n"
"intelligence, they appear to be the same as lizardman, although less evil-"
"\r\n"
"looking. Ssaurs spend most of their lives in swamps and marshes, but some have"
"\r\n"
"been known to adventure far away from their homes.\r\n";





const int intelligent_races[] = {
  RACE_NPCHUMAN,
  RACE_NPCELF   ,
  RACE_NPCDWARF  ,
  RACE_NPCKENDER,
  RACE_NPCCENTAUR,
  RACE_NPCRAKSHASA,
  RACE_TROLL       ,
  RACE_VAMPIRE   ,
  RACE_LYCANTHROPE ,
  RACE_UNDEAD     ,
  RACE_DRAGON     ,
  RACE_DEMON    ,
  RACE_GIANT   ,
  RACE_DEMIGOD,
  RACE_OGRE,
  RACE_HUMANOID ,
  RACE_FAERY    ,
  RACE_NPCSSAUR ,
  RACE_NPCMINOTAUR
};
/* if you anything above, change NUM_INTEL_RACES in structs.h */


/* ROOM_x */
const char *room_bits[] = {
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE",
  "HCRSH",
  "ATRIUM",
  "OLC",
  "*",              /* BFS MARK */
  "NEUTRAL",
  "BFR",
  "REGENROOM",
  "NO_WHO_ROOM",
  "**",             /* secret mark */
  "FLOW_NORTH",
  "FLOW_SOUTH",
  "FLOW_EAST",
  "FLOW_WEST",
  "FLOW_UP",
  "FLOW_DOWN",
  "ARENA"
};


/* EX_x */
const char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "\n"
};


/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "In Flight",
  "Desert",
  "Fire",
  "Earth",
  "Wind",
  "Water",
  "Swamp",
  "\n"
};


/* SEX_x */
const char *genders[] =
{
  "Neutral",
  "Male",
  "Female"
};


/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};


/* PLR_x */
const char *player_bits[] = {
  "OUTLAW",
  "NOTHIN",
  "FROZEN",
  "DONTSET",
  "WRITING",
  "MAILING",
  "CSH",
  "SITEOK",
  "NOSHOUT",
  "NOTITLE",
  "DELETED",
  "LOADRM",
  "!WIZL",
  "!DEL",
  "INVST",
  "CRYO",
  "WEREW",
  "VAMP",
  "IT",
  "CHOSEN",
  "REMORT",
  "\n"
};


/* MOB_x */
const char *action_bits[] = {
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "AWARE",
  "AGGR",
  "STAY-ZONE",
  "WIMPY",
  "AGGR_EVIL",
  "AGGR_GOOD",
  "AGGR_NEUTRAL",
  "MEMORY",
  "HELPER",
  "!CHARM",
  "!SUMMN",
  "!SLEEP",
  "!BASH",
  "!BLIND",
  "HUNTER",
  "AGGR24",
  "RNDLD_ZONE",
  "MOUNTABLE",
  "RARE",
  "LOOTS",
  "OKGIVE",
  "\n"
};


/* PRF_x */
const char *preference_bits[] = {
  "BRIEF",
  "COMPACT",
  "DEAF",
  "!TELL",
  "D_HP",
  "D_MANA",
  "D_MOVE",
  "AUTOEX",
  "!HASS",
  "QUEST",
  "SUMN",
  "!REP",
  "LIGHT",
  "C1",
  "C2",
  "!WIZ",
  "L1",
  "L2",
  "!AUC",
  "!GOS",
  "!GTZ",
  "RMFLG",
  "AFK",
  "AUTOLOOT",
  "AUTOGOLD",
  "AUTOSPLIT",
  "D_TANK",
  "D_TARGET",
  "!NEW",
  "INACTIVE",
  "!CTELL",
  "!BROAD",
  "\n"
};

/* MS_x */
const char *mscript_bits[] =
{
  "NONE",
  "BRIBE",
  "GREET",
  "ONGIVE",
  "SOUND",
  "DEATH",
  "ONPULSE (ALL)",
  "ONPULSE (PC)",
  "FIGHT",
  "ONCMD"
};

/* RS_x */
const char *rscript_bits[] =
{
  "NONE",
  "ENTER",
  "ONPULSE",
  "ONDROP",
  "ONGET",
  "ONCMD"
};

/* OS_x */
const char *oscript_bits[] =
{
  "NONE",
  "ONCMD",
  "ONPULSE"
};

/* AFF_x */
const char *affected_bits[] =
{
  "BLIND",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "WATERWALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "FLESH-ALTER",
  "DODGE",
  "SNEAK",
  "HIDE",
  "BERSERK",
  "CHARM",
  "FOLLOW",
  "WIMPY",
  "KUJI-KIRI",
  "CUTTHROAT",
  "FLY",
  "WEREWOLF",
  "VAMPIRE",
  "MOUNTED",
  "INVULN",
  "FLAMING",
  "NOTHING",
  "HASTE",
  "SLOW",
  "DREAM",
  "WATERBREATHE",
  "METALSKIN",
  "ROBBED",
  "\n"
};

/* this corresponds to the affected_bits array; these are
   the "spell names" for the affected bits. 90% of them are not used.
   This is for displaying affected_bits to the player instead of just
   showing them SANCT, INFRA, etc. It looks a lot cleaner this way.
            -rparet 19980731  */

const char *affected_names[] = {
  "blind",
  "invisibility",
  "detect alignment",
  "detect invisibility",
  "detect magic",
  "sense life",
  "waterwalk",
  "sanctuary",
  "group",
  "curse",
  "infravision",
  "poison",
  "protection from evil",
  "protection from good",
  "sleep",
  "no track",
  "flesh alter",
  "dodge",
  "sneak",
  "hide",
  "berserk",
  "charm",
  "follow",
  "wimpy",
  "kuji-kiri",
  "cutthroat",
  "fly",
  "werewolf",
  "vampire",
  "mounted",
  "invulnerability",
  "flaming",
  "nothing",
  "haste",
  "slow",
  "dream",
  "waterbreathe",
  "metalskin",
  "robbed"
};

/* CON_x */
const char *connected_types[] = {
  "Playing",
  "Disconnecting",
  "Get name",
  "Confirm name",
  "Get password",
  "Get new PW",
  "Confirm new PW",
  "Select sex",
  "Select class",
  "Reading MOTD",
  "Main Menu",
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete 1",
  "Self-Delete 2",
  "Object edit",
  "Room edit",
  "Zone edit",
  "Mobile edit",
  "Shop edit",
  "Ident conning",
  "Ident conned",
  "Ident reading",
  "Ident read",
  "Asking name",
  "Asking race",
  "Asking color",
  "Getting hometown",
  "Rolling abilities 1",
  "Rolling abilities 2",
  "\n"
};


/* WEAR_x - for eq list */
const char *where[] = {
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               ",
  "<held>               ",
  "<worn about legs>    ",
  "<worn on face>       ",
  "<hovering near head> "
};


/* WEAR_x - for stat */
const char *equipment_types[] = {
  "Used as light",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "Wielded for throwing",
  "Worn about legs",
  "Worn on face",
  "Hovering near head",
  "\n"
};


/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQ CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "THROW",
  "ABLEGS",
  "FACE",
  "HOVER",
  "\n"
};


/* ITEM_x (extra bits) */
const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVIS",
  "MAGIC",
  "!DROP",
  "BLESS",
  "!GOOD",
  "!EVIL",
  "!NEU",
  "!MAGE",
  "!CLE",
  "!THI",
  "!WAR",
  "!SELL",
  "NAMED",
  "!PSI",
  "!NIN",
  "!PAL",
  "!MAGUS",
  "!ASS",
  "!AVA",
  "RARE",
  "!LOCATE",
  "!RAN",
  "!MYS",
  "TWOHANDS",
  "\n"
};


/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "RACE_HATE",
  "HIT_REGEN",
  "MANA_REGEN",
  "MOVE_REGEN",
  "PERM_SPELL",
  "\n"
};


/* CONT_x */
const char *container_bits[] = {
  "CLOSEABLE",
  "PICKPROOF",
  "CLOSED",
  "LOCKED",
  "\n",
};


/* LIQ_x */
const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whiskey",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whiskey",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "water",
  "\n"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13}
};


/* color of the various drinks */
const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear"
};


/* level of fullness for drink containers */
const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[] = {
  {-5, -4, 0, 0},   /* str = 0 */
  {-5, -4, 3, 1},   /* str = 1 */
  {-3, -2, 3, 2},
  {-3, -1, 10, 3},
  {-2, -1, 25, 4},
  {-2, -1, 55, 5},  /* str = 5 */
  {-1, 0, 80, 6},
  {-1, 0, 90, 7},
  {0, 0, 100, 8},
  {0, 0, 100, 9},
  {0, 0, 115, 10},  /* str = 10 */
  {0, 0, 115, 11},
  {0, 0, 140, 12},
  {0, 0, 140, 13},
  {0, 0, 170, 14},
  {0, 0, 170, 15},  /* str = 15 */
  {0, 1, 195, 16},
  {1, 1, 220, 18},
  {1, 2, 255, 20},  /* dex = 18 */
  {3, 7, 640, 40},
  {3, 8, 700, 40},  /* str = 20 */
  {4, 9, 810, 40},
  {4, 10, 970, 40},
  {5, 11, 1130, 40},
  {6, 12, 1440, 40},
  {7, 14, 1750, 40},    /* str = 25 */
  {1, 3, 280, 22},  /* str = 18/0 - 18-50 */
  {2, 3, 305, 24},  /* str = 18/51 - 18-75 */
  {2, 4, 330, 26},  /* str = 18/76 - 18-90 */
  {2, 5, 380, 28},  /* str = 18/91 - 18-99 */
  {3, 6, 480, 30}   /* str = 18/100 */
};



/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[] = {
  {-99, -99, -90, -99, -60},    /* dex = 0 */
  {-90, -90, -60, -90, -50},    /* dex = 1 */
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},    /* dex = 5 */
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},  /* dex = 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},      /* dex = 15 */
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},      /* dex = 18 */
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},     /* dex = 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}      /* dex = 25 */
};



/* [dex] apply (all) */
struct dex_app_type dex_app[] = {
  {-7, -7, 6},      /* dex = 0 */
  {-6, -6, 5},      /* dex = 1 */
  {-4, -4, 5},
  {-3, -3, 4},
  {-2, -2, 3},
  {-1, -1, 2},      /* dex = 5 */
  {0, 0, 1},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},        /* dex = 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},       /* dex = 15 */
  {1, 1, -2},
  {2, 2, -3},
  {2, 2, -4},       /* dex = 18 */
  {3, 3, -4},
  {3, 3, -4},       /* dex = 20 */
  {4, 4, -5},
  {4, 4, -5},
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6}        /* dex = 25 */
};



/* [con] apply (all) */
struct con_app_type con_app[] = {
  {-4, 20},     /* con = 0 */
  {-3, 25},     /* con = 1 */
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},     /* con = 5 */
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},      /* con = 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},      /* con = 15 */
  {2, 95},
  {2, 97},
  {3, 99},      /* con = 18 */
  {3, 99},
  {4, 99},      /* con = 20 */
  {5, 99},
  {5, 99},
  {5, 99},
  {6, 99},
  {6, 99}       /* con = 25 */
};



/* [int] apply (all) */
struct int_app_type int_app[] = {
  {3},      /* int = 0 */
  {5},      /* int = 1 */
  {7},
  {8},
  {9},
  {10},     /* int = 5 */
  {11},
  {12},
  {13},
  {15},
  {17},     /* int = 10 */
  {19},
  {22},
  {25},
  {30},
  {35},     /* int = 15 */
  {40},
  {45},
  {50},     /* int = 18 */
  {53},
  {55},     /* int = 20 */
  {56},
  {57},
  {58},
  {59},
  {60}      /* int = 25 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[] = {
  {0},  /* wis = 0 */
  {0},  /* wis = 1 */
  {0},
  {0},
  {0},
  {0},  /* wis = 5 */
  {0},
  {0},
  {0},
  {0},
  {0},  /* wis = 10 */
  {0},
  {2},
  {2},
  {3},
  {3},  /* wis = 15 */
  {3},
  {4},
  {5},  /* wis = 18 */
  {6},
  {6},  /* wis = 20 */
  {6},
  {6},
  {7},
  {7},
  {7}  /* wis = 25 */
};



const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",      /* 0 */
  "You feel less protected.",   /* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness dissolve.",
  "!Burning Hands!",        /* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Clone!",
  "!Color Spray!",      /* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",      /* 15 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "The detect magic wears off.",/* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",     /* 25 */
  "!Fireball!",
  "!Harm!",
  "!Heal!",
  "You feel yourself exposed.",
  "!Lightning Bolt!",       /* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",     /* 35 */
  "The aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",           /* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your surroundings.",
  "!Animate Dead!",     /* 45 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",   /* 50 */
  "Your feet seem less boyant.",
  "!Mass Heal!",
  "Your feet float back down to the ground.",
  "!UNUSED!",
  "!UNUSED!",                           /*  55  */
  "!SOBRIETY!",
  "!GROUP INVIS!",
  "!HELLFIRE!",
  "!ENCHANT ARMOR!",
  "!IDENTIFY!", /* 60 */
  "!MINDPOKE!",
  "!MINDBLAST!",
  "Your chameleon-like coloring fades to normal.",
  "Your feet float back down to the ground.",
  "Your skin softens.",     /* 65 */
  "The globe of invulnerability around you vanishes.",
  "!VITALITY!",
  "!INVIGORATE!",
  "Your perception returns to normal.",
  "Your perception returns to normal.", /* 70 */
  "!MINDATTACK!",
  "Your adrenaline returns to it's normal production rate.",
  "Your feel your energy shield dissapate.",
  "Your molecular density returns to normal.",
  "!ACID BLAST!",       /* 75 */
  "Your free will returns.",
  "!CELL ADJUSTMENT!",
  "Your trance ends.",
  "!Mirror Image!",
  "!MASS DOMINATE!",        /* 80 */
  "!KATANA!",
  "Your mental capasity returns to normal.",
  "!SOUL LEECH!",
  "!MINDSIGHT!",
  "Your skin fades back into view.",    /* 85 */
  "Your perception of other's emotions ebbs.",
  "!GATE!",
  "The secrets of the world seem less clear.",
  "!LAY HANDS!",
  "!MENTAL LAPSE!",  /* 90 */
  "!SMOKESCREEN!",
  "!DISRUPT!",
  "!DISINTEGRATE",
  "!CALLIOPE!",
  "You no longer feel protected.",  /* 95 */
  "The flames around you subside.",
  "You cease moving rapidly.",
  "The world begins to pick up speed again.",
  " ", /* dream travel */
  "!PSIBLAST!",         /* 100 */
  "!CIRCLE OF SUMMONING!",
  "You feel your breathing return to normal.",
  "!CONJURE ELEMENTAL!"
};


const char *npc_class_types[] = {
  "Normal",
  "Warrior",
  "Mage",
  "\n"
};


const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};


const int movement_loss[] =
{
  2,    /* Inside     */
  2,    /* City       */
  3,    /* Field      */
  4,    /* Forest     */
  5,    /* Hills      */
  7,    /* Mountains  */
  5,    /* Swimming   */
  6,    /* Unswimable */
  2,    /* Flying     */
  6,    /* Underwater */
  8,    /* Desert     */
  6,    /* Fire Plane */
  6,    /* Eart Plane */
  6,    /* Wind Plane */
  6,    /* Water Plane*/
  4,    /* Swamp      */
};


const char *weekdays[] = {
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"
};


const char *month_name[] = {
  "Month of Winter",        /* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};


const int sharp[] = {
  0,
  0,
  0,
  1,                /* Slashing */
  0,
  0,
  0,
  0,                /* Bludgeon */
  0,
  0,
  0,
  0
};              /* Pierce   */




const struct tat_data tat[] = {
        { 0, 10000, "no tattoo", "no tattoo" },
        { 1, 30666, "grow stronger and hit harder", "damroll+2 str+2" },
        { 2,  3000, "increase your dexterity", "dex+1" },
        { 3, 18000, "summon a flaming skull to aid against thy enemies.",
                    "summon skull" },
        { 4, 14000, "the nimbleness and stamina of the tiger", "dex+1 mv+10" },
        { 5, 25000, "hit with the fierceness of the remorhaz", "dam+2" },
        { 6, 18000, "see that which is normally unseen", "greater percept" },
        { 7, 20000, "miss less and hit harder", "hit and dam+1" },
        { 8, 10000, "move like the wind", "moves+20" },
        { 9, 17000, "live longer through trust in your heart", "hp+20" },
        { 10,17000, "gain the magic of the stars", "mana+20" },
        { 11,11000, "gain the ability of movement over water",
                    "change density" },
        { 12,14000, "the nimblness of a spider", "dex+3" },
        { 13,19000, "the power of fighting a holy war", "dam+1" },
        { 14,15000, "the wisdom of your elders", "wis+3" },
        { 15,18000, "call upon the blessings of an angel", "bless" },
        { 16, 3000, "gain the intelligence of the fox", "int+1" },
        { 17, 3000, "gain the wisdom of the owl", "wis+1" }
};


field_object_data_t field_objs[] = {
 {50, FO_DAMAGE,
  "In one last gout of flame, the wall of fire burns itself out.",     0},
 {51, FO_SOLID,
  "A wall of ice rapidly melts into a large puddle.",                  20},
 {52, FO_AFFECT,
  "A noxious cloud of poison gas is whisked away by a strong gust of wind.",
                                                                       0}
};
/* increment NUM_FOS in structs.h if this changes */

