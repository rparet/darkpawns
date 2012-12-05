function onpulse()
-- When all 4 talismans have been placed in the appropriate locations, any player entering the
-- room is teleported to the Galeru area. Attached to room 1372.

  local rooms = {1360, 1364, 1380, 1384}
  local talismans = {1300, 1301, 1302, 1303}
  local tmp_room = { }
  local found = 0

  if (room.char) then
    for i = 1, getn(rooms) do
      tmp_room[i] = load_room(rooms[i])		-- Load all of the other rooms
    end

    for i = 1, getn(tmp_room) do
      if (tmp_room[i].objs) then
        for j = 1, getn(tmp_room[i].objs) do
          if (tmp_room[i].objs[j].vnum == talismans[i]) then
            found = found + 1
            break
          end
        end
      end
    end

    if (found ~= 4) then
      return
    end

    if (not isnpc(me)) then
      act("Four beams of light from the corners of the chamber converge around you.\r\n",
          TRUE, me, NIL, NIL, TO_CHAR)
      act("$n is struck by four beams of colored light and slowly vanishes!",
          TRUE, me, NIL, NIL, TO_ROOM)
      tport(me, 1389)
      act("$n materialises from nowhere in a swirl of colors.", TRUE, me, NIL, NIL, TO_ROOM)
    end
  end
end
