/* ************************************************************************
*  File: ident.c                                                          *
*                                                                         *
*  Usage: Functions for handling rfc 931/1413 ident lookups               *
*                                                                         *
*  Written by Eric Green (egreen@cypronet.com)				  *
************************************************************************ */

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

/* $Id: ident.c 1487 2008-05-22 01:36:10Z jravn $ */

#define __IDENT_C__

#include "config.h"
#include "sysdep.h"

#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netdb.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "ident.h"


/* max time in seconds to make someone wait before entering game */
#define IDENT_TIMEOUT 2

#define IDENT_PORT    113


/* extern functions */
int isbanned(char *hostname);


/* start the process of looking up remote username */
void ident_start(struct descriptor_data *d, long addr)
{
  int sock;
  struct sockaddr_in sa;

  void nonblock(int s);

  if (!ident) {
    STATE(d) = CON_ASKNAME;
    d->ident_sock = -1;
    return;
  }
  
  SEND_TO_Q("Please wait", d);

  d->idle_tics = 0;

  /*
   * create a nonblocking socket, and start
   * the connection to the remote machine
   */

  if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    d->ident_sock = -1;
    STATE(d) = CON_ASKNAME;
    return;
  }

  sa.sin_family = AF_INET;
  sa.sin_port = ntohs(IDENT_PORT);
  sa.sin_addr.s_addr = addr;

  nonblock(sock);
  d->ident_sock = sock;

  errno = 0;
  if (connect(sock, (struct sockaddr*) &sa, sizeof(sa)) != 0) {
    if (errno == EINPROGRESS) {
      /* connection in progress */
      STATE(d) = CON_IDCONING;
      return;
    }

    /* connection failed */
    else if (errno != ECONNREFUSED)
      perror("ident connect");

    STATE(d) = CON_ASKNAME;
  }

  else    /* connection completed */
    STATE(d) = CON_IDCONED;
}


void ident_check(struct descriptor_data *d, int pulse)
{
  fd_set fd;
  int rc, rmt_port, our_port, len;
  char user[256], *p;

  extern struct timeval null_time;
  extern int port;

  /*
   * Each pulse, this checks if the ident is ready to proceed to the
   * next state, by calling select to see if the socket is writeable
   * (connected) or readable (response waiting).  
   */

  switch (STATE(d)) {
  case CON_IDCONING:
    /* waiting for connect() to finish */

    if (d->ident_sock != -1) {
      FD_ZERO(&fd);
      FD_SET(d->ident_sock, &fd);
    }

    if ((rc = select(d->ident_sock + 1, (fd_set *) 0, &fd,
		     (fd_set *) 0, &null_time)) == 0)
      break;

    else if (rc < 0) {
      perror("ident check select (conning)");
      STATE(d) = CON_ASKNAME;
      break;
    }

    STATE(d) = CON_IDCONED;

  case CON_IDCONED:
    /* connected, write request */
	
    sprintf(buf, "%d, %d\n\r", ntohs(d->peer_port), port);
	
    len = strlen(buf);
    if (write(d->ident_sock, buf, len) != len) {
      if (errno != EPIPE)	/* read end closed (no remote identd) */
	perror("ident check write (conned)");

      STATE(d) = CON_ASKNAME;
      break;
    }

    STATE(d) = CON_IDREADING;
	
  case CON_IDREADING:
    /* waiting to read */
	
    if (d->ident_sock != -1) {
      FD_ZERO(&fd);
      FD_SET(d->ident_sock, &fd);
    }

    if ((rc = select(d->ident_sock+1, &fd, (fd_set *) 0,
		     (fd_set *) 0, &null_time)) == 0)
      break;

    else if (rc < 0) {
      perror("ident check select (reading)");
      STATE(d) = CON_ASKNAME;
      break;
    }

    STATE(d) = CON_IDREAD;
	
  case CON_IDREAD:
    /* read ready, get the info */

    if ((len = read(d->ident_sock, buf, sizeof(buf) - 1)) < 0)
      perror("ident check read (read)");

    else {
      buf[len] = '\0';
      if (sscanf(buf, "%d , %d : USERID :%*[^:]:%255s",
		 &rmt_port, &our_port, user) != 3) {

	/* check if error or malformed */
	if (sscanf(buf, "%d , %d : ERROR : %255s",
		   &rmt_port, &our_port, user) == 3) {
	  sprintf(buf2, "Ident error from %s: \"%s\"", d->host, user);
	  log(buf2);
	}
	else {
	  /* strip off trailing newline */
	  for (p = buf + len - 1; p > buf && ISNEWL(*p); p--);
	  p[1] = '\0';

	  sprintf(buf2, "Malformed ident response from %s: \"%s\"",
		  d->host, buf);
	  log(buf2);
	}
      }
      else {
	/*strncpy(buf2, user, IDENT_LENGTH);*/
	strcpy(buf2, user);
	strcat(buf2, "@");
	strcat(buf2, d->host);
	/*strncpy(d->host, buf2, HOST_LENGTH);*/
	strcpy(d->host, buf2);
      }
    }
	
    STATE(d) = CON_ASKNAME;
	
  case CON_ASKNAME:
    /* ident complete, ask for name */

    /* close up the ident socket, if one is opened. */
    if (d->ident_sock != -1) {
      close(d->ident_sock);
      d->ident_sock = -1;
    }
    d->idle_tics = 0;

    /* extra ban check */
    if (isbanned(d->host) == BAN_ALL) {
      sprintf(buf, "Connection attempt denied from [%s]", d->host);
      mudlog(buf, CMP, LVL_GOD, TRUE);
      close_socket(d);
      return;
    }

    SEND_TO_Q("\x1B[2K\n\rBy what name do you wish to be known? ", d);
    STATE(d) = CON_GET_NAME;
    return;

  default:
    return;
  }

  /*
   * Print a dot every second so the user knows he hasn't been forgotten.
   * Allow the user to go on anyways after waiting IDENT_TIMEOUT seconds.
   */
  if ((pulse % PASSES_PER_SEC) == 0) {
    SEND_TO_Q(".", d);
    
    if (d->idle_tics++ >= IDENT_TIMEOUT)
      STATE(d) = CON_ASKNAME;
  }
}


/* returns 1 if waiting for ident to complete, else 0 */
int waiting_for_ident(struct descriptor_data *d)
{
  switch (STATE(d)) {
  case CON_IDCONING:
  case CON_IDCONED:
  case CON_IDREADING:
  case CON_IDREAD:
  case CON_ASKNAME:
    return 1;
      
  default:
    return 0;
  }

  return 0;
}
