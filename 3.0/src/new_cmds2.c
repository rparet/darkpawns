/* ==========================================================================
   FILE   : new_cmds2.c
   HISTORY:  dlkarnes
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

/* $Id: new_cmds2.c 1258 2005-12-10 02:19:14Z mcooper $ */

#include <math.h>
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
#include "olc.h"

/* extern variables */
extern char *dirs[];
extern int mini_mud;
extern const int rev_dir[];

/* extern functions */
struct char_data *HUNTING(struct char_data *ch);   
ACMD(do_tell);
void raw_kill(struct char_data * ch, struct char_data * killer,  int attacktype); 
void set_hunting(struct char_data *ch, struct char_data *victim);
void send_to_zone(char *messg, struct char_data *ch);
void improve_skill(struct char_data *ch, int skill_num);
int real_zone(int number);
struct char_data *get_mount(struct char_data *rider);
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);
bool ok_loot(struct char_data * ch, struct obj_data * obj);


/* this command used to be called do_forage -rparet 19981125 */
ACMD(do_scrounge)
{
  int sector_type = world[ch->in_room].sector_type;
  struct obj_data *obj;
  int food = 0;
  bool find = TRUE;

  switch(sector_type) {
    case SECT_FOREST:
	    find = FALSE; /* kill*/
      food = 28;
      break;
    case SECT_FIELD:
    case SECT_HILLS:
	    find = FALSE; /* kill*/
      food = 29;
      break;
    case SECT_DESERT:
	    find = FALSE; /* kill*/
      food = 30;
      break;
    case SECT_MOUNTAIN:
      food = 31;
      break;
    case SECT_WATER_SWIM:
    case SECT_WATER_NOSWIM:
    case SECT_UNDERWATER:
	    find = FALSE; /* kill*/
      food = 27;
      break;
    default:
      stc("You need to be in the wilderness to scrounge!\r\n", ch);
      break;
  }

  if (food && skill_success(ch, SKILL_SCROUNGE, 0)) {
    if ((obj = read_object(real_object(food), REAL, NULL))) {
      obj_to_char(obj, ch);
	    if (find)
        act("You find $p.", TRUE, ch, obj, 0, TO_CHAR);
	    else
        act("You capture and kill $p.", TRUE, ch, obj, 0, TO_CHAR);
      improve_skill(ch, SKILL_SCROUNGE);
	  }
  } else if (food)
    stc("You can't seem to find anything edible.\r\n", ch);

  act("$n searches for something to eat.", TRUE, ch, 0, 0, TO_ROOM);
}


ACMD(do_first_aid)
{
  struct char_data *vict;

  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else {
   if ((GET_HIT(vict) >= 1)) {
     send_to_char("They don't really need first aid.\r\n", ch);
     return;
   }

   if (skill_success(ch, SKILL_FIRST_AID, 0)) {
     GET_HIT(vict) = 1;
     update_pos(vict);
     act("You apply some makeshift bandages to $N's wounds.", FALSE, ch, 0, vict, TO_CHAR);
     act("$n applies some bandaging to $N's wounds.", FALSE, ch, 0, vict, TO_NOTVICT);
     act("$n applies some bandaging to your wounds.", TRUE, ch, 0, vict, TO_VICT);
     improve_skill(ch, SKILL_FIRST_AID);
   } else {
     send_to_char("You fumble and ruin the bandages.\r\n",ch);
     act("$n fumbles with some bandaging and drops it all over the place!", TRUE, ch, 0, 0, TO_ROOM);
   }
  }
}

/*
 *  Recharger mob special procedure   Part of Dark Pawns (www.mystech.com 4000)
 *
 *  Based on original recharger code by 
 *        David A. Carver <DCARVER@cougar.colstate.cc.oh.us>
 *
 *  Rewritten for mobiles and "personality" by 
 *        Frontline (rparet@rubens.artisan.calpoly.edu) on 6/6/97.
 *  The author maintains exclusive rights to this code.
 *  Use only with permission.
 *
 */


SPECIAL(recharger)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  int maxcharge = 0, mincharge = 0, price = 0;

  if (!(CMD_IS("list") || CMD_IS("help") || CMD_IS("recharge")))
    return FALSE; 
  
  if (CMD_IS("list") || CMD_IS("help"))
  {
    act("$n sighs loudly.", FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'I recharge wands and staves, of course!'", 
	FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'My price is 100 coins per spell level per charge..'", 
	FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'So a wand casting heal at level 25 would cost 2500 "
	"coins per charge.'", FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'If it had 10 max charges at first, it would have 9 max "
	"after charging.",FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'That means that an item could have 1 charge maximum, with"
	" 2 charges remaining (after a recharge)'", FALSE, me, 0, 0, TO_ROOM);
    act("$n says 'To recharge an item type: recharge <staff or wand>.'", 
	FALSE, me, 0, 0, TO_ROOM);
    act("$n stares expectantly at $N.", 1, me, 0, ch, TO_NOTVICT);
    act("$n stares expectantly at you.", FALSE, me, 0, ch, TO_VICT);
    return TRUE;
  } else
    if (CMD_IS("recharge")) {
    argument = one_argument(argument, buf);

    if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
      act("$n tells you, 'You don't have that!'", FALSE, me, 0, ch, TO_VICT);
      act("$n whaps $N upside the head.", 1, me, 0, ch, TO_NOTVICT);
      act("$n whaps you upside the head.", FALSE, me, 0, ch, TO_VICT);
      return TRUE;
    }
    if (GET_OBJ_TYPE(obj) != ITEM_STAFF)
    {
      act("$n tells you, 'Ummm... does that look like a staff to you?'", 
		FALSE, me, 0, ch, TO_VICT);
      act("$n shakes his head in disgust.", FALSE, me, 0, 0, TO_ROOM);
      return TRUE;
    }
    price = MAX(100, GET_OBJ_VAL(obj, 0)*100);
    if (GET_GOLD(ch) < price) {
      act("$n tells you, 'You don't have enough gold!'", 
	FALSE, me, 0, ch, TO_VICT);
      return TRUE;
    }
    maxcharge = GET_OBJ_VAL(obj, 1);
    mincharge = GET_OBJ_VAL(obj, 2);

    if (mincharge < maxcharge)
    {
     GET_OBJ_VAL(obj, 2)++;
     GET_OBJ_VAL(obj, 1)--;
     GET_GOLD(ch) -= price;
     act("$n waves his hands around and chants strange things.", 
	FALSE, me, 0, 0, TO_ROOM);
     sprintf(buf, "The item now has %ld charges remaining.\r\n", 
		GET_OBJ_VAL(obj, 2));
     send_to_char(buf, ch);
     }
   else
     act("$n tells you, 'The item does not need recharging, stop wasting "
	"my time!'", FALSE, me, 0, ch, TO_VICT);
    return TRUE;
  }
  return FALSE;
}



/* Bare-bones detect skill      Part of Dark Pawns (www.mystech.com 4000)
 *
 * Allows players to detect "secret" doors
 * 
 * Frontline 6/7/97
 * 
 * Updated and modernized on 19990429 by rparet
 *
 */
ACMD(do_detect)
{
  int dir;
  
  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You're blind, you can't find anything!!\r\n",ch);
    return;
  }
  
  send_to_char("You carefully check the room...\r\n", ch);

  if (!skill_success(ch, SKILL_DETECT, 0)) {
    send_to_char("You can't seem to find anything.\r\n", ch);
    return;
  } 

  for (dir = 0; dir < NUM_OF_DIRS; dir++) {
    if (EXIT(ch, dir) && EXIT(ch, dir)->keyword) 
      if (strstr(EXIT(ch, dir)->keyword, "secret")) {
        if (dir == UP)
          sprintf(buf, "You notice something funny about the ceiling.\r\n");
        else if (dir == DOWN)
          sprintf(buf, "You notice something funny about the floor.\r\n");
        else
          sprintf(buf, "You notice something funny about the %s wall.\r\n", dirs[dir]);
        send_to_char(buf, ch);
        if (!ROOM_FLAGGED(ch->in_room, ROOM_SECRET_MARK))
          SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_SECRET_MARK);
     }
  }
  improve_skill(ch, SKILL_DETECT);
} 

/* This procedure used to be called create_and_start_hunting.
   I have modified it so that you may call create_mobile to create
   an instance of a mobile that is statistically correct for any level
   that you pass to this function.  This allows you to have spellcasters
   summon mobiles that have powers based upon the level of the caster.
   Important procedural note: if you call this function with the hunting
   integer set to TRUE, the mobile will be loaded in room 18201 and start
   hunting the player ch.  -rparet 19981107 
*/

struct char_data *create_mobile(struct char_data *ch, int mob_number, int level, int hunting) 
{
  struct char_data *mob = read_mobile(mob_number, VIRTUAL, NULL);
  int statmod;

  if (!mob)
    return NULL;

  GET_LEVEL(mob) = level;
  GET_EXP(mob) = 0;
  GET_DAMROLL(mob) = (level > 10 ? ((level + 1) / 1.50) : ((level + 1) / 2));
  mob_combat_dice(mob);
  GET_MOB_TIMER(mob) = 0;

  GET_HITROLL(mob) = level / 3;
  GET_DODGEROLL(mob) = level / 10;
  GET_NHPD(mob) = level;
  GET_SHPD(mob) = 8;
  GET_HPBONUS(mob) = (3 * pow(level, 1.5));

  GET_AC(mob) = pow(GET_LEVEL(mob), 1.15);
  GET_MAX_HIT(mob) = dice(GET_NHPD(mob), GET_SHPD(mob)) + GET_HPBONUS(mob);
  GET_HIT(mob) = GET_MAX_HIT(mob);

  /* Adjust abilities */
  statmod = pow(GET_LEVEL(mob), .70);
  GET_REAL_STR(mob) = 5 + statmod;
  GET_REAL_INT(mob) = 5 + statmod;
  GET_REAL_WIS(mob) = 5 + statmod;
  GET_REAL_DEX(mob) = 5 + statmod;
  GET_REAL_CON(mob) = 5 + statmod;
  GET_REAL_CHA(mob) = 5 + statmod;

  if (ch && hunting) {
    char_to_room(mob, real_room(18201)); /* start em at temple in Kir-Oshi */
    set_hunting(mob, ch);
  }

  if (!hunting) {
    char_to_room(mob, ch->in_room);
  }

  GET_LOADED_IN(mob) = mob->in_room;
  return (mob);
}

SPECIAL(black_horn)
{
  struct obj_data *obj = (struct obj_data *)me, *mag_item = NULL;
  if (CMD_IS("use"))
  {
    skip_spaces(&argument);
    mag_item = GET_EQ(ch, WEAR_HOLD);

    if (mag_item && mag_item == obj && isname(argument, obj->name))
        {
          send_to_zone("A deep, foreboding tone resounds through the "
			"air.\r\n", ch);
          act("You inhale and blow deeply on $P.",TRUE, ch, 0, obj, TO_CHAR);
          stc("A deep, foreboding tone resounds through the air.\r\n", ch);
          act("$n blows on $P.", TRUE, ch, 0, obj, TO_ROOM);
          act("A deep, foreboding tone resounds through the air.", 
		FALSE, ch, 0, 0, TO_ROOM);
   	  switch(number(0, 5)) 
	  {
    		case 0: create_mobile(ch, 14503, 25, TRUE); break;
    		case 1: create_mobile(ch, 14504, 25, TRUE); break;
    		case 2: create_mobile(ch, 14513, 25, TRUE); break;
    		case 3: create_mobile(ch, 14514, 25, TRUE); break;
    		case 4: create_mobile(ch, 14515, 25, TRUE); break;
    		default: create_mobile(ch,14516, 25, TRUE); break;
	  }
          return(TRUE);
        }
  }
  return(FALSE);     
}

/* FIX - act.offensive.c */
/* FIX - need description */
ACMD(do_serpent_kick)
{
  struct char_data *vict;
  int percent, prob;
  struct combat_cmd *qcmd;
  
  if (GET_COMBAT_WAIT(ch))
  {
    CREATE(qcmd, struct combat_cmd, 1); 
    qcmd->command = do_serpent_kick;
    qcmd->ch      = ch;
    qcmd->argument = argument;
    qcmd->cmd = cmd;
    qcmd->subcmd = subcmd;
    return;
  }

  if (!GET_SKILL(ch, SKILL_SERPENT_KICK))
    {
      stc("You'd better leave all the martial arts to others.\r\n", ch);
      return;
    }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg)))
    {
      if (FIGHTING(ch))
        vict = FIGHTING(ch);
      else
        {
          send_to_char("Kick who?\r\n", ch);
          return;
        }
    }
  if (vict == ch)
    {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
    }
   if (IS_MOUNTED(ch))
   {
     stc("Dismount first!\r\n",ch);
     return;
   }

  percent = ((7 - (GET_AC(vict) / 10)) << 1) + number(1, 101);
  /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_SERPENT_KICK);

  if (GET_POS(vict) <= POS_SLEEPING)
    prob=110;

  if (percent > prob)  /* miss */
    damage(ch, vict, 0, SKILL_SERPENT_KICK);
  else
    {
      damage(ch, vict, GET_LEVEL(ch)*1.5, SKILL_SERPENT_KICK);
      if(!number(0, 31) && (GET_SKILL(ch, SKILL_SERPENT_KICK) > 50))  /* ~3% chance */
        create_mobile(ch, 18221, GET_LEVEL(ch)+3, TRUE);
      improve_skill(ch, SKILL_SERPENT_KICK);
      COMBAT_WAIT(vict, 4);
    }
  COMBAT_WAIT(ch, 3);
}



#define h_item    0
#define h_hunter  1
#define h_hunter2 2
#define h_hunter3 3
#define h_percent 4
#define h_parts   5
const int hunteds[][h_parts] = {
  {4209,   4207,     0,     0,  1},  /* opal/bhyroga warrior */
  {4210,   4207,     0,     0,  1},  /* jade/bhyroga warrior */
  {11814, 11801,     0,     0,  1},  /* orb of power/rakshasa */
  {14223, 14203, 14205, 14207,  3}   /* spiked armor/Skarash infidel nonesuch */
};
#define NUM_HUNTEDS 4

/*
 * Cycle through the char list
 * If ch has h_item and %, find an available h_hunter and set_hunting
 */

void
hunt_items(void)
{
  struct descriptor_data *i;
  struct char_data *ch = NULL, *next_ch = NULL;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && i->character)
    {
     int j = 0, k = 0;
     for (j = 0; j < NUM_WEARS; j++) 
     {
       struct obj_data *obj = NULL, *hunting = NULL;
       bool found = FALSE;

       if ( (obj = GET_EQ(i->character, j)) )
	 for (k = 0; k < NUM_HUNTEDS; k++)
	   if (GET_OBJ_VNUM(obj)==hunteds[k][h_item])
	     if(number(0,100)<hunteds[k][h_percent])
	     {
  		for (ch = character_list; !found && ch; ch = next_ch) 
		{
    		   next_ch = ch->next;
    		   if ( (GET_MOB_VNUM(ch)!=hunteds[k][h_hunter] && 
			 GET_MOB_VNUM(ch)!=hunteds[k][h_hunter2]&&
			 GET_MOB_VNUM(ch)!=hunteds[k][h_hunter3]) ||
			FIGHTING(ch) || !AWAKE(ch) ||
		        IS_AFFECTED(ch, AFF_CHARM) || HUNTING(ch))
      		      continue;
		   else
		   {
			found = TRUE;
			hunting = obj;
                        i->character->char_specials.hunted_obj = obj;
			break;
		   }
		}
		if (found && ch)
		{
		  char *msg = NULL;
      		  msg = tprintf("%s I must have %s!", GET_NAME(i->character),
 			(hunting)->short_description);
		  if (msg)
		  {
      		    do_tell(ch, msg, find_command("tell"), 0);
		    FREE(msg);
		  }
	          set_hunting(ch, i->character);
		}
	     }
     }
   }
}


/*
 * From: Corey Hoitsma (choitsma@netcom.com)
 */
ACMD(do_build)
{
  /* Only works if you have Oasis OLC */
  extern void olc_add_to_save_list(int zone, byte type);

  char buf2[10], buf3[10], buf[80];
  int rroom = 0, dir = 0;
  long iroom = 0;

  two_arguments(argument, buf2, buf3); 
  /* buf2 is the direction, buf3 is the room */

  iroom = atol(buf3);
  rroom = real_room(iroom);

  if (!*buf2 || !*buf3)
  {
    send_to_char("Format: build <dir> <room number>\r\n", ch); 
    return; 
  }
  if (rroom <= 0) 
  {
    sprintf(buf, "There is no room with the number %ld.\r\n", iroom);
    send_to_char(buf, ch);
    return; 
  }

  if (GET_LEVEL(ch) <= LVL_IMMORT && (zone_table[world[ch->in_room].zone].number != GET_OLC_ZONE(ch))) {
    stc("You don't have permission to edit this zone.\r\n", ch);
    return;
  } else if (GET_LEVEL(ch) <= LVL_IMMORT && (real_zone(iroom) != real_zone(world[ch->in_room].number))) {
    stc("You don't have permission to edit that zone.\r\n", ch);
    return;
  }

  /* Main stuff */
  switch (*buf2) 
  {
   case 'n': case 'N': dir = NORTH; break;
   case 'e': case 'E': dir = EAST; break;
   case 's': case 'S': dir = SOUTH; break;
   case 'w': case 'W': dir = WEST; break;
   case 'u': case 'U': dir = UP; break;
   case 'd': case 'D': dir = DOWN; break;
   default: 
         stc("Valid dirs are n,s,e,w,u and d.\r\n", ch); 
  }

  CREATE(world[rroom].dir_option[rev_dir[dir]], struct room_direction_data,1); 
  world[rroom].dir_option[rev_dir[dir]]->general_description = NULL;
  world[rroom].dir_option[rev_dir[dir]]->keyword = NULL;
  world[rroom].dir_option[rev_dir[dir]]->to_room = ch->in_room; 

  CREATE(world[ch->in_room].dir_option[dir], struct room_direction_data,1); 
  world[ch->in_room].dir_option[dir]->general_description = NULL;
  world[ch->in_room].dir_option[dir]->keyword = NULL;
  world[ch->in_room].dir_option[dir]->to_room = rroom; 

  /* Only works if you have Oasis OLC */
  olc_add_to_save_list(zone_table[world[rroom].zone].number, OLC_SAVE_ROOM);

  sprintf(buf, "You make an exit %s to room %ld.\r\n", buf2, iroom);
  send_to_char(buf, ch);
}


void
flow_room(struct char_data *ch)
{
  char msg[256], direct[30];
  int dir = -1, was_in = ch->in_room;
  struct char_data *mount = NULL;

  if (IS_FLYING(ch))  /* no flowing whilst flying */
    return;

  if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_NORTH))
  {
    dir = NORTH;
    sprintf(direct, "the south"); 
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_SOUTH))
  {
    sprintf(direct, "the north"); 
    dir = SOUTH;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_EAST))
  {
    sprintf(direct, "the west"); 
    dir = EAST;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_WEST))
  {
    sprintf(direct, "the east"); 
    dir = WEST;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_UP))
  {
    sprintf(direct, "below"); 
    dir = UP;
  }
  else if(ROOM_FLAGGED(ch->in_room, ROOM_FLOW_DOWN))
  {
    sprintf(direct, "above"); 
    dir = DOWN;
  }

  if(dir < 0 || !world[was_in].dir_option[dir] || 
     (GET_LEVEL(ch) >= LVL_IMMORT && PRF_FLAGGED(ch, PRF_NOHASSLE)) || 
     IS_NPC(ch) || world[was_in].dir_option[dir]->to_room==NOWHERE)
    return;
  


  /*
   *  Prolly won't have mounts on water.. but other rooms might
   *  "flow" later
   */
  if (IS_MOUNTED(ch) && !IS_NPC(ch))
    mount = get_mount(ch);
  if (mount)
    {
      sprintf(msg, "$N drifts %s on $n.", dirs[dir]);
      act(msg, TRUE, mount, 0, ch, TO_NOTVICT);
      char_from_room(mount);
      char_to_room(mount, world[was_in].dir_option[dir]->to_room);
      sprintf(msg, "$N drifts in from %s on $n.", direct);
      act(msg, TRUE, mount, 0, ch, TO_NOTVICT);
    }
  else
    {
      sprintf(msg, "$n drifts %s.", dirs[dir]);
      act(msg, TRUE, ch, 0, 0, TO_ROOM);
    }
  sprintf(msg, "You drift %s.\r\n", dirs[dir]);
  stc(msg, ch);
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);
  look_at_room(ch, 0);
  sprintf(msg, "$n drifts in from %s.", direct);
  act(msg, TRUE, ch, 0, 0, TO_NOTVICT);
}

ACMD(do_outlaw)
{
  if (IS_NPC(ch)) {
    stc("Why would you want to do that?\r\n", ch);
    return;
  }

  if (subcmd != SCMD_OUTLAW) {
    stc("You will need to type \"outlaw\" in order to become one.\r\n", ch);
    return;
  }

  if (!IS_OUTLAW(ch)) {
    stc("You enter the deadly realm of the Outlaws.\r\n", ch);
    TOGGLE_BIT(PLR_FLAGS(ch), PLR_OUTLAW);
    OUTLAW_TIME(ch) = 60;
    sprintf(buf, "(OUT) %s has elected to become an Outlaw.", GET_NAME(ch));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
  } else {
    if (OUTLAW_TIME(ch) <= 0) {
      stc("You have elected to leave the realm of the Outlaws.\r\n", ch);
      TOGGLE_BIT(PLR_FLAGS(ch), PLR_OUTLAW);
      OUTLAW_TIME(ch) = 0;
      sprintf(buf, "(OUT) %s has elected to return from Outlaw status.", GET_NAME(ch));
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
    } else
      stc("You have not served out your time as an Outlaw yet.\r\n", ch);
  }
}

ACMD(do_skin)
{
  struct obj_data *tmp_obj, *obj;
  struct char_data *vict;
  long vnum;
  char type[20];
     
  argument = one_argument(argument, arg);
  vict = get_char_room_vis(ch, arg);

  if (!*arg) {
    send_to_char("You want to skin what?!?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("This game doesn't support self-mutilation!\r\n", ch);
    return;  
  } else if (vict != NULL) {
    send_to_char("You kill it first and THEN you can skin it!\r\n",ch);
    return;
  } else if(!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
    sprintf(buf, "You can't seem to find a %s to skin!\r\n", arg);
    send_to_char(buf, ch);
    return;
  } else if (!(strstr(obj->name,"corpse"))) {
    send_to_char("Doesn't look like there's much you could skin from that!\r\n", ch);
    return;
  } else if (!(strstr(obj->name,"skin_"))) {
    send_to_char("It's just too difficult to even attempt to skin that!\r\n", ch);
    return;
  } else if (!ok_loot(ch, obj)) {
    send_to_char("Perhaps if you had killed it, then you could skin it!\r\n", ch);
    return;
  }

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {  
    send_to_char("Your arms are already full!\r\n", ch);
    return;    
  }

  if (isname("skin_fur",obj->name)) {
    vnum = 1217;
    strcpy(type, "fur");
  } else if (isname("skin_pelt",obj->name)) {
    vnum = 1218;
    strcpy(type, "pelt");
  } else if (isname("skin_hide",obj->name)) {
    vnum = 1219;
    strcpy(type, "hide");
  } else if (isname("skin_feather",obj->name)) {
    vnum = 1220;
    strcpy(type, "feathered head-dress");
  } else {
    vnum = 1217;
    strcpy(type, "fur");
  }

  /* Load the object, rename it and reset the alias list */
  tmp_obj = read_object(vnum, VIRTUAL, NULL);
  one_argument(obj->name, arg);
  sprintf(buf, "%s %s", arg, type);
  sprintf(buf2, "a %s", buf);
  tmp_obj->short_description = str_dup(buf2);
  tmp_obj->name = str_dup(buf);
  if (tmp_obj->ex_description->keyword)
    tmp_obj->ex_description->keyword = str_dup(buf);

  if (GET_EQ(ch, WEAR_WIELD)) {
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == 3) /*type3 = slash*/
      obj_to_char(tmp_obj, ch);
    else
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == 11) /*type11 = pierce*/
        obj_to_char(tmp_obj, ch);
      else {
        send_to_char("You can't skin it with that!\n\r",ch);
        extract_obj(tmp_obj);
        return;
      }
  } else {
    send_to_char("You not wielding anything to skin it with!\n\r",ch);
    extract_obj(tmp_obj);
    return;
  }

  act("$n skins $p.",TRUE, ch, obj, 0, TO_ROOM);
  act("You skin $p.",TRUE, ch, obj, 0, TO_CHAR);
  extract_obj(obj);
}

struct obj_data *combine(struct char_data *ch, struct obj_data *obj)
{
  struct obj_data *i;
  long obj_vnum;

  strcpy(buf, "\0");
  for (i = obj->contains; i; i = i->next_content)
    sprintf(buf+strlen(buf), "%d ", GET_OBJ_VNUM(i));

  obj_vnum = run_script(ch, ch, obj, &world[ch->in_room], buf, "code", LT_OBJ);

  if (obj_vnum)
    return (read_object(obj_vnum, VIRTUAL, NULL));
  else
    return (NULL);
}

ACMD(do_combine)
{
  struct obj_data *obj = NULL, *tmp_obj, *ingredients, *result = NULL;

  one_argument(argument, arg);
  switch(subcmd) {
    case SCMD_MIX:
      if (!*arg) {
        send_to_char("What do you want to mix?\r\n", ch);
        return;
      } /* Do they have the container specified? */
      else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        send_to_char("You don't seem to have that.\r\n", ch);
        return;
      } /* Did they specify the mortar? */
      else if (GET_OBJ_VNUM(obj) != 1257) {
        send_to_char("That's not really an appropriate mixing container.\r\n", ch);
        return;
      } /* Do they have an empty vial to put the potion in? */
      else if (!(result = get_obj_in_list_num(real_object(1258), ch->carrying))) {
        send_to_char("You've got nothing to put the completed product in.\r\n", ch);
        return;
      } /* Does the mortar contain anything? */
      else if (!obj->contains) {
        act("$p is empty! There is nothing to mix!", FALSE, ch, obj, NULL, TO_CHAR);
        return;
      }

      /* Get rid of the empty vial */
      extract_obj(result);

      result = combine(ch, obj);
      for (ingredients = obj->contains; ingredients; ingredients = tmp_obj) {
        tmp_obj = ingredients->next_content;
        extract_obj(ingredients);
      }
  }

  if (skill_success(ch, SKILL_ALCHEMY, 0) && result) {
    act("You combine some ingredients in $P and produce $p.", TRUE, ch, result, obj, TO_CHAR);
    act("$n combines some ingredients in $P and produces $p.", TRUE, ch, result, obj, TO_ROOM);
    obj_to_char(result, ch);
    improve_skill(ch, SKILL_ALCHEMY);
  } else {
    act("You combine some ingredients in $p and produce nothing!", TRUE, ch, obj, NULL, TO_CHAR);
    act("$n combines some ingredients in $p and produces nothing!", TRUE, ch, obj, NULL, TO_ROOM);
    if (result)
      extract_obj(result);
  }
}


#define SCROLL_PROTO 1244
ACMD(do_scribe)
{
  char *s;
  struct obj_data *parchment, *gem, *scroll;
  int spellnum, mana, i = 0;
  extern struct skill_tree_data skill_tree[];
  int mag_manacost(struct char_data * ch, int spellnum);
  extern struct spell_info_type spell_info[];

  two_arguments(argument, buf1, buf2);
  if ((!*buf1) || (!*buf2)) {
    send_to_char("Scribe what spell onto what object??\r\n", ch);
    return;
  }

  /* Do we have a NOTE object to use? */
  if (!(parchment = get_obj_in_list_vis(ch, buf1, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(buf1), buf1);
    send_to_char(buf, ch);
    return;
  }

  if (GET_OBJ_TYPE(parchment) != ITEM_NOTE) {
    send_to_char("How do you expect to scribe anything onto that!\r\n", ch);
    return;
  }

  /* Borrowed from do_cast - let's get the spell type */
  s = strtok(argument, "'");
  if (!s) {
    send_to_char("Scribe which spell?\r\n", ch);
    return;
  }

  s = strtok(NULL, "'");
  if (!s) {
    send_to_char("Spell names must be enclosed in the magic symbols: '\r\n", ch);
    return;
  }

  spellnum = find_skill_num(s);
  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Scribe what?!?\r\n", ch);
    return;
  }

  if (GET_SPELL(ch, spellnum) == '0') {
    send_to_char("You are unfamiliar with that spell.\r\n", ch);
    return;
  }

  if (IS_SET(spell_info[spellnum].targets, TAR_NOT_SELF)) {
    send_to_char("You can't scribe a spell that you can't cast on yourself!\r\n", ch);
    return;
  }

  /* Are we holding an unused soul gem */
  gem = GET_EQ(ch, WEAR_HOLD);
  if (!gem) {
    send_to_char("You need to hold your soul gem for this skill to work.\r\n", ch);
    return;
  } else if (GET_OBJ_TYPE(gem) != ITEM_SOUL_GEM) {
    act("$p isn't a soul gem!", TRUE, ch, gem, 0, TO_CHAR);
    return;
  } else if (GET_OBJ_VAL(gem, 1) != -1) {
    send_to_char("This gem does not contain a soul!\r\n", ch);
    return;
  }

  while (*skill_tree[++i].name != '\n')
    if (skill_tree[i].skill_number == spellnum)
      break;

  if (!strcmp(skill_tree[i].name, "\n")) {
    mudlog("SYSERR: Attempting to scribe an unknown spell.", BRF, LVL_IMMORT, FALSE);
    return;
  }

  /* Is the trapped soul powerful enough to do the job */
  if (GET_OBJ_VAL(gem, 0) < (skill_tree[i].level * 25)) {
    send_to_char("Your soul gem is not powerful enough to scribe that spell.\r\n", ch);
    return;
  }

  mana = (mag_manacost(ch, spellnum) * 2);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't got the mana to scribe that spell!\r\n", ch);
    return;
  }

  GET_MANA(ch) -= mana;

  if (skill_success(ch, SKILL_SCRIBE, 20)) {  /* success */
    sprintf(buf, "You scribe the '%s' spell onto $p.", skill_tree[i].name);
    act(buf, FALSE, ch, parchment, 0, TO_CHAR);
    act("$n scribes something onto $p.", TRUE, ch, parchment, 0, TO_ROOM);
    improve_skill(ch, SKILL_SCRIBE);

    if ((scroll = read_object(SCROLL_PROTO, VIRTUAL, NULL))) {
      GET_OBJ_COST(scroll) = skill_tree[i].level  * 1000; /* ranges from 1000 to 4000 coins */
      GET_OBJ_VAL(scroll, 0) = GET_LEVEL(ch);   /* spell level */
      GET_OBJ_VAL(scroll, 1) = spellnum;        /* spell number */
      
      sprintf(buf, "a scroll of %s", skill_tree[i].name);
      scroll->short_description = str_dup(buf);
      obj_to_char(scroll, ch);
    } else {
      send_to_char("There was an error in your scroll creation, please inform the staff.\r\n", ch);
      mudlog("SYSERR: Missing scroll prototype for do_scribe.", BRF, LVL_IMPL, TRUE);
    }
  } else {
    act("You were unsuccessful in scribing onto $p.", TRUE, ch, parchment, 0, TO_CHAR);
    act("$n unsuccessfully attempts to scribe onto $p.", TRUE, ch, parchment, 0, TO_ROOM);
  }
  
  extract_obj(gem);
  extract_obj(parchment);
}
