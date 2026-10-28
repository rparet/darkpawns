/* ************************************************************************
*  file: convertpfile.c                                                   * 
*  Usage: converts pfiles from old char_file_u to current char_file_u     *
*  All Rights Reserved                                                    *
*  Copyright (C) 2001 Rich Paret                                          *
*  Based on purgeplay.c, Part of CircleMUD                                * 
*  Copyright (C) 1992, 1993 The Trustees of The Johns Hopkins University  * 
************************************************************************* */

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

/* ======== Begin Old Structs ======= */

struct player_timer_data_old {
  sbyte conditions[4];        /* Drunk, full, thirsty, tired      */
  int outlaw;                 /* When can the PC toggle outlaw?   */
  int jail;                   /* When can the PC leave jail?      */
  int tattoo;                 /* When can the PC next use tattoo? */
};



struct player_special_data_saved_old {
   struct skill_data skills;   /* skill data */
  
   bool talks[MAX_TONGUE];      /* PC s Tongues 0 for NPC               */  
   int  wimp_level;             /* Below this # of hit points, flee!    */
   byte freeze_level;           /* Level of god who froze char, if any  */
   sh_int invis_level;          /* level of invisibility                */
   room_num load_room;          /* Which room to place char in          */
   bitvector_t pref;            /* preference flags for PC's.           */
   ubyte bad_pws;               /* number of bad password attemps       */
   struct player_timer_data_old timer;  /* PC timers                        */

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */
  
   long pkcount;                /* Character's num of pks               */
   long killcount;              /* Character's num of mob kills         */
   long deathcount;             /* Character's num of deaths            */
   int race;                    /* Character's race                     */
   ubyte orig_con;              /* Characters con before con loss       */
   ubyte rank;                  /* character's rank in social structure */
   long patron;                 /* who is the character's patron (by idnum) */
   int race_hate[5];            /* 5 racial hatreds, based on races killed  */
   int spells_to_learn;         /* How many can you learn yet this level */
   int olc_zone;
   int tattoo;
   int infobar;                 /* status of infobar */
   int size;                    /* screen size */
   int religion;                /* Character's religion */
   int mount_vnum;              /* vnum of the mount                    */  
   int spare17;
   long squelch[MAX_SQUELCH];   /* character's squelch list               */
   long lastdeath;              /* when was the last time they died?      */
   long stable;                 /* vnum of the stable where mount is kept */
   long spare19;
   long spare20;
   long spare21;
};

struct char_file_u_old {
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
   struct player_special_data_saved_old player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];
 
   time_t last_logon;           /* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];    /* host of last logon */
};



/* ======= End old structs ====== */

void convert(char *filename)
{
  FILE *fl;
  FILE *outfile;
  struct char_file_u player;
  long size;

  if (!(fl = fopen(filename, "r+"))) {
    printf("Can't open %s.", filename);
    exit(1);
  }
  fseek(fl, 0L, SEEK_END);
  size = ftell(fl);
  rewind(fl);
  if (size % sizeof(struct char_file_u_old)) {
    fprintf(stderr, "\aWARNING:  File size does not match structure, recompile.\r\n");
    fclose(fl);
    exit(1);
  }

  outfile = fopen("players.new", "w");

  for (;;) 
  {
    fread(&player, sizeof(struct char_file_u_old), 1, fl);
     
    if (feof(fl)) {
      fclose(fl);
      fclose(outfile);
      printf("Done.\n");
      exit(0);
    }

      
    fwrite(&player, sizeof(struct char_file_u), 1, outfile);


  }
}



int main(int argc, char *argv[])
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    convert(argv[1]);

  return (0);
}
