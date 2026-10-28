function oncmd()
-- When a player uses the wood axe in a forest area, they are able to obtain some wood for
-- sale. There is a 20% chance that obtained wood will be "good" enough for sale. 
-- Attached to object 8029.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "use") then
    if (not obj_list(subcmd, "char") or (obj.vnum ~= 8029)) then
      return
    end

    if (room.sect ~= SECT_FOREST) then
      act("Perhaps if you were to use $p in a forest?", TRUE, me, obj, NIL, TO_CHAR)
      return(TRUE)
    end

    if (me.level > 10) then
      act("Chopping wood at your level? Go and kill something!", FALSE, me, NIL, NIL, TO_CHAR)
      return(TRUE)
    end

    local chance = number(0, 4)
    if (chance == 0) then			-- Success, we got some wood
      act("You chop away at a nearby tree and manage to collect a small bundle of wood.",
          FALSE, me, NIL, NIL, TO_CHAR)
      act("$n chops away at a nearby tree and collects a small bundle of wood.",
          TRUE, me, NIL, NIL, TO_ROOM)
      oload(me, 1221, "char")
    else
      act("You chop away at a nearby tree but the wood is of poor quality and worthless.",
          FALSE, me, NIL, NIL, TO_CHAR)
      act("$n chops away at a nearby tree but $s efforts are in vain.",
          TRUE, me, NIL, NIL, TO_ROOM)
    end

    me.move = me.move - 20
    return(TRUE)
  end
end
