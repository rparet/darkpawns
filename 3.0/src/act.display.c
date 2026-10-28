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

/* $Id: act.display.c 1201 2005-05-08 12:24:41Z mcooper $ */

#include "conf.h"
#include "sysdep.h"

#include <sys/stat.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "vt100.h"

long exp_needed_for_level(struct char_data *ch);

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
void IB_UnassignedExpStr(struct char_data *ch, int size);
void IB_UnassignedExp(struct char_data *ch, int size);
void IB_W_UnassignedExp(struct char_data *ch, int size);
void IB_ClearUnassignedExp(struct char_data *ch, int size);
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
void IB_BankStr(struct char_data *ch, int size);
void IB_Bank(struct char_data *ch, int size);
void IB_W_Bank(struct char_data *ch, int size);
void IB_ClearBank(struct char_data *ch, int size);
void IB_TankStr(struct char_data *ch, int size);
void IB_Tank(struct char_data *ch, int size);
void IB_W_Tank(struct char_data *ch, int size);
void IB_ClearTank(struct char_data *ch, int size);


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

void InfoBarOn(struct char_data *ch)
{
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
  IB_UnassignedExpStr(ch, size);

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    IB_LevelStr(ch, size);
    IB_NeededExpPointsStr(ch, size);
  }
  
  IB_GoldStr(ch, size);
  IB_BankStr(ch, size);
  IB_TankStr(ch, size);
  
  /* Update all of the info parts */
  IB_HitPoints(ch, size);
  IB_MovePoints(ch, size);
  IB_ManaPoints(ch, size);
  IB_ExpPoints(ch, size);
  IB_UnassignedExp(ch, size);
  if (GET_LEVEL(ch) < LVL_IMMORT) {
    IB_NeededExpPoints(ch, size);
    IB_Level(ch, size);
  }
  IB_Gold(ch, size);
  IB_Bank(ch, size);
  IB_Tank(ch, size);

   /* set curser to top left corner */
  sprintf(buf, VT_CURSPOS, 0, 0);
  send_to_char(buf, ch);
}

void InfoBarOff(struct char_data *ch)
{
  char buf[255];
 
  sprintf(buf, VT_MARGSET, 0, GET_SCREENSIZE(ch)- 1);
  send_to_char(buf, ch);
  send_to_char(VT_HOMECLR, ch);
}

void InfoBarUpdate(struct char_data *ch, int update)
{
  char buf[255];
  int size;
 
  size = GET_SCREENSIZE(ch);
 
  if (size <= 0)
    return;
 
  if (GET_MANA(ch) < GET_MAX_MANA(ch)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearMana(ch, size);
    IB_W_ManaPoints(ch, size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (GET_MOVE(ch) < GET_MAX_MOVE(ch)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearMove(ch, size);
    IB_W_MovePoints(ch, size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearHit(ch, size);
    IB_W_HitPoints(ch, size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (IS_SET(update, INFO_EXP)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearExpPoints(ch, size);
    IB_W_ExpPoints(ch, size);
    if (GET_LEVEL(ch) < LVL_IMMORT) {
      IB_ClearLevel(ch, size);
      IB_W_Level(ch, size);
      IB_ClearNeededExpPoints(ch, size);
      IB_W_NeededExpPoints(ch, size);
    }
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (IS_SET(update, INFO_EXP_Q)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearUnassignedExp(ch, size);
    IB_W_UnassignedExp(ch, size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (IS_SET(update, INFO_GOLD)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearGold(ch, size);
    IB_W_Gold(ch,size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (IS_SET(update, INFO_BANK)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearBank(ch, size);
    IB_W_Bank(ch,size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }

  if (IS_SET(update, INFO_TANK)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    IB_ClearTank(ch, size);
    IB_W_Tank(ch,size);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }
}

void IB_Seperator(struct char_data *ch, int size)
{
  char buf[256];

  sprintf(buf, VT_CURSPOS, size - 4, 1);
  send_to_char(buf, ch);
  sprintf(buf, "+-----+-----+-----+-----+-----+-----+-----");
  sprintf(buf, "%s+-----+-----+-----+-----+-----+", buf);
  send_to_char(buf, ch);
}

void IB_HitPointsStr(struct char_data *ch, int size)
{
  char buf[256];

  sprintf(buf, VT_CURSPOS, size - 3, 1);
  send_to_char(buf, ch);
  sprintf(buf, "Hit Pts: ");
  send_to_char(buf, ch);
}

void IB_HitPoints(struct char_data *ch, int size)
{
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
       CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else if (percent >= 0.33)
     sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
       CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else
     sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
       CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}

void IB_W_HitPoints(struct char_data *ch, int size)
{
  char buf[256];
  int count, maxcount;
  float percent;
  
  count = GET_HIT(ch);
  maxcount = GET_MAX_HIT(ch);
  percent = (float)count / (float)maxcount; 

  sprintf(buf, VT_CURSPOS, size - 3, 10);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  if (percent >= 0.95)
     sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
       CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else if (percent >= 0.33)
     sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
       CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else
     sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
       CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearHit(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 3, 10);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "          ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ManaPointsStr(struct char_data *ch, int size)
{
  char buf[256];

  sprintf(buf, VT_CURSPOS, size - 3, 21);
  send_to_char(buf, ch);
  sprintf(buf, "Mana Pts: ");
  send_to_char(buf, ch);
}

void IB_ManaPoints(struct char_data *ch, int size)
{
  char buf[256];
  int count, maxcount;
  float percent;

  count = GET_MANA(ch);
  maxcount = GET_MAX_MANA(ch);
  percent = (float)count / (float)maxcount;
   
  sprintf(buf, VT_CURSPOS, size - 3, 31);
  send_to_char(buf, ch);
  if (percent >= 0.95)
    sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else if (percent >= 0.33)
    sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else
    sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}

void IB_W_ManaPoints(struct char_data *ch, int size)
{
  char buf[256];
  int count, maxcount;
  float percent;
   
  count = GET_MANA(ch);
  maxcount = GET_MAX_MANA(ch);
  percent = (float)count / (float)maxcount;
  sprintf(buf, VT_CURSPOS, size - 3, 31);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  if (percent >= 0.95)
    sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else if (percent >= 0.33)
    sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else
    sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearMana(struct char_data *ch, int size)
{
  char buf[256];
  
  sprintf(buf, VT_CURSPOS, size - 3, 31);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "          ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_MovePointsStr(struct char_data *ch, int size)
{
  char buf[256];

  sprintf(buf, VT_CURSPOS, size - 3, 43);
  send_to_char(buf, ch);
  sprintf(buf, "Move Pts: ");
  send_to_char(buf, ch);
}

void IB_MovePoints(struct char_data *ch, int size)
{
  char buf[256];
  int count, maxcount;
  float percent;
   
  count = GET_MOVE(ch);
  maxcount = GET_MAX_MOVE(ch);
  percent = (float)count / (float)maxcount;
   
  sprintf(buf, VT_CURSPOS, size - 3, 53);
  send_to_char(buf, ch);
  if (percent >= 0.95)
    sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else if (percent >= 0.33)
    sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else
    sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}



void IB_W_MovePoints(struct char_data *ch, int size)
{
  char buf[256];
  int count, maxcount;
  float percent;
   
  count = GET_MOVE(ch);
  maxcount = GET_MAX_MOVE(ch);
  percent = (float)count / (float)maxcount;
   
  sprintf(buf, VT_CURSPOS, size - 3, 53);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  if (percent >= 0.95)
    sprintf(buf, "%s%d%s(%s%d%s)", CCGRN(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else if (percent >= 0.33)
    sprintf(buf, "%s%d%s(%s%d%s)", CCYEL(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
  else
    sprintf(buf, "%s%d%s(%s%d%s)", CCRED(ch, C_NRM), count,
      CCNRM(ch, C_NRM), CCGRN(ch, C_NRM), maxcount, CCNRM(ch, C_NRM));
   write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearMove(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 3, 53);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "          ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ExpPointsStr(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 1);
  send_to_char(buf, ch);
  sprintf(buf, "Exp: ");
  send_to_char(buf, ch);
}

void IB_ExpPoints(struct char_data *ch, int size)
{
  char buf[256];
  
  sprintf(buf, VT_CURSPOS, size - 2, 6);
  send_to_char(buf, ch);
  sprintf(buf, "%s%ld%s", CCGRN(ch, C_NRM), GET_EXP(ch), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}

void IB_W_ExpPoints(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 6);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "%s%ld%s", CCGRN(ch, C_NRM), GET_EXP(ch), CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearExpPoints(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 6);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "        ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_UnassignedExpStr(struct char_data *ch, int size)
{
  char buf[256];

  sprintf(buf, VT_CURSPOS, size - 2, 21);
  send_to_char(buf, ch);
  sprintf(buf, "Unassigned Exp: ");
  send_to_char(buf, ch);
}

void IB_UnassignedExp(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 37);
  send_to_char(buf, ch);
  sprintf(buf, "%s%ld%s", CCGRN(ch, C_NRM), GET_EXP_Q(ch), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}
 
void IB_W_UnassignedExp(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 37);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "%s%ld%s", CCGRN(ch, C_NRM), GET_EXP_Q(ch), CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearUnassignedExp(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 37);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "        ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_NeededExpPointsStr(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 48);
  send_to_char(buf, ch);
  sprintf(buf, "For level ");
  send_to_char(buf, ch);
}

void IB_NeededExpPoints(struct char_data *ch, int size)
{
  char buf[256];
  long needed_exp = (exp_needed_for_level(ch) - GET_EXP(ch));
   
  sprintf(buf, VT_CURSPOS, size - 2, 63);
  send_to_char(buf, ch);
  sprintf(buf, "%s%ld%s", CCGRN(ch, C_NRM), needed_exp, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}

void IB_W_NeededExpPoints(struct char_data *ch, int size)
{
  char buf[256];
  long needed_exp = (exp_needed_for_level(ch) - GET_EXP(ch));
   
  sprintf(buf, VT_CURSPOS, size - 2, 63);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "%s%ld%s", CCGRN(ch, C_NRM), needed_exp, CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearNeededExpPoints(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 63);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "        ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_LevelStr(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 61);
  send_to_char(buf, ch);
  sprintf(buf, ": ");
  send_to_char(buf, ch);
}

void IB_Level(struct char_data *ch, int size)
{
  char buf[256];
  int nextlevel = GET_LEVEL(ch) + 1;
   
  sprintf(buf, VT_CURSPOS, size - 2, 58);
  send_to_char(buf, ch);
  sprintf(buf, "%d", nextlevel);
  send_to_char(buf, ch);
}

void IB_W_Level(struct char_data *ch, int size)
{
  char buf[256];
  int nextlevel = GET_LEVEL(ch) + 1;
   
  sprintf(buf, VT_CURSPOS, size - 2, 58);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "%3d", nextlevel);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearLevel(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 2, 58);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "  ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_GoldStr(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 1);
  send_to_char(buf, ch);
  sprintf(buf, "Gold: ");
  send_to_char(buf, ch);
}

void IB_Gold(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 7);
  send_to_char(buf, ch);
  sprintf(buf, "%s%d%s",  CCYEL(ch, C_NRM), GET_GOLD(ch), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}

void IB_W_Gold(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 7);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "%s%d%s",  CCYEL(ch, C_NRM), GET_GOLD(ch), CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearGold(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 7);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "           ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_BankStr(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 21);
  send_to_char(buf, ch);
  sprintf(buf, "Bank: ");
  send_to_char(buf, ch);
}

void IB_Bank(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 27);
  send_to_char(buf, ch);
  sprintf(buf, "%s%d%s",  CCYEL(ch, C_NRM), GET_BANK_GOLD(ch), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);
}

void IB_W_Bank(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 27);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "%s%d%s",  CCYEL(ch, C_NRM), GET_BANK_GOLD(ch), CCNRM(ch, C_NRM));
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_ClearBank(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 27);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "           ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

void IB_TankStr(struct char_data *ch, int size)
{
  char buf[256];

  sprintf(buf, VT_CURSPOS, size - 1, 43);
  send_to_char(buf, ch);
  sprintf(buf, "Tank: ");
  send_to_char(buf, ch);
}

void IB_Tank(struct char_data *ch, int size)
{
  char buf[256];
  char *status = NULL;
  extern char *tank_status(struct char_data *ch);

  if ((status = tank_status(ch))) {
    sprintf(buf, VT_CURSPOS, size - 1, 49);
    send_to_char(buf, ch);
    sprintf(buf, "%s%s%s", CCRED(ch, C_NRM), status, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }
}

void IB_W_Tank(struct char_data *ch, int size)
{
  char buf[256];
  char *status = NULL;
  extern char *tank_status(struct char_data *ch);
  
  if ((status = tank_status(ch))) {
    sprintf(buf, VT_CURSPOS, size - 1, 49);
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
    sprintf(buf, "%s%s%s", CCRED(ch, C_NRM), status, CCNRM(ch, C_NRM));
    write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  }
}

void IB_ClearTank(struct char_data *ch, int size)
{
  char buf[256];
   
  sprintf(buf, VT_CURSPOS, size - 1, 49);
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
  sprintf(buf, "                ");
  write_to_descriptor(ch->desc->descriptor, buf, ch->desc->comp);
}

