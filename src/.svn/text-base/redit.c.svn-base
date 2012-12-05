/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - redit.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*. Original author: Levork .*/

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "olc.h"
#include "constants.h"
#include "improved-edit.h"

/*------------------------------------------------------------------------*/
/*. External data .*/

extern int      top_of_world;
extern struct room_data *world;
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern char    *room_bits[];
extern char    *sector_types[];
extern char    *exit_bits[];
extern struct zone_data *zone_table;
extern int r_mortal_start_room;
extern int r_immort_start_room;
extern int r_frozen_start_room;
extern int mortal_start_room;
extern int immort_start_room;
extern int frozen_start_room;
extern int top_of_zone_table;

/*------------------------------------------------------------------------*/
/* function protos */

void redit_disp_extradesc_menu(struct descriptor_data * d);
void redit_disp_exit_menu(struct descriptor_data * d);
void redit_disp_exit_flag_menu(struct descriptor_data * d);
void redit_disp_flag_menu(struct descriptor_data * d);
void redit_disp_sector_menu(struct descriptor_data * d);
void redit_disp_menu(struct descriptor_data * d);
void redit_parse(struct descriptor_data * d, char *arg);
void redit_setup_new(struct descriptor_data *d);
void redit_setup_existing(struct descriptor_data *d, int real_num);
void redit_save_to_disk(struct descriptor_data *d);
void redit_save_internally(struct descriptor_data *d);
void free_room(struct room_data *room);

/*------------------------------------------------------------------------*/

#define  W_EXIT(room, num) (world[(room)].dir_option[(num)])

/*------------------------------------------------------------------------*\
  Utils and exported functions.
\*------------------------------------------------------------------------*/

void
redit_setup_new(struct descriptor_data *d)
{
  CREATE(OLC_ROOM(d), struct room_data, 1);
  OLC_ROOM(d)->name = str_dup("An unfinished room");
  OLC_ROOM(d)->description = str_dup("You are in an unfinished room.\r\n");
  OLC_ROOM(d)->number = NOWHERE;
  redit_disp_menu(d);
  OLC_VAL(d) = 0;
}

/*------------------------------------------------------------------------*/

void
redit_setup_existing(struct descriptor_data *d, int real_num)
{
  struct room_data *room;
  int counter;
  
  /*. Build a copy of the room .*/
  CREATE (room, struct room_data, 1);
  *room = world[real_num];
  /* allocate space for all strings  */
  if (world[real_num].name)
    room->name = str_dup (world[real_num].name);
  if (world[real_num].description)
    room->description = str_dup (world[real_num].description);

  /* exits - alloc only if necessary */
  for (counter = 0; counter < NUM_OF_DIRS; counter++)
    {
      if (world[real_num].dir_option[counter])
	{
	  CREATE(room->dir_option[counter], struct room_direction_data, 1);
	  /* copy numbers over */
	  *room->dir_option[counter] = *world[real_num].dir_option[counter];
	  /* CREATE strings */
	  if (world[real_num].dir_option[counter]->general_description)
	    room->dir_option[counter]->general_description =
	      str_dup(world[real_num].dir_option[counter]->general_description);
	  if (world[real_num].dir_option[counter]->keyword)
	    room->dir_option[counter]->keyword =
	      str_dup(world[real_num].dir_option[counter]->keyword);
	}
    }
  
  /*. Extra descriptions if necessary .*/ 
  if (world[real_num].ex_description) 
    {
      struct extra_descr_data *this, *temp, *temp2;
      CREATE (temp, struct extra_descr_data, 1);
      room->ex_description = temp;
      for (this = world[real_num].ex_description; this; this = this->next)
	{
	  if (this->keyword)
	    temp->keyword = str_dup (this->keyword);
	  if (this->description)
	    temp->description = str_dup (this->description);
	  if (this->next)
	    {
	      CREATE (temp2, struct extra_descr_data, 1);
	      temp->next = temp2;
	      temp = temp2;
	    }
	  else
	    temp->next = NULL;
	}
    }
 
  /*. Attatch room copy to players descriptor .*/
  OLC_ROOM(d) = room;
  OLC_VAL(d) = 0;
  redit_disp_menu(d);
}

/*------------------------------------------------------------------------*/
      
#define ZCMD (zone_table[zone].cmd[cmd_no])

void
redit_save_internally(struct descriptor_data *d)
{
  int i, j, room_num, found = 0, zone, cmd_no;
  struct room_data *new_world;
  struct char_data *temp_ch;
  struct obj_data *temp_obj;

  room_num = real_room(OLC_NUM(d));
  if (room_num >= 0) 
    {
      /*. Room exits: move contents over then free and replace it .*/
      OLC_ROOM(d)->contents = world[room_num].contents;
      OLC_ROOM(d)->people = world[room_num].people;
      free_room(world + room_num);
      world[room_num] = *OLC_ROOM(d);
    }
  else 
    { /*. Room doesn't exist, hafta add it .*/

      CREATE(new_world, struct room_data, top_of_world + 2);

      /* count thru world tables */
      for (i = 0; i <= top_of_world; i++) 
	{
	  if (!found)
	    {
	      /*. Is this the place? .*/
	      if (world[i].number > OLC_NUM(d)) 
		{
		  found = 1;

		  new_world[i] = *(OLC_ROOM(d));
		  new_world[i].number = OLC_NUM(d);
		  new_world[i].func = NULL;
                  CREATE(new_world[i].script, struct script_data, 1);
                  new_world[i].script->name = NULL;
                  new_world[i].script->lua_functions = 0;
		  room_num  = i;
	
		  /* copy from world to new_world + 1 */
		  new_world[i + 1] = world[i];
		  /* people in this room must have their numbers moved */
		  for (temp_ch = world[i].people; temp_ch;
		       temp_ch = temp_ch->next_in_room)
		    if (temp_ch->in_room != -1)
		      temp_ch->in_room = i + 1;

		  /* move objects */
		  for (temp_obj = world[i].contents; temp_obj;
		       temp_obj = temp_obj->next_content)
		    if (temp_obj->in_room != -1)
		      temp_obj->in_room = i + 1;
		}
	      else 
		{ /*.   Not yet placed, copy straight over .*/
		  new_world[i] = world[i];
		}
	    }
	  else 
	    { /*. Already been found  .*/
 
	      /* people in this room must have their in_rooms moved */
	      for (temp_ch = world[i].people; temp_ch;
		   temp_ch = temp_ch->next_in_room)
		if (temp_ch->in_room != -1)
		  temp_ch->in_room = i + 1;

	      /* move objects */
	      for (temp_obj = world[i].contents; temp_obj;
		   temp_obj = temp_obj->next_content)
		if (temp_obj->in_room != -1)
		  temp_obj->in_room = i + 1;

	      new_world[i + 1] = world[i];
	    }
	}
      if (!found)
	{ /*. Still not found, insert at top of table .*/
	  new_world[i] = *(OLC_ROOM(d));
	  new_world[i].number = OLC_NUM(d);
	  new_world[i].func = NULL;
          CREATE(new_world[i].script, struct script_data, 1);
          new_world[i].script->name = NULL;
          new_world[i].script->lua_functions = 0;
	  room_num  = i;
	}

      /* copy world table over */
      FREE(world);
      world = new_world;
      top_of_world++;

      /*. Update zone table .*/
      for (zone = 0; zone <= top_of_zone_table; zone++)
	for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
	  switch (ZCMD.command)
	    {
	    case 'M':
	    case 'O':
	      if (ZCMD.arg3 >= room_num)
		ZCMD.arg3++;
	      break;
	    case 'D':
	    case 'R':
	      if (ZCMD.arg1 >= room_num)
		ZCMD.arg1++;
	    case 'G':
	    case 'P':
	    case 'E':
	    case '*':
	      break;
	    default:
	      mudlog("SYSERR: OLC: redit_save_internally: Unknown comand",
		     BRF, LVL_BUILDER, TRUE);
	    }

      /* update load rooms, to fix creeping load room problem */
      if (room_num <= r_mortal_start_room)
	r_mortal_start_room++;
      if (room_num <= r_immort_start_room)
	r_immort_start_room++;
      if (room_num <= r_frozen_start_room)
	r_frozen_start_room++;

      /*. Update world exits .*/
      for (i = 0; i < top_of_world + 1; i++)
	for (j = 0; j < NUM_OF_DIRS; j++)
	  if (W_EXIT(i, j))
	    if (W_EXIT(i, j)->to_room >= room_num)
	      W_EXIT(i, j)->to_room++;

    }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ROOM);
}


/*------------------------------------------------------------------------*/

void
redit_save_to_disk(struct descriptor_data *d)
{
  int counter, counter2, realcounter;
  FILE *fp;
  struct room_data *room;
  struct extra_descr_data *ex_desc;

  sprintf(buf, "%s/%d.wld", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);
  if (!(fp = fopen(buf, "w+")))
  {
    mudlog("SYSERR: OLC: Cannot open room file!", BRF, LVL_BUILDER, TRUE);
    return;
  }

  for (counter = zone_table[OLC_ZNUM(d)].number * 100;
       counter <= zone_table[OLC_ZNUM(d)].top;
       counter++) 
  {
    realcounter = real_room(counter);
    if (realcounter >= 0) 
    { 
      room = (world + realcounter);

      /*. Remove the '\r\n' sequences from description .*/
      strcpy(buf1, room->description ? room->description : "Empty");
      strip_string(buf1);

      /*. Build a buffer ready to save .*/
      sprintf(buf, "#%d\n", counter);
      sprintf(buf, "%s%s~\n", buf,
              room->name ? room->name : "undefined");
      sprintf(buf, "%s%s~\n", buf, buf1);
      sprintf(buf, "%s%d ", buf, zone_table[OLC_ZNUM(d)].number);
      sprintf(buf, "%s%d %d %d %d ", buf,
	      room->room_flags[0],
	      room->room_flags[1],
	      room->room_flags[2],
	      room->room_flags[3]);
      sprintf(buf, "%s%d\n", buf,
              room->sector_type);
      /*. Save this section .*/
      fputs(buf, fp);

      /*. Handle scripts .*/
      if (GET_ROOM_SCRIPT(realcounter) && GET_ROOM_SCRIPT(realcounter)->name) {
        sprintf(buf, "R %s %d\n",
                GET_ROOM_SCRIPT(realcounter)->name, ROOM_SCRIPT_FLAGS(realcounter));
        fputs(buf, fp);
      }

      /*. Handle exits .*/
      for (counter2 = 0; counter2 < NUM_OF_DIRS; counter2++) 
      {
	if (room->dir_option[counter2]) 
        {
	  int temp_door_flag;

          /*. Again, strip out the crap .*/
          if (room->dir_option[counter2]->general_description)
          {
	    strcpy(buf1, room->dir_option[counter2]->general_description);
            strip_string(buf1);
          }
	  else
	    *buf1 = 0;

          /*. Figure out door flag .*/
          if (IS_SET(room->dir_option[counter2]->exit_info, EX_ISDOOR)) 
          {
	    if (IS_SET(room->dir_option[counter2]->exit_info, EX_PICKPROOF))
	      temp_door_flag = 2;
	    else
	      temp_door_flag = 1;
	  }
	  else
	    temp_door_flag = 0;

          /*. Check for keywords .*/
          if(room->dir_option[counter2]->keyword)
            strcpy(buf2, room->dir_option[counter2]->keyword);
          else
            *buf2 = 0;
               
          /*. Ok, now build a buffer to output to file .*/
          sprintf(buf, "D%d\n%s~\n%s~\n%d %d %d\n",
		  counter2, buf1, buf2, temp_door_flag,
		  room->dir_option[counter2]->key,
		  world[room->dir_option[counter2]->to_room].number
          );
          /*. Save this door .*/
	  fputs(buf, fp);
        }
      }
      if (room->ex_description) 
      {
	for (ex_desc = room->ex_description; ex_desc; ex_desc = ex_desc->next) 
        {
	  /*. Home straight, just deal with extras descriptions..*/
          strcpy(buf1, ex_desc->description);
          strip_string(buf1);
          sprintf(buf, "E\n%s~\n%s~\n", ex_desc->keyword,buf1);
          fputs(buf, fp);
	}
      }
      fprintf(fp, "S\n");
    }
  }
  /* write final line and close */
  fprintf(fp, "$~\n");
  fclose(fp);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ROOM);
}

/*------------------------------------------------------------------------*/

void
free_room(struct room_data *room)
{
  int i;
  struct extra_descr_data *this, *next;

  if (room->name)
    FREE(room->name);
  if (room->description)
    FREE(room->description);

  /*. Free exits .*/
  for (i = 0; i < NUM_OF_DIRS; i++)
  {
    if (room->dir_option[i])
    {
      if (room->dir_option[i]->general_description)
        FREE(room->dir_option[i]->general_description);
      if (room->dir_option[i]->keyword)
        FREE(room->dir_option[i]->keyword);
    }
    FREE(room->dir_option[i]);
  }

  /*. Free extra descriptions .*/
  for (this = room->ex_description; this; this = next)
  {
    next = this->next;
    if (this->keyword)
      FREE(this->keyword);
    if (this->description)
      FREE(this->description);
    FREE(this);
  }
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/* For extra descriptions */
void
redit_disp_extradesc_menu(struct descriptor_data * d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);
  
  sprintf(buf, "\r\n"
	  "%s1%s) Keyword: %s%s\r\n"
	  "%s2%s) Description:\r\n%s%s\r\n"
	  "%s3%s) Goto next description: ",
	  grn, nrm, yel,
	  extra_desc->keyword ? extra_desc->keyword : "<NONE>",
	  grn, nrm, yel,
	  extra_desc->description ?  extra_desc->description : "<NONE>",
	  grn, nrm);
  
  if (!extra_desc->next)
    strcat(buf, "<NOT SET>\r\n");
  else
    strcat(buf, "Set.\r\n");
  strcat(buf, "Enter choice (0 to quit) : ");
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_EXTRADESC_MENU;
}

/* For exits */
void
redit_disp_exit_menu(struct descriptor_data * d)
{
  /* if exit doesn't exist, alloc/create it */
  if(!OLC_EXIT(d))
    CREATE(OLC_EXIT(d), struct room_direction_data, 1);

  /* weird door handling! */
  if (IS_SET(OLC_EXIT(d)->exit_info, EX_ISDOOR)) {
    if (IS_SET(OLC_EXIT(d)->exit_info, EX_PICKPROOF))
      strcpy(buf2, "Pickproof");
    else
      strcpy(buf2, "Is a door");
  } else
    strcpy(buf2, "No door");

  get_char_cols(d->character);
  sprintf(buf, "\r\n"
	"%s1%s) Exit to     : %s%d\r\n"
	"%s2%s) Description :-\r\n%s%s\r\n"
  	"%s3%s) Door name   : %s%s\r\n"
  	"%s4%s) Key         : %s%d\r\n"
  	"%s5%s) Door flags  : %s%s\r\n"
  	"%s6%s) Purge exit.\r\n"
	"Enter choice, 0 to quit : ",

        grn, nrm, cyn, OLC_EXIT(d)->to_room != -1 ? world[OLC_EXIT(d)->to_room].number : -1,
        grn, nrm, yel, OLC_EXIT(d)->general_description ? OLC_EXIT(d)->general_description : "<NONE>",
        grn, nrm, yel, OLC_EXIT(d)->keyword ? OLC_EXIT(d)->keyword : "<NONE>",
        grn, nrm, cyn, OLC_EXIT(d)->key,
        grn, nrm, cyn, buf2, grn, nrm
        );

  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_EXIT_MENU;
}

/* For exit flags */
void
redit_disp_exit_flag_menu(struct descriptor_data * d)
{
  get_char_cols(d->character);
  sprintf(buf,  "%s0%s) No door\r\n"
	  "%s1%s) Closeable door\r\n"
	  "%s2%s) Pickproof\r\n"
	  "Enter choice : ",
	  grn, nrm, grn, nrm, grn, nrm );
  send_to_char(buf, d->character);
}

/* For room flags */
void
redit_disp_flag_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
  send_to_char("\r\n", d->character);
  for (counter = 0; counter < NUM_ROOM_FLAGS; counter++) 
    {
      sprintf(buf, "%s%2d%s) %-20.20s ",
	      grn, counter + 1, nrm, room_bits[counter]);
      if(!(++columns % 2))
	strcat(buf, "\r\n");
      send_to_char(buf, d->character);
    }
  sprintbitarray(OLC_ROOM(d)->room_flags, room_bits, RF_ARRAY_MAX, buf1);
  sprintf(buf, 
	  "\r\nRoom flags: %s%s%s\r\n"
	  "Enter room flags, 0 to quit : ",
	  cyn, buf1, nrm
	  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_FLAGS;
}

/* for sector type */
void redit_disp_sector_menu(struct descriptor_data * d)
{
  int counter, columns = 0;

  send_to_char("\r\n", d->character);
  for (counter = 0; counter < NUM_ROOM_SECTORS; counter++)
    {
      sprintf(buf, "%s%2d%s) %-20.20s ",
	      grn, counter, nrm, sector_types[counter]);
      if(!(++columns % 2))
	strcat(buf, "\r\n");
      send_to_char(buf, d->character);
    }
  send_to_char("\r\nEnter sector type : ", d->character);
  OLC_MODE(d) = REDIT_SECTOR;
}

/* Display script flags. */
void redit_disp_script_flags(struct descriptor_data *d)
{
  int i, columns = 0;
  int rnum = real_room(OLC_ROOM(d)->number);

  get_char_cols(d->character);
  send_to_char("\033[H\033[J", d->character);
  for (i = 0; i < NUM_RSCRIPT_FLAGS; i++)
  {  sprintf(buf, "%s%2d%s) %-20.20s  ",
        grn, i+1, nrm, rscript_bits[i]
     );
     if(!(++columns % 2))
       strcat(buf, "\r\n");
     send_to_char(buf, d->character);
  }
  sprintbit(ROOM_SCRIPT_FLAGS(rnum), rscript_bits, buf1);
  sprintf(buf, "\r\n"
        "Current flags   : %s%s%s\r\n"
        "Enter script flags (0 to quit) : ",
        cyn, buf1, nrm
  );
  send_to_char(buf, d->character);
}

/* Display script menu. */
void redit_disp_script_menu(struct descriptor_data *d)
{
  if (OLC_ROOM(d)->number == NOWHERE) {
    send_to_char("\r\nCannot assign a script until the room is saved at least "
		 "once.\r\n",
 		 d->character);
    redit_disp_menu(d);
    return;
  }

  int rnum = real_room(OLC_ROOM(d)->number);

  get_char_cols(d->character);
  
  send_to_char("\r\n", d->character);
  sprintbit(ROOM_SCRIPT_FLAGS(rnum), rscript_bits, buf1);
  sprintf(buf, "%s1%s) Name: %s%s\r\n"
               "%s2%s) Script Flags: %s%s%s\r\n",
          grn, nrm, yel,
          GET_ROOM_SCRIPT(rnum)->name ? GET_ROOM_SCRIPT(rnum)->name : "None",
          grn, nrm, yel, buf1, nrm);
        
  strcat(buf, "Enter choice (0 to quit) : ");
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_SCRIPT_MENU;
}

/* the main menu */
void redit_disp_menu(struct descriptor_data * d)
{ struct room_data *room;

  get_char_cols(d->character);
  room = OLC_ROOM(d);

  sprintbitarray(room->room_flags, room_bits, RF_ARRAY_MAX, buf1);
  sprinttype(room->sector_type, sector_types, buf2);
  snprintf(buf, MAX_STRING_LENGTH,
  	"\r\n"
	"-- Room number : [%s%d%s]  	Room zone: [%s%d%s]\r\n"
	"%s1%s) Name        : %s%s\r\n"
	"%s2%s) Description :\r\n%s%s"
  	"%s3%s) Room flags  : %s%s\r\n"
	"%s4%s) Sector type : %s%s\r\n"
  	"%s5%s) Exit north  : %s%d\r\n"
  	"%s6%s) Exit east   : %s%d\r\n"
  	"%s7%s) Exit south  : %s%d\r\n"
  	"%s8%s) Exit west   : %s%d\r\n"
  	"%s9%s) Exit up     : %s%d\r\n"
  	"%sA%s) Exit down   : %s%d\r\n"
  	"%sB%s) Extra descriptions menu\r\n"
	"%sC%s) Copy another room description\r\n"
        "%sS%s) Script menu\r\n"
  	"%sQ%s) Quit\r\n"
  	"Enter choice : ",

	cyn, OLC_NUM(d), nrm,
	cyn, zone_table[OLC_ZNUM(d)].number, nrm,
	grn, nrm, yel, room->name,
	grn, nrm, yel, room->description,
	grn, nrm, cyn, buf1,
	grn, nrm, cyn, buf2,
  	grn, nrm, cyn, room->dir_option[NORTH] ?
          world[room->dir_option[NORTH]->to_room].number : -1,
	grn, nrm, cyn, room->dir_option[EAST] ?
          world[room->dir_option[EAST]->to_room].number : -1,
  	grn, nrm, cyn, room->dir_option[SOUTH] ? 
          world[room->dir_option[SOUTH]->to_room].number : -1,
  	grn, nrm, cyn, room->dir_option[WEST] ? 
          world[room->dir_option[WEST]->to_room].number : -1,
  	grn, nrm, cyn, room->dir_option[UP] ? 
          world[room->dir_option[UP]->to_room].number : -1,
  	grn, nrm, cyn, room->dir_option[DOWN] ? 
          world[room->dir_option[DOWN]->to_room].number : -1,
        grn, nrm,
        grn, nrm,
        grn, nrm,
        grn, nrm
  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = REDIT_MAIN_MENU;
}



/**************************************************************************
  The main loop
 **************************************************************************/

void redit_parse(struct descriptor_data * d, char *arg)
{ 
  extern struct room_data *world;
  int number;
  char *oldtext = NULL;

  switch (OLC_MODE(d)) {
  case REDIT_CONFIRM_SAVESTRING:
    switch (*arg) {
    case 'y':
    case 'Y':
      redit_save_internally(d);
      sprintf(buf, "OLC: %s edits room %d", GET_NAME(d->character), OLC_NUM(d));
      mudlog(buf, CMP, LVL_BUILDER, TRUE);
      /*. Do NOT free strings! just the room structure .*/
      cleanup_olc(d, CLEANUP_STRUCTS);
      send_to_char("Room saved to memory.\r\n", d->character);
      break;
    case 'n':
    case 'N':
      /* free everything up, including strings etc */
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save this room internally? : ", d->character);
      break;
    }
    return;

  case REDIT_MAIN_MENU:
    switch (*arg) {
    case 'q':
    case 'Q':
      if (OLC_VAL(d))
      { /*. Something has been modified .*/
        send_to_char("Do you wish to save this room internally? : ", d->character);
        OLC_MODE(d) = REDIT_CONFIRM_SAVESTRING;
      } else
        cleanup_olc(d, CLEANUP_ALL);
      return;
    case '1':
      send_to_char("Enter room name:-\r\n| ", d->character);
      OLC_MODE(d) = REDIT_NAME;
      break;
    case '2':
      OLC_MODE(d) = REDIT_DESC;
      send_editor_help(d);
      write_to_output(d, "Enter room description:\r\n\r\n");
      if (OLC_ROOM(d)->description) {
	write_to_output(d, "%s", OLC_ROOM(d)->description);
	oldtext = strdup(OLC_ROOM(d)->description);
      }
      string_write(d, &OLC_ROOM(d)->description, MAX_ROOM_DESC, 0, oldtext);
      OLC_VAL(d) = 1;
      break;
    case '3':
      redit_disp_flag_menu(d);
      break;
    case '4':
      redit_disp_sector_menu(d);
      break;
    case '5':
      OLC_VAL(d) = NORTH;
      redit_disp_exit_menu(d);
      break;
    case '6':
      OLC_VAL(d) = EAST;
      redit_disp_exit_menu(d);
      break;
    case '7':
      OLC_VAL(d) = SOUTH;
      redit_disp_exit_menu(d);
      break;
    case '8':
      OLC_VAL(d) = WEST;
      redit_disp_exit_menu(d);
      break;
    case '9':
      OLC_VAL(d) = UP;
      redit_disp_exit_menu(d);
      break;
    case 'a':
    case 'A':
      OLC_VAL(d) = DOWN;
      redit_disp_exit_menu(d);
      break;
    case 'b':
    case 'B':
      /* if extra desc doesn't exist . */
      if (!OLC_ROOM(d)->ex_description) {
	CREATE(OLC_ROOM(d)->ex_description, struct extra_descr_data, 1);
	OLC_ROOM(d)->ex_description->next = NULL;
      }
      OLC_DESC(d) = OLC_ROOM(d)->ex_description;
      redit_disp_extradesc_menu(d);
      break;
    case 'c':
    case 'C':
      OLC_MODE(d) = REDIT_COPY;
      send_to_char("Enter virtual number of room to copy : ", d->character);
      break;
    case 's':
    case 'S':
      OLC_MODE(d) = REDIT_SCRIPT_MENU;
      redit_disp_script_menu(d);
      break;
    default:
      send_to_char("Invalid choice!", d->character);
      redit_disp_menu(d);
      break;
    }
    return;

  case REDIT_NAME:
    if (OLC_ROOM(d)->name)
      FREE(OLC_ROOM(d)->name);
    if (strlen(arg) > MAX_ROOM_NAME)
      arg[MAX_ROOM_NAME -1] = 0;
    OLC_ROOM(d)->name = str_dup(arg);
    break;
  case REDIT_DESC:
    /* we will NEVER get here */
    mudlog("SYSERR: Reached REDIT_DESC case in parse_redit",BRF,LVL_BUILDER,TRUE);
    break;

  case REDIT_FLAGS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ROOM_FLAGS)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      redit_disp_flag_menu(d);
    } else {
      if (number == 0)
        break;
      else {
	/* toggle bits */
	if (IS_SET_AR(OLC_ROOM(d)->room_flags, (number - 1)))
	  REMOVE_BIT_AR(OLC_ROOM(d)->room_flags, (number - 1));
	else
	  SET_BIT_AR(OLC_ROOM(d)->room_flags, (number - 1));
	redit_disp_flag_menu(d);
      }
    }
    return;

  case REDIT_SECTOR:
    number = atoi(arg);
    if (number < 0 || number >= NUM_ROOM_SECTORS) {
      send_to_char("Invalid choice!", d->character);
      redit_disp_sector_menu(d);
      return;
    } else 
      OLC_ROOM(d)->sector_type = number;
    break;

  case REDIT_EXIT_MENU:
    switch (*arg) {
    case '0':
      break;
    case '1':
      OLC_MODE(d) = REDIT_EXIT_NUMBER;
      send_to_char("Exit to room number : ", d->character);
      return;
    case '2':
      OLC_MODE(d) = REDIT_EXIT_DESCRIPTION;
      send_editor_help(d);
      write_to_output(d, "Enter exit description:\r\n\r\n");
      if (OLC_EXIT(d)->general_description) {
	write_to_output(d, "%s", OLC_EXIT(d)->general_description);
	oldtext = strdup(OLC_EXIT(d)->general_description);
      }
      string_write(d, &OLC_EXIT(d)->general_description, MAX_EXIT_DESC, 0, oldtext);
      return;

    case '3':
      OLC_MODE(d) = REDIT_EXIT_KEYWORD;
      send_to_char("Enter keywords : ", d->character);
      return;
    case '4':
      OLC_MODE(d) = REDIT_EXIT_KEY;
      send_to_char("Enter key number : ", d->character);
      return;
    case '5':
      redit_disp_exit_flag_menu(d);
      OLC_MODE(d) = REDIT_EXIT_DOORFLAGS;
      return;
    case '6':
      /* delete exit */
      if (OLC_EXIT(d)->keyword)
	FREE(OLC_EXIT(d)->keyword);
      if (OLC_EXIT(d)->general_description)
	FREE(OLC_EXIT(d)->general_description);
      FREE(OLC_EXIT(d));
      OLC_EXIT(d) = NULL;
      break;
    default:
      send_to_char("Try again : ", d->character);
      return;
    }
    break;

  case REDIT_EXIT_NUMBER:
    number = (atoi(arg));
    if (number != -1)
    { number = real_room(number);
      if (number < 0)
      { send_to_char("That room does not exist, try again : ", d->character);
        return;
      }
    }
    OLC_EXIT(d)->to_room = number;
    redit_disp_exit_menu(d);
    return;

  case REDIT_EXIT_DESCRIPTION:
    /* we should NEVER get here */
    mudlog("SYSERR: Reached REDIT_EXIT_DESC case in parse_redit",BRF,LVL_BUILDER,TRUE);
    break;

  case REDIT_EXIT_KEYWORD:
    if (OLC_EXIT(d)->keyword)
      FREE(OLC_EXIT(d)->keyword);
    OLC_EXIT(d)->keyword = str_dup(arg);
    redit_disp_exit_menu(d);
    return;

  case REDIT_EXIT_KEY:
    number = atoi(arg);
    OLC_EXIT(d)->key = number;
    redit_disp_exit_menu(d);
    return;

  case REDIT_EXIT_DOORFLAGS:
    number = atoi(arg);
    if ((number < 0) || (number > 2)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      redit_disp_exit_flag_menu(d);
    } else {
      /* doors are a bit idiotic, don't you think? :) */
      if (number == 0)
	OLC_EXIT(d)->exit_info = 0;
      else if (number == 1)
	OLC_EXIT(d)->exit_info = EX_ISDOOR;
      else if (number == 2)
	OLC_EXIT(d)->exit_info = EX_ISDOOR | EX_PICKPROOF;
      /* jump out to menu */
      redit_disp_exit_menu(d);
    }
    return;

  case REDIT_EXTRADESC_KEY:
    OLC_DESC(d)->keyword = str_dup(arg);
    redit_disp_extradesc_menu(d);
    return;

  case REDIT_EXTRADESC_MENU:
    number = atoi(arg);
    switch (number) {
    case 0:
      {
	/* if something got left out, delete the extra desc
	 when backing out to menu */
	if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) 
        { struct extra_descr_data **tmp_desc;

	  if (OLC_DESC(d)->keyword)
	    FREE(OLC_DESC(d)->keyword);
	  if (OLC_DESC(d)->description)
	    FREE(OLC_DESC(d)->description);

	  /*. Clean up pointers .*/
	  for(tmp_desc = &(OLC_ROOM(d)->ex_description); *tmp_desc;
	      tmp_desc = &((*tmp_desc)->next))
          { if(*tmp_desc == OLC_DESC(d))
	    { *tmp_desc = NULL;
              break;
	    }
	  }
	  FREE(OLC_DESC(d));
	}
      }
      break;
    case 1:
      OLC_MODE(d) = REDIT_EXTRADESC_KEY;
      send_to_char("Enter keywords, separated by spaces : ", d->character);
      return;
    case 2:
      OLC_MODE(d) = REDIT_EXTRADESC_DESCRIPTION;
      send_editor_help(d);
      write_to_output(d, "Enter extra description:\r\n\r\n");
      if (OLC_DESC(d)->description) {
	write_to_output(d, "%s", OLC_DESC(d)->description);
	oldtext = strdup(OLC_DESC(d)->description);
      }
      string_write(d, &OLC_DESC(d)->description, MAX_MESSAGE_LENGTH, 0, oldtext);
      return;

    case 3:
      if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	send_to_char("You can't edit the next extra desc without completing this one.\r\n", d->character);
	redit_disp_extradesc_menu(d);
      } else {
	struct extra_descr_data *new_extra;

	if (OLC_DESC(d)->next)
	  OLC_DESC(d) = OLC_DESC(d)->next;
	else {
	  /* make new extra, attach at end */
	  CREATE(new_extra, struct extra_descr_data, 1);
	  OLC_DESC(d)->next = new_extra;
	  OLC_DESC(d) = new_extra;
	}
	redit_disp_extradesc_menu(d);
      }
      return;
    }
    break;

  case REDIT_COPY:
    {
      int i = atoi(arg);
      if (i != -1)
	{
	  i = real_room(i);
	  if (i < 0)
	    {
	      send_to_char("That room does not exist, try again : ",
			   d->character);
	      return;
	    }
	  else
	    {
	      if (world[i].name)
		OLC_ROOM(d)->name = str_dup(world[i].name);
	      if (world[i].description)
		OLC_ROOM(d)->description = str_dup(world[i].description);
	    }
	}
    }
    break;

  case REDIT_SCRIPT_MENU:
    number = atoi(arg);
    switch (number) {
      case 0:
        break;
      case 1:
        OLC_MODE(d) = REDIT_SCRIPT_NAME;
        send_to_char("Enter script name: ", d->character);
        return;
      case 2:
        OLC_MODE(d) = REDIT_SCRIPT_FLAGS; 
        redit_disp_script_flags(d);
        return;
      default:
        break;
    }    
    break;

  case REDIT_SCRIPT_NAME:
    number = real_room(OLC_ROOM(d)->number);
    if (!GET_ROOM_SCRIPT(number))
      CREATE(GET_ROOM_SCRIPT(number), struct script_data, 1);
    if (!strcmp(arg, ""))
      GET_ROOM_SCRIPT(number)->name = NULL;
    else
      GET_ROOM_SCRIPT(number)->name = str_dup(arg);
    redit_disp_script_menu(d);
    return;
            
  case REDIT_SCRIPT_FLAGS:
    if ((number = atoi(arg)) == 0)
    {
      OLC_MODE(d) = REDIT_SCRIPT_MENU;
      redit_disp_script_menu(d);
      return; 
    }
    if (!((number < 0) || (number > NUM_RSCRIPT_FLAGS)))
      TOGGLE_BIT(ROOM_SCRIPT_FLAGS(real_room(OLC_ROOM(d)->number)), 1 << (number - 1));
    redit_disp_script_flags(d);
    return; 

  default:
    /* we should never get here */
    mudlog("SYSERR: Reached default case in parse_redit",BRF,LVL_BUILDER,TRUE);
    break;
  }
  /*. If we get this far, something has be changed .*/
  OLC_VAL(d) = 1;
  redit_disp_menu(d);
}

void redit_string_cleanup(struct descriptor_data *d, int action)
{
  switch(OLC_MODE(d)) {
  case REDIT_DESC:
    redit_disp_menu(d);
    break;
  case REDIT_EXTRADESC_DESCRIPTION:
    redit_disp_extradesc_menu(d);
    break;
  case REDIT_EXIT_DESCRIPTION:
    redit_disp_exit_menu(d);
    break;
  }
}
