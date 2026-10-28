CC=gcc
CFLAGS=-g -O2 -Wall -fno-strict-prototypes
CRYPTLIB  = -lcrypt

default: 
	@echo Making utilities...
	@$(MAKE) utils
	@echo Done.

utils: purgeplay lookup passwd convertpfile

purgeplay: purgeplay.c
	$(CC) $(CFLAGS) -o ../bin/purgeplay purgeplay.c
        
lookup: lookup.c 
	$(CC) $(CFLAGS) -o ../bin/lookup lookup.c

passwd: passwd.c
	$(CC) $(CFLAGS) -o ../bin/passwd passwd.c $(CRYPTLIB)
        
convertpfile: convertpfile.c
	$(CC) $(CFLAGS) -o ../bin/convertpfile convertpfile.c

objsearch: objsearch.c
	$(CC) $(CFLAGS) -o ../bin/objsearch objsearch.c

