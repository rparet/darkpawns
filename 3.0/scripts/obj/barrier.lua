function oncmd()
-- Once present, this object prevents players from passing to the west. The barrier
-- shall be destroyed upon the sorcerer's death.

  local command = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (command == "west") then
    act("The magical barrier prevents passage to the west.", TRUE, ch, NIL, NIL, TO_CHAR)
    return (TRUE)
  end
end

function onpulse()
-- Remove the barrier if the sorcerer is dead.

  local found = FALSE

  if (room.char) then
    for i = 1, getn(room.char) do
      if (room.char[i].vnum == 1404) then
        found = TRUE
      end
    end
  end

  if (found ~= TRUE) then
    act("The magical barrier disappears in a blaze of light.", FALSE, NIL, obj, NIL, TO_ROOM)
    extobj(obj)
  end
end
