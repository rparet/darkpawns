function oncmd()
-- use this ONLY if the portal will not be in a room with other portals
  local command = ""
  local subcmd = ""
  if (strfind(argument, "%a%s") ~= NIL) then
    command = strlower(strsub(argument, 1, strfind(argument, "%a%s")))
    subcmd = strlower(gsub(argument, command.." ", ""))
    if (strfind(subcmd, "%a%s") ~= NIL) then
      subcmd = strsub(subcmd, 1, strfind(subcmd, "%a%s"))
    end
  else
    return (FALSE)
  end
  if ((command == "enter") and (strfind(strlower(obj.alias), "%s?"..subcmd))) then
    act("You step into the portal and and a strange feeling overcomes you.\r\n",
        TRUE, ch, NIL, NIL, TO_CHAR)
    act("$n enters the portal and disappears!", TRUE, ch, NIL, NIL, TO_ROOM)
    tport(ch, obj.cost)
    act("There is a brilliant flash and $n steps from a portal.", TRUE, ch, NIL, NIL, TO_ROOM)
    return (TRUE)
  end
  return (FALSE)
end

