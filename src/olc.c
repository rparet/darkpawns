/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - olc.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _RV_OLC_

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "interpreter.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "olc.h"
#include "screen.h"

/*. External data structures .*/
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern struct room_data *world;
extern int top_of_zone_table;
extern int top_of_mobt; /* Top of mob table */
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;

/*. External functions .*/
extern void zedit_setup(struct descriptor_data *d, int room_num);
extern void zedit_save_to_disk(struct descriptor_data *d);
extern void zedit_new_zone(struct char_data *ch, int new_zone);
extern void medit_setup_new(struct descriptor_data *d);
extern void medit_setup_existing(struct descriptor_data *d, int rmob_num);
extern void medit_save_to_disk(struct descriptor_data *d);
extern void redit_setup_new(struct descriptor_data *d);
extern void redit_setup_existing(struct descriptor_data *d, int rroom_num);
extern void redit_save_to_disk(struct descriptor_data *d);
extern void oedit_setup_new(struct descriptor_data *d);
extern void oedit_setup_existing(struct descriptor_data *d, int robj_num);
extern void oedit_save_to_disk(struct descriptor_data *d);
extern void sedit_setup_new(struct descriptor_data *d);
extern void sedit_setup_existing(struct descriptor_data *d, int robj_num);
extern void sedit_save_to_disk(struct descriptor_data *d);
extern int real_shop(int vnum);
extern void free_shop(struct shop_data *shop);
extern void free_room(struct room_data *room);

char *str_udup(const char *txt);

/*. Internal function prototypes .*/
int real_zone(int number);
void olc_saveinfo(struct char_data *ch);
void olc_saveall(struct char_data *ch);

/*. Internal data .*/

struct olc_scmd_data {
  char *text;
  int con_type;
};

struct olc_scmd_data olc_scmd_info[5] =
{ {"room", 	CON_REDIT},
  {"object", 	CON_OEDIT},
  {"room",	CON_ZEDIT},
  {"mobile", 	CON_MEDIT},
  {"shop", 	CON_SEDIT}
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

  /*. Parse any arguments .*/
  two_arguments(argument, buf1, buf2);
  save = !strncmp(buf1, "save", 4);

  if (subcmd == SCMD_OLC_SAVEINFO)
  {
    if (save)
      olc_saveall(ch);
    else
      olc_saveinfo(ch);
    return;
  }

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
    }
  } 
  else if (!isdigit (*buf1))
  {
    if (save)
    { 
      if (!*buf2)
      { 
 	send_to_char("Save which zone?\r\n", ch);
        return;
      } 
      else 
      { 
        number = atoi(buf2) * 100;
      }
    } 
    else if (subcmd == SCMD_OLC_ZEDIT && GET_LEVEL(ch) >= LVL_HIGOD)
    { 
      if ((strncmp("new", buf1, 3) == 0) && *buf2)
        zedit_new_zone(ch, atoi(buf2));
      else
        send_to_char("Specify a new zone number.\r\n", ch);
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
 
  if (save) {
    switch(subcmd) {
    case SCMD_OLC_REDIT: 
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
  }
  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT_AR(PLR_FLAGS (ch), PLR_WRITING);
}

#define GET_DND(nr) (mob_proto[(nr)].mob_specials.damnodice)
#define GET_DR(nr) (mob_proto[(nr)].points.damroll)
#define GET_ADDHP(nr) (mob_proto[(nr)].points.move)

ACMD(adjust_mobs)    
{
     int nr;
     int vznum;
 
     for (nr = 0; nr <= top_of_mobt; nr++)
       {
        GET_DR(nr) = GET_LEVEL(mob_proto + nr)>10?
                      GET_LEVEL(mob_proto + nr)/1.50:
                      GET_LEVEL(mob_proto + nr)/2;
           
        GET_ADDHP(nr) = (10*GET_LEVEL(mob_proto + nr)+10);/* add_hp */
        GET_ADDHP(nr) = GET_LEVEL(mob_proto + nr) > 22 ?
         GET_ADDHP(nr)+(13*(GET_LEVEL(mob_proto + nr)-22)) :
         GET_ADDHP(nr);
        GET_ADDHP(nr) = GET_LEVEL(mob_proto + nr) > 30 ?
         GET_ADDHP(nr)+(560*(GET_LEVEL(mob_proto + nr)-30)) :
         GET_ADDHP(nr);   

        vznum=mob_index[nr].virtual/100;
        olc_add_to_save_list(vznum,OLC_SAVE_MOB);
       }
     send_to_char(OK,ch);
}
/*------------------------------------------------------------*\
 Internal utlities 
\*------------------------------------------------------------*/

static char *save_msg[5] = { "Rooms", "Objects", "Zone info", "Mobiles", "Shops" };

void olc_saveall(struct char_data *ch)
{
  struct olc_save_info *entry;  

  void (*save_func[5])(struct descriptor_data *d) = {
    redit_save_to_disk,
    oedit_save_to_disk,
    zedit_save_to_disk,
    medit_save_to_disk,
    sedit_save_to_disk
  };
  
  if (!olc_save_list) {
    send_to_char("The database is up to date.\r\n", ch);
    return;
  }

  CREATE(ch->desc->olc, struct olc_data, 1);

  while ((entry = olc_save_list)) {
    SET_OLC_ZNUM(ch->desc, real_zone(entry->zone * 100));
    sprintf(buf, "%s saved for zone %d.\r\n", save_msg[(int)entry->type],
	    entry->zone);
    send_to_char(buf, ch);
    /* save_func will free and remove the entry from olc_save_list */
    save_func[(int)entry->type](ch->desc);
  }

  FREE(ch->desc->olc);

  sprintf(buf, "OLC: %s saves all", GET_NAME(ch));
  mudlog(buf, CMP, LVL_BUILDER, TRUE);
}

void olc_saveinfo(struct char_data *ch)
{ 
  struct olc_save_info *entry;

  if (olc_save_list)
    send_to_char("The following OLC components need saving:-\r\n", ch);
  else {
    send_to_char("The database is up to date.\r\n", ch);
    return;
  }

  for (entry = olc_save_list; entry; entry = entry->next) {
    sprintf(buf, " - %s for zone %d.\r\n", save_msg[(int)entry->type],
	    entry->zone);
    send_to_char(buf, ch);
  }
}


int real_zone(int number)
{ int counter;
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

    /* free storage if allocated (for tedit and aedit) */
    if (OLC_STORAGE(d))
      FREE(OLC_STORAGE(d));

    if (OLC_BUFFER(d))
      FREE(OLC_BUFFER(d));

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

    /*. Restore desciptor playing status .*/
    if (d->character)
    { REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
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

