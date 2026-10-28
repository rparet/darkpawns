function onget()
-- If a player were to get one of the elemental talismans from the room, the corresponding
-- cylinder of light would disappear. Attached to rooms 1360, 1364, 1380 and 1384.

  local talisman = { 1300, 1301, 1302, 1303 }
  local location = { 1360, 1364, 1380, 1384 }
  local cylinder = { "green", "yellow", "red", "blue" }
  local found = FALSE
  local inroom = 0
  local buf = ""

  for i = 1, getn(location) do			-- Where am I?
    if (room.vnum == location[i]) then
      inroom = i
    end
  end

  if (room.objs) then
    for i = 1, getn(room.objs) do
      if (room.objs[i].vnum == talisman[inroom]) then	-- A talisman is still in the room
        found = TRUE
      end
    end
  end

  if (found ~= TRUE) then				-- Talisman is not in the room, remove the cylinder
    buf = "The "..cylinder[inroom].." cylinder of light slowly sinks back into the pillar."
    if (obj_list(cylinder[inroom], "room")) then
      act(buf, TRUE, NIL, obj, NIL, TO_ROOM)
      extobj(obj)
    end
  end
end

function ondrop()
-- If a player drops the correct elemental talisman in the appropriate room, a corresponding
-- cylinder of light will extend.

  local talisman = { 1300, 1301, 1302, 1303 }
  local location = { 1360, 1364, 1380, 1384 }
  local cylinder = { "green", "yellow", "red", "blue" }
  local cyl_vnum = { 1304, 1305, 1306, 1307 }
  local found = 0
  local inroom = 0

  for i = 1, getn(room.objs) do			-- Is there a talisman here already?
    for j = 1, getn(talisman) do
      if (room.objs[i].vnum == talisman[j]) then
        found = found + 1
      end
    end
  end

  if (found == 1) then					-- No talisman, continue
    for i = 1, getn(location) do			-- Where am I?
      if (room.vnum == location[i]) then
        inroom = i
      end
    end

    if (obj.vnum == talisman[inroom]) then	-- The correct talisman was dropped
      oload(me, cyl_vnum[inroom], "room")
      buf = "A "..cylinder[inroom].." cylinder of light extends upwards from the pillar."
      act(buf, TRUE, NIL, obj, NIL, TO_ROOM)
    end
  end
end
