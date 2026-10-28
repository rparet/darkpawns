function oncmd()
-- When players place the moonstone amulet (10208) and the sunstone amulet (10215) into
-- the forge (10212) and close it, a key (10216) is created and left in the forge. Attached
-- to room 10245.

  local command = ""
  local subcmd = ""
  local forge = NIL
  local moonstone = NIL
  local sunstone = NIL

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "close") then
    if (room.objs) then				-- The forge should ALWAYS be here
      for i = 1, getn(room.objs) do
        if (room.objs[i].vnum == 10212) then
          forge = room.objs[i]
          break
        end
      end
    end

    if (not forge) then
      return
    end

    if ((subcmd ~= "") and strfind(forge.alias, strlower(subcmd))) then
      if (forge.contents) then
        for i = 1, getn(forge.contents) do
          if (forge.contents[i].vnum == 10208) then
            moonstone = forge.contents[i]
          elseif (forge.contents[i].vnum == 10215) then
            sunstone = forge.contents[i]
          end
        end

        if (not moonstone and not sunstone) then
          return
        else
          extobj(moonstone)			-- Got both amulets, remove them now
          extobj(sunstone)
          create_event(me, NIL, NIL, NIL, "make_key", 0, LT_ROOM)
        end
      end
    end
  end
end

function make_key()
-- Now that the forge contains both amulets, need to make the celestial key (10216).

  local forge = NIL
  local key = NIL

  if (room.objs) then				-- The forge should ALWAYS be here
    for i = 1, getn(room.objs) do
      if (room.objs[i].vnum == 10212) then
        forge = room.objs[i]
        break
      end
    end
  end

  if (not forge) then
    return
  end

  key = oload(me, 10216, "char")		-- Load the celestial key
  objfrom(key, "char")
  objto(key, "obj", forge)

  act("$p glows softly and there is a brief flash of light and heat from within.",
    TRUE, me, forge, NIL, TO_ROOM)
  act("$p glows softly and there is a brief flash of light and heat from within.",
    TRUE, me, forge, NIL, TO_CHAR)
end

