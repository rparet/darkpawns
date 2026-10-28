function onpulse_all()
-- The minion will destroy any talisman it picks up via the SCAVENGER flag. Attached to mob 1313.

  local talisman = { 1300, 1301, 1302, 1303 }
  local location = { 1360, 1364, 1380, 1384 }
  local cylinder = { "green", "yellow", "red", "blue" }
  local found = FALSE
  local inroom = 0
  local buf = ""

  if (me.objs) then
    if (obj_list("talisman", "char")) then		-- Found a talisman
      act("$n utters the words 'eradico paratus' and $p disintegrates.",
           TRUE, me, obj, NIL, TO_ROOM)
      extobj(obj)
    end
  end

  for i = 1, getn(location) do				-- Where am I?
    if (room.vnum == location[i]) then
      inroom = i
      break
    end
  end

  if (inroom ~= 0) then
    if (room.objs) then
      for i = 1, getn(room.objs) do
        if (room.objs[i].vnum == talisman[inroom]) then	-- A talisman is still in the room
          found = TRUE
          break
        end
      end
    end

    if (found ~= TRUE) then		-- Talisman is not in the room, remove the cylinder
      buf = "The "..cylinder[inroom].." cylinder of light slowly sinks back into the pillar."
      if (obj_list(cylinder[inroom], "room")) then
        act(buf, TRUE, NIL, obj, NIL, TO_ROOM)
        extobj(obj)
      end
    end
  end
end
