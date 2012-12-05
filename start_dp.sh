#!/bin/sh
#
#  New and Improved Autorun for CircleMUD 3.0 by S. Thompson
#
#  based on:
#   CircleMUD 3.0 autorun script
#   Contributions by Fred Merkel, Stuart Lamble, and Jeremy Elson
#   Copyright (c) 1996 The Trustees of The Johns Hopkins University
#   All Rights Reserved
#   See license.doc for more information
#
#############################################################################
#
# This script can be used to run CircleMUD over and over again (i.e., have it
# automatically reboot if it crashes).  It will run the game, and copy some
# of the more useful information from the system logs to the 'log' directory
# for safe keeping.
#
# You can control the operation of this script by creating and deleting files
# in Circle's root directory, either manually or by using the 'shutdown'
# command from within the MUD.
#
# Creating a file called .fastboot makes the script wait only 5 seconds
# between reboot attempts instead of the usual 45.  If you want a quick
# reboot, use the "shutdown reboot" command from within the MUD.
#
# Creating a file called .killscript makes the script terminate (i.e., stop
# rebooting the MUD).  If you want to shut down the MUD and make it stay
# shut down, use the "shutdown die" command from within the MUD.
#
# Finally, if a file called pause exists, the script will not reboot the MUD
# again until pause is removed.  This is useful if you want to turn the MUD
# off for a couple of minutes and then bring it back up without killing the
# script.  Type "shutdown pause" from within the MUD to activate this feature.
#

# Function: Init
Init() {
   # Set the max core size
   ulimit -c 1200000
   
   # The port on which to run the MUD
   PORT=4350

   # Default flags to pass to the MUD server (see running.doc for a description
   # of all flags).
   FLAGS=
}

# Function: CheckLibDirs

CheckLibDirs() {

   # Make sure all of the directories exist (this way a backup will run, 
   # almost out of the box)

   if [ ! -d lib/plralias ]; then
       mkdir lib/plralias
   fi
   if [ ! -d lib/plralias/A-E ]; then
       mkdir lib/plralias/A-E
   fi
   if [ ! -d lib/plralias/F-J ]; then
       mkdir lib/plralias/F-J
   fi
   if [ ! -d lib/plralias/K-O ]; then
       mkdir lib/plralias/K-O
   fi
   if [ ! -d lib/plralias/P-T ]; then
       mkdir lib/plralias/P-T
   fi
   if [ ! -d lib/plralias/U-Z ]; then
       mkdir lib/plralias/U-Z
   fi
   if [ ! -d lib/plralias/ZZZ ]; then
       mkdir lib/plralias/ZZZ
   fi
   if [ ! -d lib/plrobjs ]; then
       mkdir lib/plrobjs
   fi
   if [ ! -d lib/plrobjs/A-E ]; then
       mkdir lib/plrobjs/A-E
   fi
   if [ ! -d lib/plrobjs/F-J ]; then
       mkdir lib/plrobjs/F-J
   fi
   if [ ! -d lib/plrobjs/K-O ]; then
       mkdir lib/plrobjs/K-O
   fi
   if [ ! -d lib/plrobjs/P-T ]; then
       mkdir lib/plrobjs/P-T
   fi
   if [ ! -d lib/plrobjs/U-Z ]; then
       mkdir lib/plrobjs/U-Z
   fi
   if [ ! -d lib/plrobjs/ZZZ ]; then
       mkdir lib/plrobjs/ZZZ
   fi
   if [ ! -d lib/plrpoof ]; then
       mkdir lib/plrpoof
   fi
   if [ ! -d lib/plrpoof/A-E ]; then
       mkdir lib/plrpoof/A-E
   fi
   if [ ! -d lib/plrpoof/F-J ]; then
       mkdir lib/plrpoof/F-J
   fi
   if [ ! -d lib/plrpoof/K-O ]; then
       mkdir lib/plrpoof/K-O
   fi
   if [ ! -d lib/plrpoof/P-T ]; then
       mkdir lib/plrpoof/P-T
   fi
   if [ ! -d lib/plrpoof/U-Z ]; then
       mkdir lib/plrpoof/U-Z
   fi
   if [ ! -d lib/plrpoof/ZZZ ]; then
       mkdir lib/plrpoof/ZZZ
   fi
   if [ ! -d lib/house ]; then
       mkdir lib/house
   fi
   if [ ! -d log ]; then
       mkdir log
   fi

}

# Function: TouchLogs
TouchLogs() {
   touch log/syslog.1
   touch log/syslog.2
   touch log/syslog.3
   touch log/syslog.4
   touch log/syslog.5
   touch log/syslog.6
   touch log/help
}

# Function: MoveCircleNew
MoveCircleNew() {
   if [ -r bin/circle.new ]; then
       echo "moving bin/circle.new into bin/circle" >> syslog
       mv bin/circle.new bin/circle
   fi
}
MoveCore() {
   # Core nameing - FreeBSD/Linux
   # CORE=lib/circle.core
   CORE=lib/core
   if [ -r $CORE ]; then
       if [ ! -d cores ]; then
           mkdir cores
       fi
       COREDATE=`date '+%b.%d.%y-%T'`
       mv $CORE cores/core.$COREDATE
       gzip -9 cores/core.$COREDATE
       chgrp pawns-grp cores/core.$COREDATE.gz
       chmod g+rw cores/core.$COREDATE.gz
   fi
}

# Function: GrepLog
GrepLog() {
   fgrep "self-delete" syslog >> log/delete
   fgrep "death trap" syslog >> log/dts
   fgrep "killed" syslog >> log/rip
   fgrep "Running" syslog >> log/restarts
   fgrep "advanced" syslog >> log/levels
   fgrep "equipment lost" syslog >> log/rentgone
   fgrep "usage" syslog >> log/usage
   fgrep "new player" syslog >> log/newplayers
   fgrep "SYSERR" syslog >> log/errors
   fgrep "(GC)" syslog >> log/godcmds
   fgrep "HELP" syslog >> log/help
   fgrep "Bad PW" syslog >> log/badpws
   fgrep "LOSTEQ" syslog >> log/badquits
   fgrep "OLC" syslog >> log/olc
}

# Function: MoveSyslogs
MoveSyslogs() {
   rm log/syslog.1
   mv log/syslog.2 log/syslog.1
   mv log/syslog.3 log/syslog.2
   mv log/syslog.4 log/syslog.3
   mv log/syslog.5 log/syslog.4
   mv log/syslog.6 log/syslog.5
   mv syslog       log/syslog.6
   touch syslog
   chgrp pawns-grp syslog
}

# Function: FastBootCheck
FastBootCheck() {
   if [ ! -r .fastboot ]; then
       sleep 45
   else
       rm .fastboot
       sleep 5
   fi
}

# Function: KillScriptCheck
KillScriptCheck() {
   if [ -r .killscript ]; then
       DATE=`date`;
       echo "autoscript killed $DATE"  >> syslog
       rm .killscript
       exit
   fi
}

# Function: PauseCheck
PauseCheck() {
   while [ -r pause ]; do
      sleep 45
   done
}

# Function: Loop
Loop() {

   while ( : ) do

      DATE=`date`
      echo "autorun starting game $DATE" >> syslog
      echo "running bin/circle $FLAGS $PORT" >> syslog

      MoveCircleNew

      MoveCore

      bin/circle $FLAGS $PORT >> syslog 2>&1

      tail -30 syslog > syslog.CRASH

      GrepLog

      MoveSyslogs

      FastBootCheck
     
      KillScriptCheck

      PauseCheck

   done

}

# Function: Main
Main() {
   umask 006

   Init
   CheckLibDirs
   TouchLogs
   Loop

}

###########################################
###########################################

Main

###########################################
###########################################


