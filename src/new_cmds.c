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

/* $Id: new_cmds.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
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
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct index_data *obj_index;
extern char *dirs[];
extern struct review_t review[25];
extern int can_take_obj(struct char_data * ch, struct obj_data * obj);
extern int is_shopkeeper(struct char_data * chChar);
extern int mini_mud;

/* extern functions */
void raw_kill(struct char_data * ch, int attacktype);
void improve_skill(struct char_data *ch, int skill_num);
void set_hunting(struct char_data *ch, struct char_data *victim);
extern  struct char_data *create_mobile(struct char_data *ch, int
                                       mob_number,
                                       int level, int hunting);

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
    tmp_obj = read_object(8015, VIRTUAL);
  else if (isname("carve_fish",obj->name))
    tmp_obj = read_object(12, VIRTUAL);
  else if (isname("carve_bird",obj->name))
    tmp_obj = read_object(13, VIRTUAL);
  else if (isname("carve_rabbit",obj->name))
    tmp_obj = read_object(14, VIRTUAL);
  else
    tmp_obj = read_object(8015, VIRTUAL);

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
  extern int max_npc_corpse_time;
  struct obj_data *tmp_obj, *obj, *temp, *next_obj;
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


  if(ch->equipment[WEAR_WIELD])
    if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] == 3)
      {             /*type3=slash*/
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

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);
  tmp_obj = read_object(16, VIRTUAL); /*head_proto*/

  strcpy(name , "head");
  CREATE(tmp_obj->name, char, strlen(name) + 1);
  strcpy(tmp_obj->name, name);

  if(ch->equipment[WEAR_WIELD])
    if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] == 3)
      strcpy(sdescr,"a bloody head hacked from ");
    else
      strcpy(sdescr,"a bloody head ripped from ");
  else
    strcpy(sdescr,"a bloody head ripped from ");

  strcat(sdescr, obj->short_description);


  CREATE(tmp_obj->short_description, char, strlen(sdescr) + 1);
  strcpy(tmp_obj->short_description, sdescr);

  strcat(sdescr, " has been left here.");
  sdescr[0] = toupper(sdescr[0]);

  CREATE(tmp_obj->description, char, strlen(sdescr) + 1);
  strcpy(tmp_obj->description, sdescr);

  if(can_take_obj(ch, tmp_obj))
   obj_to_char(tmp_obj, ch);
  else
   obj_to_room(tmp_obj, ch->in_room);

  CREATE(tmp_obj, struct obj_data, 1);
  tmp_obj = read_object(17, VIRTUAL); /*behead corpse_proto*/
  tmp_obj->obj_flags.value[0]=0;
  tmp_obj->obj_flags.value[3]=1;
  tmp_obj->obj_flags.timer = max_npc_corpse_time;

  strcpy(name, obj->name);
  strcat(name, " headless beheaded"); /* so now it should be "Prismal corpse headless beheaded" */
  CREATE(tmp_obj->name, char, strlen(name) + 1);
  strcpy(tmp_obj->name, name);

  obj_to_room(tmp_obj, ch->in_room);

  for (temp = obj->contains; temp; temp = next_obj)
    {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_obj(temp, tmp_obj);
    }
  extract_obj(obj);

}


/* **************************************************************************
   NAME       : do_headbutt
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_headbutt)
{
  struct char_data *victim;
  char name[256];
  byte percent;

  if (!ch->desc && !subcmd)
    return;

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
     send_to_char("The Gods prevent thy violent act.\r\n", ch);
     return;
  }

  if (!GET_SKILL(ch, SKILL_HEADBUTT))
    {
      send_to_char("You aren't qualified to headbutt anyone!\r\n", ch);
      if (!subcmd)
    return;
    }
  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)))
    {
      if (FIGHTING(ch))
    victim = FIGHTING(ch);
      else
    {
      send_to_char("Headbutt who?\n\r", ch);
      return;
    }
    }

  if (victim == ch)
    {
      send_to_char("You bang your head into the nearest wall...\n\r", ch);
      return;
    }

  if ((GET_LEVEL(victim) >= LEVEL_IMMORT) && (!IS_NPC(victim)))
    {
      send_to_char("How dare you try to headbutt a god!\n\r", ch);
      send_to_char("You are thrown across the room...\n\r", ch);
      GET_POS(ch) = POS_SITTING;
      update_pos(ch);
      return;
    }

  percent = number(1, 121); /* 101% is a complete failure */

  if ((GET_POS(victim) <= POS_SLEEPING) ||
      (GET_LEVEL(ch) > LEVEL_IMMORT))
    percent = 0;

  if (MOB_FLAGGED(victim, MOB_NOBASH))
    percent = 0;

  if(GET_LEVEL(ch)/2>GET_HIT(ch))
    {
      send_to_char("But that could kill you!\n\r",ch);
      return;
    }

  if ( percent > (subcmd?number(50,100):GET_SKILL(ch,SKILL_HEADBUTT)) )
    {
      if (GET_POS(victim) > POS_DEAD)
      damage(ch, victim, 0, SKILL_HEADBUTT);
    }
  else
    if (GET_POS(victim) > POS_DEAD)
      {
    if(!GET_EQ(ch, WEAR_HEAD))
      GET_HIT(ch) -= GET_LEVEL(ch)/4;
    else
      GET_HIT(ch) -= GET_LEVEL(ch)/3;
    damage(ch, victim, GET_LEVEL(ch), SKILL_HEADBUTT);
        improve_skill(ch, SKILL_HEADBUTT);
    if (victim && GET_POS(victim) > POS_STUNNED)
    {
       GET_POS(victim) = POS_SITTING;
           update_pos(victim);
    }
        if (!subcmd)
       improve_skill(ch, SKILL_HEADBUTT);
      }
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
} /* do_headbutt*/


/* **************************************************************************
   NAME       : do_bearhug
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_bearhug)
{
  struct char_data *victim;
  byte percent, prob;
  int dam = 0;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_BEARHUG))
    {
      send_to_char("You'd better leave all the martial arts to fighters.\n\r",
           ch);
      if (!subcmd)
        return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (!(victim = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
    victim = FIGHTING(ch);
      else
    {
      send_to_char("Bear hug who?\r\n", ch);
      return;
    }
    }

  if (!IS_MOB(victim) && GET_LEVEL(victim) >= LEVEL_IMMORT)
    {
      send_to_char("The gods reject your impunity.\r\n", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }

  if (ch->equipment[WEAR_WIELD])
    {
      send_to_char("You need to be bare handed to get a good grip.\r\n", ch);
      return;
    }

  percent = number(1, 150); /* 101% is a complete failure */

  if ((GET_POS(victim) <= POS_SLEEPING) ||
      (GET_LEVEL(ch) > LEVEL_IMMORT))
    percent = 101;
  if (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_NOBASH)) /* can't bash means can't bearhug */
    percent = 101;

  prob = subcmd?number(50,100):GET_SKILL(ch, SKILL_BEARHUG);

  if (percent > prob)  /* failure */
    damage(ch, victim, 0, SKILL_BEARHUG);
  else
    {
      dam = (GET_LEVEL(ch)*1.5);
      damage(ch, victim, dam, SKILL_BEARHUG);
      improve_skill(ch, SKILL_BEARHUG);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


/* **************************************************************************
   NAME       : do_cutthroat
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_cutthroat)
{
  struct master_affected_type af;
  struct char_data *victim;
  byte percent, prob;
  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_CUTTHROAT))
    {
      send_to_char("You're not trained in slitting throats!\n\r", ch);
      return;
    }

  if (!(victim = get_char_room_vis(ch, buf)))
    {
      send_to_char("Cut what throat where?\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char("That would be bad.\n\r", ch);
      return;
    }
  if (!ch->equipment[WEAR_WIELD])
    {
      send_to_char("You need to wield a weapon to make it a success.\n\r", ch);
      return;
    }
  if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] != 11)
    {
      send_to_char("Only daggers and such can be used for cutting a "
           "throat.\n\r", ch);
      return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (IS_SET_AR(world[ch->in_room].room_flags, ROOM_PEACEFUL))
    {
      send_to_char("You feel too peaceful to slit a throat!\n\r", ch);
      return;
    }

  if (!IS_NPC(victim) && GET_LEVEL(victim) <= 10)
  {
    act("Ancient forces protect $N from your wrath!", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if (IS_AFFECTED(victim,AFF_CUTTHROAT))
    {
      send_to_char("Their throat is already slit!\r\n", ch);
      return;
    }

  if (FIGHTING(ch) || FIGHTING(victim))
    {
     stc("You can't get close enough!\r\n", ch);
      return;
    }

  percent = number(1, 101); /* 101% is a complete failure */

  prob = GET_SKILL(ch, SKILL_CUTTHROAT);
  if (GET_LEVEL(ch)>LEVEL_IMMORT)
    prob=102;
  if (GET_LEVEL(victim)>LEVEL_IMMORT)
    prob=-1;

  if (percent<prob)
    {
      af.type = SKILL_CUTTHROAT;
      af.duration = GET_LEVEL(ch) * 2;
      af.modifier = -2;
      af.location = APPLY_HITROLL;
      af.bitvector = AFF_CUTTHROAT;
      af.by_type = BY_SPELL;
      af.obj_num = 0;

      affect_join(victim, &af, af.duration, FALSE, FALSE, FALSE);
      sprintf(buf, "Suddenly %s slits your throat!\r\n", GET_NAME(ch));
      send_to_char(buf, victim);
      send_to_char("You slit their throat from ear to ear!\r\n", ch);
      improve_skill(ch, SKILL_CUTTHROAT);
    }
  else
    {
      send_to_char("Your slash at their throat barely misses!\n\r",ch);
      sprintf(buf, "%s makes a vicious lunge at your throat!\r\n",
        GET_NAME(ch));
      send_to_char(buf,victim);
    }

  if (percent < prob)   /*success*/
    damage(ch, victim, GET_LEVEL(ch)/2, SKILL_CUTTHROAT);
  else
    hit(ch, victim, SKILL_CUTTHROAT);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
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
   NAME       : do_trip
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_trip)
{
  struct char_data *victim;
  byte percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_TRIP))
    {
      stc("You'd better leave the sneaky stuff to the thieves.\r\n", ch);
      if (!subcmd)
        return;
    }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
      stc("This room just has such a peaceful, easy feeling...\r\n", ch);
      return;
    }

  if (!(victim = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
    victim = FIGHTING(ch);
      else
    {
      send_to_char("Trip who?\r\n", ch);
      return;
    }
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (victim == ch)
  {
      send_to_char("You trip over your shoe laces...\n\r", ch);
      return;
  }
  if (IS_AFFECTED(victim, AFF_FLY))
  {
    stc("You can't trip something that's FLYING!\r\n", ch);
    return;
  }
  if (GET_POS(victim) <= POS_SLEEPING)
  {
    stc("Whats the point of doing that now?\r\n", ch);
    return;
  }

  percent = number(1, 121); /* 101% is a complete failure */
  if (GET_LEVEL(victim) >= LEVEL_IMMORT)
    percent = 101;
  if (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_NOBASH))
    percent = 101;

  prob = GET_SKILL(ch, SKILL_TRIP);

  percent += MAX(GET_LEVEL(victim)-GET_LEVEL(ch),0);

  if (percent > prob)  /* failure */
    {
      damage(ch, victim, 0, SKILL_TRIP);
      GET_POS(ch) = POS_SITTING;
      update_pos(ch);
      WAIT_STATE(ch, PULSE_VIOLENCE); /* extra pulse for failing */
    }
  else if (damage(ch, victim, (GET_LEVEL(ch)/2)+1, SKILL_TRIP))
    {
      if (!subcmd)
        improve_skill(ch, SKILL_TRIP);
      if (victim)
    {
      GET_POS(victim) = POS_SITTING;
      update_pos(victim);
      WAIT_STATE(victim, PULSE_VIOLENCE);
    }
    }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_slug)
{
   struct char_data *victim;
   byte percent, prob;

   one_argument(argument, arg);

   if (!GET_SKILL(ch, SKILL_SLUG))
     {
       send_to_char("You couldn't slug your way out of a wet paper bag.\n\r",
            ch);
       return;
     }

   if (!(victim = get_char_room_vis(ch, arg)))
     {
       if (FIGHTING(ch))
     victim = FIGHTING(ch);
       else
     {
       send_to_char("Slug who?\n\r", ch);
       return;
     }
     }

   if (victim == ch)
     {
      send_to_char("You curl up your fist and slug yourself in the nose! "
           "Ouch!\n\r", ch);
      return;
     }

   if (ch->equipment[WEAR_WIELD])
     {
      send_to_char("You can't make a fist while wielding a weapon!\n\r", ch);
      return;
     }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

   percent = number(1, 101); /* 101% is a complete failure */
   prob = GET_SKILL(ch, SKILL_SLUG);

   if (percent > prob)
   {
      damage(ch, victim, 0, SKILL_SLUG);
      improve_skill(ch, SKILL_SLUG);
   }
   else
   {
      damage(ch, victim, GET_LEVEL(ch)*(number(1,4)*.5), SKILL_SLUG);
   }
   WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_charge)
{
  struct char_data *victim;
  byte percent, prob;
  struct obj_data *wielded;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_CHARGE))
    {
      stc("You couldn't charge if you wanted to!\r\n", ch);
      if (!subcmd)
        return;
    }

  if (!(victim = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
    victim = FIGHTING(ch);
      else
    {
      send_to_char("Great! Fine! Charge who?!?!\r\n", ch);
      return;
    }
    }

  if (victim == ch)
    {
      send_to_char("You charge headlong into the ground, "
           "impressing everyone..\n\r", ch);
      return;
    }

  if (!(wielded=ch->equipment[WEAR_WIELD]))
    {
      send_to_char("You're barehanded, try it with a sword or "
           "lance next time.\n\r", ch);
      return;
    }

  if (ch->equipment[WEAR_WIELD]->obj_flags.value[3] != 12
      && ch->equipment[WEAR_WIELD]->obj_flags.value[3] != 3)
    {
      stc("You need sword or a lance to run 'em through!\r\n",ch);
      return;
    }
  percent = ((5 - (GET_AC(victim) / 10)) << 1) + number(1, 101);
  if (IS_MOUNTED(ch))
    percent += 5;               /* harder on horseback, but does more dam */

  if (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_NOBASH))
    percent += 25;

  prob = subcmd?131:GET_SKILL(ch, SKILL_CHARGE);

  if (percent > prob)
    {
      damage(ch, victim, 0, SKILL_CHARGE);
      if (!IS_MOUNTED(ch))
        GET_POS(ch) = POS_SITTING;
      update_pos(ch);
    }
  else
    {
      int mountdam = 0;
      if (IS_MOUNTED(ch))
    mountdam = 50;
      damage(ch, victim,
         mountdam +(2*(dice(wielded->obj_flags.value[1],
                wielded->obj_flags.value[2]))),
         SKILL_CHARGE);
      if (!subcmd)
        improve_skill(ch, SKILL_CHARGE);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


/* **************************************************************************
   NAME       : do_smackheads
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_smackheads)
{
  struct char_data *victim1, *victim2;
  byte percent, prob;
  int average_ac;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  half_chop(argument,arg, arg2);

  if (!GET_SKILL(ch, SKILL_SMACKHEADS))
    {
      send_to_char("The only heads you're gonna smack are yours and "
           "Rosie's.\n\r", ch);
      return;
    }

  if (!(victim1 = get_char_room_vis(ch, arg))
      || !(victim2 = get_char_room_vis(ch, arg2))||(victim1==victim2))
    {
      send_to_char("Looks like the gangs not all here...\n\r", ch);
      return;
    }

  if (victim1 == ch || victim2 == ch)
    {
      send_to_char("We call that 'headbutt' around here, son...\n\r", ch);
      return;
    }

  if (victim1==ch && victim2== ch)
    {
      send_to_char("You smack your head twice and say 'DOH!'\r\n",ch);
      return;
    }

  if (ch->equipment[WEAR_WIELD])
    {
      send_to_char("You need your hands free to smack some heads!\n\r", ch);
      return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (FIGHTING(ch))
    {
      stc("You're a little busy right now!\r\n", ch);
      return;
    }

  if (FIGHTING(victim1) || FIGHTING(victim2))
  {
    stc("They are too busy fighting at the moment!\r\n", ch);
    return;
  }

  if(!*arg && !*arg2)
    {
      send_to_char("Smack whose heads together?",ch);
      return;
    }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
  {
    stc("You can't commit acts of violence here!\r\n", ch);
    return;
  }

  average_ac = ((GET_AC(victim1) + GET_AC(victim2)) / 2);
  percent = ((5 - (average_ac / 10)) << 1) + number(1, 101);
  prob = GET_SKILL(ch, SKILL_SMACKHEADS);

  if (percent > prob)
    {
      strcpy(buf, GET_NAME(victim1));
      strcat(buf, " and ");
      strcat(buf, GET_NAME(victim2));
      strcat (buf, " slip out of your hands!");
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
      strcpy(buf, GET_NAME(victim1));
      strcat(buf, " and ");
      strcat(buf, GET_NAME(victim2));
      strcat (buf, " duck as ");
      strcat (buf, GET_NAME(ch));
      strcat (buf, " lunges at them!");
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      improve_skill(ch, SKILL_SMACKHEADS);
      damage(ch, victim1, 0, SKILL_SMACKHEADS);
      damage(ch, victim2, 0, SKILL_SMACKHEADS);
    }
  else
    {
      strcpy(buf, "You grab the heads of ");
      strcat(buf, GET_NAME(victim1));
      strcat(buf, " and ");
      strcat(buf, GET_NAME(victim2));
      strcat(buf, " and bang them together with a sickening *SMACK*.");
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
      strcpy(buf,GET_NAME(ch));
      strcat(buf, " grabs the heads of ");
      strcat(buf, GET_NAME(victim1));
      strcat(buf, " and ");
      strcat(buf, GET_NAME(victim2));
      strcat(buf, " and bangs them together with a sickening *SMACK*.");
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(victim1) = POS_STUNNED;
      GET_POS(victim2) = POS_STUNNED;
      update_pos(victim1);
      update_pos(victim2);
      damage(ch, victim1, 3*GET_LEVEL(ch), SKILL_SMACKHEADS);
      damage(ch, victim2, 3*GET_LEVEL(ch), SKILL_SMACKHEADS);
      WAIT_STATE(victim1, PULSE_VIOLENCE * 3);
      WAIT_STATE(victim2, PULSE_VIOLENCE * 3);
      improve_skill(ch, SKILL_SMACKHEADS);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
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
        REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_VAMPIRE);
      if (PLR_FLAGGED(victim, PLR_WEREWOLF))
        REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_WEREWOLF);
      sprintf(buf, "%s %sd %s at %s.",
          GET_NAME(ch), weapon, GET_NAME(victim),
          world[victim->in_room].name);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      GET_PKS(ch)++;
      GET_DEATHS(victim)++;
      raw_kill(victim, TYPE_UNDEFINED);
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
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
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

      damage(ch, victim, MIN(15,GET_LEVEL(ch)), SKILL_BITE);
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
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
      damage(ch, victim, MIN(15,GET_LEVEL(ch)), SKILL_BITE);
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
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
void
full_moon(void)
{
  struct descriptor_data *i;
  ACMD(do_transform);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character )
      if (PLR_FLAGGED(i->character, PLR_VAMPIRE) ||
      PLR_FLAGGED(i->character, PLR_WEREWOLF))
    if (!IS_AFFECTED(i->character, AFF_VAMPIRE) &&
        !IS_AFFECTED(i->character, AFF_WEREWOLF))
      {
        send_to_char("The lunar light infuses your body, forcing you to "
             "transform!\r\n", i->character);
        send_to_char("Racked with the pain of the transformation, your "
             "head is thrown\r\nback and an unearthly moan escapes"
             " your lips.\r\n", i->character);
        do_transform(i->character, "", 0, 0);
      }
}


/* **************************************************************************
   NAME       : update_review()
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
void
update_review(struct char_data *speaker, char *gossip)
{
  int count = 0;

  if (!speaker || !gossip)
    return;

  for (count = 24; count > 0; count--)
    {
      /* move each item in review up one; remove the last one */
      review[count].invis = review[count - 1].invis;
      strcpy(review[count].name, review[count - 1].name);
      strcpy(review[count].string, review[count - 1].string);
    }
  strcpy(review[0].string, gossip);
  strcpy(review[0].name, GET_NAME(speaker));
  review[0].invis = GET_INVIS_LEV(speaker);
}


/* **************************************************************************
   NAME       : do_review
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_review)
{
  int count = 0;

  strcpy(buf, "Last Gossips:\r\n-------------\r\n");
  for (count = 24; count >= 0; count--)
    {
      if (strcmp(review[count].name,"\0"))/* i.e name != NULL */
    {
      strcat(buf, CCCYN(ch, C_CMP));
      if (review[count].invis <= GET_LEVEL(ch))
        strcat(buf, review[count].name);
      else
        strcat(buf, "Someone invisible");
      strcat(buf, ": ");
      strcat(buf, CCYEL(ch, C_CMP));
      strcat(buf, review[count].string);
      strcat(buf, CCNRM(ch, C_CMP));
      strcat(buf, "\n\r");
    }
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
  struct char_file_u chdata;
  extern char *class_abbrevs[];

  one_argument(argument, arg);
  if (!*arg)
    {
      send_to_char("For whom do you wish to search?\r\n", ch);
      return;
    }
  if (load_char(arg, &chdata) < 0)
    {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  sprintf( buf, "[%2d %s] %s %s\r\n",
       (int)chdata.level, class_abbrevs[(int)chdata.class],
       chdata.name, chdata.title );
  send_to_char(buf, ch);
}


/* **************************************************************************
   NAME       : do_strike
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_strike)
{
  struct char_data *vict = NULL;
  int percent = number(1, 101); /* 101% is a complete failure */
  int prob = GET_SKILL(ch, SKILL_STRIKE);

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_STRIKE)) {
    stc("Yeah, right.\r\n", ch);
    return;
  }

  if (!*arg && FIGHTING(ch))
    vict = FIGHTING(ch);
  else if (!*arg && !vict)
    {
      send_to_char("Strike who?\r\n", ch);
      return;
    }
  if (!vict && (!(vict = get_char_room_vis(ch, arg))) )
    {
      send_to_char("They don't seem to be here.\r\n", ch);
      return;
    }
  if (vict == ch)
    {
      send_to_char("You beat yourself about the face and neck.\r\n", ch);
      act("$n slaps $mself around a little.", FALSE, ch, 0, vict, TO_ROOM);
      return;
    }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    {
      act("$N is just such a good friend, you simply can't strike $M.",
      FALSE, ch, 0, vict, TO_CHAR);
      return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (GET_POS(vict) <= POS_SLEEPING)
    prob=100;

  if (percent < prob)
    {
      damage(ch, vict, GET_LEVEL(ch)*.65, SKILL_STRIKE);
      improve_skill(ch, SKILL_STRIKE);
    }
  else
    damage(ch, vict, 0, SKILL_STRIKE);

  WAIT_STATE(ch, PULSE_VIOLENCE + 2);
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
  int percent = number(1, 101); /* 101% is a complete failure */
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

  if ((GET_CLASS(ch) != CLASS_NINJA) && (GET_LEVEL(ch) < LEVEL_IMMORT))
    {
      send_to_char("You know nothing of kuji-kiri!\r\n", ch);
      return;
    }

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
      affect_join(ch, af+i, FALSE, FALSE, FALSE, FALSE);

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
void
flesh_alter_from(struct char_data *ch)
{
  ch->points.hitroll -= (GET_LEVEL(ch)/3)+1;
  ch->points.damroll -= (GET_LEVEL(ch)/2)+1;
}


/* **************************************************************************
   NAME       : flesh_alter_to()
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
void
flesh_alter_to(struct char_data *ch)
{
  ch->points.hitroll += (GET_LEVEL(ch)/3)+1;
  ch->points.damroll += (GET_LEVEL(ch)/2)+1;
}


/* **************************************************************************
   NAME       : flesh_altered_type()
   PURPOSE    :
   RETURNS    : hit_type based on what weapon it is.
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970322
   OTHER      :
   ************************************************************************ */
int
flesh_altered_type(struct char_data *ch)
{
  switch (GET_LEVEL(ch))
    {
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
char *
flesh_alter_weapon(struct char_data *ch)
{
  switch (GET_LEVEL(ch))
    {
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
  int percent = 0, prob = 0;

  if (!GET_SKILL(ch, SKILL_FLESH_ALTER))
    {
      send_to_char("You know nothing of altering your flesh!\n\r", ch);
      return;
    }

  percent = number(0, 101 + (FIGHTING(ch)?10:0));
  prob = GET_SKILL(ch, SKILL_FLESH_ALTER);
  if (percent > prob)
    {
      send_to_char("You lose your concentration!\r\n", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
      improve_skill(ch, SKILL_FLESH_ALTER);
      return;
    }
  weapon = flesh_alter_weapon(ch);
  if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
    {
      send_to_char("You shift your molecules back to normal.\r\n", ch);
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_FLESH_ALTER);
      flesh_alter_from(ch);
      sprintf(to_vict, "Your hand reverts from a %s.\r\n", weapon);
      sprintf(to_room, "$n's hand reverts from a %s!", weapon);
    }
  else
    {
      struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);
      SET_BIT_AR(AFF_FLAGS(ch), AFF_FLESH_ALTER);
      flesh_alter_to(ch);
      if (weap)
    {
      obj_to_char(unequip_char(ch, WEAR_WIELD), ch);
      act("You stop using $p.", FALSE, ch, weap, 0, TO_CHAR);
      act("$n stops using $p.", TRUE, ch, weap, 0, TO_ROOM);
    }
      sprintf(to_vict, "Your hand turns into a %s!\r\n", weapon);
      sprintf(to_room, "$n's hand turns into a %s!", weapon);
    }
  FREE(weapon);

  send_to_char(to_vict, ch);
  act(to_room, TRUE, ch, 0, 0, TO_ROOM);
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
  int diff = 0, percent, prob;
  int where1=0, where2=0;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  half_chop(argument,arg, arg2);

  if (!GET_SKILL(ch, SKILL_APPRAISE))
    prob = 20 + GET_LEVEL(ch);
  else
    prob = GET_SKILL(ch, SKILL_APPRAISE);

  if (IS_AFFECTED(ch, AFF_BLIND)) {
     send_to_char("You can't see a damned thing!\r\n", ch);
     return;
  }

  if ( !(obj1 = get_obj_in_list_vis(ch, arg, ch->carrying)) ||
       !(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)) ||
       FIGHTING(ch) )
    {
      if (FIGHTING(ch))
    {
      send_to_char("You're pretty busy right now!\n\r", ch);
      return;
    }
      else
    {
      send_to_char("Looks like you don't have those objects..\n\r", ch);
      return;
    }
    }

  if (obj1 == obj2)
    {
      send_to_char("They're the same thing!\n\r", ch);
      return;
    }

  if(!*arg && !*arg2)
    {
      send_to_char("Compare what and what?\r\n",ch);
      return;
    }

  if (GET_OBJ_TYPE(obj1) != GET_OBJ_TYPE(obj2))
    {
      stc("You can't compare those things!\r\n", ch);
      return;
    }

  if (GET_OBJ_TYPE(obj1) != ITEM_WEAPON && GET_OBJ_TYPE(obj1) !=
      ITEM_ARMOR)
    {
      stc("Compare is only for weapons and armor.\r\n", ch);
      return;
    }

  if (GET_OBJ_TYPE(obj1) == ITEM_ARMOR)
    {
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

      if (where1 != where2)
      {
        stc("You can only compare the same types of armor!\r\n", ch);
        return;
      }
  }

  percent = number(1, 101); /* 101% is a complete failure */

  if (GET_OBJ_TYPE(obj1) == ITEM_WEAPON)
    diff = ( (((GET_OBJ_VAL(obj1, 2) + 1) / 2.0) * GET_OBJ_VAL(obj1, 1)) -
       (((GET_OBJ_VAL(obj2, 2) + 1) / 2.0) * GET_OBJ_VAL(obj2, 1)) );

  if (GET_OBJ_TYPE(obj1) == ITEM_ARMOR)
    diff = ( ((GET_OBJ_VAL(obj1, 0) + 1) / 2.0) - ((GET_OBJ_VAL(obj2, 0) + 1) / 2.0));


  if (percent>prob)
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
  char arg1[MAX_INPUT_LENGTH];
  int percent = number(1, 101); /* 101% is a complete failure */
  int prob = GET_SKILL(ch, SKILL_PALM);

  one_argument(argument, arg1);

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    send_to_char("Your arms are already full!\r\n", ch);
  else if (!*arg1)
    send_to_char("Palm what?\r\n", ch);
  else
    get_from_room(ch, arg1, prob>percent);
}


/* **************************************************************************
   NAME       : do_berserk
   PURPOSE    :
   RETURNS    :
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970128
   OTHER      :
   ************************************************************************ */
ACMD(do_berserk)
{
  struct affected_type af;
  byte percent, prob;
  int failed = FALSE;

  if (!GET_SKILL(ch, SKILL_BERSERK))
    {
      send_to_char("You're about as berserk as you're gonna get.\r\n", ch);
      if (!subcmd)
        return;
    }

  percent = number(1, 101); /* 101% is a complete failure */

  if (IS_AFFECTED(ch, AFF_BERSERK))
    {
      send_to_char("You're unable to summon your battle rage right now.\r\n",
           ch);
      return;
    }

  if (GET_LEVEL(ch) > LEVEL_IMMORT)
    percent = 0;

  prob = subcmd?number(50,100):GET_SKILL(ch, SKILL_BERSERK);

  if (percent > prob)
    {
      send_to_char("You fail to summon up your battle rage.\r\n", ch);
      failed = TRUE;
    }
  else
    send_to_char("Your vision turns sanguine as you summon up "
         "your battle rage!\r\n", ch);
    improve_skill(ch, SKILL_BERSERK);

  af.modifier  = failed ? 0 : 2;
  af.duration  = 1;
  af.type      = SKILL_BERSERK;
  af.location  = APPLY_HITROLL;
  af.bitvector = AFF_BERSERK;
  affect_to_char(ch, &af);

  af.modifier  = failed ? 0 : 2;
  af.location  = APPLY_DAMROLL;
  affect_to_char(ch, &af);

  af.modifier  = failed ? 0 : 25;
  af.location  = APPLY_AC;
  affect_to_char(ch, &af);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
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
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IT);
  if (PLR_FLAGGED(vict, PLR_IT))
    REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_IT);

  SET_BIT_AR(PLR_FLAGS(vict), PLR_IT);

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
      WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    }
}

/* do_scan function is obsolete as of 19980803  -rparet */

ACMD(do_scan)
{
   int dir, found = FALSE;
   if (!((GET_LEVEL(ch)>=LEVEL_IMMORT) ||
        (world[ch->in_room].sector_type==SECT_CITY)||
        (world[ch->in_room].sector_type==SECT_FIELD)||
        (world[ch->in_room].sector_type==SECT_INSIDE)||
        ((world[ch->in_room].sector_type==SECT_FOREST&&
          GET_RACE(ch)==RACE_MINOTAUR))||
        ((world[ch->in_room].sector_type==SECT_FOREST&&
          GET_RACE(ch)==RACE_ELF))||
        ((world[ch->in_room].sector_type==SECT_MOUNTAIN&&
          GET_RACE(ch)==RACE_DWARF))||
        (((world[ch->in_room].sector_type==SECT_WATER_SWIM ||
           world[ch->in_room].sector_type==SECT_WATER_NOSWIM) &&
          GET_RACE(ch)==RACE_SSAUR))||
        ((world[ch->in_room].sector_type==SECT_DESERT&&
          GET_RACE(ch)==RACE_RAKSHASA))))
   {
     send_to_char("You're unable to see very far...\r\n", ch);
     return;
   }

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
                ( (IS_SET_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT) &&
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
                ( (IS_SET_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT) &&
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

ACMD(do_parry)
{
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_PARRY))
    {
      send_to_char("You're not good enough at swordplay to parry!\r\n", ch);
      if (!subcmd)
        return;
    }

  if (!FIGHTING(ch))
    {
      send_to_char("But you aren't fighting anyone!\r\n", ch);
      return;
    }
  if (FIGHTING(FIGHTING(ch))!=ch)
    {
      send_to_char("But noone's attacking you!\r\n", ch);
      return;
    }

  if (!GET_EQ(ch, WEAR_WIELD))
  {
    send_to_char ("Parry with what? You're unarmed!\r\n", ch);
    return;
  }

  percent = number(1, 101);   /* 101% is a complete failure */
  prob = subcmd?number(50,100):GET_SKILL(ch, SKILL_PARRY);

  if (percent > prob)
    {
      send_to_char("With a dazzling show of swordplay, you attempt "
           "to parry...but are outmaneuvered!\r\n", ch);
      if (!subcmd)
         improve_skill(ch, SKILL_PARRY);
      WAIT_STATE(ch, PULSE_VIOLENCE *3);
      return;
    }

  send_to_char("With a dazzling show of swordplay, you move into "
               "defensive position...\r\n", ch);
  act("$n displays a dazzling show of swordplay, fending off $N's every blow!",
    TRUE, ch, 0, FIGHTING(ch), TO_ROOM);
  act("$n displays a dazzling show of swordplay, fending off your every blow!",
    TRUE, ch, 0, FIGHTING(ch), TO_VICT);
  IS_PARRIED(FIGHTING(ch)) = TRUE;
  WAIT_STATE(ch, PULSE_VIOLENCE *2);
}

ACMD(do_circle)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf)))
    {
      if (!(vict = FIGHTING(ch)))
    {
          send_to_char("Circle who?\r\n", ch);
          return;
    }
    }
  if (vict == ch)
    {
      send_to_char("How can you stab yourself in the back?\r\n", ch);
      return;
    }
  if ( FIGHTING(ch) && FIGHTING(FIGHTING(ch)) == ch)
   {
    send_to_char("You're a little too busy right now!\r\n", ch);
    return;
   }
  if (!GET_EQ(ch, WEAR_WIELD))
    {
      send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
      return;
    }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)
    {
      send_to_char("Only piercing weapons can be used for backstabbing.\r\n",
           ch);
      return;
    }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict))
    {
      act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
      act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
      act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
      if (!FIGHTING(vict))
        hit(vict, ch, TYPE_UNDEFINED);
      return;
    }
  if (GET_SKILL(ch, SKILL_CIRCLE)<=0)
  {
    send_to_char("You make a circle in the air.\r\n", ch);
    return;
  }

  percent = number(1, 101); /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_CIRCLE);

  if (AWAKE(vict) && (percent > prob))
  {
    if (FIGHTING(vict))
    {
    stop_fighting(vict);
    hit(vict, ch, TYPE_UNDEFINED);
    }
    damage(ch, vict, 0, SKILL_CIRCLE);
  }
  else
    {
      hit(ch, vict, SKILL_CIRCLE);
      improve_skill(ch, SKILL_CIRCLE);
    }
  WAIT_STATE(ch, PULSE_VIOLENCE + 2);
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

ACMD(do_groinrip)
{
  struct char_data *victim;
  char name[256];
  byte percent;

  if (!ch->desc && !subcmd)
    return;

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
    {
      stc("You cannot commit acts of violence here!\r\n", ch);
      return;
    }

  if (!GET_SKILL(ch, SKILL_GROINRIP))
  {
    send_to_char("You're not trained in martial arts!\n\r", ch);
    if (!subcmd)
      return;
  }

  if (IS_MOUNTED(ch))
    {
      stc("Dismount first!\r\n", ch);
      return;
    }

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name)))
    {
      if (FIGHTING(ch))
    victim = FIGHTING(ch);
      else
    {
      send_to_char("Groinrip who?\n\r", ch);
      return;
    }
    }

  if (victim == ch)
    {
      send_to_char("No masochism allowed!\r\n", ch);
      return;
    }

  if (is_shopkeeper(victim))
    {
      stc("Ha Ha. Don't think so.\r\n", ch);
      return;
    }

  if ((GET_LEVEL(victim) >= LEVEL_IMMORT) && (!IS_NPC(victim)))
    {
      send_to_char("How dare you try to touch a god!\r\n", ch);
      send_to_char("You are thrown across the room...\r\n", ch);
      GET_POS(ch) = POS_SITTING;
      update_pos(ch);
      return;
    }

  if (GET_SEX(victim) != SEX_MALE)
     {
    stc("Umm, they have nothing there to tug on!\r\n", ch);
    return;
     }

  percent = number(1, 121); /* 101% is a complete failure */

  if ((GET_POS(victim) <= POS_SLEEPING) ||
      (GET_LEVEL(ch) > LEVEL_IMMORT))
    percent = 0;

  if (percent < (subcmd?number(50, 100):GET_SKILL(ch, SKILL_GROINRIP)))
    {
      damage(ch, victim, GET_LEVEL(ch), SKILL_GROINRIP);
      if (victim)
        act("$n falls to $s knees, clutching $s groin and throwing up\r\n"
        "everywhere!", TRUE, victim, 0, 0, TO_ROOM);
      if (!number(0,10))
    {
      struct obj_data *obj = read_object(21, VIRTUAL); /*puke*/
      if (obj)
      {
        obj_to_room(obj, ch->in_room);
        GET_OBJ_TIMER(obj)=2;
      }
    }
      improve_skill(ch, SKILL_GROINRIP);
  }
  else
      damage(ch, victim, 0, SKILL_GROINRIP);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void
lunar_hunter(void)
{
  ACMD(do_tell);
  struct descriptor_data *i;
  struct char_data *mob = NULL;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character)
    if (GET_LEVEL(i->character)>12 &&
        GET_LEVEL(i->character)<LVL_IMMORT &&
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

void
ghost_ship_appear(void)
{
   int dock = real_room(19173), ship = real_room(19100);

   if (mini_mud)
     return;

   if (!number(0,1))
    dock = real_room(19174);

   if (!dock || !ship)
    return;

   CREATE(world[dock].dir_option[NORTH], struct room_direction_data, 1);
   world[dock].dir_option[NORTH]->to_room = ship;
   CREATE(world[ship].dir_option[SOUTH], struct room_direction_data, 1);
   world[ship].dir_option[SOUTH]->to_room = dock;

   send_to_room("Suddenly a ghostly ship appears to the north!\r\n", dock);
   send_to_room("Suddenly a dock appears to the south!\r\n", ship);
}

void
ghost_ship_disappear(void)
{
   int dock = real_room(19173), ship = real_room(19100),
       dock2 = real_room(19174);

   if (mini_mud)
     return;

   if (!dock || !ship || !dock2)
    return;

   if (world[dock].dir_option[NORTH])
   {
    FREE(world[dock].dir_option[NORTH]);
    world[dock].dir_option[NORTH] = NULL;
    send_to_room("Suddenly the ghostly ship to the north disappears!\r\n",
    dock);
   }
   if (world[dock2].dir_option[NORTH])
   {
    FREE(world[dock2].dir_option[NORTH]);
    world[dock2].dir_option[NORTH] = NULL;
    send_to_room("Suddenly the ghostly ship to the north disappears!\r\n",
    dock);
   }
   if (world[ship].dir_option[SOUTH])
   {
    FREE(world[ship].dir_option[SOUTH]);
    world[ship].dir_option[SOUTH] = NULL;
    send_to_room("Suddenly the dock to the south disappears!\r\n", ship);
    send_to_room("The ghost ship has set sail!\r\n", ship);
   }
}


ACMD(do_sharpen)
{
   int i = 0, percent = 0, prob = 0;

   struct obj_data *obj = NULL;

   half_chop(argument, arg, buf);

   if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)))
   {
    stc("Sharpen what?\r\n", ch);
    return;
   }
   if (GET_OBJ_TYPE(obj) != ITEM_WEAPON ||
       GET_OBJ_VAL(obj, 3) != TYPE_SLASH - TYPE_HIT)
   {
      send_to_char("This weapon can not be sharpened.\n\r", ch);
      return;
   }
   for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE ||
          IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC))
   {
      send_to_char("This weapon can not be sharpened any further.\n\r",ch);
      return;
   }
   if (FIGHTING(ch))
   {
      send_to_char("You're too busy to be sharpening anything!\n\r",ch);
      return;
   }
   percent = number(1, 101);    /* 101% is a complete failure */
   prob = GET_SKILL(ch, SKILL_SHARPEN);

   if (percent < prob)
   {
      obj->affected[0].location = APPLY_DAMROLL;
      obj->affected[0].modifier = 1+(GET_LEVEL(ch)==30)+(GET_LEVEL(ch)>25);
      act("$n sharpens $p to perfection.", FALSE, ch, obj, 0, TO_ROOM);
      stc("You sharpen it to perfection!\r\n", ch);
   }
   else
   {
      obj->affected[0].location = APPLY_DAMROLL;
      obj->affected[0].modifier = -1;
      act("$n damages $p trying to sharpen it!", FALSE, ch, obj, 0, TO_ROOM);
      stc("You damage it trying to sharpen it!\r\n", ch);
      improve_skill(ch, SKILL_SHARPEN);
   }
}
