###
###   Makefile for CircleMUD 3.0
###
###
###  All parts of this code not covered by the copyright by the Trustees of
###  the Johns Hopkins University are Copyright (C) 1996, 97, 98 by the
###  Dark Pawns Coding Team.
###
###  This includes all original code done for Dark Pawns MUD by other authors.
###  All code is the intellectual property of the author, and is used here
###  by permission.
###
###  No original code may be duplicated, reused, or executed without the
###  written permission of the author. All rights reserved.
###
###  See dp-team.txt or "help coding" online for members of the Dark Pawns
###  Coding Team.
###

SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .o .c
.SUFFIXES: .o .ln

SRCDIR=.

CC=gcc

#CFLAGS=-g -O2 -Wall -fno-builtin
CFLAGS=-ggdb3 -Wall

LINTFLAGS=-abchu

BUILD_DIR=/home/jravn/3.0/circle/src
MAIN_DIR=/home/jravn/3.0/circle
LUAINCDIR = ./include
LUALIBDIR = ./lib
SQLDIR    = /usr/lib/mysql
LUALIBS   = -llualib -llua -lm
ZLIBDIR   = /usr/lib
CRYPTLIB  = -lcrypt
UUIDLIB   = -luuid 
SQLLIB    = -lmysqlclient
ZLIB      = -lz

OBJFILES = act.comm.o act.display.o act.informative.o act.item.o \
	act.movement.o act.offensive.o act.other.o act.social.o \
	act.wizard.o alias.o ban.o boards.o class.o comm.o \
	config.o constants.o db.o dream.o fight.o gate.o graph.o \
	handler.o house.o interpreter.o limits.o magic.o \
	mail.o mapcode.o medit.o mobact.o modify.o \
	new_cmds.o new_cmds2.o notes.o objsave.o oc.o oedit.o olc.o pc_shop.o poof.o \
	random.o redit.o scripts.o sedit.o shop.o spec_assign.o spec_procs.o \
	spec_procs2.o spec_procs3.o spec_procs4.o spell_parser.o spells.o sql.o tattoo.o \
	tedit.o utils.o version.o weather.o whod.o zedit.o events.o

LNFILES = act.comm.ln act.display.ln act.informative.ln act.item.ln \
	act.movement.ln act.offensive.ln act.other.ln act.social.ln \
	act.wizard.ln alias.ln ban.ln boards.ln clan.ln class.ln comm.ln \
	config.ln constants.ln db.ln dream.ln fight.ln gate.ln graph.ln \
	handler.ln house.ln interpreter.ln limits.ln magic.ln \
	mail.ln mapcode.ln medit.ln mobact.ln modify.ln \
	new_cmds.ln new_cmds2.ln notes.ln objsave.ln oc.ln oedit.ln olc.ln pc_shop.ln poof.ln \
	random.ln redit.ln scripts.ln sedit.ln shop.ln spec_assign.ln spec_procs.ln \
	spec_procs2.ln spec_procs3.ln spec_procs4.ln spell_parser.ln spells.ln sql.ln tattoo.ln \
	tedit.ln utils.ln version.ln weather.ln whod.ln zedit.ln events.ln \

default: circle.dep
	@echo Default build.
	@$(MAKE) circle.new
	@echo done.

all:
	@echo Complete build.
	@$(MAKE) circle.new
	@$(MAKE) utility
	@echo done.

utility: 	
	@$(MAKE) -C utils

circle.new:
	@./build.pl
	@$(CC) -c $(CFLAGS) version.c
	@$(MAKE) bin/circle.new

bin/circle.new: $(OBJFILES)
	$(CC) -o bin/circle.new $(CFLAGS) $(OBJFILES) -L$(LUALIBDIR) -L$(SQLDIR) -L$(ZLIBDIR) $(LUALIBS) $(SQLLIB) $(ZLIB)
	@chmod 770 bin/circle.new

drop: bin/circle.new
	@echo Moving build to drop point.
	@mv bin/circle.new $(BUILD_DIR)/bin/circle.new
	@echo done.

main: bin/circle.new
	@echo Moving build to main port. 
	@mv bin/circle.new $(MAIN_DIR)/bin/circle.new
	@echo done.
clean:
	@echo Cleaning all object files.
	@rm -f *.o
	@echo done.

spotless:
	@echo Spotless cleaning.
	@echo Cleaning all object files.
	@rm -f *.o
	@echo Removing binaries.
	@rm -f bin/circle.new
	@echo done.

# Dependencies for the object files (automagically generated with
# gcc -MM)

circle.dep:
	@echo Creating dependecies.
	@$(CC) -MM $(CFLAGS) -I$(LUAINCDIR) \
	$(OBJFILES:.o=.c) \
	> circle.dep 
	@echo done.

dep:
	@rm -rf circle.dep
	@$(MAKE) circle.dep

.c.o:
	$(CC) -c $(CFLAGS) -I$(LUAINCDIR) $<

.c.ln:
	$(LINT) $(LINTFLAGS) $<

