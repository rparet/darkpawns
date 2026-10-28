function onpulse()
-- If Galeru is dead, any player entering this room is teleported back to the beginning of the
-- zone.

  local found = FALSE

  for i = 1, getn(room.char) do
    if (room.char[i].vnum == 1315) then		-- Is this Galeru?
      found = TRUE
    end
  end

  if (found == FALSE) then
    act("You begin to feel very dizzy and the world around you fades...\r\n",
         TRUE, me, NIL, NIL, TO_CHAR)
    act("$n disappears in a brilliant flash of light.", TRUE, me, NIL, NIL, TO_ROOM)
    tport(me, 1395)
    act("$n appears in a brilliant flash of light.", TRUE, me, NIL, NIL, TO_ROOM)
  end
end

function enter()
-- If Galeru is dead, any player entering this room is teleported back to the beginning of the
-- zone. This second function is needed to prevent players avoiding the room pulse.

  local found = FALSE

  for i = 1, getn(room.char) do
    if (room.char[i].vnum == 1315) then		-- Is this Galeru?
      found = TRUE
    end
  end

  if (found == FALSE) then
    act("You begin to feel very dizzy and the world around you fades...\r\n",
         TRUE, me, NIL, NIL, TO_CHAR)
    act("$n disappears in a brilliant flash of light.", TRUE, me, NIL, NIL, TO_ROOM)
    tport(me, 1395)
    act("$n appears in a brilliant flash of light.", TRUE, me, NIL, NIL, TO_ROOM)
  end
end
