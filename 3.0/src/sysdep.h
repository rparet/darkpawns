/* ************************************************************************
*   File: sysdep.h                                      Part of CircleMUD *
*  Usage: machine-specific defs based on values in conf.h (from configure)*
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
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

/* $Id: sysdep.h 1329 2007-06-29 04:53:09Z rparet $ */

#ifndef _SYSDEP_H
#define _SYSDEP_H

/* Include the preprocessor defines created by the configure script */
#include "conf.h"

/*
 * CircleMUD uses the crypt(3) function to encrypt player passwords in the
 * playerfile so that they are never stored in plaintext form.  However,
 * due to U.S. export restrictions on machine-readable cryptographic
 * software, the crypt() function is not available on some operating
 * systems such as FreeBSD.  By default, the 'configure' script will
 * determine if you have crypt() available and enable or disable password
 * encryption appropriately.  #define NOCRYPT (by uncommenting the line
 * below) if you'd like to explicitly disable password encryption (i.e.,
 * if you have moved your MUD from an OS that does not support encryption
 * to one that does.)
 */

/* #define NOCRYPT */

/************************************************************************/
/*** Do not change anything below this line *****************************/
/************************************************************************/

/*
 * Set up various machine-specific things based on the values determined
 * from configure and conf.h.
 */

/* Standard C headers  *************************************************/

#include <stdio.h>
#include <ctype.h>

#if     (defined (STDC_HEADERS) || defined (__GNU_LIBRARY__))
#include <stdlib.h>
#include <string.h>

#else   /* No standard headers.  */

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef  HAVE_MEMORY_H
#include <memory.h>
#endif

extern char *malloc(), *calloc(), *realloc();
extern void free ();

extern void abort (), exit ();

#endif  /* Standard headers.  */

/* POSIX compliance  *************************************************/

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef CIRCLE_WINDOWS
# include <sys\types.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/* Now, we #define POSIX if we have a POSIX system. */

#ifdef HAVE_UNISTD_H
/* Ultrix's unistd.h always defines _POSIX_VERSION, but you only get
   POSIX.1 behavior with `cc -YPOSIX', which predefines POSIX itself!  */
#if defined (_POSIX_VERSION) && !defined (ultrix)
#define POSIX
#endif

/* Some systems define _POSIX_VERSION but are not really POSIX.1.  */
#if (defined (butterfly) || defined (__arm) || \
     (defined (__mips) && defined (_SYSTYPE_SVR3)) || \
     (defined (sequent) && defined (i386)))
#undef POSIX
#endif
#endif /* HAVE_UNISTD_H */

#if !defined (POSIX) && defined (_AIX) && defined (_POSIX_SOURCE)
#define POSIX
#endif

#if defined(_AIX)
#define POSIX_NONBLOCK_BROKEN
#endif


/* Header files *******************************************************/


#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(arg)
#endif

/* Define the type of a socket and other miscellany */
#ifdef CIRCLE_WINDOWS	/* Definitions for Win32 */
#define FD_SETSIZE 1024
#include <winsock.h>
typedef SOCKET socket_t;
#define CLOSE_SOCKET(sock) closesocket(sock)
#else					/* Definitions for UNIX */
typedef int socket_t;
#define CLOSE_SOCKET(sock) close(sock)
#endif


/* Function prototypes ************************************************/

/*
 * For reasons that perplex me, the header files of many OS's do not contain
 * function prototypes for the standard C library functions.  This produces
 * annoying warning messages (sometimes, a huge number of them) on such OS's
 * when compiling with gcc's -Wall.
 *
 * Some versions of CircleMUD prior to 3.0 patchlevel 9 attempted to include
 * prototypes taken from OS man pages for a large number of OS's in the
 * header files.  I now think such an approach is a bad idea: maintaining that
 * list is very difficult and time-consuming, and when new revisions of OS's
 * are released with new new header files, Circle can break if the
 * prototypes contained in Circle's .h files differs from the new OS header
 * files (for example, Circle 3.0 patchlevel 8 failed with compiler errors
 * under Solaris 2.5 and Linux 1.3.xx whereas under previous revisions of
 * those OS's it had been fine.
 *
 * Thus, to silence the compiler warnings but still maintain some level of
 * portability (albiet at the expense of worse error checking in the code),
 * my solution is to define a "typeless" function prototype for all problem
 * functions that have not already been prototyped by the OS.
 *
 * 20 Mar 96: My quest is not yet over.  These definitions still cause
 * clashes with some compilers.  Therefore, we only use these prototypes
 * if we're using gcc (which makes sense, since they're only here for gcc's
 * -Wall option in the first place), and configure tells gcc to use
 * -fno-strict-prototypes, so that these definitions don't clash with
 * previous prototypes.
 *
 * 21 Aug 06: This breaks GNU libc6 2.4 and gcc 3.4.6 (doesn't support the
 * -fno-strict-prototypes option anymore). Since these prototypes are only 
 * being used to hide warnings on systems with problematic libraries, and
 * they prevent compilation on modern systems, I've disabled them. - jsr
 */

#ifndef HAVE_RANDOM /* prototypes for my own random function */
void srandom(unsigned long initial_seed);
unsigned long random(void);
#endif

#if FALSE

#ifndef atoi
   int atoi();
#endif

#ifndef atol
   long atol();
#endif

#ifndef bzero
   void bzero();
#endif

#ifndef crypt
   char *crypt();
#endif

#ifndef fclose
   int fclose();
#endif

#ifndef fcntl
   int fcntl();
#endif

#ifndef fflush
   int fflush();
#endif

//#ifndef fprintf
//  int fprintf();
//#endif

#ifndef fread
   size_t fread();
#endif

#ifndef fscanf
   int fscanf();
#endif

#ifndef fseek
   int fseek();
#endif

//#ifndef fwrite
//   size_t fwrite();
//#endif

#ifndef getpid
   pid_t getpid();
#endif

#ifndef gettimeofday
   int gettimeofday();
#endif

#ifndef perror
   void perror();
#endif

#ifndef qsort
   void qsort();
#endif

#ifndef rewind
   void rewind();
#endif

//#ifndef sscanf
//   int sscanf();
//#endif

#ifndef system
   int system();
#endif

#ifndef time
   time_t time();
#endif

#ifndef unlink
   int unlink();
#endif

#endif /* __GNUC__ */

#endif /* _SYSDEP_H */
