function onpulse()
  if (number(0, 5) == 0) then
    if (number(0, 1) == 0) then
      act("Sparks of power drift up from the pattern where you walk.", TRUE, ch, NIL, NIL, TO_CHAR)
    end
  else
    local damage = number(3, 5)
    do_damage(damage)
  end
end
function do_damage(damage)
  if (ch.level < LVL_IMMORT) then
    if (number(0, 1) == 0) then
      act("Your strength withers with each movement you make!", TRUE, ch, NIL, NIL, TO_CHAR)
    elseif (number(0, 1) == 0) then
      act("The power of the pattern weakens you!", TRUE, ch, NIL, NIL, TO_CHAR)
    else
      act("The pattern takes its toll on your strength!", TRUE, ch, NIL, NIL, TO_CHAR)
    end
    ch.hp = ch.hp - damage
  else
    act("The pattern has no effect on you.", TRUE, ch, NIL, NIL, TO_CHAR)
  end
end

