/************************************************************************\
* File: whod.c, WHO Daemon.                          Adapted for DIKUMUD *
* Version: 1.01								 *
* Usage: Set up port to answer external WHO-calls to the MUD             *
* Copyright (c) 1992, d90-jkr@nada.kth.se (Johan Krisar),		 *
*		      robert@diku.dk (Robert Martin-Legene),             *
*                           DikuMud is a copyright (c) of the Diku Group *
\************************************************************************/
 
/* CVS: $Id: whod.c 1487 2008-05-22 01:36:10Z jravn $ */
 
/************************************************************************\
*                                                                        *
*      Opens a port (the port # above the port # the game itself is	 *
*      being run). On this port people can connect to see who's on.	 *
*	 The player wont have to enter the game to see if it's worth	 *
*      playing at the time, thus saving money.				 *
*									 *
*	 We hope you can use it. If you have problems/questions,	 *
*      ask either of us: d90-jkr@nada.kth.se or robert@diku.dk		 *
*									 *
*									 *
*      Change the following #define-statements to adjust the             *
*      WHOD to your server.                                              */
 
/* *** The following statement sets the name of the MUD in WHOD      *** */
#define MUDNAME "Dark Pawns"
 
/* *** The following statement indicates the WHOD default mode       *** */
#define DEFAULT_MODE (SHOW_NAME | SHOW_TITLE | SHOW_ON | SHOW_CLASS | SHOW_LEVEL)
 
/* *** The following statement tells if a character is WIZ_INVIS     *** */
#define IS_INVIS(ch) (IS_AFFECTED(ch, AFF_INVISIBLE)||GET_INVIS_LEV(ch)||ROOM_FLAGGED(ch->in_room, ROOM_NO_WHO_ROOM))
 
/* *** Specify the lowest wizard level                               *** */
#define WIZ_MIN_LEVEL LVL_IMMORT
 
/* *** The following statement will send a message to the system log *** */
#define LOG(msg) mudlog(msg,BRF,LVL_GOD,TRUE)
 
/* *** Time to wait for disconnection (in seconds)                   *** */
#define WHOD_DELAY_TIME 3
 
/* *** Show linkdead people on the list as well ?  1=yes, 0=no       *** */
#define DISPLAY_LINKDEAD 0
 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
/*
#ifdef _AIX
#include "aix_hdrs.h"
#define _XOPEN_EXTENDED_SOURCE
#include <netdb.h>
#undef _XOPEN_EXTENDED_SOURCE
#include <time.h>  
#include <sys/select.h>
#include <strings.h>
#else
*/
#include <netdb.h>
/*
#endif
*/
#include <unistd.h>

#include "config.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "whod.h"
#include "screen.h"
 
#define WHOD_OPENING 1
#define WHOD_OPEN    2
#define WHOD_DELAY   3
#define WHOD_END     4
#define WHOD_CLOSED  5
#define WHOD_CLOSING 6
 
#define SHOW_NAME	1<<0
#define SHOW_CLASS	1<<1
#define SHOW_LEVEL	1<<2
#define SHOW_TITLE	1<<3
#define SHOW_INVIS	1<<4
#define SHOW_SITE	1<<5
#define SHOW_WIZLEVEL	1<<6
#define SHOW_ON		1<<7
#define SHOW_OFF	1<<8
 
#define WRITE(d,msg) if((write((d),(msg),strlen(msg)))<0){\
                            perror("whod.c - write");}
 
/* *** External functions *** */
int init_socket(int port);
 
/* *** External variables *** */
extern struct char_data *character_list;
extern char *class_abbrevs[];
extern char *SHORT_GREETINGS;
extern struct room_data *world;          
 
/* *** Internal variables *** */
static long   disconnect_time;
static int    s;
static int    whod_mode = DEFAULT_MODE;
static int    state;
static int    whod_port;

int i ; 

int old_search_block(char *argument, int begin, int length, char **list, 
                     int mode);

/*       ------ WHOD interface for use inside of the game ------        */
 
/* Function   : do_whod
 * Parameters : doer, argument string, number of WHOD command (not used)
 * Returns    : --
 * Description: MUD command to set the mode of the WHOD-connection according
 *              to the command string.                                  */
void
do_whod (struct char_data *ch, char *arg, int cmd)
{
  char buf [256];
  char tmp [MAX_INPUT_LENGTH];
  int  bit;
 
  static char *modes[] =
  {
    "name",
    "class",
    "level",
    "title",
    "wizinvis",
    "site",
    "wizlevel",
    "on",
    "off",
    "\n"
  };
 
  half_chop(arg, buf, tmp);
  if (!*buf)
    {
      send_to_char("Current WHOD mode:\n\r------------------\n\r", ch);
      sprintbit((long) whod_mode,modes, buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      return;
    }
  
  if ((bit = old_search_block(buf,0,strlen(buf),modes,0)) == -1)
    {
      send_to_char("That mode does not exist.\n\r"
		   "Available modes are:\n\r", ch);
      *buf='\0';
      for (bit = 0; *modes [bit] != '\n'; bit++)
	{
	  strcat(buf,modes[bit]);
	  strcat(buf," ");
	}
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      return;
    }
 
  bit--; /* Is bit no + 1 */
  if (SHOW_ON == 1<<bit)
    {
      if (IS_SET(whod_mode,SHOW_ON))
	send_to_char("WHOD already turned on.\n\r",ch);
      else
	{
	  if (IS_SET(whod_mode,SHOW_OFF))
	    {
	      REMOVE_BIT(whod_mode,SHOW_OFF);
	      SET_BIT(whod_mode,SHOW_ON);
	      send_to_char("WHOD turned on.\n\r",ch);
	      sprintf(buf,"WHOD turned on by %s.",GET_NAME(ch));
	      LOG(buf);
	    }
	}
    }
  else
    {
      if (SHOW_OFF == 1<<bit)
	{
	  if (IS_SET(whod_mode,SHOW_OFF))
	    send_to_char("WHOD already turned off.\n\r",ch);
	  else
	    {
	      if (IS_SET(whod_mode,SHOW_ON))
		{
		  REMOVE_BIT(whod_mode,SHOW_ON);
		  SET_BIT(whod_mode,SHOW_OFF);
		  send_to_char("WHOD turned off.\n\r",ch);
		  sprintf(buf,"WHOD turned off by %s.",GET_NAME(ch));
		  LOG(buf);
		}
	    }
	}
      else
	{
	  if (IS_SET(whod_mode,1<<bit))
	    {
	      sprintf(buf,"%s will not be shown on WHOD.\n\r",modes[bit]);
	      send_to_char(buf,ch);
	      sprintf (buf, "%s removed from WHOD by %s.",
		       modes [bit],GET_NAME(ch));
	      LOG(buf);
	      REMOVE_BIT(whod_mode,1<<bit);
	      return;
	    }
	  else
	    {
	      sprintf(buf,"%s will now be shown on WHOD.\n\r",modes[bit]);
	      send_to_char(buf, ch);
	      sprintf(buf,"%s added to WHOD by %s.",modes[bit],GET_NAME(ch));
	      LOG(buf);
	      SET_BIT(whod_mode,1<<bit);
	      return;
	    }
	}
    }
 
  return;
}
 
 
 
/*          ------ WHO Daemon staring here ------          */
 
/* Function   : init_whod
 * Parameters : Port #-1 the daemon should be run at
 * Returns    : --
 * Description: Opens the WHOD port and sets the state of WHO-daemon to
  		OPEN							 */
void
init_whod(int port)
{
  whod_port = port+1;
  LOG("WHOD port opened.");
  s = init_socket(whod_port);
  state = WHOD_OPEN;
}
 
/* Function   : close_whod
 * Parameters : --
 * Returns    : --
 * Description: Closes the WHOD port and sets the state of WHO-daemon to
 *              CLOSED.                                                  */
void
close_whod (void)
{
  if (state != WHOD_CLOSED)
    {
      state = WHOD_CLOSED ;
      close(s);
      LOG("WHOD port closed.");
    }
}
 
/* Function   : whod_loop
 * Parameters : --
 * Returns    : --
 * Description: Serves incoming WHO calls.
 */
void
whod_loop(void)
{
  int	 nfound, players = 0, gods = 0;
  socklen_t size;
  fd_set in;
  u_long hostlong;
  struct timeval timeout;
  struct sockaddr_in newaddr;
  char   buf[MAX_STRING_LENGTH], tmp[80];
  struct char_data  *ch;
  struct hostent *hent;
 
  static int newdesc;
 
  switch (state)
    {
      /****************************************************************/
    case WHOD_OPENING:
      s = init_socket(whod_port);
      LOG("WHOD port opened.");
      state = WHOD_OPEN;
      break;
 
      /****************************************************************/
    case WHOD_OPEN:
    
      timeout.tv_sec = 0;
      timeout.tv_usec = 100;
    
      FD_ZERO(&in);
      FD_SET(s, &in);
 
      nfound = select(s+1,(fd_set*)&in,(fd_set*) 0,(fd_set*) 0,&timeout);
 
      if (FD_ISSET(s,&in))
	{
	  size = sizeof(newaddr);
	  getsockname(s, (struct sockaddr *)&newaddr, &size);
 
	  if ((newdesc = accept(s, (struct sockaddr *)&newaddr, &size)) < 0)
	    {
	      perror("WHOD - Accept");
	      return;
	    }
 
	  if (!(hent = gethostbyaddr((char *)(&newaddr.sin_addr),
				     sizeof(newaddr.sin_addr),AF_INET)))
	    {
	      hostlong = htonl(newaddr.sin_addr.s_addr); 
	      sprintf(buf,"WHO request from %d.%d.%d.%d served.", 
		      (int)((hostlong & 0xff000000) >>24),
		      (int)((hostlong & 0x00ff0000) >>16), 
		      (int)((hostlong & 0x0000ff00) >>8),
		      (int)((hostlong & 0x000000ff) >>0));
	    }
	  else 
	    sprintf(buf,"WHO request from %s served.",hent->h_name);

	  LOG(buf);

	  sprintf(buf, "%s", SHORT_GREETINGS);

	  WRITE(newdesc,buf);
 
	  sprintf(buf,"\n\rA list of active players on %s:\n\r\n\r",MUDNAME);
 
	  for (ch = character_list; ch; ch = ch->next)
	    if ((!IS_NPC(ch)) && (DISPLAY_LINKDEAD || ch->desc))
	      if (IS_SET(SHOW_INVIS,whod_mode) || ! IS_INVIS(ch))
		{
		  if (IS_SET((SHOW_LEVEL | SHOW_CLASS),whod_mode))
		    {
		      strcat(buf,"[");
		      if (!IS_SET(SHOW_WIZLEVEL,whod_mode) &&
			  (GET_LEVEL(ch) >= WIZ_MIN_LEVEL))
			{
			  if (IS_SET(SHOW_LEVEL,whod_mode) &&
			      IS_SET(SHOW_CLASS,whod_mode))
			    if (GET_LEVEL(ch) >= 40)
			      strcat(buf,"*IMP*");
			    else if (GET_LEVEL(ch) >= 38)
			      strcat(buf,"GRGOD");
			    else if (GET_LEVEL(ch) >= 36)
			      strcat(buf,"HIGOD");
			    else if (GET_LEVEL(ch) >= 35)
			      strcat(buf,"_LEG_");
			    else if (GET_LEVEL(ch) >= 34)
			      strcat(buf," GOD ");
			    else if (GET_LEVEL(ch) >= 32)
			      strcat(buf,"TITAN");
			    else
			      strcat(buf," IMM ");
			  else
			    strcat(buf,"_GOD_");
			}
		      else
			{
			  if (IS_SET(SHOW_LEVEL,whod_mode))
			    {
			      sprintf(tmp,"%2d",GET_LEVEL(ch));
			      strcat(buf,tmp);
			    }
			  if (IS_SET(SHOW_CLASS,whod_mode) &&
			      IS_SET(SHOW_LEVEL,whod_mode))
			    {
			      strcat(buf," ");
			    }
			  if (IS_SET(SHOW_CLASS,whod_mode))
			    {
			      sprintf(tmp, "%s",
				      class_abbrevs[(int)GET_CLASS(ch)]);
			      strcat(buf,tmp);
			    }
			}
		      strcat(buf,"] ");
		    }
 
		  if(IS_SET(SHOW_NAME,whod_mode))
		    strcat(buf,GET_NAME(ch));
 
		  if (IS_SET(SHOW_TITLE,whod_mode))
		    {
		      strcat(buf," ");
		      strcat(buf,GET_TITLE(ch));
		      if (PRF_FLAGGED(ch, PRF_AFK))
			strcat(buf, " (AFK)");
		    }
		  if (IS_SET(SHOW_SITE,whod_mode))
		    {
		      if (ch->desc->host != NULL)
			sprintf(tmp," [%s]",ch->desc->host);
		      else
			sprintf(tmp," [ ** Unknown ** ]");
		      strcat(buf,tmp);
		    }
	      
		  strcat(buf,"\n\r");
		  if (GET_LEVEL(ch) >= WIZ_MIN_LEVEL) gods++; else players++;
		  if (MAX_STRING_LENGTH-strlen(buf)<=80)
		    {
		      WRITE (newdesc, buf);
		      *buf='\0';
		    }
		}
 
	  sprintf(tmp,"\n\rPlayers : %d     Gods : %d\n\r\n\r",players,gods);
	  strcat(buf, tmp);
 
	  WRITE(newdesc,buf);
 
	  disconnect_time = time(NULL) + WHOD_DELAY_TIME;
 
	  state = WHOD_DELAY;
	}
      else
	if (IS_SET(SHOW_OFF,whod_mode))
	  state = WHOD_CLOSING;
    
      break;
 
      /****************************************************************/
    case WHOD_DELAY:
      if (time(NULL) >= disconnect_time) 
	state = WHOD_END;
      break;
 
      /****************************************************************/
    case WHOD_END:
      close(newdesc);
      if (IS_SET(whod_mode,SHOW_OFF))
	state = WHOD_CLOSING;
      else
	state = WHOD_OPEN;
      break;
    
      /****************************************************************/
    case WHOD_CLOSING:
      close_whod();
      state = WHOD_CLOSED ;
      break;
 
      /****************************************************************/
    case WHOD_CLOSED:
      if (IS_SET(whod_mode,SHOW_ON))
	state = WHOD_OPENING;
      break;
 
    }
  return;
}
 
/* *** You might want to use this in your help_file.		     *** */
/* *** It should be placed in the end of the file, so help on WHO is *** */
/* ***  availeble too.						     *** */
/*
WHOD
 
The who daemon is run on a separate port. The following commands exists:
 
name    : Toggles peoples name on/off the list (useless)
class   : Toggles peoples level on/off the list
title   : Toggles peoples title on/off the list
wizinvis: Toggles whether or not wizinvis people should be shown on the list
site    : Toggles peoples site names on/off the list
wizlevel: Toggles whether or not to show wizards class and level
on      : Turns the whod on, and thereby opens the port
off     : Turns the whod off, and thereby closes the port
 
NOTE:     The on/off feature is only made to use, if someone starts polling
	a few times a second or the like, and thereby abusing the net. You
	might then want to shut down the daemon for 15 minutes or so.
#
*/

#define NOT !
#define AND &&
#define OR  ||

int
old_search_block(char *argument, int begin, int length, char **list, int mode)
{
  int   guess, found, search;

  /* If the word contain 0 letters, then a match is already found */
  found = (length < 1);

  guess = 0;

  /* Search for a match */

  if (mode)
    while ( NOT found AND * (list[guess]) != '\n' )
      {
        found = (length == strlen(list[guess]));
        for (search = 0; (search < length AND found); search++)
          found = (*(argument + begin + search) == *(list[guess] + search));
        guess++;
      }
  else
    {
      while ( NOT found AND * (list[guess]) != '\n' )
        {
          found = 1;
          for (search = 0; (search < length AND found); search++)
            found = (*(argument + begin + search) == *(list[guess] + search));
          guess++;
        }
    }

  return ( found ? guess : -1 );
}

int get_whod_desc(void)
{
   return (s) ;
}
