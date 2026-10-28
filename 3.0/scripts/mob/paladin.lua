function fight()
-- Allows the mob to perform paladin skills during combat

  local case = number(0, 20)

  if ((case == 4) or (case > 5)) then
    return
  end

  if (case == 0) then
    action(me, "parry")
  elseif (case == 1) then
    action(me, "bash "..ch.alias)
  elseif (case == 2) then
    action(me, "charge "..ch.alias)
  elseif (case == 3) then
    if ((me.evil == FALSE) and (ch.evil == TRUE)) then
      spell(ch, NIL, SPELL_DISPEL_EVIL, TRUE)
    elseif ((me.evil == TRUE) and (ch.evil == FALSE)) then
      spell(ch, NIL, SPELL_DISPEL_GOOD, TRUE)
    end
  elseif (case == 5) then
    action(me, "disarm "..ch.alias)
  end
end
