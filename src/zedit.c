/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*                                                             *
*  OasisOLC - zedit.c                                                 *
*                                                             *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "olc.h"

/*-------------------------------------------------------------------*/
/*. external data areas .*/
extern struct zone_data *zone_table;    /*. db.c    .*/
extern struct room_data *world;     /*. db.c    .*/
extern int top_of_zone_table;       /*. db.c    .*/
extern struct char_data *mob_proto; /*. db.c    .*/
extern struct index_data *mob_index;    /*. db.c    .*/
extern struct obj_data *obj_proto;  /*. db.c    .*/
extern struct index_data *obj_index;    /*. db.c    .*/
extern char *equipment_types[];     /*. constants.c .*/
extern char *dirs[];            /*. constants.c .*/

/*-------------------------------------------------------------------*/
/* function protos */
void zedit_disp_menu(struct descriptor_data * d);
void zedit_setup(struct descriptor_data *d, int room_num);
void add_cmd_to_list(struct reset_com **list, struct reset_com *newcmd, int pos);
void remove_cmd_from_list(struct reset_com **list, int pos);
void delete_command(struct descriptor_data *d, int pos);
int new_command(struct descriptor_data *d, int pos);
int start_change_command(struct descriptor_data *d, int pos);
void zedit_disp_comtype(struct descriptor_data *d);
void zedit_disp_arg1(struct descriptor_data *d);
void zedit_disp_arg2(struct descriptor_data *d);
void zedit_disp_arg3(struct descriptor_data *d);
void zedit_save_internally(struct descriptor_data *d);
void zedit_save_to_disk(struct descriptor_data *d);
void zedit_create_index(int znum, char *type);
void zedit_new_zone(struct char_data *ch, int vzone_num);

/*-------------------------------------------------------------------*/
/*. Nasty internal macros to clean up the code .*/

#define ZCMD zone_table[OLC_ZNUM(d)].cmd[subcmd]
#define MYCMD OLC_ZONE(d)->cmd[subcmd]
#define OLC_CMD(d)   (OLC_ZONE(d)->cmd[OLC_VAL(d)])

/*-------------------------------------------------------------------*\
  utility functions
\*-------------------------------------------------------------------*/

void zedit_setup(struct  descriptor_data *d, int room_num)
{ struct zone_data *zone;
  int subcmd = 0, count = 0, cmd_room = -1;

  /*. Alloc some zone shaped space .*/
  CREATE(zone, struct zone_data, 1);

  /*. Copy in zone header info .*/
  zone->name = str_dup(zone_table[OLC_ZNUM(d)].name);
  zone->lifespan = zone_table[OLC_ZNUM(d)].lifespan;
  zone->top = zone_table[OLC_ZNUM(d)].top;
  zone->reset_mode = zone_table[OLC_ZNUM(d)].reset_mode;
  /*. The remaining fields are used as a 'has been modified' flag .*/
  zone->number = 0;     /*. Header info has changed .*/
  zone->age = 0;    /*. Commands have changed   .*/

  /*. Start the reset command list with a terminator .*/
  CREATE(zone->cmd, struct reset_com, 1);
  zone->cmd[0].command = 'S';

  /*. Add all entried in zone_table that relate to this room .*/
  while(ZCMD.command != 'S')
  { switch(ZCMD.command)
    { case 'M':
      case 'O':
        cmd_room = ZCMD.arg3;
        break;
      case 'D':
      case 'R':
      case 'L':
        cmd_room = ZCMD.arg1;
        break;
      default:
        break;
    }
    if(cmd_room == room_num)
    { add_cmd_to_list(&(zone->cmd), &ZCMD, count);
      count++;
    }
    subcmd++;
  }

  OLC_ZONE(d) = zone;
  /*. Display main menu .*/
  zedit_disp_menu(d);
}


/*-------------------------------------------------------------------*/
/*. Create a new zone .*/

void
zedit_new_zone(struct char_data *ch, int vzone_num)
{
  FILE *fp;
  struct zone_data *new_table;
  int i, room, found = 0;

  if(vzone_num > 326)
  { send_to_char("326 is the highest zone allowed.\r\n", ch);
    return;
  }
  sprintf(buf, "%s/%i.zon", ZON_PREFIX, vzone_num);

  /*. Check zone does not exist .*/
  room = vzone_num * 100;
  for (i = 0; i <= top_of_zone_table; i++)
    if ((zone_table[i].number * 100 <= room) &&
        (zone_table[i].top >= room))
    { send_to_char("A zone already covers that area.\r\n", ch);
      return;
    }

  /*. Create Zone file .*/
  if(!(fp = fopen(buf, "w"))) {
    mudlog("SYSERR: OLC: Can't write new zone file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp,
    "#%d\n"
    "New Zone~\n"
    "%d 30 2\n"
    "S\n"
    "$\n",
    vzone_num,
    (vzone_num * 100) + 99
  );
  fclose(fp);

  /*. Create Rooms file .*/
  sprintf(buf, "%s/%d.wld", WLD_PREFIX, vzone_num);
  if(!(fp = fopen(buf, "w")))
  { mudlog("SYSERR: OLC: Can't write new world file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp,
    "#%d\n"
        "The Begining~\n"
    "Not much here.\n"
    "~\n"
    "%d 0 0\n"
    "S\n"
    "$\n",
    vzone_num * 100,
    vzone_num
  );
  fclose(fp);

  /*. Create Mobiles file .*/
  sprintf(buf, "%s/%i.mob", MOB_PREFIX, vzone_num);
  if(!(fp = fopen(buf, "w")))
  { mudlog("SYSERR: OLC: Can't write new mob file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp, "$\n");
  fclose(fp);

  /*. Create Objects file .*/
  sprintf(buf, "%s/%i.obj", OBJ_PREFIX, vzone_num);
  if(!(fp = fopen(buf, "w")))
  { mudlog("SYSERR: OLC: Can't write new obj file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp, "$\n");
  fclose(fp);

  /*. Create Shops file .*/
  sprintf(buf, "%s/%i.shp", SHP_PREFIX, vzone_num);
  if(!(fp = fopen(buf, "w")))
  { mudlog("SYSERR: OLC: Can't write new shop file", BRF, LVL_IMPL, TRUE);
    return;
  }
  fprintf(fp, "$~\n");
  fclose(fp);

  /*. Update index files .*/
  zedit_create_index(vzone_num, "zon");
  zedit_create_index(vzone_num, "wld");
  zedit_create_index(vzone_num, "mob");
  zedit_create_index(vzone_num, "obj");
  zedit_create_index(vzone_num, "shp");

  /*. Make a new zone in memory.*/
  CREATE(new_table, struct zone_data, top_of_zone_table+3);
  new_table[top_of_zone_table + 1].number = 32000;
  for(i = 0; i <= top_of_zone_table + 1; i++)
    if(!found)
      if (i > top_of_zone_table || zone_table[i].number > vzone_num)
      { found = 1;
        new_table[i].name = str_dup("New Zone");
        new_table[i].number = vzone_num;
        new_table[i].top = (vzone_num * 100) + 99;
        new_table[i].lifespan = 30;
        new_table[i].age = 0;
        new_table[i].reset_mode = 2;
        CREATE(new_table[i].cmd, struct reset_com, 1);
        new_table[i].cmd[0].command = 'S';
        if (i <= top_of_zone_table)
          new_table[i+1] = zone_table[i];
      } else
        new_table[i] = zone_table[i];
    else
      new_table[i+1] = zone_table[i];
  FREE(zone_table);
  zone_table = new_table;
  top_of_zone_table++;

  sprintf(buf, "OLC: %s creates new zone #%d", GET_NAME(ch), vzone_num);
  mudlog(buf, BRF, LVL_BUILDER, TRUE);
  send_to_char("Zone created.\r\n", ch);
  return;
}

/*-------------------------------------------------------------------*/

void zedit_create_index(int znum, char *type)
{ FILE *newfile, *oldfile;
  char new_name[32], old_name[32], *prefix;
  int num, found = FALSE;

  switch(*type)
  { case 'z':
      prefix = ZON_PREFIX;
      break;
    case 'w':
      prefix = WLD_PREFIX;
      break;
    case 'o':
      prefix = OBJ_PREFIX;
      break;
    case 'm':
      prefix = MOB_PREFIX;
      break;
    case 's':
      prefix = SHP_PREFIX;
      break;
    default:
      /*. Caller messed up .*/
      return;
  }

  sprintf(old_name, "%s/index", prefix);
  sprintf(new_name, "%s/newindex", prefix);

  if (!(oldfile = fopen(old_name, "r"))){
    sprintf(buf, "SYSERR: OLC: Failed to open %s", buf);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
    return;
  }

  if (!(newfile = fopen(new_name, "w"))) {
    sprintf(buf, "SYSERR: OLC: Failed to open %s", buf);
    mudlog(buf, BRF, LVL_IMPL, TRUE);
    return;
  }

  /*. Index contents must be in order: search through the old file for
      the right place, insert the new file, then copy the rest over.
      .*/

  sprintf(buf1, "%d.%s", znum, type);
  while (get_line(oldfile, buf)) {
    if (*buf == '$') {
      if (!found)
        fprintf(newfile, "%s\n", buf1);
      fprintf(newfile, "$\n");
      break;
    }
    if (!found) {
      sscanf(buf, "%d", &num);
      if (num >= znum) {
    found = TRUE;
    if (num > znum)
      fprintf(newfile, "%s\n", buf1);
      }
    }
    fprintf(newfile, "%s\n", buf);
  }

  fclose(newfile);
  fclose(oldfile);
  /*. Out with the old, in with the new .*/
  remove(old_name);
  rename(new_name, old_name);
}

/*-------------------------------------------------------------------*/
/*. Save all the information on the players descriptor back into
    the zone table .*/

void zedit_save_internally(struct descriptor_data *d)
{ int subcmd = 0, cmd_room = -2, room_num;

  room_num = real_room(OLC_NUM(d));

  /*. Zap all entried in zone_table that relate to this room .*/
  while(ZCMD.command != 'S')
  { switch(ZCMD.command)
    { case 'M':
      case 'O':
        cmd_room = ZCMD.arg3;
        break;
      case 'D':
      case 'R':
      case 'L':
        cmd_room = ZCMD.arg1;
        break;
      default:
        break;
    }
    if(cmd_room == room_num)
      remove_cmd_from_list(&(zone_table[OLC_ZNUM(d)].cmd), subcmd);
    else
      subcmd++;
  }

  /*. Now add all the entries in the players descriptor list .*/
  subcmd = 0;
  while(MYCMD.command != 'S')
  { add_cmd_to_list(&(zone_table[OLC_ZNUM(d)].cmd), &MYCMD, subcmd);
    subcmd++;
  }

  /*. Finally, if zone headers have been changed, copy over .*/
  if (OLC_ZONE(d)->number)
  { FREE(zone_table[OLC_ZNUM(d)].name);
    zone_table[OLC_ZNUM(d)].name    = str_dup(OLC_ZONE(d)->name);
    zone_table[OLC_ZNUM(d)].top     = OLC_ZONE(d)->top;
    zone_table[OLC_ZNUM(d)].reset_mode  = OLC_ZONE(d)->reset_mode;
    zone_table[OLC_ZNUM(d)].lifespan    = OLC_ZONE(d)->lifespan;
  }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ZONE);
}


/*-------------------------------------------------------------------*/
/*. Save all the zone_table for this zone to disk.  Yes, this could
    automatically comment what it saves out, but why bother when you
    have an OLC as cool as this ? :> .*/

void zedit_save_to_disk(struct descriptor_data *d)
{ int subcmd, arg1 = -1, arg2 = -1, arg3 = -1;
  char fname[64];
  FILE *zfile;

  sprintf(fname, "%s/%i.zon", ZON_PREFIX,
           zone_table[OLC_ZNUM(d)].number);

  if(!(zfile = fopen(fname, "w"))) {
    sprintf(buf, "SYSERR: OLC: zedit_save_to_disk:  Can't write zone %d.",
            zone_table[OLC_ZNUM(d)].number);
    mudlog(buf, BRF, LVL_BUILDER, TRUE);
    return;
  }

  /*. Print zone header to file .*/
  sprintf(buf,
    "#%d\n"
    "%s~\n"
    "%d %d %d\n",
    zone_table[OLC_ZNUM(d)].number,
    zone_table[OLC_ZNUM(d)].name ? zone_table[OLC_ZNUM(d)].name : "undefined",
    zone_table[OLC_ZNUM(d)].top,
        zone_table[OLC_ZNUM(d)].lifespan,
        zone_table[OLC_ZNUM(d)].reset_mode
  );
  fprintf(zfile, "%s", buf);

  for(subcmd = 0; ZCMD.command != 'S'; subcmd++) {
    switch (ZCMD.command) {
      case 'M':
    arg1 = mob_index[ZCMD.arg1].virtual;
    arg2 = ZCMD.arg2;
    arg3 = world[ZCMD.arg3].number;
    break;
      case 'O':
    arg1 = obj_index[ZCMD.arg1].virtual;
    arg2 = ZCMD.arg2;
    arg3 = world[ZCMD.arg3].number;
    break;
      case 'G':
    arg1 = obj_index[ZCMD.arg1].virtual;
    arg2 = ZCMD.arg2;
    arg3 = -1;
    break;
      case 'E':
    arg1 = obj_index[ZCMD.arg1].virtual;
    arg2 = ZCMD.arg2;
    arg3 = ZCMD.arg3;
    break;
      case 'P':
    arg1 = obj_index[ZCMD.arg1].virtual;
    arg2 = ZCMD.arg2;
    arg3 = obj_index[ZCMD.arg3].virtual;
    break;
      case 'D':
    arg1 = world[ZCMD.arg1].number;
    arg2 = ZCMD.arg2;
    arg3 = ZCMD.arg3;
    break;
      case 'R':
    arg1 = world[ZCMD.arg1].number;
    arg2 = ZCMD.arg2;
    if (ZCMD.arg2)
      arg3 = obj_index[ZCMD.arg3].virtual;
    else
      arg3 = mob_index[ZCMD.arg3].virtual;
    break;
      case 'L':
    arg1 = world[ZCMD.arg1].number;
    arg2 = ZCMD.arg2;
    arg3 = ZCMD.arg3;
    break;
      case '*':
        /*. Invalid commands are replaced with '*' - Ignore them .*/
        continue;
      default:
        sprintf(buf, "SYSERR: OLC: z_save_to_disk(): Unknown cmd '%c' - NOT saving", ZCMD.command);
        mudlog(buf, BRF, LVL_BUILDER, TRUE);
        continue;
    }
    fprintf(zfile, "%c %d %d %d %d\n",
    ZCMD.command, ZCMD.if_flag, arg1, arg2, arg3);
  }
  fprintf(zfile, "S\n$\n");
  fclose(zfile);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ZONE);
}

/*-------------------------------------------------------------------*/
/*. Adds a new reset command into a list.  Takes a pointer to the list
    so that it may play with the memory locations.*/

void add_cmd_to_list(struct reset_com **list, struct reset_com *newcmd, int pos)
{  int count = 0, i, l;
   struct reset_com *newlist;

   /*. Count number of commands (not including terminator) .*/
   while((*list)[count].command != 'S')
     count++;

   CREATE(newlist, struct reset_com, count + 2);

   /*. Tight loop to copy old list and insert new command .*/
   l = 0;
   for(i=0;i<=count;i++)
     if(i==pos)
       newlist[i] = *newcmd;
     else
       newlist[i] = (*list)[l++];

   /*. Add terminator then insert new list .*/
   newlist[count+1].command = 'S';
   FREE(*list);
   *list = newlist;
}

/*-------------------------------------------------------------------*/
/*. Remove a reset command from a list.  Takes a pointer to the list
    so that it may play with the memory locations.*/

void remove_cmd_from_list(struct reset_com **list, int pos)
{  int count = 0, i, l;
   struct reset_com *newlist;

   /*. Count number of commands (not including terminator) .*/
   while((*list)[count].command != 'S')
     count++;

   CREATE(newlist, struct reset_com, count);

   /*. Tight loop to copy old list and skip unwanted command .*/
   l = 0;
   for(i=0;i<count;i++)
     if(i==pos)
       continue;
     else
       newlist[l++] = (*list)[i];

   /*. Add terminator then insert new list .*/
   newlist[count-1].command = 'S';
   FREE(*list);
   *list = newlist;
}

/*-------------------------------------------------------------------*/
/*. Error check user input and then add new (blank) command .*/

int new_command(struct descriptor_data *d, int pos)
{  int subcmd = 0;
   struct reset_com *new_com;

   /*. Error check to ensure users hasn't given too large an index .*/
   while(MYCMD.command != 'S')
     subcmd++;

   if ((pos > subcmd) || (pos < 0))
     return 0;

   /*. Ok, let's add a new (blank) command.*/
   CREATE(new_com, struct reset_com, 1);
   new_com->command = 'N';
   add_cmd_to_list(&OLC_ZONE(d)->cmd, new_com, pos);
   return 1;
}


/*-------------------------------------------------------------------*/
/*. Error check user input and then remove command .*/

void delete_command(struct descriptor_data *d, int pos)
{  int subcmd = 0;

   /*. Error check to ensure users hasn't given too large an index .*/
   while(MYCMD.command != 'S')
     subcmd++;

   if ((pos >= subcmd) || (pos < 0))
     return;

   /*. Ok, let's zap it .*/
   remove_cmd_from_list(&OLC_ZONE(d)->cmd, pos);
}

/*-------------------------------------------------------------------*/
/*. Error check user input and then setup change .*/

int start_change_command(struct descriptor_data *d, int pos)
{  int subcmd = 0;

   /*. Error check to ensure users hasn't given too large an index .*/
   while(MYCMD.command != 'S')
     subcmd++;

   if ((pos >= subcmd) || (pos < 0))
     return 0;

   /*. Ok, let's get editing .*/
   OLC_VAL(d) = pos;
   return 1;
}

/**************************************************************************
 Menu functions
 **************************************************************************/

/* the main menu */
void zedit_disp_menu(struct descriptor_data * d)
{
  int subcmd = 0, room, counter = 0, repeat = 0;

  get_char_cols(d->character);
  room = real_room(OLC_NUM(d));

  /*. Menu header .*/
  snprintf(buf, MAX_STRING_LENGTH,
    "\r\n"
    "Room number: %s%d%s        Room zone: %s%d\r\n"
        "%sZ%s) Zone name   : %s%s\r\n"
        "%sL%s) Lifespan    : %s%d minutes\r\n"
        "%sT%s) Top of zone : %s%d\r\n"
        "%sR%s) Reset Mode  : %s%s%s\r\n"
        "[Command list]\r\n",

    cyn, OLC_NUM(d), nrm,
    cyn, zone_table[OLC_ZNUM(d)].number,
        grn, nrm, yel, OLC_ZONE(d)->name ? OLC_ZONE(d)->name : "<NONE!>",
        grn, nrm, yel, OLC_ZONE(d)->lifespan,
        grn, nrm, yel, OLC_ZONE(d)->top,
        grn, nrm, yel, OLC_ZONE(d)->reset_mode ?
          ((OLC_ZONE(d)->reset_mode == 1) ?
          "Reset when no players are in zone." :
          "Normal reset.") :
          "Never reset", nrm
  );

  /*. Print the commands for this room into display buffer .*/
  while(MYCMD.command != 'S')
  { /*. Translate what the command means .*/
    switch(MYCMD.command)
    { case'M':
        sprintf(buf2, "%s%sLoad %s [%s%d%s], Max : %d",
        repeat ? "  " : "",
                MYCMD.if_flag ? " then " : "",
        mob_proto[MYCMD.arg1].player.short_descr,
                cyn, mob_index[MYCMD.arg1].virtual, yel,
        MYCMD.arg2
        );
        break;
      case'G':
        sprintf(buf2, "%s%sGive it %s [%s%d%s], Max : %d",
        repeat ? "  " : "",
        MYCMD.if_flag ? " then " : "",
        obj_proto[MYCMD.arg1].short_description,
                cyn, obj_index[MYCMD.arg1].virtual, yel,
        MYCMD.arg2
        );
        break;
      case'O':
        sprintf(buf2, "%s%sLoad %s [%s%d%s], Max : %d",
        repeat ? "  " : "",
        MYCMD.if_flag ? " then " : "",
        obj_proto[MYCMD.arg1].short_description,
                cyn, obj_index[MYCMD.arg1].virtual, yel,
        MYCMD.arg2
        );
        break;
      case'E':
        sprintf(buf2, "%s%sEquip with %s [%s%d%s], %s, Max : %d",
        repeat ? "  " : "",
        MYCMD.if_flag ? " then " : "",
        obj_proto[MYCMD.arg1].short_description,
                cyn, obj_index[MYCMD.arg1].virtual, yel,
        equipment_types[MYCMD.arg3],
        MYCMD.arg2
        );
        break;
      case'P':
        sprintf(buf2, "%s%sPut %s [%s%d%s] in %s [%s%d%s], Max : %d",
        repeat ? "  " : "",
        MYCMD.if_flag ? " then " : "",
        obj_proto[MYCMD.arg1].short_description,
                cyn, obj_index[MYCMD.arg1].virtual, yel,
        obj_proto[MYCMD.arg3].short_description,
                cyn, obj_index[MYCMD.arg3].virtual, yel,
        MYCMD.arg2
        );
        break;
      case'R':
        sprintf(buf2, "%s%sRemove %s [%s%d%s] from room.",
        repeat ? "  " : "",
        MYCMD.if_flag ? " then " : "",
        MYCMD.arg2 ? obj_proto[MYCMD.arg3].short_description : mob_proto[MYCMD.arg3].player.short_descr,
        cyn,
        MYCMD.arg2 ? obj_index[MYCMD.arg3].virtual : mob_index[MYCMD.arg3].virtual,
        yel
        );
        break;
      case'D':
        sprintf(buf2, "%s%sSet door %s as %s.",
        repeat ? "  " : "",
        MYCMD.if_flag ? " then " : "",
        dirs[MYCMD.arg2],
        MYCMD.arg3 ? ((MYCMD.arg3 == 1) ?
                  "closed" : "locked") : "open"
        );
        break;
      case 'L':
        if (!MYCMD.arg2) {
          sprintf(buf1, "Repeat for %d times...", MYCMD.arg3);
          repeat = TRUE;
        } else
          repeat = FALSE;

        sprintf(buf2, "%s%s",
          MYCMD.if_flag ? " then " : "",
          MYCMD.arg2 ? "...End Repeat" : buf1);
        break;
      default:
        strcpy(buf2, "<Unknown Command>");
        break;
    }
    /*. Build the display buffer for this command .*/
    sprintf(buf1, "%s%d - %s%s\r\n",
        nrm, counter++, yel,
        buf2
    );
    strcat(buf, buf1);
    subcmd++;
  }
  /*. Finish off menu .*/
  sprintf(buf1,
        "%s%d - <END OF LIST>\r\n"
        "%sN%s) New command.\r\n"
        "%sE%s) Edit a command.\r\n"
        "%sD%s) Delete a command.\r\n"
        "%sQ%s) Quit\r\nEnter your choice : ",
        nrm, counter,
                grn, nrm, grn, nrm, grn, nrm, grn, nrm
  );

  strcat(buf, buf1);
  send_to_char(buf, d->character);

  OLC_MODE(d) = ZEDIT_MAIN_MENU;
}


/*-------------------------------------------------------------------*/
/*. Print the command type menu and setup response catch. */

void zedit_disp_comtype(struct descriptor_data *d)
{ get_char_cols(d->character);
  sprintf(buf, "\r\n"
    "%sM%s) Load Mobile to room              %sO%s) Load Object to room\r\n"
    "%sE%s) Equip mobile with object         %sG%s) Give an object to a mobile\r\n"
    "%sP%s) Put object in another object     %sD%s) Open/Close/Lock a Door\r\n"
    "%sR%s) Remove a mobile/object from room %sL%s) Begin/End Looping\r\n"
    "What sort of command will this be? : ",
    grn, nrm, grn, nrm, grn, nrm, grn, nrm, grn, nrm,
    grn, nrm, grn, nrm , grn , nrm
  );
  send_to_char(buf, d->character);
  OLC_MODE(d)=ZEDIT_COMMAND_TYPE;
}


/*-------------------------------------------------------------------*/
/*. Print the appropriate message for the command type for arg1 and set
    up the input catch clause .*/

void zedit_disp_arg1(struct descriptor_data *d)
{
  switch(OLC_CMD(d).command)
  { case 'M':
      send_to_char("Input mob's vnum : ", d->character);
      OLC_MODE(d) = ZEDIT_ARG1;
      break;
    case 'O':
    case 'E':
    case 'P':
    case 'G':
      send_to_char("Input object vnum : ", d->character);
      OLC_MODE(d) = ZEDIT_ARG1;
      break;
    case 'D':
    case 'R':
      /*. Arg1 for these is the room number, skip to arg2 .*/
      OLC_CMD(d).arg1 = real_room(OLC_NUM(d));
      zedit_disp_arg2(d);
      break;
    case 'L':
      /*. Arg1 for these is the room number, skip to arg2 .*/
      OLC_CMD(d).arg1 = real_room(OLC_NUM(d));
      zedit_disp_arg2(d);
      break;
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog("SYSERR: OLC: zedit_disp_arg1(): Help!", BRF, LVL_BUILDER, TRUE);
      return;
  }
}



/*-------------------------------------------------------------------*/
/*. Print the appropriate message for the command type for arg2 and set
    up the input catch clause .*/

void zedit_disp_arg2(struct descriptor_data *d)
{ int i = 0;
  switch(OLC_CMD(d).command)
  { case 'M':
    case 'O':
    case 'E':
    case 'P':
    case 'G':
      send_to_char("Input the maximum number that can exist on the mud : ", d->character);
      break;
    case 'D':
      while(*dirs[i] != '\n')
      { sprintf(buf, "%d) Exit %s.\r\n", i, dirs[i]);
        send_to_char(buf, d->character);
        i++;
      }
      send_to_char("Enter exit number for door : ", d->character);
      break;
    case 'R':
      send_to_char("Input 0 for a mobile, 1 for an object: ", d->character);
      break;
    case 'L':
      send_to_char("Input 0 for loop start, 1 for loop finish: ", d->character);
      break;
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog("SYSERR: OLC: zedit_disp_arg2(): Help!", BRF, LVL_BUILDER, TRUE);
      return;
  }
  OLC_MODE(d) = ZEDIT_ARG2;
}


/*-------------------------------------------------------------------*/
/*. Print the appropriate message for the command type for arg3 and set
    up the input catch clause .*/

void zedit_disp_arg3(struct descriptor_data *d)
{ int i = 0;
  switch(OLC_CMD(d).command)
  {
    case 'E':
      while(*equipment_types[i] !=  '\n')
      { sprintf(buf, "%2d) %26.26s %2d) %26.26s\r\n", i,
         equipment_types[i], i+1, (*equipment_types[i+1] != '\n') ?
         equipment_types[i+1] : "");
        send_to_char(buf, d->character);
        if(*equipment_types[i+1] != '\n')
          i+=2;
        else
          break;
      }
      send_to_char("Input location to equip : ", d->character);
      break;
    case 'P':
      send_to_char("Input the vnum of the container : ", d->character);
      break;
    case 'D':
      send_to_char("0)  Door open\r\n"
                   "1)  Door closed\r\n"
                   "2)  Door locked\r\n"
                   "Enter state of the door : ", d->character);
      break;
    case 'L':
      if (!OLC_CMD(d).arg2)
        send_to_char("Input the number of times to repeat the loop : ", d->character);
      else
        OLC_CMD(d).arg3 = -1;
      break;
    case 'R':
      if (OLC_CMD(d).arg2)
    send_to_char("Input object's vnum : ", d->character);
      else
    send_to_char("Input mobile's vnum : ", d->character);
      break;
    case 'M':
    case 'O':
    case 'G':
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog("SYSERR: OLC: zedit_disp_arg3(): Help!", BRF, LVL_BUILDER, TRUE);
      return;
  }
  OLC_MODE(d) = ZEDIT_ARG3;
}



/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/

void zedit_parse(struct descriptor_data * d, char *arg)
{
  int pos, old_top, i = 0;

  switch (OLC_MODE(d))
  {
/*-------------------------------------------------------------------*/
  case ZEDIT_CONFIRM_SAVESTRING:
    switch (*arg) {
    case 'y':
    case 'Y':
      /*. Save the zone in memory .*/
      send_to_char("Saving zone info in memory.\r\n", d->character);
      zedit_save_internally(d);
      sprintf(buf, "OLC: %s edits zone info for room %d", GET_NAME(d->character),
              OLC_NUM(d));
      mudlog(buf, CMP, LVL_BUILDER, TRUE);
      cleanup_olc(d, CLEANUP_ALL);
      break;
    case 'n':
    case 'N':
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save the zone info? : ", d->character);
      break;
    }
    break; /* end of ZEDIT_CONFIRM_SAVESTRING */

/*-------------------------------------------------------------------*/
  case ZEDIT_MAIN_MENU:
    switch (*arg)
    { case 'q':
      case 'Q':
        if (OLC_ZONE(d)->age || OLC_ZONE(d)->number) {
      send_to_char(
        "Do you wish to save the changes to the zone info? (y/n) : ",
        d->character);
          OLC_MODE(d) = ZEDIT_CONFIRM_SAVESTRING;
        } else {
      send_to_char("No changes made.\r\n", d->character);
          cleanup_olc(d, CLEANUP_ALL);
        }
        break;
      case 'n':
      case 'N':
        /*. New entry .*/
        send_to_char("What number in the list should the new command be? : ", d->character);
        OLC_MODE(d) = ZEDIT_NEW_ENTRY;
        break;
      case 'e':
      case 'E':
        /*. Change an entry .*/
        send_to_char("Which command do you wish to change? : ", d->character);
        OLC_MODE(d) = ZEDIT_CHANGE_ENTRY;
        break;
      case 'd':
      case 'D':
        /*. Delete an entry .*/
        send_to_char("Which command do you wish to delete? : ", d->character);
        OLC_MODE(d) = ZEDIT_DELETE_ENTRY;
        break;
      case 'z':
      case 'Z':
        /*. Edit zone name .*/
        send_to_char("Enter new zone name : ", d->character);
        OLC_MODE(d) = ZEDIT_ZONE_NAME;
        break;
      case 't':
      case 'T':
        /*. Edit zone top .*/
        if(GET_LEVEL(d->character) < LVL_IMPL)
          zedit_disp_menu(d);
        else
        { send_to_char("Enter new top of zone : ", d->character);
          OLC_MODE(d) = ZEDIT_ZONE_TOP;
        }
        break;
      case 'l':
      case 'L':
        /*. Edit zone lifespan .*/
        send_to_char("Enter new zone lifespan : ", d->character);
        OLC_MODE(d) = ZEDIT_ZONE_LIFE;
        break;
      case 'r':
      case 'R':
        /*. Edit zone reset mode .*/
        send_to_char("\r\n"
                     "0) Never reset\r\n"
                     "1) Reset only when no players in zone\r\n"
                     "2) Normal reset\r\n"
                     "Enter new zone reset type : ", d->character);
        OLC_MODE(d) = ZEDIT_ZONE_RESET;
        break;
      default:
        zedit_disp_menu(d);
        break;
    }
    break; /*. End ZEDIT_MAIN_MENU .*/

/*-------------------------------------------------------------------*/
  case ZEDIT_NEW_ENTRY:
    /*. Get the line number and insert the new line .*/
    pos = atoi(arg);
    if (isdigit(*arg) && new_command(d, pos))
    {  if (start_change_command(d, pos))
       { zedit_disp_comtype(d);
         OLC_ZONE(d)->age = 1;
       }
    } else
      zedit_disp_menu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_DELETE_ENTRY:
    /*. Get the line number and delete the line .*/
    pos = atoi(arg);
    if(isdigit(*arg))
    { delete_command(d, pos);
      OLC_ZONE(d)->age = 1;
    }
    zedit_disp_menu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_CHANGE_ENTRY:
    /*. Parse the input for which line to edit, and goto next quiz .*/
    pos = atoi(arg);
    if(isdigit(*arg) && start_change_command(d, pos))
    { zedit_disp_comtype(d);
      OLC_ZONE(d)->age = 1;
    } else
      zedit_disp_menu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_COMMAND_TYPE:
    /*. Parse the input for which type of command this is,
        and goto next quiz .*/
    OLC_CMD(d).command = toupper(*arg);
    if (!OLC_CMD(d).command || (strchr("MOPEDGRL", OLC_CMD(d).command) == NULL))
    { send_to_char("Invalid choice, try again : ", d->character);
    } else
    { if (OLC_VAL(d))
      { /*. If there was a previous command .*/
        send_to_char("Is this command dependent on the success of the previous one? (y/n)\r\n", d->character);
        OLC_MODE(d) = ZEDIT_IF_FLAG;
      } else
      { /*. 'if-flag' not appropriate .*/
        OLC_CMD(d).if_flag = 0;
        zedit_disp_arg1(d);
      }
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_IF_FLAG:
    /*. Parse the input for the if flag, and goto next quiz .*/
    switch(*arg)
    { case 'y':
      case 'Y':
        OLC_CMD(d).if_flag = 1;
        break;
      case 'n':
      case 'N':
        OLC_CMD(d).if_flag = 0;
        break;
      default:
        send_to_char("Try again : ", d->character);
        return;
    }
    zedit_disp_arg1(d);
    break;


/*-------------------------------------------------------------------*/
  case ZEDIT_ARG1:
    /*. Parse the input for arg1, and goto next quiz .*/
    if  (!isdigit(*arg))
    { send_to_char("Must be a numeric value, try again : ", d->character);
      return;
    }
    switch(OLC_CMD(d).command)
    { case 'M':
        pos = real_mobile(atoi(arg));
        if (pos >= 0)
        { OLC_CMD(d).arg1 = pos;
          zedit_disp_arg2(d);
        } else
          send_to_char("That mobile does not exist, try again : ", d->character);
        break;
      case 'O':
      case 'P':
      case 'E':
      case 'G':
        pos = real_object(atoi(arg));
        if (pos >= 0)
        { OLC_CMD(d).arg1 = pos;
          zedit_disp_arg2(d);
        } else
          send_to_char("That object does not exist, try again : ", d->character);
        break;
      case 'D':
      case 'R':
      case 'L':
      default:
        /*. We should never get here .*/
        cleanup_olc(d, CLEANUP_ALL);
        mudlog("SYSERR: OLC: zedit_parse(): case ARG1: Ack!", BRF, LVL_BUILDER, TRUE);
        break;
    }
    break;


/*-------------------------------------------------------------------*/
  case ZEDIT_ARG2:
    /*. Parse the input for arg2, and goto next quiz .*/
    if  (!isdigit(*arg))
    { send_to_char("Must be a numeric value, try again : ", d->character);
      return;
    }
    switch(OLC_CMD(d).command)
    { case 'M':
      case 'O':
        OLC_CMD(d).arg2 = atoi(arg);
        OLC_CMD(d).arg3 = real_room(OLC_NUM(d));
        zedit_disp_menu(d);
        break;
      case 'G':
        OLC_CMD(d).arg2 = atoi(arg);
        zedit_disp_menu(d);
        break;
      case 'P':
      case 'E':
        OLC_CMD(d).arg2 = atoi(arg);
        zedit_disp_arg3(d);
        break;
      case 'D':
        pos = atoi(arg);
        /*. Count dirs .*/
        while(*dirs[i] != '\n')
          i++;
        if ((pos < 0) || (pos > i))
          send_to_char("Try again : ", d->character);
        else
        { OLC_CMD(d).arg2 = pos;
          zedit_disp_arg3(d);
        }
        break;
      case 'R':
        pos = atoi(arg);
        if ((pos < 0) || (pos > 1))
          send_to_char("Try again : ", d->character);
        else
        { OLC_CMD(d).arg2 = pos;
          zedit_disp_arg3(d);
        }
        break;
      case 'L':
        pos = atoi(arg);
        if ((pos < 0) || (pos > 1))
          send_to_char("Try again : ", d->character);
        else
        { OLC_CMD(d).arg2 = pos;
          if (pos)
            zedit_disp_menu(d);
          else
            zedit_disp_arg3(d);
        }
        break;
      default:
        /*. We should never get here .*/
        cleanup_olc(d, CLEANUP_ALL);
        mudlog("SYSERR: OLC: zedit_parse(): case ARG2: Ack!", BRF, LVL_BUILDER, TRUE);
        break;
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ARG3:
    /*. Parse the input for arg3, and go back to main menu.*/
    if  (!isdigit(*arg))
    { send_to_char("Must be a numeric value, try again : ", d->character);
      return;
    }
    switch(OLC_CMD(d).command)
    { case 'E':
        pos = atoi(arg);
        /*. Count number of wear positions (could use NUM_WEARS,
            this is more reliable) .*/
        while(*equipment_types[i] != '\n')
          i++;
        if ((pos < 0) || (pos > i))
          send_to_char("Try again : ", d->character);
        else
        { OLC_CMD(d).arg3 = pos;
          zedit_disp_menu(d);
        }
        break;
      case 'P':
        pos = real_object(atoi(arg));
        if (pos >= 0)
        { OLC_CMD(d).arg3 = pos;
          zedit_disp_menu(d);
        } else
          send_to_char("That object does not exist, try again : ", d->character);
        break;
      case 'D':
        pos = atoi(arg);
        if ((pos < 0) || (pos > 2))
          send_to_char("Try again : ", d->character);
        else
        { OLC_CMD(d).arg3 = pos;
          zedit_disp_menu(d);
        }
        break;
      case 'L':
        pos = atoi(arg);
        if (!OLC_CMD(d).arg2)
          if (pos > 0)
          { OLC_CMD(d).arg3 = pos;
            zedit_disp_menu(d);
          } else
            send_to_char("The loop must repeat at least once, try again : ", d->character);
        else {
          OLC_CMD(d).arg3 = -1;
          zedit_disp_menu(d);
        }
        break;
      case 'R':
        if (OLC_CMD(d).arg2)
          pos = real_object(atoi(arg));
        else
          pos = real_mobile(atoi(arg));
        if (pos >= 0)
        { OLC_CMD(d).arg3 = pos;
          zedit_disp_menu(d);
        } else {
          if (OLC_CMD(d).arg2)
            send_to_char("That object does not exist, try again : ", d->character);
          else
            send_to_char("That mobile does not exist, try again : ", d->character);
        }
        break;
      case 'M':
      case 'O':
      case 'G':
      default:
        /*. We should never get here .*/
        cleanup_olc(d, CLEANUP_ALL);
        mudlog("SYSERR: OLC: zedit_parse(): case ARG3: Ack!", BRF, LVL_BUILDER, TRUE);
        break;
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_NAME:
    /*. Add new name and return to main menu .*/
    FREE(OLC_ZONE(d)->name);
    OLC_ZONE(d)->name = str_dup(arg);
    OLC_ZONE(d)->number = 1;
    zedit_disp_menu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_RESET:
    /*. Parse and add new reset_mode and return to main menu .*/
    pos = atoi(arg);
    if (!isdigit(*arg) || (pos <  0) || (pos > 2))
      send_to_char("Try again (0-2) : ", d->character);
    else
    { OLC_ZONE(d)->reset_mode = pos;
      OLC_ZONE(d)->number = 1;
      zedit_disp_menu(d);
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_LIFE:
    /*. Parse and add new lifespan and return to main menu .*/
    pos = atoi(arg);
    if (!isdigit(*arg) || (pos <  0) || (pos > 240))
      send_to_char("Try again (0-240) : ", d->character);
    else {
      OLC_ZONE(d)->lifespan = pos;
      OLC_ZONE(d)->number = 1;
      zedit_disp_menu(d);
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_TOP:
    /*. Parse and add new top room in zone and return to main menu .*/
    old_top = OLC_ZONE(d)->top;
    if (OLC_ZNUM(d) == top_of_zone_table)
      OLC_ZONE(d)->top = MAX(OLC_ZNUM(d) * 100, MIN(32000, atoi(arg)));
    else
      OLC_ZONE(d)->top =
    MAX(OLC_ZNUM(d) * 100,
        MIN(zone_table[OLC_ZNUM(d) + 1].number * 100 - 1, atoi(arg)));
    if (old_top != OLC_ZONE(d)->top)
      OLC_ZONE(d)->number = 1;
    zedit_disp_menu(d);
    break;

/*-------------------------------------------------------------------*/
  default:
    /*. We should never get here .*/
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: zedit_parse(): Reached default case!",BRF,LVL_BUILDER,TRUE);
    break;
  }
}
/*. End of parse_zedit() .*/
