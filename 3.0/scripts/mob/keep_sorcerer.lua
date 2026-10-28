function oncmd()
-- The sorcerer prevents players from continuing to the west by creating a magical
-- barrier. Attached to mob 1404.
  
  local command = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (command == "west") then
    if (not obj_list("barrier", "room")) then		-- Barrier doesn't exist, load it
      act("$n attempts to leave west.", TRUE, ch, NIL, NIL, TO_ROOM)
      act("Waving $s hand, $n creates a magical barrier.", TRUE, me, NIL, NIL, TO_ROOM)
      say("Be gone, you are not welcome here!")
      oload(me, 1421, "room")				-- Load the barrier
      return (TRUE)
    end
  end
end
