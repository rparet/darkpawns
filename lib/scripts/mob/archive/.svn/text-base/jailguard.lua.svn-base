function bribe()
  local amount = tonumber(argument)

  if (amount < (ch.level * ch.level)) then
    say("Trying to bribe me? That's against the law you know...")
    say("And not quite enough gold, either.")
    social(me, "grin");
  else
    say("Thank you very much friend, most kind.")
    say("Now get outta here!")
    act("$n throws $N out of the cell.", TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n throws you of the cell.\r\n", TRUE, me, NIL, ch, TO_VICT)
    ch.jail = 0
    save_char(ch)
    for i = 0, getn(room.exit) do
      if (room.exit[i]) then
        tport(ch, room.exit[i])
        break
      end
    end
  end
end

function sound()
  local buf = ""

  if (number(0, 3) == 0) then
    if (room.char) then
      for i = 1, getn(room.char) do
        if (room.char[i].jail > 0) then
          action(me, "point "..room.char[i].name)
          if (room.char[i].jail > 1) then
            buf = "hours"
          else
            buf = "hour"
          end
          say("Only "..room.char[i].jail.." "..buf.." left for you.")
        end
      end
    end
  end
end

function onpulse_pc()
-- Check the player's jail time remaining and boot them out when it's over

  for i = 1, getn(room.char) do
    if (room.char[i].level <= LVL_IMMORT) then 
      ch = room.char[i]
      if (ch.jail <= 0) then
        act("The guard says, 'Time's up, scum!'", TRUE, ch, NIL, NIL, TO_CHAR)  
        act("$n gets thrown out of the cell!", TRUE, ch, NIL, NIL, TO_ROOM)    
        act("The guard throws you out of the cell!\r\n", TRUE, ch, NIL, NIL, TO_CHAR)
        for j = 0, getn(room.exit) do		-- Only one exit to the jail
          if (room.exit[j]) then
	    tport(ch, room.exit[j])
            break
          end
        end
      end
    end
  end
end

