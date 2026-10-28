/* ************************************************************************
*  File: events.h                                                         *
*                                                                         *
*  Usage: structures and prototypes for events                            *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Changed return type of EVENTFUNC from void to long.   *
*                   Moved struct event definition to events.c.            *
************************************************************************ */

struct script_event {
  struct char_data *me;             /* The mob/player used in the script */
  struct obj_data *obj;             /* The obj used in the script        */
  struct room_data *room;           /* The room used in the script       */
  char *fname;                      /* The script name to execute        */
  char *type;                       /* The type of script - mob/obj/room */
};

struct event_data {
   void (*command);                 /* the command to complete       */
   struct char_data *ch;            /* char this event happens to    */
   int cmd;                         /* command number                */   
   int subcmd;                      /* associated subcmd             */
   char *args;                      /* typed in args                 */
   int count;                       /* pulses to event completion    */
   struct script_event *script;     /* script event data if req'd    */
   struct char_data *fighting;      /* char that I'm fighting        */

   struct event_data *next;         /* next command event in list */
};

/* Function protos */
void event_activity(void);
void add_event(struct event_data *event);
void remove_event(struct event_data *event);

