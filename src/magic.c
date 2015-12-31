/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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

/* $Id: magic.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"

/* external vars */
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern int mini_mud;
extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;

/* external functions */
void clearMemory(struct char_data * ch);
void act(char *str, int i, struct char_data *c, struct obj_data *o,
     void *vict_obj, int j);
bool damage(struct char_data *ch, struct char_data *victim, int damage,
        int weapontype);
struct char_data *create_mobile(struct char_data *ch, int mob_number, int level,
                                int hunting);
void weight_change_object(struct obj_data *obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int dice(int number, int size);
extern struct spell_info_type spell_info[];
int num_followers(struct char_data *ch);
void send_to_zone(char *messg, struct char_data *ch);
struct char_data *read_mobile(int, int);
void raw_kill(struct char_data * ch, int attacktype);
bool ok_to_damage(struct char_data *ch, struct char_data *victim, int is_magic);

/* internal functions */
bool mag_materials(struct char_data *ch, int item0, int item1, int item2,
           int extract, int verbose);


/*
 * Saving throws for:
 * MCTW
 *   PARA, ROD, PETRI, BREATH, SPELL
 *     Levels 0-40
 */

const byte saving_throws[NUM_CLASSES][5][41] = {

  {             /* Mages */
        {90, 70, 69, 68, 67, 66, 65, 63, 61, 60, 59,    /* 0 - 10 */
/* PARA */  57, 55, 54, 53, 53, 52, 51, 50, 48, 46,     /* 11 - 20 */
        45, 44, 42, 40, 38, 36, 34, 32, 30, 28,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 55, 53, 51, 49, 47, 45, 43, 41, 40, 39,    /* 0 - 10 */
/* ROD */   37, 35, 33, 31, 30, 29, 27, 25, 23, 21,     /* 11 - 20 */
        20, 19, 17, 15, 14, 13, 12, 11, 10, 9,      /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 65, 63, 61, 59, 57, 55, 53, 51, 50, 49,    /* 0 - 10 */
/* PETRI */ 47, 45, 43, 41, 40, 39, 37, 35, 33, 31,     /* 11 - 20 */
        30, 29, 27, 25, 23, 21, 19, 17, 15, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 60, 59,    /* 0 - 10 */
/* BREATH */    57, 55, 53, 51, 50, 49, 47, 45, 43, 41,     /* 11 - 20 */
        40, 39, 37, 35, 33, 31, 29, 27, 25, 23,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 58, 56, 54, 52, 50, 48, 46, 45, 44,    /* 0 - 10 */
/* SPELL */ 42, 40, 38, 36, 35, 34, 32, 30, 28, 26,     /* 11 - 20 */
        25, 24, 22, 20, 18, 16, 14, 12, 10, 8,      /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */
  },

  {             /* Clerics */
        {90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,    /* 0 - 10 */
/* PARA */  33, 31, 30, 29, 27, 26, 25, 24, 23, 22,     /* 11 - 20 */
        21, 20, 18, 15, 14, 12, 10, 9, 8, 7,        /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,    /* 0 - 10 */
/* ROD */   53, 51, 50, 49, 47, 46, 45, 44, 43, 42,     /* 11 - 20 */
        41, 40, 38, 35, 34, 32, 30, 29, 28, 27,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,    /* 0 - 10 */
/* PETRI */ 48, 46, 45, 44, 43, 41, 40, 39, 38, 37,     /* 11 - 20 */
        36, 35, 33, 31, 29, 27, 25, 24, 23, 22,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,    /* 0 - 10 */
/* BREATH */    63, 61, 60, 59, 57, 56, 55, 54, 53, 52,     /* 11 - 20 */
        51, 50, 48, 45, 44, 42, 40, 39, 38, 37,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,    /* 0 - 10 */
/* SPELL */ 58, 56, 55, 54, 53, 51, 50, 49, 48, 47,     /* 11 - 20 */
        46, 45, 43, 41, 39, 37, 35, 34, 33, 32,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}       /* 31 - 40 */
  },

  {             /* Thieves */
        {90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,    /* 0 - 10 */
/* PARA */  55, 54, 53, 52, 51, 50, 49, 48, 47, 46,     /* 11 - 20 */
        45, 44, 43, 42, 41, 40, 39, 38, 37, 36,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,    /* 0 - 10 */
/* ROD */   50, 48, 46, 44, 42, 40, 38, 36, 34, 32,     /* 11 - 20 */
        30, 28, 26, 24, 22, 20, 18, 16, 14, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,    /* 0 - 10 */
/* PETRI */ 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,     /* 11 - 20 */
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,    /* 0 - 10 */
/* BREATH */    70, 69, 68, 67, 66, 65, 64, 63, 62, 61,     /* 11 - 20 */
        60, 59, 58, 57, 56, 55, 54, 53, 52, 51,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,    /* 0 - 10 */
/* SPELL */ 55, 53, 51, 49, 47, 45, 43, 41, 39, 37,     /* 11 - 20 */
        35, 33, 31, 29, 27, 25, 23, 21, 19, 17,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}           /* 31 - 40 */
  },

  {             /* Warriors */
        {90, 70, 68, 67, 65, 62, 58, 55, 53, 52, 50,    /* 0 - 10 */
/* PARA */  47, 43, 40, 38, 37, 35, 32, 28, 25, 24,     /* 11 - 20 */
        23, 22, 20, 19, 17, 16, 15, 14, 13, 12,     /* 21 - 30 */
        11, 10, 9, 8, 7, 6, 5, 4, 3, 2},        /* 31 - 40 */

        {90, 80, 78, 77, 75, 72, 68, 65, 63, 62, 60,    /* 0 - 10 */
/* ROD */   57, 53, 50, 48, 47, 45, 42, 38, 35, 34,     /* 11 - 20 */
        33, 32, 30, 29, 27, 26, 25, 24, 23, 22,     /* 21 - 30 */
        20, 18, 16, 14, 12, 10, 8, 6, 5, 4},        /* 31 - 40 */

        {90, 75, 73, 72, 70, 67, 63, 60, 58, 57, 55,    /* 0 - 10 */
/* PETRI */ 52, 48, 45, 43, 42, 40, 37, 33, 30, 29,     /* 11 - 20 */
        28, 26, 25, 24, 23, 21, 20, 19, 18, 17,     /* 21 - 30 */
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7},       /* 31 - 40 */

        {90, 85, 83, 82, 80, 75, 70, 65, 63, 62, 60,    /* 0 - 10 */
/* BREATH */    55, 50, 45, 43, 42, 40, 37, 33, 30, 29,     /* 11 - 20 */
        28, 26, 25, 24, 23, 21, 20, 19, 18, 17,     /* 21 - 30 */
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7},       /* 31 - 40 */

        {90, 85, 83, 82, 80, 77, 73, 70, 68, 67, 65,    /* 0 - 10 */
/* SPELL */ 62, 58, 55, 53, 52, 50, 47, 43, 40, 39,     /* 11 - 20 */
        38, 36, 35, 34, 33, 31, 30, 29, 28, 27,     /* 21 - 30 */
        25, 23, 21, 19, 17, 15, 13, 11, 9, 7}       /* 31 - 40 */
  },
  {              /* MAGUS */
        {90, 70, 69, 68, 67, 66, 65, 63, 61, 60, 59,    /* 0 - 10 */
/* PARA */  57, 55, 54, 53, 53, 52, 51, 50, 48, 46,     /* 11 - 20 */
        45, 44, 42, 40, 38, 36, 34, 32, 30, 28,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 55, 53, 51, 49, 47, 45, 43, 41, 40, 39,    /* 0 - 10 */
/* ROD */   37, 35, 33, 31, 30, 29, 27, 25, 23, 21,     /* 11 - 20 */
        20, 19, 17, 15, 14, 13, 12, 11, 10, 9,      /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 65, 63, 61, 59, 57, 55, 53, 51, 50, 49,    /* 0 - 10 */
/* PETRI */ 47, 45, 43, 41, 40, 39, 37, 35, 33, 31,     /* 11 - 20 */
        30, 29, 27, 25, 23, 21, 19, 17, 15, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 60, 59,    /* 0 - 10 */
/* BREATH */    57, 55, 53, 51, 50, 49, 47, 45, 43, 41,     /* 11 - 20 */
        40, 39, 37, 35, 33, 31, 29, 27, 25, 23,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 58, 56, 54, 52, 50, 48, 46, 45, 44,    /* 0 - 10 */
/* SPELL */ 42, 40, 38, 36, 35, 34, 32, 30, 28, 26,     /* 11 - 20 */
        25, 24, 22, 20, 18, 16, 14, 12, 10, 8,      /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */
  },

  {             /* Avatar */
        {90, 50, 59, 48, 46, 45, 43, 40, 37, 35, 34,    /* 0 - 10 */
/* PARA */  33, 31, 30, 29, 27, 26, 25, 24, 23, 22,     /* 11 - 20 */
        21, 20, 18, 15, 14, 12, 10, 9, 8, 7,        /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 69, 68, 66, 65, 63, 60, 57, 55, 54,    /* 0 - 10 */
/* ROD */   53, 51, 50, 49, 47, 46, 45, 44, 43, 42,     /* 11 - 20 */
        41, 40, 38, 35, 34, 32, 30, 29, 28, 27,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 65, 64, 63, 61, 60, 58, 55, 53, 50, 49,    /* 0 - 10 */
/* PETRI */ 48, 46, 45, 44, 43, 41, 40, 39, 38, 37,     /* 11 - 20 */
        36, 35, 33, 31, 29, 27, 25, 24, 23, 22,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 76, 75, 73, 70, 67, 65, 64,    /* 0 - 10 */
/* BREATH */    63, 61, 60, 59, 57, 56, 55, 54, 53, 52,     /* 11 - 20 */
        51, 50, 48, 45, 44, 42, 40, 39, 38, 37,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 74, 73, 71, 70, 68, 65, 63, 60, 59,    /* 0 - 10 */
/* SPELL */ 58, 56, 55, 54, 53, 51, 50, 49, 48, 47,     /* 11 - 20 */
        46, 45, 43, 41, 39, 37, 35, 34, 33, 32,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}       /* 31 - 40 */
  },

  {             /* Assassin */
        {90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,    /* 0 - 10 */
/* PARA */  55, 54, 53, 52, 51, 50, 49, 48, 47, 46,     /* 11 - 20 */
        45, 44, 43, 42, 41, 40, 39, 38, 37, 36,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,    /* 0 - 10 */
/* ROD */   50, 48, 46, 44, 42, 40, 38, 36, 34, 32,     /* 11 - 20 */
        30, 28, 26, 24, 22, 20, 18, 16, 14, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,    /* 0 - 10 */
/* PETRI */ 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,     /* 11 - 20 */
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,    /* 0 - 10 */
/* BREATH */    70, 69, 68, 67, 66, 65, 64, 63, 62, 61,     /* 11 - 20 */
        60, 59, 58, 57, 56, 55, 54, 53, 52, 51,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,    /* 0 - 10 */
/* SPELL */ 55, 53, 51, 49, 47, 45, 43, 41, 39, 37,     /* 11 - 20 */
        35, 33, 31, 29, 27, 25, 23, 21, 19, 17,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}           /* 31 - 40 */
  },

  {             /* Paladin */
        {90, 70, 68, 67, 65, 62, 58, 55, 53, 52, 50,    /* 0 - 10 */
/* PARA */  47, 43, 40, 38, 37, 35, 32, 28, 25, 24,     /* 11 - 20 */
        23, 22, 20, 19, 17, 16, 15, 14, 13, 12,     /* 21 - 30 */
        11, 10, 9, 8, 7, 6, 5, 4, 3, 2},        /* 31 - 40 */

        {90, 80, 78, 77, 75, 72, 68, 65, 63, 62, 60,    /* 0 - 10 */
/* ROD */   57, 53, 50, 48, 47, 45, 42, 38, 35, 34,     /* 11 - 20 */
        33, 32, 30, 29, 27, 26, 25, 24, 23, 22,     /* 21 - 30 */
        20, 18, 16, 14, 12, 10, 8, 6, 5, 4},        /* 31 - 40 */

        {90, 75, 73, 72, 70, 67, 63, 60, 58, 57, 55,    /* 0 - 10 */
/* PETRI */ 52, 48, 45, 43, 42, 40, 37, 33, 30, 29,     /* 11 - 20 */
        28, 26, 25, 24, 23, 21, 20, 19, 18, 17,     /* 21 - 30 */
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7},       /* 31 - 40 */

        {90, 85, 83, 82, 80, 75, 70, 65, 63, 62, 60,    /* 0 - 10 */
/* BREATH */    55, 50, 45, 43, 42, 40, 37, 33, 30, 29,     /* 11 - 20 */
        28, 26, 25, 24, 23, 21, 20, 19, 18, 17,     /* 21 - 30 */
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7},       /* 31 - 40 */

        {90, 85, 83, 82, 80, 77, 73, 70, 68, 67, 65,    /* 0 - 10 */
/* SPELL */ 62, 58, 55, 53, 52, 50, 47, 43, 40, 39,     /* 11 - 20 */
        38, 36, 35, 34, 33, 31, 30, 29, 28, 27,     /* 21 - 30 */
        25, 23, 21, 19, 17, 15, 13, 11, 9, 7}       /* 31 - 40 */
  },
  {             /* Ninja */
        {90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,    /* 0 - 10 */
/* PARA */  55, 54, 53, 52, 51, 50, 49, 48, 47, 46,     /* 11 - 20 */
        45, 44, 43, 42, 41, 40, 39, 38, 37, 36,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,    /* 0 - 10 */
/* ROD */   50, 48, 46, 44, 42, 40, 38, 36, 34, 32,     /* 11 - 20 */
        30, 28, 26, 24, 22, 20, 18, 16, 14, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,    /* 0 - 10 */
/* PETRI */ 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,     /* 11 - 20 */
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,    /* 0 - 10 */
/* BREATH */    70, 69, 68, 67, 66, 65, 64, 63, 62, 61,     /* 11 - 20 */
        60, 59, 58, 57, 56, 55, 54, 53, 52, 51,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,    /* 0 - 10 */
/* SPELL */ 55, 53, 51, 49, 47, 45, 43, 41, 39, 37,     /* 11 - 20 */
        35, 33, 31, 29, 27, 25, 23, 21, 19, 17,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}           /* 31 - 40 */
  },
  {             /* Psionic */
        {90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,    /* 0 - 10 */
/* PARA */  55, 54, 53, 52, 51, 50, 49, 48, 47, 46,     /* 11 - 20 */
        45, 44, 43, 42, 41, 40, 39, 38, 37, 36,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,    /* 0 - 10 */
/* ROD */   50, 48, 46, 44, 42, 40, 38, 36, 34, 32,     /* 11 - 20 */
        30, 28, 26, 24, 22, 20, 18, 16, 14, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,    /* 0 - 10 */
/* PETRI */ 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,     /* 11 - 20 */
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,    /* 0 - 10 */
/* BREATH */    70, 69, 68, 67, 66, 65, 64, 63, 62, 61,     /* 11 - 20 */
        60, 59, 58, 57, 56, 55, 54, 53, 52, 51,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,    /* 0 - 10 */
/* SPELL */ 55, 53, 51, 49, 47, 45, 43, 41, 39, 37,     /* 11 - 20 */
        35, 33, 31, 29, 27, 25, 23, 21, 19, 17,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}           /* 31 - 40 */
  },
  {             /* Ranger */
        {90, 70, 68, 67, 65, 62, 58, 55, 53, 52, 50,    /* 0 - 10 */
/* PARA */  47, 43, 40, 38, 37, 35, 32, 28, 25, 24,     /* 11 - 20 */
        23, 22, 20, 19, 17, 16, 15, 14, 13, 12,     /* 21 - 30 */
        11, 10, 9, 8, 7, 6, 5, 4, 3, 2},        /* 31 - 40 */

        {90, 80, 78, 77, 75, 72, 68, 65, 63, 62, 60,    /* 0 - 10 */
/* ROD */   57, 53, 50, 48, 47, 45, 42, 38, 35, 34,     /* 11 - 20 */
        33, 32, 30, 29, 27, 26, 25, 24, 23, 22,     /* 21 - 30 */
        20, 18, 16, 14, 12, 10, 8, 6, 5, 4},        /* 31 - 40 */

        {90, 75, 73, 72, 70, 67, 63, 60, 58, 57, 55,    /* 0 - 10 */
/* PETRI */ 52, 48, 45, 43, 42, 40, 37, 33, 30, 29,     /* 11 - 20 */
        28, 26, 25, 24, 23, 21, 20, 19, 18, 17,     /* 21 - 30 */
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7},       /* 31 - 40 */

        {90, 85, 83, 82, 80, 75, 70, 65, 63, 62, 60,    /* 0 - 10 */
/* BREATH */    55, 50, 45, 43, 42, 40, 37, 33, 30, 29,     /* 11 - 20 */
        28, 26, 25, 24, 23, 21, 20, 19, 18, 17,     /* 21 - 30 */
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7},       /* 31 - 40 */

        {90, 85, 83, 82, 80, 77, 73, 70, 68, 67, 65,    /* 0 - 10 */
/* SPELL */ 62, 58, 55, 53, 52, 50, 47, 43, 40, 39,     /* 11 - 20 */
        38, 36, 35, 34, 33, 31, 30, 29, 28, 27,     /* 21 - 30 */
        25, 23, 21, 19, 17, 15, 13, 11, 9, 7}       /* 31 - 40 */
  },
{               /* Mystic */
        {90, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,    /* 0 - 10 */
/* PARA */  55, 54, 53, 52, 51, 50, 49, 48, 47, 46,     /* 11 - 20 */
        45, 44, 43, 42, 41, 40, 39, 38, 37, 36,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 70, 68, 66, 64, 62, 60, 58, 56, 54, 52,    /* 0 - 10 */
/* ROD */   50, 48, 46, 44, 42, 40, 38, 36, 34, 32,     /* 11 - 20 */
        30, 28, 26, 24, 22, 20, 18, 16, 14, 13,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 60, 59, 58, 58, 56, 55, 54, 53, 52, 51,    /* 0 - 10 */
/* PETRI */ 50, 49, 48, 47, 46, 45, 44, 43, 42, 41,     /* 11 - 20 */
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71,    /* 0 - 10 */
/* BREATH */    70, 69, 68, 67, 66, 65, 64, 63, 62, 61,     /* 11 - 20 */
        60, 59, 58, 57, 56, 55, 54, 53, 52, 51,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0},          /* 31 - 40 */

        {90, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57,    /* 0 - 10 */
/* SPELL */ 55, 53, 51, 49, 47, 45, 43, 41, 39, 37,     /* 11 - 20 */
        35, 33, 31, 29, 27, 25, 23, 21, 19, 17,     /* 21 - 30 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0}           /* 31 - 40 */
  }

};


int
mag_savingthrow(struct char_data * ch, int type)
{
  int save;

  /* negative apply_saving_throw values make saving throws better! */

  if (IS_NPC(ch)) /* NPCs use warrior tables according to some book */
    save = saving_throws[CLASS_WARRIOR][type][(int) GET_LEVEL(ch)];
  else
    save = saving_throws[(int) GET_CLASS(ch)][type][(int) GET_LEVEL(ch)];

  save += GET_SAVE(ch, type);

  /* throwing a 0 is always a failure */
  if (MAX(1, save) < number(0, 99))
    return TRUE;
  else
    return FALSE;
}


/* affect_update: called from comm.c (causes spells to wear off) */
void
affect_update(void)
{
  static struct master_affected_type *af, *next;
  static struct char_data *i;
  extern char *spell_wear_off_msg[];

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next)
      {
    next = af->next;
    if (af->duration >= 1)
      af->duration--;
    else if (af->duration == -1)    /* No action */
      af->duration = -1;    /* GODs only! unlimited */
    else {
      if ((af->type > 0) && (af->type <= MAX_SPELLS))
        if (!af->next || (af->next->type != af->type) ||
        (af->next->duration > 0))
          if (*spell_wear_off_msg[af->type])
        {
          send_to_char(spell_wear_off_msg[af->type], i);
          send_to_char("\r\n", i);
        }
      affect_remove(i, af);
    }
      }
}


#define r_spellnum 0
#define r_item0 1
#define r_item1 2
#define r_item2 3
#define r_parts 4
const int reagents[][r_parts] = {
  { SPELL_SLEEP   ,     1226, 0, 0 },  /* 0 */
  { SPELL_FIREBALL,     1225, 0, 0 },  /* 1 */
  { SPELL_DISINTEGRATE, 1227, 0, 0 },  /* 2 */
  { SPELL_ENERGY_DRAIN, 1230, 0, 0 },  /* 3 */
  { SPELL_CURSE,        1233, 0, 0 },  /* 4 */
  { SPELL_BLINDNESS,    1234, 0, 0 },  /* 5 */
  { SPELL_CHARM,        1231, 0, 0 },  /* 6 */ /*TBD*/
  { SPELL_METALSKIN,    1229, 0, 0 },  /* 7 */
  { SPELL_WATERWALK,    1228, 0, 0 },  /* 8 */
  { SPELL_COLOR_SPRAY,  1236, 0, 0 },  /* 9 */
  { SPELL_MAGIC_MISSILE,1239, 0, 0 }   /* 10 */
};
#define r_reagents 11

bool
has_reagents(struct char_data *ch, int spellnum)
{
  int i;
  for (i = 0; i < r_reagents; i++)
    if(reagents[i][r_spellnum] == spellnum)
      return(mag_materials(ch,
               reagents[i][r_item0],
               reagents[i][r_item1],
               reagents[i][r_item2],
               TRUE,
               FALSE));
  return(FALSE);
}


/* =========================================================================
   NAME       : get_reagent_names()
   DESCRIPTION:
   RETURNS    : ALLOCATED string of reagent names or NULL
   WARNINGS   :
   HISTORY    : Created by dkarnes 970624
   OTHER      :
   ========================================================================= */
char *
get_reagent_names(int spellnum)
{
  char *return_string = NULL;
    char buf[MAX_STRING_LENGTH];
  int i, j;

  for (i = 0; i < r_reagents; i++)
    if(reagents[i][r_spellnum] == spellnum)
      for(j = r_item0; j <=r_item2; j++)
          if(reagents[i][j])
          {
            struct obj_data *obj = read_object(reagents[i][j], VIRTUAL);
            if (obj)
            {
                char *tmp = NULL;
                if (return_string)
                  tmp = return_string;
                sprintf(buf, "%s%s%s", tmp?tmp:"", tmp?", ":"",
                         (obj)->short_description);
                extract_obj(obj);
                        return_string = str_dup(buf);
            }
          }
  return(return_string);
}

/* =========================================================================
   NAME       : mag_materials()
   DESCRIPTION: check for and optionally extract reagents
   RETURNS    : TRUE if the reagents are there
   WARNINGS   : item0-2 should be a reagent vnum, or 0
   HISTORY    : Documented by dkarnes 970624
   OTHER      :
   ========================================================================= */
bool
mag_materials(struct char_data *ch, int item0, int item1, int item2,
          int extract, int verbose)
{
   struct obj_data *tobj = NULL;
   struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

   for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
   {
      if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0))
      {
     obj0 = tobj;
     item0 = -1;
      }
      else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1))
      {
     obj1 = tobj;
     item1 = -1;
      }
      else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2))
      {
     obj2 = tobj;
     item2 = -1;
      }
   }

   if ((item0 > 0) || (item1 > 0) || (item2 > 0))
      return (FALSE);

   if (extract)
   {
      if (item0 < 0)
      {
     obj_from_char(obj0);
     extract_obj(obj0);
      }
      if (item1 < 0)
      {
     obj_from_char(obj1);
     extract_obj(obj1);
      }
      if (item2 < 0)
      {
     obj_from_char(obj2);
     extract_obj(obj2);
      }
   }
   if (verbose)
   {
      send_to_char("A puff of smoke rises from your pack.\r\n", ch);
      act("A puff of smoke rises from $n's pack.",
      TRUE, ch, NULL, NULL, TO_ROOM);
   }
   return (TRUE);
}

/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 */

void
mag_damage(int level, struct char_data * ch, struct char_data * victim,
       int spellnum, int savetype)
{
   int is_mage = 0, is_cleric = 0, dam = 0, reag = 0;

   if (victim == NULL || ch == NULL)
      return;

   is_mage = (GET_CLASS(ch) == CLASS_MAGIC_USER ||
          GET_CLASS(ch) == CLASS_MAGUS);
   is_cleric = (GET_CLASS(ch) == CLASS_CLERIC ||
        GET_CLASS(ch) == CLASS_AVATAR);

   switch (spellnum)
   {
      /* Mostly mages */
   case SPELL_MAGIC_MISSILE:
      if (is_mage)
      {
     reag = has_reagents(ch, SPELL_MAGIC_MISSILE)?level:0;
     if(reag)
       {
         stc("Pulling a shard of obsidian from a pocket, you crush "
        "it under your heel...\r\n", ch);
         act("$n pulls something out of $s pocket and crushes it beneath"
         " $s heel...", TRUE, ch, NULL, NULL, TO_ROOM);
       } else
            stc("You attempt the spell without the components..\r\n", ch);
     dam = dice(4, 3) + reag + level;
      }
      else
     dam = dice(4, 3) + level;
      break;
   case SPELL_CHILL_TOUCH:  /* chill touch also has an affect */
      if (is_mage)
     dam = dice(5, 3) + level;
      else
     dam = dice(5, 3) + level;
      break;
   case SPELL_BURNING_HANDS:
       dam = dice(4, 5) + level;
      break;
   case SPELL_SHOCKING_GRASP:
      if (is_mage)
     dam = dice(4, 7) + level;
      else
     dam = dice(4, 7) + level;
      break;
   case SPELL_LIGHTNING_BOLT:
      if (is_mage)
     dam = dice(9, 4) + level;
      else
     dam = dice(9, 4) + level;
      break;
   case SPELL_COLOR_SPRAY:
      if (is_mage)
      {
     reag = has_reagents(ch, SPELL_COLOR_SPRAY)?level:0;
     if(reag)
       {
         stc("Pulling a prism from a pocket, you crush it under your heel"
         "...\r\n", ch);
         act("$n pulls something out of $s pocket and crushes it beneath"
         " $s heel...", TRUE, ch, NULL, NULL, TO_ROOM);
       }
     dam = dice(9, 7) + reag + level;
      }
      else
     dam = dice(9, 7) + level;
      break;
   case SPELL_FIREBALL:
     if (is_mage)
       {
     reag = has_reagents(ch, SPELL_FIREBALL)?level:0;
     if(reag)
       {
         stc("Pulling a pinch of ash from a pocket, you cast it about the"
         " room...\r\n", ch);
         act("$n pulls a pinch of ash out of a pocket and casts it about "
         "the room.", TRUE, ch, NULL, NULL, TO_ROOM);
       }
     dam = dice(12, 8) + 20 + level + level + reag;
       }
     else
       dam = dice(12, 8) + level*2;
     break;
   case SPELL_DISINTEGRATE:
      if (is_mage)
      {
         reag = has_reagents(ch, SPELL_DISINTEGRATE)?level:0;
         if(reag)
           {
             stc("Pulling the eye of a beholder from a pocket, you throw it to"
                 " the ground...\r\n", ch);
             act("$n pulls a small orb out of a pocket and dashes it to "
                 "the ground.", TRUE, ch, NULL, NULL, TO_ROOM);
           }
     dam = dice(18, 8) + 3*level+reag;
      }
      else
     dam = dice(18, 8) + level;
      if (!number(0,50) && !IS_NPC(ch))
      {
     stc("Your magick backfires!\r\n", ch);
     victim = ch;
      }
      break;
   case SPELL_DISRUPT:
      if (is_mage)
     dam = dice(20, 7) + 3*level;
      else
     dam = dice(20, 7) + level;

      if (!number(0,50) && !IS_NPC(ch))
      {
     stc("Your magick backfires!\r\n", ch);
     victim = ch;
      }
      break;

      /* Mostly clerics */
   case SPELL_DISPEL_EVIL:
      dam = dice(9, 5) + level+5+level/2;
      if (IS_EVIL(ch))
      {
     if (!IS_NPC(ch))
     {
        victim = ch;
        dam = GET_HIT(ch) - 10;
     }
      }
      else if (IS_GOOD(victim))
      {
     act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
     dam = 0;
     return;
      }
      break;
   case SPELL_DISPEL_GOOD:
      dam = dice(9, 5) + level+5;
      if (IS_GOOD(ch))
      {
     if (!IS_NPC(ch))
     {
        victim = ch;
        dam = GET_HIT(ch) - 10;
     }
      }
      else if (IS_EVIL(victim))
      {
     act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
     dam = 0;
     return;
      }
      break;

   case SPELL_CALL_LIGHTNING:
      dam = dice(10, 8) + level+5;
      break;

   case SPELL_HARM:
      dam = dice(12, 8) + level*2;
      break;

      /* Ninja, Mage */
   case SPELL_SOUL_LEECH:
   case SPELL_ENERGY_DRAIN:
      if (GET_LEVEL(victim) <= 2)
     dam = 100;
      else
     dam = dice(10, 6)+level;
      if (is_mage)
      {
         reag = has_reagents(ch, SPELL_ENERGY_DRAIN)?level:0;
         if(reag)
           {
             stc("Pulling the vampire dust from a pocket, you throw it into"
                 " the air...\r\n", ch);
             act("$n throws some dust into the air...",
        TRUE, ch, NULL, NULL, TO_ROOM);
         dam +=reag;
           }
      }
      break;
      /* Area spells */
   case SPELL_EARTHQUAKE:
      dam = dice(7, 7) + level;
      break;

   case SPELL_ACID_BLAST:
      dam = dice(4, 3) + level;
      break;

      /* PSI spells */
   case SPELL_MINDPOKE:
      dam = dice(3, 3) + level;
      break;

   case SPELL_MINDATTACK:
      dam = dice(4, 6) + level;
      break;

   case SPELL_MINDBLAST:
      dam = dice(9, 7) + level + (level/2);
      break;

   case SPELL_PSIBLAST:
      dam = dice(15, 13) + 3 * level;

      if (!number(0,30) && !IS_NPC(ch))
      {
         stc("Suddenly, your psionic power recoils!\r\n", ch);
         victim = ch;
      }
      break;


   } /* switch(spellnum) */


   /* divide damage by two if victim makes his saving throw */
   if (mag_savingthrow(victim, savetype))
      dam >>= 1;

   /* and finally, inflict the damage and check if the spell made noise*/
   if(damage(ch, victim, dam, spellnum))
   {
     switch(spellnum)
     {
     case SPELL_FIREBALL:
            if (ch && !number(0,10))
          send_to_zone("A blast of hot air washes over you.", ch);
        break;
     case SPELL_CALL_LIGHTNING:
     case SPELL_LIGHTNING_BOLT:
            if (ch && !number(0,10))
              send_to_zone("Thunder rumbles through the air.", ch);
        break;
         case SPELL_SOUL_LEECH:
            GET_HIT(ch) += (dam/3);
            if (GET_HIT(ch) > GET_MAX_HIT(ch))
              GET_HIT(ch) = GET_MAX_HIT(ch);
            break;
     default:
        break;
     }
   }
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

#define MAX_SPELL_AFFECTS 5 /* change if more needed */

void
mag_affects(int level, struct char_data * ch, struct char_data * victim,
        int spellnum, int savetype)
{
   struct master_affected_type af[MAX_SPELL_AFFECTS];
   int is_mage = FALSE, is_cleric = FALSE, i, reag = 0;
   bool accum_affect = FALSE, accum_duration = FALSE, aggro = FALSE;
   char *to_vict = NULL, *to_room = NULL, *to_self = NULL;

   if (victim == NULL || ch == NULL)
      return;

   is_mage = (IS_MAGIC_USER(ch) || IS_MAGUS(ch));
   is_cleric = (IS_CLERIC(ch) || IS_AVATAR(ch));

   for (i = 0; i < MAX_SPELL_AFFECTS; i++)
   {
      af[i].type = spellnum;
      af[i].bitvector = 0;
      af[i].modifier = 0;
      af[i].location = APPLY_NONE;
   }

   switch (spellnum)
   {

   case SPELL_CHILL_TOUCH:
      af[0].location = APPLY_STR;
      if (mag_savingthrow(victim, savetype))
     af[0].duration = 1;
      else
     af[0].duration = 4;
      af[0].modifier = -1;
      af[0].bitvector = AFF_NOTHING;
      accum_duration = TRUE;
      to_vict = "You feel your strength wither!";
      to_self = "Summoning the forces of magick, you press your icy hand"
     " against $M.";
      break;

   case SPELL_ARMOR:
      af[0].location = APPLY_AC;
      af[0].modifier = -15;
      af[0].duration = 24;
      af[0].bitvector = AFF_NOTHING;
      accum_duration = FALSE;
      to_vict = "You feel someone protecting you.";
      to_self = "The magick protects $M.";
      break;

   case SPELL_BLESS:
      af[0].location = APPLY_HITROLL;
      af[0].modifier = 2;
      af[0].duration = 6;
      af[0].bitvector = AFF_NOTHING;

      af[1].location = APPLY_SAVING_SPELL;
      af[1].modifier = -2;
      af[1].duration = 6;
      af[1].bitvector = AFF_NOTHING;

      accum_duration = TRUE;
      to_vict = "You feel righteous.";
      to_self = "You bestow the blessing of your gods on $M.";
      break;

   case SPELL_BLINDNESS:
   case SPELL_SMOKESCREEN:
      if (is_mage)
      {
         reag = has_reagents(ch, SPELL_BLINDNESS);
         if(reag)
           {
             stc("You crush a small lens under your heel.\r\n", ch);
             act("$n crushes a small lens under $s heel.",
        TRUE, ch, NULL, NULL, TO_ROOM);
           }
      }
      if (MOB_FLAGGED(victim,MOB_NOBLIND) || mag_savingthrow(victim, savetype))
      {
     if (spellnum == SPELL_BLINDNESS)
        stc("Your magic fades, then dies out totally.\r\n", ch);
     if (IS_NPC(victim))
            hit(victim, ch, TYPE_UNDEFINED);
     return;
      }

      af[0].location = APPLY_HITROLL;
      af[0].modifier = 0-(4+reag);
      af[0].duration = 2;
      af[0].bitvector = AFF_BLIND;

      af[1].location = APPLY_AC;
      af[1].modifier = 40;
      af[1].duration = 2+reag;
      af[1].bitvector = AFF_BLIND;

      if (spellnum == SPELL_BLINDNESS)
      {
        to_room = "$n seems to be blinded!";
        to_vict = "You have been blinded!";
        to_self = "A streak of blackness courses from your hand!";
      }
      break;

   case SPELL_CURSE:
      if (mag_savingthrow(victim, savetype))
      {
     send_to_char(NOEFFECT, ch);
     if (IS_NPC(victim))
            hit(victim, ch, TYPE_UNDEFINED);
     return;
      }

      if (is_mage)
      {
         reag = has_reagents(ch, SPELL_CURSE);
         if(reag)
           {
             stc("You throw a raven's feather into the air...\r\n", ch);
             act("$n throws a feather into the air...",
        TRUE, ch, NULL, NULL, TO_ROOM);
           }
      }
      af[0].location = APPLY_HITROLL;
      af[0].duration = 1 + (GET_LEVEL(ch) >> 1);
      af[0].modifier = 0-(3+reag);
      af[0].bitvector = AFF_CURSE;

      af[1].location = APPLY_DAMROLL;
      af[1].duration = 1 + (GET_LEVEL(ch) >> 1);
      af[1].modifier = 0-(3+reag);
      af[1].bitvector = AFF_CURSE;

      accum_duration = TRUE;
      accum_affect = TRUE;
      to_room = "$n briefly glows red!";
      to_vict = "You feel very uncomfortable.";
      to_self = "A streak of red light courses from your hand!";
      aggro=TRUE;
      break;

   case SPELL_KNOW_ALIGN:
   case SPELL_DETECT_ALIGN:
      af[0].duration = 12 + level;
      af[0].bitvector = AFF_DETECT_ALIGN;
      accum_duration = FALSE;
      if (spellnum == SPELL_DETECT_ALIGN)
     to_vict = "Your eyes tingle.";
      else
     to_vict="Like a physical blow, emotions of others wash over you.";
      break;

   case SPELL_DETECT_INVIS:
      af[0].duration = 12 + level;
      af[0].bitvector = AFF_DETECT_INVIS;
      accum_duration = TRUE;
      to_vict = "Your eyes tingle.";
      to_self = "A streak of yellow light courses from your "
     "hand, washing over $M!";
      break;

   case SPELL_DETECT_MAGIC:
      af[0].duration = 12 + level;
      af[0].bitvector = AFF_DETECT_MAGIC;
      accum_duration = TRUE;
      to_vict = "Your eyes tingle.";
      to_self = "A streak blue light courses from your fingertips, "
     "washing over $M!";
      break;

   case SPELL_INFRAVISION:
      af[0].duration = 12 + level;
      af[0].bitvector = AFF_INFRAVISION;
      accum_duration = TRUE;
      to_vict = "Your eyes glow red.";
      to_room = "$n's eyes glow red.";
      to_self = "With a light touch, you bestow the magick into $S eyes.";
      break;

   case SPELL_HASTE:
      af[0].duration = level;
      af[0].bitvector = AFF_HASTE;
      accum_duration = FALSE;
      to_self = "You feel your movement quicken!";
      break;

   case SPELL_SLOW:
      af[0].duration = level;
      af[0].bitvector = AFF_HASTE;
      accum_duration = FALSE;
      to_vict = "You feel the world speed up around you.";
      to_self = "You send the forces of time against $S!";
      break;

   case SPELL_DREAM_TRAVEL:
      af[0].duration = 6;
      af[0].bitvector = AFF_DREAM;
      accum_duration = FALSE;
      to_vict = "You feel the power of the Dream Lords surround you.";
      break;

   case SPELL_WATER_BREATHE:
      af[0].duration = GET_LEVEL(ch);
      af[0].bitvector = AFF_WATERBREATHE;
      accum_duration = TRUE;
      to_vict = "You feel your breath become colder.";
      break;

   case SPELL_TRANSPARENCY:
   case SPELL_INVISIBLE:
      if (!victim)
     victim = ch;

      af[0].duration = 12 + (GET_LEVEL(ch) >> 2);
      af[0].bitvector = AFF_INVISIBLE;
      accum_duration = FALSE;
      if (spellnum == SPELL_INVISIBLE)
     to_vict = "You vanish.";
      else
     to_vict = "Your skin turns transparent.";
      to_room = "$n slowly fades out of existence.";
      break;

   case SPELL_POISON:
      if (mag_savingthrow(victim, savetype))
      {
     send_to_char(NOEFFECT, ch);
     if (IS_NPC(victim))
            hit(victim, ch, TYPE_UNDEFINED);
     return;
      }

      af[0].location = APPLY_STR;
      af[0].duration = (level/2)-2;
      af[0].modifier = -2;
      af[0].bitvector = AFF_POISON;
      af[1].location = APPLY_HITROLL;
      af[1].duration = (level/2)-2;
      af[1].modifier = -2;
      af[1].bitvector = AFF_POISON;
      to_vict = "You feel very sick.";
      to_room = "$n gets violently ill!";
      to_self = "Your tainted magick pulses towards $M.";
      aggro = TRUE;
      break;

   case SPELL_FLAMESTRIKE:
      if (mag_savingthrow(victim, savetype))
       {
     send_to_char(NOEFFECT, ch);
         if (IS_NPC(victim))
            hit(victim, ch, TYPE_UNDEFINED);
         return;
       }

       if (!OUTSIDE(ch))
         {
           stc("You can only do this outdoors!\r\n", ch);
           return;
         }

       af[0].duration = (GET_LEVEL(ch)*.17);  /* sets a max of 5 hr for mortals */
       af[0].bitvector = AFF_FLAMING;
       to_vict = "A bolt of flame shoots down from the heavens and engulfs you!";
       to_room = "A bolt of flame shoots down from the heavens and engulfs $n!";
       to_self = "You call down a bolt of flame on $M!";
       aggro = TRUE;
       break;

   case SPELL_FLY:
   case SPELL_LEVITATE:
      af[0].duration = GET_LEVEL(ch);
      af[0].bitvector = AFF_FLY;
      accum_duration = TRUE;
      to_vict = "Your feet rise off the ground!";
      to_room = "$n's feet rise off the ground!";
      to_self = "Like a falling feather, your magick floats toward $M.";
      break;

   case SPELL_PROT_FROM_EVIL:
      if (IS_EVIL(victim))
      {
    stc("You cannot protect yourself from the Evil inside you!\r\n",ch);
        sprintf(buf, "%s killed by Protection from Evil.", GET_NAME(ch));
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        raw_kill(ch, TYPE_BLAST);
      }
      else
      {
     af[0].duration = 24;
     af[0].bitvector = AFF_PROTECT_EVIL;
     accum_duration = FALSE;
     to_vict = "A stream of silver light surges from your fingertips, "
        "covering you!";
     to_room = "A stream of silver light surges from $n's fingertips, "
        "covering $m!";
      }
      break;

   case SPELL_PROT_FROM_GOOD:
      if (IS_GOOD(victim))
      {
        stc("The forces of Light destroy you for your betrayal!\r\n",ch);
        sprintf(buf, "%s killed by Protection from Good.", GET_NAME(ch));
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        raw_kill(ch, TYPE_BLAST);
      }
      else
      {
         af[0].duration = 24;
         af[0].bitvector = AFF_PROTECT_GOOD;
         accum_duration = FALSE;
         to_vict = "A stream of silver light surges from your fingertips, "
            "covering you!";
         to_room = "A stream of silver light surges from $n's fingertips, "
            "covering $m!";
      }
      break;

   case SPELL_SANCTUARY:
      af[0].duration = 4;
      af[0].bitvector = AFF_SANCTUARY;

      accum_duration = TRUE;
      if (IS_EVIL(victim))
      {
     to_vict = "A black aura momentarily surrounds you.";
     to_room = "$n is surrounded by a black aura.";
      }
      else
      {
     to_vict = "A white aura momentarily surrounds you.";
     to_room = "$n is surrounded by a white aura.";
      }
      break;

   case SPELL_SLEEP:
     {
      reag = has_reagents(ch, SPELL_SLEEP);
      if(reag)
    {
      stc("Pulling a bit of sand from a pocket, you cast it about the"
          " room...\r\n", ch);
      act("$n pulls a bit of sand out of a pocket and casts it about "
          "the room.", TRUE, ch, NULL, NULL, TO_ROOM);
    }
      else
    stc("You attempt the spell without the components...\r\n", ch);

      if (!IS_NPC(victim) && !IS_OUTLAW(ch))
      {
        stc("Your spell fails to affect them because you are not an Outlaw!\r\n", ch);
        sprintf(buf, "%s tried to cast a spell on you but failed because %s is not an Outlaw!\r\n",
               GET_NAME(ch), GET_NAME(ch));
        stc(buf, victim);
        return;
      }

      if (!IS_NPC(victim) && GET_LEVEL(ch) < LVL_IMMORT)
        if ((GET_LEVEL(victim) > GET_LEVEL(ch) + 3) || (GET_LEVEL(victim) < GET_LEVEL(ch) - 3))
        {
          act("$n shakes his head wearily, but then snaps out of it!",
             TRUE, victim, 0, 0, TO_ROOM);
          return;
        }

      if (MOB_FLAGGED(victim, MOB_NOSLEEP)||mag_savingthrow(victim, savetype))
      {
     act("$n shakes his head wearily, but then snaps out of it!",
         TRUE, victim, 0, 0, TO_ROOM);
     if (IS_NPC(victim))
            hit(victim, ch, TYPE_UNDEFINED);
     return;
      }

      af[0].duration = 4 + (GET_LEVEL(ch) >> 2) + reag;
      af[0].bitvector = AFF_SLEEP;

      if (GET_POS(victim) > POS_SLEEPING)
      {
     act("You feel very sleepy...  Zzzz......",
         FALSE, victim, 0, 0, TO_CHAR);
     act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
     GET_POS(victim) = POS_SLEEPING;
      }
     }
   break;
   case SPELL_ADRENALINE:
   case SPELL_STRENGTH:
      af[0].location = APPLY_STR;
      af[0].duration = (GET_LEVEL(ch) >> 1) + 4;
      af[0].modifier = 1 + (level > 18)+
     (ch==victim && spellnum == SPELL_ADRENALINE)?1:0;
      af[0].bitvector = AFF_NOTHING;
      if (spellnum == SPELL_STRENGTH)
     accum_affect = TRUE;
      else
     accum_duration = TRUE;
      to_vict = "You feel stronger!";
      to_self = "Grabbing $M, you feel a strong flow of magick course "
     "between you.";
      break;

   case SPELL_SENSE_LIFE:
      to_vict = "Your feel your awareness improve.";
      af[0].duration = GET_LEVEL(ch);
      af[0].bitvector = AFF_SENSE_LIFE;
      accum_duration = TRUE;
      break;

   case SPELL_WATERWALK:
      reag = has_reagents(ch, SPELL_WATERWALK);
      if(reag)
          stc("A frog leg turns into dust as you mouth the words of "
        "the spell...\r\n", ch);
   case SPELL_CHANGE_DENSITY:
      af[0].duration = 4+reag?20:0;
      af[0].bitvector = AFF_WATERWALK;
      accum_duration = TRUE;
      if (spellnum == SPELL_CHANGE_DENSITY) {
     to_vict = "Your molecular density shifts.";
         to_self = "You shift $S molecular density.";
      }
      else {
     to_vict = "You feel webbing between your toes.";
         to_self = "Your magic makes $M light footed.";
      }
      break;

   case SPELL_CHAMELEON:
      af[0].duration = GET_LEVEL(ch);
      af[0].bitvector = AFF_HIDE;
      accum_duration = FALSE;
      to_vict = "You blend into the surroundings.";
      break;

   case SPELL_METALSKIN:
      reag = has_reagents(ch, SPELL_METALSKIN);
      if(reag)
    {
      stc("A small chunk of iron melts in your palm as you cast the "
        "spell...\r\n", ch);
      act("A small chunk of iron melts in $n's palm as $e cast a "
          "spell...", TRUE, ch, NULL, NULL, TO_ROOM);
    }
      af[0].duration = 5;
      af[0].location = APPLY_AC;
      af[0].modifier = -(15+(GET_LEVEL(ch)/2)+reag);
      to_vict = "Your skin turns metallic!";
      af[0].bitvector = AFF_METALSKIN;
      accum_duration = TRUE;
      break;

   case SPELL_INVULNERABILITY:
      af[0].duration = 7;
      af[0].bitvector = AFF_INVULN;
      af[0].location = APPLY_AC;
      af[0].modifier = -100;
      af[1].duration = 7;
      af[1].bitvector = AFF_INVULN;
      af[1].location = APPLY_SAVING_SPELL;
      af[1].modifier = -7;
      to_vict = "A globe of protection appears around you!";
      to_vict = "A globe of protection appears around you!";
      accum_duration = TRUE;
      break;

   case SPELL_PSYSHIELD:
      af[0].location = APPLY_AC;
      af[0].modifier = -15;
      af[0].duration = GET_LEVEL(ch)/2;
      af[0].bitvector = AFF_NOTHING;
      accum_duration = TRUE;
      to_vict = "You feel a shield of energy form around you.";
      break;

   case SPELL_GREATPERCEPT:
      af[1].duration = level/2 + 4;
      af[1].bitvector = AFF_DETECT_INVIS;
      af[0].duration = level/2 + 4;
      af[0].bitvector = AFF_SENSE_LIFE;
      af[0].location = APPLY_SPELL;
      accum_duration = FALSE;
      to_vict = "Your eyes glow briefly.";
      to_room = "$n's eyes glow briefly.";
      break;
   case SPELL_LESSPERCEPT:
      af[1].duration = level/2 + 4;
      af[1].bitvector = AFF_DETECT_ALIGN;
      af[0].duration = level/2 + 4;
      af[0].bitvector = AFF_INFRAVISION;
      af[0].location = APPLY_SPELL;
      accum_duration = FALSE;
      to_vict = "Your eyes glow briefly.";
      to_room = "$n's eyes glow briefly.";
      break;

   case SPELL_INTELLECT:
      af[0].location = APPLY_INT;
      af[0].modifier = 1;
      af[0].duration = 8;
      af[0].bitvector = AFF_NOTHING;
      accum_affect = TRUE;
      to_vict= "Your head clears and you realize some of the secrets of life!";
      break;

   case SPELL_MIND_BAR:
      af[0].location = APPLY_INT;
      af[0].modifier = -18;
      af[0].duration = (level/2)-2;
      af[0].bitvector = AFF_NOTHING;
      accum_duration = FALSE;
      accum_affect = FALSE;
      to_vict="Suddenly, your mind numbs and you feel somewhat impaired.";
      to_self="You place a mental bar across $S mind.";
      break;

   }

   /*
    * If this is a mob that has this affect set in its mob file, do not
    * perform the affect.  This prevents people from un-sancting mobs
    * by sancting them and waiting for it to fade, for example.
    */
   if (IS_NPC(victim) &&
       (IS_AFFECTED(victim, af[0].bitvector) ||
        IS_AFFECTED(victim, af[1].bitvector)) &&
       !affected_by_spell(victim, spellnum))
   {
      send_to_char(NOEFFECT, ch);
      return;
   }

   /*
    * If the victim is already affected by this spell, and the spell does
    * not have an accumulative effect, then fail the spell.
    */
   if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect))
   {
      send_to_char(NOEFFECT, ch);
      return;
   }

   af->by_type = BY_SPELL;
   af->obj_num = 0;


   for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      if (af[i].bitvector || (af[i].location != APPLY_NONE))
     affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);

   if (to_self != NULL && ch!=victim)
      act(to_self, TRUE, ch, 0, victim, TO_CHAR);
   if (to_vict != NULL)
      act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
   if (to_room != NULL)
      act(to_room, TRUE, victim, 0, ch, TO_ROOM);
   if (IS_NPC(victim) && aggro)
      hit(victim, ch, TYPE_UNDEFINED);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */

void
perform_mag_groups(int level, struct char_data * ch, struct char_data * tch,
           int spellnum, int savetype)
{
  switch (spellnum)
    {
    case SPELL_GROUP_HEAL:
      mag_points(level, ch, tch, SPELL_HEAL, savetype);
      break;
    case SPELL_HOLY_SHIELD:
      mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
      break;
    case SPELL_GROUP_RECALL:
      spell_recall(level, ch, tch, NULL, NULL);
      break;
    case SPELL_MASS_DOMINATE:
      spell_charm(level, ch, tch, NULL, NULL);
    case SPELL_GROUP_INVIS:
      mag_affects(level, ch, tch, SPELL_INVISIBLE, savetype);
    }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void
mag_groups(int level, struct char_data * ch, int spellnum, int savetype)
{
   struct char_data *k;
   struct follow_type *f;

   if (!ch)
      return;

   if (!IS_AFFECTED(ch, AFF_GROUP))
      return;

   if (!(k = ch->master))
      k = ch;

   /* do all followers of k who are not ch */
   for (f = k->followers; f; f = f->next)
   {
      if (f->follower == ch)  /* need to do ch last for group recall, etc. */
        continue;
      if (f->follower->in_room != ch->in_room)
         continue;
      if (!IS_AFFECTED(f->follower, AFF_GROUP))
         continue;
      perform_mag_groups(level, ch, f->follower, spellnum, savetype);
   }

   /* do k */
   if ((k->in_room == ch->in_room) && IS_AFFECTED(k, AFF_GROUP))
      perform_mag_groups(level, ch, k, spellnum, savetype);

   /* do yourself if you didn't already */
   if (k != ch)
     perform_mag_groups(level, ch, ch, spellnum, savetype);

}

/*
 * mass spells affect every creature in the room except the caster.
 * This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 * Updated to affect everyone in the room except the caster and those in
 * her group.  -rparet 20000614
 */
void
mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
   struct char_data *tch, *tch_next;
   char *to_char = NULL;
   char *to_room = NULL;

   switch (spellnum)
   {
     case SPELL_SMOKESCREEN:
       to_char = "As you quickly mumble the incantation a cloud of thick, acrid black smoke forms around you.";
       to_room = "Suddenly, a cloud of thick, acrid black smoke appears, obscuring your view of $n!";
       break;
   }

   if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
   if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);

   for (tch = world[ch->in_room].people; tch; tch = tch_next)
   {
      tch_next = tch->next_in_room;

      if (tch == ch)
     continue;
      if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
        continue;
      if (IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
        continue;
      if (are_grouped(ch, tch))
         continue;

      mag_affects(level, ch, tch, spellnum, savetype);
   }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void
mag_areas(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  char *to_char = NULL;
  char *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum)
    {
    case SPELL_EARTHQUAKE:
      to_char = "You gesture and the earth begins to shake all around you!";
      to_room = "$n gracefully gestures and the earth begins to "
    "shake violently!";
      break;
    case SPELL_ACID_BLAST:
      to_char = "A spray of acid flows from your fingertips!";
      to_room = "$n raises a hand and acid sprays from $s fingers!";
      break;
    case SPELL_HELLFIRE:
      /* this is just a dummy to stop bug exploiting until I fix it for real */
      return;
    }

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);


  for (tch = world[ch->in_room].people; tch; tch = next_tch)
    {
      next_tch = tch->next_in_room;

      /*
       * The skips: 1: the caster
       *            2: immortals
       *            3: if no pk on this mud, skips over all players
       *            4: pets (charmed NPCs)
       *        5: group members
       * players can only hit players in CRIMEOK rooms 4) players can only hit
       * charmed mobs in CRIMEOK rooms
       */

      if (tch == ch)
    continue;
      if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
    continue;
      if (IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
    continue;
      if (are_grouped(ch, tch))
    continue;

      if (spellnum == SPELL_MASS_DOMINATE)
        spell_charm(GET_LEVEL(ch),ch, tch, NULL, NULL);
      else
        mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
    }
}


/*
 *  Every spell which summons/gates/conjours a mob comes through here.
 *
 *  None of these spells are currently implemented in Circle 3.0; these
 *  were taken as examples from the JediMUD code.  Summons can be used
 *  for spells like clone, ariel servant, etc.
 */

static char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!\r\n",
  "$n animates a corpse!\r\n",
  "$N appears from a cloud of thick blue smoke!\r\n",
  "$N appears from a cloud of thick green smoke!\r\n",
  "$N appears from a cloud of thick red smoke!\r\n",
  "$N disappears in a thick black cloud!\r\n"
  "As $n makes a strange magical gesture, you feel a strong breeze.\r\n",
  "As $n makes a strange magical gesture, you feel a searing heat.\r\n",
  "As $n makes a strange magical gesture, you feel a sudden chill.\r\n",
  "As $n makes a strange magical gesture, you feel the dust swirl.\r\n",
  "$n magically divides!\r\n",
  "$n animates a corpse, which begins moving with a life of it's own!\r\n"
};

static char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Oh shit!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};

#define MOB_MONSUM_I        130
#define MOB_MONSUM_II       140
#define MOB_MONSUM_III      150
#define MOB_GATE_I      160
#define MOB_GATE_II     170
#define MOB_GATE_III        180
#define MOB_ELEMENTAL_BASE  110
#define MOB_CLONE       69
#define MOB_ZOMBIE      10
#define MOB_AERIALSERVANT   109


void
mag_summons(int level, struct char_data * ch, struct obj_data * obj,
              int spellnum, int savetype)
{
   void add_follower_quiet(struct char_data *ch, struct char_data *leader);

   struct char_data *mob = NULL;
   struct obj_data *tobj, *next_obj;
   int pfail = 0;
   int msg = 0, fmsg = 0;
   int num = 1;
   int i;
   int mob_num = 0;
   int handle_corpse = 0;

   if (ch == NULL)
      return;

   switch (spellnum)
   {
   case SPELL_ANIMATE_DEAD:
      if ((obj == NULL) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) ||
      (!GET_OBJ_VAL(obj, 3)))
      {
     act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
     return;
      }
      handle_corpse = 1;
      msg = 12;
      mob_num = MOB_ZOMBIE;
      pfail = 8;
      break;

   default:
      return;
   }

   if (IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You are too giddy to have any followers!\r\n", ch);
      return;
   }
   if (num_followers(ch)>=(GET_CHA(ch)/2))
   {
      stc("You can't have any more followers!\r\n", ch);
      return;
   }
   if (number(0, 101) < pfail)
   {
      send_to_char(mag_summon_fail_msgs[fmsg], ch);
      return;
   }
   for (i = 0; i < num; i++)
   {
      mob = create_mobile(ch, mob_num, GET_LEVEL(ch)/2, FALSE);
      SET_BIT_AR(AFF_FLAGS(mob), AFF_CHARM);
      add_follower_quiet(mob, ch);
      act(mag_summon_msgs[fmsg], FALSE, ch, 0, mob, TO_ROOM);
      if (spellnum == SPELL_CLONE)
      {
     strcpy(GET_NAME(mob), GET_NAME(ch));
     strcpy(mob->player.short_descr, GET_NAME(ch));
      }
      else if (spellnum == SPELL_ANIMATE_DEAD) {
         stc("The corpse starts to twitch, then stands with a"
         " life of it's own!\r\n", ch);
      }
   }
   if (handle_corpse)
   {
      for (tobj = obj->contains; tobj; tobj = next_obj)
      {
     next_obj = tobj->next_content;
     obj_from_obj(tobj);
     obj_to_char(tobj, mob);
      }
      extract_obj(obj);
   }
}


void
mag_points(int level, struct char_data * ch, struct char_data * victim,
             int spellnum, int savetype)
{
   int hit = 0;
   int move = 0;

   if (victim == NULL)
      return;

   switch (spellnum)
   {
   case SPELL_CURE_LIGHT:
      hit = dice(2, 8) + 1 + (level >> 2);
      send_to_char("You feel better.\r\n", victim);
      if (ch !=victim)
     act("$N's wounds glow and mend themselves.",
         TRUE, ch, 0, victim, TO_NOTVICT);
      break;
   case SPELL_CURE_CRITIC:
      hit = dice(5, 8) + 3 + (level >> 2);
      send_to_char("You feel a lot better!\r\n", victim);
      if (ch !=victim)
     act("$N's wounds glow and mend themselves.",
         TRUE, ch, 0, victim, TO_NOTVICT);
      break;
   case SPELL_HEAL:
   case SPELL_CELL_ADJUSTMENT:
      if (IS_PSIONIC(ch) || IS_MYSTIC(ch))
      {
     hit = 90 + dice(2, 8);
     send_to_char("You focus your mind on healing your body..\r\n",
              victim);
     send_to_char("Your wounds mend themselves!\r\n", victim);
      }
      else
      {
     hit = 100 + dice(3, 8);
     send_to_char("A warm feeling floods your body.\r\n", victim);
     if (ch !=victim)
        act("$N's wounds glow and mend themselves.",
        TRUE, ch, 0, victim, TO_NOTVICT);
      }
      if (affected_by_spell(victim, SKILL_CUTTHROAT))
     affect_from_char(victim, SKILL_CUTTHROAT);
      break;
   case SPELL_MASS_HEAL:
      hit = 200;
      send_to_char("A warm feeling floods your body.\r\n", victim);
      if (ch !=victim)
     act("$N's wounds glow and mend themselves.",
         TRUE, ch, 0, victim, TO_NOTVICT);
      if (affected_by_spell(victim, SKILL_CUTTHROAT))
     affect_from_char(victim, SKILL_CUTTHROAT);
      break;
   case SPELL_VITALITY:
      send_to_char("You feel vitalized!\r\n", victim);
      move = dice(10,10);
      hit = dice(5,10);
      break;
   case SPELL_INVIGORATE:
      send_to_char("You feel invigorated!\r\n", victim);
      move = dice(10,10);
      break;
   case SPELL_LAY_HANDS:
      if (victim == ch)
     send_to_char("Your wounds mend beneath your hands!\r\n", victim);
      else
     act("Your wounds start to heal beneath $n's hands!", TRUE, ch, 0,
         victim, TO_VICT);
      hit = dice(3, GET_LEVEL(ch));
      break;
   }
   if (GET_MAX_HIT(victim) > GET_HIT(victim))
     GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
   GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
   update_pos(victim);
}


void
mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
          int spellnum, int type)
{
   int spell = 0, spell2 = 0;
   char *to_vict = NULL, *to_room = NULL;

   if (victim == NULL)
      return;

   switch (spellnum)
   {
   case SPELL_CURE_BLIND:
   case SPELL_HEAL:
   case SPELL_MASS_HEAL:
      spell = SPELL_BLINDNESS;
      spell2 = SPELL_SMOKESCREEN;
      to_vict = "Your vision returns!";
      to_room = "There's a momentary gleam in $n's eyes.";
      break;
   case SPELL_REMOVE_POISON:
      spell = SPELL_POISON;
      to_vict = "A warm feeling runs through your body!";
      to_room = "$n looks better.";
      break;
   case SPELL_REMOVE_CURSE:
      spell = SPELL_CURSE;
      to_vict = "You don't feel so unlucky.";
      break;
   default:
      sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects",
          spellnum);
      log(buf);
      return;
      break;
   }

   if (!affected_by_spell(victim, spell) && !affected_by_spell(victim, spell2))
   {
      if (spellnum != SPELL_HEAL && spellnum != SPELL_MASS_HEAL)
     send_to_char(NOEFFECT, ch);
      return;
   }

   affect_from_char(victim, spell);
   if (spell2)
      affect_from_char(victim, spell2);

   if (to_vict != NULL)
      act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
   if (to_room != NULL)
      act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void
mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
           int spellnum, int savetype)
{
   char *to_char = NULL;
   char *to_room = NULL;

   if (obj == NULL)
      return;

   switch (spellnum)
   {
   case SPELL_BLESS:
      if (!IS_OBJ_STAT(obj, ITEM_BLESS) && !IS_OBJ_STAT(obj, ITEM_MAGIC) &&
      (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch)))
      {
     SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BLESS);
     to_char = "$p glows briefly with a ethereal light.";
      }
      break;
   case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP))
      {
     SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODROP);
     if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
        GET_OBJ_VAL(obj, 2)--;
     to_char = "$p briefly glows red.";
      }
      break;
   case SPELL_INVISIBLE:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS))
      {
     SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
     to_char = "$p vanishes.";
      }
      break;
   case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
       (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
       (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3))
      {
     GET_OBJ_VAL(obj, 3) = 1;
     to_char = "$p steams briefly.";
      }
      break;
   case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP))
      {
     REMOVE_BIT_AR(obj->obj_flags.extra_flags, ITEM_NODROP);
     if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
        GET_OBJ_VAL(obj, 2)++;
     to_char = "$p briefly glows blue.";
      }
      break;
   case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
       (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
       (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3))
      {
     GET_OBJ_VAL(obj, 3) = 0;
     to_char = "$p steams briefly.";
      }
      break;
   }

   if (to_char == NULL)
      send_to_char(NOEFFECT, ch);
   else
      act(to_char, TRUE, ch, obj, 0, TO_CHAR);

   if (to_room != NULL)
      act(to_room, TRUE, ch, obj, 0, TO_ROOM);
   else if (to_char != NULL)
      act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}


void
mag_creations(int level, struct char_data * ch, int spellnum)
{
   struct obj_data *tobj;
   int z;

   if (ch == NULL)
      return;
   level = MAX(MIN(level, LVL_IMPL), 1);

   switch (spellnum)
   {
   case SPELL_CREATE_FOOD:
      z = 8062;
      break;
   default:
      send_to_char("Spell unimplemented, it would seem.\r\n", ch);
      return;
      break;
   }

   if (!(tobj = read_object(z, VIRTUAL)))
   {
      send_to_char("I seem to have goofed.\r\n", ch);
      sprintf(buf, "SYSERR: spell_creations, spell %d, obj %d: obj not found",
          spellnum, z);
      log(buf);
      return;
   }
   obj_to_char(tobj, ch);
   act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
   act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
}
