function enter()
-- As a player may enter this room from the portal (room 20071), they may not possess
-- the correct key. If this is the case, load it for them onto the mob - attached to
-- room 20097.

  local found = FALSE

  if (ch.objs) then
    for i = 1, getn(ch.objs) do
      if (ch.objs[i].vnum == 20023) then
        found = TRUE
        break
      end
    end
  end

  -- No key found, load it onto the thrall.
  if (found == FALSE) then
    if (room.char) then
      for i = 1, getn(room.char) do
        if (isnpc(room.char[i])) then
          if (room.char[i].vnum == 20051) then
            oload(room.char[i], 20023, "char")
            return
          end
        end
      end
    end

    -- We got here, so the mob doesn't exist!
    oload(ch, 20023, "char")
    act("\r\nA key magically appears in your hand!", TRUE, ch, NIL, NIL, TO_CHAR)
  end
end
