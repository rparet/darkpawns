/* ************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* CVS: $Id: weather.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"

extern struct time_info_data time_info;

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);


void
weather_and_time(int mode)
{
  another_hour(mode);
  if (mode)
    weather_change();
}


void
another_hour(int mode)
{
  void full_moon(void);
  void lunar_hunter(void);
  void load_night_gate(void);
  void remove_night_gate(void);
  void ghost_ship_appear(void);
  void ghost_ship_disappear(void);
  
  time_info.hours++;

  if (mode)
    {
      switch (time_info.hours)
	{
	case 5:
	  weather_info.sunlight = SUN_RISE;
	  send_to_outdoor("The suns rise in the east and north.\r\n");
          ghost_ship_disappear();
	  remove_night_gate();
	  break;
	case 6:
	  weather_info.sunlight = SUN_LIGHT;
	  send_to_outdoor("The day has begun.\r\n");
	  break;
	case 21:
	  weather_info.sunlight = SUN_SET;
	  send_to_outdoor("The suns slowly disappear in the west and "
		"south.\r\n");
	  ghost_ship_appear();
	  load_night_gate();
	  if (time_info.day+1 <26 && time_info.day+1 >=22)
	  {
	    full_moon();
	    lunar_hunter();
	  }
	  break;
	case 22:
	  weather_info.sunlight = SUN_DARK;
	  send_to_outdoor("The night has begun.\r\n");
	  break;
	default:
	  break;
	}
    }
  if (time_info.hours > 23)
    {	/* Changed by HHS due to bug ??? */
      time_info.hours -= 24;
      time_info.day++;

      switch(time_info.day)
	{
	case 1: time_info.moon = MOON_NEW;
	  break;
	case 6: time_info.moon = MOON_QUARTER_FULL;
	  break;
	case 12: time_info.moon = MOON_HALF_FULL;
	  break;
	case 17: time_info.moon = MOON_THREE_FULL;
	  break;
	case 22: time_info.moon = MOON_FULL;
	  break;
	case 26: time_info.moon = MOON_QUARTER_EMPTY;
	  break;
	case 30: time_info.moon = MOON_HALF_EMPTY;
	  break;
	case 34: time_info.moon = MOON_THREE_EMPTY;
	  break;
	default:
	  break;
	}

      if (time_info.day > 34)
	{
	  time_info.day = 0;
	  time_info.month++;

	  if (time_info.month > 16)
	    {
	      time_info.month = 0;
	      time_info.year++;
	    }
	}
    }
}


void
weather_change(void)
{
  int diff, change;
  if ((time_info.month >= 9) && (time_info.month <= 16))
    diff = (weather_info.pressure > 985 ? -2 : 2);
  else
    diff = (weather_info.pressure > 1015 ? -2 : 2);

  weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);

  weather_info.pressure += weather_info.change;

  weather_info.pressure = MIN(weather_info.pressure, 1040);
  weather_info.pressure = MAX(weather_info.pressure, 960);

  change = 0;

  switch (weather_info.sky)
    {
    case SKY_CLOUDLESS:
      if (weather_info.pressure < 990)
	change = 1;
      else if (weather_info.pressure < 1010)
	if (dice(1, 4) == 1)
	  change = 1;
      break;
    case SKY_CLOUDY:
      if (weather_info.pressure < 970)
	change = 2;
      else if (weather_info.pressure < 990)
	if (dice(1, 4) == 1)
	  change = 2;
	else
	  change = 0;
      else if (weather_info.pressure > 1030)
	if (dice(1, 4) == 1)
	  change = 3;

      break;
    case SKY_RAINING:
      if (weather_info.pressure < 970)
	if (dice(1, 4) == 1)
	  change = 4;
	else
	  change = 0;
      else if (weather_info.pressure > 1030)
	change = 5;
      else if (weather_info.pressure > 1010)
	if (dice(1, 4) == 1)
	  change = 5;

      break;
    case SKY_LIGHTNING:
      if (weather_info.pressure > 1010)
	change = 6;
      else if (weather_info.pressure > 990)
	if (dice(1, 4) == 1)
	  change = 6;

      break;
    default:
      change = 0;
      weather_info.sky = SKY_CLOUDLESS;
      break;
    }

  switch (change)
    {
    case 0:
      break;
    case 1:
      send_to_outdoor("The sky starts to get cloudy.\r\n");
      weather_info.sky = SKY_CLOUDY;
      break;
    case 2:
      send_to_outdoor("It starts to rain.\r\n");
      weather_info.sky = SKY_RAINING;
      break;
    case 3:
      send_to_outdoor("The clouds disappear.\r\n");
      weather_info.sky = SKY_CLOUDLESS;
      break;
    case 4:
      send_to_outdoor("Lightning starts to show in the sky.\r\n");
      weather_info.sky = SKY_LIGHTNING;
      break;
    case 5:
      send_to_outdoor("The rain stops.\r\n");
      weather_info.sky = SKY_CLOUDY;
      break;
    case 6:
      send_to_outdoor("The lightning stops.\r\n");
      weather_info.sky = SKY_RAINING;
      break;
    default:
      break;
    }
}




