/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - olc.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _RV_OLC_

#include <math.h>
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "interpreter.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "olc.h"
#include "screen.h"

/*. External data structures .*/
extern int top_of_treasure;
extern struct tlist_data *treasure_list;

/*. External functions .*/
extern void zedit_setup(struct descriptor_data *d, long room_num);
extern void zedit_save_to_disk(struct descriptor_data *d);
extern void zedit_new_zone(struct char_data *ch, int new_zone);
extern void medit_setup_new(struct descriptor_data *d);
extern void medit_setup_existing(struct descriptor_data *d, int rmob_num);
extern void medit_save_to_disk(struct descriptor_data *d);
extern void redit_setup_new(struct descriptor_data *d);
extern void redit_setup_existing(struct descriptor_data *d, long rroom_num);
extern void redit_save_to_disk(struct descriptor_data *d);
extern void oedit_setup_new(struct descriptor_data *d);
extern void oedit_setup_existing(struct descriptor_data *d, int robj_num);
extern void oedit_save_to_disk(struct descriptor_data *d);
extern void sedit_setup_new(struct descriptor_data *d);
extern void sedit_setup_existing(struct descriptor_data *d, int robj_num);
extern void sedit_save_to_disk(struct descriptor_data *d);
extern void tedit_disp_types(struct char_data *ch);
extern void tedit_new_list(struct char_data *ch, char *type);
extern void tedit_setup(struct descriptor_data *d, int tlist_num);
extern void tedit_save_to_disk(struct descriptor_data *d);
extern int real_shop(int vnum);
extern void free_shop(struct shop_data *shop);
extern void free_room(struct room_data *room);

char *str_udup(const char *txt);

/*. Internal function prototypes .*/
int real_zone(int number);
void olc_saveinfo(struct char_data *ch);

/*. Internal data .*/

struct olc_scmd_data {
  char *text;
  int con_type;
};

struct olc_scmd_data olc_scmd_info[6] =
{ {"room", 	CON_REDIT},
  {"object", 	CON_OEDIT},
  {"room",	CON_ZEDIT},
  {"mobile", 	CON_MEDIT},
  {"shop", 	CON_SEDIT},
  {"treasure list",  CON_TEDIT}
};

/*------------------------------------------------------------*\
 Eported ACMD do_olc function

 This function is the OLC interface.  It deals with all the 
 generic OLC stuff, then passes control to the sub-olc sections.
\*------------------------------------------------------------*/

ACMD(do_olc)
{
  int number = -1, save = 0, real_num;
  struct descriptor_data *d;

  if (IS_NPC(ch))
    /*. No screwing arround .*/
    return;

  if (subcmd == SCMD_OLC_SAVEINFO)
  { 
    olc_saveinfo(ch);
    return;
  }

  /*. Parse any arguments .*/
  two_arguments(argument, buf1, buf2);
  if (!*buf1)
  { /* No argument given .*/
    switch(subcmd)
    { 
      case SCMD_OLC_ZEDIT:
      case SCMD_OLC_REDIT:
        number = world[IN_ROOM(ch)].number;
        break;
      case SCMD_OLC_OEDIT:
      case SCMD_OLC_MEDIT:
      case SCMD_OLC_SEDIT:
        sprintf(buf, "Specify a %s VNUM to edit.\r\n", 
		olc_scmd_info[subcmd].text);
        send_to_char (buf, ch);
        return;
      case SCMD_OLC_TEDIT:
        tedit_disp_types(ch);
        return;
    }
  } 
  else if (!isdigit (*buf1))
  {
    if (strncmp("save", buf1, 4) == 0)
    {
      switch(subcmd)
      {
        case SCMD_OLC_TEDIT:
          save = 1;
          break;
        default:
          if (!*buf2) { 
 	    send_to_char("Save which zone?\r\n", ch);
            return;
          } else { 
 	    save = 1;
            number = atoi(buf2) * 100;
          }
          break;
      }
    }
    else if (subcmd == SCMD_OLC_ZEDIT && GET_LEVEL(ch) >= LVL_HIGOD)
    { 
      if ((strncmp("new", buf1, 3) == 0) && *buf2) {
        CREATE(ch->desc->olc, struct olc_data, 1);
        zedit_new_zone(ch, atoi(buf2));
      } else
        send_to_char("Specify a new zone number.\r\n", ch);
      return;
    }
    else if (subcmd == SCMD_OLC_TEDIT && GET_LEVEL(ch) >= LVL_GRGOD) {
      if ((strncmp("new", buf1, 3) == 0) && *buf2) {
        half_chop(argument, buf1, buf2);
        tedit_new_list(ch, buf2);
      } else
        send_to_char("Specify a new treasure list.\r\n", ch);
      return;
    }
    else
    { 
      send_to_char ("Yikes!  Stop that, someone will get hurt!\r\n", ch);
      return;
    }
  }

  /*. If a numeric argument was given, get it .*/
  if (number == -1)
    number = atoi(buf1);

  /*. Check whatever it is isn't already being edited .*/
  for (d = descriptor_list; d; d = d->next)
    if (d->connected == olc_scmd_info[subcmd].con_type)
      if (d->olc && OLC_NUM(d) == number)
      { 
	sprintf(buf, "That %s is currently being edited by %s.\r\n",
                olc_scmd_info[subcmd].text, GET_NAME(d->character));
        send_to_char(buf, ch);
        return;
      }

  d = ch->desc; 

  /*. Give descriptor an OLC struct .*/
  CREATE(d->olc, struct olc_data, 1);

  /*. Find the zone .*/
  switch(subcmd) {
    case SCMD_OLC_TEDIT:	/* We don't want to do anything here */
      break;
    default:
      OLC_ZNUM(d) = real_zone(number);
      if (OLC_ZNUM(d) == -1)
      { 
        send_to_char ("Sorry, there is no zone for that number!\r\n", ch); 
        FREE(d->olc);
        return;
      }

      /*   If you can set your olc, you can edit any zone. -Ser 970614 */
      if ((GET_LEVEL(ch) < LVL_SET_BUILD) && 
          (zone_table[OLC_ZNUM(d)].number != GET_OLC_ZONE(ch)))
      { 
        send_to_char("You do not have permission to edit this zone.\r\n", ch); 
        FREE(d->olc);
        return;
      }
  }

  if (!ZONE_FLAGGED(OLC_ZNUM(d), ZONE_UNLOAD))
    reload_room_descs(OLC_ZNUM(d));

  if(save)
  { switch(subcmd)
    { case SCMD_OLC_REDIT: 
        send_to_char("Saving all rooms in zone.\r\n", ch);
        sprintf(buf, "OLC: %s saves rooms for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        mudlog(buf, CMP, LVL_BUILDER, TRUE);
        redit_save_to_disk(d); 
        break;
      case SCMD_OLC_ZEDIT:
        send_to_char("Saving all zone information.\r\n", ch);
        sprintf(buf, "OLC: %s saves zone info for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        mudlog(buf, CMP, LVL_BUILDER, TRUE);
        zedit_save_to_disk(d); 
        break;
      case SCMD_OLC_OEDIT:
        send_to_char("Saving all objects in zone.\r\n", ch);
        sprintf(buf, "OLC: %s saves objects for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        mudlog(buf, CMP, LVL_BUILDER, TRUE);
        oedit_save_to_disk(d); 
        break;
      case SCMD_OLC_MEDIT:
        send_to_char("Saving all mobiles in zone.\r\n", ch);
        sprintf(buf, "OLC: %s saves mobs for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        mudlog(buf, CMP, LVL_BUILDER, TRUE);
        medit_save_to_disk(d); 
        break;
      case SCMD_OLC_SEDIT:
        send_to_char("Saving all shops in zone.\r\n", ch);
        sprintf(buf, "OLC: %s saves shops for zone %d",
		 GET_NAME(ch), zone_table[OLC_ZNUM(d)].number);
        mudlog(buf, CMP, LVL_BUILDER, TRUE);
        sedit_save_to_disk(d); 
        break;
      case SCMD_OLC_TEDIT:
        send_to_char("Saving all of the treasure lists.\r\n", ch);
        sprintf(buf, "OLC: %s saves the treasure listing.", GET_NAME(ch));
        mudlog(buf, CMP, LVL_BUILDER, TRUE);
        tedit_save_to_disk(d);
        break;
    }
    FREE(d->olc);
    return;
  }
 
  OLC_NUM(d) = number;

  /*. Steal players descriptor start up subcommands .*/
  switch(subcmd)
  { case SCMD_OLC_REDIT:
      real_num = real_room(number);
      if (real_num >= 0)
        redit_setup_existing(d, real_num);
      else
        redit_setup_new(d);
      STATE(d) = CON_REDIT;
      break;
    case SCMD_OLC_ZEDIT:
      real_num = real_room(number);
      if (real_num < 0)
      {  send_to_char("That room does not exist.\r\n", ch); 
         FREE(d->olc);
         return;
      }
      zedit_setup(d, real_num);
      STATE(d) = CON_ZEDIT;
      break;
    case SCMD_OLC_MEDIT:
      real_num = real_mobile(number);
      if (real_num < 0)
        medit_setup_new(d);
      else
        medit_setup_existing(d, real_num);
      STATE(d) = CON_MEDIT;
      break;
    case SCMD_OLC_OEDIT:
      real_num = real_object(number);
      if (real_num >= 0)
        oedit_setup_existing(d, real_num);
      else
        oedit_setup_new(d);
      STATE(d) = CON_OEDIT;
      break;
    case SCMD_OLC_SEDIT:
      real_num = real_shop(number);
      if (real_num >= 0)
        sedit_setup_existing(d, real_num);
      else
        sedit_setup_new(d);
      STATE(d) = CON_SEDIT;
      break;
    case SCMD_OLC_TEDIT:
      number--;
      if ((number < 0) || (number >= top_of_treasure)) {
        send_to_char("That treasure list does not exist.\r\n", ch);
        FREE(d->olc);
        return;
      }
      tedit_setup(d, number);
      STATE(d) = CON_TEDIT;
      break;
  }
  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT(PLR_FLAGS (ch), PLR_WRITING);
}

ACMD(adjust_mobs)    
{
  extern void medit_save_to_disk(struct descriptor_data *d);
  extern int find_exp(int level);
  int nr, vznum, i, statmod;
    
  CREATE(ch->desc->olc, struct olc_data, 1);

  for (nr = 0; nr <= top_of_mobt; nr++) {
    GET_EXP(mob_proto + nr) = (find_exp(GET_LEVEL(mob_proto + nr)) / 20);
    mob_combat_dice(mob_proto + nr);
    GET_DAMROLL(mob_proto + nr) = (GET_LEVEL(mob_proto + nr) > 10 ? ((GET_LEVEL(mob_proto + nr) + 1) / 1.50 ):
        	                  ((GET_LEVEL(mob_proto + nr) + 1) / 2) );
    GET_NHPD(mob_proto + nr) = GET_LEVEL(mob_proto + nr);
    GET_SHPD(mob_proto + nr) = 8;
    GET_HPBONUS(mob_proto + nr) = (3 * pow(GET_LEVEL(mob_proto + nr), 1.5));
    GET_AC(mob_proto + nr) = pow(GET_LEVEL(mob_proto + nr), 1.15);
    GET_HITROLL(mob_proto + nr) = GET_LEVEL(mob_proto + nr) / 3;
    GET_DODGEROLL(mob_proto + nr) = GET_LEVEL(mob_proto + nr) / 10;

    /* Adjust abilities */
    statmod = pow(GET_LEVEL(mob_proto + nr), .70);
    GET_REAL_STR(mob_proto + nr) = 5 + statmod;
    GET_REAL_INT(mob_proto + nr) = 5 + statmod;
    GET_REAL_WIS(mob_proto + nr) = 5 + statmod;
    GET_REAL_DEX(mob_proto + nr) = 5 + statmod;
    GET_REAL_CON(mob_proto + nr) = 5 + statmod;
    GET_REAL_CHA(mob_proto + nr) = 5 + statmod;

    vznum = mob_index[nr].virtual / 100;          
    olc_add_to_save_list(vznum,OLC_SAVE_MOB); 
  }

  for (i = 0; i <= top_of_zone_table; i++) {
    OLC_ZNUM(ch->desc) = i;
    medit_save_to_disk(ch->desc);
  }

  send_to_char(OK,ch);
  FREE(ch->desc->olc);
}
/*------------------------------------------------------------*\
 Internal utlities 
\*------------------------------------------------------------*/

void olc_saveinfo(struct char_data *ch)
{ struct olc_save_info *entry;
  static char *save_info_msg[5] = { "Rooms", "Objects", "Zone info", "Mobiles", "Shops" };

  if (olc_save_list)
    send_to_char("The following OLC components need saving:-\r\n", ch);
  else
    send_to_char("The database is up to date.\r\n", ch);

  for (entry = olc_save_list; entry; entry = entry->next)
  {
    if (entry->type == OLC_SAVE_TREASURE)
      sprintf(buf, " - Treasure list.\r\n");
    else
      sprintf(buf, " - %s for zone %d.\r\n", save_info_msg[(int)entry->type], entry->zone );
    send_to_char(buf, ch);
  }
}


int real_zone(int number)
{ 
  int counter;
  for (counter = 0; counter <= top_of_zone_table; counter++)
    if ((number >= (zone_table[counter].number * 100)) &&
        (number <= (zone_table[counter].top)))
      return counter;

  return -1;
}

/*------------------------------------------------------------*\
 Exported utlities 
\*------------------------------------------------------------*/

/*. Add an entry to the 'to be saved' list .*/

void olc_add_to_save_list(int zone, byte type)
{ struct olc_save_info *new;

  /*. Return if it's already in the list .*/
  for(new = olc_save_list; new; new = new->next)
    if ((new->zone == zone) && (new->type == type))
      return;

  CREATE(new, struct olc_save_info, 1);
  new->zone = zone;
  new->type = type;
  new->next = olc_save_list;
  olc_save_list = new;
}

/*. Remove an entry from the 'to be saved' list .*/

void olc_remove_from_save_list(int zone, byte type)
{ struct olc_save_info **entry;
  struct olc_save_info *temp;

  for(entry = &olc_save_list; *entry; entry = &(*entry)->next)
    if (((*entry)->zone == zone) && ((*entry)->type == type))
    { temp = *entry;
      *entry = temp->next;
      FREE(temp);
      return;
    }
}

/*. Set the colour string pointers for that which this char will
    see at color level NRM.  Changing the entries here will change 
    the colour scheme throught the OLC.*/

void get_char_cols(struct char_data *ch)
{ nrm = CCNRM(ch, C_NRM);
  grn = CCGRN(ch, C_NRM);
  cyn = CCCYN(ch, C_NRM);
  yel = CCYEL(ch, C_NRM);
}


/*. This procedure removes the '\r\n' from a string so that it may be
    saved to a file.  Use it only on buffers, not on the oringinal
    strings.*/

void strip_string(char *buffer)
{ register char *ptr, *str;

  ptr = buffer;
  str = ptr;

  while((*str = *ptr))
  { str++;
    ptr++;
    if (*ptr == '\r')
      ptr++;
  }
}


/*. This procdure frees up the strings and/or the structures
    attatched to a descriptor, sets all flags back to how they
    should be .*/

void cleanup_olc(struct descriptor_data *d, byte cleanup_type)
{ 
  if (d->olc)
  {
    /*. Check for room .*/
    if(OLC_ROOM(d))
    { /*. free_room performs no sanity checks, must be carefull here .*/
      switch(cleanup_type)
      { case CLEANUP_ALL:
          free_room(OLC_ROOM(d));
          break;
        case CLEANUP_STRUCTS:
          FREE(OLC_ROOM(d));
          break;
        default:
          /*. Caller has screwed up .*/
          break;
      }
    }
  
    /*. Check for object .*/
    if(OLC_OBJ(d))
    { /*. free_obj checks strings arn't part of proto .*/
      free_obj(OLC_OBJ(d));
    }

    /*. Check for mob .*/
    if(OLC_MOB(d))
    { /*. free_char checks strings arn't part of proto .*/
      free_char(OLC_MOB(d));
    }
  
    /*. Check for zone .*/
    if(OLC_ZONE(d))
    { /*. cleanup_type is irrelivent here, free everything .*/
      FREE(OLC_ZONE(d)->name);
      FREE(OLC_ZONE(d)->cmd);
      FREE(OLC_ZONE(d));
    }

    /*. Check for shop .*/
    if(OLC_SHOP(d))
    { /*. free_shop performs no sanity checks, must be carefull here .*/
      switch(cleanup_type)
      { case CLEANUP_ALL:
          free_shop(OLC_SHOP(d));
          break;
        case CLEANUP_STRUCTS:
          FREE(OLC_SHOP(d));
          break;
        default:
          /*. Caller has screwed up .*/
          break;
      }
    }

    /*. Check for treasure list .*/
    if(OLC_TLIST(d))
    {
      FREE(OLC_TLIST(d)->vnum);
      FREE(OLC_TLIST(d));
    }

    /*. Restore desciptor playing status .*/
    if (d->character)
    { REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
      STATE(d)=CON_PLAYING;
      act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
    }
    FREE(d->olc);
  }
}

char *str_udup(const char *txt)
{
  return str_dup((txt && *txt) ? txt : "undefined");
}
