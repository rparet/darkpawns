function onpulse_pc()
-- The mob will strike any player it can see in the room.

  if (room.char) then
    for i = 1, getn(room.char) do
      if (not isnpc(room.char[i]) and cansee(room.char[i]) and (number(0, 5) == 0)) then
        action(me, "strike "..ch.alias)
      end
    end
  end
end

