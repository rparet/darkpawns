/* ==========================================================================
   FILE   : tattoo.c - tattoo code
   HISTORY: dlkarnes 970417 started for Dark Pawns
   ========================================================================= */

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

/* =========================================================================
   NAME       : tattoo_af()
   DESCRIPTION: add or remove the affects of a tattoo
   RETURNS    : n/a
   WARNINGS   :
   HISTORY    : Created by dlkarnes 970418
   OTHER      :
   ========================================================================= */
#define MAX_TAT_AFFECTS 3 /*change if necessary */
void tattoo_af( struct char_data *ch, bool add )
{
  struct affected_type af[MAX_TAT_AFFECTS];
  int i = 0;

  extern sbyte dye_affects[][NUM_DYES * 2];

  if (!ch || !GET_TATTOO(ch))
    return;

  for (i = 0; i < MAX_TAT_AFFECTS; i++) {
    af[i].type = 0;
    af[i].bitvector = AFF_NOTHING;
    af[i].modifier = 0;
    af[i].location = APPLY_NONE;
  }

  /* Let's deal with the dye colors first */
  for (i = 0; i < NUM_DYES; i++)
    if (IS_SET(GET_DYE(ch) , 1 << i)) {
      af[0].location = dye_affects[GET_TATTOO(ch) - 1][i * 2];
      af[0].modifier = dye_affects[GET_TATTOO(ch) - 1][(i * 2) + 1];
      af[0].bitvector = AFF_NOTHING;
      affect_modify(ch, af[0].location, af[0].modifier, af[0].bitvector, add);
    }

  /* Now we'll add the main tattoo affect */
  switch (GET_TATTOO(ch)) {
    case TATTOO_TRIBAL:
      af[0].location = APPLY_MOVE;
      af[0].modifier = 10;
      af[1].location = APPLY_DEX;
      af[1].modifier = 1;
      break;
    case TATTOO_SCORPION:
      af[0].location = APPLY_AC;
      af[0].modifier = -10;
      af[1].location = APPLY_MOVE;
      af[1].modifier = 20;
      break;
    case TATTOO_MOON:
      af[0].location = APPLY_SPELL;
      af[0].bitvector = AFF_DETECT_INVIS;
      af[1].location = APPLY_SPELL;
      af[1].bitvector = AFF_SENSE_LIFE;
      af[2].location = APPLY_SPELL;
      af[2].bitvector = AFF_INFRAVISION;
      break;
    case TATTOO_SUN:
      af[0].location = APPLY_SPELL;
      af[0].bitvector = AFF_INFRAVISION;
      af[1].location = APPLY_MANA;
      af[1].modifier = 40;
      af[2].location = APPLY_HIT;
      af[2].modifier = 40;
      break;
    case TATTOO_DRAGON:
      af[0].location = APPLY_DAMROLL;
      af[0].modifier = 2;
      af[1].location = APPLY_STR;
      af[1].modifier = 2;
      break;
    case TATTOO_SKULL:
      af[0].location = APPLY_WIS;
      af[0].modifier = 1;
      af[1].location = APPLY_INT;
      af[1].modifier = 1;
      break;
    case TATTOO_ROSE:
      af[0].location = APPLY_CHA;
      af[0].modifier = 1;
      af[1].location = APPLY_SAVING_BREATH;
      af[1].modifier = 5;
      break;
    default: 
      break;
  }
  
  for (i = 0; i < MAX_TAT_AFFECTS; i++)
    if (af[i].location != APPLY_NONE)
      affect_modify(ch, af[i].location, af[i].modifier, af[i].bitvector, add);
}
