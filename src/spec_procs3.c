
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

/* CVS: $Id: spec_procs3.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int top_of_world;
extern struct str_app_type str_app[];
extern struct dex_skill_type dex_app_skill[];
extern char *pc_class_types[];
extern char *hometowns[];
extern char *tattoos[];
extern struct tat_data tat[];
extern int mini_mud;
extern field_object_data_t field_objs[NUM_FOS];

/* extern functions */
void raw_kill(struct char_data * ch, int attacktype);
void send_to_zone(char *messg, struct char_data *ch);
void add_follower(struct char_data * ch, struct char_data * leader);
ACMD(do_action);
ACMD(do_behead);
bool can_speak(struct char_data *ch);
void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont);
ACMD(do_action);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_get);
ACMD(do_tell);
ACMD(do_say);
ACMD(do_stand);
ACMD(do_wake);
ACMD(do_flee);
ACMD(do_look);
ACMD(do_backstab);
ACMD(do_spike);
SPECIAL(shop_keeper);
ACMD(do_steal);
struct char_data *HUNTING( struct char_data *ch);

SPECIAL(clerk)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[256];
  int homet = 0;
  int zone = world[ch->in_room].zone;

  if (!cmd || FIGHTING(ch) || !AWAKE(ch))
        return(FALSE);

  switch (zone_table[zone].number) {
   case 80:
    homet = 1;
    break;
   case 182:
    homet = 2;
    break;
   case 212:
    homet = 3;
    break;
   default:
    send_to_char("default case reached in clerk special - tell a god",
     ch);
  }


  if (!(CMD_IS("list") || CMD_IS("buy")))
        return(FALSE);

  skip_spaces(&argument);
  if( (CMD_IS("buy") || CMD_IS("list")) && !CAN_SEE(mobile, ch))
  {
    act("$n exclaims, 'Who's there? I can't see you!'",
        TRUE, mobile, 0, 0, TO_ROOM);
    return(TRUE);
  }
  if (CMD_IS("buy"))
   if (!argument || (argument && strcasecmp(argument, "citizenship")))
   {
      sprintf (msg, "%s BUY CITIZENSHIP, if you're interested.",
                   GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }
  if (CMD_IS("list")) {
    sprintf (msg, "%s Citizenship costs 2,000 coins.", GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return (TRUE);
  } else {
     if (GET_GOLD(ch) < 2000) {
      sprintf (msg, "%s You cannot afford it!", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
      }
     if (ch->player.hometown == homet) {
        sprintf (msg, "%s You are already a citizen here!", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
     } else
        ch->player.hometown = homet;
     GET_GOLD(ch) = GET_GOLD(ch) - 2000;
     sprintf (msg, "%s You are now a citizen of %s.", GET_NAME(ch),
      hometowns[homet]);
     do_tell(mobile, msg, find_command("tell"), 0);
     save_char(ch, NOWHERE);
     return TRUE;
    }
}

SPECIAL(butler)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *next_obj=NULL, *obj = NULL;
  struct obj_data *cas = get_obj_in_list_vis(mobile, "case",
			 world[mobile->in_room].contents);
  struct obj_data *cabinet = get_obj_in_list_vis(mobile, "cabinet",
			 world[mobile->in_room].contents);
  struct obj_data *chest = get_obj_in_list_vis(mobile, "chest",
			 world[mobile->in_room].contents);
  int got = 0;

  if( cmd || !AWAKE(mobile) || FIGHTING(mobile) ||
      !chest || !cas || !cabinet)
	return FALSE;

  for (obj = world[mobile->in_room].contents; got<4 && obj; obj = next_obj)
  {
    next_obj = obj->next_content;
    if (CAN_GET_OBJ(mobile, obj))
    {
	got++;
	act("$n gets $P.", TRUE, mobile, 0, obj, TO_ROOM);
	obj_from_room(obj);
	obj_to_char(obj, mobile);
        if (GET_OBJ_TYPE(obj) == ITEM_ARMOR ||
	    GET_OBJ_TYPE(obj) == ITEM_WORN)
	{
          do_gen_door(mobile, "case", 0, SCMD_OPEN);
	  perform_put(mobile, obj, cas);
	}
        else if (GET_OBJ_TYPE(obj) == ITEM_WEAPON ||
		 GET_OBJ_TYPE(obj) == ITEM_FIREWEAPON)
	{
          do_gen_door(mobile, "cabinet", 0, SCMD_OPEN);
	  perform_put(mobile, obj, cabinet);
	}
	else
	{
          do_gen_door(mobile, "chest", 0, SCMD_OPEN);
	  perform_put(mobile, obj, chest);
	}
    }
  }
  if (got)
    {
	do_gen_door(mobile, "case", 0, SCMD_CLOSE);
	do_gen_door(mobile, "cabinet", 0, SCMD_CLOSE);
	do_gen_door(mobile, "chest", 0, SCMD_CLOSE);
	return(TRUE);
    }
  return(FALSE);
}

SPECIAL(brain_eater)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *i = NULL;

  if (FIGHTING(ch) || cmd || !AWAKE(ch) || GET_HIT(ch) < 0)
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content)
    {
      if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3) &&
          strstr(i->name, "corpse") && !strstr(i->name, "headless"))
        {
	  do_behead(mobile, "corpse", 0, 0);
	  act("$n pulls the brain out of the head and eats it with a noisy"
		"\r\nslurp, blood and drool flying everywhere.",
		TRUE, mobile, NULL, NULL, TO_ROOM);
	  if (GET_LEVEL(mobile)<30)
	    GET_LEVEL(mobile)++;
	  else
	    GET_DAMROLL(mobile)+=2;
          return (TRUE);
	}
    }
   return(FALSE);
}

SPECIAL(teleport_victim)
{
  if (cmd || !FIGHTING(ch) || !AWAKE(ch))
    return(FALSE);

  do_action(ch, GET_NAME(FIGHTING(ch)), find_command("scoff"), 0);
  if (can_speak(ch))
	act("$n says, 'You can't harm me, mortal. Begone.'",
		TRUE, ch, 0, 0, TO_ROOM);

  call_magic(ch, FIGHTING(ch), 0, SPELL_TELEPORT, GET_LEVEL(ch), CAST_SPELL);
  return(TRUE);
}

SPECIAL(con_seller)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[256];

  if (!cmd || FIGHTING(ch) || !AWAKE(ch))
	return(FALSE);

  skip_spaces(&argument);
  if( (CMD_IS("buy") || CMD_IS("list")) && !CAN_SEE(mobile, ch))
  {
    act("$n exclaims, 'Who's there? I can't see you!'",
	TRUE, mobile, 0, 0, TO_ROOM);
    return(TRUE);
  }

  if (!(CMD_IS("list") || CMD_IS("buy")))
    return(FALSE);

  if (CMD_IS("buy"))
   if (!argument || (argument && strcasecmp(argument, "con")))
   {
      sprintf (msg, "%s BUY CON, if you really want to do it.",
		   GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }

  if (CMD_IS("list"))
    {
      if (GET_ORIG_CON(ch)-(ch)->real_abils.con<1)
      {
        sprintf (msg, "%s You seem perfectly healthy!", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }
      sprintf (msg, "%s You can buy up to %d point%s, at %d per point.",
		GET_NAME(ch), GET_ORIG_CON(ch)-(ch)->real_abils.con,
		GET_ORIG_CON(ch)-(ch)->real_abils.con>1?"s":"",
		GET_LEVEL(ch)*400);
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }
  else
    {
      if (GET_GOLD(ch) < GET_LEVEL(ch)*400)
	{
	  sprintf (msg, "%s You can't afford it!", GET_NAME(ch));
	  do_tell(mobile, msg, find_command("tell"), 0);
	  return (TRUE);
	}
      else
	{
      	  if (GET_ORIG_CON(ch)-(ch)->real_abils.con<1)
      	  {
           sprintf (msg, "%s You seem perfectly healthy!", GET_NAME(ch));
           do_tell(mobile, msg, find_command("tell"), 0);
           return (TRUE);
          }
          GET_GOLD(ch) -= GET_LEVEL(ch)*400;
	  sprintf (msg, "%s That'll be %d coins, you should feel "
	           "much better.. if you wake up.",
		   GET_NAME(ch), GET_LEVEL(ch)*400);
	  do_tell(mobile, msg, find_command("tell"), 0);
          act("$n stares at $N and mutters some arcane words.",
              FALSE, mobile, 0, ch, TO_NOTVICT);
          act("$N falls, stunned.", FALSE, mobile, 0, ch, TO_NOTVICT);
          if ((ch)->real_abils.con<18)
	        ch->real_abils.con++;
	  affect_total(ch);
	  GET_POS(ch)= POS_STUNNED;
	  return(TRUE);
	}
    }
    return(FALSE);
}

SPECIAL(no_move_down)
{
    struct char_data *mobile = (struct char_data *)me;

    if (!cmd || mini_mud || !IS_MOVE(cmd) || !AWAKE(mobile))
       return FALSE ;

    if (GET_LEVEL(ch) >= LVL_IMMORT || HUNTING(ch))
        return FALSE;

    if (CMD_IS("down") && GET_LEVEL(ch) < LEVEL_IMMORT )
       {
           act("$n blocks $N's way.", FALSE, mobile, 0, ch, TO_NOTVICT) ;
           act("$n blocks your way.", FALSE, mobile, 0, ch, TO_VICT) ;
           act("$n says 'Thou shalt not pass.'",
                FALSE, mobile, 0, 0, TO_ROOM) ;
           return TRUE ;
       }
    return FALSE ;
}


 /* Hey! Don't go calling this w/o making checks! */
static void
npc_regen(struct char_data * ch) {
  int regen_rate = 2;
  GET_HIT(ch) += GET_LEVEL(ch) * regen_rate;
  if(GET_HIT(ch) > GET_MAX_HIT(ch))
     GET_HIT(ch) = GET_MAX_HIT(ch);
}

SPECIAL(troll)
{
  if (cmd || !AWAKE(ch) || GET_HIT(ch)<=0)
     return FALSE;

  if (GET_POS(ch) != POS_FIGHTING && GET_HIT(ch) != GET_MAX_HIT(ch)) {
     if(!number(0, 20)) {
        npc_regen(ch);
        act("$n's wounds glow brightly for a moment, then disappear!",
		TRUE, ch, 0, 0, TO_ROOM);
     }
  }
  else if (FIGHTING(ch)) {
     if(!number(0, 10)) {
        npc_regen(ch);
        act("$n's wounds glow brightly for a moment, then disappear!",
		TRUE, ch, 0, 0, TO_ROOM);
     }
  }
  else
     return FALSE;

  return TRUE;
}

SPECIAL(quan_lo)
{
 struct char_data *mobile = (struct char_data *)me;
 if(cmd && AWAKE(mobile))
 {
   char msg[256];
   if (CMD_IS("flee") || CMD_IS("retreat") || CMD_IS("escape"))
   {
      sprintf(msg, "What was that, %s? This is not a shawade. Try it again. "
		   "This time with fewing.", GET_NAME(ch));
      do_gen_comm(mobile, msg, 0, SCMD_GOSSIP);
   }

   skip_spaces(&argument);

   if(CMD_IS("look") || CMD_IS("examine"))
     if(argument && isname(argument, mobile->player.name))
     {
       sprintf(msg, "$n says, 'What is it you seek, %s? Tell me and be gone.'",
		GET_NAME(ch));
       act(msg, TRUE, mobile, 0, 0, TO_ROOM);
     }
  }

  return(FALSE);
}



SPECIAL(alien_elevator)
{
  struct char_data *tch = NULL, *next_tch = NULL;

  if(argument)
  {
    skip_spaces(&argument);
    if (CMD_IS("close") && argument && (!strcasecmp(argument, "door")) &&
	!IS_SET(world[(ch)->in_room].dir_option[EAST]->exit_info, EX_CLOSED))
     {
      int to_room = 0, from_room = ch->in_room;
      to_room = (from_room == real_room(19551))?19599:19551;
      for(tch = world[from_room].people; tch; tch = next_tch)
      {
	next_tch = tch->next_in_room;
	char_from_room(tch);
	char_to_room(tch, real_room(to_room));
	stc("The room starts to move!\r\n", tch);
      }
      return(TRUE);
     }
  }
  return(FALSE);
}


SPECIAL(werewolf)
{
   struct char_data *mob = (struct char_data *)me, *vict = NULL;

   if (!cmd && FIGHTING(mob) && mob)
   {
      vict = FIGHTING(mob);
      if (!number(0, 9) && mob && (GET_HIT(mob) > 0))
      {
         act("$n looks up and lets out a long, fierce howl.",
		TRUE, mob, 0, 0, TO_ROOM);
         send_to_zone("You hear a loud howling in the distance.", mob);
      }
      if (!number(0, 3) && mob && (GET_HIT(mob) > 0))
      {
         act("$n tears into your leg with $s huge fangs!",
		TRUE, mob, 0, vict, TO_VICT);
         act("$n rips apart $N's leg with $s fangs!",
		TRUE, mob, 0, vict, TO_NOTVICT);
         damage(mob, vict, dice(GET_LEVEL(mob), 2), TYPE_BITE);
         GET_MOVE(vict) -= GET_LEVEL(mob) * 1.5;
         if (GET_MOVE(vict) < 0)
           GET_MOVE(vict) = 0;
      }
      return TRUE;
   }
   return FALSE;
}

SPECIAL(field_object)
{
   int i = 0, index = 0, dam = 0, level = 0, affect = 0;
   bool damaged = FALSE;
   struct obj_data *obj = (struct obj_data *)me;
   struct char_data *vict = NULL, *next_vict = NULL;

   if (cmd || obj->in_room <= 0)
	return FALSE;

   for (i = 0; i < NUM_FOS; i++)
      if (field_objs[i].obj_vnum == GET_OBJ_VNUM(obj))
      {
	 index = i;
	 break;
      }

   if (index >= NUM_FOS)
      return(FALSE);

   if (field_objs[i].fo_type == FO_DAMAGE)
      dam = dice(GET_OBJ_VAL(obj, 0), GET_OBJ_VAL(obj, 1));
   else if (field_objs[i].fo_type == FO_AFFECT)
   {
      affect = GET_OBJ_VAL(obj, 0);
      level = GET_OBJ_VAL(obj, 1);
   }

   for (vict = world[obj->in_room].people; vict; vict = next_vict)
   {
      next_vict = vict->next_in_room;
      if (dam)
      {
         GET_HIT(vict)-=dam;
         if (obj->action_description)
        	act(obj->action_description, FALSE, vict, obj, vict, TO_VICT);
	 else
		stc("An incredible force hits you!\r\n", ch);

         if (GET_HIT(vict)<=0)
         {
            act ("$N falls to the ground, screaming in agony!",
                 TRUE, vict, 0, vict, TO_NOTVICT);
            raw_kill(vict, TYPE_UNDEFINED);
	 }
         damaged = TRUE;
      }
      if (vict && vict->in_room>0 && affect)
      {
         if (obj->action_description)
        	act(obj->action_description, FALSE, vict, obj, vict, TO_VICT);
	 call_magic(vict, vict, 0, affect, level, CAST_SPELL);
         damaged = TRUE;
      }

   }
   return (damaged);
}

SPECIAL(portal_to_temple)
{
   int location = 0;
   if(mini_mud || ( !CMD_IS("say") && !CMD_IS("'") ) )
        return(FALSE);

   location=real_room(8008);    /*destination room*/

   skip_spaces(&argument);
   if (strcasecmp(argument,"setchswayno"))
      return(FALSE);

   do_say(ch, argument, 0, 0);

   stc("With a blinding flash of light and a crack of thunder, you are"
       " teleported...\r\n", ch);
   act ("With a blinding flash of light and a crack of thunder, $n "
        "disappears!", TRUE, ch, NULL, NULL, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, location);
   act ("With a blinding flash of light and a crack of thunder, $n "
        "appears!", TRUE, ch, NULL, NULL, TO_ROOM);
   do_look(ch,"",0,0);

   return(TRUE);
}


SPECIAL(turn_undead)
{
  struct obj_data *obj = (struct obj_data *)me;
  int this_room = real_room(19875);
  int that_room = real_room(19876);

  if (CMD_IS("use") && (ch->in_room == this_room ||
			ch->in_room == that_room ))
  {
    skip_spaces(&argument);
    if (isname(argument, obj->name))
    {
	act("A ray of flame bursts out of $p, consuming the undead!", FALSE,
		ch, obj, 0, TO_ROOM);

        CREATE(world[this_room].dir_option[NORTH],
		struct room_direction_data, 1);
   	world[this_room].dir_option[NORTH]->to_room = that_room;
   	CREATE(world[that_room].dir_option[SOUTH],
		struct room_direction_data, 1);
   	world[that_room].dir_option[SOUTH]->to_room = this_room;
	return(TRUE);
    }
  }
  if(!cmd)
  {
   if (world[this_room].dir_option[NORTH])
   {
    FREE(world[this_room].dir_option[NORTH]);
    world[this_room].dir_option[NORTH] = NULL;
   }
   if (world[that_room].dir_option[SOUTH])
   {
    FREE(world[that_room].dir_option[SOUTH]);
    world[that_room].dir_option[SOUTH] = NULL;
   }
  }
  return(FALSE);
}

SPECIAL(itoh)
{
   int location = 0;
   if(mini_mud || ( !CMD_IS("say") && !CMD_IS("'") ) )
        return(FALSE);

   location=real_room(19875);    /*destination room*/

   skip_spaces(&argument);
   if (strcasecmp(argument,"itoh"))
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


SPECIAL(mirror)
{
   struct obj_data *obj = (struct obj_data *)me;
   struct char_data *ch2;

   if (obj->in_room>=0)
   	ch2 = world[real_room(14496)].people;
   else
	return(FALSE);

   skip_spaces(&argument);
   if (isname(argument, obj->name))
   {
      if (CMD_IS("hit") || CMD_IS("kill"))
      {
         act("You break $p into tiny pieces!", 0, ch, obj, 0, TO_CHAR);
         act("$n shatters $p into a million pieces!",
		TRUE, ch, obj, 0, TO_ROOM);
         if (ch2)
         {
            char_from_room(ch2);
            char_to_room(ch2, obj->in_room);
            act("You feel pulled in a hundred different directions!",
		FALSE, ch2, 0, 0, TO_CHAR);
            act("$n appears in a brilliant flash!", TRUE, ch2, 0, 0, TO_ROOM);
         }
         obj_to_room(read_object(14503, VIRTUAL), obj->in_room);
	 extract_obj(obj);
	 if (ch2)
	   look_at_room(ch2, 0);
         return(TRUE);
      }
      if (CMD_IS("look") )
      {
         act("You feel pulled in a hundred different directions!",
		FALSE, ch, 0, 0, TO_CHAR);
         act("$n disappears in a brilliant flash!", FALSE, ch, 0, 0, TO_ROOM);
         if (ch2)
         {
            char_from_room(ch2);
            char_to_room(ch2, obj->in_room);
            act("You feel pulled in a hundred different directions!",
		FALSE, ch2, 0, 0, TO_CHAR);
            act("$n appears in a brilliant flash!", TRUE, ch2, 0, 0, TO_ROOM);
         }
         char_from_room(ch);
         char_to_room(ch, real_room(14496));
	 look_at_room(ch, 0);
  	 if (ch2)
	   look_at_room(ch2, 0);
         return(TRUE);
      }
   }
   return(FALSE);
}



SPECIAL(prostitute)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[256];

  if (!cmd || FIGHTING(ch) || !AWAKE(ch))
	return(FALSE);

  if( (CMD_IS("buy") || CMD_IS("list")) && !CAN_SEE(mobile, ch))
  {
    act("$n says, 'If I could see you, we could do business..'",
	TRUE, mobile, 0, 0, TO_ROOM);
    act("$n winks coyly.", TRUE, mobile, 0, 0, TO_ROOM);
    return(TRUE);
  }

  if (!(CMD_IS("list") || CMD_IS("buy")))
    return(FALSE);

  if (CMD_IS("buy"))
   {
      sprintf (msg, "%s I ain't for sale, just rent. "
	       "Give me 5 gold for a good time.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
   }

  if (CMD_IS("list"))
    {
      sprintf (msg, "%s For five coins, I'll show you a good time.",
		GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }
  return(FALSE);
}

SPECIAL(roach) {
   struct obj_data *i = NULL;
   int teleport;
   struct char_data *mob = NULL;

   if (ch == NULL)
      return FALSE;

   if (cmd || !AWAKE(ch))
      return (FALSE);

   if (!number(0,10000))
     if (!number(0,10000))
       if (GET_MAX_HIT(ch) < 11) {
         act ("$n seems to starve to death and simply fades out of existence.",
              FALSE, ch, 0, 0, TO_ROOM);
         /* char_from_room(ch); */
         extract_char(ch);
         return TRUE;
       }

   /* first see if there is something to eat */
   for (i = world[ch->in_room].contents; i; i = i->next_content) {
      if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
        continue;
      act("$n feeds on $p.", FALSE, ch, i, 0, TO_ROOM);
      if (!number(0,2)) {
        GET_MAX_HIT(ch) += (int) GET_OBJ_COST(i) / 2;
        if (!number(0,2)) ch->mob_specials.damnodice += 1;
        if (!number(0,2)) ch->mob_specials.damsizedice += 1;
        /* if the roach grows too big, split it */
        if (GET_MAX_HIT(ch) > 400) {
           GET_MAX_HIT(ch) = 10;
           GET_HIT(ch) = 10;
           ch->mob_specials.damnodice = 2;
           ch->mob_specials.damsizedice = 4;
           mob = read_mobile(23, VIRTUAL);
           char_to_room(mob, ch->in_room);
           IS_CARRYING_W(mob) = 0;
           IS_CARRYING_N(mob) = 0;
           act("$n splits in half forming a new roach!",
               FALSE, ch, 0, 0, TO_ROOM);
           GET_MAX_HIT(mob) = 10;
           GET_HIT(mob) = 10;
           ch->mob_specials.damnodice = 2;
           ch->mob_specials.damsizedice = 4;
        } else {
        /* otherwise just do an act */
           if (number(0,1))
             act("You hear some stretching noises.", FALSE, ch, 0, 0, TO_ROOM);
           else
             act("You hear a strange rumbling from $n's stonach.",
                 FALSE, ch, 0, 0, TO_ROOM);
        }
     } else {
        act("You hear $n burp.", FALSE, ch, 0, 0, TO_ROOM);
     }
     /* obj_from_room(i); */
     extract_obj(i);
     return TRUE;
  }

  switch(number(0,10)) {
     case 0:  act("$n chirps gleefully.", FALSE, ch, 0, 0, TO_ROOM);
              break;
     case 1:  act("$n changes colors and clicks happily.",
                  FALSE, ch, 0, 0, TO_ROOM);
              break;
     case 2:  act("$n skitters around in tight circles.",
                  FALSE, ch, 0, 0, TO_ROOM);
              break;
     case 3:  act("Strange purple dots appear on $n's back.",
                  FALSE, ch, 0, 0, TO_ROOM);
              break;
     case 4:  if (!number(0,5)) {
                 teleport = number(0, top_of_world);
                 if ((IS_SET_AR(ROOM_FLAGS(teleport), ROOM_PRIVATE)) ||
                     (IS_SET_AR(ROOM_FLAGS(teleport), ROOM_GODROOM)) ||
                     (IS_SET_AR(ROOM_FLAGS(teleport), ROOM_NOMAGIC)) ||
                     (IS_SET_AR(ROOM_FLAGS(teleport), ROOM_DEATH)) ) {
                    act("$n fades out and back in again.",
                        FALSE, ch, 0, 0, TO_ROOM);
                    return FALSE;
                 }
                 act("$n fades out slowly with a soft swoosh.",
                     TRUE, ch, 0, 0, TO_ROOM);
                 char_from_room(ch);
                 char_to_room(ch, teleport);
                 act("$n fades in slowly, looking a bit disoriented.",
                     TRUE, ch, 0, 0, TO_ROOM);
                 look_at_room(ch, 0);
                 return TRUE;
              }
              break;
     default:
              break;
   }
   return FALSE;
}

SPECIAL(mortician)
{
 struct char_data *mobile = (struct char_data *)me;
 struct obj_data *obj;
 extern struct obj_data *object_list;
 char *temp_name;
 char msg[256];
 int cost = 0;
 int level_multiplier = 116;

 if(!cmd)
  return FALSE;

 temp_name = GET_NAME(ch);
 skip_spaces(&temp_name);
 cost = GET_LEVEL(ch) * level_multiplier;

 if (CMD_IS("list")) {
    sprintf (msg, "%s It will cost %d coins to retrieve your corpse.",
     GET_NAME(ch), cost);
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
 }
 if (CMD_IS("retrieve")) {
    if(GET_GOLD(ch) < cost) {
      sprintf (msg, "%s I'm sorry, you can't afford the cost.",
       GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return TRUE;
    }
    for (obj = object_list; obj; obj = obj->next)
     if(isname(temp_name, obj->name) && GET_OBJ_VAL(obj, 3) &&
        (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)) {
       obj_from_room(obj);
       obj_to_room(obj, ch->in_room);
       act("The Mortician dumps your corpse on the ground.", 0,
        ch, 0,0,TO_CHAR);
       act("The Mortician dumps $n's corpse on the ground.", TRUE,
        ch, 0, 0, TO_ROOM);
       GET_GOLD(ch) = GET_GOLD(ch) - cost;
       return TRUE;
     }
    sprintf (msg, "%s I'm sorry, I can't find your corpse anywhere!",
     GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
 }
 return FALSE;

}


SPECIAL(conjured) {

   struct char_data *mob = (struct char_data *) me;
   struct char_data *player = mob->master;
   ACMD(do_say);

   if (!IS_AFFECTED(mob, AFF_CHARM)) {
      switch(GET_MOB_VNUM(mob)) {
      case 81:
      case 82:
      case 83:
      case 84:
         if (player)
         {
           sprintf(buf, "You lose control and %s fizzles away!\r\n",
                 GET_NAME(mob));
           send_to_char(buf, player);
         }
         act("$n returns to its own plane of existence.", TRUE, mob, 0, 0, TO_ROOM);
         break;
      default:
         do_say(mob, "My work here is done.", 0 , 0);
         sprintf(buf, "%s disappears in a flash of white light!\r\n",
                 GET_NAME(mob));
         act(buf, FALSE, mob, 0, 0, TO_ROOM);
         break;
      }

      extract_char(mob);
      return TRUE;
   }
   return FALSE;

}

SPECIAL(hisc)
{
  SPECIAL(no_move_south);
  SPECIAL(cleric);

  if (CMD_IS("south"))
    return (no_move_south(ch, me, cmd, NULL));

  return (cleric(ch, me, 0, NULL));
}

SPECIAL(recruiter)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[256];

  if (!cmd)
    return FALSE;

  if (CMD_IS("kill") || CMD_IS("hit"))
  {
    sprintf(msg, "%s Why don't you sign up for training?  Just head south through those doors!",
    GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
  }

  if (CMD_IS("cast") || CMD_IS("will"))
  {
    sprintf(msg, "%s Hey now! None of that voodoo mumbo jumbo in my office!",
    GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
  }

  return FALSE;
}

/* ********************************************************************
*  Elemental Zone (zone 13)                                           *
********************************************************************* */

SPECIAL(elements_master_column)
{
  /* Attached to room 1315. When players move into this room, depending on certain objects */
  /* they are carrying, they will be teleported to various locations within the zone. */

  struct obj_data *obj;
  struct char_data *ppl, *next;
  int location, i, found = 0;
  char *obj_name[] = {"earth", "air", "fire", "water"};
  int has_object[] = {0, 0, 0, 0};
  int new_location[] = {1320, 1331, 1342, 1353, 1372};

  for (ppl = world[ch->in_room].people; ppl; ppl = next)
  {
    next = ppl->next_in_room;                           /* Store the next player */
    found = 0;

    for (obj = ppl->carrying; obj; obj = obj->next_content)
      switch (GET_OBJ_VNUM(obj))                        /* Determine if player is missing a talisman */
      {
        case 1300:   /* Earth talisman */
          has_object[0] = 1;
          break;
        case 1301:   /* Air talisman */
          has_object[1] = 1;
          break;
        case 1302:   /* Fire talisman */
          has_object[2] = 1;
          break;
        case 1303:   /* Water talisman */
          has_object[3] = 1;
          break;
        default:        /* Different object */
          break;
      };

    for (i = 0; i < 4; i++)
      if (has_object[i] == 0)                           /* Teleport the player to the required plane */
      {
        if (i != 0)
          sprintf(buf, "The talisman of %s glows softly and your vision fades. When you wake...\r\n",
                        obj_name[i-1]);
        else
          sprintf(buf, "You feel a tingling sensation and your vision fades. When you wake...\r\n");
        found = i;
        break;
      }
      else
      {
        found++;
        has_object[i] = 0;                              /* Reset for any other players in room */
      }

    if (found == 4)
      sprintf(buf, "The four talismans glow softly and your vision fades. When you wake...\r\n");

    location = real_room(new_location[found]);
    act(buf, FALSE, ppl, 0, 0, TO_CHAR);
    act("$n vanishes in a brilliant flash of light.", TRUE, ppl, 0, 0, TO_NOTVICT);
    char_from_room(ppl);
    char_to_room(ppl, location);
    look_at_room(ppl, 0);
    act("$n appears in a brilliant flash of light.", TRUE, ppl, 0, 0, TO_NOTVICT);
  }

  return (TRUE);
}

SPECIAL(elements_platforms)
{
  /* Attached to rooms 1326, 1337, 1348 and 1359. When a player enters these rooms, they */
  /* are automatically teleported back to room 1314 (base of the initial column). */

  struct char_data *ppl, *next;
  int location = real_room(1314);

  for (ppl = world[ch->in_room].people; ppl; ppl = next)
  {
    next = ppl->next_in_room;                           /* Store the next player */
    sprintf(buf, "A wave of power surges through you and you feel dizzy.");
    act(buf, FALSE, ppl, 0, 0, TO_CHAR);
    act("$n disappears in a brilliant flash of light.", TRUE, ppl, 0, 0, TO_NOTVICT);
    char_from_room(ppl);
    char_to_room(ppl, location);
    act("$n appears in a brilliant flash of light.", TRUE, ppl, 0, 0, TO_NOTVICT);
  }

  return (TRUE);
}

void elements_remove_cylinders(struct char_data *ch)
{
  struct obj_data *obj, *obj_remove = NULL;
  int talisman[] = {1300, 1301, 1302, 1303};
  int cylinder[] = {1304, 1305, 1306, 1307};
  char *cyl_name[] = {"green", "yellow", "red", "blue"};
  int i;

  for (i = 0; i < 4; i++)
  {
    for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
      if (GET_OBJ_VNUM(obj) == talisman[i])             /* Found the talisman, stop looking */
        return;
      else if (GET_OBJ_VNUM(obj) == cylinder[i])        /* Found the cylinder, remember it */
        obj_remove = obj;
      else
        continue;

    if (obj_remove != NULL)
    {
      sprintf(buf, "The %s cylinder of light slowly sinks back into the pillar.\r\n",
                    cyl_name[i]);
      send_to_room(buf, ch->in_room);

      extract_obj(obj_remove);          /* Remove the cylinder from the room */
      obj_remove = NULL;
    }
  }

  return;
}

SPECIAL(elements_load_cylinders)
{
  /* Attached to rooms 1360, 1364, 1380 and 1384. If the player drops the appropriate */
  /* talisman in the appropriate room, load an object to indicate that something has */
  /* occured. When all 4 talismans are in place, anyone entering room 1372 will be */
  /* teleported to room 1382 (another SPEC PROC). */

  struct obj_data *obj;
  int cylinder[] = {1304, 1305, 1306, 1307};
  int i, r_num = 0, location = GET_ROOM_VNUM(ch->in_room);

  ACMD(do_drop);

  if (CMD_IS("get"))
  {
    do_get(ch, argument, cmd, 0);
    elements_remove_cylinders(ch);
    return (TRUE);
  }

  if (!CMD_IS("drop"))
    return (FALSE);

  for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
    for (i = 0; i <4; i++)
      if (GET_OBJ_VNUM(obj) == cylinder[i])
        return (FALSE);                 /* If a talisman has already been dropped, don't do this proc */

  do_drop(ch, argument, cmd, 0);
  argument = one_argument(argument, arg);
  strcpy(buf, "");                      /* Set the buffer to NULL */

  if (!*arg)
    return (TRUE);
  else if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))
    return (TRUE);
  else
    switch (location)
    {
      case 1360:
        if (GET_OBJ_VNUM(obj) == 1300)  /* Earth talisman */
        {
          sprintf(buf, "A green cylinder of light extends upwards from the pillar.\r\n");
          r_num = real_object(cylinder[0]);
        }
        break;
      case 1364:
        if (GET_OBJ_VNUM(obj) == 1301)  /* Air talisman */
        {
          sprintf(buf, "A yellow cylinder of light extends upwards from the pillar.\r\n");
          r_num = real_object(cylinder[1]);
        }
        break;
      case 1380:
        if (GET_OBJ_VNUM(obj) == 1302)  /* Fire talisman */
        {
          sprintf(buf, "A red cylinder of light extends upwards from the pillar.\r\n");
          r_num = real_object(cylinder[2]);
        }
        break;
      case 1384:
        if (GET_OBJ_VNUM(obj) == 1303)  /* Water talisman */
        {
          sprintf(buf, "A blue cylinder of light extends upwards from the pillar.\r\n");
          r_num = real_object(cylinder[3]);
        }
        break;
   }

 if (*buf)                      /* Load the cylinder into the room */
 {
   send_to_room(buf, ch->in_room);
   obj = read_object(r_num, REAL);
   obj_to_room(obj, ch->in_room);
 }

  return (TRUE);
}

SPECIAL(elements_galeru_column)
{
  /* Attached to room 1372. When all 4 talismans have been placed in their appropriate */
  /* corners, any player entering this room will be teleported to the final staging */
  /* area. Do not teleport NPCs. */

  struct char_data *ppl, *next;
  struct obj_data *obj;
  int location, i, found = 0;
  int room[] = {1360, 1364, 1380, 1384};
  int talisman[] = {1300, 1301, 1302, 1303};

  for (i = 0; i < 4; i++)               /* Are the talismans still in the right rooms? */
  {
    location = real_room(room[i]);
    for (obj = world[location].contents; obj; obj = obj->next_content)
      if (GET_OBJ_VNUM(obj) == talisman[i])
      {
        found++;
        break;
      }
  }

  if (found != 4)                               /* If missing, return */
    return (FALSE);

  location = real_room(1389);
  for (ppl = world[ch->in_room].people; ppl; ppl = next)
  {
    next = ppl->next_in_room;           /* Store the next player */
    if (IS_NPC(ppl))
      continue;
    else
    {
      sprintf(buf, "Four beams of colored light from the corners of the chamber converge around you.\r\n\n");
      send_to_char(buf, ppl);
      act("$n is struck by four beams of colored light and slowly vanishes!", TRUE, ppl, 0, 0, TO_NOTVICT);
      char_from_room(ppl);
      char_to_room(ppl, location);
      look_at_room(ppl, 0);
      act("$n materialises from nowhere in a swirl of colors.", TRUE, ppl, 0, 0, TO_NOTVICT);
    }
  }

  return (TRUE);
}

SPECIAL(elements_galeru_alive)
{
  /* Attached to room 1394. If the mob who loads in here is dead, teleport any players */
  /* back to the start of the zone (room 1395). */

  struct char_data *ppl, *next;
  int location = real_room(1395), vnum = 1315, found = 0;

  if (!cmd)
    return (FALSE);

  for (ppl = world[ch->in_room].people; ppl; ppl = ppl->next_in_room)
    if (GET_MOB_VNUM(ppl) == vnum)
      found++;

  if (!found)
  {
    for (ppl = world[ch->in_room].people; ppl; ppl = next)
    {
      next = ppl->next_in_room;
      send_to_char("You begin to feel very dizzy and the world around you fades...\r\n\n", ppl);
      act("$n disappears in a brilliant flash of light.", TRUE, ppl, 0, 0, TO_NOTVICT);
      char_from_room(ppl);
      char_to_room(ppl, location);
      look_at_room(ppl, 0);
      act("$n appears in a brilliant flash of light.", TRUE, ppl, 0, 0, TO_NOTVICT);
    }
    return (TRUE);
  }

  return (FALSE);
}

SPECIAL(elements_minion)
{
  /* Attached to mob 1313 - the mob will scavenge any eq it finds as per the SCAVENGE */
  /* flag and will then destroy any object containing the words contained in the array */
  /* below. Used in an effort to remove the talismans from the zone. */

  struct char_data *mobile = (struct char_data *) me;
  struct obj_data *obj;
  int i = 0;
  char *destroy[] = {"talisman", "element", "earth", "fire", "water", "air", "!"};

  while (*destroy[i] != '!')
  {
    if ((obj = get_obj_in_list_vis(mobile, destroy[i], mobile->carrying)))
    {
      act("$n utters the words 'eradico paratus' and $p disintegrates.", TRUE, mobile, obj, 0, TO_ROOM);
      extract_obj(obj);
      elements_remove_cylinders(mobile);
    }
    i++;
  }

  return (FALSE);
}

SPECIAL(elements_guardian)
{
  /* Attached to mob 1314 - the mob will attempt to "charm" players through song. Any of */
  /* these charmed players will attack another within the room. If they are alone, the */
  /* player will go mad and injure himself/herself. */

  struct char_data *ppl, *next, *mobile = (struct char_data *) me;
  int dam;

  if (!cmd)
    return (FALSE);

  for (ppl = world[ch->in_room].people; ppl; ppl = ppl->next_in_room)
    if ((IS_NPC(ppl)) || (GET_LEVEL (ppl) > LVL_IMMORT) || (FIGHTING(ppl)))
      continue;
    else
    {
      next = ppl->next_in_room;
      if ((!next) || (IS_NPC(next)) || (GET_LEVEL (next) > LVL_IMMORT) || (FIGHTING(next)))
      {
        dam = number(10,50);
        damage(ppl, ppl, dam, TYPE_UNDEFINED);
        act("$n mumbles softly and $N begins screaming loudly, hitting $Mself.",
             TRUE, mobile, 0, ppl, TO_NOTVICT);
        act("$n mumbles softly and you begin to scream, involuntarily hitting yourself.",
             TRUE, mobile, 0, ppl, TO_VICT);
        return (FALSE);
      }
      else
      {
        sprintf(buf, "%s mumbles softly and %s screams loudly, attacking %s!",
                GET_NAME(mobile), GET_NAME(ppl), GET_NAME(next));
        act(buf, TRUE, ppl, 0, next, TO_NOTVICT);
        sprintf(buf, "%s mumbles softly and %s screams loudly, attacking you!",
                GET_NAME(mobile), GET_NAME(ppl));
        act(buf, TRUE, next, 0, 0, TO_CHAR);
        sprintf(buf, "%s mumbles softly and you scream loudly, attacking %s!",
                GET_NAME(mobile), GET_NAME(next));
        act(buf, TRUE, ppl, 0, 0, TO_CHAR);
        hit(ppl, next, TYPE_UNDEFINED);
        return (FALSE);
      }
    }

  return (FALSE);
}

SPECIAL(fly_exit_up)
{
  /* Attached to rooms 2203 and 1389 - if player can't fly, they can't travel upwards */
  /* Do not affect Imms, NPCs or anyone with a FLY flag */

  if ((GET_LEVEL (ch) > LVL_IMMORT) || IS_NPC(ch) || !CMD_IS("up") || (AFF_FLAGGED(ch, AFF_FLY)))
    return (FALSE);

  send_to_char("You try and jump up there but it's just too high.\r\n", ch);
  act("$n jumps up and down in a vain attempt to travel upwards.", TRUE, ch, 0, 0, TO_NOTVICT);

  return (TRUE);
}
