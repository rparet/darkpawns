/* ==========================================================================
   FILE   : new_cmds.c
   HISTORY: dlkarnes 970128 with a little help from cjackson 9/96
   ========================================================================= */

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

/* $Id: new_cmds.c 1273 2005-12-20 05:19:42Z mcooper $ */

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

/* extern variables */
extern char *dirs[];
extern struct review_t review_goss[25];
extern struct review_t review_auct[25];
extern struct review_t review_wizn[25];
extern int can_take_obj(struct char_data * ch, struct obj_data * obj);
extern int is_shopkeeper(struct char_data * chChar);
extern int top_of_treasure;
extern struct tlist_data *treasure_list;

/* extern functions */
void raw_kill(struct char_data * ch, struct char_data * killer, int attacktype);
void improve_skill(struct char_data *ch, int skill_num);
extern  struct char_data *create_mobile(struct char_data *ch, int
                                       mob_number,
                                       int level, int hunting); 
extern int find_exp(int level);
extern int min_exp(int level);
bool ok_loot(struct char_data * ch, struct obj_data * obj);

ACMD(do_mold)
{
   char objname[256], name[256], buf[MAX_STRING_LENGTH];
   char sdescr[MAX_STRING_LENGTH];
   struct obj_data *obj;
 
  
   argument = one_argument(argument, objname);
   skip_spaces(&argument);
   argument = one_word(argument, name);
   skip_spaces(&argument);

   strcpy(sdescr , argument);

   if (!(obj = get_obj_in_list_vis(ch, objname, ch->carrying)))
   {
      send_to_char("You don't have one of those.\r\n", ch);
      return;
   }

   if ((!isname("halo", obj->name))&&(!isname("clay", obj->name))
       &&(!isname("playdough", obj->name)))
   {
      send_to_char("You do not have anything to mold!\r\n", ch);
      return;
   }
  
   if (!(*name) || !(*sdescr))
   {
      send_to_char("You must specify a name and a description.\r\n", ch);
      return;
   }

   sprintf(buf , "%s _%s_ mold_item" , name , GET_NAME(ch));
   strcpy(name , buf);
   CREATE(obj->name, char, strlen(name) + 1);
   strcpy(obj->name, name);

   CREATE(obj->short_description, char, strlen(sdescr) + 1);
   strcpy(obj->short_description, sdescr);

   strcat(sdescr, " has been left here.");
   sdescr[0] = toupper(sdescr[0]);
   CREATE(obj->description, char, strlen(sdescr) + 1);
   strcpy(obj->description, sdescr);

   sprintf(sdescr, "The material magically hardens when you create %s.\r\n"
	   , obj->short_description);
   send_to_char(sdescr, ch);
 
}

/* ************************************************************************** 
   NAME       : do_carve
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : 
   OTHER      : 
   ************************************************************************ */
ACMD(do_carve)
{
  struct obj_data *tmp_obj, *obj, *temp, *next_obj;
  struct char_data *vict;

  argument = one_argument(argument, arg);

  vict = get_char_room_vis(ch, arg);

  if (GET_POS(ch) == POS_FIGHTING)
    {
      send_to_char("How can you think of food at a time like this?!?\r\n", ch);
      return;
    }
  else if (!*arg)
    {
      send_to_char("You want to carve what?!?\r\n", ch);
      return;
    }
  else if (vict == ch)
    {
      send_to_char("This game doesn't support self-mutilation!\r\n", ch);
      return;
    }
  else if (vict != NULL)
    {
      send_to_char("You kill it first and THEN you can eat it!\r\n",ch);
      return;
    }
  else if(!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))
    {
      sprintf(buf, "You can't seem to find a %s to carve!\r\n", arg);
      send_to_char(buf, ch);
      return;
    }
  else if (!(strstr(obj->name,"corpse")))
    {
      send_to_char("Your initials are about all you can carve in that!\r\n",
		   ch);
      return;
    }
  else if (!(strstr(obj->name,"carve_")))
    {
      send_to_char("There's no way you could ever eat THAT!!!\r\n", ch);
      return;
    }

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) 
  {
    send_to_char("Your arms are already full!\r\n", ch);
    return;
  }

  if (isname("carve_meat",obj->name)) 
    tmp_obj = read_object(1216, VIRTUAL, NULL);
  else if (isname("carve_fish",obj->name)) 
    tmp_obj = read_object(12, VIRTUAL, NULL);
  else if (isname("carve_bird",obj->name)) 
    tmp_obj = read_object(13, VIRTUAL, NULL);
  else if (isname("carve_rabbit",obj->name)) 
    tmp_obj = read_object(14, VIRTUAL, NULL);
  else 
    tmp_obj = read_object(1216, VIRTUAL, NULL);

  if(ch->equipment[WEAR_WIELD])
    {
      if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] == 3)/*type3=slash*/
	obj_to_char(tmp_obj, ch);
      else 
	if (ch->equipment[WEAR_WIELD]->obj_flags.value[3]==11)/*type11=pierce*/
	  obj_to_char(tmp_obj, ch);
	else
	  {
	    send_to_char("You can't carve with that!\n\r",ch);
	    extract_obj(tmp_obj);
	    return;
	  }
    }
  else
    {
      send_to_char("You don't have anything to carve with!\n\r",ch);
      extract_obj(tmp_obj);
      return;
    }

  act("$n carves up some meat from the $p.",TRUE,ch,obj,0,TO_ROOM);
  act("You carve up some meat from the $p.",TRUE,ch,obj,0,TO_CHAR);

  /* dump corpse contents */
  for (temp = obj->contains; temp; temp = next_obj)
    {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_room(temp, ch->in_room);
    }
  extract_obj(obj);
}


/* ************************************************************************** 
   NAME       : do_behead
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_behead)
{
  extern ubyte max_npc_corpse_time;
  struct obj_data *corpse_head, *corpse_body, *obj, *temp, *next_obj;
  struct char_data *vict;
  char name[256], buf[MAX_STRING_LENGTH];
  char sdescr[MAX_STRING_LENGTH];

  argument = one_argument(argument, arg);

  vict = get_char_room_vis(ch, arg);


  if (GET_POS(ch) == POS_FIGHTING)
    {
      send_to_char("You're a little busy for that!\r\n", ch);
      return;
    }
  else if (!*arg)
    {
      send_to_char("Behead who?\r\n", ch);
      return;
    }
  else if (vict == ch)
    {
      send_to_char("This MUD doesn't support self-mutilation!\r\n", ch);
      return;
    }
  else if (vict != NULL)
    {
      send_to_char("You kill it first and THEN you behead it!\r\n",ch);
      return;
    }
  else if(!(obj = get_obj_in_list_vis(ch,arg,world[ch->in_room].contents)))
    {
      sprintf(buf, "You can't seem to find a %s to behead!\r\n", arg);
      send_to_char(buf, ch);
      return;
    }
  else if ((strstr(obj->name,"headless")))
    {
      send_to_char("You can't behead something without a head!\r\n", ch);
      return;
    }
  else if (!((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && 
           (GET_OBJ_VAL(obj,3))))
    {
      send_to_char("You can't behead that!\r\n", ch);
      return;
    }
  else if (!ok_loot(ch, obj))
    {
      send_to_char("Perhaps if you killed it, then you could behead it!\r\n", ch);
      return;
    }


  if(ch->equipment[WEAR_WIELD])
    if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] == 3)
      {				/*type3=slash*/
	act("$n beheads $p!",TRUE,ch,obj,0,TO_ROOM);
	act("You behead $p!",TRUE,ch,obj,0,TO_CHAR);
      }
    else
      {
	if (!IS_NPC(ch))
	  act("$n rips the head off $p with $s bare hands!",
	      TRUE,ch,obj,0,TO_ROOM);
	else
	  act("$n rips the head off $p!", TRUE,ch,obj,0,TO_ROOM);
	
	act("You rip the head off $p with your bare hands!",
	    TRUE,ch,obj,0,TO_CHAR);
      }
  else
    {
	if (!IS_NPC(ch))
	  act("$n rips the head off $p with $s bare hands!",
	      TRUE,ch,obj,0,TO_ROOM);
	else
	  act("$n rips the head off $p!", TRUE,ch,obj,0,TO_ROOM);

      act("You rip the head off $p with your bare hands!",
	  TRUE,ch,obj,0,TO_CHAR);
    }

  CREATE(corpse_head, struct obj_data, 1);
  corpse_head = read_object(16, VIRTUAL, NULL); /*head_proto*/
 
  strcpy(name , "head");
  CREATE(corpse_head->name, char, strlen(name) + 1);
  strcpy(corpse_head->name, name);

  if(ch->equipment[WEAR_WIELD])
    if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] == 3)
      strcpy(sdescr,"a bloody head hacked from ");
    else
      strcpy(sdescr,"a bloody head ripped from ");
  else
    strcpy(sdescr,"a bloody head ripped from ");
 
  strcat(sdescr, obj->short_description);
  
   
  CREATE(corpse_head->short_description, char, strlen(sdescr) + 1);
  strcpy(corpse_head->short_description, sdescr);

  strcat(sdescr, " has been left here.");
  sdescr[0] = toupper(sdescr[0]);
   
  CREATE(corpse_head->description, char, strlen(sdescr) + 1);
  strcpy(corpse_head->description, sdescr);

  if (obj->owner)
    corpse_head->owner = str_dup(obj->owner);

  if(can_take_obj) 
   obj_to_char(corpse_head, ch);
  else
   obj_to_room(corpse_head, ch->in_room);

  CREATE(corpse_body, struct obj_data, 1);
  corpse_body = read_object(17, VIRTUAL, NULL); /*behead corpse_proto*/  
  corpse_body->obj_flags.value[0]=0;
  corpse_body->obj_flags.value[3]=1;
  corpse_body->obj_flags.timer = max_npc_corpse_time;
  
  strcpy(name, obj->name);
  strcat(name, " headless beheaded");
  CREATE(corpse_body->name, char, strlen(name) + 1);
  strcpy(corpse_body->name, name);
    

  if (obj->owner)
    corpse_body->owner = str_dup(obj->owner);  /* free'd in free_obj */
  if (obj->killer)
    corpse_body->killer = str_dup(obj->killer); /* free'd in free_obj */
  obj_to_room(corpse_body, ch->in_room); 

  for (temp = obj->contains; temp; temp = next_obj)
    {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_obj(temp, corpse_body);
    }
  extract_obj(obj);

}


/* ************************************************************************** 
   NAME       : do_otouch
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by farthammer
   OTHER      : 
   ************************************************************************ */
ACMD(do_otouch) /*orgasmic touch for immortals*/
{
  struct char_data *victim;

  if(!IS_NPC(ch))
    if (GET_LEVEL(ch) < LEVEL_IMMORT)
      {
	send_to_char("Come again?\n\r", ch); /*I couldn't resist =)*/
	return;
      }

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Yeah, ok, but who?\n\r", ch);
  else
    {
      if (!(victim = get_char_room_vis(ch, arg)))
	send_to_char("Humm...seems that person doesn't need your help.\n\r",
		     ch);
      else if (ch == victim)
	{
	  send_to_char("Yes, it WILL fall off if you don't stop that.\n\r",
		       ch);
	  act("$n just can't stop playing with $Mself!\n\r",
	      FALSE, ch, 0, victim, TO_NOTVICT);
	}
      else
	{
	  act("You touch $N, who slumps over in orgasm!",
	      FALSE, ch, 0, victim, TO_CHAR);
	  act("A wonderful feeling spreads throughout your entire body "
	      "as $N touches you in all the right places...\n\r"
	      "You explode with a breath-taking orgasm!!",
	      FALSE, victim, 0, ch, TO_CHAR);
	  act("$N collapses in a quivering, moaning heap as $n touches them..",
	      FALSE, ch, 0, victim, TO_NOTVICT);
	  GET_HIT(victim)+=2;

	  if (GET_SEX(victim) == SEX_MALE)
	    { 
	      send_to_char("God, you need some beer and some food.\n\r",
			   victim);
	      act("A dark stain appears under $N's armor and slowly "
		  "spreads down his leg.", FALSE, ch, 0, victim, TO_NOTVICT); 
	      if (GET_COND(victim,FULL)>=0)
		GET_COND(victim,FULL)=0;
	    }

	  if ((GET_SEX(victim) == SEX_FEMALE)&&(GET_SEX(ch)==SEX_MALE))
	    {
	      send_to_char("Once is never enough, you beg for more!\n\r",
			   victim);
	      act("$N pleads for more and $n deftly glides a hand beneath "
		  "her panties!", FALSE, ch, 0, victim, TO_NOTVICT); 
	    }
	}
    }
}


/* ************************************************************************** 
   NAME       : do_spike
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_spike)
{
  struct char_data *victim;
  char weapon[6];
  char spikebuf[MAX_STRING_LENGTH];

  if (subcmd == SCMD_SPIKE)
    sprintf(weapon, "spike");
  else
    sprintf(weapon, "stake");

  if (FIGHTING(ch))
    {
      sprintf(spikebuf, "You can't %s someone while fighting!\r\n", weapon);
      send_to_char(spikebuf,ch);
      return;
    }
  one_argument(argument, arg);

  if (!*arg)
    {
      sprintf(spikebuf, "Whom do you wish to %s?\r\n", weapon); 
      send_to_char(spikebuf, ch);
    }
  else if (!(victim = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);           
  else if ( !GET_EQ(ch, WEAR_WIELD) || 
	    !strstr(OBJN(GET_EQ(ch, WEAR_WIELD), ch), weapon) )
    {
      sprintf(spikebuf, "You need to wield a %s to succeed!\r\n", weapon);
      send_to_char(spikebuf, ch);
    }
  else if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    stc("You can't commit murder in this holy place!\r\n", ch);
  else if (victim == ch)
    send_to_char("The monster in you won't let you suicide!\r\n", ch);
  else if (subcmd==SCMD_SPIKE && !IS_AFFECTED(victim, AFF_WEREWOLF))
    send_to_char("Spiking is only for werewolves..\r\n", ch);
  else if (subcmd==SCMD_STAKE && !IS_AFFECTED(victim, AFF_VAMPIRE))
    send_to_char("Staking is only for vampires..\r\n", ch);
  else if ((subcmd == SCMD_SPIKE && PLR_FLAGGED(ch, PLR_WEREWOLF)) ||
	   (subcmd == SCMD_STAKE && PLR_FLAGGED(ch, PLR_VAMPIRE)) )
    send_to_char("You can't destroy your own kind!\r\n", ch);

  /* took this out so you can do nightbreeds any time -rparet 
  else if(weather_info.sunlight == SUN_SET||
	  weather_info.sunlight == SUN_DARK ) 
    send_to_char("The beast is too strong at night to be destroyed!\r\n",
		 ch);
  */
  else if ((GET_LEVEL(victim)>=LEVEL_IMMORT) && (GET_LEVEL(ch)<LEVEL_IMMORT))
    send_to_char("Yeah, right.\r\n", ch);
  else
    {
      /* success: your level > than victims or
	 victim-char_level < random_numberfrom0to31 or
	 the victim is asleep */
      if ((GET_LEVEL(ch) > GET_LEVEL(victim)) ||
	  (GET_LEVEL(victim)-GET_LEVEL(ch) < number(0,LEVEL_IMMORT)) ||
	  !AWAKE(victim) )
	{
	  act("You drive $p into $S chest!", 
	      FALSE, ch, GET_EQ(ch, WEAR_WIELD), victim, TO_CHAR);
	  act("$n drives $p into the chest of $N!",
	      FALSE, ch, GET_EQ(ch, WEAR_WIELD), victim, TO_NOTVICT);
	  act("$n drives $p into your chest with a solid blow!",
	      FALSE, ch, GET_EQ(ch, WEAR_WIELD), victim, TO_VICT);
	  if (PLR_FLAGGED(victim, PLR_VAMPIRE))
	    REMOVE_BIT(PLR_FLAGS(victim), PLR_VAMPIRE);          
	  if (PLR_FLAGGED(victim, PLR_WEREWOLF))
	    REMOVE_BIT(PLR_FLAGS(victim), PLR_WEREWOLF);          
	  sprintf(buf, "%s %sd %s at %s.",
		  GET_NAME(ch), weapon, GET_NAME(victim), 
		  world[victim->in_room].name);
	  mudlog(buf, BRF, LVL_IMMORT, TRUE); 
	  raw_kill(victim, ch,  TYPE_UNDEFINED);
	}
      else
	{
	  act("$N twists at the last moment, and you miss!",
	      FALSE, ch, 0, victim, TO_CHAR);
	  act("$N growls in anger as $n tries to drive a $p into $M!",
	      FALSE, ch, GET_EQ(ch, WEAR_WIELD), victim, TO_NOTVICT);
	  act("$n comes at you with a $p, but you dodge the attempt!",
	      FALSE, ch, GET_EQ(ch, WEAR_WIELD), victim, TO_VICT);
	}	
      WAIT_STATE(ch, 2);
    }
}


/* ************************************************************************** 
   NAME       : do_bite
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_bite)
{
  struct char_data *victim;
  char name[256];

  ACMD(do_transform);

  if (!ch->desc || ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    return;

  one_argument(argument, name);

  if (FIGHTING(ch) && (!*name))
  {
    victim=FIGHTING(ch);
    return;
  }
  else if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("Bite who?!\r\n", ch);
    return;
  }
 
  if (victim == ch)
    {
      send_to_char("You bite your tongue and say nothing.\r\n", ch);
      return;
    }
   
  if ( (!(PLR_FLAGGED(ch, PLR_WEREWOLF)) && !(PLR_FLAGGED(ch, PLR_VAMPIRE)))
       || (IS_NPC(ch)) )
    {
      if(!victim)
	{
	  send_to_char("You gnash your teeth.\r\n", ch);
	  act("$n gnashes $s teeth.", TRUE, ch, 0, 0, TO_ROOM);
	}
      else
	{
	  act("You give $N a love bite.", TRUE, ch, 0, victim, TO_CHAR);
	  act("$n tries to give you a little love bite.", TRUE,
	      ch, 0, victim, TO_VICT);
	 act("$n gives $N a love bite.", TRUE, ch, 0, victim, TO_NOTVICT);
	}
      return;
    }
  else if(!(IS_AFFECTED(ch, AFF_WEREWOLF))&&!(IS_AFFECTED(ch, AFF_VAMPIRE)))
    {
      send_to_char("You must be transformed to bite!\r\n", ch);
      return;
    }


  if ((GET_LEVEL(victim) >= LEVEL_IMMORT)&&(GET_LEVEL(victim)> GET_LEVEL(ch)))
    {
      send_to_char("Yeah, right.\r\n", ch);
      return;
    }

  if(PLR_FLAGGED(victim, PLR_WEREWOLF) || PLR_FLAGGED(victim, PLR_VAMPIRE))
    {
      send_to_char("Your victim is already a creature of the night!", ch);
      return;
    }
  

  if(IS_AFFECTED(ch, AFF_WEREWOLF))
    {
      act("You rip the flesh of $N, and blood pours over your lips!",
          TRUE,ch,0,victim,TO_CHAR);
      act("$n rips your flesh, leaving you bleeding and dazed!",
	  TRUE,ch,0,victim,TO_VICT);
      act("$n rips the flesh of $N, growling with bloodlust!",
	  TRUE,ch,0,victim,TO_NOTVICT);
	
      damage(ch, victim, MIN(15,GET_LEVEL(ch)), TYPE_BITE);
      WAIT_STATE(ch, 2);
    }
  else
    {
      act("Your fangs sink into the soft flesh of $N, and $S blood pours "
	  "over your lips." ,TRUE,ch,0,victim,TO_CHAR);
      act("$n's fangs sink into your flesh, leaving you bleeding and dazed!",
	  TRUE,ch,0,victim,TO_VICT);
      act("$n sinks $s fangs into the flesh of $N, feeding off $S blood!",
	  TRUE,ch,0,victim,TO_NOTVICT);

      if (!number(0,GET_LEVEL(ch)/2))
	{ /* fighting, or a sloppy bite */
	  act("$N screams in agony!", TRUE, ch, 0, victim, TO_NOTVICT);
	  damage(ch, victim, MIN(15,GET_LEVEL(ch)), TYPE_BITE);
	  WAIT_STATE(ch, 2);
	}
     if ((GET_COND(ch, FULL) < 40) && (GET_COND(ch, FULL) >= 0))
	GET_COND(ch, FULL) += GET_LEVEL(victim);
     if ((GET_COND(ch, THIRST) < 40) &&(GET_COND(ch, THIRST) >= 0))
	GET_COND(ch, THIRST) += GET_LEVEL(victim);
    }
  return;
}


/* ************************************************************************** 
   NAME       : full_moon()
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
void full_moon(void)
{
  struct descriptor_data *i;          
  ACMD(do_transform);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character )
      if (PLR_FLAGGED(i->character, PLR_VAMPIRE) || PLR_FLAGGED(i->character, PLR_WEREWOLF))
        if (!IS_AFFECTED(i->character, AFF_VAMPIRE) && !IS_AFFECTED(i->character, AFF_WEREWOLF)) {
          send_to_char("The lunar light infuses your body, forcing you to transform!\r\n", i->character);
          send_to_char("Racked with the pain of the transformation, your head is thrown back and an "
                       "unearthly moan escapes your lips.\r\n", i->character);
          do_transform(i->character, "", 0, 0);
        }
}

void remove_nightbreed(void)
{
  struct descriptor_data *i;          
  ACMD(do_transform);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character )
      if (PLR_FLAGGED(i->character, PLR_VAMPIRE) || PLR_FLAGGED(i->character, PLR_WEREWOLF))
        if (IS_AFFECTED(i->character, AFF_VAMPIRE) || IS_AFFECTED(i->character, AFF_WEREWOLF)) {
          send_to_char("The dawning of a new day forces you to transform!\r\n", i->character);
          send_to_char("Racked with the pain of the transformation, your head is thrown back and an "
                       "unearthly moan escapes your lips.\r\n", i->character);
          do_transform(i->character, "", 0, 0);
        }
}

/* ************************************************************************** 
   NAME       : update_review()
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128, modified mcooper 010526
   OTHER      : 
   ************************************************************************ */

void update_review(struct char_data *speaker, char *text, int type)
{
  int count = 0;
  
  if (!speaker || !text)
    return;

  switch(type)
  {
    case SCMD_REVGOSS:		/* move each item in review up one; remove the last one */
      for (count = 24; count > 0; count--)
      {
        review_goss[count].invis = review_goss[count - 1].invis;
        strcpy(review_goss[count].name, review_goss[count - 1].name);
        strcpy(review_goss[count].string, review_goss[count - 1].string);
      }
      strcpy(review_goss[0].string, text);    
      strcpy(review_goss[0].name, GET_NAME(speaker));
      review_goss[0].invis = GET_INVIS_LEV(speaker);
      break;
    case SCMD_REVAUCT:		/* move each item in review up one; remove the last one */
      for (count = 24; count > 0; count--)
      {
        review_auct[count].invis = review_auct[count - 1].invis;
        strcpy(review_auct[count].name, review_auct[count - 1].name);
        strcpy(review_auct[count].string, review_auct[count - 1].string);
      }
      strcpy(review_auct[0].string, text);    
      strcpy(review_auct[0].name, GET_NAME(speaker));
      review_auct[0].invis = GET_INVIS_LEV(speaker);
      break;
    case SCMD_REVWIZN:		/* move each item in review up one; remove the last one */
      for (count = 24; count > 0; count--)
      {
        review_wizn[count].invis = review_wizn[count - 1].invis;
        strcpy(review_wizn[count].name, review_wizn[count - 1].name);
        strcpy(review_wizn[count].string, review_wizn[count - 1].string);
      }
      strcpy(review_wizn[0].string, text);    
      strcpy(review_wizn[0].name, GET_NAME(speaker));
      review_wizn[0].invis = GET_INVIS_LEV(speaker);
      break;
  }
}

/* ************************************************************************** 
   NAME       : do_review
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128, modified mcooper 010526
   OTHER      : 
   ************************************************************************ */

ACMD(do_review)
{
  int count = 0, type = 9;

  skip_spaces(&argument);
  if (!*argument)
     type = SCMD_REVGOSS;

  if (!str_cmp(argument, "gossip"))
    type = SCMD_REVGOSS;
  else if (!str_cmp(argument, "auction"))
    type = SCMD_REVAUCT;
  else if (!str_cmp(argument, "wiznet"))
    type = SCMD_REVWIZN;

  switch(type)
  {
    case SCMD_REVGOSS:
      strcpy(buf, "Last Gossips:\r\n-------------\r\n");
      for (count = 24; count >= 0; count--)
      {
        if (strcmp(review_goss[count].name,"\0"))		/* i.e name != NULL */
        {
          strcat(buf, CCCYN(ch, C_CMP));
          if (review_goss[count].invis <= GET_LEVEL(ch))           
            strcat(buf, review_goss[count].name);
          else
            strcat(buf, "Someone invisible");
          strcat(buf, ": ");
          strcat(buf, CCYEL(ch, C_CMP));
          strcat(buf, review_goss[count].string);
          strcat(buf, CCNRM(ch, C_CMP));
          strcat(buf, "\n\r");
        }
      }
      break;
    case SCMD_REVAUCT:
      strcpy(buf, "Last Auctions:\r\n-------------\r\n");
      for (count = 24; count >= 0; count--)
      {
        if (strcmp(review_auct[count].name,"\0"))		/* i.e name != NULL */
        {
          strcat(buf, CCCYN(ch, C_CMP));
          if (review_auct[count].invis <= GET_LEVEL(ch))           
            strcat(buf, review_auct[count].name);
          else
            strcat(buf, "Someone invisible");
          strcat(buf, ": ");
          strcat(buf, CCMAG(ch, C_CMP));
          strcat(buf, review_auct[count].string);
          strcat(buf, CCNRM(ch, C_CMP));
          strcat(buf, "\n\r");
        }
      }
      break;
    case SCMD_REVWIZN:
      if (GET_LEVEL(ch) < LVL_IMMORT) {
        send_to_char("Huh?!?\r\n", ch);
        return;
      }
      else
      {
        strcpy(buf, "Last Wiznets:\r\n-------------\r\n");
        for (count = 24; count >= 0; count--)
        {
          if (strcmp(review_wizn[count].name,"\0"))		/* i.e name != NULL */
          {
            strcat(buf, CCCYN(ch, C_CMP));
            if (review_wizn[count].invis <= GET_LEVEL(ch))           
              strcat(buf, review_wizn[count].name);
            else
              strcat(buf, "Someone invisible");
            strcat(buf, ": ");
            strcat(buf, review_wizn[count].string);
            strcat(buf, CCNRM(ch, C_CMP));
            strcat(buf, "\n\r");
          }
        }
      }
      break;
    default:
      sprintf(buf, "Valid review lists are gossip");
      if (GET_LEVEL(ch) < LVL_IMMORT)
        strcat(buf, " and auction.\r\n");
      else
        strcat(buf, ", auction and wiznet.\r\n");
      break;
  }
  page_string(ch->desc, buf, 1);
}

/* ************************************************************************** 
   NAME       : do_whois
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_whois)
{
  struct char_data *player;
  
  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }

  if (!(player = load_char(arg))) {
    send_to_char("There is no such player.\r\n", ch);
    free_char(player);
    return;
  }

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    sprintf(buf, "[%3d] ", GET_LEVEL(player));
  else
    strcpy(buf, "\0");
  sprintf(buf + strlen(buf), "%s %s\r\n", GET_NAME(player), GET_TITLE(player));
  send_to_char(buf, ch);
  free_char(player);
}

/* ************************************************************************** 
   NAME       : check_kk_success()
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
static int
check_kk_success( struct char_data *ch, int skill_num)
{
  int percent = number(1, 101);	/* 101% is a complete failure */
  int prob = 0;

  switch (skill_num)
    {
    case SKILL_KK_RIN:
      prob = GET_SKILL(ch, SKILL_KK_RIN);
      break;
    case SKILL_KK_KYO:
      prob = GET_SKILL(ch, SKILL_KK_KYO);
      break;
    case SKILL_KK_TOH:
      prob = GET_SKILL(ch, SKILL_KK_TOH);
      break;
    case SKILL_KK_SHA:
      prob = GET_SKILL(ch, SKILL_KK_SHA);
      break;
    case SKILL_KK_KAI:
      prob = GET_SKILL(ch, SKILL_KK_KAI);
      break;
    case SKILL_KK_JIN:
      prob = GET_SKILL(ch, SKILL_KK_JIN);
      break;
    case SKILL_KK_RETSU:
      prob = GET_SKILL(ch, SKILL_KK_RETSU);
      break;
    case SKILL_KK_ZAI:
      prob = GET_SKILL(ch, SKILL_KK_ZAI);
      break;
    case SKILL_KK_ZHEN:
      prob = GET_SKILL(ch, SKILL_KK_ZHEN);
      break;
    }

  if (percent > prob)
    return (FALSE);
  else
    return (TRUE);
}


/* ************************************************************************** 
   NAME       : do_kuji_kiri
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_kuji_kiri)
{
  int MAX_SPELL_AFFECTS = 5;
  struct master_affected_type af[MAX_SPELL_AFFECTS];
  char *tovict = NULL, *toroom = NULL;
  int heal = 0, success = FALSE, i = 0;
  
  /* FIX - when you fix kuji kiri to use 1 skill slot, put the damn check here */

  if (FIGHTING(ch))
    {
      send_to_char("You are too busy fighting to practice kuji-kiri!\r\n", ch);
      return;
    }

  if (IS_AFFECTED(ch, AFF_KUJI_KIRI))
    {
      send_to_char("You can not practice kuji-kiri again right now!\r\n", ch);
      return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  success = check_kk_success(ch, subcmd);
  
  toroom = "$n interlaces $s fingers and meditates deeply.";

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    {
      af[i].type = subcmd;
      af[i].bitvector = 0;
      af[i].modifier = 0;
      af[i].location = APPLY_NONE;
    }

  af[0].bitvector = AFF_KUJI_KIRI;
  af[0].duration = 5;
  af[0].location = APPLY_SPELL;

  af[1].bitvector = AFF_KUJI_KIRI;
  af[1].duration = 5;
  af[1].location = APPLY_SPELL;

  switch(subcmd)
    {
    case SKILL_KK_RIN:
      if (!GET_SKILL(ch, SKILL_KK_RIN))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      af[0].location = APPLY_AC;
      af[0].duration = 5;
      af[0].modifier = -(15+(GET_LEVEL(ch)/2));
      af[1].bitvector = AFF_METALSKIN;
      af[1].duration = 5;
      tovict="Interlacing your fingers, you harden your mind and body.\r\n";
      toroom="$n interlaces $s fingers, and $s skin becomes metal!";
      break;
    case SKILL_KK_KYO:
      if (!GET_SKILL(ch, SKILL_KK_KYO))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      af[0].location = APPLY_HITROLL;
      af[0].duration = 5;
      af[0].modifier = 1;
      tovict="Interlacing your fingers, you focus your battle rage.\r\n";
      break;
    case SKILL_KK_TOH:
      if (!GET_SKILL(ch, SKILL_KK_TOH))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      af[0].location = APPLY_DAMROLL;
      af[0].duration = 5;
      af[0].modifier = 1;
      af[1].location = APPLY_AC;
      af[1].duration = 5;
      af[1].modifier = 10;
      tovict="Interlacing your fingers, you focus your inner strength.\r\n";
      break;
    case SKILL_KK_SHA:
      if (!GET_SKILL(ch, SKILL_KK_SHA))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      af[0].duration = 5;
      heal = GET_LEVEL(ch)<15?15:GET_LEVEL(ch);
      tovict="Interlacing your fingers, you heal your wounds.\r\n";
      break;
    case SKILL_KK_KAI:
      if (!GET_SKILL(ch, SKILL_KK_KAI))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      af[0].location = APPLY_DAMROLL;
      af[0].duration = 5;
      af[0].modifier = -1;
      af[1].location = APPLY_AC;
      af[1].duration = 5;
      af[1].modifier = -10;
      tovict="Interlacing your fingers, your body becomes your fortress.\r\n";
      break;
    case SKILL_KK_JIN:
      if (!GET_SKILL(ch, SKILL_KK_JIN))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      tovict="Interlacing your fingers, you focus on recooperation.\r\n";
      break;
    case SKILL_KK_RETSU:
      if (!GET_SKILL(ch, SKILL_KK_RETSU))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      if (success)
	call_magic(ch, ch, NULL, SPELL_TELEPORT, GET_LEVEL(ch), CAST_SPELL);
      else
	tovict="Your concentration is broken!\r\n";
      af[0].duration = 5;
      toroom = NULL;
      break;
    case SKILL_KK_ZAI:
      if (!GET_SKILL(ch, SKILL_KK_ZAI))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      af[0].location = APPLY_HITROLL;
      af[0].duration = 5;
      af[0].modifier = 0;
      af[1].bitvector = AFF_INVISIBLE;
      af[1].duration = 5;
      tovict="Interlacing your fingers, you slowly fade from view.\r\n";
      toroom="$n interlaces $s fingers and slowly fades from view.";
      break;
    case SKILL_KK_ZHEN:
      if (!GET_SKILL(ch, SKILL_KK_ZHEN))
      {
        stc("You have not mastered this art yet!\r\n", ch);
        return;
      }
      tovict="Interlacing your fingers, you focus on your endurance.\r\n";
      break;
    }

  if (!success)
    {
      af[0].modifier = 0;
      af[1].modifier = 0;
      af[1].bitvector = AFF_NOTHING;
      heal = FALSE;
      tovict = "You try the art of kuji-kiri, but can't concentrate!\r\n";
      improve_skill(ch, subcmd);
    }
  af->by_type = BY_SPELL;
  af->obj_num = 0;
   
  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || (af[i].location != APPLY_NONE))
      affect_join(ch, af+i, FALSE, FALSE, FALSE);

  if (heal)
    {
      GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + heal);
      update_pos(ch);
    }
    
  if (tovict != NULL)
    send_to_char(tovict, ch);
  if (toroom != NULL)
    act(toroom, TRUE, ch, 0, NULL, TO_ROOM);
}

/* ************************************************************************** 
   NAME       : flesh_alter_from()
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
void flesh_alter_from(struct char_data *ch)
{
  GET_HITROLL(ch) -= (GET_LEVEL(ch) / 10) + 1;
  GET_DAMROLL(ch) -= (GET_LEVEL(ch) / 7) + 1;
}


/* ************************************************************************** 
   NAME       : flesh_alter_to()
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
void flesh_alter_to(struct char_data *ch)
{
  GET_HITROLL(ch) += (GET_LEVEL(ch) / 10) + 1;
  GET_DAMROLL(ch) += (GET_LEVEL(ch) / 7) + 1;
}


/* ************************************************************************** 
   NAME       : flesh_altered_type()
   PURPOSE    : 
   RETURNS    : hit_type based on what weapon it is.
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970322
   OTHER      : 
   ************************************************************************ */
int flesh_altered_type(struct char_data *ch)
{
  int level = (GET_LEVEL(ch) / 3);
  switch (level) {
    case 1:
    case 2:      
    case 3:
      return(7);
    case 4:      
    case 5:
    case 6:      
      return(11);
    case 7:
    case 8:      
    case 9:
      return(3);
    case 10:      
    case 11:
    case 12:      
    case 13:
    case 14:      
    case 15:
      return(7);
    case 16:      
    case 17:
    case 18:      
    case 19:
    case 20:
    case 21:            
      return(3);
    case 22:
    case 23:            
    case 24:
      return(7);
    case 25:            
    case 26:
    case 27:            
    case 28:
    case 29:            
    default:
      return(3);
    }
  
}
/* ************************************************************************** 
   NAME       : flesh_alter_weapon()
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : if you change this, change flesh_altered_type() too
   ************************************************************************ */
char *flesh_alter_weapon(struct char_data *ch)
{
  int level = (GET_LEVEL(ch) / 3);

  switch (level) {
    case 1:
    case 2:      
    case 3:
      return(str_dup("studded wooden club"));
    case 4:      
    case 5:
    case 6:      
      return(str_dup("razor-sharp dagger"));
    case 7:
    case 8:      
    case 9:
      return(str_dup("steel-shafted axe"));
    case 10:      
    case 11:
    case 12:      
      return(str_dup("studded steel mace"));
    case 13:
    case 14:      
    case 15:
      return(str_dup("battle flail"));
    case 16:      
    case 17:
    case 18:      
      return(str_dup("steel-shafted battle axe"));
    case 19:
    case 20:
    case 21:            
      return(str_dup("double-headed battle axe"));
    case 22:
    case 23:            
    case 24:
      return(str_dup("studded morning-star"));
    case 25:            
    case 26:
    case 27:            
      return(str_dup("gleaming broad sword"));
    case 28:
    case 29:            
      return(str_dup("gleaming long sword"));
    default:
      return(str_dup("gleaming scythe"));
  }
}


/* ************************************************************************** 
   NAME       : do_flesh_alter
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_flesh_alter)
{
  char to_vict[180], to_room[180], *weapon;
  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);
  
  if (IS_AFFECTED(ch, AFF_FLESH_ALTER)) {
    send_to_char("You shift your molecules back to normal.\r\n", ch);
    REMOVE_BIT(AFF_FLAGS(ch), AFF_FLESH_ALTER);
    weapon = flesh_alter_weapon(ch);
    flesh_alter_from(ch);
    sprintf(to_vict, "Your hand reverts from a %s.\r\n", weapon);
    sprintf(to_room, "$n's hand reverts from a %s!", weapon);
    send_to_char(to_vict, ch);
    act(to_room, TRUE, ch, 0, 0, TO_ROOM);

    FREE(weapon);
    return;
  }

  if (skill_success(ch, SKILL_FLESH_ALTER, 0)) {
    weapon = flesh_alter_weapon(ch);
    SET_BIT(AFF_FLAGS(ch), AFF_FLESH_ALTER);
    flesh_alter_to(ch);
    if (weap) {
      obj_to_char(unequip_char(ch, WEAR_WIELD), ch);
      act("You stop using $p.", FALSE, ch, weap, 0, TO_CHAR);
      act("$n stops using $p.", TRUE, ch, weap, 0, TO_ROOM);
    }  
    sprintf(to_vict, "Your hand turns into a %s!\r\n", weapon);
    sprintf(to_room, "$n's hand turns into a %s!", weapon);
    send_to_char(to_vict, ch);
    act(to_room, TRUE, ch, 0, 0, TO_ROOM);
    improve_skill(ch, SKILL_FLESH_ALTER);

    FREE(weapon);
  } else {
    send_to_char("You lose your concentration!\r\n", ch);
    WAIT_STATE(ch, 2);
  }
}

/* ************************************************************************** 
   NAME       : do_compare
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
              : Updated for 2.2 by rparet 981210
   OTHER      : 
   ************************************************************************ */
ACMD(do_compare)
{
  struct obj_data *obj1, *obj2;
  int diff = 0;
  int where1 = 0, where2 = 0;
  char arg2[MAX_INPUT_LENGTH];

  half_chop(argument,arg, arg2);

  if ((!arg) && (!arg2)) {
    send_to_char("Compare what and what?\r\n",ch);
    return;
  }

  if (!(obj1 = get_obj_in_list_vis(ch, arg, ch->carrying)) ||
      !(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)) || FIGHTING(ch)) {
    send_to_char("Looks like you don't have those objects..\n\r", ch);
    return;
  }

  if (obj1 == obj2) {
    send_to_char("They're the same thing!\n\r", ch);
    return;
  }

  if (GET_OBJ_TYPE(obj1) != GET_OBJ_TYPE(obj2)) {
    stc("You can't compare those things!\r\n", ch);
    return;
  }

  if (GET_OBJ_TYPE(obj1) != ITEM_WEAPON && GET_OBJ_TYPE(obj1) != ITEM_ARMOR) {
    stc("Compare is only for weapons and armor.\r\n", ch);
    return;
  }

  if (GET_OBJ_TYPE(obj1) == ITEM_ARMOR) {
    if (CAN_WEAR(obj1, ITEM_WEAR_FINGER))      where1 = WEAR_FINGER_R;
    if (CAN_WEAR(obj1, ITEM_WEAR_NECK))        where1 = WEAR_NECK_1;   
    if (CAN_WEAR(obj1, ITEM_WEAR_BODY))        where1 = WEAR_BODY; 
    if (CAN_WEAR(obj1, ITEM_WEAR_HEAD))        where1 = WEAR_HEAD;
    if (CAN_WEAR(obj1, ITEM_WEAR_LEGS))        where1 = WEAR_LEGS;
    if (CAN_WEAR(obj1, ITEM_WEAR_FEET))        where1 = WEAR_FEET;
    if (CAN_WEAR(obj1, ITEM_WEAR_HANDS))       where1 = WEAR_HANDS;
    if (CAN_WEAR(obj1, ITEM_WEAR_ARMS))        where1 = WEAR_ARMS;
    if (CAN_WEAR(obj1, ITEM_WEAR_SHIELD))      where1 = WEAR_SHIELD;
    if (CAN_WEAR(obj1, ITEM_WEAR_ABOUT))       where1 = WEAR_ABOUT;
    if (CAN_WEAR(obj1, ITEM_WEAR_WAIST))       where1 = WEAR_WAIST;
    if (CAN_WEAR(obj1, ITEM_WEAR_WRIST))       where1 = WEAR_WRIST_R;
    if (CAN_WEAR(obj1, ITEM_WEAR_ABLEGS))      where1 = WEAR_ABLEGS;
    if (CAN_WEAR(obj1, ITEM_WEAR_FACE))        where1 = WEAR_FACE;
    if (CAN_WEAR(obj1, ITEM_WEAR_HOVER))       where1 = WEAR_HOVER;
    if (CAN_WEAR(obj1, ITEM_WEAR_WIELD))       where1 = WEAR_WIELD;
        
    if (CAN_WEAR(obj2, ITEM_WEAR_FINGER))      where2 = WEAR_FINGER_R;
    if (CAN_WEAR(obj2, ITEM_WEAR_NECK))        where2 = WEAR_NECK_1;   
    if (CAN_WEAR(obj2, ITEM_WEAR_BODY))        where2 = WEAR_BODY; 
    if (CAN_WEAR(obj2, ITEM_WEAR_HEAD))        where2 = WEAR_HEAD;
    if (CAN_WEAR(obj2, ITEM_WEAR_LEGS))        where2 = WEAR_LEGS;
    if (CAN_WEAR(obj2, ITEM_WEAR_FEET))        where2 = WEAR_FEET;
    if (CAN_WEAR(obj2, ITEM_WEAR_HANDS))       where2 = WEAR_HANDS;
    if (CAN_WEAR(obj2, ITEM_WEAR_ARMS))        where2 = WEAR_ARMS;
    if (CAN_WEAR(obj2, ITEM_WEAR_SHIELD))      where2 = WEAR_SHIELD;
    if (CAN_WEAR(obj2, ITEM_WEAR_ABOUT))       where2 = WEAR_ABOUT;
    if (CAN_WEAR(obj2, ITEM_WEAR_WAIST))       where2 = WEAR_WAIST;
    if (CAN_WEAR(obj2, ITEM_WEAR_WRIST))       where2 = WEAR_WRIST_R;
    if (CAN_WEAR(obj2, ITEM_WEAR_ABLEGS))      where2 = WEAR_ABLEGS;
    if (CAN_WEAR(obj2, ITEM_WEAR_FACE))        where2 = WEAR_FACE;
    if (CAN_WEAR(obj2, ITEM_WEAR_HOVER))       where2 = WEAR_HOVER;
    if (CAN_WEAR(obj2, ITEM_WEAR_WIELD))       where2 = WEAR_WIELD;
      
    if (where1 != where2) {
      stc("You can only compare the same types of armor!\r\n", ch);
      return;
    }
  }

  if (GET_OBJ_TYPE(obj1) == ITEM_WEAPON)
    diff = ( (((GET_OBJ_VAL(obj1, 2) + 1) / 2.0) * GET_OBJ_VAL(obj1, 1)) -
     (((GET_OBJ_VAL(obj2, 2) + 1) / 2.0) * GET_OBJ_VAL(obj2, 1)) );

  if (GET_OBJ_TYPE(obj1) == ITEM_ARMOR)
    diff = ( ((GET_OBJ_VAL(obj1, 0) + 1) / 2.0) - ((GET_OBJ_VAL(obj2, 0) + 1) / 2.0));

  if (!skill_success(ch, SKILL_COMPARE, 0))
    diff += number (-3,3);
  else
    improve_skill(ch, SKILL_COMPARE);
  
  if (diff < -5)
    sprintf(buf,"%s looks much worse than %s.\r\n",
     obj1->short_description, obj2->short_description);
  else if (diff < -3)
    sprintf(buf,"%s looks a little worse than %s.\r\n",
     obj1->short_description, obj2->short_description);
  else if (diff < 0)
    sprintf(buf,"%s looks slightly worse than %s.\r\n",
     obj1->short_description, obj2->short_description);
  else if (!diff)
    sprintf(buf,"They look just about the same.\r\n");
  else if (diff < 3)
    sprintf(buf,"%s looks slightly better than %s.\r\n",
     obj1->short_description, obj2->short_description);
  else if (diff < 5)
    sprintf(buf,"%s looks a little better than %s.\r\n",
     obj1->short_description, obj2->short_description);
  else
    sprintf(buf,"%s looks much better than %s.\r\n",
     obj1->short_description, obj2->short_description);
  
  CAP(buf);
  send_to_char(buf, ch);
}


/* ************************************************************************** 
   NAME       : do_palm
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_palm)
{
  void get_from_room(struct char_data *ch, char *arg, bool quiet);

  one_argument(argument, arg);

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    send_to_char("Your arms are already full!\r\n", ch);
  else if (!*arg)
    send_to_char("Palm what?\r\n", ch);
  else
    get_from_room(ch, arg, skill_success(ch, SKILL_PALM, 0));
}

/* ************************************************************************** 
   NAME       : do_tag
   PURPOSE    : 
   RETURNS    : 
   WARNINGS   : 
   HISTORY    : Created by dlkarnes 970128
   OTHER      : 
   ************************************************************************ */
ACMD(do_tag)
{
  struct descriptor_data *d;
  struct char_data *it = NULL, *vict = NULL;
  
  one_argument(argument, arg);

  if (!*arg)
    {
      send_to_char("Tag who?\r\n", ch);
      return;
    }
  if (!(vict = get_char_room_vis(ch, arg)) )
    {
      send_to_char("They don't seem to be here.\r\n", ch);
      return;
    }
  
  if (!IS_NPC(vict) && !vict->desc) { /* linkless */
    send_to_char("No way! They're linkless!\r\n", ch);
    return;
  }

  for (d = descriptor_list; d; d = d->next) {
    if(!d->connected && d->character) {
      if (PLR_FLAGGED(d->character, PLR_IT))
        it = d->character;
    }
  }

  if (it && ch!=it)
    {
      send_to_char("But you are not it!\r\n", ch);
      return;
    }

  if (vict == it)
    {
      send_to_char("You're it already!\r\n", ch);
      return;
    }

  if (IS_NPC(vict))
    {
      act("You tag $N, but $E tags you right back!\r\n",
	  TRUE, ch, 0, vict, TO_CHAR);
      return;
    }
    
  if (PLR_FLAGGED(ch, PLR_IT))
    REMOVE_BIT(PLR_FLAGS(ch), PLR_IT);
  if (PLR_FLAGGED(vict, PLR_IT))
    REMOVE_BIT(PLR_FLAGS(vict), PLR_IT);

  SET_BIT(PLR_FLAGS(vict), PLR_IT);         

  if (ch == vict)
    send_to_char("Let the game begin!\r\n", ch);    
  else
    {
      act("$n taps $N and screams, 'TAG! You're it!'",
	  TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n taps you and screams, 'TAG! You're it!'",
	  TRUE, ch, 0, vict, TO_VICT); 
      act("You tap $N and scream, 'TAG! You're it!'",
	  TRUE, ch, 0, vict, TO_CHAR);
      WAIT_STATE(vict, 2); 
    }
}

/* do_scan function is obsolete as of 19980803  -rparet */

ACMD(do_scan)
{
   int dir, found = FALSE;

   for(dir = 0; dir < NUM_OF_DIRS; dir++)
   {
      int ok = FALSE;
      if(CAN_GO(ch, dir))
      {
         struct char_data *tch = NULL;
         struct char_data *tch_next = NULL;
	 for(tch = world[EXIT(ch, dir)->to_room].people; tch; tch=tch_next)
	 {
	    tch_next = tch->next_in_room;
	    if ( (CAN_SEE(ch, tch) && !IS_AFFECTED(tch, AFF_HIDE)) ||
                ( (IS_SET(PRF_FLAGS(ch), PRF_HOLYLIGHT) &&
                   GET_REAL_LEVEL(ch)>=GET_INVIS_LEV(tch)) ) )
	    {
		ok = TRUE;
	        break;
	    }
	 }
	 if (ok)
 	 {
	  if (world[EXIT(ch, dir)->to_room].people)
	  {
	    sprintf(buf, "%s%s:\r\n",
		    ((dir==5)?"Below":(dir==4)?"Above": "To the "), 
		    ((dir==5)?"":(dir==4)?"":dirs[dir]));
	    send_to_char(buf, ch);
	    found = TRUE;
	  }
	  for(tch = world[EXIT(ch, dir)->to_room].people; tch; tch=tch_next)
	  {
	    tch_next = tch->next_in_room;
	    if ( (CAN_SEE(ch, tch) && !IS_AFFECTED(tch, AFF_HIDE)) ||
                ( (IS_SET(PRF_FLAGS(ch), PRF_HOLYLIGHT) &&
                   GET_REAL_LEVEL(ch)>=GET_INVIS_LEV(tch)) ) )
	    {
	      sprintf(buf, "   %s\r\n", GET_NAME(tch));
	      send_to_char(buf, ch);
	    }
	  }
	 }
      }
   }
   if (!found)
	send_to_char("You see nothing around.\r\n", ch);
   return;
}

ACMD(do_jump)
{

  act("You jump into the air.", TRUE, ch, 0, 0, TO_CHAR);
  act("$n jumps into the air.", TRUE, ch, 0, 0, TO_ROOM);

  SET_BIT(COMBAT_STATE(ch), CS_JUMP);
  WAIT_STATE(ch, 1);

}

ACMD(do_duck)
{
  act("You duck down.", TRUE, ch, 0, 0, TO_CHAR);
  act("$n ducks down.", TRUE, ch, 0, 0, TO_ROOM);

  SET_BIT(COMBAT_STATE(ch), CS_DUCK);
  WAIT_STATE(ch, 1);
}


static void
point_obj(struct char_data *ch, struct obj_data *tobj)
{
   struct obj_data *obj = NULL;

   if ((obj = GET_EQ(ch, WEAR_WIELD)))
   {
    act("$n points $p at $P.", FALSE, ch, obj, tobj, TO_ROOM);
    act("You point $p at $P.", TRUE, ch, obj, tobj, TO_CHAR);
   }
   else
   {
    act("$n points at $P.", FALSE, ch, obj, tobj, TO_ROOM);
    act("You point at $P.", TRUE, ch, obj, tobj, TO_CHAR);
   }
}

static char *
ismove(char *buf)
{
  if (is_abbrev(buf, "east"))
	return(str_dup("east"));
  if (is_abbrev(buf, "west"))
	return(str_dup("west"));
  if (is_abbrev(buf, "up"))
	return(str_dup("up"));
  if (is_abbrev(buf, "down"))
	return(str_dup("down"));
  if (is_abbrev(buf, "north"))
	return(str_dup("north"));
  if (is_abbrev(buf, "south"))
	return(str_dup("south"));
  return(NULL);
}

ACMD(do_point)
{
  struct char_data *vict = NULL;
  struct obj_data *obj = NULL, *tobj = NULL;
  char *to_room = NULL, *to_char = NULL, *to_vict = NULL;
  one_argument(argument, buf);

  vict = get_char_room_vis(ch, buf);

  if (!vict)
  {
   char *word = NULL;
   if ((tobj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents)))
   {
     point_obj(ch, tobj);
     return;
   }
   else if ((word = ismove(buf)))
   {
	char mybuf[256];
	sprintf(mybuf, "$n points %s.", word);
	act(mybuf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(mybuf, "You point %s.\r\n", word);
	send_to_char(mybuf, ch);	
	FREE(word);
   }
   else
   {
    to_room = "$n points around the room.";
    to_char = "You point around the room.";
   }
  }
  else  if (vict == ch)
  {
   to_room = "$n points at $mself.";
   to_char = "You point at yourself.";
  }
  else 
  {
   if ((obj = GET_EQ(ch, WEAR_WIELD)))
   {
    to_room = "$n points $p at $N.";
    to_char = "You point $p at $N.";
    to_vict = "$n points $p at you.";
   }
   else
   {
    to_room = "$n points at $N.";
    to_char = "You point at $N.";
    to_vict = "$n points at you.";
   }
  }
  if (to_room)
	act(to_room, FALSE, ch, obj, vict, TO_NOTVICT);
  if (to_char)
	act(to_char, TRUE, ch, obj, vict, TO_CHAR);
  if (to_vict)
	act(to_vict, TRUE, ch, obj, vict, TO_VICT);
}

void
lunar_hunter(void)
{
  ACMD(do_tell);
  struct descriptor_data *i;
  struct char_data *mob = NULL;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character)
	if (GET_LEVEL(i->character) > 30 &&
	    GET_LEVEL(i->character) < LVL_IMMORT && 
	    number(0,5) == 0)
	{
	   char msg[256];
           mob = create_mobile(i->character,7,(GET_LEVEL(i->character)/2)+2,TRUE);    
           char_from_room(mob);
           char_to_room(mob, real_room(8067));
           sprintf (msg, "%s By the light of the full moon.. DIE!",
                   GET_NAME(i->character));
           do_tell(mob, msg, find_command("tell"), 0);
	 } 
}

#define DOCK_KO 4883
#define DOCK_KM 21295
#define GHOST_SHIP 19101

/* 
  ship_appear is a more generic ghost_ship_appear which allows for the movement of lots of
  different boat-type rooms, moving platforms, etc.
*/
void
ship_appear(int ship_vnum, int dock_vnum, char *arrive_msg_ship, char *arrive_msg_dock)
{
   int dock = real_room(dock_vnum), ship = real_room(ship_vnum);

   if (dock <= 0 || ship <=0 )
	return;

   CREATE(world[dock].dir_option[NORTH], struct room_direction_data, 1);
   world[dock].dir_option[NORTH]->to_room = ship;
   CREATE(world[ship].dir_option[SOUTH], struct room_direction_data, 1);
   world[ship].dir_option[SOUTH]->to_room = dock;
  
   send_to_room(arrive_msg_dock, dock);
   send_to_room(arrive_msg_ship, ship);
}

void ship_disappear(int ship_vnum, char *leave_msg_ship, char *leave_msg_dock)
{
  int ship = real_room(ship_vnum), dock = NOWHERE;
    
  /* figure out where the ship is docked by seeing what room it is attached to */
  if (world[ship].dir_option[SOUTH])
    dock = world[ship].dir_option[SOUTH]->to_room;
 
  if (ship <= 0 || dock <= 0)
    return;
 
  if (world[dock].dir_option[NORTH]) {
    FREE(world[dock].dir_option[NORTH]);
    world[dock].dir_option[NORTH] = NULL;
    send_to_room(leave_msg_dock, dock);
  }
 
  if (world[ship].dir_option[SOUTH]) {
    FREE(world[ship].dir_option[SOUTH]);
    world[ship].dir_option[SOUTH] = NULL;
    send_to_room(leave_msg_ship, ship);
  }
}

void ghost_ship_appear(void)
{
 int dock = 0;
 if (!number(0,1))
   dock = DOCK_KO;
 else
   dock = DOCK_KM;
  
 ship_appear(GHOST_SHIP, dock, "Suddenly, a dock appears to the south!\r\n", "Suddenly, a ghostly ship appears to the north!\r\n");
}

void
ghost_ship_disappear(void)
{
   ship_disappear(GHOST_SHIP, "Suddenly the dock to the south disappears!\r\nThe ghost ship has set sail!\r\n",
    "Suddenly the ghostly ship to the north disappears!\r\n");
}

/* **************************************************************************
   NAME       : do_checkload
   PURPOSE    : Locate objects/mobs and display where they load
   RETURNS    :
   WARNINGS   :
   HISTORY    :
   OTHER      : Ceramic Mouse website (raymond@brokersys.com)
   ************************************************************************ */

#define ZCMD zone_table[zone].cmd[cmd_no]  /*fom DB.C*/

/* 
  type  = 'O', 'M' or 'T'
  which = real number of mob/object, or treasure list number
  name = common name of mob/object, or treasure list name
*/

void check_load_status (char type, int which, struct char_data *ch, char *name)   
{
  int zone, lastroom_v = 0, lastroom_r = 0, lastmob_r = 0;
  int cmd_no, perc_load = 0;
  char mobname[100];
  struct char_data *mob = NULL;
  struct obj_data *obj = NULL;
  
  sprintf(buf, "Checking load info for %s...\r\n", name);

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) 
      if (type == 'm' || type == 'M')
        switch (ZCMD.command) {
          case 'M':                   /* read a mobile */
            if (ZCMD.arg1 == which)
              sprintf(buf+strlen(buf), "  [%5ld] %s (%d Max)\r\n", world[ZCMD.arg3].number,
                      world[ZCMD.arg3].name, ZCMD.arg2);
            break;
          case 'R':                   /* rem mob from room */
            lastroom_v = world[ZCMD.arg1].number;
            lastroom_r = ZCMD.arg1;
            if (!ZCMD.arg2 && ZCMD.arg3 == which)
              sprintf(buf+strlen(buf), "  [%5d] %s (Removed from room)\r\n", lastroom_v, 
                      world[lastroom_r].name);
            break;
  	}
      else if (type == 't' || type == 'T')
        switch (ZCMD.command) {
          case 'M':
	    lastroom_v = world[ZCMD.arg3].number;
	    lastroom_r = ZCMD.arg3;
	    lastmob_r = ZCMD.arg1;
	    mob = read_mobile(lastmob_r, REAL, NULL);
	    char_to_room(mob, 0);
	    strcpy(mobname, GET_NAME(mob));
	    extract_char(mob);
	    break;
          case 'T':               /* read a treasure list */
            if (ZCMD.arg1 == which) {
              if (!ZCMD.arg3)
                sprintf(buf + strlen(buf), "  [%5d] %s (Equipped to %s [%d])\r\n",
                  lastroom_v, world[lastroom_r].name, mobname, mob_index[lastmob_r].virtual);
              else
                sprintf(buf + strlen(buf), "  [%5d] %s (Given to %s [%d])\r\n",
                  lastroom_v, world[lastroom_r].name, mobname, mob_index[lastmob_r].virtual);
            }
            break;
        }
      else
        switch (ZCMD.command) {
          case 'M':
            lastroom_v = world[ZCMD.arg3].number;
            lastroom_r = ZCMD.arg3;
            lastmob_r = ZCMD.arg1;
            mob = read_mobile(lastmob_r, REAL, NULL);
            char_to_room(mob, 0);
            strcpy(mobname, GET_NAME(mob));
            extract_char(mob);
            break;
          case 'O':                   /* read an object */
            lastroom_v = world[ZCMD.arg3].number;
            lastroom_r = ZCMD.arg3;
            obj = read_object(which, REAL, NULL);
            perc_load = GET_OBJ_LOAD(obj);
            extract_obj(obj);
            if (ZCMD.arg1 == which)
              sprintf(buf+strlen(buf), "  [%5d] %s (%d%% Load, %d Max)\r\n", lastroom_v,
                      world[lastroom_r].name, perc_load, ZCMD.arg2);
            break;
          case 'P':                   /* object to object */
            if (ZCMD.arg1 == which)
              sprintf(buf+strlen(buf), "  [%5d] %s (Put in another object [%d%% Load, %d"
                      " Max])\r\n",lastroom_v, world[lastroom_r].name,
                      perc_load, ZCMD.arg2);
            break;
          case 'E':                   /* equip/give object */
	    if (ZCMD.arg1 == which) {
              if (!ZCMD.arg3)
	        sprintf(buf+strlen(buf), "  [%5d] %s (Equipped to %s [%d][%d%% Load, %d Max])\r\n",
                        lastroom_v, world[lastroom_r].name, mobname, mob_index[lastmob_r].virtual,
                        perc_load, ZCMD.arg2);
               else
   		sprintf(buf+strlen(buf), "  [%5d] %s (Given to %s [%d][%d%% Load, %d Max])\r\n",
                        lastroom_v, world[lastroom_r].name, mobname, mob_index[lastmob_r].virtual, 
                        perc_load, ZCMD.arg2);
            }
            break;
          case 'R':                   /* rem obj from room */
            lastroom_v = world[ZCMD.arg1].number;
            lastroom_r = ZCMD.arg1;
            if (ZCMD.arg2 && ZCMD.arg3 == which)
              sprintf(buf+strlen(buf), "  [%5d] %s (Removed from room)\r\n", lastroom_v, 
                      world[lastroom_r].name);
            break;
	  } /*switch for object*/

  page_string (ch->desc, buf, 1);
}

ACMD(do_checkloadstatus)
{  
  long which;
  struct char_data *mob;
  struct obj_data *obj;

  two_arguments(argument, buf2, buf3);
  if ((!*buf2) || (!*buf3) || (!isdigit(*buf3))) {
    send_to_char("Usage: checkload { obj | mob | treasure } <number>\r\n", ch);
    return;
  }   

  /*These lines do nothing but look up the name of the object or mob
    so the output is more user friendly*/
  if (*buf2 == 'm') {
    which = real_mobile(atoi(buf3));
    if (which < 0) {
      send_to_char("That mob does not exist.\r\n", ch);
	    return;
    }
    mob = read_mobile(which, REAL, NULL);
    char_to_room(mob, 0);
    strcpy(buf3, GET_NAME(mob));
    extract_char(mob);
  } else if (*buf2 == 'o') {
    which = real_object(atoi(buf3));
    if (which < 0) {
      send_to_char("That object does not exist.\r\n", ch);
	    return;
    }
    obj = read_object(which, REAL, NULL);
    strcpy(buf3, obj->short_description);
    extract_obj(obj);
  } else {
    which = atoi(buf3);
    if ((which <= 0) || (which > top_of_treasure)) {
      send_to_char("That treasure list does not exist.\r\n", ch);
      return;
    }
    sprintf(buf3, "'%s' treasure items", treasure_list[--which].type);
  }

  /*end name look up*/

  check_load_status(buf2[0], which, ch, buf3);
}
