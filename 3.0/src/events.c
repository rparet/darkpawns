/* ************************************************************************
*  File: events.c                                                         *
*                                                                         *
*  Usage: Contains routines to handle events                              *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "db.h"
#include "events.h"

/* external functions */
int run_script(struct char_data *ch, struct char_data *me, struct obj_data *obj,
               struct room_data *room, char *argument, char *fname, char *type);

/* external variables */
extern unsigned long pulse;
extern struct event_data *pending_events;
extern int num_events;

void add_event(struct event_data *event)
{
  if (!event) {
    sprintf(buf, "SYSERR: NULL event specified in add_event.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  if (pending_events)
    event->next = pending_events;

  pending_events = event;
  num_events++;
}

void remove_event(struct event_data *event)
{
  struct event_data *temp;

  if (!event) {
    sprintf(buf, "SYSERR: NULL event specified in remove_event.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  if (event->script)
    FREE(event->script);

  REMOVE_FROM_LIST(event, pending_events, next);
  FREE(event->args);
  FREE(event);
  num_events--;
}


void event_activity(void)
{
  int i, j;
  struct event_data *event;
  void (*command) (struct char_data *ch, char * argument, int cmd, int subcmd);
  extern struct combat_skill_table combat_table[];
  ACMD(do_cast);

  if (pending_events && !num_events) {
    sprintf(buf, "SYSERR: pending_events to be processed with no events listed.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    pending_events = NULL;
  }

  for (event = pending_events, i = 0; i < num_events; i++) {
    if (!event->count--) {
      command = event->command;
      if (!check_dead(event->ch) && event->ch) {
        /* Check for a script event first - handled differently */
        if (event->script) {
          run_script(event->ch, event->script->me, event->script->obj, event->script->room,
            event->args, event->script->fname, event->script->type);
        } else {
          if (event->fighting) {
            if (FIGHTING(event->ch) == event->fighting)
              ((*command) (event->ch, event->args, event->cmd, event->subcmd));
          } else
              ((*command) (event->ch, event->args, event->cmd, event->subcmd));
          j = 0;

          /* Check if mob/player died as a result of the event */
          if (!check_dead(event->ch)) {
            while (combat_table[j++].skill)
              if (!strcmp(combat_table[j].cmdname, cmd_info[event->cmd].command)) {
                DELAY(event->ch) -= combat_table[j].delay * PULSE_VIOLENCE;
                DELAY(event->ch) -= calculate_speed(event->ch);
                DELAY(event->ch) = MAX(DELAY(event->ch), 0);
                break;
              }

            /* Was it a spell instead? */
            if (command == do_cast) {
              DELAY(event->ch) -= calculate_speed(event->ch);
              DELAY(event->ch) = MAX(DELAY(event->ch), 0);
            }
          }
        }
      }
      remove_event(event);
    }
    event = event->next;
  }
}
