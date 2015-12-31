/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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

/* $Id: fight.c 1521 2008-06-14 21:52:55Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "scripts.h"

/* Structures */
struct char_data *combat_list = NULL;	/* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int auto_save;		/* see config.c */
extern int max_exp_gain;	/* see config.c */
extern struct index_data *mob_index;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern int r_mortal_start_room;
extern int is_shopkeeper(struct char_data * chChar);

/* External procedures */
char *fread_action(FILE * fl, int nr);
char *fread_string(FILE * fl, char *error);
void stop_follower(struct char_data * ch);
ACMD(do_flee);
ACMD(do_retreat);
ACMD(do_gen_comm);
void hit(struct char_data * ch, struct char_data * victim, int type);
int forget(struct char_data * ch, struct char_data * victim);
void remember(struct char_data * ch, struct char_data * victim);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
int flesh_altered_type(struct char_data * ch);
void tattoo_af(struct char_data * ch, bool add);
SPECIAL(take_to_jail);
SPECIAL(wall_guard_ns);
void unmount(struct char_data *ch, struct char_data *mount);
struct char_data *get_mount(struct char_data *ch);
struct char_data *HUNTING(struct char_data *ch);
void set_hunting(struct char_data *ch, struct char_data *victim);
bool can_speak(struct char_data *ch);
void make_dust(struct char_data *ch, int attacktype);

/*
 *                  Weapon attack texts
 * if you change this, change flesh_altered_type() too
 */
struct attack_hit_type attack_hit_text[] =
{
   {"hit", "hits"},		/* 0 */
   {"sting", "stings"},
   {"whip", "whips"},
   {"slash", "slashes"},
   {"bite", "bites"},
   {"bludgeon", "bludgeons"},	/* 5 */
   {"crush", "crushes"},
   {"pound", "pounds"},
   {"claw", "claws"},
   {"maul", "mauls"},
   {"thrash", "thrashes"},	/* 10 */
   {"pierce", "pierces"},
   {"blast", "blasts"},
   {"punch", "punches"},
   {"stab", "stabs"}
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

/* The Fight related routines */

void
appear(struct char_data * ch)
{
   if (affected_by_spell(ch, SPELL_INVISIBLE))
      affect_from_char(ch, SPELL_INVISIBLE);

   REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INVISIBLE);
   REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

   if (GET_LEVEL(ch) < LVL_IMMORT)
      act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
   else
      act("You feel a strange presence as $n appears, seemingly from nowhere.",
	  FALSE, ch, 0, 0, TO_ROOM);
}



void
load_messages(void)
{
   FILE *fl;
   int i, type;
   struct message_type *messages;
   char chk[128];

   if (!(fl = fopen(MESS_FILE, "r"))) {
      sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
      perror(buf2);
      exit(1);
   }
   for (i = 0; i < MAX_MESSAGES; i++) {
      fight_messages[i].a_type = 0;
      fight_messages[i].number_of_attacks = 0;
      fight_messages[i].msg = 0;
   }


   fgets(chk, 128, fl);
   while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);

   while (*chk == 'M') {
      fgets(chk, 128, fl);
      sscanf(chk, " %d\n", &type);
      for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	      (fight_messages[i].a_type); i++);
      if (i >= MAX_MESSAGES) {
	 fprintf(stderr, "Too many combat messages.  Increase MAX_MESSAGES and "
		 "recompile.");
	 exit(1);
      }
      CREATE(messages, struct message_type, 1);
      fight_messages[i].number_of_attacks++;
      fight_messages[i].a_type = type;
      messages->next = fight_messages[i].msg;
      fight_messages[i].msg = messages;

      messages->die_msg.attacker_msg = fread_action(fl, i);
      messages->die_msg.victim_msg = fread_action(fl, i);
      messages->die_msg.room_msg = fread_action(fl, i);
      messages->miss_msg.attacker_msg = fread_action(fl, i);
      messages->miss_msg.victim_msg = fread_action(fl, i);
      messages->miss_msg.room_msg = fread_action(fl, i);
      messages->hit_msg.attacker_msg = fread_action(fl, i);
      messages->hit_msg.victim_msg = fread_action(fl, i);
      messages->hit_msg.room_msg = fread_action(fl, i);
      messages->god_msg.attacker_msg = fread_action(fl, i);
      messages->god_msg.victim_msg = fread_action(fl, i);
      messages->god_msg.room_msg = fread_action(fl, i);
      fgets(chk, 128, fl);
      while (!feof(fl) && (*chk == '\n' || *chk == '*'))
	 fgets(chk, 128, fl);
   }

   fclose(fl);
}


void update_pos(struct char_data * victim)
{

   if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
      return;
   else if (GET_HIT(victim) > 0)
      GET_POS(victim) = POS_STANDING;
   else if (GET_HIT(victim) <= -11)
      GET_POS(victim) = POS_DEAD;
   else if (GET_HIT(victim) <= -6)
      GET_POS(victim) = POS_MORTALLYW;
   else if (GET_HIT(victim) <= -3)
      GET_POS(victim) = POS_INCAP;
   else
      GET_POS(victim) = POS_STUNNED;
}



/* start one char fighting another (yes, it is horrible, I know... )  */
void
set_fighting(struct char_data * ch, struct char_data * vict)
{
   if (ch == vict)
      return;

   assert(!FIGHTING(ch));

   ch->next_fighting = combat_list;
   combat_list = ch;

   if (IS_AFFECTED(ch, AFF_SLEEP)) {
      affect_from_char(ch, SPELL_SLEEP);
      affect_from_char(ch, SKILL_SLEEPER);
   }

   FIGHTING(ch) = vict;
   GET_POS(ch) = POS_FIGHTING;

}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data * ch)
{
  struct char_data *temp, *k;

 if (FIGHTING(ch))
  if (GET_POS(FIGHTING(ch)) == POS_DEAD || FIGHTING(ch)->in_room != ch->in_room)  /* dead or fled */
   for (k = combat_list; k; k = temp) {
    temp = k->next_fighting;
    if (k && FIGHTING(k) == ch && GET_POS(k) > POS_DEAD)
    {
      FIGHTING(ch) = k;
      return;
    }
   }

   if (ch == next_combat_list)
      next_combat_list = ch->next_fighting;

   REMOVE_FROM_LIST(ch, combat_list, next_fighting);
   ch->next_fighting = NULL;
   FIGHTING(ch) = NULL;
   GET_POS(ch) = POS_STANDING;
   update_pos(ch);
}



void
make_corpse(struct char_data * ch, int attacktype)
{
   struct obj_data *corpse, *o;
   struct obj_data *money;
   int i, x, y;
   extern int max_npc_corpse_time, max_pc_corpse_time;

   struct obj_data *create_money(int amount);

   if (attacktype == SPELL_DISINTEGRATE)
     {
       make_dust(ch, attacktype);
       return;
     }

   corpse = create_obj();

   corpse->item_number = NOTHING;
   corpse->in_room = NOWHERE;
   strcpy(buf,ch->player.name);
   strcat(buf," corpse");
   corpse->name = str_dup(buf);

   switch (attacktype)
   {
     case TYPE_UNDEFINED:
      sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
      break;
     case SPELL_BURNING_HANDS:
     case SPELL_FIREBALL:
     case SPELL_HELLFIRE:
     case SPELL_FLAMESTRIKE:
      sprintf(buf2, "The charred corpse of %s is lying here, still"
                    " smoking.", GET_NAME(ch));
      break;
     case SPELL_CHILL_TOUCH:
      sprintf(buf2, "The frozen corpse of %s is thawing here.",
              GET_NAME(ch));
      break;
     case SPELL_COLOR_SPRAY:
     case SPELL_METEOR_SWARM:
     case SPELL_DISRUPT:
     case TYPE_BLAST:
      sprintf(buf2, "A blasted corpse lies here in pieces.");
      break;
     case SPELL_ENERGY_DRAIN:
     case SPELL_SOUL_LEECH:
      sprintf(buf2, "A dried up husk of a corpse is lying here.");
      break;
     case SPELL_LIGHTNING_BOLT:
     case SPELL_CALL_LIGHTNING:
     case SPELL_SHOCKING_GRASP:
      sprintf(buf2, "The shocked looking corpse of %s is lying here.",
              GET_NAME(ch));
      break;
     case SPELL_PSIBLAST:
      sprintf(buf2, "The corpse of %s is lying here, brains exploded"
              " everywhere.", GET_NAME(ch));
      break;
     case SKILL_BASH:
     case SKILL_KICK:
     case SKILL_PUNCH:
     case SKILL_DRAGON_KICK:
     case SKILL_TIGER_PUNCH:
     case SKILL_HEADBUTT:
     case SKILL_SMACKHEADS:
     case SKILL_SLUG:
     case SKILL_SERPENT_KICK:
     case TYPE_BLUDGEON:
     case TYPE_POUND:
     case TYPE_PUNCH:
     case TYPE_WHIP:
      sprintf(buf2, "The bruised, battered corpse of %s is lying here.",
              GET_NAME(ch));
      break;
     case SKILL_BITE:
     case TYPE_BITE:
     case TYPE_CLAW:
     case TYPE_SLASH:
     case SKILL_BACKSTAB:
     case SKILL_CIRCLE:
      sprintf(buf2, "The hacked up, bloody corpse of %s is lying here.",
              GET_NAME(ch));
      break;
     case SKILL_DISEMBOWEL:
      sprintf(buf2, "The corpse of %s is lying here, guts spilled"
              " everywhere.", GET_NAME(ch));
      break;
     case SKILL_NECKBREAK:
      sprintf(buf2, "The corpse of %s is lying here, %s neck snapped"
              " in two.", GET_NAME(ch), HSHR(ch));
      break;
     case TYPE_CRUSH:
     case TYPE_MAUL:
     case TYPE_THRASH:
      sprintf(buf2, "The crushed, barely recognizable corpse of %s"
              " is lying here.", GET_NAME(ch));
      break;
     case TYPE_PIERCE:
     case TYPE_STAB:
      sprintf(buf2, "The well-ventilated corpse of %s is lying here.",
              GET_NAME(ch));
      break;
     case SPELL_DROWNING:
      sprintf(buf2, "The bloated, waterlogged corpse of %s is lying"
              " here.", GET_NAME(ch));
      break;
     case SPELL_PETRIFY:
      sprintf(buf2, "The corpse of %s is here, frozen in stone.",
              GET_NAME(ch));
      break;
     default:
      sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
      break;
   }

   corpse->description = str_dup(buf2);

   sprintf(buf2, "the corpse of %s", GET_NAME(ch));
   corpse->short_description = str_dup(buf2);

   GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
   for (x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++) {
      if (x < EF_ARRAY_MAX)
         GET_OBJ_EXTRA_AR(corpse, x) = 0;
      if (y < TW_ARRAY_MAX)
         corpse->obj_flags.wear_flags[y] = 0;
   }
   /* I don't think I want people taking corpses anymore -rparet 19990315 */
   /* SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE); */
   SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
   GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
   GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
   GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
   GET_OBJ_LOAD(corpse) = 100000;
   if (IS_NPC(ch))
      GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
   else
      GET_OBJ_TIMER(corpse) = max_pc_corpse_time;

   /* transfer character's inventory to the corpse */
   corpse->contains = ch->carrying;
   for (o = corpse->contains; o != NULL; o = o->next_content)
      o->in_obj = corpse;
   object_list_new_owner(corpse, NULL);

   /* transfer character's equipment to the corpse */
   for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i))
	 obj_to_obj(unequip_char(ch, i), corpse);

   /* transfer gold */
   if (GET_GOLD(ch) > 0) {
      /* following 'if' clause added to fix gold duplication loophole */
      if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
	 money = create_money(GET_GOLD(ch));
	 obj_to_obj(money, corpse);
      }
      GET_GOLD(ch) = 0;
   }
   ch->carrying = NULL;
   IS_CARRYING_N(ch) = 0;
   IS_CARRYING_W(ch) = 0;

   if (ch->in_room>0)
     obj_to_room(corpse, ch->in_room);
   else if (GET_WAS_IN(ch)>0)
     obj_to_room(corpse, GET_WAS_IN(ch));
   else
     obj_to_room(corpse,r_mortal_start_room);
}

#define dust 18
#define vampire_dust 1230
void
make_dust(struct char_data * ch, int attacktype)
{
   struct obj_data *o, *next_o, *money;
   int i;
   extern int max_npc_corpse_time;

   struct obj_data *create_money(int amount);

   /* transfer character's inventory to the room */
   for (o = ch->carrying; o != NULL; o = next_o)
   {
      next_o = o->next_content;
      obj_from_char(o);
      obj_to_room(o, ch->in_room);
   }

   /* transfer character's equipment to the room */
   for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i))
	 obj_to_room(unequip_char(ch, i), ch->in_room);

   /* transfer gold */
   if (GET_GOLD(ch) > 0)
   {
      /* following 'if' clause added to fix gold duplication loophole */
      if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc))
      {
	 money = create_money(GET_GOLD(ch));
	 obj_to_room(money, ch->in_room);
      }
      GET_GOLD(ch) = 0;
   }
   ch->carrying = NULL;
   IS_CARRYING_N(ch) = 0;
   IS_CARRYING_W(ch) = 0;

   CREATE(o, struct obj_data, 1);
   if (GET_RACE(ch)==RACE_UNDEAD || attacktype==SPELL_DISINTEGRATE)
   {
     o = read_object(dust, VIRTUAL); /*dust_proto*/
     o->obj_flags.timer = max_npc_corpse_time;
   }
   else
     o = read_object(vampire_dust, VIRTUAL); /*vampire_dust_proto*/
   o->obj_flags.value[0]=0;
   o->obj_flags.value[3]=1;
   obj_to_room(o, ch->in_room);
}


/* When ch kills victim */
void
change_alignment(struct char_data * ch, struct char_data * victim)
{

  if (IS_NPC(ch))  /* mobiles shouldn't change align */
    return;

  /* neutral mobs (and PC's) don't affect alignment.
				-rparet 06181998 */
  if (IS_NEUTRAL(victim))
    return;

  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 4;

  if (GET_ALIGNMENT(ch)>1000)
        GET_ALIGNMENT(ch) = 1000;
  if (GET_ALIGNMENT(ch)<-1000)
        GET_ALIGNMENT(ch) = -1000;
}


void
death_cry(struct char_data * ch)
{
   int door, was_in;

   if (ch->in_room == NOWHERE)
   {
      sprintf(buf, "death_cry() in fight.c called with ch->in_room = NOWHERE");
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      char_to_room(ch, 0);
      return;
   }
   act("Your blood freezes as you hear $n's death cry.",
       FALSE, ch, 0, 0, TO_ROOM);
   was_in = ch->in_room;

   for (door = 0; door < NUM_OF_DIRS; door++) {
      if (CAN_GO(ch, door)) {
	 ch->in_room = world[was_in].dir_option[door]->to_room;
	 act("Your blood freezes as you hear someone's death cry.",
	     FALSE, ch, 0, 0, TO_ROOM);
	 ch->in_room = was_in;
      }
   }
}



void
raw_kill(struct char_data * ch, int attacktype)
{
   struct char_data *mob = NULL;

   if (ch->in_room == NOWHERE)
     return;

   if (FIGHTING(ch))
      stop_fighting(ch);

   while (ch->affected)
      affect_remove(ch, ch->affected);

   if (GET_TATTOO(ch))
   {
     tattoo_af(ch, FALSE);
     GET_TATTOO(ch) = TATTOO_NONE;
     TAT_TIMER(ch) = 0;
   }

   if (IS_AFFECTED(ch, AFF_WEREWOLF))
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_WEREWOLF);
   if (IS_AFFECTED(ch, AFF_VAMPIRE))
   {
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_VAMPIRE);
      if (GET_MANA(ch) > GET_MAX_MANA(ch))
        GET_MANA(ch) = GET_MAX_MANA(ch);
   }
   if (IS_MOB(ch))
      unmount(ch->master, ch);
   else
      unmount(ch, get_mount(ch));
   for (mob = character_list; mob; mob = mob->next)
   {
      if (MOB_FLAGGED(mob, MOB_MEMORY))
        forget(mob, ch);
      if (HUNTING(mob) == ch)
        set_hunting(mob, NULL);
   }
   death_cry(ch);

   if (GET_RACE(ch) == RACE_UNDEAD || GET_RACE(ch) == RACE_VAMPIRE)
      make_dust(ch, attacktype);
   else
      make_corpse(ch, attacktype);

   extract_char(ch);
}


/* used by damage() and spec procs that need to know the killer */
void
die_with_killer(struct char_data *ch, struct char_data *killer, int
                attacktype)
{
   gain_exp(ch, -(GET_EXP(ch)/37));
   if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
   {
      FIGHTING(ch) = killer;
      (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
   }

  if (IS_NPC(ch) && GET_MOB_SCRIPT(ch) && ch != killer && MOB_SCRIPT_FLAGGED(ch, MS_DEATH))
    run_script(killer, ch, NULL, &world[ch->in_room], NULL, "death", LT_MOB);

   if (GET_HIT(ch)<0)
   {
      if (!IS_NPC(ch))
      {
        if (GET_LEVEL(ch)>5 && !number(0,3))
        {
 	 (ch)->real_abils.con--;
	 if (GET_LEVEL(ch)>20 && !number(0,5))
 	   (ch)->real_abils.con--;
 	 affect_total(ch);
        }
      }

      if (FIGHTING(ch))
	stop_fighting(ch);

      if (FIGHTING(killer))
	stop_fighting(killer);

      GET_POS(ch) = POS_DEAD;
      raw_kill(ch, attacktype);
   }
}

/* used by people bleeding to death and legacy ranged
   weapons code */
void
die(struct char_data *ch)
{
   gain_exp(ch, -(GET_EXP(ch)/3));
   raw_kill(ch, TYPE_UNDEFINED);
}

/*
 * returns true if:
 * aff_grouped, in the same room with a group member
 */
static int
is_in_group(struct char_data *ch)
{
   struct char_data *k = ch->master;
   struct follow_type *f = NULL;

   if (!k)
   {/* leader */
      if (IS_AFFECTED(ch, AFF_GROUP))
	 for (f = ch->followers; f; f = f->next)
	    if(IS_AFFECTED(f->follower, AFF_GROUP) &&
	       f->follower->in_room == ch->in_room && f->follower != ch)
               return(TRUE);
   }
   else if (IS_AFFECTED(ch, AFF_GROUP))
      for (f = k->followers; f; f = f->next)
	 if( (IS_AFFECTED(f->follower, AFF_GROUP) &&
	      f->follower->in_room == ch->in_room && f->follower != ch) ||
	     (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room) )
	    return(TRUE);
   return(FALSE);
}

static int
calc_level_diff(struct char_data *ch, struct char_data *victim, int base)
{
   int level_diff = 0, share = MIN(max_exp_gain, MAX(1, base));
   struct char_data *k = ch->master;

   if (!k)
      k = ch;

   if ( (level_diff = GET_LEVEL(ch)-GET_LEVEL(victim)) > 0 )
   {
      if (!is_in_group(ch))
	 level_diff -= 2; /* give solo players a bit more slack */
      if(level_diff>15)
	 share -= (share*(.7));
      else if (level_diff>10)
	 share -= (share*(.5));
      else if (level_diff>5)
	 share -= (share*(.3));
      /* within 4 levels, give em full xp */
   }
   /* unless they're over level 20 :) */
   if (GET_LEVEL(ch)>20)
     share -= (share *(.2));
   share = MAX(share, 1);
   return(share);
}

void
perform_group_gain(struct char_data *ch, int base, struct char_data *victim)
{
   int share = calc_level_diff(ch, victim, base);

   if (share > 1)
   {
      sprintf(buf2, "You receive your share of experience -- %d points.\r\n",
	      share);
      send_to_char(buf2, ch);
   }
   else
      send_to_char("You receive your share of experience -- one measly little "
		   "point!\r\n", ch);

   if (!IS_NPC(ch))
      gain_exp(ch, share);
   change_alignment(ch, victim);
}

void
group_gain(struct char_data * ch, struct char_data * victim)
{
   int tot_members, base;
   struct char_data *k;
   struct follow_type *f;
   int gold_looted = 0;
   int gold_per_member = 0;

   if (!(k = ch->master))
      k = ch;

   if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      tot_members = 1;
   else
      tot_members = 0;

   for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  f->follower->in_room == ch->in_room)
	 tot_members++;

   /* round up to the next highest tot_members */
   base = (GET_EXP(victim)/tot_members);
   if (base >100)
     base -= base*.01;

   if (tot_members >= 1)
      base = MAX(1, base);
   else
      base = 0;

   if (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room)
      perform_group_gain(k, base, victim);

   for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP)&&
	  f->follower->in_room == ch->in_room)
	 perform_group_gain(f->follower, base, victim);

   if (PRF_FLAGGED(ch, PRF_AUTOGOLD))
   {
      gold_looted = GET_GOLD(victim) ;
      GET_GOLD(victim) = 0 ;
      if (tot_members >= 1)
         base = MAX(1, gold_looted / ( 3 * tot_members) ) ;
      else
         base = 0 ;

      if (PRF_FLAGGED(ch, PRF_AUTOSPLIT) && gold_looted)
      {
         sprintf(buf, "You loot %d coins from the corpse of %s.\r\n",
                 gold_looted, GET_NAME(victim) );
         send_to_char (buf, ch) ;
         sprintf(buf, "$n loots some gold from the corpse of %s.",
                 GET_NAME(victim) );
         act(buf, FALSE, ch, 0, NULL, TO_ROOM) ;

         if (tot_members > 1)
         {
            gold_per_member = gold_looted / tot_members ;
	    if (k != ch && gold_per_member > 0 && IN_ROOM(ch) ==
                 IN_ROOM(k))
	    {
	       sprintf(buf, "You share %d gold with %s.\r\n",
		       gold_per_member, GET_NAME(k) ) ;
	       send_to_char (buf, ch) ;
	       sprintf(buf, "%s splits some gold with you,",
		       GET_NAME(ch) ) ;
	       sprintf(buf, "%s you get %d.\r\n",
		       buf, gold_per_member) ;
	       send_to_char (buf, k) ;
	       GET_GOLD(k) += gold_per_member ;
	       gold_looted -= gold_per_member ;
	    }


            for (f = k->followers; f; f = f->next)
               if ( IS_AFFECTED(f->follower, AFF_GROUP) && f->follower!=ch &&
                    f->follower->in_room == ch->in_room )
               {
                  if (gold_per_member > 0)
                  {
                     sprintf(buf, "You share %d gold with %s.\r\n",
                             gold_per_member, GET_NAME(f->follower) ) ;
                     send_to_char (buf, ch) ;
                     sprintf(buf, "%s splits some gold with you,",
                             GET_NAME(ch) ) ;
                     sprintf(buf, "%s you get %d.\r\n",
                             buf, gold_per_member) ;
                     send_to_char (buf, f->follower) ;
                     GET_GOLD(f->follower) += gold_per_member ;
                     gold_looted -= gold_per_member ;
                  }
                  else
                  {
                     sprintf(buf, "You would share gold with %s, but there ",
                             GET_NAME(f->follower) ) ;
                     sprintf(buf, "%swas none to split!\r\n", buf) ;
                     send_to_char (buf, ch) ;
                     sprintf(buf, "%s would have shared some gold with you ",
                             GET_NAME(ch) ) ;
                     sprintf(buf, "%sbut there was none to split!\r\n", buf);
                     send_to_char (buf, f->follower) ;
                  }

               }
            if (gold_per_member > 0)
            {
               sprintf(buf, "You split the gold and keep %d",
                       gold_per_member) ;
               sprintf(buf, "%s for yourself.\r\n", buf ) ;
	       GET_GOLD(ch)+=gold_per_member;
               send_to_char(buf, ch) ;
            }
            else
               send_to_char("When you split no gold, you got none.\r\n",ch) ;

            gold_looted -= gold_per_member ;
            if (gold_looted > 0)
            {
               sprintf(buf, "You keep %d gold leftover.\r\n",
                       gold_looted) ;
               send_to_char(buf, ch) ;
               GET_GOLD(ch) += gold_looted ;
            }

         }
      }
      else if (gold_looted)
      {
         sprintf(buf, "You loot %d coins from the corpse of %s.\r\n",
                 gold_looted, GET_NAME(victim) ) ;
         send_to_char (buf, ch) ;
         sprintf(buf, "$n loots some gold from the corpse of %s.",
                 GET_NAME(victim) ) ;
         act(buf, FALSE, ch, 0, NULL, TO_ROOM) ;
         GET_GOLD(ch) += gold_looted ;
      }
   }

}



char *
replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
   static char buf[256];
   char *cp;

   cp = buf;

   for (; *str; str++)
   {
      if (*str == '#')
      {
	 switch (*(++str))
	 {
	 case 'W':
	    for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	    break;
	 case 'w':
	    for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	    break;
	 default:
	    *(cp++) = '#';
	    break;
	 }
      }
      else
	 *(cp++) = *str;

      *cp = 0;
   }				/* For */

   return (buf);
}


/* message for doing damage with a weapon */
void
dam_message(int dam, struct char_data * ch, struct char_data * victim,
	    int w_type)
{
   char *buf;
   int msgnum;

   static struct dam_weapon_type {
      char *to_room;
      char *to_char;
      char *to_victim;
   } dam_weapons[] = {

      /* use #w for singular (i.e. "slash") and
	 #W for plural (i.e. "slashes") */

      {
	 "$n tries to #w $N, but misses.",	/* 0: 0     */
	 "You try to #w $N, but miss.",
	 "$n tries to #w you, but misses."
      },

      {
	 "$n scratches $N as $e #W $M.",	/* 1: 1..2  */
	 "You scratch $N as you #w $M.",
	 "$n scratches you as $e #W you."
      },

      {
	 "$n barely #W $N.",		/* 2: 3..4  */
	 "You barely #w $N.",
	 "$n barely #W you."
      },

      {
	 "$n #W $N.",			/* 3: 5..6  */
	 "You #w $N.",
	 "$n #W you."
      },

      {
	 "$n #W $N hard.",			/* 4: 7..10  */
	 "You #w $N hard.",
	 "$n #W you hard."
      },

      {
	 "$n #W $N very hard.",		/* 5: 11..14  */
	 "You #w $N very hard.",
	 "$n #W you very hard."
      },

      {
	 "$n #W $N extremely hard.",	/* 6: 15..19  */
	 "You #w $N extremely hard.",
	 "$n #W you extremely hard."
      },

      {
	 "$n massacres $N to small fragments with $s #w.",	/* 7: 19..23 */
	 "You massacre $N to small fragments with your #w.",
	 "$n massacres you to small fragments with $s #w."
      },

      {
	 "$n OBLITERATES $N with $s deadly #w!!",	/* 8: 23-33   */
	 "You OBLITERATE $N with your deadly #w!!",
	 "$n OBLITERATES you with $s deadly #w!!"
      },

      {
	 "$n EVISCERATES $N with $s incredible #w!!",	/* 9: 33-43   */
	 "You EVISCERATE $N with your incredible #w!!",
	 "$n EVISCERATES you with $s incredible #w!!"
      },

      {
	 "$n DESTROYS $N with $s ungodly #w!!",	/* 10: 43-53   */
	 "You DESTROY $N with your ungodly #w!!",
	 "$n DESTROYS you with $s ungodly #w!!"
      },

      {
	 "$n ROCKS THE HELL OUT OF $N with $s ultimate #w!!",	/* 11: > 53   */
	 "You ROCK THE HELL OUT OF $N with your ultimate #w!!",
	 "$n ROCKS THE HELL OUT OF you with $s ultimate #w!!"
      }

   };


   w_type -= TYPE_HIT;		/* Change to base of table with text */

   if (dam == 0)		msgnum = 0;
   else if (dam <= 2)    msgnum = 1;
   else if (dam <= 4)    msgnum = 2;
   else if (dam <= 6)    msgnum = 3;
   else if (dam <= 10)   msgnum = 4;
   else if (dam <= 14)   msgnum = 5;
   else if (dam <= 19)   msgnum = 6;
   else if (dam <= 23)   msgnum = 7;
   else if (dam <= 33)   msgnum = 8;
   else if (dam <= 43)   msgnum = 9;
   else if (dam <= 53)   msgnum = 10;
   else			msgnum = 11;

   /* damage message to onlookers */
   buf = replace_string(dam_weapons[msgnum].to_room,
			attack_hit_text[w_type].singular,
			attack_hit_text[w_type].plural);
   act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

   /* damage message to damager */
   send_to_char(CCYEL(ch, C_CMP), ch);
   buf = replace_string(dam_weapons[msgnum].to_char,
			attack_hit_text[w_type].singular,
			attack_hit_text[w_type].plural);
   act(buf, FALSE, ch, NULL, victim, TO_CHAR);
   send_to_char(CCNRM(ch, C_CMP), ch);

   /* damage message to damagee */
   send_to_char(CCRED(victim, C_CMP), victim);
   buf = replace_string(dam_weapons[msgnum].to_victim,
			attack_hit_text[w_type].singular,
			attack_hit_text[w_type].plural);
   act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
   send_to_char(CCNRM(victim, C_CMP), victim);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int
skill_message(int dam, struct char_data * ch, struct char_data * vict,
	      int attacktype)
{
   int i, j, nr;
   struct message_type *msg;

   struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);

   for (i = 0; i < MAX_MESSAGES; i++)
   {
      if (fight_messages[i].a_type == attacktype)
      {
	 nr = dice(1, fight_messages[i].number_of_attacks);
	 for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	    msg = msg->next;

	 if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT))
	 {
	    act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	    act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	    act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	 }
	 else if (dam != 0)
	 {
	    if (GET_POS(vict) == POS_DEAD)
	    {
	       send_to_char(CCYEL(ch, C_CMP), ch);
	       act(msg->die_msg.attacker_msg,
		   FALSE, ch, weap, vict, TO_CHAR);
	       send_to_char(CCNRM(ch, C_CMP), ch);

	       send_to_char(CCRED(vict, C_CMP), vict);
	       act(msg->die_msg.victim_msg,
		   FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	       send_to_char(CCNRM(vict, C_CMP), vict);

	       act(msg->die_msg.room_msg,
		   FALSE, ch, weap, vict, TO_NOTVICT);
	    }
	    else
	    {
	       send_to_char(CCYEL(ch, C_CMP), ch);
	       act(msg->hit_msg.attacker_msg,
		   FALSE, ch, weap, vict, TO_CHAR);
	       send_to_char(CCNRM(ch, C_CMP), ch);

	       send_to_char(CCRED(vict, C_CMP), vict);
	       act(msg->hit_msg.victim_msg,
		   FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	       send_to_char(CCNRM(vict, C_CMP), vict);

	       act(msg->hit_msg.room_msg,
		   FALSE, ch, weap, vict, TO_NOTVICT);
	    }
	 }
	 else if (ch != vict)
	 {	/* Dam == 0 */
	    send_to_char(CCYEL(ch, C_CMP), ch);
	    act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	    send_to_char(CCNRM(ch, C_CMP), ch);

	    send_to_char(CCRED(vict, C_CMP), vict);
	    act(msg->miss_msg.victim_msg,
		FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	    send_to_char(CCNRM(vict, C_CMP), vict);

	    act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	 }
	 return 1;
      }
   }
   return 0;
}

/* **************************************************************************
   NAME       : attitude_loot()
   PARAMETERS : TYPE                NAME                     DESCRIPTION
                ----                ----                     -----------
                struct char_data *  ch                       killer info
		struct char_data *  victim                   dead guy's info
   PURPOSE    : controls looting and gossiping of attitudes after a kill
   RETURNS    : n/a
   HISTORY    : Created by dlkarnes 961012
   ************************************************************************ */
static void
attitude_loot(struct char_data *ch, struct char_data *victim)
{
   struct obj_data *obj;
   struct obj_data *tmp_obj;
   ACMD(do_get);
   ACMD(do_wear);

   if (!victim || !ch)
      return;
   /*
    * loot the corpse
    */
   do_get(ch, "all corpse", 0, 0);
   for (obj = ch->carrying; obj; )
   {
     tmp_obj = obj->next_content;
     if ((GET_OBJ_TYPE(obj) != ITEM_CONTAINER) &&
         (GET_OBJ_TYPE(obj) != ITEM_KEY) &&
         (GET_OBJ_COST(obj) <= 150))
     {
       /* fake a junking */
       act("You junk $p. It vanishes in a puff of smoke!", FALSE, ch, obj, 0, TO_CHAR);
       act("$n junks $p. It vanishes in a puff of smoke!", TRUE, ch, obj, 0, TO_ROOM);
       extract_obj(obj);
     }
     obj = tmp_obj;
   }
   do_wear(ch, "all", 0, 0);
   do_get(ch, "all corpse", 0, 0);
   for (obj = ch->carrying; obj; )
   {
     tmp_obj = obj->next_content;
     if ((GET_OBJ_TYPE(obj) != ITEM_CONTAINER) &&
         (GET_OBJ_TYPE(obj) != ITEM_KEY) &&
         (GET_OBJ_COST(obj) <= 150))
     {
       /* fake a junking */
       act("You junk $p. It vanishes in a puff of smoke!", FALSE, ch, obj, 0, TO_CHAR);
       act("$n junks $p. It vanishes in a puff of smoke!", TRUE, ch, obj, 0, TO_ROOM);
       extract_obj(obj);
     }
     obj = tmp_obj;
   }
   do_wear(ch, "all", 0, 0);


   /* log it */
   if (!IS_NPC(victim))
   {
     sprintf(buf, "(LOOT) %s attitude looted %s.", GET_NAME(ch), GET_NAME(victim));
     mudlog(buf, CMP, LVL_IMMORT, TRUE);
   }

   if (!MOB_FLAGGED(ch, MOB_AGGR24))  /* if not att, just loot */
     return;

   /*
    * brag about it
    */

   if (!can_speak(ch))  /* if they can't talk anyway don't brag */
     return;

   if (ch == victim)  /* if he kills himself, don't tell anyone */
     return;

   if (!IS_MOB(victim) || !number(0,20))
   {
      switch(number(1,12))
      {
      case 1:
	 sprintf(buf2, "I killed %s and looted %s stinkin' corpse!",
		 GET_NAME(victim), HSHR(victim));
	 break;
      case 2:
	 sprintf(buf2, "%s was tough, but had good eq...",
		 GET_NAME(victim));
	 break;
      case 3:
	 sprintf(buf2, "%s was easy xp.",
		 GET_NAME(victim));
	 break;
      case 4:
	 sprintf(buf2, "Muhahahaha... %s is dead!",
		 GET_NAME(victim));
	 break;
      case 5:
	 if (IS_EVIL(ch) && !IS_EVIL(victim))
	   sprintf(buf2, "Now you will see that evil will always triumph, "
			 "because good is dumb.");
	 else
	   sprintf(buf2, "%s is dead! R.I.P.",
		 	 GET_NAME(victim));

	break;
      case 6:
	 sprintf(buf2, "Kill number %d: %s.",
		 (int)GET_KILLS(ch), GET_NAME(victim));
	break;
      case 7:
	 if (!IS_NPC(victim))
	   sprintf(buf2, "Oh, did that hurt, %s? *innocent stare*",
		GET_NAME(victim));
	 else
	   return;
	 break;
      case 8:
	 if (!IS_NPC(victim))
	   sprintf(buf2, "What the hell was %s doing out of newbie training?",
		 GET_NAME(victim));
	 else
	  return;
	break;
      case 9:
	 if (!IS_NPC(victim))
	   sprintf(buf2, "I think I finally found a use for that punk %s: "
		"fertilizer!", GET_NAME(victim));
	 else
	   return;
	break;
      case 10:
	 if (!IS_NPC(victim))
	   sprintf(buf2, "Hrmm.. Is this your head, %s? *cackle*",
		 GET_NAME(victim));
	 else
	   return;
	break;
      case 11:
	 if (!IS_NPC(victim))
	   sprintf(buf2,
		   "Hey %s, was that suicide or did you try to fight back?",
		   GET_NAME(victim));
	 else
	   return;
	break;
      default:
	 return;
      }
      do_gen_comm(ch, buf2, 0, SCMD_GOSSIP);
   }
}


static void
counter_procs(struct char_data *ch)
{
   struct descriptor_data *i;
   int reward=FALSE;

   if (IS_NPC(ch) || IS_MOB(ch))
     return;

   switch(GET_KILLS(ch))
   {
   case 5000:
   case 15000:
   case 25000:
   case 35000:
   case 45000:
      send_to_char("The gods reward your glory in battle!\n\r",ch);
      reward=TRUE;
      GET_HIT(ch)=GET_MAX_HIT(ch);
      break;
   case 1000:
   case 2000:
   case 10000:
   case 20000:
   case 30000:
   case 40000:
   case 50000:
      send_to_char("The gods reward your many victories!\n\r",ch);
      reward=TRUE;
      switch(number(1,3)) {
      case 1:
	 GET_MAX_HIT(ch)++;
      case 2:
	 GET_MAX_MANA(ch)++;
      case 3:
	 GET_MAX_MOVE(ch)++;
      default:
	 GET_MAX_HIT(ch)++;
	 break;
      }
      GET_HIT(ch)=GET_MAX_HIT(ch);
      break;
   default:
      return;
   }

   if(reward)
   {
      char msg[256];
      for (i = descriptor_list; i; i = i->next)
	 if (!i->connected && i != ch->desc)
	 {
            stc("The gods of war and death bestow a blessing upon you.\r\n",
		i->character);
	    /*heal everyone in game*/
	    GET_HIT(i->character)=GET_MAX_HIT(i->character);
	 }
      sprintf(msg, "%s hit %ld kills.", GET_NAME(ch), GET_KILLS(ch));
      mudlog(msg, NRM, LVL_IMMORT, FALSE);
    }
}

bool
damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype)
{
   int exp, i;
   ACMD(do_dismount);

   if (GET_POS(victim) <= POS_DEAD) {
     /* This is "normal"-ish now with delayed extraction. -gg 3/15/2001 */
     if (PLR_FLAGGED(victim, PLR_EXTRACT) || MOB_FLAGGED(victim, MOB_EXTRACT))
       return FALSE;

      log("SYSERR: Attempt to damage a corpse.");
      return FALSE;			/* -je, 7/7/92 */
   }

   if (ch->in_room != victim->in_room)
     if (GET_LEVEL(ch) < LVL_IMMORT)
     {
       mudlog("Attempt to assign damage when ch and vict are in different rooms.", NRM, LVL_IMMORT,FALSE);
       return FALSE;
     }

   /* peaceful rooms */
   if (!IS_OUTLAW(victim) && FIGHTING(victim) != ch)  /* Afford outlaws no protection */
     if (ch != victim && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
     {
        send_to_char("This room just has such a peaceful, easy feeling...\r\n",
  		   ch);
        return FALSE;
     }

   if (victim != ch) {
     if (!IS_NPC(ch) && !IS_NPC(victim) && GET_LEVEL(ch) <= 10) {
        act("You are not experienced enough to attack $N!", FALSE,
           ch, 0, victim, TO_CHAR);
        return FALSE;
     }

     if (!IS_NPC(ch) && !IS_NPC(victim) && GET_LEVEL(victim) <= 10 &&
         !PLR_FLAGGED(victim, PLR_OUTLAW)) {
        act("Ancient forces protect $N from your wrath!", FALSE, ch, 0,
           victim, TO_CHAR);
        return FALSE;
     }
   }

   /* shopkeeper protection */
   if (!ok_damage_shopkeeper(ch, victim) || is_shopkeeper(victim))
   {
     stc("Ha ha... Don't think so.\r\n", ch);
     if (FIGHTING(ch))
       stop_fighting(ch);
     if (FIGHTING(victim))
       stop_fighting(victim);
     return FALSE;
   }

   if (IS_NPC(ch) && !IS_NPC(victim) && (GET_MOB_SPEC(ch)==take_to_jail ||
       GET_MOB_SPEC(ch)==wall_guard_ns))
   {
     /* Only saves them if he can see them, and he's not wounded too bad */
     if ( CAN_SEE(ch, victim) && GET_HIT(ch)>(GET_MAX_HIT(ch)/2) &&
         (!( (IS_AFFECTED(victim, AFF_VAMPIRE) ||
	  IS_AFFECTED(victim, AFF_WEREWOLF)) )) )
     {
       if ( FIGHTING(victim) )
         if (FIGHTING(FIGHTING(victim))==victim)
           stop_fighting(FIGHTING(victim));
         GET_HIT(victim) = 1;
         stop_fighting(victim);
	 if (MOB_FLAGGED(ch, MOB_MEMORY))
	   forget(ch, victim);
	 if (HUNTING(ch) == victim)
	   set_hunting(ch, NULL);
         act("$n grabs $N by the collar, and quickly beats $M into"
                 " submission.\r\nJerking $M to $S feet, $n carts $N off"
	         " to jail.", TRUE, ch, 0, victim, TO_NOTVICT);
         act("$n grabs you by the collar and quickly beats you "
                  "into submission.", TRUE, ch, 0, victim, TO_VICT);
         send_to_char("Jerking you to your feet, he carts you off "
                           "to jail...\r\n", victim);
	 unmount(victim, get_mount(victim));
         char_from_room(victim);
         char_to_room(victim,real_room(8118));
         look_at_room(victim, 0);
	 GET_JAIL_TIMER(victim) = MAX(2, GET_LEVEL(victim)/2);
         return FALSE;
      }
   }

   if (victim != ch)
   {
      if (GET_POS(ch) > POS_STUNNED)
      {
	 if (!(FIGHTING(ch)))
	    set_fighting(ch, victim);  /* Fighting starts here */

	 if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	     !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
	     (victim->master->in_room == ch->in_room))
	 {
	    if (FIGHTING(ch))
	       stop_fighting(ch);
	    hit(ch, victim->master, TYPE_UNDEFINED);
	    return FALSE;
	 }
	  /* switcheroo */
	 if (IS_NPC(ch) && GET_LEVEL(ch)>20)
	 {
	  struct char_data *vict = NULL;
	  bool found = FALSE;

  	  for (vict = world[ch->in_room].people;
	       vict;
	       vict = vict->next_in_room)
    	    if (FIGHTING(vict) == ch && !number(0, 80))
      	    {
        	found = TRUE;
        	break;
      	    }
  	  if (found && vict != NULL)
	  {
	    if (FIGHTING(ch))
	       stop_fighting(ch);
	    hit(ch, vict, TYPE_UNDEFINED);
	    return FALSE;
	  }
	 }

      }
      if (GET_POS(victim) > POS_STUNNED && !FIGHTING(victim))
      {
	 set_fighting(victim, ch);
	 if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
	     (GET_LEVEL(ch) < LVL_IMMORT))
	    remember(victim, ch);
	 if (MOB_FLAGGED(victim, MOB_HUNTER) && !IS_NPC(ch) &&
	     (GET_LEVEL(ch) < LVL_IMMORT))
	    set_hunting(victim, ch);
      }
      if (MOB_FLAGGED(ch, MOB_HUNTER) && !IS_NPC(victim) &&
	  (GET_LEVEL(victim) < LVL_IMMORT))
	 set_hunting(ch, victim);
   }
   if (victim->master == ch)
      stop_follower(victim);

   if (IS_AFFECTED(ch, AFF_HIDE))  /* we do this inline now, instead of calling appear() */
   {
     REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
     act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
   }

   for (i = 0; i < 5; i++)
      if (GET_RACE_HATE(ch, i) == GET_RACE(victim))
	 dam += GET_LEVEL(ch); /* extra damage for race_hate weapons */

   if (IS_AFFECTED(victim, AFF_SANCTUARY))
      dam /= 2;		/* 1/2 damage when sanctuary */

   if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
      dam -= GET_LEVEL(victim)/4;

   if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
      dam -= GET_LEVEL(victim)/4;

   /* You can't damage an immortal! */
   if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_IMMORT))
      dam = 0;

   dam = MAX(MIN(dam, 3000), 0);
   GET_HIT(victim) -= dam;

   if (ch != victim && !IS_NPC(ch) && GET_LEVEL(ch)<2)
     gain_exp(ch, GET_LEVEL(victim) * dam);

   update_pos(victim);

   if (GET_POS(victim) <= POS_STUNNED)
   {
      if (IS_NPC(ch) && !IS_NPC(victim) && GET_LEVEL(victim) <= 5)
        stop_fighting(ch);

      if (!IS_NPC(victim) && ROOM_FLAGGED(victim->in_room,ROOM_NEUTRAL))
      {
         if ( FIGHTING(victim) )
            if (FIGHTING(FIGHTING(victim))==victim)
              stop_fighting(FIGHTING(victim));
         GET_HIT(victim) = 1;
         stop_fighting(victim);
	 if (IS_NPC(ch))
         {
          if (MOB_FLAGGED(ch, MOB_MEMORY))
            forget(ch, victim);
          if (HUNTING(ch) == victim)
            set_hunting(ch, NULL);
	 }
         act("$n is startled as $N is saved by the powers of the gods!",
		TRUE, ch, 0, victim, TO_NOTVICT);
         act("$N is saved by the powers of the gods!",
                TRUE, ch, 0, victim, TO_CHAR);
         send_to_char("You are saved by the gods!\r\n", victim);
         unmount(victim, get_mount(victim));
         char_from_room(victim);
         char_to_room(victim, real_room(8004));
         look_at_room(victim, 0);
         return FALSE;
      }
   }

   /*
    * skill_message sends a message from the messages file in lib/misc.
    * dam_message just sends a generic "You hit $n extremely hard.".
    * skill_message is preferable to dam_message because it is more
    * descriptive.
    *
    * If we are _not_ attacking with a weapon (i.e. a spell), always use
    * skill_message. If we are attacking with a weapon: If this is a miss or a
    * death blow, send a skill_message if one exists; if not, default to a
    * dam_message. Otherwise, always send a dam_message.
    */
   if (!IS_WEAPON(attacktype))
      skill_message(dam, ch, victim, attacktype);
   else
   {
      if (GET_POS(victim) == POS_DEAD || dam == 0)
      {
	 if (!skill_message(dam, ch, victim, attacktype))
	    dam_message(dam, ch, victim, attacktype);
      }
      else
	 dam_message(dam, ch, victim, attacktype);
   }

   /* chance to dismount victim on a hit */
   if (!IS_NPC(victim) && IS_MOUNTED(victim) && dam > 0 && number(0, 99) < 10 ) {
     act("The hit knocks you off of your mount!\r\n", 0, victim, 0, 0, TO_CHAR);
     act("$n gets knocked off of his mount.", 0, victim, 0, 0, TO_ROOM);
     do_dismount(victim, NULL, 0, 0);
     if (number(0,99) < (GET_DEX(victim) * 1.5)) {
       send_to_char("You land on your feet!\r\n", ch);
     } else {
        send_to_char("You hit the ground with a thud.\r\n", ch);
        GET_POS(ch) = POS_SITTING;
      }
   }

   /* Use send_to_char -- act() doesn't send message if you are DEAD. */
   switch (GET_POS(victim))
   {
   case POS_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.",
	  TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are mortally wounded, and will die soon, if not "
		   "aided.\r\n", victim);
      break;
   case POS_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.",
	  TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are incapacitated an will slowly die, "
		   "if not aided.\r\n", victim);
      break;
   case POS_STUNNED:
      act("$n is stunned, but will probably regain consciousness again.",
	  TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness "
		   "again.\r\n", victim);
      break;
   case POS_DEAD:
      act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
      send_to_char("You are dead!  Sorry...\r\n", victim);
      break;

   default:			/* >= POSITION SLEEPING */
      if (dam > (GET_MAX_HIT(victim) / 4))
      {
	 act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);
	 if (!number(0,2))
	   act("$N screams in pain!", FALSE, ch, 0, victim, TO_NOTVICT);
      }

      if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 4)) {
	 sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING "
		 "so much!%s\r\n", CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
	 send_to_char(buf2, victim);
	 if (MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim))
	    do_flee(victim, "", 0, 0);
      }
      if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	  GET_POS(victim)>=POS_FIGHTING &&
	  GET_HIT(victim) < GET_WIMP_LEV(victim))
      {
	 send_to_char("You wimp out, and attempt to flee!\r\n", victim);
	 if (GET_SKILL(victim, SKILL_RETREAT)
             || GET_SKILL(victim, SKILL_ESCAPE))
	   do_retreat(victim, "", 0, 0);
	 else
	   do_flee(victim, "", 0, 0);
      }
      break;
   }

/*  removed because of player abuse 19991111 -rparet
   if (!IS_NPC(victim) && !(victim->desc))
   {
      do_flee(victim, "", 0, 0);
      if (!FIGHTING(victim))
      {
	 act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
	 GET_WAS_IN(victim) = victim->in_room;
	 char_from_room(victim);
	 char_to_room(victim, 0);
      }
   }

*/

   if (!AWAKE(victim))
      if (FIGHTING(victim))
	 stop_fighting(victim);

   if (GET_POS(victim) == POS_DEAD)
   {
     if (IS_NPC(victim)) {
       if (is_in_group(ch))
	 group_gain(ch, victim);
       else {
         exp = MIN(max_exp_gain, GET_EXP(victim));

         exp = calc_level_diff(ch, victim, exp);

         if (exp > 1)
         {
           sprintf(buf2, "You receive %d experience points.\r\n", exp);
           send_to_char(buf2, ch);
         }
         else
           send_to_char("You receive one lousy experience point.\r\n", ch);
         if (!IS_NPC(ch))
           gain_exp(ch, exp);

         if (PRF_FLAGGED(ch, PRF_AUTOGOLD))
         {
           if ( GET_GOLD(victim) != 0 )
           {
             sprintf(buf, "You loot %d gold from the corpse.\r\n",
                     GET_GOLD(victim) ) ;
             send_to_char (buf, ch) ;
             sprintf( buf, "$n loots some gold from the corpse of $N." );
             act( buf, FALSE, ch, 0, victim, TO_ROOM ) ;
             GET_GOLD(ch) += GET_GOLD(victim) ;
             GET_GOLD(victim) = 0 ;
           }
         }
         change_alignment(ch, victim);
       }
     }

     if (!IS_NPC(victim)) {
       if(!IS_NPC(ch) && (ch != victim)) { /* Pkill */
         sprintf(buf2, "(PK) %s killed by %s at %s", GET_NAME(victim),
                 GET_NAME(ch), world[victim->in_room].name);
         if(!PLR_FLAGGED(victim, PLR_OUTLAW))
           SET_BIT_AR(PLR_FLAGS(ch), PLR_OUTLAW);
       } else {
         sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
                 world[victim->in_room].name);
       }
       mudlog(buf2, BRF, LVL_IMMORT, TRUE);
       if (ch != victim)
         GET_PKS(ch)++;
       GET_DEATHS(victim)++;
       GET_LAST_DEATH(victim) = (long)time(0);
       /* REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_OUTLAW); 20000626 -rparet */
     }

     GET_KILLS(ch)++;
     counter_procs(ch);
     die_with_killer(victim, ch, attacktype);

     if (ch != victim &&
         (MOB_FLAGGED(ch, MOB_AGGR24) ||
          MOB_FLAGGED(ch, MOB_LOOTS)  ||
          GET_MOB_VNUM(ch) == 19650))
       attitude_loot(ch, victim);

     if(GET_MOB_VNUM(ch) == 19405) /* quan lo */
     {
       char msg[256];
       sprintf(msg, "%s has just learned the Five Fingers of Enlightenment. "
               "Who else wishes to learn from the master?",
               GET_NAME(victim));
       do_gen_comm(ch, msg, 0, SCMD_GOSSIP);
     }

     if (!IS_NPC(ch) && IS_NPC(victim) && ch != victim && PRF_FLAGGED(ch, PRF_AUTOLOOT))
     {
       ACMD(do_get);
       do_get(ch, "all corpse", 0, 0);
     }
   }

   if (dam)
     return TRUE;
   else
     return FALSE;
}

int
get_minusdam(int dam, struct char_data *ch)
{
   double pcmod = 2.0;
   int ac = GET_AC(ch);

   if (ac > 90)    return (dam);
   if (ac > 80)    return (dam - (dam*(.01*pcmod)));
   if (ac > 70)    return (dam - (dam*(.02*pcmod)));
   if (ac > 60)    return (dam - (dam*(.03*pcmod)));
   if (ac > 50)    return (dam - (dam*(.04*pcmod)));
   if (ac > 40)    return (dam - (dam*(.05*pcmod)));
   if (ac > 30)    return (dam - (dam*(.06*pcmod)));
   if (ac > 20)    return (dam - (dam*(.07*pcmod)));
   if (ac > 10)    return (dam - (dam*(.08*pcmod)));
   if (ac > 0)     return (dam - (dam*(.10*pcmod)));
   if (ac > -10)   return (dam - (dam*(.11*pcmod)));
   if (ac > -20)   return (dam - (dam*(.12*pcmod)));
   if (ac > -30)   return (dam - (dam*(.13*pcmod)));
   if (ac > -40)   return (dam - (dam*(.14*pcmod)));
   if (ac > -50)   return (dam - (dam*(.15*pcmod)));
   if (ac > -60)   return (dam - (dam*(.16*pcmod)));
   if (ac > -70)   return (dam - (dam*(.17*pcmod)));
   if (ac > -80)   return (dam - (dam*(.18*pcmod)));
   if (ac > -90)   return (dam - (dam*(.19*pcmod)));
   if (ac > -95)   return (dam - (dam*(.20*pcmod)));
   if (ac > -110)  return (dam - (dam*(.21*pcmod)));
   if (ac > -130)  return (dam - (dam*(.22*pcmod)));
   if (ac > -150)  return (dam - (dam*(.23*pcmod)));
   if (ac > -170)  return (dam - (dam*(.24*pcmod)));
   if (ac > -190)  return (dam - (dam*(.25*pcmod)));
   if (ac > -210)  return (dam - (dam*(.26*pcmod)));
   if (ac > -230)  return (dam - (dam*(.27*pcmod)));
   if (ac > -250)  return (dam - (dam*(.28*pcmod)));
   if (ac > -270)  return (dam - (dam*(.29*pcmod)));
   if (ac > -290)  return (dam - (dam*(.30*pcmod)));
   if (ac > -310)  return (dam - (dam*(.31*pcmod)));
 return(dam - (dam*(.32*pcmod)));
}


void
hit(struct char_data * ch, struct char_data * victim, int type)
{
   struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
   int w_type, victim_ac, calc_thaco, dam, diceroll;

   extern int thaco[NUM_CLASSES][LVL_IMPL+1];
   extern struct str_app_type str_app[];
   extern struct dex_app_type dex_app[];

   int backstab_mult(int level);

   if (ch->in_room != victim->in_room)
   {
      if (FIGHTING(ch) && FIGHTING(ch) == victim)
	 stop_fighting(ch);
      return;
   }

   /* Calculate the raw armor including magic armor.  Lower AC is better. */

   if (!IS_NPC(ch))
      calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
   else		/* THAC0 for monsters is set in the HitRoll */
      calc_thaco = 20;

   calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;

   if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
   {
      w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
      if (IS_OBJ_STAT(wielded, ITEM_BLESS))
	 calc_thaco -=1;
   }
   else
   {
      if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
	 w_type = ch->mob_specials.attack_type + TYPE_HIT;
      else
      {
	 if (IS_AFFECTED(ch, AFF_FLESH_ALTER))
	    w_type = TYPE_HIT+flesh_altered_type(ch);
	 else
	    w_type = TYPE_HIT;
      }
   }

   if (GET_COND(ch, DRUNK) > 1)
      calc_thaco += 2;

   calc_thaco -= GET_HITROLL(ch);
   calc_thaco -= (int) ((GET_INT(ch) - 13) / 1.5);  /* Intelligence helps! */
   calc_thaco -= (int) ((GET_WIS(ch) - 13) / 1.5);	/* So does wisdom */
   diceroll = number(1, 20);

   victim_ac = GET_AC(victim) / 10;

   if (AWAKE(victim))
      victim_ac += dex_app[GET_DEX(victim)].defensive;

   victim_ac = MAX(-10, victim_ac);	/* -10 is lowest */

   /* decide whether this is a hit or a miss */
   if ((((diceroll < 20) && AWAKE(victim)) &&
	((diceroll == 1) || ((calc_thaco - diceroll) > victim_ac))))
   {
      if (type == SKILL_BACKSTAB)
	 damage(ch, victim, 0, SKILL_BACKSTAB);
      else if (type == SKILL_CIRCLE)
	 damage(ch, victim, 0, SKILL_CIRCLE);
      else if (type == SKILL_DISEMBOWEL)
         damage(ch, victim, 0, SKILL_DISEMBOWEL);
      else
	 damage(ch, victim, 0, w_type);
   }
   else
   {
      /* okay, we know the guy has been hit.  now calculate damage. */
      dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
      dam += GET_DAMROLL(ch);

      if (!IS_NPC(ch) && wielded)
	 dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
      else
      {
	 if (IS_NPC(ch))
	    dam += dice(ch->mob_specials.damnodice,
			ch->mob_specials.damsizedice);
	 else
	    dam += number(0, GET_LEVEL(ch)/3);/* Max. lvl/3 dam w bare hands */
      }

      if (GET_POS(victim) < POS_FIGHTING)
	 dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
      /* Position  sitting  x 1.33 */
      /* Position  resting  x 1.66 */
      /* Position  sleeping x 2.00 */
      /* Position  stunned  x 2.33 */
      /* Position  incap    x 2.66 */
      /* Position  mortally x 3.00 */

      dam = MAX(1, dam);		/* at least 1 hp damage min per hit */

      if (type == SKILL_BACKSTAB)
      {
	 dam *= backstab_mult(GET_LEVEL(ch));
	 damage(ch, victim, dam, SKILL_BACKSTAB);
      }
      else if (type == SKILL_CIRCLE)
      {
	 dam *= (backstab_mult(GET_LEVEL(ch))/3);
	 damage(ch, victim, dam, SKILL_CIRCLE);
      }
      else if (type == SKILL_DISEMBOWEL)
      {
        dam = ((GET_LEVEL(ch) * 2) + GET_DAMROLL(ch));
        damage(ch, victim, dam, SKILL_DISEMBOWEL);
      }
      else
      {
	 dam = get_minusdam(dam, victim);
	 if (GET_STR(ch) ==0)
	    dam = 1;
	 damage(ch, victim, dam, w_type);
      }
   }

   if (IS_NPC(ch) && GET_POS(ch) > POS_STUNNED)
     if (GET_MOB_SCRIPT(ch) && MOB_SCRIPT_FLAGGED(ch, MS_FIGHTING))
       run_script(victim, ch, NULL, &world[ch->in_room], NULL, "fight", LT_MOB);
}



/* control the fights going on.  Called every 2 seconds from comm.c. */
void
perform_violence(void)
{
   struct char_data *ch;
   extern struct index_data *mob_index;
   extern struct dex_app_type dex_app[];
   float i = 0;
   float attacks = 4;

   for (ch = combat_list; ch; ch = next_combat_list)
   {
      next_combat_list = ch->next_fighting;

      if (IS_NPC(ch))
      {
	 if (GET_LEVEL(ch)>=31) attacks = 5;
	 if (GET_LEVEL(ch)<=30) attacks = 4;
	 if (GET_LEVEL(ch)<=27) attacks = 3;
	 if (GET_LEVEL(ch)<=20) attacks = 2;
	 if (GET_LEVEL(ch)<=10) attacks = 1;
	 if (number(0, 900)<GET_LEVEL(ch))
	    attacks++;
         if(IS_AFFECTED(ch, AFF_HASTE))
           attacks++;
         if(IS_AFFECTED(ch, AFF_SLOW))
           attacks--;
     }
      else
      {
	 attacks=1;
	 if((IS_WARRIOR(ch) || IS_PALADIN(ch) || IS_RANGER(ch))&&
	    (GET_LEVEL(ch)>10)&& (number(1,100)<60+GET_LEVEL(ch)) )
	    attacks++;
         if((GET_CLASS(ch)==CLASS_AVATAR || GET_CLASS(ch)==CLASS_NINJA)
            && (GET_LEVEL(ch)>12) &&
            (number(1,100)<60+GET_LEVEL(ch)) )
            attacks++;
	 if((GET_CLASS(ch)==CLASS_THIEF || GET_CLASS(ch) == CLASS_ASSASSIN)&&
	    (GET_LEVEL(ch)>15)&& (number(1,100)<30+GET_LEVEL(ch)) )
	    attacks++;
	 if((GET_LEVEL(ch)>25)&& (number(1,100)<75) )
	    attacks++;
	 if((GET_LEVEL(ch)>30) || (!number(0, 500)))
	    attacks++;
	 if((GET_LEVEL(ch)>39))
	    attacks=attacks+2;
         if(IS_AFFECTED(ch, AFF_HASTE))
            attacks++;
         if(IS_AFFECTED(ch, AFF_SLOW))
            attacks--;
      }

      if (!IS_NPC(ch) && number(0,10000)<=GET_SKILL(ch,SKILL_PARRY) &&
	FIGHTING(ch) &&
	FIGHTING(FIGHTING(ch)) == ch)
      {
	 /*
	  * IS_PARRIED is set here, or in do_parry()  and unset at end of
	  * this function (perform_violence())
	  */
	 send_to_char("With a dazzling show of swordplay, you move into "
		      "defensive position...\r\n", ch);
	 act("$n displays a dazzling show of swordplay, fending off $N's every"
	     " blow!", TRUE, ch, 0, FIGHTING(ch), TO_ROOM);
	 act("$n displays a dazzling show of swordplay, fending off your every"
	     " blow!", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
	 IS_PARRIED(FIGHTING(ch)) = TRUE;
      }
      if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_DODGE) &&
          number(0,100)<GET_LEVEL(ch) && FIGHTING(ch) &&
	  FIGHTING(FIGHTING(ch)) == ch)
      {
	 send_to_char("You dodge!\r\n", ch);
	 act("$n dodges $N's attack!", FALSE, ch, 0, FIGHTING(ch), TO_NOTVICT);
	 act("$n dodges your attack!", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
	 IS_PARRIED(FIGHTING(ch)) = TRUE;
      }

      if (IS_NPC(ch))
        {
	 if (GET_MOB_WAIT(ch) > 0)
	 {
	   GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
           attacks = 0;
	 }
	 if ((GET_POS(ch) < POS_FIGHTING) && !GET_MOB_WAIT(ch))
	 {
	   GET_POS(ch) = POS_FIGHTING;
	   act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
	   send_to_char("You drag yourself to your feet.\r\n", ch);
         }
	}

      if (!IS_NPC(ch))
        {
          if ((GET_POS(ch) < POS_FIGHTING) && !CHECK_WAIT(ch))
            {
              GET_POS(ch) = POS_FIGHTING;
              act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
              stc("You drag yourself to your feet.\r\n", ch);
            }
        }
      if (IS_PARRIED(ch)) {
        if (dex_app[GET_DEX(FIGHTING(ch))].defensive < 0)
          attacks += dex_app[GET_DEX(FIGHTING(ch))].defensive;
        else
          attacks--;
      }

      if (attacks < 0)  /* sanity check for slow -rparet */
         attacks = 0;

      i=0;
      while(i<attacks)
      {

	 if (FIGHTING(ch) && ch != FIGHTING(ch))
	 {
	    if (AWAKE(ch) && (ch->in_room == FIGHTING(ch)->in_room))
	       hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
	    else /* Not in same room */
	       if (FIGHTING(ch))
		  stop_fighting(ch);
	 }
	 else if (FIGHTING(ch)) /*ch fighting themselves*/
	    stop_fighting(ch);

	 i++;
      }

      if (IS_PARRIED(ch))
	 IS_PARRIED(ch) = FALSE;

      if (MOB_FLAGGED(ch, MOB_SPEC) && GET_MOB_SPEC(ch) && !MOB_FLAGGED(ch, MOB_EXTRACT))
	 (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
   }
}
