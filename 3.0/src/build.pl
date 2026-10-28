#!/usr/bin/perl

#
#  $Id: build.pl 62 2000-06-23 20:46:10Z rparet $
#
#  All parts of this code are Copyright (c) 1998, The Dark Pawns Coding Team.
#
#  This includes all original code done for Dark Pawns MUD by other authors.
#  All code is the intellectual property of the author, and is used here
#  by permission.
#
#  No original code may be duplicated, reused, or executed without the
#  written permission of the author. All rights reserved.
#
#  See dp-team.txt or "help coding" online for members of the Dark Pawns
#  Coding Team.
#

$filename = "build.count.h";
$count = ".build.count";

print "Discovering build count.\n";
if (-e $count) {
   # read in the number and write it back out
   open(FILE, $count);
   while (<FILE>) {
      $build = $_;
   }
   close(FILE);
   open(FILE, ">$filename") || die "Could Not Open File!\n";
   $build++;
   print FILE "#ifndef _BUILD_COUNT_H\n";
   print FILE "#define _BUILD_COUNT_H\n";
   print FILE "#define BUILD_COUNT  $build\n";
   print FILE "#endif  /* _BUILD_COUNT_H */\n";
   print "Build Count: $build\n";
   close(FILE);
   open(FILE, ">$count") || die "Could Not Open File!\n";
   print FILE "$build\n";
   close(FILE);
} else {
   # build count begins at 1 and gets written to the file
   open(FILE, ">>$filename") || die "Could Not Open File!\n";
   $build = 1;
   print FILE "#ifndef _BUILD_COUNT_H\n";
   print FILE "#define _BUILD_COUNT_H\n";
   print FILE "#define BUILD_COUNT  $build\n";
   print FILE "#endif  /* _BUILD_COUNT_H */\n";
   print "Build Count: $build\n";
   close(FILE);
   open(FILE, ">$count") || die "Could Not Open File!\n";
   print FILE "$build\n";
   close(FILE);
}

