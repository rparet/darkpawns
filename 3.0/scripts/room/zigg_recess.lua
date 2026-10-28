function oncmd()
-- If the player places the correct "key" into the correct "recess", then a secret door will
-- open allowing further access. The combination array below specifies the correct [room, key]
-- combination. Attached to rooms 2211, 2311, 2222, 2322, 2231 and 2331.

  local command = ""
  local subcmd = ""
  local item = ""
  local location = ""
  local temp = ""
  local combination = { 2211, 2200, 2311, 2200, 2222, 2210, 2322, 2210, 2231, 2209, 2331, 2209 }
  local index = 0

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "put") then
    if (strfind(subcmd, "%a%s") ~= NIL) then		-- Break down the arguments specified
      item = strsub(subcmd, 1, strfind(subcmd, "%a%s")) -- This should be the item
      subcmd = gsub(subcmd, item.." ", "")
      if (strfind(subcmd, "%a%s") ~= NIL) then		-- Did they specify "in" something?
        temp = strsub(subcmd, 1, strfind(subcmd, "%a%s"))
        if (temp == "in") then
          location = gsub(subcmd, temp.." ", "")	-- This is where the item should be put
        else
          location = temp				-- A third argument specified??
        end
      else
        location = subcmd
      end
    else
      return (FALSE)
    end

    if (location ~= "recess") then			-- Did we specify the recess?
      return (FALSE)
    end

    if (not obj_list(item, "char")) then		-- Do we have the item specified?
      act("You don't seem to have that.", TRUE, me, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    for i = 1, getn(combination) do			-- Find what room we are in
      if (room.vnum == combination[i]) then
        index = i
        break
      else
        i = i + 1
      end
    end

    act("You insert $p into the small recess.", TRUE, me, obj, NIL, TO_CHAR)
    act("$n inserts $p into the small recess.", TRUE, me, obj, NIL, TO_ROOM)
    local room_one = 0
    local room_two = 0
    local room_add = 0

    if (obj.vnum == combination[index + 1]) then		-- Correct key specified
      if ((room.vnum == 2211) or (room.vnum == 2311)) then
        room_one = room
        room_two = load_room(room.exit[DOWN])
        if (not exit_flagged(room_one, DOWN, EX_CLOSED)) then	-- Door already open
          return (TRUE)
        else
          exit_flags(room_one, DOWN, "remove", EX_LOCKED)
          exit_flags(room_one, DOWN, "remove", EX_CLOSED)
          exit_flags(room_two, UP, "remove", EX_LOCKED)
          exit_flags(room_two, UP, "remove", EX_CLOSED)
          echo(room_one, "room", "There is a grinding noise as the top of one of the"
            .." altars moves to the side.\r\n")
          echo(room_two, "room", "There is a grinding noise above your head as a trapdoor opens.\r\n")
        end
      elseif ((room.vnum == 2222) or (room.vnum == 2322)) then
        if (room.vnum == 2322) then
          room_add = 100
        end
        room_one = load_room(2218 + room_add)
        room_two = load_room(room_one.exit[WEST])
        if (not exit_flagged(room_one, WEST, EX_CLOSED)) then	-- Door already open
          return (TRUE)
        else
          exit_flags(room_one, WEST, "remove", EX_LOCKED)
          exit_flags(room_one, WEST, "remove", EX_CLOSED)
          exit_flags(room_two, EAST, "remove", EX_LOCKED)
          exit_flags(room_two, EAST, "remove", EX_CLOSED)
          echo(room_one, "room", "The western wall slowly sinks into the floor revealing a passage.\r\n")
          echo(room_two, "room", "The eastern wall slowly sinks into the floor revealing a passage.\r\n")
          echo(room, "room", "There is a simple click and nothing more.\r\n")
        end
      elseif ((room.vnum == 2231) or (room.vnum == 2331)) then
        if (room.vnum == 2331) then
          room_add = 100
        end
        room_one = load_room(2227 + room_add)
        room_two = load_room(room_one.exit[NORTH])
        if (not exit_flagged(room_one, NORTH, EX_CLOSED)) then	-- Door already open
          return (TRUE)
        else
          exit_flags(room_one, NORTH, "remove", EX_LOCKED)
          exit_flags(room_one, NORTH, "remove", EX_CLOSED)
          exit_flags(room_two, SOUTH, "remove", EX_LOCKED)
          exit_flags(room_two, SOUTH, "remove", EX_CLOSED)
          echo(room_one, "room", "The northern wall slowly sinks into the floor revealing a passage.\r\n")
          echo(room_two, "room", "The southern wall slowly sinks into the floor revealing a passage.\r\n")
          echo(room, "room", "There is a simple click and nothing more.\r\n")
        end
      end
    end

    return (TRUE)
  end
end
