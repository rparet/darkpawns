/*
 \file    lookup.c
 \brief   Resolves an IP through DNS and stores results in a file.
 \author  Derrick Coetzee
          $Date: 2001-04-09 04:55:33 -0400 (Mon, 09 Apr 2001) $

 Meant to be called from CircleMUD, resolves an IP synchronously
 (but asynchronously from the main MUD process) and stores the
 results in a DNS file. It then signals the MUD, which (in
 resolve_active_IPs() in comm.c) reads the results out.

 lookup can also be called from a shell by putting 0 for the
 pid. If you do this, you may wish to remove the lines that
 clean out the file in resolve_active_IPs().
*/

#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

#include "../db.h"       /* FILE_DNS */

#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
  struct hostent *from;
  struct in_addr peer;
  FILE *f;

  int pid = 0;

  char ipbuf[128];
  char hostbuf[128];

  if (argc < 3)
  {
      return -1;
  }

  pid = atoi(argv[1]);

  f = fopen(DNS_FILE, "rt");
  if (!f)
  {
      f = fopen(DNS_FILE, "wt");
      if (!f)
      {
         return -1; /* abort if we can't open, might lose a DNS */
      }
  }

  while (!feof(f))
  {
      if (fscanf(f, "%s %s\n", ipbuf, hostbuf) < 2)
      {
         break;
      }
      if(!strcmp(ipbuf, argv[2]))
      {
         /* Already cached */
         fclose(f);
         if (pid) kill(pid, SIGUSR2);
         return 0;
      }
  }

  fclose(f);

  inet_aton(argv[2], &peer);

  if ( !(from = gethostbyaddr((char *)&peer.s_addr, sizeof(peer.s_addr), AF_INET)) )
  {
      /* Failed, shhh, let the MUD run on with a numeric IP */
      return -1;
  }

  // Successful, add to DNS file and signal MUD
  f = fopen(DNS_FILE, "at");
  if (!f)
  {
      return -1; /* abort if we can't open, might lose a DNS */
  }
  fprintf(f, "%s %s\n", argv[2], from->h_name);
  fclose(f);

  if (pid) kill(pid, SIGUSR2);
  return 0;
}

