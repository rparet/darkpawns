function oncmd()
-- The mob will attack a player if they attempt to obtain an item from the room.
-- Attached to mobs 14416 and 14430.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if ((command == "get") or (command == "palm")) then
    if (obj_list(subcmd, "room") or obj_list(subcmd, "cont")) then
      act("$N reaches for $p but $n strikes at $S's hand!", TRUE, me, obj, ch, TO_NOTVICT)
      act("You reach for $p but $n strikes at your hand!", FALSE, me, obj, ch, TO_VICT)
      action(me, "kill "..ch.name)
      return (TRUE)
    end
  end
end
