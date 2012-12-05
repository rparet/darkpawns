/* ==========================================================================
   FILE   : tattoo.c - tattoo code
   HISTORY: dlkarnes 970417 started for Dark Pawns
   ========================================================================= */

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

/* extern functions */

/* =========================================================================
   NAME       : use_tattoo()
   DESCRIPTION: 
   RETURNS    : TRUE if a tattoo was used
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970417
   OTHER      :
   ========================================================================= */
int
use_tattoo( struct char_data *ch )
{
  void add_follower_quiet(struct char_data *ch, struct char_data *leader);

  if (ch && !IS_NPC(ch))
  {
    if (TAT_TIMER(ch))
    {
      char mybuf[256];
      sprintf(mybuf, "You can't use your tattoo's magick for "
              "%d more hour%s.\r\n", TAT_TIMER(ch),TAT_TIMER(ch)>1?"s":"");
      send_to_char(mybuf, ch);
      return(FALSE);
    }
    switch (GET_TATTOO(ch))
    {
      case TATTOO_NONE:
	send_to_char ("You don't have a tattoo.\r\n", ch);
	break;
      case TATTOO_SKULL:
      {
	struct char_data *skull = read_mobile(9, VIRTUAL);
        struct affected_type af;

	char_to_room(skull, ch->in_room);
	add_follower_quiet(skull, ch);
        IS_CARRYING_W(skull) = 0;
        IS_CARRYING_N(skull) = 0;
 
        af.type = SPELL_CHARM;
        af.duration = 20;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(skull, &af);

	act("$n's tattoo glows brightly for a second, and $N appears!",
		TRUE, ch, 0, skull, TO_ROOM);
	act("Your tattoo glows brightly for a second, and $N appears!",
		TRUE, ch, 0, skull, TO_CHAR);
      }
      break;
      case TATTOO_EYE:
        call_magic(ch, ch, NULL, SPELL_GREATPERCEPT, 
		   DEFAULT_WAND_LVL, CAST_WAND);
	break;
      case TATTOO_SHIP:
        call_magic(ch, ch, NULL, SPELL_CHANGE_DENSITY, 
		   DEFAULT_WAND_LVL, CAST_WAND);
	break;
      case TATTOO_ANGEL:
        call_magic(ch, ch, NULL, SPELL_BLESS, DEFAULT_WAND_LVL, CAST_WAND);
	break;
      default:  
	send_to_char("Your tattoo can't be 'use'd.\r\n", ch);
	return (FALSE);
    }
    TAT_TIMER(ch)=24;
  }

  return(FALSE); 
}

/* =========================================================================
   NAME       : tattoo_af()
   DESCRIPTION: add or remove the affects of a tattoo
   RETURNS    : n/a
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970418
   OTHER      :
   ========================================================================= */
#define MAX_TAT_AFFECTS 3 /*change if necessary */
void
tattoo_af( struct char_data *ch, bool add )
{
  struct affected_type af[MAX_TAT_AFFECTS];
  int i = 0;

  if (!ch || !GET_TATTOO(ch))
    return;

  for (i = 0; i < MAX_TAT_AFFECTS; i++)
    {
      af[i].type = 0;
      af[i].bitvector = AFF_NOTHING;
      af[i].modifier = 0;
      af[i].location = APPLY_NONE;
    }

  switch (GET_TATTOO(ch))
    {
    case TATTOO_DRAGON:
      af[0].location = APPLY_DAMROLL;
      af[0].modifier = 2;
      af[1].location = APPLY_STR;
      af[1].modifier = 2;
      break;
    case TATTOO_TIGER:
      af[0].location = APPLY_DEX;
      af[0].modifier = 1;
      af[1].location = APPLY_MOVE;
      af[1].modifier = 10;
      break;
    case TATTOO_TRIBAL:
      af[0].location = APPLY_DEX;
      af[0].modifier = 1;
      break;
    case TATTOO_WORM:
      af[0].location = APPLY_DAMROLL;
      af[0].modifier = 2;
      break;
    case TATTOO_SWORDS:
      af[0].location = APPLY_DAMROLL;
      af[0].modifier = 1;
      af[1].location = APPLY_HITROLL;
      af[1].modifier = 1;
      break;
    case TATTOO_EAGLE:
      af[0].location = APPLY_MOVE;
      af[0].modifier = 20;
      break;
    case TATTOO_HEART:
      af[0].location = APPLY_HIT;
      af[0].modifier = 20;
      break;
    case TATTOO_STAR:
      af[0].location = APPLY_MANA;
      af[0].modifier = 20;
      break;
    case TATTOO_SPIDER:
      af[0].location = APPLY_DEX;
      af[0].modifier = 3;
      break;
    case TATTOO_JYHAD:
      af[0].location = APPLY_DAMROLL;
      af[0].modifier = 1;
      break;
    case TATTOO_MOM:
      af[0].location = APPLY_WIS;
      af[0].modifier = 3;
      break;
    case TATTOO_FOX:
      af[0].location = APPLY_INT;
      af[0].modifier = 1;
      break;
    case TATTOO_OWL:
      af[0].location = APPLY_WIS;
      af[0].modifier = 1;
      break;
    default: 
      break;
    }
  for (i = 0; i < MAX_TAT_AFFECTS; i++)
    if (af[i].location != APPLY_NONE)
      affect_modify(ch, af[i].location, af[i].modifier, af[i].bitvector, add);
}
