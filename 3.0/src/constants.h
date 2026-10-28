/* ************************************************************************
*  File: constants.h                                    Part of CircleMUD *
*  Usage: header file for database constants                              *
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

extern char *circlemud_version;
extern char *dirs[];
extern char *room_bits[];
extern char *exit_bits[];
extern char *sector_types[];
extern char *genders[];
extern char *position_types[];
extern char *player_bits[];
extern char *action_bits[];
extern char *preference_bits[];
extern char *affected_bits[];
extern char *connected_types[];
extern char *where[];
extern char *where_mxp[];
extern char *equipment_types[];
extern char *item_types[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *apply_types[];
extern char *container_bits[];
extern char *desc_bits[];
extern char *drinks[];
extern char *drinknames[];
extern char *color_liquid[];
extern char *fullness[];
extern const char *weekdays[];
extern const char *month_name[];
extern char *phases[];
extern char *sector_types[];
extern const struct str_app_type str_app[];
extern const struct dex_app_type dex_app[];
extern const struct con_app_type con_app[];
extern const struct int_app_type int_app[];
extern const struct wis_app_type wis_app[];
extern int drink_aff[][3];
extern const int rev_dir[];
extern const int movement_loss[];
extern const struct npc_race_data npc_races[];
extern char *shop_types[];
extern const long shop_keeps[];
extern const struct mob_dice_struct mob_dice[];
extern char *oscript_bits[];
extern char *rscript_bits[];
extern char *mscript_bits[];
extern char *craft_level[];
extern char *weapon_names[];
extern char *req_types[];

