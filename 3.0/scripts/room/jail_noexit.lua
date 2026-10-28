function oncmd()
-- The jail guard will prevent players from leaving the prison, direction based on the
-- jail zone.

  local command = 0
  local jails =
   { [5365]  = { "west" },
     [8062]  = { "north" },
     [18290] = { "down" },
     [21226] = { "south"} }

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (me.level < LVL_IMMORT) then
    for index, value in jails do
      if (room.vnum == index) then
        if (command ~= jails[index][1]) then
          return
        end
      end
    end

    act("The guard grabs $n with one hand and throws $m back in the room.",
         TRUE, me, NIL, NIL, TO_ROOM)
    act("The guard stops you from leaving with one flabby hand.", TRUE, me, NIL, NIL, TO_CHAR)
    return (TRUE)
  end
end

