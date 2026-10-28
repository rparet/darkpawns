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

/* $Id: constants.c 1258 2005-12-10 02:19:14Z mcooper $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "spells.h"
#include "shop.h"

const char *circlemud_version[] = {
"Dark Pawns version 3.0 Release: Fireslayer\r\n"
};


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
        "Kir Drax'in",
	"Kir-Oshi",
	"Kir-Morthis",
        "Xixieqi",
        "Mist Keep",
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

const struct skill_tree_data skill_tree[] = {
  {"!reserved!", 0, 0, 0 },   /* 0 */
    /* Rejuvenation */
  {"cure light",    GP_REJUVENATION, 1, SPELL_CURE_LIGHT },
  {"remove curse",  GP_REJUVENATION, 1, SPELL_REMOVE_CURSE },
  {"remove poison", GP_REJUVENATION, 1, SPELL_REMOVE_POISON },
  {"sobriety",      GP_REJUVENATION, 1, SPELL_SOBRIETY },
  {"heal",          GP_REJUVENATION, 2, SPELL_HEAL },
  {"cure blind",    GP_REJUVENATION, 2, SPELL_CURE_BLIND },
  {"mass heal",     GP_REJUVENATION, 3, SPELL_MASS_HEAL },
  {"invigorate",    GP_REJUVENATION, 3, SPELL_INVIGORATE },
  {"vitality",      GP_REJUVENATION, 4, SPELL_VITALITY },
  {"group heal",    GP_REJUVENATION, 4, SPELL_GROUP_HEAL }, /* 10 */
    /* Detection */
  {"detect alignment",    GP_DETECTION, 1, SPELL_DETECT_ALIGN },
  {"detect magic"    ,    GP_DETECTION, 1, SPELL_DETECT_MAGIC },
  {"infravision"     ,    GP_DETECTION, 1, SPELL_INFRAVISION },
  {"detect invisibility", GP_DETECTION, 2, SPELL_DETECT_INVIS },
  {"sense life"      ,    GP_DETECTION, 2, SPELL_SENSE_LIFE},
  {"detect race"     ,    GP_DETECTION, 2, SPELL_DETECT_RACE},
  {"detect poison"   ,    GP_DETECTION, 3, SPELL_DETECT_POISON },
  {"mindsight"       ,    GP_DETECTION, 4, SPELL_MINDSIGHT },
    /* Protection */
  {"bless",                       GP_PROTECTION, 1, SPELL_BLESS },
  {"protection from evil",        GP_PROTECTION, 2, SPELL_PROT_FROM_EVIL }, /* 20 */
  {"protection from good",        GP_PROTECTION, 2, SPELL_PROT_FROM_GOOD },
  {"sanctuary",                   GP_PROTECTION, 3, SPELL_SANCTUARY },
  {"metalskin",                   GP_PROTECTION, 3, SPELL_METALSKIN },
  {"holy shield",                 GP_PROTECTION, 4, SPELL_HOLY_SHIELD },
  {"invulnerability",             GP_PROTECTION, 4, SPELL_INVULNERABILITY },
  {"group protection from evil",  GP_PROTECTION, 4, SPELL_GROUP_PROT_FROM_EVIL },
  {"group protection from good",  GP_PROTECTION, 4, SPELL_GROUP_PROT_FROM_GOOD },
    /* Psychic */
  {"mind poke",     GP_PSYCHIC, 1, SPELL_MINDPOKE },
  {"mind attack",   GP_PSYCHIC, 1, SPELL_MINDATTACK },
  {"mind blast",    GP_PSYCHIC, 2, SPELL_MINDBLAST },
  {"turn",          GP_PSYCHIC, 2, SKILL_TURN },
  {"flesh alter",   GP_PSYCHIC, 3, SKILL_FLESH_ALTER }, /* 30 */
  {"mindlink",     GP_PSYCHIC, 3, SKILL_MINDLINK },
  {"mind bar",      GP_PSYCHIC, 4, SPELL_MIND_BAR },
  {"mental lapse",  GP_PSYCHIC, 4, SPELL_MENTAL_LAPSE },
  {"soul trap",     GP_PSYCHIC, 4, SPELL_SOULTRAP },
    /* Malediction */
  {"poison",            GP_MALEDICTION, 1, SPELL_POISON },
  {"curse",             GP_MALEDICTION, 1, SPELL_CURSE },
  {"burning hands",     GP_MALEDICTION, 1, SPELL_BURNING_HANDS },
  {"magic missile",     GP_MALEDICTION, 2, SPELL_MAGIC_MISSILE },
  {"color spray",       GP_MALEDICTION, 2, SPELL_COLOR_SPRAY }, /* 40 */
  {"entrapment",        GP_MALEDICTION, 2, SPELL_ENTRAP },
  {"blindness",         GP_MALEDICTION, 3, SPELL_BLINDNESS },
  {"harm",              GP_MALEDICTION, 3, SPELL_HARM },
  {"fireball",          GP_MALEDICTION, 3, SPELL_FIREBALL },
  {"energy drain",      GP_MALEDICTION, 4, SPELL_ENERGY_DRAIN },
  {"acid blast",        GP_MALEDICTION, 4, SPELL_ACID_BLAST },
  {"ray of disruption", GP_MALEDICTION, 4, SPELL_DISRUPT },
  {"disintegration",    GP_MALEDICTION, 4, SPELL_DISINTEGRATE },
    /* Necromancy */
  {"dispel good",         GP_NECROMANCY, 1, SPELL_DISPEL_GOOD },
  {"dispel evil",         GP_NECROMANCY, 1, SPELL_DISPEL_EVIL },
  {"animate dead",        GP_NECROMANCY, 1, SPELL_ANIMATE_DEAD }, /* 50 */
  {"clone",               GP_NECROMANCY, 2, SPELL_CLONE },
  {"dominate",            GP_NECROMANCY, 2, SPELL_DOMINATE },
  {"mass dominate",       GP_NECROMANCY, 3, SPELL_MASS_DOMINATE },
  {"syphon energy",       GP_NECROMANCY, 3, SPELL_SYPHON },
  {"conjure elemental",   GP_NECROMANCY, 4, SPELL_CONJURE_ELEMENTAL },
  {"divine intervention", GP_NECROMANCY, 4, SPELL_DIVINE_INT },
    /* Astral */
  {"word of recall",  GP_ASTRAL, 1, SPELL_WORD_OF_RECALL },
  {"gate",            GP_ASTRAL, 2, SPELL_GATE }, /* 60 */
  {"group recall",    GP_ASTRAL, 3, SPELL_GROUP_RECALL },
  {"dream travel",    GP_ASTRAL, 4, SPELL_DREAM_TRAVEL },
    /* Environmental */
  {"call lightning",      GP_ENVIRONMENT, 1, SPELL_CALL_LIGHTNING },
  {"invisibility",        GP_ENVIRONMENT, 1, SPELL_INVISIBLE },
  {"waterwalk",           GP_ENVIRONMENT, 1, SPELL_WATERWALK },
  {"waterbreathe",        GP_ENVIRONMENT, 1, SPELL_WATER_BREATHE },
  {"fly",                 GP_ENVIRONMENT, 2, SPELL_FLY },
  {"haste",               GP_ENVIRONMENT, 2, SPELL_HASTE },
  {"slow",                GP_ENVIRONMENT, 2, SPELL_SLOW },
  {"control weather",     GP_ENVIRONMENT, 3, SPELL_CONTROL_WEATHER }, /* 70 */
  {"group invisibility",  GP_ENVIRONMENT, 3, SPELL_GROUP_INVIS },
  {"earthquake",          GP_ENVIRONMENT, 3, SPELL_EARTHQUAKE },
  {"meteor swarm",        GP_ENVIRONMENT, 4, SPELL_METEOR_SWARM },
  {"hellfire",            GP_ENVIRONMENT, 4, SPELL_HELLFIRE },
  {"flamestrike",         GP_ENVIRONMENT, 4, SPELL_FLAMESTRIKE },
  {"smokescreen",         GP_ENVIRONMENT, 4, SPELL_SMOKESCREEN },
    /* Utility */
  { "create food",        GP_UTILITY, 1, SPELL_CREATE_FOOD },
  { "create water",       GP_UTILITY, 1, SPELL_CREATE_WATER },
  { "sleep",              GP_UTILITY, 2, SPELL_SLEEP },
  { "strength",           GP_UTILITY, 3, SPELL_STRENGTH },
  { "word of intellect",  GP_UTILITY, 3, SPELL_INTELLECT },
  { "enchant weapon",     GP_UTILITY, 4, SPELL_ENCHANT_WEAPON },
  { "enchant armor",      GP_UTILITY, 4, SPELL_ENCHANT_ARMOR },
    
    /* Stealth */
  { "pick lock",  GP_STEALTH, 1, SKILL_PICK_LOCK },
  { "peek",       GP_STEALTH, 1, SKILL_PEEK },
  { "palm",       GP_STEALTH, 1, SKILL_PALM },
  { "shadow",     GP_STEALTH, 2, SKILL_SHADOW },
  { "hide",       GP_STEALTH, 2, SKILL_HIDE },
  { "retreat",    GP_STEALTH, 2, SKILL_RETREAT },
  { "circle",     GP_STEALTH, 3, SKILL_CIRCLE },
  { "backstab",   GP_STEALTH, 3, SKILL_BACKSTAB },
  { "subdue",     GP_STEALTH, 3, SKILL_SUBDUE },
  { "sneak",      GP_STEALTH, 4, SKILL_SNEAK },
  { "steal",      GP_STEALTH, 4, SKILL_STEAL },
    /* Field Craft */
  { "scrounge",  GP_FIELD, 1, SKILL_SCROUNGE },
  { "scout",     GP_FIELD, 1, SKILL_SCOUT },
  { "track",     GP_FIELD, 1, SKILL_TRACK },
  { "alchemy",   GP_FIELD, 1, SKILL_ALCHEMY },
  { "skin",      GP_FIELD, 1, SKILL_SKIN },
  { "search",    GP_FIELD, 2, SKILL_DETECT },
  { "first aid", GP_FIELD, 2, SKILL_FIRST_AID },
  { "refuge",    GP_FIELD, 2, SKILL_REFUGE },
  { "rescue",    GP_FIELD, 3, SKILL_RESCUE },
  { "ambush",    GP_FIELD, 3, SKILL_AMBUSH },
  { "evasion",   GP_FIELD, 4, SKILL_EVASION },
  { "scribe",    GP_FIELD, 4, SKILL_SCRIBE },
    /* Hand-to-Hand */
  { "kick",       GP_HAND, 1, SKILL_KICK },
  { "bash",       GP_HAND, 1, SKILL_BASH },
  { "headbutt",   GP_HAND, 1, SKILL_HEADBUTT },
  { "bearhug",    GP_HAND, 2, SKILL_BEARHUG },
  { "trip",       GP_HAND, 2, SKILL_TRIP },
  { "slug",       GP_HAND, 2, SKILL_SLUG },
  { "smackheads", GP_HAND, 3, SKILL_SMACKHEADS },
  { "neckbreak",  GP_HAND, 4, SKILL_NECKBREAK },
  { "groinrip",   GP_HAND, 4, SKILL_GROINRIP },
  { "sleeper",    GP_HAND, 4, SKILL_SLEEPER },
    /* Weaponry */
  { "appraise",   GP_WEAPONRY, 1, SKILL_APPRAISE },
  { "compare",    GP_WEAPONRY, 1, SKILL_COMPARE },
  { "parry",      GP_WEAPONRY, 2, SKILL_PARRY },
  { "cutthroat",  GP_WEAPONRY, 2, SKILL_CUTTHROAT },
  { "charge",     GP_WEAPONRY, 3, SKILL_CHARGE },
  { "berserk",    GP_WEAPONRY, 3, SKILL_BERSERK },
  { "disembowel", GP_WEAPONRY, 4, SKILL_DISEMBOWEL },
  { "disarm",     GP_WEAPONRY, 4, SKILL_DISARM },

  {"\n", 0, 0, 0 }   /* do not remove */
};

const char *skill_groups[] =
{
  "!RESERVED!",
  "Rejuvenation",
  "Detection",
  "Protection",
  "Psychic",
  "Malediction",
  "Necromancy",
  "Astral",
  "Environmental",
  "Utility",
  "Stealth",
  "Field Craft",
  "Hand to Hand",
  "Weaponry",
  "\n"
};


/* HP, MANA and MOVE gain modifiers, based on the skill/spell groups known */
const sh_int group_mod[MAX_SKILL_GROUPS + 1][3] = {
  { 0,  0,  0 },
  { 1, -1,  1 },
  { 0,  1,  0 },
  { 0,  1,  0 },
  { 0,  1, -1 },
  { 0,  1,  0 },
  { 0,  1,  0 },
  { 0,  1,  1 },
  { 0,  1,  1 },
  { 1,  1,  0 },
  { 0, -1,  1 },
  { 0, -1,  1 },
  { 1, -1,  0 },
  { 1,  0,  0 }
};


/* const struct skill_tree_data skill_tree[] = {
  {"!reserved!", 0, 0, 1.0},  /  0  /
  {"combat", 0, 0, 1.0},      
   {"offensive", SK_COMBAT, 0, 1.0}, /  2  /
    {"bash",       SK_OFFENSIVE, 0, 1.0},
    {"kick",       SK_OFFENSIVE, 0, 1.0},
    {"punch",      SK_OFFENSIVE, 0, 1.0},
    {"headbutt",   SK_OFFENSIVE, 0, 1.0},
    {"bearhug",    SK_OFFENSIVE, 0, 1.0},
    {"trip",       SK_OFFENSIVE, 0, 1.0},
    {"smackheads", SK_OFFENSIVE, 0, 1.0},
    {"slug",       SK_OFFENSIVE, 0, 1.0},
    {"charge",     SK_OFFENSIVE, 0, 1.0},
    {"bite",       SK_OFFENSIVE, 0, 1.0},
    {"berserk",    SK_OFFENSIVE, 0, 1.0},
    {"sleeper",    SK_OFFENSIVE, 0, 1.0},
   {"defensive", SK_COMBAT, 0, 1.0},   / 15  /
    {"parry",      SK_DEFENSIVE, 0, 1.0},
    {"disarm",     SK_DEFENSIVE, 0, 1.0},
    {"rescue",     SK_DEFENSIVE, 0, 1.0},
    {"retreat",    SK_DEFENSIVE, 0, 1.0},
  {"magic", 0, 0, 1.0},               /  20  /
   {"summoning"  , SK_MAGIC, 0, 1.0},
   {"thaumaturgy", SK_MAGIC, 0, 1.0},
  {"rakish", 0, 0, 1.0},              /  23  /
   {"backstab", SK_RAKISH, 0, 1.0},
   {"steal"   , SK_RAKISH, 0, 1.0},
  {"\n", 0, 0, 1.0}  /  do not remove  /
}; */

const struct npc_race_data npc_races[] = {
  {"Kir", 0, 0, MOB_ISNPC, 0},					/* 0 */
  {"Xixian", 0, 0, MOB_ISNPC, 0},
  {"Antali", 0, 0, MOB_ISNPC, 0},
  {"Kenderkin", 0, 0, MOB_ISNPC|MOB_SCAVENGER, 0},
  {"Rakshasan", 8, AFF_SENSE_LIFE|AFF_SNEAK, MOB_ISNPC, -1},
  {"Ssauran", 8, 0, MOB_ISNPC, -1},                                       /* 5 */ 
  {"Amphibian", 10, AFF_WATERWALK|AFF_NOTRACK|AFF_WATERBREATHE, MOB_ISNPC|MOB_STAY_TERR|MOB_WIMPY, -1},
  {"Annelid", 10, AFF_SENSE_LIFE, MOB_ISNPC|MOB_WIMPY|MOB_NOBLIND|MOB_NOBASH, -1},
  {"Arachnid", 4, AFF_INFRAVISION|AFF_SENSE_LIFE|AFF_SNEAK|AFF_HIDE, MOB_ISNPC|MOB_NOBASH, -2},
  {"Avian", 8, AFF_NOTRACK|AFF_FLY, MOB_ISNPC|MOB_WIMPY|MOB_NOBASH, -1},		
  {"Bat", 8, AFF_SENSE_LIFE|AFF_INFRAVISION|AFF_NOTRACK|AFF_FLY, MOB_ISNPC|MOB_NOBASH, -2}, /* 10 */
  {"Canine", 4, 0, MOB_ISNPC, 0},
  {"Demi-god", 12, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_WATERWALK, MOB_ISNPC|MOB_AWARE|MOB_NOCHARM|MOB_NOSLEEP, -4},
  {"Demon", 10, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_WATERWALK, 
                MOB_ISNPC|MOB_AWARE|MOB_NOCHARM|MOB_NOSLEEP, -4},
  {"Dragon", 3, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_FLY, 
                MOB_ISNPC|MOB_NOCHARM|MOB_NOSLEEP, -5}, 				
  {"Elemental", 12, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_WATERWALK|AFF_NOTRACK,  /* 15 */
                    MOB_ISNPC|MOB_AWARE|MOB_NOCHARM|MOB_NOSLEEP|MOB_NOBASH|MOB_NOBLIND, -3},
  {"Faery", 0, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_WATERWALK|AFF_NOTRACK|AFF_SNEAK, 
               MOB_ISNPC|MOB_SCAVENGER|MOB_NOCHARM|MOB_NOSLEEP, 0},
  {"Feline", 8, AFF_SENSE_LIFE|AFF_SNEAK, MOB_ISNPC, -1},
  {"Fish", 10, AFF_WATERBREATHE|AFF_NOTRACK, MOB_ISNPC|MOB_STAY_TERR|MOB_NOBASH, 0},
  {"Giant", 0, 0, MOB_ISNPC, 3},							
  {"Goblin", 0, AFF_SENSE_LIFE, MOB_ISNPC|MOB_SCAVENGER, 1},                        /* 20 */
  {"Humanoid", 0, 0, MOB_ISNPC|MOB_SCAVENGER, 1},
  {"Hybrid", 12, 0, MOB_ISNPC, 1},
  {"Insect", 1, 0, MOB_ISNPC|MOB_WIMPY|MOB_NOBASH, -5},
  {"Lycanthrope", 4, AFF_DETECT_INVIS|AFF_SENSE_LIFE, MOB_ISNPC|MOB_NOCHARM|MOB_NOSLEEP, 1},
  {"Magical Construct", 12, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_WATERWALK|AFF_NOTRACK,  /* 25 */
                        MOB_ISNPC|MOB_AWARE|MOB_NOCHARM|MOB_NOSLEEP|MOB_NOBASH|MOB_NOBLIND, 0},
  {"Mammal", 4, 0, MOB_ISNPC, 0},
  {"Ogre", 0, 0, MOB_ISNPC|MOB_SCAVENGER, 3},
  {"Orc", 0, AFF_SENSE_LIFE, MOB_ISNPC|MOB_SCAVENGER, 1},
  {"Reptile", 4, 0, MOB_ISNPC|MOB_NOBASH, -1},					
  {"Rodent", 4, AFF_INFRAVISION|AFF_SNEAK|AFF_HIDE, MOB_ISNPC|MOB_SCAVENGER|MOB_NOBASH, -1}, /* 30 */
  {"Troll", 0, 0, MOB_ISNPC|MOB_SCAVENGER, 1},
  {"Undead", 0, AFF_DETECT_INVIS|AFF_SENSE_LIFE, MOB_ISNPC|MOB_NOCHARM|MOB_NOSLEEP|MOB_NOBLIND, 3},
  {"Ungulate", 9, 0, MOB_ISNPC|MOB_MOUNTABLE, 1},
  {"Ursine", 9, 0, MOB_ISNPC, 0},							
  {"Vampire", 4, AFF_DETECT_INVIS|AFF_SENSE_LIFE|AFF_WATERWALK|AFF_NOTRACK, /* 35 */
                 MOB_ISNPC|MOB_NOCHARM|MOB_NOSLEEP, 0},
  {"Vegetable", 10, AFF_SENSE_LIFE,
               MOB_ISNPC|MOB_SENTINEL|MOB_AWARE|MOB_NOCHARM|MOB_NOSLEEP|MOB_NOBASH|MOB_NOBLIND, 5},
  {"Ghost", 12, AFF_SENSE_LIFE|AFF_FLY|AFF_SNEAK|AFF_NOTRACK|AFF_INFRAVISION|AFF_DETECT_INVIS|
               AFF_INVISIBLE, MOB_ISNPC|MOB_AWARE|MOB_NOSLEEP|MOB_NOBASH|MOB_NOBLIND, -3},
};

/* Tattoo Types */
const char *tattoos[] =
{
  "None",
  "of a Tribal Symbol",
  "of a Scorpion poised to strike",
  "of Jehduti, the Moon God",
  "of Aethen, the Sun God",
  "of a Dragon rising from the flames",
  "of a Skull resting on a bed of thorns",
  "of a Wild Rose in full blossom",
  "\n"
};

/* Dye Types */
const char *dyes[] =
{
  "Red",
  "Black",
  "Yellow",
  "Blue",
  "White",
  "Green",
  "Orange",
  "Purple"
};

/* Dye Matrix */
/* Array is designed as [tattoo][color] */
const int dye_affects[NUM_TATTOOS][NUM_DYES * 2] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, APPLY_DEX, 1, 0, 0, 0, 0, 0, 0, APPLY_STR, 1, APPLY_DEX, -1, APPLY_MOVE, -100 },
  { APPLY_AC, -10, 0, 0, APPLY_STR, 1, APPLY_DEX, -1, 0, 0, APPLY_AC, 20, 0, 0, APPLY_MOVE, -100 },
  { 0, 0, APPLY_CHA, 1, APPLY_STR, -1, APPLY_DEX, -2, APPLY_CON, 2, APPLY_AC, -10, APPLY_MOVE_REGEN, -10, APPLY_SAVING_SPELL, -10 },
  { APPLY_STR, 1, APPLY_HIT, -100, APPLY_HIT_REGEN, 5, APPLY_HIT, -100, 0, 0, APPLY_STR, -1, APPLY_MANA, 20, APPLY_CON, -2 },
  { APPLY_HIT_REGEN, 2, 0, 0, APPLY_HIT, 100, APPLY_HITROLL, 1, 0, 0, 0, 0, 0, 0, APPLY_DAMROLL, -2},
  { 0, 0, 0, 0, APPLY_WIS, -1, APPLY_INT, -1, APPLY_CHA, 2, 0, 0, 0, 0, APPLY_STR, -1 },
  { APPLY_CHA, 1, 0, 0, 0, 0, APPLY_SAVING_BREATH, -3, 0, 0, APPLY_MANA, 100, APPLY_CHA, -2, APPLY_SAVING_PETRI, 5 }
};

const char *race_menu =
"\r\n"
"Choose a race:\r\n"
"  [&cK&n]ir        [&cA&n]ntali       [&cX&n]ixian\r\n"
"  [&c?&n] Help on races in general\r\n";

const int intelligent_races[] = {
  RACE_KIR		,
  RACE_XIXIAN		,
  RACE_ANTALI		,
  RACE_KENDER   	,
  RACE_RAKSHASA	   	,
  RACE_SSAUR		,
  RACE_DEMIGOD		,
  RACE_DEMON		,
  RACE_DRAGON		,
  RACE_ELEMENTAL	,
  RACE_FAERY		,
  RACE_GIANT		,
  RACE_GOBLIN		,
  RACE_HUMANOID		,
  RACE_LYCANTHROPE	,
  RACE_MAMMAL		,
  RACE_OGRE		,
  RACE_ORC		,
  RACE_TROLL		,
  RACE_UNDEAD		,
  RACE_VAMPIRE          ,
  RACE_GHOST
};
/* if you change anything above, change NUM_INTEL_RACES in structs.h */


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
  "*",				/* BFS MARK */
  "NEUTRAL",
  "BFR",
  "REGENROOM",
  "NO_WHO_ROOM",
  "**",				/* secret mark */
  "FLOW_NORTH",
  "FLOW_SOUTH",
  "FLOW_EAST",
  "FLOW_WEST",
  "FLOW_UP",
  "FLOW_DOWN",
  "ARENA",
  "BIND",
  "SHP_CLOSED",
  "!UNLOAD"
};

const char *zflag_bits[] = {
  "BFR",
  "TELEPORT",
  "KIR DRAX'IN",
  "KIR MORTHIS",
  "KIR OSHI",
  "UNLOAD",
  "LAW",
  "\n"
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
  "Road",
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
  "EXTRACT",
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
  "MXP",
  "MSP",
  "DEBUG",
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
  "!DISPLAY",
  "!SLEEP",
  "!BASH",
  "!BLIND",
  "HUNTER",
  "AGGR75",
  "RNDLD_ZONE",
  "MOUNTABLE",
  "RARE",
  "LOOTS",
  "OKGIVE",
  "STAY-TERR",
  "UNIQUE",
  "MVD",
  "CSH",
  "UNLOAD",
  "WARN1",
  "WARN2",
  "EXTRACT",
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
  "<unused>",
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
  "BLIND", /* 0 */
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "WATERWALK",
  "SANCT",
  "GROUP",
  "CURSE", 
  "INFRA", /* 10 */
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "FLESH-ALTER",
  "DODGE",
  "SNEAK",
  "HIDE",  
  "BERSERK", /* 20 */
  "CHARM",
  "FOLLOW",
  "WIMPY",
  "KUJI-KIRI",
  "CUTTHROAT",
  "FLY",
  "WEREWOLF",
  "VAMPIRE",
  "MOUNTED", 
  "INVULN", /* 30 */
  "FLAMING",
  "NOTHING",
  "HASTE",
  "SLOW",
  "DREAM",
  "WATERBREATHE",
  "METALSKIN",
  "PARALYSE",
  "SOUL-TRAP",
  "DET_RACE",
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
  "paralyse",
  "soul trap",
  "detect race"
};

/* CON_x */
const char *connected_types[] = {
  "Playing",
  "Disconnecting",
  "Account name",
  "Confirm account",
  "Get password",
  "Get new PW",
  "Confirm new PW",
  "Reading MOTD",
  "Lobby Menu",
  "Account Menu",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete Char 1",
  "Self-Delete Char 2",
  "Self-Delete Account 1",
  "Self-Delete Account 2",
  "Asking name",
  "Confirm name",
  "Select sex",
  "Select race",
  "Asking color",
  "Select hometown",
  "Creation",
  "Creation q1",
  "Creation q2",
  "Creation q3",
  "Object edit",
  "Room edit",
  "Zone edit",
  "Mobile edit",
  "Shop edit",
  "Treasure edit",
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


/* WEAR_x - for mxp eq list */
const char *where_mxp[] = {
  "&lt;used as light&&gt;      ",
  "&lt;worn on finger&&gt;     ",
  "&lt;worn on finger&&gt;     ",
  "&lt;worn around neck&&gt;   ",
  "&lt;worn around neck&&gt;   ",
  "&lt;worn on body&&gt;       ",
  "&lt;worn on head&&gt;       ",
  "&lt;worn on legs&&gt;       ",
  "&lt;worn on feet&&gt;       ",
  "&lt;worn on hands&&gt;      ",
  "&lt;worn on arms&&gt;       ",
  "&lt;worn as shield&&gt;     ",
  "&lt;worn about body&&gt;    ", 
  "&lt;worn about waist&&gt;   ",  
  "&lt;worn around wrist&&gt;  ",
  "&lt;worn around wrist&&gt;  ",
  "&lt;wielded&&gt;            ",
  "&lt;held&&gt;               ",
  "&lt;held&&gt;               ",
  "&lt;worn about legs&&gt;    ",
  "&lt;worn on face&&gt;       ",
  "&lt;hovering near head&&gt; "
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
  "UNUSED",
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
  "SOUL GEM",
  "NOTICE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "BOOK",
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
  "UNUSED",
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
  "!PERSIST",
  "RNDLD_ZONE",
  "BURIED",
  "TMP_PERSIST",
  "!SELL",
  "NAMED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "BROKEN",
  "UNLOAD",
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
  "DODGEROLL",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHP",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "AC-APPLY",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "UNUSED",
  "HIT_REGEN",
  "MANA_REGEN",
  "MOVE_REGEN",
  "PERM_SPELL",
  "\n"
};

const char *req_types[] = {
  "NONE",
  "SWORD",
  "AXE",
  "STAFF",
  "SPEAR",
  "MACE",
  "DAGGER",
  "WHIP",
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


/* Descriptor bits */
const char *desc_bits[] = {
  "MSP",
  "MXP",
  "MCCP",
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


const char *weapon_names[] =
{
 "no weapon",
 "sword",
 "axe",
 "staff",
 "spear",
 "mace",
 "dagger",
 "whip",
 "\n"
};

const char *craft_level[] =
{
  "Flawless",
  "Nearly Flawless",
  "Exquisitely crafted",
  "Finely crafted",
  "Well crafted",
  "Decently crafted",
  "Shabbily crafted",
  "Poorly crafted",
  "Broken",
  "\n"
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[] = {
  {-5, 0.0, 0, 0},	/* str = 0 */
  {-5, 0.2, 3, 1},	/* str = 1 */
  {-3, 0.4, 3, 2},
  {-3, 0.6, 10, 3},
  {-2, 0.8, 25, 4},
  {-2, 1.0, 55, 5},	/* str = 5 */
  {-1, 1.0, 80, 6},
  {-1, 1.0, 90, 7},
  {0, 1.0, 100, 8},
  {0, 1.2, 100, 9},
  {0, 1.4, 115, 10},	/* str = 10 */
  {0, 1.6, 115, 11},
  {0, 1.8, 120, 12},
  {0, 2.0, 120, 13},
  {0, 2.0, 130, 14},
  {0, 2.0, 130, 15},	/* str = 15 */
  {0, 2.0, 145, 16},
  {1, 2.2, 150, 18},
  {1, 2.4, 155, 20},
  {1, 2.6, 160, 21},
  {1, 2.8, 170, 21},	/* str = 20 */
  {2, 3.0, 180, 22},
  {2, 3.3, 185, 22},
  {3, 3.6, 200, 23},
  {4, 4.0, 210, 24},
  {5, 4.3, 220, 25},	/* str = 25 */
  {6, 4.6, 230, 30},
  {6, 5.0, 240, 35},
  {6, 5.3, 250, 40},
  {7, 5.6, 255, 40},
  {7, 6.0, 260, 40},	/* str = 30 */
  {7, 6.0, 265, 40},
  {8, 6.0, 270, 40}
};

/* [dex] skill apply (thieves only) */
/* const struct dex_skill_type dex_app_skill[] = {
  {-99, -99, -90, -99, -60},	 dex = 0
  {-90, -90, -60, -90, -50},	 dex = 1 
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},	* dex = 5 
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},	* dex = 10 
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},		* dex = 15 
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},		* dex = 18 
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},		* dex = 20 
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}		* dex = 25 
}; */


/* [dex] apply (all) */
struct dex_app_type dex_app[] = {
  {-7, -7, 6},		/* dex = 0 */
  {-6, -6, 5},		/* dex = 1 */
  {-4, -4, 5},
  {-3, -3, 4},
  {-2, -2, 3},
  {-2, -1, 3},		/* dex = 5 */
  {-1, -1, 2},
  {-1, -1, 1},
  {-1, 0, 1},
  {0, 0, 1},
  {0, 0, 0},		/* dex = 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},		/* dex = 15 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},
  {1, 1, -2},
  {2, 2, -3},		/* dex = 20 */
  {2, 2, -4},
  {3, 3, -4},
  {3, 3, -4},
  {4, 4, -5},
  {4, 4, -5},		/* dex = 25 */
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6},
  {5, 5, -7},
  {6, 6, -7},	  /* dex = 30 */
  {6, 6, -7},
  {6, 7, -7}
};



/* [con] apply (all) */
struct con_app_type con_app[] = {
  {-4, 20},		/* con = 0 */
  {-3, 25},		/* con = 1 */
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},		/* con = 5 */
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},		/* con = 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},		/* con = 15 */
  {2, 95},
  {2, 97},
  {3, 99},		/* con = 18 */
  {3, 99},
  {4, 99},		/* con = 20 */
  {5, 99},
  {5, 99},
  {5, 99},
  {6, 99},
  {6, 99}		/* con = 25 */
};



/* [int] apply (all) */
struct int_app_type int_app[] = {
  {3},		/* int = 0 */
  {5},		/* int = 1 */
  {7},
  {8},
  {9},
  {10},		/* int = 5 */
  {11},
  {12},
  {13},
  {15},
  {17},		/* int = 10 */
  {19},
  {22},
  {25},
  {30},
  {35},		/* int = 15 */
  {40},
  {45},
  {50},		/* int = 18 */
  {53},
  {55},		/* int = 20 */
  {56},
  {57},
  {58},
  {59},
  {60}		/* int = 25 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[] = {
  {0},	/* wis = 0 */
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
  {5},	/* wis = 18 */
  {6},
  {6},  /* wis = 20 */
  {6},
  {6},
  {7},
  {7},
  {7}  /* wis = 25 */
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
  2,	/* Inside     */
  0,	/* City       */
  3,	/* Field      */
  4,	/* Forest     */
  5,	/* Hills      */
  7,	/* Mountains  */
  5,	/* Swimming   */
  6,	/* Unswimable */
  2,	/* Flying     */
  6,    /* Underwater */
  8,    /* Desert     */
  6,    /* Fire Plane */
  6,    /* Eart Plane */
  6,    /* Wind Plane */
  6,    /* Water Plane*/
  4,    /* Swamp      */
  2,	/* Road       */
};

/* Moves required for certain skills - combat and non-combat */
const int needed_moves[] = {
  SKILL_AMBUSH, 30,
  SKILL_BACKSTAB, 30,
  SKILL_BASH, 10,
  SKILL_BEARHUG, 20,
  SKILL_BERSERK, 30,
  SKILL_CHARGE, 30,
  SKILL_CIRCLE, 30,
  SKILL_CUTTHROAT, 20,
  SKILL_DISARM, 40,
  SKILL_DISEMBOWEL, 40,
  SKILL_DRAGON_KICK, 10,
  SKILL_GROINRIP, 40,
  SKILL_HEADBUTT, 10,
  SKILL_KICK, 10,
  SKILL_NECKBREAK, 100,
  SKILL_PARRY, 20,
  SKILL_RESCUE, 30,
  SKILL_RETREAT, 0,
  SKILL_SLUG, 20,
  SKILL_SMACKHEADS, 30,
  SKILL_SUBDUE, 30,
  SKILL_TIGER_PUNCH, 10,
  SKILL_TRIP, 20,
  SKILL_SLEEPER, 40,
  SKILL_REFUGE, 20,
  0, 0
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
  "Month of Winter",		/* 0 */
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


/* costing defaults for PC owned shops */
struct shop_cost_data shop_cost[] = {
  { 300000, 0.15, 6, 8 },    /* Kir Drax'in */
  { 200000, 0.12, 5, 4 },    /* Kir Oshi */
  { 200000, 0.12, 5, 5 },    /* Kir Morthis */
  { 120000, 0.08, 3, 4 },    /* Xixieqi */
  { 160000, 0.09, 4, 8 }     /* Mist Keep */
};

/* types of stores that can be purchased */
const char *shop_types[] = {
  "an Armory",
  "a Weaponry",
  "a Jeweler",
  "a Provisioner",
  "a General Store",
  "an Apothecary",
  "a Store for the Mystical Arts",
  "\n"
};

/* vnums of available shop keepers */
const long shop_keeps[] =
{
  8031,
  18250,
  21248,
  4829,
  5333
};

/* mob NDD and SDD rolls */
const struct mob_dice_struct mob_dice[] = {
  { 2, 3 },	/* 1 - 3 */
  { 3, 3 },	/* 4 - 9 */
  { 3, 4 },	/* 10 - 19 */
  { 3, 5 },	/* 20 - 29 */
  { 4, 5 },	/* 30 - 39 */
  { 5, 5 },	/* 40 - 49 */
  { 5, 6 },	/* 50 - 59 */
  { 6, 6 },	/* 60 - 69 */
  { 6, 7 },	/* 70 - 79 */
  { 7, 7 },	/* 80 - 83 */
  { 7, 8 },	/* 84 - 89 */
  { 8, 8 },	/* 90 - 93 */
  { 8, 9 },	/* 94 - 96 */
  { 9, 9 }	/* 97 - 100 */
};

