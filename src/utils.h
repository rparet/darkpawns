/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
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

/* $Id: utils.h 1453 2008-05-15 01:13:37Z jravn $ */

#ifndef _UTILS_H
#define _UTILS_H

/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern int has_boat(struct char_data *ch);

extern FILE * logfile;

#define log basic_mud_log

/* public functions in utils.c */
char	*str_dup(const char *source);
int	str_cmp(char *arg1, char *arg2);
int	strn_cmp(char *arg1, char *arg2, int n);
void    basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
bool    matches(const char *s, const char *regex);
/* void	log(char *str); */
void	alog (char const *str, ...);
int	touch(char *path);
void	mudlog(char *str, char type, int level, byte file);
void	log_death_trap(struct char_data *ch);
int	number(int from, int to);
int	dice(int number, int size);
float   uniform();
void	sprintbit(long vektor, char *names[], char *result);
void	sprinttype(int type, char *names[], char *result);
int	get_line(FILE *fl, char *buf);
int	get_filename(char *orig_name, char *filename, int mode);
struct time_info_data age(struct char_data *ch);
struct time_info_data playing_time(struct char_data *ch);
bool    is_veteran(struct char_data *ch);
int	num_pc_in_room(struct room_data *room);
char   *tprintf(char *fmt, ...);
void    sprintbitarray(int bitvector[], char *names[], int maxar, char *result);
void    core_dump_real(const char *, int);
bool    check_dead(struct char_data *ch);

#define core_dump()             core_dump_real(__FILE__, __LINE__)


/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);

/* in magic.c */
bool	circle_follow(struct char_data *ch, struct char_data * victim);

/* in act.informative.c */
void	look_at_room(struct char_data *ch, int mode);

/* in act.movmement.c */
int	do_simple_move(struct char_data *ch, int dir, int following);
int	perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int	mana_limit(struct char_data *ch);
int	hit_limit(struct char_data *ch);
int	move_limit(struct char_data *ch);
int	mana_gain(struct char_data *ch);
int	hit_gain(struct char_data *ch);
int	move_gain(struct char_data *ch);
void	advance_level(struct char_data *ch);
void	set_title(struct char_data *ch, char *title);
void	gain_exp(struct char_data *ch, int gain);
void	gain_exp_regardless(struct char_data *ch, int gain);
void	gain_condition(struct char_data *ch, int condition, int value);
void	check_idling(struct char_data *ch);
void	point_update(void);
void	update_pos(struct char_data *victim);
bool    are_grouped(struct char_data *ch1, struct char_data *ch2);   


/* various constants *****************************************************/


/* defines for mudlog() */
#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3

/* get_filename() */
#define CRASH_FILE	0
#define ETEXT_FILE	1
#define ALIAS_FILE      2
#define POOF_FILE       3

/* breadth-first searching */
#define BFS_ERROR		-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH		-3

/* idle time-out */
#define IDLE_TO_VOID		8
#define IDLE_DISCONNECT		30

/* mud-life time */
#define SECS_PER_MUD_HOUR	63
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH	(35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR	(17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)

/* Misc. defines (rparet) */

#define IS_FLYING(ch)   (IS_AFFECTED((ch), AFF_FLY) || \
                           PRF_FLAGGED((ch),PRF_NOHASSLE))                 
#define CAN_SWIM(ch)    (IS_AFFECTED((ch), AFF_WATERWALK) || \
                         IS_AFFECTED((ch), AFF_FLY) || \
                         PRF_FLAGGED((ch), PRF_NOHASSLE) || \
                         has_boat((ch)))

/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/

#define CREATE(result, type, number)  do {\
  if ((number) * sizeof(type) <= 0) \
    log("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);	\
  if (!((result) = (type *) calloc ((number), sizeof(type)))) \
    { perror("SYSERR: malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number)))) \
    { perror("SYSERR: realloc failure"); abort(); } } while(0)

#define FREE(pointer) do { free(pointer); (pointer) = NULL; } while(0)

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)	\
   if ((item) == (head))		\
      head = (item)->next;		\
   else {				\
      temp = head;			\
      while (temp && (temp->next != (item))) \
	 temp = temp->next;		\
      if (temp)				\
         temp->next = (item)->next;	\
   }					\


/* basic bitvector utils *************************************************/

#define Q_FIELD(x)        ((int) (x) >> 5)
#define Q_BIT(x)          (1 << ((x) & 0x1F))

#define IS_SET_AR(var, bit)     ((var)[Q_FIELD(bit)] & Q_BIT(bit))
#define SET_BIT_AR(var, bit)    ((var)[Q_FIELD(bit)] |= Q_BIT(bit))
#define REMOVE_BIT_AR(var, bit) ((var)[Q_FIELD(bit)] &= ~Q_BIT(bit))
#define TOGGLE_BIT_AR(var, bit) ((var)[Q_FIELD(bit)] = \
                                 (var)[Q_FIELD(bit)] ^ Q_BIT(bit))

#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))


#define EXIT_FLAGGED(exit, flag) (IS_SET((exit)->exit_info, (flag)))
#define MOB_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PRF_FLAGS(ch) ((ch)->player_specials->saved.pref)
#define AFF_FLAGS(ch) ((ch)->char_specials.saved.affected_by)
#define ROOM_FLAGS(loc) (world[(loc)].room_flags)

#define IS_NPC(ch)  (IS_SET_AR(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET_AR(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET_AR(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET_AR(PRF_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET_AR(ROOM_FLAGS(loc), (flag)))
#define OBJ_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_EXTRA(obj), (flag)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PLR_FLAGS(ch), \
                              (flag))) & Q_BIT(flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PRF_FLAGS(ch), \
                              (flag))) & Q_BIT(flag))


/* room utils ************************************************************/


#define SECT(room)	(world[(room)].sector_type)

#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))

#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : NULL)
#define VALID_RNUM(rnum)  ((rnum) >= 0 && (rnum) <= top_of_world)
#define GET_ROOM_VNUM(rnum) \
          ((VALID_RNUM(rnum) ? world[(rnum)].number : NOWHERE))

/* char utils ************************************************************/


#define IN_ROOM(ch)	((ch)->in_room)
#define GET_WAS_IN(ch)	((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch).year)

#define GET_NAME(ch)    (IS_NPC(ch) ? \
			 (ch)->player.short_descr : (ch)->player.name)
#define SET_NAME(ch, i)	(IS_NPC(ch) ? \
			 ((ch)->player.short_descr = i) : \
			 ((ch)->player.name = i))
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_PASSWD(ch)	((ch)->player.passwd)
#define GET_PFILEPOS(ch)((ch)->pfilepos)

#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_CLASS(ch)   ((ch)->player.class)
#define GET_HOME(ch)	((ch)->player.hometown)
#define HOME_KD		1
#define HOME_KO		2
#define HOME_AZ		3

#define GET_HEIGHT(ch)	((ch)->player.height)
#define GET_WEIGHT(ch)	((ch)->player.weight)
#define GET_SEX(ch)	((ch)->player.sex)

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_ADD(ch)     ((ch)->aff_abils.str_add)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)

#define GET_EXP(ch)	  ((ch)->points.exp)
#define GET_AC(ch)        ((ch)->points.armor)
#define GET_HIT(ch)	  ((ch)->points.hit)
#define GET_MAX_HIT(ch)	  ((ch)->points.max_hit)
#define GET_MOVE(ch)	  ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)	  ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_GOLD(ch)	  ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)	  ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)

#define GET_POS(ch)	  ((ch)->char_specials.position)
#define GET_IDNUM(ch)	  ((ch)->char_specials.saved.idnum)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)	  ((ch)->char_specials.fighting)
#define GET_SAVE(ch, i)	  ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)

#define GET_COND(ch, i)		((ch)->player_specials->saved.conditions[(i)])
#define GET_LOADROOM(ch)	((ch)->player_specials->saved.load_room)
#define GET_PRACTICES(ch)	((ch)->player_specials->saved.spells_to_learn)
#define GET_INVIS_LEV(ch)	((ch)->player_specials->saved.invis_level)
#define GET_WIMP_LEV(ch)	((ch)->player_specials->saved.wimp_level)
#define GET_FREEZE_LEV(ch)	((ch)->player_specials->saved.freeze_level)
#define GET_BAD_PWS(ch)		((ch)->player_specials->saved.bad_pws)
#define GET_TALK(ch, i)		((ch)->player_specials->saved.talks[i])
#define POOFIN(ch)		((ch)->player_specials->poofin)
#define POOFOUT(ch)		((ch)->player_specials->poofout)
#define GET_LAST_OLC_TARG(ch)	((ch)->player_specials->last_olc_targ)
#define GET_LAST_OLC_MODE(ch)	((ch)->player_specials->last_olc_mode)
#define GET_ALIASES(ch)		((ch)->player_specials->aliases)
#define GET_LAST_TELL(ch)	((ch)->player_specials->last_tell)

#define GET_SKILL(ch, i)	((ch)->player_specials->saved.skills[i])
#define SET_SKILL(ch, i, pct)	((ch)->player_specials->saved.skills[i] = pct)

     /* Serapis macros */
#define GET_RACE(ch)           (IS_MOB(ch) ? (ch)->mob_specials.race : \
				(ch)->player_specials->saved.race)
/* these guys are here now because gcc4 is really particular about expressions as lvalues so GET_RACE doesn't work */
#define GET_PLR_RACE(ch)	((ch)->player_specials->saved.race)
#define GET_MOB_RACE(ch)	((ch)->mob_specials.race)

#define SET_RACE(ch, i)	       (IS_MOB(ch) ? ((ch)->mob_specials.race = i) : \
				((ch)->player_specials->saved.race = i))
#define GET_KILLS(ch)          ((ch)->player_specials->saved.killcount)
#define GET_PKS(ch)            ((ch)->player_specials->saved.pkcount)
#define GET_DEATHS(ch)         ((ch)->player_specials->saved.deathcount)
#define GET_LAST_DEATH(ch)     ((ch)->player_specials->saved.lastdeath)
#define GET_MOUNT_RENT_TIME(ch) ((ch)->player_specials->saved.mount_rent)
#define GET_MOUNT_NUM(ch)      ((ch)->player_specials->saved.mount_vnum)
#define GET_MOUNT_COST_DAY(ch) ((ch)->player_specials->saved.mount_cost_day)
#define IS_MOUNTED(ch)         (IS_AFFECTED(ch, AFF_MOUNT))
#define IS_MOUNTABLE(ch)       ( IS_MOB(ch) && !IS_MOUNTED(ch) && \
				 (IS_SET_AR(MOB_FLAGS(ch),MOB_MOUNTABLE)) )
#define CAN_MOUNT(ch)          (!IS_MOB(ch) && !IS_MOUNTED(ch))
#define IS_PSIONIC(ch)	       (!IS_NPC(ch) && (GET_CLASS(ch) == CLASS_PSIONIC))
#define GET_RACE_HATE(ch, i)   ((ch)->char_specials.race_hate[i])
#define IS_NINJA(ch)	       (!IS_NPC(ch) && (GET_CLASS(ch) == CLASS_NINJA))
#define IS_PARRIED(ch)         ((ch)->char_specials.parried)
#define GET_JAIL_TIMER(ch)     ((ch)->char_specials.jailtimer)
#define GET_TATTOO(ch)         ((ch)->player_specials->saved.tattoo)
#define TAT_TIMER(ch)	       ((ch)->player_specials->saved.tattimer)
#define IS_REMORT_ONLY_CLASS(ch) (GET_CLASS(ch)==CLASS_MAGUS || \
				  GET_CLASS(ch)==CLASS_AVATAR || \
				  GET_CLASS(ch)==CLASS_ASSASSIN || \
				  GET_CLASS(ch)==CLASS_PALADIN || \
                                  GET_CLASS(ch)==CLASS_RANGER || \
                                  GET_CLASS(ch)==CLASS_MYSTIC)
#define stc(line, ch)          (send_to_char(line, ch))
#define IS_NIGHTBREED(ch)      (IS_AFFECTED(ch, AFF_VAMPIRE) || \
				IS_AFFECTED(ch, AFF_WEREWOLF))
#define GET_NOISE(ch)	       ((ch)->mob_specials.noise)

#define GET_MOB_SCRIPT(mob)        ((mob_index[GET_MOB_RNUM((mob))].script))
#define GET_OBJ_SCRIPT(obj)        (obj_index[GET_OBJ_RNUM((obj))].script)
#define GET_ROOM_SCRIPT(rnum)      (world[(rnum)].script)

#define MOB_SCRIPT_FLAGS(mob)      (mob_index[GET_MOB_RNUM((mob))].script->lua_functions)
#define MOB_SCRIPT_FLAGGED(mob, flag) (IS_NPC(mob) && IS_SET(MOB_SCRIPT_FLAGS(mob), (flag)))

#define OBJ_SCRIPT_FLAGS(obj)      (obj_index[GET_OBJ_RNUM((obj))].script->lua_functions)
#define OBJ_SCRIPT_FLAGGED(obj, flag) (IS_SET(OBJ_SCRIPT_FLAGS(obj), (flag)))

#define ROOM_SCRIPT_FLAGS(rnum)    (world[(rnum)].script->lua_functions)
#define ROOM_SCRIPT_FLAGGED(rnum, flag) (IS_SET(ROOM_SCRIPT_FLAGS(rnum), (flag)))

#define IS_COVERED(ch, whe) ( (whe==WEAR_BODY && GET_EQ(ch, WEAR_ABOUT)) ||\
                 	      (whe==WEAR_FINGER_L && GET_EQ(ch, WEAR_HANDS)) ||\
                 	      (whe==WEAR_FINGER_R && GET_EQ(ch, WEAR_HANDS)) ||\
                 	      (whe==WEAR_LEGS && GET_EQ(ch, WEAR_ABLEGS)) )
#define GET_ORIG_CON(ch)	((ch)->player_specials->saved.orig_con)
#define ROOM_FLOWS(room)	(ROOM_FLAGGED(room, ROOM_FLOW_NORTH) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_SOUTH) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_EAST) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_WEST) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_UP) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_DOWN))
#define IS_PALADIN(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_PALADIN))
#define IS_MAGUS(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_MAGUS))
#define IS_ASSASSIN(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_ASSASSIN))
#define IS_AVATAR(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_AVATAR))
#define IS_RANGER(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_RANGER))
#define IS_MYSTIC(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_MYSTIC))
#define IS_SHADOWING(ch)        (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_DODGE))
#define GET_ACTION(ch)          ((ch)->action)
#define IS_CHOSEN(ch)		(!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_CHOSEN))
#define IS_OUTLAW(ch)		(!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_OUTLAW))

     /* Serapis macros end */

#define GET_EQ(ch, i)		((ch)->equipment[i])

#define GET_MOB_SPEC(ch) (IS_MOB(ch) ? (mob_index[(ch->nr)].func) : NULL)
#define GET_MOB_RNUM(mob)	((mob)->nr)
#define GET_MOB_VNUM(mob)	(IS_MOB(mob) ? \
				 mob_index[GET_MOB_RNUM(mob)].virtual : -1)

#define GET_MOB_WAIT(ch)	((ch)->mob_specials.wait_state)
#define GET_DEFAULT_POS(ch)	((ch)->mob_specials.default_pos)
#define MEMORY(ch)		((ch)->mob_specials.memory)
#define GET_NDD(mob) 		((mob)->mob_specials.damnodice)
#define GET_SDD(mob) 		((mob)->mob_specials.damsizedice)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (5 + (GET_DEX(ch) >> 1) + (GET_LEVEL(ch) >> 1))
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || PRF_FLAGGED(ch, PRF_HOLYLIGHT))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))


/* descriptor-based utils ************************************************/


#define WAIT_STATE(ch, cycle) { \
	if ((ch)) (ch)->wait = (cycle); \
	else if (IS_NPC(ch)) GET_MOB_WAIT(ch) = (cycle); }

#define GET_PC_WAIT(ch) { \
        if ((ch)) (ch)->wait; }
#define CHECK_WAIT(ch)	(((ch)) ? ((ch)->wait > 1) : 0)
#define STATE(d)	((d)->connected)


/* object utils **********************************************************/


#define GET_OBJ_TYPE(obj)	((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)	((obj)->obj_flags.cost)
#define GET_OBJ_LOAD(obj)	((obj)->obj_flags.load)
#define SET_OBJ_LOAD(obj, val)  ((obj)->obj_flags.load = (val))
#define GET_OBJ_EXTRA(obj)	((obj)->obj_flags.extra_flags)
#define GET_OBJ_WEAR(obj)	((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)	((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)	((obj)->obj_flags.timer)
#define GET_OBJ_RNUM(obj)	((obj)->item_number)
#define GET_OBJ_VNUM(obj)	(GET_OBJ_RNUM(obj) >= 0 ? \
				 obj_index[GET_OBJ_RNUM(obj)].virtual : -1)
#define IS_OBJ_STAT(obj,stat)   (IS_SET_AR((obj)->obj_flags.extra_flags, \
                                 (stat)))

#define IS_CORPSE(obj)		(GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
				 GET_OBJ_VAL((obj), 3) == 1)

#define IS_FOOD(obj)		(GET_OBJ_TYPE(obj) == ITEM_FOOD)

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
	(obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET_AR((obj)->obj_flags.wear_flags, (part)))

#define GET_OBJ_EXTRA_AR(obj, i)   ((obj)->obj_flags.extra_flags[(i)])

/* compound utilities and other macros **********************************/


#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!IS_AFFECTED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || IS_AFFECTED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!IS_AFFECTED((obj),AFF_INVISIBLE) || IS_AFFECTED(sub,AFF_DETECT_INVIS)))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || PRF_FLAGGED(sub, PRF_HOLYLIGHT))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= GET_INVIS_LEV(obj)) && IMM_CAN_SEE(sub, obj)))

/* End of CAN_SEE */


#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || IS_AFFECTED((sub), AFF_DETECT_INVIS))

#define MORT_CAN_SEE_OBJ(sub, obj) (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || PRF_FLAGGED((sub), PRF_HOLYLIGHT))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))


#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define SPELL_ROUTINES(spl)     (spell_info[spl].routines)
#define HAS_SPELL_ROUTINE(spl, flag) (IS_SET(SPELL_ROUTINES(spl), (flag)))

#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))


#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[(int)GET_CLASS(ch)])

#define IS_MAGIC_USER(ch)	(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_MAGIC_USER))
#define IS_CLERIC(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_CLERIC))
#define IS_THIEF(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_THIEF))
#define IS_WARRIOR(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_WARRIOR))

#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS) || \
                     SECT(ch->in_room) != SECT_INSIDE)

#define IS_IMMORT(ch)		(!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMMORT))


/**** INFOBAR DEFS *****/
#define GET_INFOBAR(ch)    ((ch)->player_specials->saved.infobar)
#define GET_SCREENSIZE(ch) ((ch)->player_specials->saved.size)

#define GET_LASTMANA(ch)    ((ch)->last.mana)
#define GET_LASTMAXMANA(ch) ((ch)->last.maxmana)
#define GET_LASTHIT(ch)     ((ch)->last.hit)
#define GET_LASTMAXHIT(ch)  ((ch)->last.maxhit)
#define GET_LASTMOVE(ch)    ((ch)->last.move)
#define GET_LASTMAXMOVE(ch) ((ch)->last.maxmove)
#define GET_LASTEXP(ch)     ((ch)->last.exp)
#define GET_LASTGOLD(ch)    ((ch)->last.gold)

/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

#if defined(NOCRYPT) || !defined(HAVE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

#endif  /* _UTILS_H */

