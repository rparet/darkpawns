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

/* $Id: conf.h 1329 2007-06-29 04:53:09Z rparet $ */

#ifndef _CONF_H
#define _CONF_H

/* src/conf.h.  Generated automatically by configure.  */
/* src/conf.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if you have the crypt function.  */
#define HAVE_CRYPT 1 

/* Define if you have the random function.  */
#define HAVE_RANDOM 1

/* Define if you have the <arpa/telnet.h> header file.  */
#define HAVE_ARPA_TELNET_H 1

/* Define if you have the <assert.h> header file.  */
#define HAVE_ASSERT_H 1

/* Define if you have the <crypt.h> header file.  */
/* #define HAVE_CRYPT_H 1 */

/* Define if you have the <errno.h> header file.  */
#define HAVE_ERRNO_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <net/errno.h> header file.  */
/* #undef HAVE_NET_ERRNO_H */

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/fcntl.h> header file.  */
/* #define HAVE_SYS_FCNTL_H 1 */

/* Define if you have the <sys/select.h> header file.  */
/* #undef HAVE_SYS_SELECT_H */

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the crypt library (-lcrypt).  */
#define HAVE_LIBCRYPT 1

/* Define if you have the malloc library (-lmalloc).  */
/* #undef HAVE_LIBMALLOC */

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

#endif /* _CONF_H */
