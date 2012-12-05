function oncmd()
-- This mob allows rapid transport between the major cities of the realm for a small fee.

  local command = ""
  local subcmd = ""
  local buf, buf2, buf3, buf4, buf5, buf6 = ""
  local inroom = { 8013, 21223, 18232, 4804, 5317 }
  local count = 0

  -- Gold format is: Drax'in, Morthis, Oshi, Xixi, Keep
  local locations = {
    ["Kir Oshi"]    = { 18232, 1500, 2500, 0, 3500, 5000 },
    ["Xixieqi"]     = { 4804, 4500, 5500, 3500, 0, 1500 },
    ["Kir Drax'in"] = { 8013, 0, 1500, 2000, 4500, 6000 },
    ["Kir Morthis"] = { 21223, 1500, 0, 3000, 5500, 6000 },
    ["Mist Keep"]   = { 5317, 6000, 6000, 5000, 1500, 0 }
  }

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (strfind(argument, "^'[%a%s]") ~= NIL) then
    command = "say"
    subcmd = skip_spaces(strsub(argument, 2))
  end

  -- List the destinations and the appropriate price
  if (command == "list") then
    buf2 = mxp("Kir Drax'in", "say Kir Drax'in")
    buf3 = mxp("Kir Morthis", "say Kir Morthis")
    buf4 = mxp("Kir Oshi", "say Kir Oshi")
    buf5 = mxp("Xixieqi", "say Xixieqi")
    buf6 = mxp("Mist Keep", "say Mist Keep")
    buf = "\r\nAvailable locations for teleport are:\r\n"
    buf = "  "..buf.."\r\n  "..buf2.."\r\n  "..buf3.."\r\n  "..buf4.."\r\n  "..buf5.."\r\n  "..buf6.."\r\n"
    buf = buf.."\r\nSimply say the name of the location to which you wish to travel."
    act(buf, FALSE, ch, NIL, NIL, TO_CHAR)
    return (TRUE)
  elseif (command == "say") then
    for i = 1, getn(inroom) do
      if (room.vnum == inroom[i]) then
        count = i
      end
    end

    for index, value in locations do
      if (strlower(subcmd) == strlower(index)) then				-- City name match
        if (room.vnum == locations[index][1]) then				-- We're already there!
          act("$n exclaims, 'You are already here!'", TRUE, me, NIL, ch, TO_VICT)
          return (TRUE)
        elseif (ch.gold < locations[index][count + 1]) then			-- Not enough gold
          buf = "$n states, 'You don't have the "..locations[index][count + 1].." coins I require."
          act(buf, TRUE, me, NIL, ch, TO_VICT)
          return (TRUE)
        else									-- We have a match...send them away!
          buf = "You mention "..index.." to $N and $E nods."
          act(buf, TRUE, ch, NIL, me, TO_CHAR)
          act("With a wave of $S hand, $N mumbles a few words and the world goes dark...\r\n", TRUE, ch, NIL, me, TO_CHAR)
          act("$n mentions "..index.." to $N and $E nods.", TRUE, ch, NIL, me, TO_ROOM)
          act("With a wave of $S hand, $N mumbles a few words and $n disappears.", TRUE, ch, NIL, me, TO_ROOM)
          tport(ch, locations[index][1])
          ch.gold = ch.gold - locations[index][count + 1]
          return (TRUE)
        end
      end
    end
  end
end
