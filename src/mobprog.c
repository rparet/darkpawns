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

/* $Id: mobprog.c 1487 2008-05-22 01:36:10Z jravn $ */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "mobprog.h"
#include "spells.h"

char buf2[MAX_STRING_LENGTH];

/* externals */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern void death_cry (struct char_data *ch);
ACMD(do_action);
ACMD(do_say);
ACMD(do_tell);
ACMD(do_gen_door);
void add_follower(struct char_data * ch, struct char_data * leader);
struct char_data *get_mount(struct char_data *ch);
SPECIAL(prostitute);
SPECIAL(cityguard);
SPECIAL(shop_keeper);
SPECIAL(guild_guard);
SPECIAL(guild);
SPECIAL(butler);
SPECIAL(clerk);

void
mp_greet(struct char_data *who, int room)
{
  struct char_data *tch, *tch_next;
  int cmd_lick, cmd_growl, cmd_say;
  int is_citizen(struct char_data * chChar);
  int is_shopkeeper(struct char_data * chChar);

  cmd_say = find_command("say");
  cmd_lick = find_command("growl");
  cmd_growl = find_command("lick");

  for (tch = world[room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == who)
      continue;
    if (IS_DOG(tch) && !(number(0, 5))) {
      if (GET_ALIGNMENT(who) < 0)
        do_action(tch, GET_NAME(who), cmd_lick, 0);
      else
        do_action(tch, GET_NAME(who), cmd_growl, 0);
    } else if (is_shopkeeper(tch) && IS_DOG(who)) {
      act("$n mutters something about filthy animals in $s shop...",
        TRUE, tch, 0, 0, TO_ROOM);
      hit(tch, who, TYPE_UNDEFINED);
    }
    else if (GET_MOB_VNUM(tch)==19406)
     if (!who->master || who->master==who)/*leader or soloers*/
    act("$n says, 'Have a seat there! Stay a while, rest your bones "
        "and warm your feet by the fire!'", TRUE, tch, 0, 0, TO_ROOM);
  }
}


void
mp_ride_greet(struct char_data *who, int room)
{
  struct char_data *tch, *tch_next;
  int cmd_lick, cmd_growl, cmd_say;

  cmd_say = find_command("say");
  cmd_lick = find_command("growl");
  cmd_growl = find_command("lick");

  for (tch = world[room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == who)
      continue;
  }
}

void
mp_give(struct char_data *ch, struct char_data *mob, struct obj_data *obj)
{
  int is_junk(struct obj_data * i);
  //char tell_buf[MAX_STRING_LENGTH];

  if (IS_DOG(mob) && (GET_OBJ_TYPE(obj) == ITEM_FOOD)) {
    act("$n devours $p and wags $s tail happily.",
        TRUE, mob, obj, 0, TO_ROOM);
    obj_from_char(obj);
    extract_obj(obj);
  } else if (IS_DOG(mob)) {
    act("$n sniffs around and plays with $p for a while.",
        TRUE, mob, obj, 0, TO_ROOM);
    obj_from_char(obj);
    obj_to_room(obj, mob->in_room);
    act("$n quickly loses interest.", TRUE, mob, 0, 0, TO_ROOM);
  }

  if (IS_DEMON(mob)) /* soul eater */
  {
    if (GET_OBJ_VNUM(obj)!=9900)/* soul */
    {
      act("$n peers at $p closely, then hands it back.",
        TRUE, mob, obj, 0, TO_ROOM);
      act("$n growls, 'Are you mocking me?'",
        TRUE, mob, 0, 0, TO_ROOM);
      obj_from_char(obj);
      obj_to_char(obj, ch);
    }
    else
    {
          struct obj_data *portal = read_object(19611, VIRTUAL);
      act("$n peers at the soul, then licks $s lips.",
        TRUE, mob, 0, 0, TO_ROOM);
      act("$n says, 'This will do nicely.. you may enter!'",
        TRUE, mob, 0, 0, TO_ROOM);
      act("$n pops the soul into his mouth and swallows it, as a "
        "hideous\r\n screaming rings in your ears...",
        TRUE, mob, 0, 0, TO_ROOM);
      obj_from_char(obj);
      extract_obj(obj);

          act("$n parts his gnarled hands and a shimmering black portal "
               "materializes before you!", TRUE, mob, 0, 0, TO_ROOM);
          GET_OBJ_VAL(portal, 2) = 2;
          obj_to_room(portal, ch->in_room);
          do_say(mob, "Enter the portal quickly! It will not last long!", 0, 0);
    }
  }
  if (IS_JANITOR(mob)) {
    if (is_junk(obj))
      act("$n says, 'Thanks for helping clean this place up...'",
        TRUE, mob, 0, 0, TO_ROOM);
    else
      act("$n says, 'Wow, this is pretty neat, thanks.'",
        TRUE, mob, 0, 0, TO_ROOM);
  }
}

void
mp_bribe(struct char_data *ch, struct char_data *mob, int amount)
{
  int is_citizen(struct char_data * chChar);
  int is_cityguard(struct char_data * chChar);
  int cmd_bow = find_command("bow");

  if (IS_DOG(mob)) {
    act("$n sniffs the coins and proceeds to eat them.",
        TRUE, mob, 0, 0, TO_ROOM);
    GET_GOLD(mob) -= amount;
  } else if (IS_JANITOR(mob)) {
    act("$n tips his hat and smiles a thank you.", TRUE, mob, 0, 0, TO_ROOM);
  } else if (IS_MERCENARY(mob)) {
    if (amount > 99 && !mob->master) {
      SET_BIT_AR(AFF_FLAGS(mob), AFF_CHARM);
      GET_GOLD(mob) -= amount;
      stc("The mercenary counts the coins then secrets them "
        "away.\r\n", ch);
      stc("The mercenary swears his allegiance to you.\r\n", ch);
      add_follower(mob, ch);
      act("$n hires the mercenary.", FALSE, ch, 0, mob, TO_ROOM);
    } else {
      act ("$n laughs somewhat rudely.", TRUE, mob, 0, 0, TO_ROOM);
    }
  } else if (GET_MOB_VNUM(mob)==8088) {
    if (amount < GET_LEVEL(ch)*GET_LEVEL(ch)) {
    act("$n says, 'Are you trying to bribe me?  That's against"
        " the law you\r\nknow...'\r\n", TRUE, mob, 0, 0, TO_ROOM);
        act("$n says, 'And not quite enough cash, either.'\r\n",
        TRUE, mob, 0, 0, TO_ROOM);
        act("$n grins evilly.", TRUE, mob, 0, 0, TO_ROOM);
        GET_GOLD(mob) -= amount;
    GET_GOLD(ch) += amount;
    }
    else {
    act("$n says, 'Thank you very much, monsieur.'\r\n",
        TRUE, mob, 0, 0, TO_ROOM);
        act("$n says, 'Now get outta here!'\r\n", TRUE, mob, 0, 0, TO_ROOM);
        act("$N throws you out of the cell!", TRUE, ch, 0, mob, TO_CHAR);
        act("$n throws $N out of the cell!", TRUE, mob, 0, ch, TO_NOTVICT);
        GET_GOLD(mob) -= amount;
    char_from_room(ch);
    char_to_room(ch, real_room(8117));
        if (get_mount(ch)) {
           char_from_room(get_mount(ch));
           char_to_room(get_mount(ch), real_room(8117));
        }
    }
  } else if (is_cityguard(mob)) {
    if ((!(number(0, 2)) || (amount < 200))) {
      act("$n says, 'Are you trying to bribe me?  That's against the law "
            "you know...'", TRUE, mob, 0, 0, TO_ROOM);
      hit(mob, ch, TYPE_UNDEFINED);
    } else {
      act("$n glances around warily and says, 'I am off duty now...'",
        TRUE, mob, 0, 0, TO_ROOM);
      act("$n lays down and falls asleep on the job!",
        TRUE, mob, 0, 0, TO_ROOM);
      GET_POS(mob) = POS_SLEEPING;
      GET_GOLD(mob) -= amount;
    }
  } else if (is_citizen(mob)) {
    act("$n says, 'Thanks for the investment, I appreciate it.'",
        TRUE, mob, 0, 0, TO_ROOM);
    do_action(mob, GET_NAME(ch), cmd_bow, 0);
  } else if(IS_WHORE(mob)) {
    if (amount >=5) {
      act ("$n pulls $N into the shadows for a few minutes... you decide"
        " not to watch.", TRUE, mob, 0, ch, TO_NOTVICT);
      act ("$n pulls you into the shadows, and gives you a lot more than"
        " you\r\nexpected for your money.", TRUE, mob, 0, ch, TO_VICT);
      stc ("A few coins lighter and quite a bit happier, you continue"
        " on your way.\r\n", ch);
    }
    else
      act("$n says, 'Thanks hon, but I ain't THAT cheap.'",
        TRUE, mob, 0, 0, TO_ROOM);
  } else if (GET_MOB_VNUM(mob)==13108) {
    if (amount >= 1000)
    {
      act("$n leads $N through a hidden door.", TRUE, mob, 0, ch, TO_NOTVICT);
      char_from_room(ch);
      char_to_room(ch, real_room(13154));

      if (get_mount(ch))
      {
        char_from_room(get_mount(ch));
        char_to_room(get_mount(ch), real_room(13154));
      }
      stc("  You follow the gremlin through a series of tunnels,"
      " full of twists, turns,\r\nand circles. Slowly you become aware"
      " of a tiny crack of light coming through\r\nthe top of the cavern."
      " Suddenly the gremlin turns around and hurries away\r\nbefore"
      " you can even turn around.\r\n\r\n", ch);
    }
    else
    {
      act("$n exclaims, 'Cheap bastard, come back with some money!'",
                TRUE, mob, 0, 0, TO_ROOM);
      act("$N gets kicked out on $S ass!'", TRUE, mob, 0, ch, TO_NOTVICT);
      act("$n kicks you out on your ass!\r\n", TRUE, mob, 0, ch, TO_VICT);
      char_from_room(ch);
      char_to_room(ch, real_room(13193));
    }
    look_at_room(ch, 0);
  }
}


void
entry_prog(struct char_data *mob, int room)
{
  struct char_data *tch;
  struct char_data *tch_next;

  int is_citizen(struct char_data * chChar);
  int is_cityguard(struct char_data * chChar);

  if (room == NOWHERE)
    return;
  /* the captain of the guard */
  if (GET_MOB_VNUM(mob) == 8059)
  {
    for (tch = world[room].people; tch; tch = tch_next)
    {
      tch_next = tch->next_in_room;
      if (tch == mob)
        continue;
      if (is_cityguard(tch))
      {
        if (GET_POS(tch) < POS_STANDING)
        {
          act("$n barks 'On your feet, slacker!'", TRUE, mob, 0, 0, TO_ROOM);
          act("$n wakes up and quickly snaps to attention!",
        TRUE, tch, 0, 0, TO_ROOM);
          act("$n growls, 'Report to my office at 0500 tomorrow morning.'",
        TRUE, mob, 0, 0, TO_ROOM);
          GET_POS(tch) = POS_STANDING;
      break;
        }
        else
        {
          act("$n snaps to attention and salutes!", TRUE, tch, 0, 0, TO_ROOM);
          act("$n growls, 'At ease, soldier.'", TRUE, mob, 0, 0, TO_ROOM);
      break;
        }
      }
      else if (is_citizen(tch))
      {
          act("$n frowns at $N.", TRUE, mob, 0, tch, TO_ROOM);
          act("$n says, 'Hail unto the True One, Captain.'",
        TRUE, tch, 0, 0, TO_ROOM);
      break;
      }
    }
  }
}

int
is_citizen(struct char_data * chChar)
{
  int ch_num;

  if (!IS_NPC(chChar))
    return (FALSE);

  ch_num = GET_MOB_VNUM(chChar);
  switch (ch_num) {
    case 2749:
    case 2750:
    case 8062:
    case 18201:
    case 18202:
    case 21243:
      return (TRUE);
      break;
  }
  return (FALSE);
}

int
is_cityguard(struct char_data * chChar)
{
  int ch_num;

  if (!chChar || !IS_NPC(chChar))
    return FALSE;

  if (GET_MOB_SPEC(chChar) == cityguard)
    return TRUE;

  ch_num = GET_MOB_VNUM(chChar);
  switch(ch_num) {
    case 2747:
    case 8001:
    case 8002:
    case 8020:
    case 8027:
    case 8059:
    case 8060:
    case 12111:
    case 21200:
    case 21201:
    case 21203:
    case 21227:
    case 21228:
      return (TRUE);
      break;
  }
  return (FALSE);
}

struct char_data *
get_bad_guy(struct char_data * chAtChar)
{
  struct char_data *ch;
  int iNum_bad_guys = 0, iVictim;

  for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
    if (
         FIGHTING(ch) &&
     (is_citizen(FIGHTING(ch)) || is_cityguard(FIGHTING(ch))) )
      iNum_bad_guys++;

  if (!iNum_bad_guys)
    return NULL;

  iVictim = number(0, iNum_bad_guys);   /* How nice, we give them a chance */
  if (!iVictim)
    return NULL;

  iNum_bad_guys = 0;

  for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
    if (FIGHTING(ch) &&
    (is_citizen(FIGHTING(ch)) || is_cityguard(FIGHTING(ch))) &&
    ++iNum_bad_guys == iVictim)
      return ch;

  return NULL;
}

/* Makes a character banzaii on attackers of the town citizens / other guards */
int
kill_bad_guy(struct char_data * ch)
{
  struct char_data *chOpponent = NULL;

  if (!AWAKE(ch) || GET_POS(ch) == POS_FIGHTING)
    return FALSE;

  if ((chOpponent = get_bad_guy(ch))) {
    act("$n roars: 'Protect the innocent!  BANZAIIII!  CHARGE!'",
    FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, chOpponent, TYPE_UNDEFINED);
    return TRUE;
  }
  return FALSE;
}


int
npc_rescue(struct char_data * ch_hero, struct char_data * ch_victim)
{
  struct char_data *ch_bad_guy;

  for (ch_bad_guy = world[ch_hero->in_room].people;
       ch_bad_guy && (FIGHTING(ch_bad_guy) != ch_victim);
       ch_bad_guy = ch_bad_guy->next_in_room);
  if (ch_bad_guy) {
    if (ch_bad_guy == ch_hero)
      return FALSE;     /* NO WAY I'll rescue the one I'm fighting! */
    act("You bravely rescue $N.\r\n", FALSE, ch_hero, 0, ch_victim, TO_CHAR);
    act("You are rescued by $N, your loyal friend!\r\n",
    FALSE, ch_victim, 0, ch_hero, TO_CHAR);
    act("$n heroically rescues $N.", FALSE, ch_hero, 0, ch_victim, TO_NOTVICT);

    if (FIGHTING(ch_bad_guy))
      stop_fighting(ch_bad_guy);
    if (FIGHTING(ch_hero))
      stop_fighting(ch_hero);

    set_fighting(ch_hero, ch_bad_guy);
    set_fighting(ch_bad_guy, ch_hero);
    return TRUE;
  }
  return FALSE;
}

int
is_junk(struct obj_data * i)
{
  if (IS_SET_AR(i->obj_flags.wear_flags, ITEM_WEAR_TAKE) &&
      ((GET_OBJ_TYPE(i) == ITEM_DRINKCON) || (GET_OBJ_COST(i) <= 10)))
    return TRUE;
  else
    return FALSE;
}

int
is_shopkeeper(struct char_data * chChar)
{
  int ch_num;

  if (!IS_NPC(chChar))
    return (FALSE);

  if (GET_MOB_SPEC(chChar) == shop_keeper)
    return TRUE;

  if (GET_MOB_SPEC(chChar) == guild)
    return TRUE;

  if (GET_MOB_SPEC(chChar) == guild_guard)
    return TRUE;

  if (GET_MOB_SPEC(chChar) == butler)
    return TRUE;

  if (GET_MOB_SPEC(chChar) == clerk)
    return TRUE;

  ch_num = GET_MOB_VNUM(chChar);
  switch (ch_num) {
    case 8003:
    case 8004:
    case 8005:
    case 8006:
    case 8007:
    case 8008:
    case 8009:
    case 8010:
    case 8011:
        case 8078:
      return (TRUE);
      break;
  }
  return (FALSE);
}

#define MP_NONE 0
#define MP_SPEAK 1
#define MP_EMOTE 2
void
mp_sound(struct char_data *mob)
{
   ACMD(do_echo);
   ACMD(do_say);
   char sound[MAX_STRING_LENGTH];
   int type = MP_NONE;

   switch(GET_MOB_VNUM(mob))
   {
   case 8066: /*Petitioner*/
    if (number(0,1))
       strcpy(sound, "Sign this, please! There's too much violence!");
    else
       strcpy(sound, "You look like a kind person.. sign this petition?");
    type = MP_SPEAK;
    break;
   case 8067: /*carpenter*/
    if (number(0,1))
       strcpy(sound, "adjusts his tool belt.");
    else
       strcpy(sound, "wipes the sweat of labor from his brow.");
    type = MP_EMOTE;
    break;
   case 8068: /*town crier*/
    if (number(0,1))
       strcpy(sound, "Arch Bishop Dinive to arrive on the Day of Winter "
            "Dawning!");
    else
       strcpy(sound, "By mandate of the church, no violence in town! The "
            "penalty is jail time!");
    type = MP_SPEAK;
    break;
   case 8069: /*zealot*/
    strcpy(sound, "Repent sinners! The end time is near!");
    type = MP_SPEAK;
    break;
   case 8071: /*beggar */
    if (number(0,1))
        {
       strcpy(sound, "Spare a coin, buddy?");
       type = MP_SPEAK;
    }
    else
        {
       strcpy(sound, "jingles his cup.");
       type = MP_EMOTE;
    }
    break;
   case 8072: /* singling drunk */
    strcpy(sound, "sings an old war ditty... badly off-key.");
    type = MP_EMOTE;
    break;
   case 8074: /* minstrel */
    if (number(0,1))
       strcpy(sound, "plays a lilting tune about your mother's beauty.");
    else
       strcpy(sound, "sings a melody about your conquests in battle.");
    type = MP_EMOTE;
    break;
   case 8079: /*mime*/
    strcpy(sound, "tries to escape from an invisible box only he can see.");
    type = MP_EMOTE;
    break;
   case 14202: /*Bhang*/
    strcpy(sound, "tokes up on some kind bud.");
    type = MP_EMOTE;
    break;
   case 8059: /*Aversin*/
    do_echo(mob, "looks at you.", find_command("emote"), SCMD_EMOTE);
    strcpy(sound, "Carry on, citizen.");
    type = MP_SPEAK;
    break;
   case 8023: /*elven prostitute*/
    strcpy(sound, "jiggles in your direction.");
    type = MP_EMOTE;
    break;
   case 16300: /* KD recruiter */
        if (number(0, 1))
        {
          strcpy(sound, "smiles at you.");
          type = MP_EMOTE;
        }
        else
        {
          strcpy(sound, "shuffles some papers around on his desk.");
          type = MP_EMOTE;
        }
        break;
   default: break;
   }
   if (type == MP_SPEAK)
    do_say(mob, sound, 0, 0);
   if (type == MP_EMOTE)
    do_echo(mob, sound, find_command("emote"), SCMD_EMOTE);

  if (IS_DEMON(mob))
  {
    if (number(0,2))
      return;
    else if (GET_ALIGNMENT(mob) == -1000)
    {
      do_say(mob, "I seek the dull blackened stones in which the souls of mortals have been trapped!", 0, 0);
      GET_ALIGNMENT(mob) = -999;
      return;
    }
    else if (GET_ALIGNMENT(mob) == -999)
    {
      do_say(mob, "I shall open a portal to the Grey Fortress in exchange for a soul stone.", 0, 0);
      GET_ALIGNMENT(mob) = -1000;
      return;
    }
  }

  if (IS_DOG(mob))
  {
     if (!number(0,25))
     {
          struct obj_data *tobj = NULL;
      do_echo(mob, "relieves itself, nearly hitting your foot.",
        find_command("emote"), SCMD_EMOTE);
          if (!(tobj = read_object(20, VIRTUAL)))
           {
                log("SYSERR: creating puddle: obj not found");
                return;
           }
          GET_OBJ_TIMER(tobj) = 2;
          obj_to_room(tobj, mob->in_room);
     }
  }
}
