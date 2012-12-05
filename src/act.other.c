/* ************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
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

/* $Id: act.other.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include <sys/stat.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "house.h"
#include "ident.h"
#include "clan.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char *class_abbrevs[];
extern char *crowd_size[];
extern struct time_info_data time_info; 
extern char *dirs[];

/* extern procedures */
SPECIAL(shop_keeper);
void write_aliases(struct char_data *ch);
void write_poofs(struct char_data *ch);
ACMD(do_say);
int use_tattoo(struct char_data *ch);
void InfoBarOff(struct char_data *ch);
int num_followers(struct char_data *ch);
void improve_skill(struct char_data *ch, int skill);
bool is_owner(struct char_data *ch, int room_vnum);
char *delete_ansi_controls(char *string);


ACMD(do_quit)
{
  void die(struct char_data * ch);
  void Crash_rentsave(struct char_data * ch, int cost);
  void Crash_cryosave(struct char_data * ch, int cost);
  void  unmount(struct char_data *ch, struct char_data *mount);
  struct char_data *get_mount(struct char_data *ch);
  extern int free_rent;
  int isokquit = 0;
  struct descriptor_data *d, *next_d;
  struct room_data *rm = &world[ch->in_room];

  if (IS_NPC(ch) || !ch->desc)
    return;
  
  switch(rm->number) {
    case 8004:
     isokquit = 1;
     break;
    case 8008:
     isokquit = 1;
     break;
    case 18201:
     if (ch->player.hometown == 2)
       isokquit = 1;
     break;
    case 21202:
     if (ch->player.hometown == 3)
       isokquit = 1;
     break;
    case 21258:
     if (ch->player.hometown == 3)
       isokquit = 1;
     break;
    default:
     if (is_owner(ch, rm->number))
       isokquit = 1;
     else
      isokquit = 0;
     break;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT && (subcmd != SCMD_QUIT &&
      subcmd != SCMD_REALLY_QUIT))
    send_to_char("You have to type quit--no less, to quit!\r\n", ch);
  else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
  else if (GET_POS(ch) <= POS_INCAP) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch);
  }
  else if ((subcmd != SCMD_REALLY_QUIT) && 
           !(isokquit || GET_LEVEL(ch) >= LVL_IMMORT))
  {
     stc("Type REALLYQUIT to quit the game and lose your eq.\r\n", ch);
     stc("Return to the temple and QUIT to leave the game and keep"
          " your equipment.\r\n", ch);
     if (GET_LEVEL(ch) <= 5)
       stc("You can type RECALL to return to your temple.\r\n", ch);   
     return;
  } else {
    if (!GET_INVIS_LEV(ch))
      act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "%s has quit the game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);
 
    /* if they have there infobar on clear it out for them */
    if (GET_INFOBAR(ch) == INFOBAR_ON)
       InfoBarOff(ch);

    /*
     * kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping.
     */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (d == ch->desc)
        continue;
      if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
        close_socket(d);
    }

   if (free_rent)
   {
	/*
	 *  Free rent in temple, your house, or if immortal
	 */
	if (isokquit || GET_LEVEL(ch) >= LVL_IMMORT) 
        {
          if (GET_LEVEL(ch) <= LVL_IMMORT)
            GET_LOADROOM(ch) = rm->number; /* load players back where they legally quit */
          if (PLR_FLAGGED(ch, PLR_NODELETE))
            Crash_cryosave(ch, 0);
          else
            Crash_rentsave(ch, 0);
	} 
        else
	{
    	   sprintf(buf, "LOSTEQ:%s has quit out of a save room.", 
	          GET_NAME(ch));
	   mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);      
	}
    }
    if (IS_MOUNTED(ch))
      unmount(ch, get_mount(ch));

    extract_char(ch);		/* Char is saved in extract char */

  }
}



ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  if (cmd) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }
  write_aliases(ch);
  if (GET_LEVEL(ch)>=LVL_IMMORT)
    write_poofs(ch);
  save_char(ch, NOWHERE);
  Crash_crashsave(ch);

 /*  if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
    House_crashsave(world[ch->in_room].number); */
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}


ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);
  if (IS_AFFECTED(ch, AFF_SNEAK))
  {
    affect_from_char(ch, SKILL_SNEAK);
    affect_from_char(ch, SKILL_STEALTH);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_SNEAK);
  }

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}


ACMD(do_hide)
{
  byte percent;

  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  if (weather_info.sunlight != SUN_DARK)
  {
    switch(SECT(ch->in_room))
    {
     case SECT_FIELD:
      stc("Hide out here during the day? Yeah right.\r\n", ch);
      return;
     case SECT_DESERT:
      stc("You can't hide very well with all the sun and sand out here!\r\n", ch);
      return;
     case SECT_WATER_SWIM:
     case SECT_WATER_NOSWIM:
     case SECT_UNDERWATER:
     case SECT_WATER:
      stc("Hide in the water? Don't think so.\r\n", ch);  
      return;
     case SECT_FLYING:
     case SECT_FIRE:
     case SECT_EARTH:
     case SECT_WIND:
      stc("You are completely exposed here, nowhere to hide!\r\n", ch);
      return;
     default:
      break;
    }
  }
  if (subcmd)
    stc("You attempt to practice the art of kabuki.\r\n", ch);
  else
    stc("You attempt to hide yourself.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (subcmd)
  {
    if (percent > GET_SKILL(ch, SKILL_KABUKI) + dex_app_skill[GET_DEX(ch)].hide)
      return;
  }
  else if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
         return;

  SET_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
  if (subcmd)
    improve_skill(ch, SKILL_KABUKI);
  else
    improve_skill(ch, SKILL_HIDE);
}


ACMD(do_steal)
{
  struct char_data *vict;
  struct obj_data *obj;
  char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;
  struct master_affected_type af;
  ACMD(do_gen_comm);

  af.type = SKILL_STEAL;
  af.duration = 6;
  af.modifier = 0;
  af.location = APPLY_NONE;     
  af.bitvector = AFF_ROBBED;
  af.by_type = BY_SPELL;
  af.obj_num = 0;
    

  argument = one_argument(argument, obj_name);
  one_argument(argument, vict_name);

  if (!(vict = get_char_room_vis(ch, vict_name)))
    {
      send_to_char("Steal what from who?\r\n", ch);
      return;
    }
  else if (vict == ch)
    {
      send_to_char("Come on now, that's rather stupid!\r\n", ch);
      return;
    }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
      send_to_char("You can't contemplate stealing in such a place!\r\n", ch);
      return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (!IS_NPC(vict) && !IS_OUTLAW(ch) && !subcmd)
  {
    sprintf(buf, "You can not steal from %s because you are not an Outlaw!\r\n", 
            GET_NAME(vict));
    stc(buf, ch);
    sprintf(buf, "%s failed to steal from you because %s is not an Outlaw!\r\n",
            GET_NAME(ch), GET_NAME(ch));
    stc(buf, vict);
    return;
  }

  /* 101% is a complete failure */
  percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (GET_POS(vict) <= POS_SLEEPING && (!IS_NPC(vict) && 
      GET_LEVEL(vict) < LVL_IMMORT))
    percent = -1;		/* ALWAYS SUCCESS */

  if (!IS_NPC(vict))
    pcsteal = 1;


  if (!IS_NPC(vict) && GET_LEVEL(ch) < LVL_IMMORT)
   if ((GET_LEVEL(vict) > GET_LEVEL(ch) + 3) || (GET_LEVEL(vict) < GET_LEVEL(ch) - 3))
     percent = 101;  /* Failure */

  /* NO NO With Imp's and Shopkeepers, and if player thieving is not allowed */
  if (GET_LEVEL(vict) >= LVL_IMMORT ||  
      (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) ||
      GET_MOB_SPEC(vict) == shop_keeper ||
      IS_AFFECTED(vict, AFF_ROBBED))
    percent = 101;		/* Failure */

  if (GET_LEVEL(ch) > LVL_IMMORT && GET_LEVEL(vict) < GET_LEVEL(ch))
    percent = -1; /* success */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold"))
    {
      if (!(obj = get_obj_in_list_vis(ch, obj_name, vict->carrying)))
	{
	  for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	    if (GET_EQ(vict, eq_pos) &&
		(isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
		CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos)))
	      {
		obj = GET_EQ(vict, eq_pos);
		break;
	      }
	  if (!obj)
	    {
	      act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
	      return;
	    }
	  else
	    {			/* It is equipment */
	      if ((GET_POS(vict) > POS_SLEEPING))
		{
		  send_to_char("Steal the equipment now?  Impossible!\r\n",
			       ch);
		  return;
		}
	      else if (percent > 100) /* Failure */
	        {
                  act("You try to unequip $p, but fail!",
                      FALSE, ch, obj, vict, TO_CHAR);
                  act("$n tries to steal $p from $N, but fails!",
                      FALSE, ch, obj, vict, TO_NOTVICT);
		  update_pos(vict);
		  if (GET_POS(vict) != POS_SLEEPING)
                    ohoh = TRUE;
		}
	      else
		{
                  if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
                  {
                  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <
                      CAN_CARRY_W(ch)) {
                  obj_to_char(unequip_char(vict, eq_pos), ch);
                  if (!IS_NPC(vict))
                  {
                    sprintf(buf, "(PS) %s stole %s from %s.", GET_NAME(ch), obj->short_description,
                     GET_NAME(vict));
                    mudlog(buf, CMP, LVL_IMMORT, TRUE);
                    affect_join(vict, &af, af.duration, FALSE, FALSE, FALSE);
                  }
		  act("You unequip $p and steal it.",
		      FALSE, ch, obj, 0, TO_CHAR);
		  act("$n steals $p from $N.",
		      TRUE, ch, obj, vict, TO_NOTVICT);
                  improve_skill(ch, SKILL_STEAL);
                  }
                 }
                  else
                   stc("You cannot carry that much.\r\n", ch); 
		}
	    }
	}
      else
	{			/* obj found in inventory */
	  percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */
	  if (GET_LEVEL(vict)>GET_LEVEL(ch))
	    percent += GET_LEVEL(vict) - GET_LEVEL(ch);
	  if (percent > GET_SKILL(ch, SKILL_STEAL))
	    {
	      ohoh = TRUE;
	      act("$N catches you trying to steal something...", 
		  FALSE, ch, 0, vict, TO_CHAR);
	      act("$n tried to steal something from you!",
		  FALSE, ch, 0, vict, TO_VICT);
	      act("$n tries to steal something from $N.",
		  TRUE, ch, 0, vict, TO_NOTVICT);
	    }
	  else
	    {			/* Steal the item */
	      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
		{
		  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <
		      CAN_CARRY_W(ch))
		    {
		      obj_from_char(obj);
		      obj_to_char(obj, ch);
		      if (!IS_NPC(vict))
		      {
                        sprintf(buf, "(PS) %s stole %s from %s.", GET_NAME(ch), obj->short_description,
                         GET_NAME(vict));
                        mudlog(buf, CMP, LVL_IMMORT, TRUE);
                        affect_join(vict, &af, af.duration, FALSE, FALSE, FALSE);
                      }
		      if (!subcmd)
		        send_to_char("Got it!\r\n", ch);
		      else
			act("Somehow $p makes it's way into your pack.",
				TRUE, ch, obj, 0, TO_CHAR);
		      improve_skill(ch, SKILL_STEAL);
                    }
		}
	      else
		send_to_char("You cannot carry that much.\r\n", ch);
	    }
	}
    }
  else
    {			/* Steal some coins */
      if (percent > GET_SKILL(ch, SKILL_STEAL))
	{
	  ohoh = TRUE;
	  act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	  act("You discover that $n has $s hands in your wallet.",
	      FALSE, ch, 0, vict, TO_VICT);
	  act("$n tries to steal gold from $N.",
	      TRUE, ch, 0, vict, TO_NOTVICT);
	}
      else
	{
	  /* Steal some gold coins */
	  gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
	  gold = MIN(1782, gold);
	  if (gold > 0)
	    {
	      GET_GOLD(ch) += gold;
	      GET_GOLD(vict) -= gold;
	      if (gold > 1)
		{
		  sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
		  send_to_char(buf, ch);
		  improve_skill(ch, SKILL_STEAL);
                }
	      else 
		send_to_char("You manage to swipe a solitary "
			     "gold coin.\r\n", ch);
	    }
	  else
	    send_to_char("You couldn't get any gold...\r\n", ch);
	}
    }

  if (ohoh && IS_NPC(vict) && AWAKE(vict))
    hit(vict, ch, TYPE_UNDEFINED);
  
  if (ohoh && !IS_NPC(vict) && !subcmd)
  {
    SET_BIT_AR(PLR_FLAGS(ch), PLR_OUTLAW);
    sprintf(buf, "(PS) %s unsuccessfuly tried to steal from %s.", GET_NAME(ch), GET_NAME(vict));
    mudlog(buf, CMP, LVL_IMMORT, TRUE);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);   
}



ACMD(do_practice)
{
  void list_skills(struct char_data * ch);

  one_argument(argument, arg);

  if (*arg)
    send_to_char("You can only practice skills in your guild.\r\n", ch);
  else
    list_skills(ch);
}



ACMD(do_visible)
{
  void appear(struct char_data * ch);
  void perform_immort_vis(struct char_data *ch);
  int altered = FALSE;

  if (affected_by_spell(ch, SKILL_KK_ZAI))
    {
      stc("You cannot become visible until your zai ends!\r\n", ch);
      return;
    }

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    {
      perform_immort_vis(ch);
      return;
    }

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    {
      appear(ch);
      send_to_char("You fade into view.\r\n", ch);
      altered = TRUE;
    }
  if (IS_AFFECTED(ch, AFF_SNEAK))
    {
      stc("You stop sneaking.\r\n", ch);
      affect_from_char(ch, SKILL_SNEAK);
      affect_from_char(ch, SKILL_STEALTH);
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_SNEAK);
      altered = TRUE;
    }
  if (!altered)
    send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);
  delete_ansi_controls(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- "
		 "you shouldn't have abused it!\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_TITLE_LENGTH)
    {
      sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
	      MAX_TITLE_LENGTH);
      send_to_char(buf, ch);
    }
  else
    {
      set_title(ch, argument);
      sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
      send_to_char(buf, ch);
    }
}


int
perform_group(struct char_data *ch, struct char_data *vict)
{
  if (IS_AFFECTED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return 0;

  SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
  if (ch != vict)
    act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
  act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
  return 1;
}


void
print_group(struct char_data *ch)
{
  struct char_data *k;
  struct follow_type *f;

  if (!IS_AFFECTED(ch, AFF_GROUP))
    send_to_char("But you are not the member of a group!\r\n", ch);
  else {
    send_to_char("Your group consists of:\r\n", ch);

    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP))
      {
	sprintf(buf, "     [%s%3d%sH %s%3d%sM %s%3d%sV] [%2d %s%s%s] $N "
		     "(Head of group)",
		CCGRN(ch, C_CMP), GET_HIT(k), CCNRM(ch, C_CMP),
		CCGRN(ch, C_CMP), GET_MANA(k), CCNRM(ch, C_CMP),
		CCGRN(ch, C_CMP), GET_MOVE(k), CCNRM(ch, C_CMP),
		GET_LEVEL(k), 
		CCCYN(ch, C_CMP),CLASS_ABBR(k), CCNRM(ch, C_CMP));
	act(buf, FALSE, ch, 0, k, TO_CHAR);
      }

    for (f = k->followers; f; f = f->next)
      {
	if (!IS_AFFECTED(f->follower, AFF_GROUP))
	  continue;

        if (IS_NPC(f->follower))  /* checking out mob stats via group is bad */
          sprintf(buf, "     [---H ---M ---V] [-- --] $N");
        else
	  sprintf(buf, "     [%s%3d%sH %s%3d%sM %s%3d%sV] [%2d %s%s%s] $N", 
		CCGRN(ch, C_CMP), GET_HIT(f->follower), CCNRM(ch, C_CMP),
		CCGRN(ch, C_CMP), GET_MANA(f->follower), CCNRM(ch, C_CMP),
		CCGRN(ch, C_CMP), GET_MOVE(f->follower), CCNRM(ch, C_CMP),
		GET_LEVEL(f->follower), 
		CCGRN(ch, C_CMP), CLASS_ABBR(f->follower), CCNRM(ch, C_CMP));

	act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
      }
  }
}



ACMD(do_group)
{
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf)
    {
      print_group(ch);
      return;
    }

  if (ch->master)
    {
      act("You can not enroll group members without being head of a group.",
	  FALSE, ch, 0, 0, TO_CHAR);
      return;
    }

  if (!str_cmp(buf, "all"))
    {
      perform_group(ch, ch);
      for (found = 0, f = ch->followers; f; f = f->next)
        if (f->follower->in_room == ch->in_room &&
	    !IS_SHADOWING(f->follower))
	  found += perform_group(ch, f->follower);
      if (!found)
	send_to_char("Everyone following you here is already in "
		     "your group.\r\n", ch);
      return;
    }

  if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (((vict->master != ch) && (vict != ch)) || IS_SHADOWING(vict))
    act("$N must follow you to enter your group.",
	FALSE, ch, 0, vict, TO_CHAR);
  else
    {
      if (!IS_AFFECTED(vict, AFF_GROUP))
	perform_group(ch, vict);
      else
	{
	  if (ch != vict)
	    act("$N is no longer a member of your group.",
		FALSE, ch, 0, vict, TO_CHAR);
	  act("You have been kicked out of $n's group!",
	      FALSE, ch, 0, vict, TO_VICT);
	  act("$N has been kicked out of $n's group!",
	      FALSE, ch, 0, vict, TO_NOTVICT);
	  REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
	}
    }
}



ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  struct char_data *tch;
  void stop_follower(struct char_data * ch);

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }
    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
	send_to_char(buf2, f->follower);
        if (!IS_AFFECTED(f->follower, AFF_CHARM))
	  stop_follower(f->follower);
      }
    }

    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    send_to_char("You disband the group.\r\n", ch);
    return;
  }
  if (!(tch = get_char_room_vis(ch, buf))) {
    send_to_char("There is no such person!\r\n", ch);
    return;
  }
  if (tch->master != ch) {
    send_to_char("That person is not following you!\r\n", ch);
    return;
  }

  if (!IS_AFFECTED(tch, AFF_GROUP)) {
    send_to_char("That person isn't in your group.\r\n", ch);
    return;
  }

  REMOVE_BIT_AR(AFF_FLAGS(tch), AFF_GROUP);

  act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
 
  if (!IS_AFFECTED(tch, AFF_CHARM))
    stop_follower(tch);
}




ACMD(do_report)
{
  struct char_data *tch = NULL;
  act ("$n reports:", FALSE, ch, 0,0, TO_ROOM);
  send_to_char("You report:\r\n", ch);
  for(tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
  {
    sprintf(buf, "    [%s%d%s/%s%d%s]H [%s%d%s/%s%d%s]M [%s%d%s/%s%d%s]V"
	         " [%s%d%s]Kills [%s%d%s]PKs [%s%d%s]Deaths",
         CCGRN(tch, C_NRM),GET_HIT(ch),CCNRM(tch, C_NRM), 
         CCGRN(tch, C_NRM),GET_MAX_HIT(ch),CCNRM(tch, C_NRM), 
         CCGRN(tch, C_NRM),GET_MANA(ch),CCNRM(tch, C_NRM), 
         CCGRN(tch, C_NRM),GET_MAX_MANA(ch),CCNRM(tch, C_NRM), 
         CCGRN(tch, C_NRM),GET_MOVE(ch),CCNRM(tch, C_NRM), 
         CCGRN(tch, C_NRM),GET_MAX_MOVE(ch),CCNRM(tch, C_NRM), 
         CCRED(tch, C_NRM),(int)GET_KILLS(ch),CCNRM(tch, C_NRM), 
         CCRED(tch, C_NRM),(int)GET_PKS(ch),CCNRM(tch, C_NRM), 
         CCRED(tch, C_NRM),(int)GET_DEATHS(ch),CCNRM(tch, C_NRM) );
     act (buf, FALSE, tch, 0, 0, TO_CHAR);
  }
}



ACMD(do_split)
{
  int amount, num, share;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room))
	num++;

    if (num && IS_AFFECTED(ch, AFF_GROUP))
      share = amount / num;
    else {
      send_to_char("With whom do you wish to share your gold?\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k)) && k != ch) {
      GET_GOLD(k) += share;
      sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	      amount, share);
      send_to_char(buf, k);
    }
    for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch) {
	GET_GOLD(f->follower) += share;
	sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
		amount, share);
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}



ACMD(do_use)
{
  struct obj_data *mag_item;
  int equipped = 1;
  int eq_pos = 0;

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      equipped = 0;
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      if (isname(arg, "tattoo"))
      {
 	use_tattoo(ch);
	return;
      }
      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (GET_EQ(ch, eq_pos) &&
	    (isname(arg, GET_EQ(ch, eq_pos)->name)) &&
	    CAN_SEE_OBJ(ch, GET_EQ(ch, eq_pos))) 
	  mag_item = GET_EQ(ch, eq_pos);
      if (!mag_item)
      {	
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    /* wearable wands Serapis 140896
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
    */
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_use");
      return;
      break;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char("You can only recite scrolls.", ch);
      return;
    }
    break;
  case SCMD_USE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char("You can't seem to figure out how to use it.\r\n"
		   "Try holding it.(?)\r\n", ch);
      return;
    }
    break;
  }

  mag_objectmagic(ch, mag_item, buf);
}



ACMD(do_wimpy)
{
  int wimp_lev;

  one_argument(argument, arg);

  if (!*arg) {
    if (GET_WIMP_LEV(ch)) {
      sprintf(buf, "Your current wimp level is %d hit points.\r\n",
	      GET_WIMP_LEV(ch));
      send_to_char(buf, ch);
      return;
    } else {
      send_to_char("At the moment, you're not a wimp. (sure, sure...)\r\n", ch);
      return;
    }
  }
  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg))) {
      if (wimp_lev < 0)
	send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
      else if (wimp_lev > GET_MAX_HIT(ch))
	send_to_char("That doesn't make much sense, now does it?\r\n", ch);
      else if (wimp_lev > (GET_MAX_HIT(ch)/3))
	stc("You can't set your wimp level above one third "
		"your hit points.\r\n", ch);
      else {
	sprintf(buf, "Okay, you'll wimp out if you drop below %d "
		"hit points.\r\n", wimp_lev);
	send_to_char(buf, ch);
	GET_WIMP_LEV(ch) = wimp_lev;
      }
    } else {
      send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", 
		ch);
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char("Specify at how many hit points you want to wimp out at.  "
		"(0 to disable)\r\n", ch);

  return;

}


ACMD(do_display)
{
  size_t i;

  if (IS_NPC(ch))
    {
      send_to_char("Monsters don't need displays.  Go away.\r\n", ch);
      return;
    }
  skip_spaces(&argument);

  if (!*argument)
    {
      send_to_char("Usage: prompt { H | M | V | T | F | all | none }\r\n", ch);
      return;
    }
  if ((!str_cmp(argument, "on")) || (!str_cmp(argument, "all")))
  { 
      SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
      SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
      SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
      SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
      SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTARGET);
  } else {
      REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
      REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
      REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
      REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
      REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPTARGET);
      if (!str_cmp(argument, "off"))
	return;
      for (i = 0; i < strlen(argument); i++)
	switch (LOWER(argument[i]))
	  {
	  case 'h':
	    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
	    break;
	  case 'f':
	    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTARGET);
	    break;
	  case 'm':
	    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
	    break;
	  case 't':
	    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
	    break;
	  case 'v':
	    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	    break;
	  }
    }
  send_to_char(OK, ch);
}


ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, *filename, buf[MAX_STRING_LENGTH];
  int r;
  time_t ct;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  case SCMD_TODO:
    filename = TODO_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
      send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
      return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)  {
      send_to_char("That must be a mistake...\r\n", ch);
      return;
  }
  
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(buf, CMP, LVL_IMMORT, FALSE);

  if (!(fl = fopen(filename, "a"))) {
    perror("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  
  r = fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
              world[ch->in_room].number, argument);
  if (r < 0)
    log("Error writing to file %s.", filename);  

  fclose(fl);
  
  send_to_char("Okay.  Thanks!\r\n", ch);
}


#define TOG_OFF 0
#define TOG_ON  1

ACMD(do_gen_tog)
{
  long result;
  extern int nameserver_is_slow;

  char *tog_messages[][2] = {
    {"You are now safe from summoning by other players.\r\n",
    "You may now be summoned by other players.\r\n"},
    {"Nohassle disabled.\r\n",
    "Nohassle enabled.\r\n"},
    {"Brief mode off.\r\n",
    "Brief mode on.\r\n"},
    {"Compact mode off.\r\n",
    "Compact mode on.\r\n"},
    {"You can now hear tells.\r\n",
    "You are now deaf to tells.\r\n"},
    {"You can now hear auctions.\r\n",
    "You are now deaf to auctions.\r\n"},
    {"You can now hear shouts.\r\n",
    "You are now deaf to shouts.\r\n"},
    {"You can now hear gossip.\r\n",
    "You are now deaf to gossip.\r\n"},
    {"You can now hear the congratulation messages.\r\n",
    "You are now deaf to the congratulation messages.\r\n"},
    {"You can now hear the Wiz-channel.\r\n",
    "You are now deaf to the Wiz-channel.\r\n"},
    {"You are no longer part of the Quest.\r\n",
    "Okay, you are part of the Quest!\r\n"},
    {"You will no longer see the room flags.\r\n",
    "You will now see the room flags.\r\n"},
    {"You will now have your communication repeated.\r\n",
    "You will no longer have your communication repeated.\r\n"},
    {"HolyLight mode off.\r\n",
    "HolyLight mode on.\r\n"},
    {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
    "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
    {"Autoexits disabled.\r\n",
    "Autoexits enabled.\r\n"},
    {"Ident changed to NO;  remote username lookups will not be attempted.\r\n",
     "Ident changed to YES;  remote usernames lookups will be attempted.\r\n"},
    {"Newbie channel on.\r\n",
     "Newbie channel off.\r\n"},
    {"Clan tells are now on.\r\n",
     "Clan tells are now off.\r\n"},
    {"Broadcast channel is now on.\r\n",
     "Broadcast channel is now off.\r\n"}
  };


  if (IS_NPC(ch))
    return;

  if (subcmd == SCMD_NOCTELL && !GET_CLAN(ch))
    {
     stc("You aren't even in a clan!\r\n", ch);
     return;
    }

  if (subcmd == SCMD_NOWIZ && (GET_LEVEL(ch) < LVL_IMMORT) 
      && !PLR_FLAGGED(ch, PLR_CHOSEN))
    {
      stc("Huh?!?\r\n", ch);
      return;
    }

  switch (subcmd) {
  case SCMD_NOSUMMON:
    result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    break;
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    break;
  case SCMD_NOAUCTION:
    result = PRF_TOG_CHK(ch, PRF_NOAUCT);
    break;
  case SCMD_DEAF:
    result = PRF_TOG_CHK(ch, PRF_DEAF);
    break;
  case SCMD_NOGOSSIP:
    result = PRF_TOG_CHK(ch, PRF_NOGOSS);
    break;
  case SCMD_NOGRATZ:
    result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    break;
  case SCMD_QUEST:
    result = PRF_TOG_CHK(ch, PRF_QUEST);
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    break;
  case SCMD_SLOWNS:
    result = (nameserver_is_slow = !nameserver_is_slow);
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
    break;
  case SCMD_IDENT:
    result = (ident = !ident);
    break;
  case SCMD_NONEWBIE:
    result = PRF_TOG_CHK(ch, PRF_NONEWBIE);
    break;
  case SCMD_NOCTELL:
    result = PRF_TOG_CHK(ch, PRF_NOCTELL);
    break;
  case SCMD_NOBROAD:
    result = PRF_TOG_CHK(ch, PRF_NOBROAD);
    break;
  default:
    log("SYSERR: Unknown subcmd in do_gen_toggle");
    return;
    break;
  }

  if (result)
    send_to_char(tog_messages[subcmd][TOG_ON], ch);
  else
    send_to_char(tog_messages[subcmd][TOG_OFF], ch);

  return;
}

ACMD(do_afk)
{
   if (PRF_FLAGGED(ch, PRF_AFK))
   {
      REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AFK);
      send_to_char("You return from the world of the living.\r\n", ch);
      act("$n returns from some repulsive act...", TRUE, ch, 0,0, TO_ROOM);
   }
   else
   {
      SET_BIT_AR(PRF_FLAGS(ch), PRF_AFK);
      send_to_char("Go leave..no one will notice anyways.\r\n", ch);
      act("$n goes AFK...", TRUE, ch, 0, 0, TO_ROOM);
   }
   return;
}

ACMD(do_auto)
{

    int autotags = FALSE ;

    if (!*argument)
    {

       sprintf(buf, "You have the following autos set:\r\n") ;
       if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
       {
          sprintf(buf, "%sExits ", buf) ;
          autotags = TRUE ;
       }
       if (PRF_FLAGGED(ch, PRF_AUTOLOOT))
       {
          sprintf(buf, "%sLoot ", buf) ;
          autotags = TRUE ;
       }
       if (PRF_FLAGGED(ch, PRF_AUTOGOLD))
       {
          sprintf(buf, "%sGold ", buf) ;
          autotags = TRUE ;
       }
       if (PRF_FLAGGED(ch, PRF_AUTOSPLIT))
       {
          sprintf(buf, "%sSplit", buf) ;
          autotags = TRUE ;
       }

       if(autotags)
         sprintf(buf, "%s\r\n", buf) ;
       else
         sprintf(buf, "%sNone.\r\n", buf) ;

       send_to_char (buf, ch) ;

    }
    else
    {
       skip_spaces(&argument) ;

       if ( ( !strcmp(argument, "exit") ) || (!strcmp(argument, "exits")) )
       {
          if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
          {
             REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AUTOEXIT);
             send_to_char("You will no longer see room exits.\r\n", ch);
          }
          else
          {
             SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOEXIT);
             send_to_char("You will now see room exits.\r\n", ch);
          }

          return ;
       }

       if (!strcmp(argument, "loot"))
       {
          if (PRF_FLAGGED(ch, PRF_AUTOLOOT))
          {
             REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AUTOLOOT);
             send_to_char("You will no longer loot corpses.\r\n", ch);
          }
          else
          {
             SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOLOOT);
             send_to_char("You will now automatically loot corpses.\r\n", ch);
          }

          return ;
       }

       if (!strcmp(argument, "gold"))
       {
          if (PRF_FLAGGED(ch, PRF_AUTOGOLD))
          {
             REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AUTOGOLD);
             send_to_char("You will no longer get the gold from corpses.\r\n",
               ch);
          }
          else
          {
             SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOGOLD);
             send_to_char("You will now get the gold from corpses.\r\n", ch);
          }

          return ;
       }

       if (!strcmp(argument, "split"))
       {
          if (PRF_FLAGGED(ch, PRF_AUTOSPLIT))
          {
             REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AUTOSPLIT);
             send_to_char("You will no longer split gold with your group.\r\n",
               ch);
          }
          else
          {
             SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOSPLIT);
             send_to_char("You will now split gold with your group.\r\n", ch);
          }

          return ;
       }

       send_to_char("What do you want to make automatic?\r\n", ch) ;

    }

    return ;
}

ACMD(do_transform)
{
   if (!PLR_FLAGGED(ch, PLR_WEREWOLF) && !PLR_FLAGGED(ch, PLR_VAMPIRE)){
	send_to_char("You aren't transformable!\n\r", ch);
	return;
   }
   if (PLR_FLAGGED(ch, PLR_WEREWOLF)) {
	if ( weather_info.sunlight == SUN_SET || 
	     weather_info.sunlight == SUN_DARK ) {
		if (IS_AFFECTED(ch, AFF_WEREWOLF))
		{
		send_to_char("You can't change back until morning!\n\r", ch);
		return;
		}
		else if (time_info.day<6 && time_info.day >=1)
		{
		stc("You can't transform when there's no moon in the sky!\r\n",
			ch);
		return;
		}
		else
		{
		send_to_char("Your nails grow into talons, and hair sprouts"
			" from every pore.\n\r", ch);
		act("$n shivers and transforms into a werewolf!",
			FALSE, ch, 0,0, TO_ROOM);
		SET_BIT_AR(AFF_FLAGS(ch), AFF_WEREWOLF);        
		switch(time_info.moon)
		  {
		  case MOON_NEW:
		    GET_HIT(ch) += GET_MAX_HIT(ch)/6;
		    break;
		  case MOON_QUARTER_FULL:
		  case MOON_THREE_EMPTY:
		    GET_HIT(ch) += GET_MAX_HIT(ch)/5;
		    break;
		  case MOON_HALF_FULL:
		  case MOON_HALF_EMPTY:
		    GET_HIT(ch) += GET_MAX_HIT(ch)/4;
		    break;
		  case MOON_THREE_FULL:
		  case MOON_QUARTER_EMPTY:
		    GET_HIT(ch) += GET_MAX_HIT(ch)/3;
		    break;
		  case MOON_FULL:
		    GET_HIT(ch) += GET_MAX_HIT(ch)/2;
		    break;
		  default:
                    break;
		  }
                if (GET_HIT(ch) > 666) 
                  GET_HIT(ch) = 666;
		return;
		}
	}
	else
	{
	  if (IS_AFFECTED(ch, AFF_WEREWOLF)){
		send_to_char("Your hair and nails shorten and you revert to"
			" your normal shape.\n\r", ch);
		act("$n shivers and transforms out of werewolf form!", 
			FALSE, ch, 0,0, TO_ROOM);
		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_WEREWOLF);         
		if (GET_HIT(ch) > GET_MAX_HIT(ch))
			GET_HIT(ch) = GET_MAX_HIT(ch);
		return;
	  }
	  else{
		send_to_char("You can't transform during the day!\n\r", ch);
		return;
	  }
	}
   }
   
   if (PLR_FLAGGED(ch, PLR_VAMPIRE)) {
	if ( weather_info.sunlight == SUN_SET || 
	     weather_info.sunlight == SUN_DARK ) {
		if (IS_AFFECTED(ch, AFF_VAMPIRE))
		{
		send_to_char("You can't change back until morning!\n\r", ch);
		return;
		}
		else
		{
		send_to_char("Your nails grow transluscent and fangs sprout"
			" from your incisors!\n\r", ch);
		act("$n shivers and transforms into a vampire!",
			FALSE, ch, 0,0, TO_ROOM);
		SET_BIT_AR(AFF_FLAGS(ch), AFF_VAMPIRE);        
		switch(time_info.moon) {
			case MOON_NEW:
				GET_MANA(ch) += GET_MAX_MANA(ch)/5;
				break;
		  	case MOON_HALF_FULL:
		  	case MOON_HALF_EMPTY:
				GET_MANA(ch) += GET_MAX_MANA(ch)/4;
				break;
			case MOON_THREE_FULL:
			case MOON_QUARTER_EMPTY:
				GET_MANA(ch) += GET_MAX_MANA(ch)/3;
				break;
			case MOON_FULL:
				GET_MANA(ch) += GET_MAX_MANA(ch)/2;
				break;
			default:
                        break;
		}
		return;
		}
	}
	else
	{
	  if (IS_AFFECTED(ch, AFF_VAMPIRE)){
		send_to_char("Your fangs recess, and you revert to"
			" your normal shape.\n\r", ch);
		act("$n shivers and transforms out of vampire form!", 
			FALSE, ch, 0,0, TO_ROOM);
		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_VAMPIRE);         
		if(GET_MANA(ch) > GET_MAX_MANA(ch))
			GET_MANA(ch) = GET_MAX_MANA(ch);
		return;
	  }
	  else{
		send_to_char("You can't transform during the day!\n\r", ch);
		return;
	  }
	}
   }
}

ACMD(do_ride)
{
  struct char_data *mount;
  void add_follower_quiet(struct char_data *ch, struct char_data *leader);

  if (FIGHTING(ch))
    {
      send_to_char("You're too busy fighting!\n\r", ch);
      return;
    }
  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS)) {
     send_to_char("Go outside if you want to ride!\r\n", ch);
     return;
  }

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("What do you wish to ride?\r\n", ch);
  else if (!(mount = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (mount == ch)
    send_to_char("That's disgusting!\r\n", ch);
  else
    {
      if(IS_MOUNTED(ch))
	send_to_char("You can't ride two beasts at once!\r\n", ch);
      else if (IS_MOUNTED(mount))
	send_to_char("The beast is already being ridden!\r\n", ch);
      else if(!IS_MOUNTABLE(mount))
	act("You can't ride $N!", TRUE, ch, 0, mount, TO_CHAR);
      else if (!CAN_MOUNT(ch))
	send_to_char("You can't ride!\r\n", ch);
      else if (IS_AFFECTED(ch, AFF_CHARM))
	send_to_char("Get your master's permission first!\r\n", ch);
      else if ( IS_AFFECTED(mount, AFF_CHARM) && (mount->master != ch) )
	act("$S master would not like that!", TRUE, ch, 0, mount, TO_CHAR);
      else
	{
	  SET_BIT_AR(AFF_FLAGS(ch), AFF_MOUNT);        
	  SET_BIT_AR(AFF_FLAGS(mount), AFF_MOUNT);
	  if (!IS_AFFECTED(mount, AFF_CHARM))
	    SET_BIT_AR(AFF_FLAGS(mount), AFF_CHARM);
	  if ( !mount->master || mount->master != ch )
	    add_follower_quiet(mount, ch);
	  send_to_char("You hop on your mount.\r\n", ch);
	  act("$n hops on your back!", TRUE, ch, 0, mount, TO_VICT);
	  act("$n hops onto the back of $N.", TRUE, ch, 0, mount, TO_ROOM);
	}
    }
}

ACMD(do_dismount)
{
  struct char_data *mount = NULL;

  struct char_data *get_mount(struct char_data *ch);
  void unmount(struct char_data *rider, struct char_data *mount);

  if(!IS_MOUNTED(ch))
    send_to_char("You need to be riding before you can dismount!\r\n", ch);
  else
    {
      send_to_char("You hop off your mount.\r\n", ch);
      mount = get_mount(ch);
      if (mount)
      {
        act ("$n dismounts from the back of $N.",
	   TRUE, ch, 0, mount, TO_ROOM);
        send_to_char("Your rider dismounts, whew!\r\n", mount);
      }
      unmount(ch, get_mount(ch));
     }
}

ACMD(do_yank)
{
  ACMD(do_stand);

  struct char_data *victim = NULL;
  
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Who do you wish to yank?\r\n", ch);
  else if (!(victim = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("That's wierd.\r\n", ch);
  else
    {
	if (victim->master != ch)
	{
		send_to_char("That probably wouldn't be appreciated.\r\n", ch);
		return;
	}
	if (GET_POS(victim) > POS_SITTING)
	{
	     if (!IS_MOUNTED(victim))
		act("$N is already on $S feet.", TRUE, ch, 0, victim, TO_CHAR);
	     else
		act("You can't yank $M off $S mount!", 
			TRUE, ch, 0, victim, TO_CHAR);
	     return;
	}
	if (GET_POS(victim) <= POS_SLEEPING)
	{
		act("$N is is no position to be yanked around!", 
			TRUE, ch, 0, victim, TO_CHAR);
		return;
	}
	act("You yank $M to $S feet.", TRUE, ch, 0, victim, TO_CHAR);
	act("$n yanks you to your feet.", TRUE, ch, 0, victim, TO_VICT);
	act("$n yanks $N to $S feet.", TRUE, ch, 0, victim, TO_NOTVICT);
	GET_POS(victim) = POS_STANDING;

    }
}


ACMD(do_peek)
{
  void look_at_char(struct char_data *i, struct char_data *ch);
  ACMD(do_look);

  struct char_data *victim;
  byte percent;

  if (GET_CLASS(ch) != CLASS_THIEF && GET_CLASS(ch) != CLASS_ASSASSIN && 
	 GET_LEVEL(ch) < LEVEL_IMMORT)
    {
      send_to_char("You're not a thief!\r\n", ch);
      return;
    }
  
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to peek at?\r\n", ch);
  else if (!(victim = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Try the 'inventory' command!\r\n", ch);
  else
    {
      percent = number(1, 101);	/* 101% is a complete failure */
      if ( (percent > GET_SKILL(ch, SKILL_PEEK) &&
	    (GET_LEVEL(ch) < LEVEL_IMMORT)) )
	{
	  do_look(ch, argument, 0, 0);
	  return;
	}

      look_at_char(victim, ch);
      improve_skill(ch, SKILL_PEEK);  
  }
}

void 
improve_skill(struct char_data *ch, int skill)
{
  extern char *spells[];
  int percent, newpercent;
  char skillbuf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
	return;
  percent = GET_SKILL(ch, skill);
  if (number(1, 200) > GET_WIS(ch) + GET_INT(ch))
     return;
  if (percent >= 97 || percent <= 0)
     return;
  newpercent = number(1, 3);
  percent += newpercent;
  SET_SKILL(ch, skill, percent);
  if (newpercent == 3) {
     sprintf(skillbuf, "Your skill in %s improves.\r\n", spells[skill]);
     send_to_char(skillbuf, ch);
  }
}


ACMD(do_recall)
{
  void unmount(struct char_data *rider, struct char_data *mount);
  struct char_data *get_rider(struct char_data *mount);
  struct char_data *get_mount(struct char_data *rider);

  if (GET_LEVEL(ch)>5 || IS_NPC(ch)) {
	send_to_char("This command is not available for someone of your "
		"experience!\r\n", ch);
	return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_BFR)) {
      send_to_char("You can't recall from this magickal place.\r\n", ch);
      return;
   }
  if (FIGHTING(ch))
  {
	send_to_char("Your concentration is broken by your fighting!", ch);
	return;
  }
  spell_recall(30, ch, ch, NULL, NULL);
}


ACMD(do_stealth)
{
  struct affected_type af;
  byte percent;

  if (IS_MOUNTED(ch))
  {
    stc("Dismount first!\r\n", ch);
    return;
  }

  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);
  if (IS_AFFECTED(ch, AFF_SNEAK))
  {
    affect_from_char(ch, SKILL_SNEAK);
    affect_from_char(ch, SKILL_STEALTH);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_SNEAK);
  }

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_STEALTH) +dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_STEALTH;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}


ACMD(do_appraise)
{
  byte percent = number(1, 101);	/* 101% is a complete failure */
  struct obj_data *object;
  long cost;
  
  half_chop(argument, arg, buf);
  
  if (!*arg)
    {
      send_to_char("What do you want to appraise?\r\n", ch);
      return;
    }

  object = get_obj_in_list_vis(ch, arg, ch->carrying);
  if (!object)
    {
      send_to_char("You don't seem to have one of those...\r\n", ch);
      return;
    }

  cost = GET_OBJ_COST(object);
  
  if (percent > GET_SKILL(ch, SKILL_APPRAISE)) /* fail, give a bad value */
    cost += number( -cost, cost*2 );
  else 
  {
    cost += number( cost>20?-20:0, 20 );
    improve_skill(ch, SKILL_APPRAISE);
  }
  sprintf(buf, "You estimate it's worth %ld gold coins.\r\n", cost);
  send_to_char(buf, ch);
}

ACMD(do_inactive)
{
   if (PRF_FLAGGED(ch, PRF_INACTIVE)) 
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_INACTIVE);
   else
	SET_BIT_AR(PRF_FLAGS(ch), PRF_INACTIVE);
}

ACMD(do_scout)
{
  int dir, items=0, groups=0;
  char *terrain;
  struct char_data *people;
  struct obj_data *obj;
  one_argument(argument, arg);

  if (!*arg)
  {
    stc("Scout where?\r\n", ch);
    return;
  }
  if ((dir = search_block(arg, dirs, FALSE)) < 0)
  {
    stc("Scout in which direction?\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_SCOUT))
  {
    stc("You have no idea how!\r\n", ch);
    return;
  }
  if (!OUTSIDE(ch))
  {
    stc("You can only do this outdoors.\r\n", ch);
    return;
  }
  if (!EXIT(ch, dir))
  {
    stc("There is nothing of interest there.\r\n", ch);
    return;
  }
  
  switch (SECT(EXIT(ch,dir)->to_room))
  {
    case SECT_CITY:
       terrain = "the cobblestones of a city";
       break;
    case SECT_FIELD:
       terrain = "a wide swath of field";
       break;
    case SECT_FOREST:
       terrain = "the dense forest";
       break;
    case SECT_HILLS:
       terrain = "high hills";
       break;
    case SECT_MOUNTAIN:
       terrain = "jagged mountains";
       break;
    case SECT_WATER_SWIM:
    case SECT_WATER_NOSWIM:
       terrain = "a large stretch of water";
       break;
    case SECT_UNDERWATER:
        terrain = "the watery depths";
       break;
    case SECT_FLYING:
       terrain = "thin air";
       break;
    case SECT_DESERT:
       terrain = "a vast wasteland";
       break;
    case SECT_INSIDE:
       terrain = "the inside of a structure";
       break;
    case SECT_SWAMP:
       terrain = "a murky swamp";
       break;
    default:
       terrain = "the endless elemental plane";
       break;
  }
  sprintf(buf, "You see %s to the %s.\r\n", terrain, dirs[dir]);
  stc(buf, ch);
  if (IS_DARK(EXIT(ch, dir)->to_room))
    stc("It looks pretty dark there.\r\n", ch);
  if (ROOM_FLAGGED(EXIT(ch,dir)->to_room, ROOM_DEATH))
    stc("You sense that it is not safe to travel there.\r\n", ch);
  if (ROOM_FLAGGED(EXIT(ch,dir)->to_room, ROOM_TUNNEL))
    stc("It looks like a very narrow passage.\r\n",ch);

  for (obj = world[EXIT(ch,dir)->to_room].contents; obj;
       obj=obj->next_content)
     items++;
  for (people = world[EXIT(ch,dir)->to_room].people;
       people;people=people->next_in_room)
     groups++;

  if (world[EXIT(ch,dir)->to_room].contents)
    stc("It looks like there is something on the ground there.\r\n", ch);
  if (world[EXIT(ch,dir)->to_room].people) {
    sprintf(buf,"It looks like there is %s over there.\r\n",
             crowd_size[groups]);
    stc(buf,ch);
  }

}


ACMD(do_roll)
{
  unsigned int max_roll, result;
 
  one_argument(argument, arg);
  if (!*arg) {
    max_roll = 100;
  } else {
    max_roll = atoi(arg);
    if (!max_roll)
      max_roll = 100;
  }

  result = number(1, max_roll);
 
  sprintf(buf, "You roll %u (1-%u).", result, max_roll);
  act(buf, 0, ch, 0, 0, TO_CHAR);

  sprintf(buf, "With a toss of the dice, $n rolls %u (1-%u).", result, max_roll);
  act(buf, 0, ch, 0, 0, TO_ROOM);
}
