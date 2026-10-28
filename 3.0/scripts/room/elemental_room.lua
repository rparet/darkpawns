function onpulse()
  if (ch.level < LVL_IMMORT) then
    if (room.sect == SECT_FIRE) then
      act("Your skin blackens as fire burns you alive...", TRUE, ch, NIL, NIL, TO_CHAR)
    elseif ((room.sect == SECT_EARTH) and (number(0, 9) == 0)) then
      act("Semi-molten sand rains down upon you, scorching your skin...",
           TRUE, ch, NIL, NIL, TO_CHAR)
    elseif (room.sect == SECT_WIND) then
      act("Your flesh is peeled from your bones as the forces of air pummel you...",
           TRUE, ch, NIL, NIL, TO_CHAR)
    elseif (room.sect == SECT_WATER) then
      act("You struggle for air as your lungs fill with water...",
           TRUE, ch, NIL, NIL, TO_CHAR)
    else
      act("The forces of nature slowly rip you apart...", TRUE, ch, NIL, NIL, TO_CHAR)
    end

    act("You are DYING!", TRUE, ch, NIL, NIL, TO_CHAR)
    local damage = number(50, 100)
    ch.hp = ch.hp - damage
  else
    act("You ignore the forces of nature...", TRUE, ch, NIL, NIL, TO_CHAR)
  end
end
