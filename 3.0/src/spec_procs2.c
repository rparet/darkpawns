/*************************************************************************
*   File: Spec_procs2.c                                 Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
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

/* $Id: spec_procs2.c 1253 2005-12-03 00:57:46Z mcooper $ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include <stdio.h>

/*   external vars  */
extern int mini_mud;
extern struct house_control_rec house_control[MAX_HOUSES];

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
int isname_with_abbrevs(char *str, char *namelist);
int find_house(long vnum);  /* house.c */
ACMD(do_tell);
ACMD(do_say);
ACMD(do_look);
SPECIAL(shop_keeper);
ACMD(do_steal);
void tattoo_af(struct char_data *ch, bool add);           
struct char_data *get_mount(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);
void unmount(struct char_data *rider, struct char_data *mount);
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);


struct social_type {
  char *cmd;
  int next_line;
};


/* Normal Red and Black checkers */
SPECIAL(normal_checker)
{
  struct char_data *i;
  char buf[256];

  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch))
    return (FALSE);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    {
      if (!IS_NPC(i) && GET_LEVEL(i) < LVL_IMMORT)
	{
	  act("$n sees $N and jumps quite high!", TRUE, ch, NULL, i,TO_NOTVICT);
	  sprintf(buf, "%s sees you and jumps high, right at you!\r\n",
		  GET_NAME(ch));
	  buf[0] = UPPER(buf[0]);
	  send_to_char(buf, i);
	  hit(ch,i,TYPE_UNDEFINED);
	  return(TRUE);
	}
    }
  return(FALSE);
}


SPECIAL(ninelives)
{
  struct char_data *i;
  struct char_data *mobile = (struct char_data *) me;    

  if ((!AWAKE(ch) && GET_HIT(ch)>0))
    return (FALSE);

  if (cmd)
    {
      skip_spaces(&argument);

      if (!*argument)
	return(FALSE);
      if (!strstr(argument, "chest"))
	return(FALSE);      	
      if (CMD_IS("open") || CMD_IS("look") || CMD_IS("examine"))
	{ 
	  if(!IS_NPC(ch) && CAN_SEE(mobile, ch)&&!PRF_FLAGGED(ch,PRF_NOHASSLE))
	    hit(mobile, ch, TYPE_UNDEFINED);
	  return(TRUE);
	}
      else
	return(FALSE);	
    }
    
  if (!FIGHTING(ch) || GET_HIT(ch) > 0)
    return (FALSE);
  /* MAX_MOVE is used to set the number of lives */
  if ( GET_MAX_MOVE(ch) > 0 )
    {
      if (GET_MAX_MOVE(ch) >8)
	GET_MAX_MOVE(ch) = 8;
      else
	GET_MAX_MOVE(ch)--;

	GET_HIT(ch) = GET_MAX_HIT(ch);

      if (FIGHTING(ch))
	{
	  i = FIGHTING(ch);
	  act("$n rises from the dead and keeps fighting!\n\n",
	      TRUE, ch, NULL, i, TO_ROOM);
	  if (FIGHTING(i) == ch)
	    stop_fighting(i);
	  stop_fighting(ch);        
	  hit(ch, i, TYPE_UNDEFINED);
	}
      return(TRUE);
    }	
  return(FALSE);
}


SPECIAL(whirlpool)
{
  struct char_data *vict, *tmp;
  int spec_occured = FALSE, to_room = 0;
  struct char_data *mobile = (struct char_data *)me;
  
  if (mini_mud || !ch) 
    return FALSE;

  for (vict = world[ch->in_room].people; vict; vict = tmp)
    {
      tmp = vict->next_in_room;
      if (!PRF_FLAGGED(vict,PRF_NOHASSLE) && vict !=mobile && !IS_NPC(vict))
	{
	  do
	    {
	      to_room = number(real_room(4600), real_room(4699));
	    }  
	  while( (IS_SET(world[to_room].room_flags,ROOM_PRIVATE)) ||
		 (IS_SET(world[to_room].room_flags,ROOM_GODROOM)) ||
		 (IS_SET(world[to_room].room_flags,ROOM_DEATH))   ||
		 (IS_SET(world[to_room].room_flags,ROOM_NOMOB)));
	  char_from_room(vict);
	  char_to_room(vict, to_room);
	  send_to_char("A ravaging whirlpool sucks you under!\n\r", vict);
	  send_to_char("You finally surface, sputtering...\n\r\n\r", vict);
	  look_at_room(vict, 0);
	  spec_occured = TRUE;
	}
    }
  return(spec_occured); 
}


#define MIMIC_ROOM_VNUM 5798
/********************************************************************
  MIMIC -the mob- must load in room MIMIC_ROOM_VNUM
  ********************************************************************/
SPECIAL (couch)
{
  struct char_data *mimic;
  struct obj_data *obj;
  
  if (!argument || mini_mud)
    return(FALSE);
  skip_spaces(&argument);
  if (!( (CMD_IS("look") || CMD_IS("examine")) && !strcmp(argument, "couch")) )
    return(FALSE);
  
  for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj) && (!strcmp(obj->name, "couch")))
      {
	obj_from_room(obj);
	extract_obj(obj);
	if ((mimic = world[real_room(MIMIC_ROOM_VNUM)].people))
	  {
	    char_from_room(mimic);
	    char_to_room(mimic, ch->in_room);
	    act("Starved and needing food to make more pillows, the mimic "
		"attacks!\n\r",TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("Starved and needing food to make more pillows, the "
		"mimic attacks you!\n\r\n\r",ch);
	    hit(mimic, ch, TYPE_UNDEFINED);
	    return(TRUE);
	  }
      }
  return(FALSE);
  
}
  
SPECIAL(stableboy)
{
  struct char_data *mobile = (struct char_data *) me;  
  struct char_data *mount = NULL;
  char msg[256];
  int vnum;

  if (CMD_IS("list") || CMD_IS("buy")) {
    if (!CAN_SEE(ch, mobile)) {
      send_to_char("Who's there??\r\n", ch);
      return (TRUE);
    }
    sprintf(buf, "%s", CMD_NAME);
    vnum = run_script(ch, mobile, NULL, &world[ch->in_room], buf, "code", LT_MOB);

    if (vnum) {                 /* Script will return mount vnum based on location */
      mount = read_mobile(vnum, VIRTUAL, NULL);

      if (!mount) {             /* Missing mount - problem! */
        mudlog("SYSERR: Stable script returned unknown mount VNUM.", BRF, LVL_IMMORT, TRUE);
        sprintf(msg, "%s Hmm, it appears I'm all out of mounts. I think you better speak "
          "to one of the Gods.", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }

      GET_EXP(mount) = 0;
      SET_BIT(AFF_FLAGS(mount), AFF_CHARM);
      IS_CARRYING_W(mount) = 1000;
      IS_CARRYING_N(mount) = 100;
      GET_MOVE(mount) = 230;
      GET_MAX_MOVE(mount) = 230;
      GET_MOB_TIMER(mount) = GET_LEVEL(mount) * 2;	/* How long before mount leaves */

      char_to_room(mount, ch->in_room);
      act("$N brings $n up from the stables out back.", FALSE, mount, NULL, mobile, TO_ROOM);
      add_follower(mount, ch);
    }

    return(TRUE);
  } else if (CMD_IS("stable")) {
    if (GET_MOUNT_NUM(ch)) {
      send_to_char("You may only stable one mount at a time within the realm.\r\n", ch);
      return (TRUE);
    }
    sprintf(buf, "%s", CMD_NAME);
    vnum = run_script(ch, mobile, NULL, &world[ch->in_room], buf, "code", LT_MOB);

    if (vnum) {                   /* Script will return vnum of the mount in the room */
      for (mount = world[ch->in_room].people; mount; mount = mount->next_in_room)
        if (GET_MOB_VNUM(mount) == vnum)
          break;

      if (!mount) {               /* Missing mount - problem! */
        sprintf(buf, "SYSERR: Unable to locate mount in the same room as %s.", GET_NAME(ch));
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        sprintf(msg, "%s Hmm, it appears the stables are full right now, best come back "
          "later.", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }

      stop_follower(mount);
      MOUNT_TIME(ch) = (long)time(0);
      GET_MOUNT_NUM(ch) = vnum;
      GET_STABLE(ch) = GET_ROOM_VNUM(ch->in_room);
      act("$N takes $n out back to the stables.", FALSE, mount, NULL, mobile, TO_ROOM);
      extract_char(mount);
    }

    return (TRUE);
  } else if (CMD_IS("collect")) {
    if (GET_STABLE(ch) != GET_ROOM_VNUM(ch->in_room)) {
      sprintf(msg, "%s I'm sorry, your name isn't on our books. Perhaps you stabled "
        "your mount somewhere else?", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }

    sprintf(buf, "%s mount %d time %ld", CMD_NAME, GET_MOUNT_NUM(ch),
      (long)time(0) - MOUNT_TIME(ch));

    vnum = run_script(ch, mobile, NULL, &world[ch->in_room], buf, "code", LT_MOB);

    if (vnum) {
      mount = read_mobile(vnum, VIRTUAL, NULL);

      if (!mount) {             /* Missing mount - problem! */
        mudlog("SYSERR: Stable script returned unknown stabled mount VNUM.",
          BRF, LVL_IMMORT, TRUE);
        sprintf(msg, "%s Hmm, it appears your mount isn't ready, come back later.",
          GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }

      GET_MOUNT_NUM(ch) = 0;
      GET_STABLE(ch) = 0;
      MOUNT_TIME(ch) = 0;
      GET_EXP(mount) = 0;
      SET_BIT(AFF_FLAGS(mount), AFF_CHARM);
      IS_CARRYING_W(mount) = 1000; 
      IS_CARRYING_N(mount) = 100;
      GET_MOVE(mount) = 230;
      GET_MAX_MOVE(mount) = 230;
      GET_MOB_TIMER(mount) = GET_LEVEL(mount) * 2;    /* How long before the mount leaves */

      char_to_room(mount, ch->in_room);
      act("$N brings $n up from the stables out back.", FALSE, mount, NULL, mobile, TO_ROOM);
      add_follower(mount, ch);
    }

    return (TRUE);
  }

  return(FALSE);
}

SPECIAL(tipster)
{
  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch))
    return (FALSE);

  switch (number (0, 50))
    {
    case 0:
      do_say(ch, "For ansi color, type COLOR COMPLETE!", 0, 0); break;
    case 1:
      do_say(ch, "Wargs make a tasty meal if you CARVE their corpse.",
	     0, 0); break;
    case 2:
      do_say(ch, "AUTO EXIT will show you the exits for every room.",
	     0, 0); break;
    case 3:
      do_say(ch, "It's always safest to quit in the temple.", 0, 0);
      break;
    case 4:
      do_say(ch, "You're allowed to play up to 3 characters at once.",
	     0, 0); break;
    case 5:
      do_say(ch, "You can hire a mercenary by giving him 100 coins.",
	     0, 0); break;
    case 6:
      do_say(ch, "A bribe of a couple hundred coins will make a guard"
	     " look the other way while you fight.", 0, 0);
      do_say(ch, "Guards will attack you if you fight in front of "
	     "them.", 0, 0); break; 
    case 7:
      do_say(ch, "Use the CONSIDER command!", 0, 0); break;
    case 8:
      do_say(ch, "If you don't like something, use the IDEA command."
	     , 0, 0); break;
    case 9:
      do_say(ch, "If you see something out of place, use the BUG "
	     "command.", 0, 0); break;
    case 10:
      do_say(ch, "If you see something spelled incorrectly, use "
	     "the TYPO command.", 0, 0); break;
    case 11:
      do_say(ch, "Use an identify scroll on yourself to see "
	     "your numerical statistics. (Available at your "
	     "local magick shop.)", 0, 0);
      break;
    case 12:
      do_say(ch, "Check out HELP ALIAS to see how to abbreviate "
	     "commands or do multiple commands at once.", 0, 0);
      break;
    default:
      break;
    }
  return FALSE;
}

SPECIAL(pissedalchemist)
{
  struct char_data *i = NULL;
  bool cont = FALSE;

  if (cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  if (FIGHTING(ch))
    return (FALSE);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (!IS_NPC(i) && GET_LEVEL(i) < LEVEL_IMMORT)
      cont = TRUE;

  if (!cont)
    return(FALSE);

  if (GET_ALIGNMENT(ch) != -69) /*arbitrary number, just a number that 
				  he is NOT*/
    {
      GET_ALIGNMENT(ch) = -69;
      do_say(ch, "So, finally come to steal my secret!?!", 0, 0);
      do_say(ch, "The secret of the Philosopher's Stone!?!", 0, 0); 
      return(FALSE);
    }
  do_say(ch, "Well -- Ya can't have it!! Die!!!", 0, 0);

  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (!IS_NPC(i))
      {
	hit(ch, i, TYPE_UNDEFINED);
	return(TRUE);
      }
  for (i=world[ch->in_room].people; i; i=i->next_in_room)
    if (!IS_NPC(i))
      {
	hit(ch, i, TYPE_UNDEFINED);
	return(TRUE);
      }
  
  SET_BIT(MOB_FLAGS(ch), MOB_AGGRESSIVE);

  return(FALSE);
}


void
kender_steal(struct char_data *ch, struct char_data *victim)
{
  struct obj_data *tmp_obj = NULL;

  if (!IS_NPC(victim))
    return;
  
  for (tmp_obj = victim->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 600) < GET_LEVEL(ch)))
	{
	  int percent = number(1, 101);
	  int pcsteal = 0;
	  
	  if (GET_POS(victim) < POS_SLEEPING)
	    percent = -1; /* Success */

	  if (!IS_NPC(victim))
	    pcsteal = 1;

	  /* NO NO With Imp's and Shopkeepers,
	     and if player thieving is not allowed */
	  if (GET_LEVEL(victim) >= LVL_IMMORT ||  
	      (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) ||
	      GET_MOB_SPEC(victim) == shop_keeper)
	    percent = 101; /* Failure */
          
          /* newbie protection */
          if ((GET_LEVEL(ch) <= 10) || (GET_LEVEL(victim) <= 10))
            return;

	  if (GET_LEVEL(ch) > LVL_IMMORT && GET_LEVEL(victim) < GET_LEVEL(ch))
	    percent = -1;  /* Success */

	  if ( (!IS_NPC(victim)) && (percent < GET_SKILL(ch, SKILL_STEAL)) )
	    {
	      obj_from_char(tmp_obj);
	      obj_to_char(tmp_obj, ch);
              sprintf(buf, "%s kender stole %s from %s.\r\n", GET_NAME(ch),
               tmp_obj->name, GET_NAME(victim));
              mudlog(buf, BRF, LVL_IMMORT, TRUE);
	    }
	  else if (GET_LEVEL(ch) > 5)
	    {
	      char tmp_obj_name[80], tmp_mob_name[80];
	      one_argument((tmp_obj)->name, tmp_obj_name);
	      one_argument(victim->player.name, tmp_mob_name);
	      sprintf(buf, "%s %s", tmp_obj_name, tmp_mob_name);
	      do_steal(ch, buf, 0, 1);
	    }
	}
    }
}

/* for remorter spec */
#define REMORT_COST 60000


SPECIAL(remorter)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[80];
  int i;

  if (IS_NPC(ch))
    return(FALSE);
  
  skip_spaces(&argument);
  
  if (CMD_IS("buy") || CMD_IS("list"))
   {
      sprintf (msg, "%s Type REMORT to remort!", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }
  if (!(CMD_IS("remort") || CMD_IS("buy")))
    return(FALSE);

      
  if (GET_LEVEL(ch) < LEVEL_IMMORT-1)
    {
      sprintf (msg, "%s You can't remort until level %d!\r\n",
		   GET_NAME(ch), LEVEL_IMMORT-1);
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }

  if (GET_LEVEL(ch) >= LEVEL_IMMORT)
    {
      sprintf(msg, "%s Immortals cannot remort!\r\n", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return TRUE;
    }

  if (GET_GOLD(ch) < REMORT_COST)
    {
	  sprintf (msg, "%s It costs %d gold to work my magicks.",
		   GET_NAME(ch), REMORT_COST);
	  do_tell(mobile, msg, find_command("tell"), 0);
	  return (TRUE); 
    }
 for (i = 0; i < NUM_WEARS; i++) 
    if (GET_EQ(ch, i))
      {
        send_to_char("You must come unto me naked, wearing only thy old "
			"body.\r\n", ch);
	return(TRUE);
      }
      
 if (PLR_FLAGGED(ch, PLR_IT))
   REMOVE_BIT(PLR_FLAGS(ch), PLR_IT);
 if (PLR_FLAGGED(ch, PLR_VAMPIRE))
   {
     REMOVE_BIT(PLR_FLAGS(ch), PLR_VAMPIRE);
     if (IS_AFFECTED(ch, AFF_VAMPIRE))
       REMOVE_BIT(AFF_FLAGS(ch), AFF_VAMPIRE);
   }
 if (PLR_FLAGGED(ch, PLR_WEREWOLF))
   {
     REMOVE_BIT(PLR_FLAGS(ch), PLR_WEREWOLF);
     if (IS_AFFECTED(ch, AFF_WEREWOLF))
       REMOVE_BIT(AFF_FLAGS(ch), AFF_WEREWOLF);
   }
 if (ch->affected)
   while (ch->affected)
	affect_remove(ch, ch->affected, TRUE);

      GET_GOLD(ch) -= REMORT_COST;
      GET_LEVEL(ch) = 1;
      GET_EXP(ch) = 1;
      GET_WIMP_LEV(ch) = 0;

      ch->points.max_hit = number(30,40);
      ch->points.max_mana = 100+(number(20,30));
      
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);

      tattoo_af(ch, FALSE);  /* remove nasty tattoo affs */
      vitae_af(ch, FALSE);

      /* this pair of functions controls remort stat adjusts and
         anti-hunger, anti-thirst  */

      SET_BIT(PLR_FLAGS(ch), PLR_REMORT);  /* new remort flag */
 
/*      for (i = 1; i <= MAX_SKILLS; i++)
	SET_SKILL(ch, i, 0);

      if (GET_RACE(ch) == RACE_KENDER)
	SET_SKILL(ch, SKILL_STEAL, 45); */

      tattoo_af(ch, TRUE);   /* add tattoo affects back in */
      vitae_af(ch, TRUE);
      affect_total(ch);      /* and total */

      mudlog("Due to remorting:", BRF, LVL_IMMORT, TRUE);
      advance_level(ch);
      sprintf (msg, "%s Enjoy your new life...",
	       GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      send_to_char("The remorter moves his hands over your eyes, closing them"
		   " with a touch.\r\nColors spiral in your sight... You open"
		   " your eyes, feeling refreshed.\r\n", ch);
      return(TRUE);
}

#define ASSASSIN_PRICE(as) (GET_LEVEL(as) * 1000)
SPECIAL(assassin)
{
  int as_loc;
  struct char_data *as, *vict;

  as_loc = ch->in_room + 1;

  if (CMD_IS("list"))
    {
      send_to_char("To hire an assassin: hire <assassin> <victim>\r\n", ch);
      send_to_char("Available assassins are:\r\n", ch);
      for (as = world[as_loc].people;as;as = as->next_in_room)
	{
	  sprintf(buf, "%8d - %s\r\n", ASSASSIN_PRICE(as), GET_NAME(as));
	  send_to_char(buf, ch);
	}
      return (TRUE);
    }
  else if (CMD_IS("hire"))
    {
      two_arguments(argument, buf, buf2);
      if(!*buf)
	{
	  send_to_char("Hire who?\r\n", ch);
	  return (TRUE);
	}
      if (!(as = get_char_room(buf, as_loc)))
	{
	  send_to_char("There is nobody called that!\r\n", ch);
	  return (TRUE);
	}
      if(!IS_NPC(as))
	{
	  send_to_char("GET THE HELL OUT OF THAT ROOM, NOW !!!\r\n", as);
	  sprintf(buf, "%s is in the assassin store room.", GET_NAME(as));
	  mudlog(buf, BRF, LVL_IMMORT, TRUE);
	  send_to_char("You can't hire players.\r\n", ch);
	  return (TRUE);
	}
      if (!*buf2)
	{
	  send_to_char("Whom do you want to assassinate?\r\n", ch);
	  return (TRUE);
	}
      if (GET_GOLD(ch) < ASSASSIN_PRICE(as))
	{
	  send_to_char("You don't have enough gold!\r\n", ch);
	  return 1;
	}
      if ((vict = get_player_vis(as, buf2, 0)))
	{
	  if (GET_LEVEL(vict) < 5)
	    {
	      send_to_char("We cannot lower ourselves to such easy prey.\r\n",
			   ch);
	      return(TRUE);
	    }
	  GET_GOLD(ch) -= ASSASSIN_PRICE(as);
	  as = read_mobile(GET_MOB_RNUM(as), REAL, NULL);
	  char_to_room(as, ch->in_room);
	  if(!MOB_FLAGGED(as,MOB_HUNTER))
	    SET_BIT(MOB_FLAGS(as),MOB_HUNTER);
	  set_hunting(as, vict);
	  send_to_char("We cannot contact you if the job succeeds or "
		       "not...security, you know.\r\n", ch);
	  act("$n hires $N for a job.", FALSE, ch, 0, as, TO_ROOM);
	  sprintf(buf, "%s hires %s to kill %s.\r\n", GET_NAME(ch),
		  GET_NAME(as), GET_NAME(vict));
	  mudlog(buf, BRF, LVL_IMMORT, TRUE);
	  return (TRUE);
	}
      else
	{
	  send_to_char("Our underground doesn't know the whereabouts of "
		       "the victim!\r\n", ch);
	  return (TRUE);
	}
    }
  return (FALSE);
}

SPECIAL(tattoo)
{
  struct char_data *artist = (struct char_data *)me;

  if (CMD_IS("buy")) {
    sprintf(buf, "%s%s", CMD_NAME, argument);
    if (run_script(ch, artist, NULL, &world[artist->in_room], buf, "code", LT_MOB)) {
      GET_POS(ch) = POS_STUNNED;
      update_pos(ch);           
      tattoo_af(ch, TRUE);
      affect_total(ch);
    }
    return (TRUE);
  }

  return (FALSE);
}

SPECIAL(evillead)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *obj = NULL;
  char msg[160];
  if (CMD_IS("give"))
  {
    if (!*argument || mini_mud)
        return(FALSE);
    two_arguments(argument, buf, buf2);
    if ( !*buf || !*buf2 || !isname(buf2, mobile->player.name))
        return(FALSE);
    if (!obj)
        return(FALSE);
    if (GET_OBJ_VNUM(obj) != 13100) /* slice of cheese */
    {
     sprintf (msg, "%s Don't give me that crap.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    if (GET_ALIGNMENT(ch)>0)
    {
     sprintf (msg, "%s Get lost, goody-goody.", GET_NAME(ch));
     do_tell(mobile, msg, find_command("tell"), 0);
     return(TRUE);
    }
    obj_from_char(obj);
    obj_to_char(obj, mobile);
    act("$n leads $N through a hidden trapdoor and returns a moment later,"
	" alone.\r\n", TRUE, mobile, 0, ch, TO_ROOM);
    send_to_char("  You follow Swiss though a small trapdoor into a maze of"
		 " twists and\r\nturns, winding through the various tunnels"
		 " as if he's been doing it all his\r\nlife. As you near a"
		 " narrow, well lit passage, Swiss turns around and "
		 "vanishes\r\nin a puff of smoke!\r\n", ch);
    char_from_room(ch);
    char_to_room(ch, real_room(13195));
    if (get_mount(ch))
      {
        char_from_room(get_mount(ch));
        char_to_room(get_mount(ch), real_room(13195));
      }
    look_at_room(ch, 0);
    return(TRUE);
  }
  return(FALSE);
}


SPECIAL(little_boy)
{
  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0 || FIGHTING(ch))
    return FALSE;
     
  if (!number(0,10))
    {
      act("$n exclaims, 'Don't let the bad man hurt me, mommy!", FALSE, ch,
          NULL, NULL, TO_ROOM);
      return TRUE;
    }
  return FALSE;
} 
   
SPECIAL(ira)
{
  struct char_data *tmp_ch = NULL;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0 || FIGHTING(ch))
    return FALSE;

  for ( tmp_ch = world[ch->in_room].people; tmp_ch;
	tmp_ch = tmp_ch->next_in_room)
    {
      if ( !IS_NPC(tmp_ch) && !PRF_FLAGGED(tmp_ch, PRF_NOHASSLE))
	{
	  act("$n exclaims, 'Dirty Orange Scum!'", FALSE, ch, NULL, NULL, 
	      TO_ROOM);
	  hit(ch, tmp_ch, TYPE_UNDEFINED);
	  return TRUE;
	}
    }

  return FALSE;
} 

void loud_mobs(struct char_data * ch)
{
  struct char_data *tmp_ch;
  int door, was_in;

  if ((!GET_NOISE(ch) && !GET_SOUND(ch)) || ch->in_room == NOWHERE || (number(0,20) > 0))
    return;

  was_in = ch->in_room;

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      if (GET_NOISE(ch))
        act(GET_NOISE(ch), FALSE, ch, 0, 0, TO_ROOM);
      if (GET_SOUND(ch))
        for (tmp_ch = world[ch->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
          if (IS_MSP(tmp_ch)) {
            sprintf(buf, "!!SOUND(Mobs/%s T=Mobs)", GET_SOUND(ch));
            send_to_char(buf, tmp_ch);
          }
      ch->in_room = was_in;
    }
  }
}

void
npc_kender_steal(struct char_data *ch, struct char_data *victim)
{
  struct obj_data *tmp_obj = NULL;

  for (tmp_obj = victim->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
    {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 60) < GET_LEVEL(ch)))
	{
	  int percent = number(1, 101);
	  
	  if (GET_POS(victim) < POS_SLEEPING)
	    percent = -1; /* Success */

	  if (GET_LEVEL(victim) >= LVL_IMMORT || 
	      (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)))
	    percent = 101; /* Failure */

	  if (GET_LEVEL(ch) > LVL_IMMORT && GET_LEVEL(victim) < GET_LEVEL(ch))
	    percent = -1;  /* Success */

	  if ( (!IS_NPC(victim)) && (percent < number(50,100)) )
	    {
	      obj_from_char(tmp_obj);
	      obj_to_char(tmp_obj, ch);
	      return;
	    }
	}
    }
}

SPECIAL(portal_room)
{
   int location;
   ACMD(do_look);
   ACMD(do_say);
   
   if(mini_mud || ( !CMD_IS("say") && !CMD_IS("'") ) )
        return(FALSE);

   location=real_room(21264);    /*destination room*/

   skip_spaces(&argument);
   if (strcasecmp(argument,"kallinistra"))
      return(FALSE);

   do_say(ch, argument, 0, 0);

   stc("\r\nWith a blinding flash of light and a crack of thunder, you are"
       " teleported...\r\n", ch);
   act ("\r\nWith a blinding flash of light and a crack of thunder, $n "
	"disappears!\r\n\r\n", FALSE, ch, NULL, NULL, TO_ROOM);

   char_from_room(ch);
   char_to_room(ch, location);
   act ("\r\nWith a blinding flash of light and a crack of thunder, $n "
	"appears!\r\n\r\n", FALSE, ch, NULL, NULL, TO_ROOM);
   do_look(ch,"",0,0);

   return(TRUE);
}

/* is_owner assumes that room_vnum is the HOUSE vnum */
bool
is_owner(struct char_data *ch, long room_vnum)
{
   int i, j;

   if (IS_NPC(ch))
      return FALSE; 

   if ((i = find_house(room_vnum)) < 0)
     return FALSE;

   /* let the house owner in */
   if (ch && (GET_IDNUM(ch) == house_control[i].owner))
	return TRUE;

   /* let any house guests in */
   for (j = 0; j <= house_control[i].num_of_guests; j++)
   {
      if (ch && (GET_IDNUM(ch) == house_control[i].guests[j]))
        return TRUE;
   }
   
   return FALSE;   
}

SPECIAL(wall_guard_ns)
{
  struct char_data *mobile = (struct char_data *)me;
  struct char_data *tch;
  struct char_data *tch_next;
  static int dir_to_move;
  static int talk = TRUE;

  if (cmd || (GET_POS(mobile) <= POS_SLEEPING) || FIGHTING(mobile))
    return FALSE;

  if (CAN_GO(mobile, NORTH) && !CAN_GO(mobile, SOUTH))  /* at southern end */
    dir_to_move = NORTH;

  if (CAN_GO(mobile, SOUTH) && !CAN_GO(mobile, NORTH)) /* at northern end */
    dir_to_move = SOUTH;
 
  perform_move(mobile, dir_to_move, 1);   /* walk the wall */

  for (tch = world[mobile->in_room].people; tch; tch = tch_next) { 
    tch_next = tch->next_in_room;
    if (tch == mobile)
      continue;
      
    if (IS_NPC(tch) && GET_MOB_VNUM(tch) == 8020 && talk) {  /* church guard */ 
      act("$n snaps to attention and salutes $N!", TRUE,  ch, 0, tch, TO_ROOM);
      act("$n says, 'Hello gents!'", TRUE, ch, 0, tch, TO_ROOM);
      act("$N nods at $n.", TRUE, ch, 0, tch, TO_ROOM);
	    act("$N says, 'On your way, soldier!'", TRUE, ch, 0, tch, TO_ROOM);
      talk = FALSE;
    }
  }
  return FALSE;    
}
