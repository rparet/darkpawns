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

/* $Id: utils.h 1273 2005-12-20 05:19:42Z mcooper $ */

#ifndef _UTILS_H
#define _UTILS_H

/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern int has_boat(struct char_data *ch);
extern int get_skill(struct char_data *ch, int skill);
int calculate_speed(struct char_data *ch);

extern FILE * logfile;

#define log basic_mud_log

/* public functions in utils.c */
char	*str_dup(const char *source);
int	str_cmp(char *arg1, char *arg2);
int	strn_cmp(char *arg1, char *arg2, int n);
void    basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
unsigned long circle_random(void);
void	alog (char const *str, ...);
int	touch(char *path);
void	mudlog(char *str, char type, int level, byte file);
void	log_death_trap(struct char_data *ch);
int	number(int from, int to);
int	dice(int num, int size);
void	sprintbit(bitvector_t vektor, char *names[], char *result);
void	sprinttype(int type, char *names[], char *result);
int	get_line(FILE *fl, char *buf);
int	get_filename(char *orig_name, char *filename, int mode);
struct time_info_data age(struct char_data *ch);
int	num_pc_in_room(struct room_data *room);
char   *tprintf(char *fmt, ...);
void    sprintbitarray(int bitvector[], char *names[], int maxar, char *result);
int	sprintascii(char *out, bitvector_t bits);
bitvector_t asciiflag_conv(char *flag);
void    core_dump_real(const char *, int);
void race_hate_sort(struct char_data *ch);
int calculate_skill_value(struct char_data *ch, int skill_num);
int in_zone(int virtual, int zone, ubyte type);
int in_world(int virtual, ubyte type);
bool skill_success(struct char_data *ch, int skill_num, int modifier);
bool check_dead(struct char_data *ch);
void mob_combat_dice(struct char_data *mob);

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

/* in act.other.c */
void vitae_af(struct char_data *ch, bool add);

/* in act.movement.c */
int	do_simple_move(struct char_data *ch, int dir, int following);
int	perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int	point_gain(struct char_data *ch, int update);
void	advance_level(struct char_data *ch);
void	set_title(struct char_data *ch, char *title);
void	gain_exp(struct char_data *ch, int gain);
void	gain_exp_regardless(struct char_data *ch, int gain);
void	gain_condition(struct char_data *ch, int condition, int value);
void	check_idling(struct char_data *ch);
void	point_update(void);
void	item_update(void);
void    hunger_and_thirst(void);
void	update_pos(struct char_data *victim);
bool    are_grouped(struct char_data *ch1, struct char_data *ch2);   


/* in class.c FIX */
long find_skill_exp (int skill_level);

/* in_zone */
#define SCMD_MOB  0
#define SCMD_OBJ  1

/* point_gain */
#define SCMD_HIT  0
#define SCMD_MANA 1
#define SCMD_MOVE 2

/* various constants *****************************************************/


/* defines for mudlog() */
#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3

/* get_filename() */
#define ETEXT_FILE	1

/* breadth-first searching */
#define BFS_ERROR		-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH		-3

/* idle time-out */
#define IDLE_TO_VOID		8
#define IDLE_DISCONNECT		30

/* mud-life time */
#define SECS_PER_TICK           63   /* ~1.1 RL minutes */
#define SECS_PER_MUD_HOUR	270  /* ~4.5 RL minutes */
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)  /* ~108 RL minutes */
#define SECS_PER_MUD_MONTH	(35*SECS_PER_MUD_DAY)   /* ~3,780 RL minutes, ~63 RL hours */
#define SECS_PER_MUD_YEAR	(17*SECS_PER_MUD_MONTH) /* ~64,260 RL minutes, ~1,071 RL hours, ~18 RL days */

#define HOURS_PER_DAY		24
#define DAYS_PER_WEEK		7
#define	DAYS_PER_MONTH		35
#define	MONTHS_PER_YEAR		17

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)

/* Misc. defines (rparet) */

#define LIMIT(var, low, high)	MIN(high, MAX(var, low))    /* Tidier approach to MIN/MAX */

#define IS_FLYING(ch)   (IS_AFFECTED((ch), AFF_FLY) || \
                           PRF_FLAGGED((ch),PRF_NOHASSLE))                 
#define CAN_SWIM(ch)    (IS_AFFECTED((ch), AFF_WATERWALK) || \
                         IS_AFFECTED((ch), AFF_FLY) || \
                         PRF_FLAGGED((ch), PRF_NOHASSLE) || \
                         has_boat((ch)))

#define LIFE_POOR       1
#define LIFE_AVERAGE    2
#define LIFE_RICH       3

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


/* basic bitvector utils ************************************************* */

#define Q_FIELD(x)        ((unsigned long long int) (x) / 64)
#define Q_BIT(x)          (1 << ((x) % 64))

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
#define ZONE_FLAGS(zone) (zone_table[zone].zflags)

#define IS_NPC(ch)  (IS_SET(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && GET_MOB_RNUM(ch) >= 0)

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)))
#define ZONE_FLAGGED(zone, flag) (IS_SET(ZONE_FLAGS(zone), (flag)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))


/* room utils ************************************************************/


#define SECT(room)	(world[(room)].sector_type)

#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))
#define OK_BURY(room)  ((SECT(room) == SECT_FOREST) || (SECT(room) == SECT_FIELD) || \
                        (SECT(room) == SECT_HILLS)  || (SECT(room) == SECT_DESERT)) 

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
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_LEVEL(ch)   ((ch)->player.level)

#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_HOME(ch)	((ch)->player.hometown)

#define GET_WEIGHT(ch)	((ch)->player.weight)
#define GET_SEX(ch)	((ch)->player.sex)

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)

#define GET_REAL_STR(ch) ((ch)->real_abils.str)
#define GET_REAL_DEX(ch) ((ch)->real_abils.dex)
#define GET_REAL_INT(ch) ((ch)->real_abils.intel)
#define	GET_REAL_WIS(ch) ((ch)->real_abils.wis)
#define GET_REAL_CON(ch) ((ch)->real_abils.con)
#define GET_REAL_CHA(ch) ((ch)->real_abils.cha)
#define MAX_ABILITY      25

#define GET_EXP(ch)	  ((ch)->points.exp)
#define GET_EXP_Q(ch)     ((ch)->points.exp_queue)
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
#define GET_DODGEROLL(ch) ((ch)->points.dodgeroll)

#define GET_POS(ch)	  ((ch)->char_specials.position)
#define GET_IDNUM(ch)	  ((ch)->char_specials.saved.idnum)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)	  ((ch)->char_specials.fighting) 
#define GET_SAVE(ch, i)	  ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)
#define GUARDING(ch)      ((ch)->char_specials.guarding)

#define GET_SQUELCH(ch, i)      ((ch)->player_specials->saved.squelch[(i)])
#define GET_COND(ch, i)		((ch)->player_specials->saved.timer.conditions[(i)])
#define GET_LOADROOM(ch)	((ch)->player_specials->saved.load_room)
#define GET_BINDROOM(ch)	((ch)->player_specials->saved.bind_room)
#define GET_INVIS_LEV(ch)	((ch)->player_specials->saved.invis_level)
#define GET_WIMP_LEV(ch)	((ch)->player_specials->saved.wimp_level)
#define GET_VITAE(ch)     	((ch)->player_specials->saved.vitae)
#define GET_VITAE_EXP(ch) 	((ch)->player_specials->saved.vitae_exp)
#define GET_STAT_PTS(ch)        ((ch)->player_specials->saved.stat_points)
#define GET_GROUP_PTS(ch)       ((ch)->player_specials->saved.group_points)
#define GET_FREEZE_LEV(ch)	((ch)->player_specials->saved.freeze_level)
#define POOFIN(ch)		((ch)->player_specials->poofin)
#define POOFOUT(ch)		((ch)->player_specials->poofout)
#define GET_LAST_OLC_TARG(ch)	((ch)->player_specials->last_olc_targ)
#define GET_LAST_OLC_MODE(ch)	((ch)->player_specials->last_olc_mode)
#define GET_ALIASES(ch)		((ch)->player_specials->aliases)
#define GET_LAST_TELL(ch)	((ch)->player_specials->last_tell)
#define GET_LAST_MOVED(ch)	((ch)->player_specials->last_moved)

/* Group macros */
#define GET_SKILL(ch, i)	(get_skill(ch, i))
#define GET_SPELL(ch, i)        ((ch)->skills.known[i])
#define GET_GROUP_LVL(ch, i)    ((ch)->skills.group_level[i])
#define GET_GROUP_SKILL(ch, i)  ((ch)->skills.group_skill[i])
#define SET_SKILL(ch, i, pct)	{ (ch)->skills.group_skill[i] = pct; }
#define GET_SKILL_EXP(ch, i)    find_skill_exp(GET_SKILL((ch), i))

     /* Serapis macros */
#define GET_RACE(ch)           (IS_MOB(ch) ? (ch)->mob_specials.race : \
				(ch)->player_specials->saved.race)
#define GET_RACE_HATE(ch, i)   ((ch)->player_specials->saved.race_hate[i])
#define GET_RACE_KILLS(ch, i)  ((ch)->player_specials->race_kills[i])

#define GET_MOB_RACE(ch)       ((ch)->mob_specials.race)
#define GET_UUID(ch)	       ((ch)->mob_specials.id)
#define GET_KILLS(ch)          ((ch)->player_specials->saved.killcount)
#define GET_PKS(ch)            ((ch)->player_specials->saved.pkcount)
#define GET_DEATHS(ch)         ((ch)->player_specials->saved.deathcount)
#define GET_LAST_DEATH(ch)     ((ch)->player_specials->saved.lastdeath)
#define GET_MOUNT_NUM(ch)      ((ch)->player_specials->saved.mount_vnum)
#define GET_STABLE(ch)         ((ch)->player_specials->saved.stable)

/* Timers */
#define OUTLAW_TIME(ch)        ((ch)->player_specials->saved.timer.outlaw)
#define JAIL_TIME(ch)          ((ch)->player_specials->saved.timer.jail)
#define MOUNT_TIME(ch)         ((ch)->player_specials->saved.timer.mount)
#define IMPROVE_TIME(ch, i)    ((ch)->player_specials->saved.timer.improve[i])

#define IS_MOUNTED(ch)         (IS_AFFECTED(ch, AFF_MOUNT))
#define IS_MOUNTABLE(ch)       ( IS_MOB(ch) && !IS_MOUNTED(ch) && \
				 (IS_SET(MOB_FLAGS(ch),MOB_MOUNTABLE)) )
#define CAN_MOUNT(ch)          (!IS_MOB(ch) && !IS_MOUNTED(ch))
#define IS_PARRIED(ch)         ((ch)->char_specials.parried)
#define GET_TATTOO(ch)         ((ch)->player_specials->saved.tattoo.type)
#define GET_DYE(ch)            ((ch)->player_specials->saved.tattoo.dyes)

#define stc(line, ch)          (send_to_char(line, ch))
#define IS_NIGHTBREED(ch)      (IS_AFFECTED(ch, AFF_VAMPIRE) || \
				IS_AFFECTED(ch, AFF_WEREWOLF))
#define GET_NOISE(ch)	       ((ch)->mob_specials.noise)
#define GET_SOUND(ch)          ((ch)->mob_specials.sound)

#define GET_MOB_SCRIPT(mob)        ((mob_index[GET_MOB_RNUM((mob))].script))
#define GET_OBJ_SCRIPT(obj)        (obj_index[GET_OBJ_RNUM((obj))].script)
#define GET_ROOM_SCRIPT(rnum)      (world[(rnum)].script)

#define MOB_SCRIPT_FLAGS(mob)      (mob_index[GET_MOB_RNUM((mob))].script->lua_functions)
#define MOB_SCRIPT_FLAGGED(mob, flag) (IS_NPC(mob) && IS_SET(MOB_SCRIPT_FLAGS(mob), (flag)))

#define OBJ_SCRIPT_FLAGS(obj)      (obj_index[GET_OBJ_RNUM((obj))].script->lua_functions)
#define OBJ_SCRIPT_FLAGGED(obj, flag) (IS_SET(OBJ_SCRIPT_FLAGS(obj), (flag)))

#define ROOM_SCRIPT_FLAGS(rnum)    (world[(rnum)].script->lua_functions)
#define ROOM_SCRIPT_FLAGGED(rnum, flag)  (IS_SET(ROOM_SCRIPT_FLAGS(rnum), (flag)))

#define GET_LOADED_IN(ch)      ((ch)->mob_specials.loaded_in)
#define IS_COVERED(ch, whe) ( (whe==WEAR_BODY && GET_EQ(ch, WEAR_ABOUT)) ||\
                 	      (whe==WEAR_FINGER_L && GET_EQ(ch, WEAR_HANDS)) ||\
                 	      (whe==WEAR_FINGER_R && GET_EQ(ch, WEAR_HANDS)) ||\
                 	      (whe==WEAR_LEGS && GET_EQ(ch, WEAR_ABLEGS)) )
#define ROOM_FLOWS(room)	(ROOM_FLAGGED(room, ROOM_FLOW_NORTH) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_SOUTH) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_EAST) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_WEST) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_UP) || \
				 ROOM_FLAGGED(room, ROOM_FLOW_DOWN))

#define IS_SHADOWING(ch)        (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_DODGE))
#define IS_CHOSEN(ch)		(!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_CHOSEN))
#define IS_OUTLAW(ch)		(!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_OUTLAW))

     /* Serapis macros end */

#define GET_EQ(ch, i)		((ch)->equipment[i])

#define GET_MOB_SPEC(ch) (IS_MOB(ch) ? (mob_index[(ch->nr)].func) : NULL)
#define GET_MOB_RNUM(mob)	((mob)->nr)
#define GET_MOB_VNUM(mob)	(IS_MOB(mob) ? \
				 mob_index[GET_MOB_RNUM(mob)].virtual : -1)

#define GET_DEFAULT_POS(ch)	((ch)->mob_specials.default_pos)
#define MEMORY(ch)		((ch)->mob_specials.memory)
#define GET_NDD(mob) 		((mob)->mob_specials.damnodice)
#define GET_SDD(mob) 		((mob)->mob_specials.damsizedice)
#define GET_NHPD(mob)           ((mob)->mob_specials.hpnumdice)
#define GET_SHPD(mob)           ((mob)->mob_specials.hpsizedice)
#define GET_HPBONUS(mob)        ((mob)->mob_specials.hpbonus)
#define GET_MOB_TIMER(mob)	((mob)->mob_specials.timer)

#define STRENGTH_APPLY_INDEX(ch) GET_STR(ch)  /* FIX FIX */

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (5 + (sh_int)(GET_DEX(ch) >> 1) + (GET_LEVEL(ch) >> 1))
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || PRF_FLAGGED(ch, PRF_HOLYLIGHT))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))


/* descriptor-based utils ************************************************/


#define WAIT_STATE(ch, cycle) do { \
                              if (IS_NPC(ch)) break; \
                              GET_WAIT_STATE(ch) += (cycle); } while(0)
#define GET_WAIT_STATE(ch)    ((ch)->wait)
#define GET_COMBAT_WAIT(ch)   ((ch)->combat_wait)
#define DELAY(ch)             ((ch)->char_specials.delay)
#define COMBAT_WAIT(ch, cycle)  do { \
                                if (!FIGHTING(ch)) break; \
                                GET_COMBAT_WAIT(ch) += PULSE_VIOLENCE * (cycle); \
                                } while(0)

#define STATE(d)	((d)->connected)

#define IS_PLAYING(d)   (STATE(d) == CON_REDIT || STATE(d) == CON_PLAYING ||    \
                        STATE(d) == CON_MEDIT || STATE(d) == CON_OEDIT ||       \
                        STATE(d) == CON_ZEDIT || STATE(d) == CON_SEDIT)
#define GET_ACCOUNT(d)  ((d)->account->name)
#define GET_PASSWD(d)	((d)->account->pwd)
#define GET_BAD_PWS(d)  ((d)->account->bad_pws)

/* object utils **********************************************************/


#define GET_OBJ_TYPE(obj)	((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)	((obj)->obj_flags.cost)
#define GET_OBJ_LOAD(obj)	((obj)->obj_flags.percentage_load)
#define GET_OBJ_EXTRA(obj)	((obj)->obj_flags.extra_flags)
#define GET_OBJ_BITV(obj)	((obj)->obj_flags.bitvector)
#define GET_OBJ_WEAR(obj)	((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_CRAFT_VAL(obj)	((obj)->obj_flags.value[4])
#define GET_OBJ_WEIGHT(obj)	((obj)->obj_flags.weight)
#define GET_OBJ_SPEED(obj) 	((obj)->obj_flags.speed)
#define GET_OBJ_LEVEL(obj) 	((obj)->obj_flags.level)
#define GET_OBJ_TIMER(obj)	((obj)->obj_flags.timer)
#define GET_OBJ_RNUM(obj)	((obj)->item_number)
#define GET_OBJ_VNUM(obj)	(GET_OBJ_RNUM(obj) >= 0 ? \
				 obj_index[GET_OBJ_RNUM(obj)].virtual : -1)
#define IS_OBJ_STAT(obj,stat)   (IS_SET((obj)->obj_flags.extra_flags, \
                                 (stat)))
#define IS_CORPSE(obj)		(GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
				 GET_OBJ_VAL((obj), 3) == 1)
#define IS_FOOD(obj)		(GET_OBJ_TYPE(obj) == ITEM_FOOD)

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
	(obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))

#define GET_OBJ_EXTRA_AR(obj, i)   ((obj)->obj_flags.extra_flags[(i)])

#define SPELL_ROUTINES(spl)	(spell_info[spl].routines)
#define HAS_SPELL_ROUTINE(spl, flag) (IS_SET(SPELL_ROUTINES(spl), (flag)))
#define OBJ_FLAGGED(obj, flag) (IS_SET(GET_OBJ_EXTRA(obj), (flag)))

/* compound utilities and other macros **********************************/


#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define PLURAL(name) (strchr("sS", *(name + strlen(name) - 1)) ? TRUE : FALSE)


/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!IS_AFFECTED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || IS_AFFECTED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!IS_AFFECTED((obj),AFF_INVISIBLE) || IS_AFFECTED(sub,AFF_DETECT_INVIS)))

#define OBJ_LIT(sub, obj) (!IS_AFFECTED(sub, AFF_BLIND) && (IS_OBJ_STAT(obj, ITEM_GLOW) || \
                          GET_OBJ_TYPE(obj) == ITEM_LIGHT))

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

#define MORT_CAN_SEE_OBJ(sub, obj) ((LIGHT_OK(sub) || OBJ_LIT(sub, obj)) && \
                                    INVIS_OK_OBJ(sub, obj) && (!IS_OBJ_STAT(obj, ITEM_BURIED)))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || PRF_FLAGGED((sub), PRF_HOLYLIGHT))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))

#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : \
                         (GET_SEX(ch)==SEX_NEUTRAL ? "something" : "someone"))

/* used for returning the alias list for NPCs instead of their short_descr */
#define PERS_NPC(ch, vict)   (CAN_SEE(vict, ch) ? (ch)->player.name : \
                             (GET_SEX(ch)==SEX_NEUTRAL ? "something" : "someone"))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) || \
                         GET_RACE(ch) == RACE_GHOST))


#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))

#define IS_IMMORT(ch)		(!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMMORT))

/* combat state stuff */
#define COMBAT_STATE(ch)       ((ch)->char_specials.combat_state)
#define CS_CIRCLE	(1 << 0)
#define CS_JUMP		(1 << 1)
#define CS_DUCK		(1 << 2)
#define CS_BLOCK	(1 << 3)


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

#define	IS_MXP(ch)	(PLR_FLAGGED((ch), PLR_MXP))
#define IS_MSP(ch)  (PLR_FLAGGED((ch), PLR_MSP))
#define IS_MCCP(ch) (IS_SET((ch)->desc->bits, DESC_MCCP))

#define MIN_AC	1
#define MAX_AC	200

#endif  /* _UTILS_H */

