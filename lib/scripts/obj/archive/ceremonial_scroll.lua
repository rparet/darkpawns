function oncmd()
-- The scroll can only be used in room 1410 and "creates" a spell based on the reagant
-- combination in the room. 5 bloodwax candles (1402), a focus (1407) and the necessary
-- reagants are required before a spell is "assigned" to the focus. Once all 4 spells
-- are assigned, a daemonic focus (1406) is created. Attached to obj 1400.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "recite") then
    if (not obj_list(subcmd, "char") or (obj.vnum ~= 1400)) then
      return
    end

    if (room.vnum ~= 1410) then		-- Not in the correct room, use the scroll up
      return
    end

    act("You recite $p which dissolves.", TRUE, me, obj, NIL, TO_CHAR)
    act("$n recites $p.", TRUE, me, obj, NIL, TO_ROOM)
    objfrom(obj, "char")			-- Remove it, extract it later
    objto(obj, "room", 1)

    create_event(me, ch, obj, NIL, "light_candles", 4, LT_OBJ)
    return (TRUE)
  end
end

function light_candles()
  local bone = NIL
  local ash = NIL
  local pumice = NIL
  local obsidian = NIL
  local focus = NIL
  local candles = 0
  local colour = ""
  local spell = ""

  for i = 1, getn(room.objs) do
    if (room.objs[i].vnum == 1401) then		-- Daemon Bone
      bone = room.objs[i]
    elseif (room.objs[i].vnum == 1402) then	-- Bloodwax Candle
      candles = candles + 1
    elseif (room.objs[i].vnum == 1403) then	-- Pumice
      pumice = room.objs[i]
    elseif (room.objs[i].vnum == 1404) then	-- Purified Ash
      ash = room.objs[i]
    elseif (room.objs[i].vnum == 1405) then 	-- Obsidian
      obsidian = room.objs[i]
    elseif (room.objs[i].vnum == 1407) then	-- Focus
      focus = room.objs[i]
    end
  end

  if ((candles < 5) or (focus == NIL)) then	-- Missing candles or the focus
    act("Strange, nothing seems to happen. Perhaps you're missing something?",
      FALSE, me, NIL, NIL, TO_CHAR)
    return (TRUE)
  end

  if (ash) then
    extobj(ash)
    if (pumice) then
      extobj(pumice)
      if (bone) then
        extobj(bone)
        if (obsidian) then
          extobj(obsidian)
          if (focus.val[3] == 0) then
            focus.val[3] = 1				-- Summon daemon
            colour = "orange"
            spell = "  gewbar miobar\r\n"
          end
        else
          if (focus.val[4] == 0) then
            focus.val[4] = 1				-- Banish daemon
            colour = "red"
            spell = "  vibugp miobar\r\n"
          end
        end
      else
        if (focus.val[1] == 0) then
          focus.val[1] = 1				-- Ignite candles
          colour = "yellow"
          spell = "  utbuho qibmrog\r\n"
        end
      end
    end
  elseif (obsidian) then
    extobj(obsidian)
    if (pumice) then
      extobj(pumice)
      if (focus.val[2] == 0) then
        focus.val[2] = 1				-- Endure heat
        colour = "green"
        spell = "  unsmecondus poih\r\n"
      end
    end
  end

  if (colour == "") then				-- Should something happen?
    act("A number of objects on the ground vanish but nothing happens.",
      FALSE, NIL, focus, NIL, TO_ROOM)
    return (TRUE)
  else
    act("The candles ignite, one by one in a small burst of flame.",
      FALSE, NIL, focus, NIL, TO_ROOM)
    act("A number of objects vanish in a flash and $p glows a soft "..colour.." color.",
      FALSE, NIL, focus, NIL, TO_ROOM)
  end

  if (not obj_flagged(focus, ITEM_GLOW)) then	-- Add the "spell words"
    obj_extra(focus, "set", ITEM_GLOW)
    local buf = "\r\nAround its circumference are written the words:\r\n"..spell
    extra(focus, buf)
  else
    extra(focus, spell)
  end

  local num_spells = 0					-- Is the focus complete?
  for i = 1, getn(focus.val) do
    if (focus.val[i] == 1) then
      num_spells = num_spells + 1
    end
  end

  if (num_spells == 4) then				-- Complete, create the daemonic focus
    oload(me, 1406, "room")
    act("A burst of flame surrounds $p and then recedes.", FALSE, NIL, focus, NIL, TO_ROOM)
    extobj(focus)
  else
    save_obj(focus)
  end

  create_event(NIL, ch, obj, NIL, "extinguish_candles", 8, LT_OBJ)
end

function extinguish_candles()
  act("With the ceremony complete, the candles extinguish one by one.",
    FALSE, ch, NIL, NIL, TO_ROOM)
  act("With the ceremony complete, the candles extinguish one by one.",
    FALSE, ch, NIL, NIL, TO_CHAR)
  extobj(obj)
end
