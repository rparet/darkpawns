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

/* $Id: mapcode.c 1487 2008-05-22 01:36:10Z jravn $ */

/*
   Jeff Jahr Sun Apr 17 18:20:29 EDT 1994
   --Functions to display a map of the world to a char.--
   map(), do_map()
   The mapper only maps in zone, unless the 1st char of the argument
   is an 'a', in which case it maps beyone zone borders.

   m == at least 1 mob in room
   p == at least 1 player in room
   M == mob entered a room with a player
   P == player entered a room with a mob
   F == Either a fighting mob or a fighting player was found
   before both a player and a mob were found.
   # == sectortype of the room.
   o == a linkback, or a nowhere exit.
   * == a non-euclidian mapping glitch.

   do_map should be called by the interpreter.  do_map makes the
   initial call to map(), which then recursively calls itself while
   drawing an encoded map onto the array display[][].
   When control is returned to do_map(), it decodes the information
   stored in display[][], checks the mapped rooms for the presance
   of people, then outputs the information line by line to the
   character.

   The JJ_MAPMAX? #defines determine the size of the map produced,
   and how many non-Euclidan overlap glitches the fucntion will
   allow before it stops trying to map them. (JJ_MAPMAXO)

*/
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"

#define JJ_MAPMAXX 76
#define JJ_MAPMAXY 25
#define JJ_MAPMAXO 1

int display[JJ_MAPMAXX+2][JJ_MAPMAXY+2];    /*a drawing buffer for the mapper.*/

void map(room_num thisroom, int x, int y, int overlap,int dontleavezone, struct char_data *ch)
{
  int dir;
  int nextroom;

  const int offx[4] = {0,1,0,-1};
  const int offy[4] = {-1,0,1,0};
  const int link[4] = {-2,-3,-2,-3};

  if ((x<1)||(y<1)||(x>(JJ_MAPMAXX-2))||(y>(JJ_MAPMAXY-2))) /*check bounds*/
    return;

  if (display[x][y] == thisroom) /*already mapped this room.*/
    return;

  if(display[x][y] != 0)
  {                 /*there is already a room drawn here, and it aint me!*/
    switch (dontleavezone)
    {
     case 1:
       display[x][y] = -1;
       break;
     case 2:
       display[x][y] = thisroom; /* overlaps */
       break;
     case 3:
       display[x][y] = display[x][y]; /*underlaps */
       break;
     default:
       display[x][y] = -1;
       break;
    }
    overlap++;
  } else {
    display[x][y] = thisroom;
    overlap = 0;
  }

  if (overlap >= JJ_MAPMAXO)
    return;

  /* draw in the up and down exits, if needed.*/

  if (( world[thisroom].dir_option[DOWN] ))
    display[x+1][y-1] = -7;

  if((world[thisroom].dir_option[UP]))
    display[x-1][y+1] = -7;

  /*Do the following for all of the other exits.*/
  for(dir=0;(dir<4);dir++) {
    if(world[thisroom].dir_option[dir]) { /*if there is a door in this dir*/
      nextroom = world[thisroom].dir_option[dir]->to_room;

      if (nextroom > -2) {  /*if there is an exit...*/
    if((nextroom == thisroom)||(nextroom == -1))
      display[x+offx[dir]][y+offy[dir]] = -4; /*it is a linkback.*/
    else {
      if(nextroom > 0) {
        display[x+offx[dir]][y+offy[dir]] = link[dir];
        if(!dontleavezone || (world[thisroom].zone == world[nextroom].zone))
          map(nextroom,(x+3*offx[dir]),(y+3*offy[dir]),overlap,dontleavezone, ch);
      }
    }
      } else
    display[x+offx[dir]][y+offy[dir]] = -5; /*link to bad or nonexistant room.*/
    }
  }

  if((overlap)&&(display[x+offx[dir]][y+offy[dir]] != 0))
    display[x+offx[dir]][y+offy[dir]] = -6; /*overlapped link.*/


} /* end of map.*/

void do_map(struct char_data *ch, char *argument, int cmd)
{
  int thisroom;
  char line[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  const char *graph[9] = {"&RX&n","/","+","?","&Do&n","&D-&n","&D|&n","&D*&n"," "};
  char *sect_icons[17] = {"&g0&n", "&m#&n", "&Y:&n", "&G+&n", "&w%&n", "&y^&n", "&B~&n", "&b~&n",
                          "&b_&n", "&C@&n", "&r$&n", "&RF&n", "&YE&n", "&CW&n", "&cw&n", "&D`&n",
                          "&WI&n"};
  int x,y,i;

  if (GET_LEVEL(ch)<LVL_IMMORT)
  {
    stc("Type HELP MAP to see a map of town.\r\n", ch);
    return;
  }

  /* initalize the map */
  for (y = 0; y < JJ_MAPMAXY; y++)
  {
      for(x = 0; x < JJ_MAPMAXX; x++)
    display[x][y] = 0;
  }

  x = JJ_MAPMAXX / 2;
  y = JJ_MAPMAXY / 2;
  thisroom = ch->in_room;

  if (!*argument)
    i = 1;
  else if (argument[1] == 'a')
    i = 0;
  else if (argument[1] == 'b')
    i = 2;
  else if (argument[1] == 'c')
    i = 3;
  else
    i = 1;  /*dont leave the zone.*/

  map(thisroom,x,y,0,i, ch);

  display[x][y] = -8;

  send_to_char("You look down upon the world and see...\r\n", ch);

  for(y=0;y<JJ_MAPMAXY;y++)
  {
    line[0] = '\0';  /*clear the line*/
    for(x = 0; x < JJ_MAPMAXX; x++)
    {
      if(display[x][y] > 0)
      {
    strcat(line, sect_icons[world[display[x][y]].sector_type]);
      }
      else
      {
      strcat(line, graph[ (display[x][y]) +8 ]);
      }
    }
    strcat(line, "\r\n");
    send_to_char(line, ch);
  }
  sprintf(buf, "\r\nKEY: inside = %s, city = %s, field = %s, forest = %s, hills = %s,\r\n"
          "     mountain = %s, swim water = %s, noswim water = %s, underwater = %s,\r\n"
          "     flying = %s, desert = %s, fire = %s, earth = %s, wind = %s,\r\n"
          "     water = %s, swamp = %s\r\n"
          "     &RX&n = You are here. &D*&n = overlapping room(s)\r\n",
          sect_icons[SECT_INSIDE],
          sect_icons[SECT_CITY],
          sect_icons[SECT_FIELD],
          sect_icons[SECT_FOREST],
          sect_icons[SECT_HILLS],
          sect_icons[SECT_MOUNTAIN],
          sect_icons[SECT_WATER_SWIM],
          sect_icons[SECT_WATER_NOSWIM],
          sect_icons[SECT_UNDERWATER],
          sect_icons[SECT_FLYING],
          sect_icons[SECT_DESERT],
          sect_icons[SECT_FIRE],
          sect_icons[SECT_EARTH],
          sect_icons[SECT_WIND],
          sect_icons[SECT_WATER],
          sect_icons[SECT_SWAMP]);
  stc(buf, ch);
}

