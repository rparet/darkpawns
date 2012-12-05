function onpulse_pc()
-- The guard captain will wake any guard listed within the array below.

  local guards = { 4807, 4815, 5302, 8060, 18215, 21200, 21201 }
  local citizens = { 4801, 5319, 8062, 21243 }

  if (room.char) then
    for i = 1, getn(room.char) do
      for j = 1, getn(guards) do
        if (isnpc(room.char[i])) then
          if (room.char[i].vnum == guards[j]) then		-- It's a city guard
            if (me.pos < POS_STANDING) then				-- He's asleep or sitting
              act("$n barks 'On your feet, slacker!'", TRUE, me, NIL, NIL, TO_ROOM)
              act("$n wakes up and quickly snaps to attention!", TRUE, room.char[i], NIL, NIL, TO_ROOM)
              act("$n growls, 'Report to my office at 0500 tomorrow morning.'", TRUE, me, NIL, NIL, TO_ROOM)
              room.char[i] = POS_STANDING
              save_char(room.char[i])
              return
            else
              if (number(0, 4) == 0) then
                act("$n snaps to attention and salutes!", TRUE, room.char[i], NIL, NIL, TO_ROOM)
                act("$n growls, 'At ease, soldier.'", TRUE, me, NIL, NIL, TO_ROOM)
                return
              end
            end
          elseif (room.char[i].vnum == citizens[j]) then
            if (number(0, 4) == 0) then
              act("$n frowns at $N.", TRUE, me, NIL, room.char[i], TO_ROOM)
              act("$n says, 'Hail unto the True One, Captain.'", TRUE, room.char[i], NIL, NIL, TO_ROOM)
              return
            end
          end
        end
      end
    end
  end
end
