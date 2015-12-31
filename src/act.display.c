/* ************************************************************************
*   File: act.display.c                                 Part of CircleMUD *
*  Usage: Miscellaneous display commands                                  *
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

/* $Id: act.display.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include <sys/stat.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "vt100.h"

extern struct title_type titles[NUM_CLASSES][LVL_IMPL + 1];
int exp_needed_for_level(struct char_data *ch);

void InfoBarOn(struct char_data *ch);
void InfoBarOff(struct char_data *ch);
void InfoBarUpdate(struct char_data *ch, int update);
void IB_Seperator(struct char_data *ch, int size);
void IB_HitPointsStr(struct char_data *ch, int size);
void IB_HitPoints(struct char_data *ch, int size);
void IB_W_HitPoints(struct char_data *ch, int size);
void IB_ClearHit(struct char_data *ch, int size);
void IB_ManaPointsStr(struct char_data *ch, int size);
void IB_ManaPoints(struct char_data *ch, int size);
void IB_W_ManaPoints(struct char_data *ch, int size);
void IB_ClearMana(struct char_data *ch, int size);
void IB_MovePointsStr(struct char_data *ch, int size);
void IB_MovePoints(struct char_data *ch, int size);
void IB_W_MovePoints(struct char_data *ch, int size);
void IB_ClearMove(struct char_data *ch, int size);
void IB_ExpPointsStr(struct char_data *ch, int size);
void IB_ExpPoints(struct char_data *ch, int size);
void IB_W_ExpPoints(struct char_data *ch, int size);
void IB_ClearExpPoints(struct char_data *ch, int size);
void IB_NeededExpPointsStr(struct char_data *ch, int size);
void IB_NeededExpPoints(struct char_data *ch, int size);
void IB_W_NeededExpPoints(struct char_data *ch, int size);
void IB_ClearNeededExpPoints(struct char_data *ch, int size);
void IB_LevelStr(struct char_data *ch, int size);
void IB_Level(struct char_data *ch, int size);
void IB_W_Level(struct char_data *ch, int size);
void IB_ClearLevel(struct char_data *ch, int size);
void IB_GoldStr(struct char_data *ch, int size);
void IB_Gold(struct char_data *ch, int size);
void IB_W_Gold(struct char_data *ch, int size);
void IB_ClearGold(struct char_data *ch, int size);



ACMD(do_lines)
{
   int size;
   one_argument(argument, arg);

   if (!*arg) {
      size = GET_SCREENSIZE(ch);
      sprintf(buf, "Your current screen size is %d.\r\n", size);
      send_to_char(buf,ch);
      return;
   }

   size = atoi(arg);

   if (size > 50) {
      send_to_char("Screen size is limited to 50 lines.\r\n", ch);
      return;
   }

   if (size < 7) {
      send_to_char("Screen size must be at least 7 lines.\r\n", ch);
      return;
   }

   GET_SCREENSIZE(ch) = size;
   /* only redraw it if it is already on */
   if (GET_INFOBAR(ch) == INFOBAR_ON)
      InfoBarOn(ch);
   sprintf(buf, "Your new lines count is %d.\r\n", size);
   send_to_char(buf,ch);
}

ACMD(do_infobar)
{
  int infobar;
  any_one_arg(argument, arg);

  if (!*arg) {
    infobar = GET_INFOBAR(ch);
    switch(infobar) {
       case INFOBAR_OFF:
          send_to_char("Your infobar is off.\r\n", ch);
          break;
       case INFOBAR_ON:
          send_to_char("Your infobar is on.\r\n", ch);
          break;
       default:
          send_to_char("You had an unknown infobar setting.\r\n", ch);
          send_to_char("It is being set to OFF.\r\n", ch);
          GET_INFOBAR(ch) = INFOBAR_OFF;
    }
    return;
  } else if (!str_cmp(arg, "off")) {
    if (GET_INFOBAR(ch) == INFOBAR_ON) {
       GET_INFOBAR(ch) = INFOBAR_OFF;
       InfoBarOff(ch);
       send_to_char("Your infobar is now set to off.\r\n", ch);
    } else {
       send_to_char("Your infobar is already off.\r\n", ch);
    }
    return;
  } else if (!str_cmp(arg, "on")) {
    if (GET_INFOBAR(ch) == INFOBAR_OFF) {
       if (GET_SCREENSIZE(ch) == 0)
          GET_SCREENSIZE(ch) = 25;
       GET_INFOBAR(ch) = INFOBAR_ON;
       InfoBarOn(ch);
       send_to_char("Your infobar is now set to on.\r\n", ch);
    } else {
       send_to_char("Your infobar is already on.\r\n", ch);
    }
    return;
  } else
    send_to_char("Usage:  infobar < on | off >\r\n", ch);
  return;

}

void InfoBarOn(struct char_data *ch) {

  char buf[255];
  int size;
  size = GET_SCREENSIZE(ch);

  /* clear screen */
  sprintf(buf, VT_HOMECLR);
  send_to_char(buf,ch);

  /* set margin */
  sprintf(buf, VT_MARGSET, 0, size - 5);
  send_to_char(buf,ch);

  IB_Seperator(ch, size);

  IB_HitPointsStr(ch, size);

  IB_ManaPointsStr(ch, size);

  IB_MovePointsStr(ch, size);

  IB_ExpPointsStr(ch, size);

  if (GET_LEVEL(ch) < LVL_IMMORT) {
     IB_LevelStr(ch, size);

     IB_NeededExpPointsStr(ch, size);
  }

  IB_GoldStr(ch, size);

  /*  set these as "last known" values  */
  GET_LASTMANA(ch) = GET_MANA(ch);
  GET_LASTMAXMANA(ch) = GET_MAX_MANA(ch);
  GET_LASTHIT(ch) = GET_HIT(ch);
  GET_LASTMAXHIT(ch) = GET_MAX_HIT(ch);
  GET_LASTMOVE(ch) = GET_MOVE(ch);
  GET_LASTMAXMOVE(ch) = GET_MAX_MOVE(ch);
  GET_LASTEXP(ch) = GET_EXP(ch);
  GET_LASTGOLD(ch) = GET_GOLD(ch);

  /* Update all of the info parts */
  IB_HitPoints(ch, size);
  IB_MovePoints(ch, size);
  IB_ManaPoints(ch, size);
  IB_ExpPoints(ch, size);
  if (GET_LEVEL(ch) < LVL_IMMORT) {
     IB_NeededExpPoints(ch, size);
     IB_Level(ch, size);
  }
  IB_Gold(ch, size);


  /* set curser to top left corner */
  sprintf(buf, VT_CURSPOS, 0, 0);
  send_to_char(buf, ch);

}

void InfoBarOff(struct char_data *ch) {
  char buf[255];

  sprintf(buf, VT_MARGSET, 0, GET_SCREENSIZE(ch)- 1);
  send_to_char(buf, ch);
  send_to_char(VT_HOMECLR, ch);
}

void InfoBarUpdate(struct char_data *ch, int update) {
  char buf[255];
  int size;

  size = GET_SCREENSIZE(ch);

  if (size<=0)
     return;

  if(IS_SET(update, INFO_MANA)) {
     sprintf(buf, VT_CURSAVE);
     write_to_output(ch->desc, buf);;
     IB_ClearMana(ch, size);
     IB_W_ManaPoints(ch, size);
     sprintf(buf, VT_CURREST);
     write_to_output(ch->desc, buf);;
  }

  if(IS_SET(update, INFO_MOVE)) {
     sprintf(buf, VT_CURSAVE);
     write_to_output(ch->desc, buf);;
     IB_ClearMove(ch, size);
     IB_W_MovePoints(ch, size);
     sprintf(buf, VT_CURREST);
     write_to_output(ch->desc, buf);;
   }

  if(IS_SET(update, INFO_HIT)) {
     sprintf(buf, VT_CURSAVE);
     write_to_output(ch->desc, buf);;
     IB_ClearHit(ch, size);
     IB_W_HitPoints(ch, size);
     sprintf(buf, VT_CURREST);
     write_to_output(ch->desc, buf);;
   }

  if(IS_SET(update, INFO_EXP)) {
     sprintf(buf, VT_CURSAVE);
     write_to_output(ch->desc, buf);;
     IB_ClearExpPoints(ch, size);
     IB_W_ExpPoints(ch, size);
     if (GET_LEVEL(ch) < LVL_IMMORT) {
        IB_ClearLevel(ch, size);
        IB_W_Level(ch, size);
        IB_ClearNeededExpPoints(ch, size);
        IB_W_NeededExpPoints(ch, size);
     }
     sprintf(buf, VT_CURREST);
     write_to_output(ch->desc, buf);;
   }

  if(IS_SET(update, INFO_GOLD)) {
     sprintf(buf, VT_CURSAVE);
     write_to_output(ch->desc, buf);;
     IB_ClearGold(ch, size);
     IB_W_Gold(ch,size);
     sprintf(buf, VT_CURREST);
     write_to_output(ch->desc, buf);;
   }

}

void IB_Seperator(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 4, 1);
   send_to_char(buf, ch);
   sprintf(buf, "+-----+-----+-----+-----+-----+-----+-----");
   sprintf(buf, "%s+-----+-----+-----+-----+-----+", buf);
   send_to_char(buf, ch);

}

void IB_HitPointsStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 3, 1);
   send_to_char(buf, ch);
   sprintf(buf, "Hit Pts: ");
   send_to_char(buf, ch);

}

void IB_HitPoints(struct char_data *ch, int size) {

   char buf[256];
   int count, maxcount;
   float percent;

   count = GET_HIT(ch);
   maxcount = GET_MAX_HIT(ch);
   percent = (float)count / (float)maxcount;

   sprintf(buf, VT_CURSPOS, size - 3, 10);
   send_to_char(buf, ch);
   if (percent >= 0.95)
      sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else if (percent >= 0.33)
      sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else
      sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

}

void IB_W_HitPoints(struct char_data *ch, int size) {

   char buf[256];
   int count, maxcount;
   float percent;

   count = GET_HIT(ch);
   maxcount = GET_MAX_HIT(ch);
   percent = (float)count / (float)maxcount;

   sprintf(buf, VT_CURSPOS, size - 3, 10);
   write_to_output(ch->desc, buf);;
   if (percent >= 0.95)
      sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else if (percent >= 0.33)
      sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else
      sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   write_to_output(ch->desc, buf);;

}

void IB_ClearHit(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 3, 10);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "          ");
   write_to_output(ch->desc, buf);;

}

void IB_ManaPointsStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 3, 26);
   send_to_char(buf, ch);
   sprintf(buf, "Mana Pts: ");
   send_to_char(buf, ch);

}

void IB_ManaPoints(struct char_data *ch, int size) {

   char buf[256];
   int count, maxcount;
   float percent;

   count = GET_MANA(ch);
   maxcount = GET_MAX_MANA(ch);
   percent = (float)count / (float)maxcount;

   sprintf(buf, VT_CURSPOS, size - 3, 36);
   send_to_char(buf, ch);
   if (percent >= 0.95)
      sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else if (percent >= 0.33)
      sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else
      sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

}

void IB_W_ManaPoints(struct char_data *ch, int size) {

   char buf[256];
   int count, maxcount;
   float percent;

   count = GET_MANA(ch);
   maxcount = GET_MAX_MANA(ch);
   percent = (float)count / (float)maxcount;
   sprintf(buf, VT_CURSPOS, size - 3, 36);
   write_to_output(ch->desc, buf);;
   if (percent >= 0.95)
      sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else if (percent >= 0.33)
      sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else
      sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   write_to_output(ch->desc, buf);;

}

void IB_ClearMana(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 3, 36);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "          ");
   write_to_output(ch->desc, buf);;

}

void IB_MovePointsStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 3, 53);
   send_to_char(buf, ch);
   sprintf(buf, "Move Pts: ");
   send_to_char(buf, ch);

}

void IB_MovePoints(struct char_data *ch, int size) {

   char buf[256];
   int count, maxcount;
   float percent;

   count = GET_MOVE(ch);
   maxcount = GET_MAX_MOVE(ch);
   percent = (float)count / (float)maxcount;

   sprintf(buf, VT_CURSPOS, size - 3, 63);
   send_to_char(buf, ch);
   if (percent >= 0.95)
      sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else if (percent >= 0.33)
      sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else
      sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

}



void IB_W_MovePoints(struct char_data *ch, int size) {

   char buf[256];
   int count, maxcount;
   float percent;

   count = GET_MOVE(ch);
   maxcount = GET_MAX_MOVE(ch);
   percent = (float)count / (float)maxcount;

   sprintf(buf, VT_CURSPOS, size - 3, 63);
   write_to_output(ch->desc, buf);;
   if (percent >= 0.95)
      sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else if (percent >= 0.33)
      sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   else
      sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
              CCNRM(ch, C_NRM), CCGRN(ch, C_NRM),
              maxcount, CCNRM(ch, C_NRM));
   write_to_output(ch->desc, buf);;

}

void IB_ClearMove(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 3, 63);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "          ");
   write_to_output(ch->desc, buf);;

}

void IB_ExpPointsStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 1);
   send_to_char(buf, ch);
   sprintf(buf, "Exp: ");
   send_to_char(buf, ch);

}

void IB_ExpPoints(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 6);
   send_to_char(buf, ch);
   sprintf(buf, "%s%d%s", CCBLU(ch, C_NRM), GET_EXP(ch),
           CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

}

void IB_W_ExpPoints(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 6);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "%s%d%s", CCBLU(ch, C_NRM), GET_EXP(ch),
           CCNRM(ch, C_NRM));
   write_to_output(ch->desc, buf);;

}

void IB_ClearExpPoints(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 6);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "        ");
   write_to_output(ch->desc, buf);;

}

void IB_NeededExpPointsStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 26);
   send_to_char(buf, ch);
   sprintf(buf, "Needed for Level ");
   send_to_char(buf, ch);

}

void IB_NeededExpPoints(struct char_data *ch, int size) {

   char buf[256];
   int needed_exp = ( (exp_needed_for_level(ch))-GET_EXP(ch) );

   sprintf(buf, VT_CURSPOS, size - 2, 47);
   send_to_char(buf, ch);
   sprintf(buf, "%d", needed_exp);
   send_to_char(buf, ch);

}

void IB_W_NeededExpPoints(struct char_data *ch, int size) {

   char buf[256];
   int needed_exp = ( (exp_needed_for_level(ch))-GET_EXP(ch) );

   sprintf(buf, VT_CURSPOS, size - 2, 47);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "%d", needed_exp);
   write_to_output(ch->desc, buf);;

}

void IB_ClearNeededExpPoints(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 47);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "        ");
   write_to_output(ch->desc, buf);;

}

void IB_LevelStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 45);
   send_to_char(buf, ch);
   sprintf(buf, ": ");
   send_to_char(buf, ch);

}

void IB_Level(struct char_data *ch, int size) {

   char buf[256];
   int nextlevel = GET_LEVEL(ch) + 1;

   sprintf(buf, VT_CURSPOS, size - 2, 43);
   send_to_char(buf, ch);
   sprintf(buf, "%2d", nextlevel);
   send_to_char(buf, ch);

}

void IB_W_Level(struct char_data *ch, int size) {

   char buf[256];
   int nextlevel = GET_LEVEL(ch) + 1;

   sprintf(buf, VT_CURSPOS, size - 2, 43);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "%2d", nextlevel);
   write_to_output(ch->desc, buf);;

}

void IB_ClearLevel(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 2, 43);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "  ");
   write_to_output(ch->desc, buf);;

}

void IB_GoldStr(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 1, 1);
   send_to_char(buf, ch);
   sprintf(buf, "Gold: ");
   send_to_char(buf, ch);

}

void IB_Gold(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 1, 7);
   send_to_char(buf, ch);
   sprintf(buf, "%s%d%s",  CCMAG(ch, C_NRM), GET_GOLD(ch), CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

}

void IB_W_Gold(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 1, 7);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "%s%d%s",  CCMAG(ch, C_NRM), GET_GOLD(ch), CCNRM(ch, C_NRM));
   write_to_output(ch->desc, buf);;

}

void IB_ClearGold(struct char_data *ch, int size) {

   char buf[256];

   sprintf(buf, VT_CURSPOS, size - 1, 7);
   write_to_output(ch->desc, buf);;
   sprintf(buf, "           ");
   write_to_output(ch->desc, buf);;

}


