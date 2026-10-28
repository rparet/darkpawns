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

/* CVS: $Id: spec_procs3.c 1242 2005-11-07 11:47:28Z mcooper $ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "events.h"
#include "screen.h"
#include "shop.h"

/*   external vars  */
extern char *hometowns[];
extern int mini_mud;
extern struct shop_cost_data shop_cost[];

/* extern functions */
ACMD(do_tell);
ACMD(do_say);
ACMD(do_look);
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);
void player_buy_shop(struct char_data *ch, int homet, int type);
void shop_display_costs(struct char_data *ch, int homet);

SPECIAL(clerk)
{
  struct char_data *mobile = (struct char_data *)me;
  struct obj_data *deed;
  char msg[256];
  int homet, buy, i;
  int zone = world[ch->in_room].zone;
  char *mob_race_name(int mob_race);

  if (!cmd || FIGHTING(ch) || !AWAKE(ch))
    return(FALSE);

  switch (zone_table[zone].number) {
   case 80:  homet = 0; break;
   case 182: homet = 1; break;
   case 212: homet = 2; break;
   case 48:  homet = 3; break;
   case 53:  homet = 4; break;
   default:
     stc("There is a problem with your clerk, please tell a god", ch);
     return (FALSE);
   }

  if (!(CMD_IS("list") || CMD_IS("buy")))
    return(FALSE);                          

  skip_spaces(&argument);
  two_arguments(argument, buf1, buf2);

  if(!CAN_SEE(mobile, ch)) {
    act("$n exclaims, 'Who's there? I can't see you!'", TRUE, mobile, 0, 0, TO_ROOM);
    return(TRUE);
  }                  

  if (CMD_IS("buy")) {
    if (!*argument) {
      sprintf(msg, "%s You can buy CITIZENSHIP, a SHOP, or a PARDON if you're interested.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
    } else if (!strcmp(buf1, "citizenship")) {          /* Buy citizenship */
      if (GET_GOLD(ch) < (GET_LEVEL(ch) * 1000)) {      /* Not enough gold */
        sprintf (msg, "%s You cannot afford it!", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
      } else if (ch->player.hometown == homet) {
        sprintf (msg, "%s You are already a citizen here!", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
      } else {                                          /* Change hometown */
        ch->player.hometown = homet;
        GET_GOLD(ch) -= GET_LEVEL(ch) * 1000;
        sprintf(msg, "%s You are now a citizen of %s.", GET_NAME(ch), hometowns[homet]);
        sprintf(buf, "%s has changed citizenship to %s.", GET_NAME(ch), hometowns[homet]);
        mudlog(buf, BRF, LVL_IMMORT, TRUE);
        do_tell(mobile, msg, find_command("tell"), 0);
        save_char(ch, NOWHERE);
      }
    } else if (!strcmp(buf1, "shop")) {                 /* Buy a store */
      if (!*buf2) {
        act("$n pulls out a scroll and hands it to you. It states:\r\n", TRUE, mobile, 0, ch, TO_VICT);
        shop_display_costs(ch, homet);
      } else if (!is_number(buf2)) {                    /* No store number specified */
        sprintf(msg, "%s Which type of shop do you wish to buy?", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
      } else {
        if (ch->player.hometown != homet) {             /* Not a citizen */
          sprintf(msg, "%s You need to be a citizen before you can purchase a store here!", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
        } else if (GET_GOLD(ch) < shop_cost[homet].cost) {  /* Not enough gold */
          sprintf(msg, "%s You do not have the gold necessary to purchase a store!", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
        } else if (!shop_cost[homet].for_sale) {        /* No stores left */
          sprintf(msg, "%s Unfortunately, we have no more shops available for purchase.\r\n", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
        } else if (((buy = atoi(buf2)) < 1) || (buy > NUM_SHOP_TYPES)) {
          sprintf(msg, "%s Which type of shop do you wish to buy?", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
        } else {
          sprintf(msg, "%s Congratulations on your purchase!", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
          act("$n hands $N the deed to one of the city's shops.", TRUE, mobile, 0, ch, TO_NOTVICT);
          act("$n hands you the deed to your new store.", TRUE, mobile, 0, ch, TO_VICT);
          deed = read_object(1222, VIRTUAL, NULL);
          obj_to_char(deed, ch);
          GET_OBJ_VAL(deed, 0) = --buy;   /* Set the store type bought */
          GET_OBJ_VAL(deed, 1) = homet;   /* Set the hometown it can be used in */
          GET_OBJ_VAL(deed, 2) = GET_IDNUM(ch);  /* ID of the player buying */
          GET_OBJ_VAL(deed, 3) = (shop_cost[homet].cost / 10000); /* Cost of the store */
          sprintf(msg, "%s Use this scroll when you locate an empty store and it shall be yours."
            " Return it to me if you wish to have your money refunded.", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
          GET_GOLD(ch) -= shop_cost[homet].cost;
        }
      }
    } else if (!strcmp(buf1, "pardon")) {
      for (i = 0, buy = 0; i < 5; i++)
        if (GET_RACE_HATE(ch, i) != -1)
          buy++;

      if (!buy) {
        sprintf(msg, "%s You have no racial hatreds, you do not require a pardon.", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
      } else if (!*buf2) {
        sprintf(msg, "%s Which racial hatred did you want a pardon for?", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
      } else if (!str_cmp(buf2, "all")) {
        if (GET_GOLD(ch) < buy * 10000) {
          sprintf (msg, "%s You cannot afford it!", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
        } else {
          for (i = 0; i < 5; i++) {
            GET_RACE_KILLS(ch, GET_RACE_HATE(ch, i)) = 0;
            GET_RACE_HATE(ch, i) = -1;
          }
          GET_GOLD(ch) -= buy * 10000;
          sprintf(msg, "%s Very well, I shall speak on your behalf and clear your good name.", GET_NAME(ch));
          do_tell(mobile, msg, find_command("tell"), 0);
        }
      } else {
        for (i = 0; i < 5; i++)
          if (!str_cmp(buf2, mob_race_name(GET_RACE_HATE(ch, i)))) {
            if (GET_GOLD(ch) < 10000) {
              sprintf (msg, "%s You cannot afford it!", GET_NAME(ch));
              do_tell(mobile, msg, find_command("tell"), 0);
              return (TRUE);
            } else {
              GET_RACE_KILLS(ch, GET_RACE_HATE(ch, i)) = 0;
              race_hate_sort(ch);
              GET_GOLD(ch) -= 10000;
              sprintf(msg, "%s Very well, I shall speak on your behalf and clear your good name.", GET_NAME(ch));
              do_tell(mobile, msg, find_command("tell"), 0);
              return (TRUE);
            }
          }
        sprintf(msg, "%s Which racial hatred did you want a pardon for?", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
      }
    } else {
      sprintf(msg, "%s You can buy CITIZENSHIP, a SHOP, or a PARDON if you're interested.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
    }
  }

  if (CMD_IS("list")) {
    sprintf(msg, "%s Citizenship costs %d coins", GET_NAME(ch), GET_LEVEL(ch) * 1000);
    for (i = 0, buy = 0; i < 5; i++)
      if (GET_RACE_HATE(ch, i) != -1)
        buy++;
    if (buy)
      sprintf(msg + strlen(msg), " and a pardon costs %d coins.", buy * 10000);
    else
      strcat(msg, ".");
    strcat(msg, " Typing 'buy shop' will detail store costs.");
    do_tell(mobile, msg, find_command("tell"), 0);
  }

  return (TRUE);
}

SPECIAL(vitae_seller)
{
  struct char_data *mobile = (struct char_data *)me;
  char msg[256];

  long exp_needed_for_level(struct char_data *ch);

  if (!cmd || FIGHTING(ch) || !AWAKE(ch))
    return (FALSE);

  skip_spaces(&argument);
  if( (CMD_IS("buy") || CMD_IS("list")) && !CAN_SEE(mobile, ch)) {
    act("$n exclaims, 'Who's there? I can't see you!'", TRUE, mobile, 0, 0, TO_ROOM);
    return (TRUE);
  }

  if (!(CMD_IS("list") || CMD_IS("buy")))
    return (FALSE);

  if (CMD_IS("buy")) {
    if (!argument || (argument && strcmp(argument, "vitae"))) {
      sprintf (msg, "%s BUY VITAE, if you really want to do it.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    } else {
      if (!GET_VITAE(ch)) {
        sprintf (msg, "%s You don't have any vitae points to restore!", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }

      if (GET_EXP_Q(ch) < GET_VITAE_EXP(ch)) {
        sprintf(msg, "%s You don't have the required unassigned experience.", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }

      if (GET_GOLD(ch) < GET_LEVEL(ch) * 500) {
        sprintf(msg, "%s You don't have enough gold to pay for my services!", GET_NAME(ch));
        do_tell(mobile, msg, find_command("tell"), 0);
        return (TRUE);
      }

      GET_GOLD(ch) -= GET_LEVEL(ch) * 500;
      GET_EXP_Q(ch) -= GET_VITAE_EXP(ch);
      sprintf (msg, "%s That'll be %d coins, you should feel much better.. if you wake up.",
        GET_NAME(ch), GET_LEVEL(ch) * 500);
      do_tell(mobile, msg, find_command("tell"), 0);
      act("$n stares at $N and mutters some arcane words.", FALSE, mobile, 0, ch, TO_NOTVICT);
      act("$N falls, stunned.", FALSE, mobile, 0, ch, TO_NOTVICT);
      send_to_char("You fall to the ground, stunned.\r\n", ch);
      GET_POS(ch) = POS_STUNNED;

      vitae_af(ch, FALSE);
      GET_VITAE(ch)--;
      GET_VITAE_EXP(ch) = 0;
      vitae_af(ch, TRUE);
      if (GET_VITAE(ch))            /* We've still got one left, reset the exp */
        GET_VITAE_EXP(ch) = (long)((exp_needed_for_level(ch) - GET_EXP(ch)) * 0.1);

      return (TRUE);
    }
  }

  if (CMD_IS("list")) {
    if (!GET_VITAE(ch)) {
      sprintf (msg, "%s You don't have any vitae points to restore!", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return (TRUE);
    }

    sprintf (msg, "%s You can buy a vitae point for %ld of unassigned experience and %d coins.",
      GET_NAME(ch), GET_VITAE_EXP(ch), GET_LEVEL(ch) * 500);
    do_tell(mobile, msg, find_command("tell"), 0);
    return (TRUE);
  }

  return(FALSE);
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
           mob = read_mobile(23, VIRTUAL, NULL);
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
                 if ((IS_SET(ROOM_FLAGS(teleport), ROOM_PRIVATE)) ||
                     (IS_SET(ROOM_FLAGS(teleport), ROOM_GODROOM)) ||
                     (IS_SET(ROOM_FLAGS(teleport), ROOM_NOMAGIC)) ||
                     (IS_SET(ROOM_FLAGS(teleport), ROOM_DEATH)) ) {
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
  char *temp_name, msg[256];
  int cost = 0, level_multiplier = 116;
  long room_vnum;

  extern struct obj_data *object_list; 
  long sql_find_obj(char *searchname);

  if (!cmd)
    return FALSE;

  temp_name = GET_NAME(ch);
  skip_spaces(&temp_name);
  cost = GET_LEVEL(ch) * level_multiplier;

  if (CMD_IS("list")) {
    sprintf(msg, "%s It will cost %d coins to retrieve your corpse.", GET_NAME(ch), cost);
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;    
  }

  if (CMD_IS("retrieve")) {
    if(GET_GOLD(ch) < cost) {
      sprintf (msg, "%s I'm sorry, you can't afford the cost.", GET_NAME(ch));
      do_tell(mobile, msg, find_command("tell"), 0);
      return TRUE;
    }

    if ((room_vnum = sql_find_obj(temp_name)))    /* Check the database first */
      if (!ZONE_FLAGGED(world[real_room(room_vnum)].zone, ZONE_UNLOAD))
        reload_room_descs(world[real_room(room_vnum)].zone);

    for (obj = object_list; obj; obj = obj->next)    
      if (isname(temp_name, obj->name) && IS_CORPSE(obj))
        if (obj->in_room != ch->in_room) {
          obj_from_room(obj);
          obj_to_room(obj, ch->in_room);
          act("The Mortician dumps your corpse on the ground.", 0, ch, 0,0,TO_CHAR);
          act("The Mortician dumps $n's corpse on the ground.", TRUE, ch, 0, 0, TO_ROOM);
          GET_GOLD(ch) = GET_GOLD(ch) - cost;
          return TRUE;
        }

    sprintf(msg, "%s I'm sorry, I can't find your corpse anywhere!", GET_NAME(ch));
    do_tell(mobile, msg, find_command("tell"), 0);
    return TRUE;
  } 
  return FALSE;
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
    
/***
 These are Matt's SpecProcs
 ***/

/* ********************************************************************
*  Generic (Used in multiple Areas)                                   *
********************************************************************* */

SPECIAL(fall_down)
{
  /* Attached to room 2202 - if player can't fly, they fall into the room below. */
  /* Do not affect Imms or anyone with a FLY flag. Check to see that the room is */
  /* not a tunnel. */

  int exit = EXIT(ch, 5)->to_room;

  if ((GET_LEVEL (ch) > LVL_IMMORT) || (AFF_FLAGGED(ch, AFF_FLY)))
    return (FALSE);

  if (ROOM_FLAGGED(exit, ROOM_TUNNEL) && num_pc_in_room(&(world[exit])) > 0)
    return (FALSE);

  sprintf(buf, "%ld", GET_ROOM_VNUM(exit));
  run_script(ch, ch, NULL, &world[ch->in_room], buf, "code_one", LT_ROOM);
  return (FALSE);
}

SPECIAL(fall_and_dropeq)
{
  /* Attached to room 19184 - if player can't fly, they fall into the room below. */
  /* Do not affect Imms or anyone with a FLY flag. Check to see that the room is */
  /* not a tunnel. */

  int exit = EXIT(ch, 5)->to_room;

  if ((GET_LEVEL (ch) >= LVL_IMMORT) || (AFF_FLAGGED(ch, AFF_FLY)))
    return (FALSE);

  if (ROOM_FLAGGED(exit, ROOM_TUNNEL) && num_pc_in_room(&(world[exit])) > 0)
    return (FALSE);

  sprintf(buf, "%ld", GET_ROOM_VNUM(exit));
  run_script(ch, ch, NULL, &world[ch->in_room], buf, "code_two", LT_ROOM);
  return (FALSE);
}
