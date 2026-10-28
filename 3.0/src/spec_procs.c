/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
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

/* $Id: spec_procs.c 1277 2006-04-17 22:50:26Z rparet $ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"


/*   external vars  */
extern struct time_info_data time_info;
extern int mini_mud;
extern struct spell_info_type spell_info[];
extern char *unused_spellname;	/* spell_parser.c */

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
int num_followers(struct char_data *ch);
long find_skill_exp (int skill_level);
ACMD(do_stand);
ACMD(do_wake);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_parry);
ACMD(do_headbutt);
ACMD(do_berserk);
ACMD(do_charge);
ACMD(do_disarm);

int mag_savingthrow(struct char_data * ch, int type);
struct char_data *HUNTING(struct char_data *ch);
void send_to_zone(char *messg, struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);

/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS+1];

int compare_spells(const void *x, const void *y)
{
  int a = *(const int *)x, b = *(const int *)y;

  return strcmp(spell_info[a].name, spell_info[b].name);
}

void sort_spells(void)
{
  int a;

  /* initialize array, avoiding reserved. */
  for (a = 1; a <= MAX_SKILLS; a++)    
    spell_sort_info[a] = a;

  qsort(&spell_sort_info[1], MAX_SKILLS, sizeof(int), compare_spells);
}


char *
how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else if (percent <= 98)
    strcpy(buf, " (superb)");
  else
    strcpy(buf, " (MASTER)");

  return (buf);
}

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */

/* depreciated - remove after all the skills are fixed for 3.0 */
/* void
list_skills(struct char_data * ch)
{
  int mag_manacost(struct char_data * ch, int spellnum);
  int i, sortpos, mana;
  char manastring[30];
  struct char_data *pers;

  if (ch->desc && ch->desc->original)
    pers = ch->desc->original;
  else
    pers = ch;

  if (!GET_PRACTICES(pers))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(pers), (GET_PRACTICES(pers) == 1 ? "" : "s"));

  sprintf(buf, "%sYou know of the following skills:\r\n", buf);

  strcpy(buf2, buf);

  for (sortpos = 1; sortpos < MAX_SKILLS; sortpos++)
    {
      i = spell_sort_info[sortpos];
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32)
	{
	  strcat(buf2, "**OVERFLOW**\r\n");
	  break;
	}
      
      if ((GET_SKILL(pers, i) > 0) && (strcmp(spell_info[i].name, unused_spellname)))
	/ has learned at least once and is not !UNUSED!, etc. /
      {
        mana = mag_manacost(pers, find_skill_num(spell_info[i].name));   
        if(mana)
        sprintf(manastring ,"( %s%d %s%s )", 
		    CCRED(pers, C_CMP), mana, 
		    "mana",
		    CCNRM(pers, C_CMP));
        sprintf(buf, "%-20s (%d) (%ld) %s\r\n", spell_info[i].name,
        GET_SKILL(pers, i), find_skill_exp(GET_SKILL(pers, i)), mana?manastring:"");
        strcat(buf2, buf);
      }
    }

  page_string(ch->desc, buf2, 1);
} */

SPECIAL(guild)
{
  struct char_data *mobile = (struct char_data *)me;

  if (IS_NPC(ch) || !CMD_IS("practice"))
    return 0;

  skip_spaces(&argument);

  if (!*argument)
    {
      /* list_skills(ch); */
      return 1;
    }

  if (*argument)
  {
    sprintf(buf, "Try learn %s %s.\r\n", GET_NAME(mobile), argument);
    stc(buf, ch);
    return 1;
  }

/*
  depricated - schedule for removal -rparet 
  if (GET_PRACTICES(ch) <= 0)
    {
      send_to_char("You do not seem to be able to practice now.\r\n", ch);
      return 1;
    }

  skill_num = find_skill_num(argument);

  if (skill_num < 1)
    {
      sprintf(buf, "You do not know of that skill.\r\n");
      send_to_char(buf, ch);
      return 1;
    }
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    {
      send_to_char("You are already learned in that area.\r\n", ch);
      return 1;
    }
  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char("You are now learned in that area.\r\n", ch);
*/
  return 1;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */

SPECIAL(summoner)
{
  struct char_data *vict = NULL;
  struct descriptor_data *d;
  memory_rec *names;
  bool found = FALSE;

  if (cmd || GET_POS(ch) != POS_STANDING)
    return FALSE;

  if (HUNTING(ch))
  {
    vict = HUNTING(ch);
    found = TRUE;
  }

  if (!vict && MEMORY(ch))
    for (d = descriptor_list; !found && d; d = d->next) 
    {
      if (!d->connected)
      {
       vict = (d->original ? d->original : d->character);
       if (vict && CAN_SEE(ch, vict) && (vict->in_room != NOWHERE))
        for (names = MEMORY(ch); names && !found; names = names->next)
          if (names->id == GET_IDNUM(vict)) 
            found = TRUE;
      }
    }

  if (vict && found && !number(0,3))
  {
    call_magic(ch, vict, 0, SPELL_SUMMON, GET_LEVEL(ch), CAST_SPELL);
    if(ch->in_room == vict->in_room)
      hit(ch, vict, TYPE_UNDEFINED);
    return(TRUE);
  }
  return(FALSE);
}

/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */
SPECIAL(fighter)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_HIT(ch) < 0 || !FIGHTING(ch))
    return FALSE;

  if (GET_COMBAT_WAIT(ch))
    return FALSE;

  /* removed the pick-a-random-fighting character bs  -rparet 19980717 */
  vict = FIGHTING(ch);

  if (vict == ch)
    return FALSE;

  switch(number(0,10))
   {
     case 1: do_headbutt(ch, GET_NAME(vict), 0, 1);
	     	break;
     case 2: do_parry(ch, GET_NAME(vict), 0, 1);
		break;
     case 3: do_bash(ch, GET_NAME(vict), 0, 1);
	     	break;
     case 4: do_berserk(ch, NULL, 0, 1);
                break;
     default: return(FALSE);
   }
  return(TRUE);
}

SPECIAL(mayor)
{
  ACMD(do_gen_door);

  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  if (!move)
    {
      if (time_info.hours == 6)
	{
	  move = TRUE;
	  path = open_path;
	  index = 0;
	}
      else if (time_info.hours == 20)
	{
	  move = TRUE;
	  path = close_path;
	  index = 0;
	}
    }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index])
    {
    case '0':
    case '1':
    case '2':
    case '3':
      perform_move(ch, path[index] - '0', 1);
      break;

    case 'W':
      GET_POS(ch) = POS_STANDING;
      act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'S':
      GET_POS(ch) = POS_SLEEPING;
      act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'a':
      act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'b':
      act("$n says 'What a view!  I must get something done about that dump!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'c':
      act("$n says 'Vandals!  Youngsters nowadays have no respect "
	  "for anything!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'd':
      act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'e':
      act("$n says 'I hereby declare the bazaar open!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'E':
      act("$n says 'I hereby declare Bourbon closed!'",
	  FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'O':
      do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
      do_gen_door(ch, "gate", 0, SCMD_OPEN);
      break;

    case 'C':
      do_gen_door(ch, "gate", 0, SCMD_CLOSE);
      do_gen_door(ch, "gate", 0, SCMD_LOCK);
      break;

    case '.':
      move = FALSE;
      break;

    }

  index++;
  return FALSE;
}

#define BLACK_UNDEAD  18401
#define RED_UNDEAD    18402

SPECIAL(black_undead_knight)
{
  struct char_data *tmp_ch ;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
    return(FALSE);

  if ( FIGHTING(ch) )
    {
      switch ( number( 1, 20 ) )
	{
	case 1 : 
	  act ( "$n screams, 'Protect the kingdom!'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 2:
	  act ( "$n shouts, 'If I'm going to hell, your going with me!'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 3:
	  act ( "$n says, 'You dirty rotten scoundrel.  "
		"I'm gonna make you very sorry.'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 4:
	  act ( "$n says, 'I know what your thinking...'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  act ( "$n says, 'Did he fire five shots, or did he fire six.'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  act ( "$n says, 'Well let me ask you...'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  act ( "$n asks, 'Do you feel lucky PUNK?  Well... DO YOU?'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	case 5:
	  act ( "$n claims, 'I am the greatest!'",
		FALSE, ch, NULL, NULL, TO_ROOM) ;
	  break ;
	default:
	  break ;
	}
      return ( TRUE ) ;
    }

  for ( tmp_ch = world[ch->in_room].people; tmp_ch;
	tmp_ch = tmp_ch->next_in_room)
    if ( IS_NPC(tmp_ch) &&
	 (mob_index[tmp_ch->nr].virtual == RED_UNDEAD) &&
	 !number(0,2) )
      {
	act ( "$n sees the knight dressed in red and gives a battle cry!",
	      FALSE, ch, NULL, NULL, TO_ROOM ) ;
	hit (ch, tmp_ch, TYPE_UNDEFINED) ;
	return ( TRUE ) ;
      }
   
  return ( FALSE ) ;

}


SPECIAL(red_undead_knight)
{
   struct char_data *i ;

   if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
      return(FALSE);

   if ( FIGHTING(ch) )
   {
      switch ( number( 1, 20 ) )
      {
	 case 1 : 
	    act ( "$n screams, 'Protect the homeland!'",
	          FALSE, ch, NULL, NULL, TO_ROOM) ;
	    break ;
	 case 2:
	    act ( "$n shouts, 'If you think you have had a bad day "
		  "before, watch this!'",
	          FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
	 case 3:
	    act ( "$n says, 'Don't ever argue with the big dog,'",
                  FALSE, ch, NULL, NULL, TO_ROOM) ;
	    act ( "$n says, 'cause the big dog is always right.'",
                  FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
         case 4:
            act ( "$n says, 'There's more that one way to skin a cat:'",
                  FALSE, ch, NULL, NULL, TO_ROOM) ;
            act ( "$n continues: 'Way number 15 -- Krazy Glue and a "
		  "toothbrush.'", FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
         case 5:
           act ( "$n says, 'A friend with weed is a friend indeed.'",
                 FALSE, ch, NULL, NULL, TO_ROOM) ;
            break ;
         default:
            break ;
      }
      return ( TRUE ) ;
   }

   for ( i = world[ch->in_room].people; i;
         i = i->next_in_room)
      if ( IS_NPC(i) && 
           (mob_index[i->nr].virtual == BLACK_UNDEAD)
           && !number(0,2) )
      {
         act ( "$n sees the knight dressed in black and gives a battle cry!",
            FALSE, ch, NULL, NULL, TO_ROOM ) ;
         hit (ch, i, TYPE_UNDEFINED) ;
         return ( TRUE ) ;
      }
   
   return ( FALSE ) ;
}

/* ==========================================================================
   Room special procedures
   ========================================================================= */


#define PET_PRICE(pet) (GET_LEVEL(pet) * 25)

SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;

  pet_room = ch->in_room + 1;

  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (num_followers(ch)>=GET_CHA(ch)/2)
    {
	stc("You can't have any more followers!\r\n", ch);
	return(TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= PET_PRICE(pet);

    pet = read_mobile(GET_MOB_RNUM(pet), REAL, NULL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* FREE(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says "
		"'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* FREE(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return 1;
  }
  /* All commands except list and buy */
  return 0;
}

static int
MobCountInRoom( struct char_data *list)
{
  int i;
  struct char_data *tmp;
  for (i=0, tmp = list; tmp; tmp = tmp->next_in_room, i++)
    ;
  return(i);
}

SPECIAL(enter_circle)
{
  struct char_data *tmp;
  int portal_room;
  ACMD(do_look);

  if (mini_mud || (!CMD_IS("enter")&&!CMD_IS("look")))
	return FALSE;
  portal_room=real_room(5799);	/*This is the elevator itself*/
  if (CMD_IS("enter"))
    {				/*enter*/
      skip_spaces (&argument);
      if (strcmp(argument,"circle")&&strcmp(argument,"platform"))
	{
	  send_to_char("Enter what?\n\r",ch);
	  return(TRUE);	
	}

      if (MobCountInRoom(world[portal_room].people)>=2)
	{
	  send_to_char("You can't fit on the portal, it's too crowded.\n\r",
		       ch);
	  return(TRUE);
	}
      else
	{
	  send_to_char("You stand in the circle.\n\r",ch);
	  act("$n enters the circle which suddenly starts glowing brightly, "
	      "obscuring your view of $m!",1,ch,0,0,TO_ROOM);
	  char_from_room(ch);
	  char_to_room(ch,portal_room);
	  do_look(ch,"",0,0);
	  return(TRUE);
	}
    }
  else
    {			/*look*/
      skip_spaces(&argument);
      if (strcmp(argument,"circle")&&strcmp(argument,"platform"))
	{
	  do_look(ch,argument,0,0);
	  return(TRUE);
	}
	
      send_to_char("Looking into the circle at the platform in the middle"
		   " of the room, you see\n\r",ch);
      if (MobCountInRoom(world[portal_room].people))
	for (tmp = world[portal_room].people; tmp;tmp=tmp->next_in_room)
	  {
	    send_to_char(GET_NAME(tmp),ch);
	    if(tmp->next_in_room)
	      send_to_char(" and ",ch);
	  }
      else
	send_to_char("no one",ch);
      send_to_char(".\n\r\n\r",ch);
      return(TRUE);
    }
return FALSE;
}

SPECIAL(elevator)
{
   struct char_data *tmp_ch, *next;
   int location;
   ACMD(do_look);
   ACMD(do_say);
   int portal_room;

   if(mini_mud || ( !CMD_IS("say") && !CMD_IS("'") ) )
	return(FALSE);
   
   if ((strcmp(argument," Sumuni Elementi Avia Elevata"))
       &&(strcmp(argument," sumuni elementi avia elevata")))
      return(FALSE);

   portal_room=real_room(5799);	/*This is the elevator itself*/
   location=real_room(5743);	/*destination room*/

   do_say(ch,argument,0,0);
   send_to_char("The portal begins to rise, lifted by the air elemental "
		"summoned by your rune!\n\r\n\r",ch);
   act("The portal begins to rise, lifted by the air elemental summoned by "
       "$n!\r\n\r\n", FALSE,ch,NULL,NULL,TO_ROOM);

   if( (tmp_ch = world[portal_room].people) )
   {
      next=tmp_ch->next_in_room;
      char_from_room(tmp_ch);
      char_to_room(tmp_ch,location);
      do_look(tmp_ch,"",0,0);
      if(next)
      {
	 char_from_room(next);
	 char_to_room(next,location);
	 do_look(next,"",0,0);
      }
   }
   return(TRUE);
}


SPECIAL(fearface)
{
   struct char_data *victim, *tmp_ch;
   int count;

   if (!AWAKE(ch) || !IS_NPC(ch) || cmd || GET_HIT(ch) < 0)
      return(FALSE);
  
   if (FIGHTING(ch))
   {
      if (GET_POS(ch)>POS_SLEEPING && GET_POS(ch)<POS_FIGHTING) 
	 do_stand(ch, "", 0, 0);
      else
	 for (victim = 0, tmp_ch = world[ch->in_room].people;
	      tmp_ch && !victim; tmp_ch = tmp_ch->next_in_room)
	    if(!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&
	       !PRF_FLAGGED(tmp_ch,PRF_NOHASSLE))
	    {
	       victim=tmp_ch;
	       if ( !mag_savingthrow(victim, SAVING_SPELL) )
	       {
		  act("$n stares into the eyes of $N, who panics in fear!",
		      TRUE, ch, NULL, NULL, TO_ROOM);
		  for (count = number(1,5); count > 0; count++)
		     do_flee(victim, "",0,0);
		  return(TRUE);
	       }
	    }
   }
   return(FALSE);
}

#define NEWBIE_LEVEL 11
SPECIAL(newbie_zone_entrance)
{
   if ( !CMD_IS("south") )
     return FALSE;

   if ( (GET_LEVEL(ch) >= NEWBIE_LEVEL) && (GET_LEVEL(ch) < LEVEL_IMMORT))
   {
      stc("Nah, you're too much of a badass to go in there!\r\n", ch);
      return TRUE;
   }

   return FALSE;
} 


#define PAINTING_ROOM  18101

SPECIAL(suck_in)
  {
    ACMD(do_look);
  
    if (mini_mud || !CMD_IS("look"))
      return FALSE;

    argument = one_argument(argument, buf);
      
    if (!strcasecmp(buf, "painting"))
      {
	do_look(ch, buf, 0, SCMD_LOOK);  
	send_to_char("\r\n\r\nYou suddenly feel very dizzy...\r\n\r\n", ch);
	act("$n suddenly vanishes!", FALSE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, real_room(PAINTING_ROOM));
	look_at_room(ch, 1);
	return TRUE;
      }
    return FALSE;
  }


SPECIAL(oro_quarters_room)
{
   if ( IS_MOB(ch) || !CMD_IS("south") )
     return FALSE ;

   if (strcmp(GET_NAME(ch), "Orodreth"))
     {
       act("A strong force jolts $n in $s attempt to leave south.",
	   FALSE, ch, 0, 0, TO_ROOM);
       send_to_char("A strong force blocks your way and gives you a nasty "
		    "jolt.\r\n", ch) ;
       GET_HIT(ch) = GET_HIT(ch)/2;
       return TRUE ;
     }

   return FALSE ;
}


SPECIAL(oro_study_room)
{
   if ( IS_MOB(ch) || !CMD_IS("north") )
     return FALSE ;

   if (strcmp(GET_NAME(ch), "Orodreth"))
     {
       act("A strong force jolts $n in $s attempt to leave north.",
	   FALSE, ch, 0, 0, TO_ROOM) ;
       send_to_char("A strong force blocks your way and gives you a nasty "
		    "jolt.\r\n", ch) ;
       GET_HIT(ch) = GET_HIT(ch)/2;
       return TRUE ;
     }

   return FALSE ;
}
