function oncmd()
-- Provided the player has the "treasure" map (obj 9139) and digs in the location
-- of the lone tree (obj 20300), they will uncover the treasure (obj 20306).

  local command = ""
  local chest = NIL
  local treasure = NIL
  local found = FALSE

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (command == "dig") then
    if (not ch.wear) then			-- Idiot check for a shovel first
      return
    else
      for i = 1, getn(ch.wear) do
        if (strfind(ch.wear[i].alias, "shovel")) then
          found = TRUE
          break
        end
      end
    end

    if (found == FALSE) then
      return
    end

    if (spiritwood_tree == TRUE) then	-- We only want 1 chest!
      return
    end
   
    for i = 1, getn(ch.objs) do	-- Let's look for the map
      if (ch.objs[i].vnum == 9139) then
        chest = oload(ch, 20306, "room")
        for j = 1, 4 do
          treasure = oload(ch, number(422,435), "room")
          if (treasure.perc_load > number(0, 100)) then
            objfrom(treasure, "room")
            objto(treasure, "obj", chest)
          else
            extobj(treasure)
          end
        end

        act("$n digs up $p.", TRUE, ch, chest, NIL, TO_ROOM)
        act("You dig up $p.", TRUE, ch, chest, NIL, TO_CHAR)
        act("$p crumbles into dust before your eyes.", TRUE, ch, ch.objs[i], NIL, TO_CHAR)
        extobj(ch.objs[i])
        spiritwood_tree = TRUE
        return (TRUE)
      end
    end
  end
end


function onpulse()
-- If the treasure has been found, we want to remove the tree and allow it to be
-- randomly loaded elsewhere on the zone reset.

  if (spiritwood_tree == TRUE) then
    if (room.char) then
      for i = 1, getn(room.char) do
        if (not isnpc(room.char[i])) then
          return
        end
      end
    end
    extobj(obj)
    spiritwood_tree = FALSE
  end
end
