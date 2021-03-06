/***********************
 *  CircleMUD Licence  *
 ***********************/

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996-1999 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

/* $Id: clan.h 1338 2008-03-21 17:58:08Z jravn $ */

#ifndef _CLAN_H
#define _CLAN_H

#define MAX_CLANS        20
#define LVL_CLAN_GOD     LVL_GOD
#define DEFAULT_APP_LVL  8
#define CLAN_PLAN_LENGTH 1024  /* 240? */

#define GET_CLAN(ch)            ((ch)->player_specials->saved.clan)
#define GET_CLAN_RANK(ch)       ((ch)->player_specials->saved.clan_rank)

#define CP_SET_PLAN   0
#define CP_ENROLL     1
#define CP_EXPEL      2
#define CP_PROMOTE    3
#define CP_DEMOTE     4
#define CP_SET_FEES   5
#define CP_WITHDRAW   6
#define CP_SET_APPLEV 7
#define NUM_CP        8        /* Number of clan privileges */

#define CM_DUES   1
#define CM_APPFEE 2

#define CB_DEPOSIT  1
#define CB_WITHDRAW 2

#define CLAN_PUBLIC  0
#define CLAN_PRIVATE 1

void save_clans(void);
void init_clans(void);
int find_clan_by_id(int clan_id);
int find_clan(char *name);

extern struct clan_rec *clan;
extern int num_of_clans;

struct clan_rec {
  int    id;
  char   name[32];
  ubyte  ranks;
  char   rank_name[20][20];
  long   treasure;
  int    members;
  int    power;
  int    app_fee;
  int    dues;
  int    spells[5];
  int    app_level;
  ubyte  privilege[20];
  int    at_war[4];
  char * plan;
  char   description[CLAN_PLAN_LENGTH];
  int    private;
};

#endif /* _CLAN_H */
