function onpulse_pc()
-- The mob will backstab any player it can see in the room. Attached to mobs
-- 9151 and 12912.

  if (room.char) then
    for i = 1, getn(room.char) do
      if (room.char[i].level >= LVL_IMMORT) then
        return
      elseif (not isnpc(room.char[i]) and cansee(room.char[i])) then
        action(me, "backstab "..room.char[i].alias)
      end
    end
  end
end
