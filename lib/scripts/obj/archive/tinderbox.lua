function oncmd()
-- When a player strikes the tinderbox in the phoenix nest (room 1401), they are transported to the
-- beginning of the zone. NPCs will not be transported. Attached to object 1412.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "use") then
    if (not obj_list(subcmd, "char") or (obj.vnum ~= 1412)) then
      return
    end

    if (room.vnum ~= 1401) then
      return
    end
    act("$n strikes a tinderbox and a spark ignites some of the surrounding wood.\r\n", TRUE, me, NIL, NIL, TO_ROOM)
    act("You strike a tinderbox and a spark ignites some of the surrounding wood.\r\n", TRUE, me, NIL, NIL, TO_CHAR)
    local buf = 
      "Almost instantly, the entire nest is enguled in flames, yet they don't seem to\r\n"
      .."burn you. There is a loud shriek and a giant phoenix rises from the ashes, its\r\n"
      .."body ablaze with fire. Having heard of these mythical beasts, you climb onto\r\n"
      .."its back and watch as it leaps into the air with a single beat of its wings.\r\n"
      .."You admire the landscape as it passes below you at great speed and after\r\n"
      .."several minutes, you approach a stone tower high amongst a series of mountain\r\n"
      .."tops. The phoenix deposits you lightly on the ground before flying off again\r\n"
      .."into the distance.\r\n"
    act(buf, TRUE, me, NIL, NIL, TO_ROOM)
    act(buf, TRUE, me, NIL, NIL, TO_CHAR)

    tport(me, 1402)                  -- Since "me" is not included in the room
    for i = 1, getn(room.char) do
      tport(room.char[i], 1402)
    end
    echo(me, "outdoor", "A giant bird ablaze with fire streaks across the sky.\r\n")
    extobj(obj)
    return (TRUE)
  end
end
