function oncmd()
-- If a player opens the panel to the south, the mastiff (19120) will pass through
-- the opening (provided it exists) and attack the leader in the room. Attached to
-- room 19195.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "open") then				-- Did the player open the panel?
    if (subcmd ~= "panel") then
      return
    end

    create_event(me, NIL, NIL, NIL, "mastiff", 0, LT_ROOM)	-- Open the panel, continue on
    return
  end
end

function mastiff()
  local tmp_room = load_room(19192)
  local mastiff = NIL
  local vict = me

  if (tmp_room.char) then				-- Is the mastiff in there?
    for i = 1, getn(tmp_room.char) do
      if (tmp_room.char[i].vnum == 19120) then
        mastiff = tmp_room.char[i]
        break
      end
    end
  end

  if (not mastiff) then
    return
  end

  action(mastiff, "north")				-- Move the mastiff through the panel
  if (room.char) then
    for i = 1, getn(room.char) do			-- Find who the leader is
      if (not room.char[i].leader) then
        vict = room.char[i]
        break
      end
    end
  end

  action(mastiff, "kill "..vict.name)			-- Attack the leader
  return (TRUE)
end    
