/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Ussage: header file: constants and fn prototypes for spell system       *
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

/* $Id: spells.h 1338 2008-03-21 17:58:08Z jravn $ */

#ifndef _SPELLS_H
#define _SPELLS_H

#define DEFAULT_STAFF_LVL	12
#define DEFAULT_WAND_LVL	12

#define blue_portal 4001
#define red_portal  4002
#define COC_VNUM 64

#define CAST_UNDEFINED	-1
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4
#define CAST_BREATH     5

#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)


#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_METEOR_SWARM           41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD	     45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD	     46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HOLY_SHIELD            47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL	     48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL	     49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION	     50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK		     51 /* Reserved Skill[] DO NOT CHANGE */
/* begin new spells */
#define SPELL_MASS_HEAL              52
#define SPELL_FLY		     53
#define SPELL_LYCANTHROPY            54
#define SPELL_VAMPIRISM              55
#define SPELL_SOBRIETY               56
#define SPELL_GROUP_INVIS            57
#define SPELL_HELLFIRE               58
#define SPELL_ENCHANT_ARMOR  	     59
#define SPELL_IDENTIFY		     60
#define SPELL_MINDPOKE               61
#define SPELL_MINDBLAST              62
#define SPELL_CHAMELEON              63
#define SPELL_LEVITATE               64
#define SPELL_METALSKIN              65
#define SPELL_INVULNERABILITY        66
#define SPELL_VITALITY               67
#define SPELL_INVIGORATE             68
#define SPELL_LESSPERCEPT	     69
#define SPELL_GREATPERCEPT	     70
#define SPELL_MINDATTACK	     71
#define SPELL_ADRENALINE	     72
#define SPELL_PSYSHIELD   	     73
#define SPELL_CHANGE_DENSITY	     74
#define SPELL_ACID_BLAST             75
#define SPELL_DOMINATE               76
#define SPELL_CELL_ADJUSTMENT        77
#define SPELL_ZEN                    78
#define SPELL_MIRROR_IMAGE           79
#define SPELL_MASS_DOMINATE          80
#define SPELL_DIVINE_INT             81
#define SPELL_MIND_BAR               82
#define SPELL_SOUL_LEECH             83
#define SPELL_MINDSIGHT              84
#define SPELL_TRANSPARENCY           85
#define SPELL_KNOW_ALIGN             86
#define SPELL_GATE                   87
#define SPELL_INTELLECT		     88
#define SPELL_LAY_HANDS        	     89
#define SPELL_MENTAL_LAPSE           90
#define SPELL_SMOKESCREEN            91
#define SPELL_DISRUPT                92
#define SPELL_DISINTEGRATE           93
#define SPELL_CALLIOPE		     94
#define SPELL_PROT_FROM_GOOD         95
#define SPELL_FLAMESTRIKE	     96
#define SPELL_HASTE                  97
#define SPELL_SLOW                   98
#define SPELL_DREAM_TRAVEL	     99
#define SPELL_PSIBLAST		    100
#define SPELL_COC		    101	
#define SPELL_WATER_BREATHE	    102
#define SPELL_DROWNING              103
#define SPELL_PETRIFY               104
#define SPELL_CONJURE_ELEMENTAL     105

/* change olc.h to this number +1 ---^^ */
/* don't forget constants.c wear off messages */
/* Insert new spells here, up to MAX_SPELLS */
#define MAX_SPELLS		    130

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB              131 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  132 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  133 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  134 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             135 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PUNCH                 136 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                137 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 138 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 139 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK		    140 /* Reserved Skill[] DO NOT CHANGE */
/* begin new skills */
#define SKILL_HEADBUTT		    141
#define SKILL_BEARHUG		    142
#define SKILL_CUTTHROAT		    143
#define SKILL_TRIP		    144
#define SKILL_SMACKHEADS	    145
#define SKILL_SLUG	 	    146
#define SKILL_CHARGE	 	    147
#define SKILL_SHOOT		    148
#define SKILL_RETREAT		    149
#define SKILL_BITE		    150
#define SKILL_PEEK                  151
#define SKILL_SUBDUE                152
#define SKILL_STEALTH               153
#define SKILL_KABUKI                154
#define SKILL_STRIKE                155
#define SKILL_SERPENT_KICK          156
#define SKILL_ESCAPE                157
#define SKILL_KK_RIN                158
#define SKILL_KK_KYO                159
#define SKILL_KK_TOH                160
#define SKILL_KK_KAI                161
#define SKILL_KK_JIN                162
#define SKILL_KK_RETSU              163
#define SKILL_KK_ZAI                164
#define SKILL_KK_ZHEN               165
#define SKILL_KK_SHA                166
#define SKILL_APPRAISE              167
#define SKILL_FLESH_ALTER           168
#define SKILL_COMPARE               169
#define SKILL_PALM                  170
#define SKILL_BERSERK               171
#define SKILL_PARRY                 172
#define SKILL_CIRCLE                173
#define SKILL_GROINRIP		    174
#define SKILL_SHARPEN               175
#define SKILL_SCROUNGE		    176
#define SKILL_DISARM		    177
#define SKILL_MINDLINK              178
#define SKILL_FIRST_AID             179
#define SKILL_DETECT		    180
#define SKILL_SHADOW		    181
#define SKILL_SWORDPLAY		    182
#define SKILL_KNIFEPLAY             183
#define SKILL_DISEMBOWEL            184
#define SKILL_TURN		    185
#define SKILL_EVASION		    186
#define SKILL_SLEEPER		    187
#define SKILL_DRAGON_KICK           188
#define SKILL_TIGER_PUNCH	    189
#define SKILL_NECKBREAK             190
#define SKILL_AMBUSH                191
#define SKILL_SCOUT		    192
	
/* end new skills   */
/* New skills may be added here up to MAX_SKILLS (200) */


/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects or non-players (such as NPC-only spells).
 */

#define SPELL_UNUSED                 201
#define SPELL_FIRE_BREATH            202
#define SPELL_GAS_BREATH             203
#define SPELL_FROST_BREATH           204
#define SPELL_ACID_BREATH            205
#define SPELL_LIGHTNING_BREATH       206

#define TOP_SPELL_DEFINE	     299
/* NEW NPC/OBJECT SPELLS can be inserted here up to 299 */


/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     300
#define TYPE_STING                   301
#define TYPE_WHIP                    302
#define TYPE_SLASH                   303
#define TYPE_BITE                    304
#define TYPE_BLUDGEON                305
#define TYPE_CRUSH                   306
#define TYPE_POUND                   307
#define TYPE_CLAW                    308
#define TYPE_MAUL                    309
#define TYPE_THRASH                  310
#define TYPE_PIERCE                  311
#define TYPE_BLAST		     312
#define TYPE_PUNCH		     313
#define TYPE_STAB		     314

/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING		     399



#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type {
   byte min_position;	/* Position for caster	 */
   int mana_min;	/* Min amount of mana used by a spell (highest lev) */
   int mana_max;	/* Max amount of mana used by a spell (lowest lev) */
   int mana_change;	/* Change in mana used by spell from lev to lev */

   int min_level[NUM_CLASSES];
   int routines;
   byte violent;
   int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
   char	*singular;
   char	*plural;
};


#define ASPELL(spellname) \
void	spellname(int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj, \
                  char *argument)

#define MANUAL_SPELL(spellname)	spellname(level, caster, cvict, ovict, \
arg);

ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_lycanthropy);
ASPELL(spell_vampirism);
ASPELL(spell_intoxify);
ASPELL(spell_sobriety);
ASPELL(spell_hellfire);
ASPELL(spell_enchant_armor);
ASPELL(spell_zen);
ASPELL(spell_silken_missile);
ASPELL(spell_mindsight);
ASPELL(spell_gate);
ASPELL(spell_mental_lapse);
ASPELL(spell_calliope);
ASPELL(spell_control_weather);
ASPELL(spell_meteor_swarm);
ASPELL(spell_coc);
ASPELL(spell_conjure_elemental);
ASPELL(spell_mirror_image);
ASPELL(spell_divine_int);

/* basic magic calling functions */

int find_skill_num(char *name);

void mag_damage(int level, struct char_data *ch, struct char_data *victim,
		int spellnum, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
		 int spellnum, int savetype);

void mag_group_switch(int level, struct char_data *ch, struct char_data *tch, 
		      int spellnum, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, int savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, int savetype);

void mag_areas(int level, struct char_data *ch, int spellnum, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
		 int spellnum, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
		int spellnum, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
		   int spellnum, int type);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
		    int spellnum, int type);

void mag_creations(int level, struct char_data *ch, int spellnum);

int call_magic(struct char_data *caster, struct char_data *cvict,
	       struct obj_data *ovict, int spellnum, int level, int casttype);

void mag_objectmagic(struct char_data *ch, struct obj_data *obj,
		     char *argument);

int cast_spell(struct char_data *ch, struct char_data *tch,
	       struct obj_data *tobj, int spellnum);


/* other prototypes */
void spell_level(int spell, int class, int level);
void init_spell_levels(void);
char *skill_name(int num);

#endif /* _SPELLS_H */
