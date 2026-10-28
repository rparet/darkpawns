function oncmd()
-- The jail guard will prevent players from entering the prison, direction based on the
-- jail zone.
  
  local command = 0
  local jails =
    { [5333] =  { "east" },
      [8055]  = { "south" },
      [18286] = { "up" },
      [21233] = { "north" } }

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

    act("The guard grabs $n by the collar and blocks $s way.", TRUE, me, NIL, NIL, TO_ROOM)
    act("The guard stops you from entering with one quick jerk of your collar.",
      TRUE, me, NIL, NIL, TO_CHAR)
    return (TRUE)
  end
end

