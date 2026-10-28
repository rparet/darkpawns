function onget()
-- When a player gets the moonstone amulet (10208), the room fills with water, the
-- aboleth mob (10204) and 3 eel mobs (10218) are loaded. The water will remain until
-- another player leaves room 6230 to the north. Attached to rooms 6230 and 10239.

  if ((room.vnum == 10239) and (obj.vnum == 10208)) then
    room.sect = SECT_UNDERWATER
    save_room(room)			-- Save the new sector type
    mload(10204, room.vnum)		-- Load the aboleth
    for i = 1, 3 do			-- Load 3 eels
      mload(10218, room.vnum)
    end
    buf = "With a grinding noise, channels open in the floor and water floods into the room."
      .."\r\nHorrific water serpents pour in through the channels and attack you - "
      .."It's a trap, and there's no way out!\r\n"
    echo(room, "room", buf)
  end
end


function oncmd()
  local command = ""
  local buf = ""
  local tmp_room = load_room(10239)
  local mobs = NIL

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if ((room.vnum == 6230) and (command == "north") and not isnpc(me)) then
    if (tmp_room.sect == SECT_INSIDE) then
      return
    end

    tmp_room.sect = SECT_INSIDE
    save_room(tmp_room)
    buf = "The water suddenly drains from the room leaving you soaked but alive.\r\n"
    echo(tmp_room, "room", buf)

    if (tmp_room.char) then
      for i = 1, getn(tmp_room.char) do		-- Empty the room of the water mobs
        if ((tmp_room.char[i].vnum == 10204) or (tmp_room.char[i].vnum == 10218)) then
          mobs = TRUE
          extchar(tmp_room.char[i])
        end
      end
    end

    if (mobs) then
      buf = "The serpents disappear through the drains as the water recedes.\r\n"
      echo(tmp_room, "room", buf)
    end
  end
end

function enter()
  dofile("scripts/room/load_recall.lua")
  call(enter, me, "x")
end
