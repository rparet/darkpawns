function fight()
  local switch = number(0, 15)

  if ((switch == 0) or (switch == 5)) then
    act("The tentacles on $n's face surge forward, wrapping around $N's head!",
         TRUE, me, NIL, ch, TO_NOTVICT)
    act("The tentacles on $n's face surge forward, wrapping around your head!",
         TRUE, me, NIL, ch, TO_VICT)
    spell(ch, NIL, SPELL_SOUL_LEECH, FALSE)
  elseif (switch == 15) then
    act("Blood runs from $N's nose and ears as $n stares intently at $M.",
         TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n stares intently at you.. you feel $m battering your mind!",
         TRUE, me, NIL, ch, TO_VICT)
    spell(ch, NIL, SPELL_PSIBLAST, FALSE)
  end
end

