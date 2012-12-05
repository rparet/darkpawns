function onpulse_all()
-- The mob will "mine" for crystalline chunks (obj 11701) and when a roaming golem
-- passes by (mob 11700), any chunks the mob has will be given to the golem. Attached
-- to mob 11702.

  local alias = ""

  if (number(0, 50) == 0) then
    if (me.objs and (getn(me.objs) > 3)) then		-- Limit to 3 objects
      return
    end

    act("$n breaks off a crystalline chunk from the tunnel wall.",
      TRUE, me, NIL, NIL, TO_ROOM)
    oload(me, 11701, "char")					-- Found a crystal, mine it
    return
  end

  if (room.char) then
    for i = 1, getn(room.char) do
      if (room.char[i].vnum == 11700) then		-- Found a golem
        if (me.objs) then
          if (room.char[i].objs and (getn(room.char[i].objs) > 3)) then
            return
          end

          for j = 1, getn(me.objs) do			-- Do I have any crystals?
            if (me.objs[j].vnum == 11701) then
              alias = strsub(me.objs[j].alias, 1, strfind(me.objs[j].alias, "%a%s"))
              action(me, "give all."..alias.." to golem")
              return
            end
          end
        end
      end
    end
  end
end
