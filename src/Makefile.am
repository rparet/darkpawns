# recursive builds
SUBDIRS = lua .
DIST_SUBDIRS = .

# additional gcc flags
AM_CFLAGS = -Wall -pedantic
AM_CPPFLAGS = -I$(srcdir)/lua/include

# build darkpawns library
noinst_LIBRARIES = libdp.a
libdp_a_SOURCES = act.comm.c act.display.c act.informative.c act.item.c	\
	act.movement.c act.offensive.c act.other.c act.social.c act.wizard.c	\
	alias.c ban.c boards.c clan.c class.c comm.c config.c constants.c db.c	\
	dream.c fight.c file-edit.c gate.c graph.c handler.c house.c		\
	improved-edit.c ident.c interpreter.c limits.c luaedit.c magic.c	\
	mail.c mapcode.c medit.c mobact.c mobprog.c modify.c new_cmds.c		\
	new_cmds2.c objsave.c oc.c oedit.c olc.c poof.c random.c redit.c	\
	scripts.c sedit.c shop.c spec_assign.c spec_procs.c spec_procs2.c	\
	spec_procs3.c spell_parser.c spells.c tattoo.c tedit.c utils.c		\
	version.c weather.c whod.c zedit.c events.c queue.c 
include_HEADERS = boards.h clan.h comm.h constants.h db.h dream.h events.h	\
	file-edit.h handler.h house.h ident.h improved-edit.h interpreter.h	\
	mail.h mobprog.h oc.h olc.h queue.h random.h screen.h scripts.h shop.h	\
	spells.h structs.h sysdep.h tedit.h timezone.h utils.h vt100.h whod.h

nodist_libdp_a_SOURCES = svn_revision.h
BUILT_SOURCES = svn_revision.h
CLEANFILES = svn_revision.h ../bin/circle$(EXEEXT)

# build circle executable
bin_PROGRAMS = circle
circle_LDADD = libdp.a lua/lib/liblualib.a lua/lib/liblua.a
circle_SOURCES = circle.c

# create header file with svn revision information
svn_revision.h: Makefile $(srcdir)/*.c
	@echo 'dp-build: Updating SVN revision.'
	echo '#define SVN_REVISION "$(shell svnversion -n)"' > $@

# move circle to top level
../bin/circle: circle
	@echo 'dp-build: Installing circle to top level.'
	mv -f $^ $@

# add 'move circle' task
all-local: ../bin/circle
