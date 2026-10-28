function oncmd()
-- Assign a global to the character who puts an item in the basin..that way,
-- the mob can attack the correct person.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "put") then
    if (subcmd ~= "") then
      if (strfind(strlower(subcmd), "bas", 1, 1) or strfind(strlower(subcmd), "dep", 1, 1) or strfind(strlower(subcmd), "pit", 1, 1)) then
        basin_char = me
      end
    end
  end
end

function onpulse()
-- If a player "offers" items by placing them in the basin (10207), the sun god
-- appears and will either attack the player if the items do not match the
-- required criteria, or drop a talisman (10215). Attached to room 10221.

  local criteria = NIL
  local item = NIL
  local num_items = 0
  local basin = NIL
  local mobile = NIL
  local mob_here = NIL
  local char_here = NIL
  local mob_obj = NIL
  local percent = number(0, 100)

  if (room.objs) then
    for i = 1, getn(room.objs) do
      if (room.objs[i].vnum == 10207) then
        basin = room.objs[i]
        break
      end
    end
  end

  if (not basin) then					-- This should never happen!
    return
  end

  if (basin.contents) then
    for i = 1, getn(basin.contents) do
      item = basin.contents[i]
      if (item.vnum ~= 1225) then
        num_items = num_items + 1
        if ((item.cost > 2000) and (item.perc_load < 15)) then	-- Does it meet the criteria?
          criteria = TRUE
        end
        act("$p vanishes in a puff of white smoke.", TRUE, me, item, NIL, TO_ROOM)
        act("$p vanishes in a puff of white smoke.", TRUE, me, item, NIL, TO_CHAR)
        extobj(item)
      end
    end

    if ((num_items == 0) or not basin_char) then
      return
    end

    if (room.char) then					-- Are the sun god and donater already here?
      for i = 1, getn(room.char) do
        if (room.char[i].vnum == 10205) then
          mob_here = TRUE
        elseif ((room.char[i].name == basin_char.name) or (basin_char.name == me.name)) then
          char_here = TRUE
        end
      end
    end

    if (mob_here or not char_here) then
      return
    end

    mobile = mload(10205, room.vnum)			-- Load the sun god and items
    mob_obj = oload(mobile, 10210, "char")
    if (mob_obj.perc_load <= percent) then
      extobj(mob_obj)
    else
      equip_char(mobile, mob_obj)
    end
    act("In a flash of flame and icy winds, the fire god materializes before you!",
      FALSE, mobile, NIL, NIL, TO_ROOM)

    if (not criteria) then				-- Doesn't match the criteria, attack!
      say("You dare to offend the gods with your trite offerings?! Prepare to die!")
      action(mobile, "kill "..basin_char.name)
      basin_char = NIL
    else
      say("Faithful one, the talisman of the Sun shall protect you in your adventures.")
      mob_obj = oload(mobile, 10215, "char")
      action(mobile, "drop "..mob_obj.name)
      basin_char = NIL
      extchar(mobile)
    end
  end
end    
