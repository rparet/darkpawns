function oncmd()
-- Teleport the player to the appropriate room based on their current location, provided they
-- specify the "portal" to enter. Attached to rooms 1414 and 1423.

  local location = { 1414, 1423 }
  local new_location = { 1423, 1414 }
  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if ((command == "enter") and strfind(subcmd, "portal")) then
    act("You step into the shimmering blue portal...\r\n", TRUE, me, NIL, NIL, TO_CHAR)
    act("$n enters the portal and disappears!", TRUE, me, NIL, NIL, TO_ROOM)

    for i = 1, getn(location) do
      if (room.vnum == location[i]) then
        tport(me, new_location[i])
        act("There is a brilliant flash and $n steps from the portal.", TRUE, me, NIL, NIL, TO_ROOM)
        break
      end
    end
    return (TRUE)
  end
end
