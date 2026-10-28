/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * _TwyliteMud_ by
Rv.  Based on CircleMud3.0bpl9 * * * * OasisOLC - medit.c * * * * Copyright 1996 Harvey
Gilpin.  *
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

/* $Id: medit.c 1273 2005-12-20 05:19:42Z mcooper $ */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "shop.h"
#include "olc.h"
#include "constants.h"

/*-------------------------------------------------------------------*/
/* external variables */
extern struct attack_hit_type attack_hit_text[]; 	/*. fight.c 	.*/
extern int top_shop;					/*. shop.c	.*/
extern struct shop_data *shop_index;			/*. shop.c	.*/
extern int find_exp(int level);                        /*. class.c .*/
extern double pow(double x, double y);
/*-------------------------------------------------------------------*/
/*. Handy  macros .*/

#define GET_ALIAS(mob) ((mob)->player.name)
#define GET_SDESC(mob) ((mob)->player.short_descr)
#define GET_LDESC(mob) ((mob)->player.long_descr)
#define GET_DDESC(mob) ((mob)->player.description)
#define GET_ATTACK(mob) ((mob)->mob_specials.attack_type)
#define S_KEEPER(shop) ((shop)->keeper)

/*-------------------------------------------------------------------*/
/*. Function prototypes .*/

void medit_parse(struct descriptor_data * d, char *arg);
void medit_disp_menu(struct descriptor_data * d);
void medit_setup_new(struct descriptor_data *d);
void medit_setup_existing(struct descriptor_data *d, int rmob_num);
void medit_save_internally(struct descriptor_data *d);
void medit_save_to_disk(struct descriptor_data *d);
void init_mobile(struct char_data *mob);
void copy_mobile(struct char_data *tmob, struct char_data *fmob);
void medit_disp_positions(struct descriptor_data *d);
void medit_disp_mob_flags(struct descriptor_data *d);
void medit_disp_aff_flags(struct descriptor_data *d);
void medit_disp_script_flags(struct descriptor_data *d);
void medit_disp_attack_types(struct descriptor_data *d);

/*-------------------------------------------------------------------*\
  utility functions 
\*-------------------------------------------------------------------*/


void
medit_setup_new(struct descriptor_data *d)
{
  struct char_data *mob;

  /*. Alloc some mob shaped space .*/
  CREATE(mob, struct char_data, 1);
  init_mobile(mob);
  
  GET_MOB_RNUM(mob) = -1;
  /*. default strings .*/
  GET_ALIAS(mob) = str_dup("mob unfinished");
  GET_SDESC(mob) = str_dup("the unfinished mob");
  GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
  GET_DDESC(mob) = str_dup("It looks, err, unfinished.\r\n");
  GET_MOB_RACE(mob) = RACE_HUMANOID;
  OLC_MOB(d) = mob;
  OLC_VAL(d) = 0;   /*. Has changed flag .*/
  medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

void
medit_setup_existing(struct descriptor_data *d, int rmob_num)
{
  struct char_data *mob;

  /*. Alloc some mob shaped space .*/
  CREATE(mob, struct char_data, 1);
  copy_mobile(mob, mob_proto + rmob_num);
  OLC_MOB(d) = mob;
  medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/
/*. Copy one mob struct to another .*/

void
copy_mobile(struct char_data *tmob, struct char_data *fmob)
{
  /*. Free up any used strings .*/
  if (GET_ALIAS(tmob))
    FREE(GET_ALIAS(tmob));
  if (GET_SDESC(tmob))
    FREE(GET_SDESC(tmob));
  if (GET_LDESC(tmob))
    FREE(GET_LDESC(tmob));
  if (GET_DDESC(tmob))
    FREE(GET_DDESC(tmob));
  if (GET_NOISE(tmob))
    FREE(GET_NOISE(tmob));
  if (GET_SOUND(tmob))
    FREE(GET_SOUND(tmob));

  /*.Copy mob .*/
  *tmob = *fmob;
 
  /*. Realloc strings .*/
  if (GET_ALIAS(fmob))
    GET_ALIAS(tmob) = str_dup(GET_ALIAS(fmob));

  if (GET_SDESC(fmob))
    GET_SDESC(tmob) = str_dup(GET_SDESC(fmob));

  if (GET_LDESC(fmob))
    GET_LDESC(tmob) = str_dup(GET_LDESC(fmob));

  if (GET_DDESC(fmob))
    GET_DDESC(tmob) = str_dup(GET_DDESC(fmob));

  GET_MOB_RACE(tmob) = GET_MOB_RACE(fmob);

  if (GET_NOISE(fmob))
    GET_NOISE(tmob) = str_dup(GET_NOISE(fmob));

  if (GET_SOUND(fmob))
    GET_SOUND(tmob) = str_dup(GET_SOUND(fmob));
}


/*-------------------------------------------------------------------*/
/*. Ideally, this function should be in db.c, but I'll put it here for
    portability.*/

void
init_mobile(struct char_data *mob)
{
  GET_HIT(mob) = 1;
  GET_MANA(mob) = 100;
  GET_MOVE(mob) = 100;
  GET_MAX_MANA(mob) = 100;
  GET_MAX_MOVE(mob) = 100;
  GET_NDD(mob) = 1;
  GET_SDD(mob) = 1;
  GET_NHPD(mob) = 1;
  GET_SHPD(mob) = 8;
  GET_HPBONUS(mob) = 0;
  GET_WEIGHT(mob) = 200;
  GET_MOB_TIMER(mob) = 0;

  mob->real_abils.str = 10;
  mob->real_abils.intel = 10;
  mob->real_abils.wis = 10;
  mob->real_abils.dex = 10;
  mob->real_abils.con = 10;
  mob->real_abils.cha = 10;
  mob->aff_abils = mob->real_abils;

  SET_BIT(MOB_FLAGS(mob), MOB_ISNPC);
  mob->player_specials = &dummy_mob;
  GET_NOISE(mob) = NULL;
  GET_SOUND(mob) = NULL;
  GET_MOB_SCRIPT(mob) = NULL;
}

/*-------------------------------------------------------------------*/
/*. Save new/edited mob to memory .*/

#define ZCMD zone_table[zone].cmd[cmd_no]

void
medit_save_internally(struct descriptor_data *d)
{
  int rmob_num, found = 0, new_mob_num = 0, zone, cmd_no, shop;
  struct char_data *new_proto;
  struct index_data *new_index;
  struct char_data *live_mob;
  struct descriptor_data *dsc;

  rmob_num = real_mobile(OLC_NUM(d));

  /*. Mob exists? Just update it .*/
  if (rmob_num != -1)
    {
      copy_mobile((mob_proto + rmob_num), OLC_MOB(d));
      /*. Update live mobiles .*/
      for(live_mob = character_list; live_mob; live_mob = live_mob->next)
	if(IS_MOB(live_mob) && GET_MOB_RNUM(live_mob) == rmob_num)
	  { /*. Only really need update the strings, since these can cause
	      protection faults.  The rest can wait till a reset/reboot .*/
	    GET_ALIAS(live_mob) = GET_ALIAS(mob_proto + rmob_num);
	    GET_SDESC(live_mob) = GET_SDESC(mob_proto + rmob_num);
	    GET_LDESC(live_mob) = GET_LDESC(mob_proto + rmob_num);
	    GET_DDESC(live_mob) = GET_DDESC(mob_proto + rmob_num);
	    GET_NOISE(live_mob) = GET_NOISE(mob_proto + rmob_num);
            GET_SOUND(live_mob) = GET_SOUND(mob_proto + rmob_num);
	  }
    } 
  /*. Mob does not exist, hafta add it .*/
  else
    {
      CREATE(new_proto, struct char_data, top_of_mobt + 2);
      CREATE(new_index, struct index_data, top_of_mobt + 2);

      for (rmob_num = 0; rmob_num <= top_of_mobt; rmob_num++)
	{
	  if (!found)
	    { /*. Is this the place?  .*/
	      if ((rmob_num > top_of_mobt) ||
		  (mob_index[rmob_num].virtual > OLC_NUM(d)))
		{ /*. Yep, stick it here .*/
		  found = 1;
		  new_index[rmob_num].virtual = OLC_NUM(d);
		  new_index[rmob_num].func = NULL;
                  CREATE(new_index[rmob_num].script, struct script_data, 1);
                  new_index[rmob_num].script->name = NULL;
                  new_index[rmob_num].script->lua_functions = 0;
		  new_mob_num = rmob_num;
		  GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
		  copy_mobile((new_proto + rmob_num), OLC_MOB(d));
		  /*. Copy the mob that should be here on top .*/
		  new_index[rmob_num + 1] = mob_index[rmob_num];
		  new_proto[rmob_num + 1] = mob_proto[rmob_num];
		  GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
		}
	      else
		{ /*. Nope, copy over as normal.*/
		  new_index[rmob_num] = mob_index[rmob_num];
		  new_proto[rmob_num] = mob_proto[rmob_num];
		}
	  }
	  else
	    { /*. We've already found it, copy the rest over .*/
	      new_index[rmob_num + 1] = mob_index[rmob_num];
	      new_proto[rmob_num + 1] = mob_proto[rmob_num];
	      GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
	    }
	}
      if (!found)
	{ /*. Still not found, must add it to the top of the table .*/
	  new_index[rmob_num].virtual = OLC_NUM(d);
	  new_index[rmob_num].func = NULL;
          CREATE(new_index[rmob_num].script, struct script_data, 1);
          new_index[rmob_num].script->name = NULL;  
          new_index[rmob_num].script->lua_functions = 0;
	  new_mob_num = rmob_num;
	  GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
	  copy_mobile((new_proto + rmob_num), OLC_MOB(d));
	}

      /*. Replace tables .*/
      mob_index = new_index;
      mob_proto = new_proto;
      top_of_mobt++;

      /*. Update live mobile rnums .*/
      for(live_mob = character_list; live_mob; live_mob = live_mob->next)
	if(GET_MOB_RNUM(live_mob) >= new_mob_num)/* SER 961222 was > */
	  GET_MOB_RNUM(live_mob)++;
	  
	  /*. Update zone table .*/
	  for (zone = 0; zone <= top_of_zone_table; zone++)
	    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) 
	      if (ZCMD.command == 'M') {
		if (ZCMD.arg1 >= new_mob_num)/* SER 961222 was > */
		  ZCMD.arg1++;
             }  else if (ZCMD.command == 'R')
                if (!ZCMD.arg2)
                  if (ZCMD.arg3 >= new_mob_num)
                    ZCMD.arg3++;

      /*. Update shop keepers .*/
      for(shop = 0; top_shop && shop <= top_shop; shop++)
	if(SHOP_KEEPER(shop) >= new_mob_num)/* SER 961222 was > */
	  SHOP_KEEPER(shop)++;

	  /*. Update keepers in shops being edited .*/
	  for(dsc = descriptor_list; dsc; dsc = dsc->next)
	    if(dsc->connected == CON_SEDIT)
	      if(S_KEEPER(OLC_SHOP(dsc)) >= new_mob_num)
		S_KEEPER(OLC_SHOP(dsc))++;
    }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}


/*-------------------------------------------------------------------*/
/*. Save ALL mobiles for a zone to their .mob file, mobs are all 
    saved in Extended format, regardless of whether they have any
    extended fields.  Thanks to Samedi for ideas on this bit of code.*/

void
medit_save_to_disk(struct descriptor_data *d)
{ 
  int i, rmob_num, zone, top;
  FILE *mob_file;
  char fname[64];
  struct char_data *mob;

  zone = zone_table[OLC_ZNUM(d)].number; 
  top = zone_table[OLC_ZNUM(d)].top; 

  sprintf(fname, "%s/%i.mob", MOB_PREFIX, zone);

  if(!(mob_file = fopen(fname, "w")))
  {
    mudlog("SYSERR: OLC: Cannot open mob file!", BRF, LVL_BUILDER, TRUE);
    return;
  }

  /*. Seach database for mobs in this zone and save em .*/
  for(i = zone * 100; i <= top; i++)
    {
      rmob_num = real_mobile(i);
    
      if(rmob_num != -1) 
	{
	  if(fprintf(mob_file, "#%d\n", i) < 0)
	    {
	      mudlog("SYSERR: OLC: Cannot write mob file!\r\n",
		     BRF, LVL_BUILDER, TRUE);
	      fclose(mob_file);
	      return;
	    }
	  mob = (mob_proto + rmob_num);

	  /*. Clean up strings .*/
	  strcpy (buf1, GET_LDESC(mob) ? GET_LDESC(mob) : "undefined");
	  strip_string(buf1);
	  strcpy(buf2, GET_DDESC(mob) ? GET_DDESC(mob) : "undefined");
	  strip_string(buf2);

          sprintascii(buf, MOB_FLAGS(mob));
          sprintascii(buf3, AFF_FLAGS(mob));

	  fprintf(mob_file, 
		  "%s~\n"
		  "%s~\n"
		  "%s~\n"
		  "%s~\n"
		  "%s "   /* mob flags */
                  "%s "   /* mob affects */
                  "%i E\n" 
		  "%d %i %dd%d+%d %dd%d\n" 
		  "%ld %ld\n"
		  "%d %d %d\n",
		  GET_ALIAS(mob) ? GET_ALIAS(mob) : "undefined",
		  GET_SDESC(mob) ? GET_SDESC(mob) : "undefined",
		  buf1,
		  buf2,
		  buf,  /* mob flags */
                  buf3, /* mob affects */
		  GET_ALIGNMENT(mob), 
		  GET_LEVEL(mob),
		  GET_AC(mob),
		  GET_NHPD(mob),
		  GET_SHPD(mob),
		  GET_HPBONUS(mob),
		  GET_NDD(mob),
		  GET_SDD(mob),
		  (long)GET_GOLD(mob),
		  (long)GET_EXP(mob),
		  GET_POS(mob),
		  GET_DEFAULT_POS(mob),
		  GET_SEX(mob)
		  );

	  /*. Deal with Extra stats in case they are there .*/
	  if(GET_ATTACK(mob) != 0)
	    fprintf(mob_file, "BareHandAttack: %d\n", GET_ATTACK(mob));
/*
	  if(GET_STR(mob) != 11)
	    fprintf(mob_file, "Str: %d\n", GET_STR(mob));
	  if(GET_DEX(mob) != 11)
	    fprintf(mob_file, "Dex: %d\n", GET_DEX(mob));
	  if(GET_INT(mob) != 11)
	    fprintf(mob_file, "Int: %d\n", GET_INT(mob));
	  if(GET_WIS(mob) != 11)
	    fprintf(mob_file, "Wis: %d\n", GET_WIS(mob));
	  if(GET_CON(mob) != 11)
	    fprintf(mob_file, "Con: %d\n", GET_CON(mob));
	  if(GET_CHA(mob) != 11)
	    fprintf(mob_file, "Cha: %d\n", GET_CHA(mob));
*/
	  /* Begin Serapis additions */
	  if(GET_MOB_RACE(mob) != RACE_HUMANOID)
	    fprintf(mob_file, "Race: %d\n", GET_MOB_RACE(mob));
	  if(GET_NOISE(mob))
	    fprintf(mob_file, "Noise: %s\n", GET_NOISE(mob));
          if (GET_SOUND(mob))
            fprintf(mob_file, "Sound: %s\n", GET_SOUND(mob));
          if(GET_MOB_SCRIPT(mob) && GET_MOB_SCRIPT(mob)->name)
            fprintf(mob_file, "Script: %s %d\n", GET_MOB_SCRIPT(mob)->name, MOB_SCRIPT_FLAGS(mob));
	  /* End Serapis additions */
	  
	  /*. Add E-mob handlers here .*/

	  fprintf(mob_file, "E\n");
	}
    }
  fprintf(mob_file, "$\n");
  fclose(mob_file);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/
/*. Display poistions (sitting, standing etc) .*/

void
medit_disp_positions(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);

  send_to_char("[H[J", d->character);
  for (i = 0; *position_types[i] != '\n'; i++)
    {
      sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, position_types[i]);
      send_to_char(buf, d->character);
    }
  send_to_char("Enter position number : ", d->character);
}

/*-------------------------------------------------------------------*/
/*. Display sex (Oooh-err).*/

void
medit_disp_sex(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);

  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_GENDERS; i++)
    {
      sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, genders[i]);
      send_to_char(buf, d->character);
    }
  send_to_char("Enter gender number : ", d->character);
}

/*-------------------------------------------------------------------*/
/*. Display attack types menu .*/

void
medit_disp_attack_types(struct descriptor_data *d)
{
  int i;
  
  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_ATTACK_TYPES; i++)
    {
      sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, attack_hit_text[i].singular);
      send_to_char(buf, d->character);
    }
  send_to_char("Enter attack type : ", d->character);
}
 

/*-------------------------------------------------------------------*/
/*. Display mob-flags menu .*/

void
medit_disp_mob_flags(struct descriptor_data *d)
{
  int i, columns = 0;
  
  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_MOB_FLAGS; i++)
    {
      sprintf( buf, "%s%2d%s) %-20.20s  ",
	       grn, i+1, nrm, action_bits[i] );
      if(!(++columns % 2))
	strcat(buf, "\r\n");
      send_to_char(buf, d->character);
    }
  /* LITERAL 4 -- VERY BAD */
  sprintbit(MOB_FLAGS(OLC_MOB(d)), action_bits, buf1);
  sprintf( buf, "\r\n"
	   "Current flags : %s%s%s\r\n"
	   "Enter mob flags (0 to quit) : ",
	   cyn, buf1, nrm );
  send_to_char(buf, d->character);
}

/*-------------------------------------------------------------------*/
/*. Display aff-flags menu .*/

void medit_disp_aff_flags(struct descriptor_data *d)
{ 
  int i, columns = 0;
  
  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_AFF_FLAGS; i++)
  {  sprintf(buf, "%s%2d%s) %-20.20s  ", 
	grn, i+1, nrm, affected_bits[i]
     );
     if(!(++columns % 3))
       strcat(buf, "\r\n");
     send_to_char(buf, d->character);
  }
  sprintbit(AFF_FLAGS(OLC_MOB(d)), affected_bits, buf1);
  sprintf(buf, "\r\n"
	"Current flags   : %s%s%s\r\n"
	"Enter aff flags (0 to quit) : ",
        cyn, buf1, nrm
  );
  send_to_char(buf, d->character);
}


void medit_disp_script_flags(struct descriptor_data *d)
{ 
  int i, columns = 0;
  
  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_MSCRIPT_FLAGS; i++)
  {  sprintf(buf, "%s%2d%s) %-20.20s  ", 
	grn, i+1, nrm, mscript_bits[i]
     );
     if(!(++columns % 2))
       strcat(buf, "\r\n");
     send_to_char(buf, d->character);
  }
  sprintbit(MOB_SCRIPT_FLAGS(OLC_MOB(d)), mscript_bits, buf1);
  sprintf(buf, "\r\n"
	"Current flags   : %s%s%s\r\n"
	"Enter script flags (0 to quit) : ",
        cyn, buf1, nrm
  );
  send_to_char(buf, d->character);
}


/*-------------------------------------------------------------------*/
/*. Display attack types menu .*/

void
medit_disp_races(struct descriptor_data *d)
{
  int i, columns = 0;
  
  get_char_cols(d->character);
  send_to_char("[H[J", d->character);
  for (i = 0; i < NUM_RACES; i++)
    {
      sprintf(buf, "%s%2d%s) %-20.20s  ", grn, i, nrm, npc_races[i].race);
      if(!(++columns % 3))
	strcat(buf, "\r\n");
      send_to_char(buf, d->character);
    }
  send_to_char("\r\nEnter mob race : ", d->character);
}
 
  
/*-------------------------------------------------------------------*/
/*. Display main menu .*/

void
medit_disp_menu(struct descriptor_data * d)
{
  struct char_data *mob = OLC_MOB(d);
  get_char_cols(d->character);
  
  sprintf(buf, "[H[J" 
	  "-- Mob Number:  [%s%ld%s]\r\n"
	  "%s1%s) Sex: %s%-7.7s%s	         %s2%s) Alias: %s%s\r\n"
	  "%s3%s) S-Desc: %s%s\r\n"
	  "%s4%s) L-Desc:-\r\n%s%s"
	  "%s5%s) D-Desc:-\r\n%s%s"
	  "%s6%s) Level:       [%s%4d%s],  %s7%s) Alignment:    [%s%4d%s]\r\n"
	  "%sA%s) NumDamDice:  [%s%4d%s],  %sB%s) SizeDamDice:  [%s%4d%s]\r\n"
	  "%sC%s) Num HP Dice: [%s%4d%s],  %sD%s) Size HP Dice: [%s%4d%s],  %sE%s) HP Bonus: [%s%5d%s]\r\n"
	  "%sF%s) Armor Class: [%s%4d%s],  %sG%s) Exp:     [%s%9ld%s],  %sH%s) Gold:  [%s%8ld%s]\r\n",
	  cyn, OLC_NUM(d), nrm,
	  grn, nrm, yel, genders[(int)GET_SEX(mob)], nrm,
	  grn, nrm, yel, GET_ALIAS(mob),
	  grn, nrm, yel, GET_SDESC(mob),
	  grn, nrm, yel, GET_LDESC(mob) ? GET_LDESC(mob) : "<None>\r\n",
	  grn, nrm, yel, GET_DDESC(mob),
	  grn, nrm, cyn, GET_LEVEL(mob), nrm,
	  grn, nrm, cyn, GET_ALIGNMENT(mob), nrm,
	  grn, nrm, cyn, GET_NDD(mob), nrm,
	  grn, nrm, cyn, GET_SDD(mob), nrm,
	  grn, nrm, cyn, GET_NHPD(mob), nrm,
	  grn, nrm, cyn, GET_SHPD(mob), nrm,
	  grn, nrm, cyn, GET_HPBONUS(mob), nrm,
	  grn, nrm, cyn, GET_AC(mob), nrm, 
	  /*. Gold & Exp are longs in my mud, ignore any warnings .*/
	  grn, nrm, cyn, (long)GET_EXP(mob), nrm,
	  grn, nrm, cyn, (long)GET_GOLD(mob), nrm);
  send_to_char(buf, d->character);

  sprintbit(MOB_FLAGS(mob), action_bits, buf1);
  sprintbit(AFF_FLAGS(mob), affected_bits, buf2);
  sprintf(buf,
	  "%sI%s) Position  : %s%s\r\n"
	  "%sJ%s) Default   : %s%s\r\n"
	  "%sK%s) Attack    : %s%s\r\n"
	  "%sL%s) NPC Flags : %s%s\r\n"
	  "%sM%s) AFF Flags : %s%s\r\n"
	  "%sN%s) Race      : %s%s\r\n"
	  "%sO%s) Noise     : %s%s\r\n"
          "%sP%s) Sound     : %s%s\r\n"
          "%sS%s) Script Menu   \r\n"
	  "%sQ%s) Quit\r\n"
	  "Enter choice : ",

	  grn, nrm, yel, position_types[(int)GET_POS(mob)],
	  grn, nrm, yel, position_types[(int)GET_DEFAULT_POS(mob)],
	  grn, nrm, yel, attack_hit_text[GET_ATTACK(mob)].singular,
	  grn, nrm, cyn, buf1, 
	  grn, nrm, cyn, buf2,
	  grn, nrm, cyn, npc_races[GET_MOB_RACE(mob)].race,
	  grn, nrm, cyn, GET_NOISE(mob)?GET_NOISE(mob):"None",
          grn, nrm, cyn, GET_SOUND(mob) ? GET_SOUND(mob) : "None",
          grn, nrm, grn, nrm );

  send_to_char(buf, d->character);

  OLC_MODE(d) = MEDIT_MAIN_MENU;
}

void 
medit_disp_script_menu(struct descriptor_data *d)
{
  struct char_data *mob = OLC_MOB(d);   
  get_char_cols(d->character);

  sprintbit(MOB_SCRIPT_FLAGS(mob), mscript_bits, buf1);

  sprintf(buf, "[H[J" 
          "%s1%s) Name: %s%s\r\n"
          "%s2%s) Script Flags: %s%s%s\r\n",
          grn, nrm, yel,
          GET_MOB_SCRIPT(mob)->name ? GET_MOB_SCRIPT(mob)->name : "None",
          grn, nrm, yel, buf1, nrm);

  strcat(buf, "Enter choice (0 to quit) : ");
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_SCRIPT_MENU;
}

/**************************************************************************
  The GARGANTUAN event handler
 **************************************************************************/

void
medit_parse(struct descriptor_data * d, char *arg)
{
  int i, statmod;

  if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE)
    {
      if(!*arg || (!isdigit(arg[0]) && ((*arg == '-') && (!isdigit(arg[1])))))
	{
	  send_to_char("Field must be numerical, try again : ", d->character);
	  return;
	}
    }

  switch (OLC_MODE(d)) 
    {
      /*-------------------------------------------------------------------*/
    case MEDIT_CONFIRM_SAVESTRING:
      /*. Ensure mob has MOB_ISNPC set or things will go pair shaped .*/
      SET_BIT(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
      switch (*arg) {
      case 'y':
      case 'Y':
	/*. Save the mob in memory and to disk  .*/
	send_to_char("Saving mobile to memory.\r\n", d->character);
	medit_save_internally(d);
	sprintf(buf, "OLC: %s edits mob %ld", GET_NAME(d->character),
		OLC_NUM(d));	      
	mudlog(buf, CMP, LVL_BUILDER, TRUE);
	cleanup_olc(d, CLEANUP_ALL);
	return;
      case 'n':
      case 'N':
	cleanup_olc(d, CLEANUP_ALL);
	return;
      default:
	send_to_char("Invalid choice!\r\n", d->character);
	send_to_char("Do you wish to save the mobile? : ", d->character);
	return;
      }
      break;

      /*-------------------------------------------------------------------*/
    case MEDIT_MAIN_MENU:
      i = 0;
      switch (*arg) 
	{
	case 'q':
	case 'Q':
	  if (OLC_VAL(d)) /*. Anything been changed? .*/
	    {
	      send_to_char("Do you wish to save the changes to the "
			   "mobile? (y/n) : ", d->character);
	      OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
	    }
	  else
	    cleanup_olc(d, CLEANUP_ALL);
	  return;
	case '1':
	  OLC_MODE(d) = MEDIT_SEX;
	  medit_disp_sex(d);
	  return;
	case '2':
	  OLC_MODE(d) = MEDIT_ALIAS;
	  i--;
	  break;
	case '3':
	  OLC_MODE(d) = MEDIT_S_DESC;
	  i--;
	  break;
	case '4':
	  OLC_MODE(d) = MEDIT_L_DESC;
	  i--;
	  break;
	case '5':
	  send_to_char("Enter new mob description:\r\n", d->character);
	  /*. Pass control to modify.c .*/
	  OLC_MODE(d) = MEDIT_D_DESC;
          CREATE(d->str, char *, sizeof(char *));
	  *(d->str) = NULL;
	  d->max_str = MAX_MOB_DESC;
	  d->mail_to = 0;
	  OLC_VAL(d) = 1;
	  return;
	case '6':
	  OLC_MODE(d) = MEDIT_LEVEL;
	  i++;
	  break;
	case '7':
	  OLC_MODE(d) = MEDIT_ALIGNMENT;
	  i++;
	  break;
	case 'a':
	case 'A':
	  OLC_MODE(d) = MEDIT_NDD;
	  i++;
	  break;
	case 'b':
	case 'B':
	  OLC_MODE(d) = MEDIT_SDD;
	  i++;
	  break;
	case 'c':
	case 'C':
	  OLC_MODE(d) = MEDIT_NUM_HP_DICE;
	  i++;
	  break;
	case 'd':
	case 'D':
	  OLC_MODE(d) = MEDIT_SIZE_HP_DICE;
	  i++;
	  break;
	case 'e':
	case 'E':
	  OLC_MODE(d) = MEDIT_ADD_HP;
	  i++;
	  break;
	case 'f':
	case 'F':
	  OLC_MODE(d) = MEDIT_AC;
	  i++;
	  break;
	case 'g':
	case 'G':
	  OLC_MODE(d) = MEDIT_EXP;
	  i++;
	  break;
	case 'h':
	case 'H':
	  OLC_MODE(d) = MEDIT_GOLD;
	  i++;
	  break;
	case 'i':
	case 'I':
	  OLC_MODE(d) = MEDIT_POS;
	  medit_disp_positions(d);
	  return;
	case 'j':
	case 'J':
	  OLC_MODE(d) = MEDIT_DEFAULT_POS;
	  medit_disp_positions(d);
	  return;
	case 'k':
	case 'K':
	  OLC_MODE(d) = MEDIT_ATTACK;
	  medit_disp_attack_types(d);
	  return;
	case 'l':
	case 'L':
	  OLC_MODE(d) = MEDIT_NPC_FLAGS;
	  medit_disp_mob_flags(d);
	  return;
	case 'm':
	case 'M':
	  OLC_MODE(d) = MEDIT_AFF_FLAGS;
	  medit_disp_aff_flags(d);
	  return;
	case 'n':
	case 'N':
	  OLC_MODE(d) = MEDIT_RACE;
	  medit_disp_races(d);
	  return;
        case 'o':
	case 'O':
	  OLC_MODE(d) = MEDIT_NOISE;
	  i--;
	  break;
        case 'p':
        case 'P':
          OLC_MODE(d) = MEDIT_SOUND;
          i=-2;
          break;
        case 's':
        case 'S':
          if (GET_MOB_RNUM(OLC_MOB(d)) != -1)  /* not in index yet */
          {
            OLC_MODE(d) = MEDIT_SCRIPT_MENU;
            medit_disp_script_menu(d);
            return;
          }
	default:
	  medit_disp_menu(d);
	  return;
	}

        if (i == 1) {
	  send_to_char("\r\nEnter new value : ", d->character);
	  return;
	}
        if (i == -1) {
	  send_to_char("\r\nEnter new text :\r\n", d->character);
	  return;
	}
        if (i == -2) {
	  send_to_char("\r\nEnter new sound file :\r\n", d->character);
	  return;
  	}
      break; 

      /*-------------------------------------------------------------------*/
    case MEDIT_ALIAS:
      if(GET_ALIAS(OLC_MOB(d)))
	FREE(GET_ALIAS(OLC_MOB(d)));
      GET_ALIAS(OLC_MOB(d)) = str_dup(arg); 
      break;
      /*-------------------------------------------------------------------*/
    case MEDIT_S_DESC:
      if(GET_SDESC(OLC_MOB(d)))
	FREE(GET_SDESC(OLC_MOB(d)));
      GET_SDESC(OLC_MOB(d)) = str_dup(arg); 
      break;
      /*-------------------------------------------------------------------*/
    case MEDIT_L_DESC:
      if(GET_LDESC(OLC_MOB(d)))
	FREE(GET_LDESC(OLC_MOB(d)));
      if (!*arg)
        GET_LDESC(OLC_MOB(d)) = NULL;
      else {
        strcpy(buf, arg);
        strcat(buf, "\r\n");
        GET_LDESC(OLC_MOB(d)) = str_dup(buf); 
      }
      break;
      /*-------------------------------------------------------------------*/
    case MEDIT_D_DESC:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog("SYSERR: OLC: medit_parse(): Reached D_DESC case!",BRF,LVL_BUILDER,TRUE);
      break;
      /*-------------------------------------------------------------------*/
    case MEDIT_NPC_FLAGS:
      if ((i = atoi(arg)) == 0)
	break;
      if (!((i < 0) || (i > NUM_MOB_FLAGS)))
        TOGGLE_BIT(MOB_FLAGS(OLC_MOB(d)), 1 << (i - 1));
      medit_disp_mob_flags(d);
      return;
      /*-------------------------------------------------------------------*/
    case MEDIT_AFF_FLAGS:
      if ((i = atoi(arg)) == 0)
        break;
      if (!((i < 0) || (i > NUM_AFF_FLAGS)))
        TOGGLE_BIT(AFF_FLAGS(OLC_MOB(d)), 1LL << (i - 1));
      medit_disp_aff_flags(d);
      return;
      /*-------------------------------------------------------------------*/
    case MEDIT_NOISE:
      if(GET_NOISE(OLC_MOB(d)))
	FREE(GET_NOISE(OLC_MOB(d)));
      if (strlen(arg) > 2)
        GET_NOISE(OLC_MOB(d)) = str_dup(arg); 
      else 
	GET_NOISE(OLC_MOB(d)) = NULL;
      break;
      /*-------------------------------------------------------------------*/
    case MEDIT_SOUND:
      if (GET_SOUND(OLC_MOB(d)))
        FREE(GET_SOUND(OLC_MOB(d)));
      if (strlen(arg) > 2)
        GET_SOUND(OLC_MOB(d)) = str_dup(arg); 
      else 
        GET_SOUND(OLC_MOB(d)) = NULL;
      break;
      /*-------------------------------------------------------------------*/

    case MEDIT_SCRIPT_MENU:
      i = atoi(arg);
      switch (i) {
        case 0:  
          break;
        case 1:
          OLC_MODE(d) = MEDIT_SCRIPT_NAME;
          send_to_char("Enter script name: ", d->character);
          return; 
        case 2:
          OLC_MODE(d) = MEDIT_SCRIPT_FLAGS;
          medit_disp_script_flags(d);
          return;
        default:
          break;
       }
    break;
    case MEDIT_SCRIPT_NAME:
      if (!GET_MOB_SCRIPT(OLC_MOB(d)))
        CREATE(GET_MOB_SCRIPT(OLC_MOB(d)), struct script_data, 1);
      if (!strcmp(arg, ""))
        GET_MOB_SCRIPT(OLC_MOB(d))->name = NULL;
      else
        GET_MOB_SCRIPT(OLC_MOB(d))->name = str_dup(arg);
      medit_disp_script_menu(d);
      return;
   
    case MEDIT_SCRIPT_FLAGS:
      if ((i = atoi(arg)) == 0)
      { 
        OLC_MODE(d) = MEDIT_SCRIPT_MENU;
        medit_disp_script_menu(d);
        return;
      }
      if (!((i < 0) || (i > NUM_MSCRIPT_FLAGS)))
        TOGGLE_BIT(MOB_SCRIPT_FLAGS(OLC_MOB(d)), 1 << (i - 1));
      medit_disp_script_flags(d);
      return; 

      /*. Numerical responses .*/

    case MEDIT_SEX:
      GET_SEX(OLC_MOB(d)) = LIMIT(atoi(arg), 0, NUM_GENDERS - 1);
      break;

    case MEDIT_NDD:
      GET_NDD(OLC_MOB(d)) = LIMIT(atoi(arg), 0, 127);
      break;

    case MEDIT_SDD:
      GET_SDD(OLC_MOB(d)) = LIMIT(atoi(arg), 0, 127);
      break;

    case MEDIT_NUM_HP_DICE:
      GET_NHPD(OLC_MOB(d)) = LIMIT(atoi(arg), 0, 50);
      break;

    case MEDIT_SIZE_HP_DICE:
      GET_SHPD(OLC_MOB(d)) = LIMIT(atoi(arg), 0, 3000);
      break;

    case MEDIT_ADD_HP:
      GET_HPBONUS(OLC_MOB(d)) = LIMIT(atoi(arg), 0, 30000);
      break;

    case MEDIT_AC:
      GET_AC(OLC_MOB(d)) = LIMIT(atoi(arg), 0, 200);
      break;

    case MEDIT_EXP:
      GET_EXP(OLC_MOB(d)) = MAX(0, atol(arg));
      break;

    case MEDIT_GOLD:
      GET_GOLD(OLC_MOB(d)) = MAX(0, atol(arg));
      break;

    case MEDIT_POS:
      GET_POS(OLC_MOB(d)) = LIMIT(atoi(arg), 0, NUM_POSITIONS - 1);
      break;

    case MEDIT_DEFAULT_POS:
      GET_DEFAULT_POS(OLC_MOB(d)) = LIMIT(atoi(arg), 0, NUM_POSITIONS - 1);
      break;

    case MEDIT_ATTACK:
      GET_ATTACK(OLC_MOB(d)) = LIMIT(atoi(arg), 0, NUM_ATTACK_TYPES - 1);
      break;

    case MEDIT_LEVEL:
      GET_LEVEL(OLC_MOB(d)) = LIMIT(atoi(arg), 1, 100);
      GET_EXP(OLC_MOB(d)) = (find_exp(GET_LEVEL(OLC_MOB(d))) / 20);
      mob_combat_dice(OLC_MOB(d));
      GET_DAMROLL(OLC_MOB(d)) = ( GET_LEVEL(OLC_MOB(d))>10?
				  ((GET_LEVEL(OLC_MOB(d)) + 1) / 1.50 ):
			       	  ((GET_LEVEL(OLC_MOB(d)) + 1) / 2) );
      GET_NHPD(OLC_MOB(d)) = GET_LEVEL(OLC_MOB(d));
      GET_SHPD(OLC_MOB(d)) = 8;
      GET_HPBONUS(OLC_MOB(d)) = (3 * pow(GET_LEVEL(OLC_MOB(d)), 1.5));
      GET_AC(OLC_MOB(d)) = pow(GET_LEVEL(OLC_MOB(d)), 1.15);
      GET_HITROLL(OLC_MOB(d)) = GET_LEVEL(OLC_MOB(d)) / 3;
      GET_DODGEROLL(OLC_MOB(d)) = GET_LEVEL(OLC_MOB(d)) / 10;

      /* set it again to handle 0s */
      GET_LEVEL(OLC_MOB(d)) = MAX(0, MIN(100, atoi(arg)));

      /* Adjust abilities */
      statmod = pow(GET_LEVEL(OLC_MOB(d)), .70);
      GET_REAL_STR(OLC_MOB(d)) = 5 + statmod;
      GET_REAL_INT(OLC_MOB(d)) = 5 + statmod;
      GET_REAL_WIS(OLC_MOB(d)) = 5 + statmod;
      GET_REAL_DEX(OLC_MOB(d)) = 5 + statmod;
      GET_REAL_CON(OLC_MOB(d)) = 5 + statmod;
      GET_REAL_CHA(OLC_MOB(d)) = 5 + statmod;

      break;

    case MEDIT_ALIGNMENT:
      GET_ALIGNMENT(OLC_MOB(d)) = LIMIT(atoi(arg), -1000, 1000);
      break;

    case MEDIT_RACE:
      GET_MOB_RACE(OLC_MOB(d)) = LIMIT(atoi(arg), 0, NUM_RACES - 1);
      send_to_char("Do you wish to apply flag defaults? ", d->character);
      OLC_MODE(d) = MEDIT_RACE_DEFAULTS;
      return;

    case MEDIT_RACE_DEFAULTS:
      switch (*arg) {
        case 'y':
        case 'Y':
          GET_ATTACK(OLC_MOB(d)) = npc_races[(int)GET_MOB_RACE(OLC_MOB(d))].attack_type;
          MOB_FLAGS(OLC_MOB(d)) = npc_races[(int)GET_MOB_RACE(OLC_MOB(d))].npc_flags;
          AFF_FLAGS(OLC_MOB(d)) = npc_races[(int)GET_MOB_RACE(OLC_MOB(d))].aff_flags;
          break;
        case 'n':
        case 'N':
          break;
        default:
	        send_to_char("Invalid choice!\r\n", d->character);
          send_to_char("Do you wish to apply flag defaults? ", d->character);
          return;
      }
      break;

      /*-------------------------------------------------------------------*/
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog("SYSERR: OLC: medit_parse(): Reached default case!",
	     BRF, LVL_BUILDER, TRUE);
      break;
    }
  /*-------------------------------------------------------------------*/
  /*. END OF CASE 
    If we get here, we have probably changed something, and now want to
    return to main menu.  Use OLC_VAL as a 'has changed' flag .*/

  OLC_VAL(d) = 1;
  medit_disp_menu(d);
}
/*. End of medit_parse() .*/

