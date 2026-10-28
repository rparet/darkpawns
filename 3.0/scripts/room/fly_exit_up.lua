function oncmd()
-- If a player/mob cannot fly, they will not be able to travel upwards in these rooms.

  local command = 0
  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (me.level >= LVL_IMMORT) then			-- Don't affect Imms
    return
  end

  if (command == "up") then
    if (isnpc(me) or not aff_flagged(me, AFF_FLY)) then
      act("You try and jump up there but it's just too high.", TRUE, me, NIL, NIL, TO_CHAR)
      act("$n jumps up and down in a vain attempt to travel upwards.", TRUE, me, NIL, NIL, TO_ROOM)
      return (TRUE)
    end
  end
end
