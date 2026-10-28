function fight()
  local damage = number(10, 30)

  if (number(0, 3) == 0) then				-- 25% chance of hitting
    act("$n hurls a large boulder at $N, crushing $S!", TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n hurls a large boulder at you, crushing your body!", TRUE, me, NIL, ch, TO_VICT)
    ch.hp = ch.hp - damage
    save_char(ch)
  end
end