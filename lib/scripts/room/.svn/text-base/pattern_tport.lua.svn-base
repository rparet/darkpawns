function pattern_tport()
-- teleports players to a predefined room when they say the keyword for that room.
  local locations = { 
    ["kir'draxin"] =  8008,
    ["aloazar"]    = 21258,
    ["haven"]      = 12119,
    ["kir'oshi"]   = 18203,
    ["xixieqi"]    =  4886,
    ["amber"]      =  2818,
    ["rakshasa"]   = 11295
  }
  local location = ""
  local command = ""
  local found = FALSE;
  found, t, command, location = strfind(argument, "^%s*(%w+)%s*([%w%p]+)%s*")
  if not found then
    found, t, command, location = strfind(argument, "^%s*(%p)%s*([%w%p]+)%s*")
    if not found then return (FALSE) end
  end
  if ((strlower(command) == "say") or (command == "'")) then
    location = strlower(location)
    if (locations[location]) then
      act("There is a sudden rushing sound as the power of the pattern takes you away...\r\n",
          TRUE, me, NIL, NIL, TO_CHAR)
      act("$n fades out of existance into the gathering of shadow!", TRUE, me, NIL, NIL, TO_ROOM)
      tport(me, locations[location])
      act("There is a brief sound like rushing water as $n appears out of shadow.\r\n",
          TRUE, me, NIL, NIL, TO_ROOM)
      return (TRUE)
    end
  end
  return (FALSE)
end

