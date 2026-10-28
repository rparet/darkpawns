function fight()
  call(onpulse_pc, me, "x")
end

function onpulse_pc()
-- At any time when a player is present, there will be a 20% chance that the mob
-- will remove all spell affections from a random player within the room. Attached
-- to mobs 10107 and 10108.

  local vict = NIL
  local counter = 0

  if (number(0, 4) == 0) then
    if (room.char) then
      repeat							-- Locate a random PC
        vict = room.char[number(1, getn(room.char))]
        counter = counter + 1
        if (counter > 100) then
          return
        end
      until (not isnpc(vict))
    
      if (vict.level >= LVL_IMMORT) then			-- Don't hit the Imms
        return
      end

      if (cansee(vict)) then					-- Can the mob see the player?
        act("A single touch of $N disrupts your concentration.",
          TRUE, vict, NIL, me, TO_CHAR)
        act("$n looks bewildered as $N creeps across $s boots.",
          TRUE, vict, NIL, me, TO_ROOM)
        unaffect(vict)						-- Remove spell affections
      end
    end
  end
end
