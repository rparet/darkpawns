function fight()
  if (number(0, 5) == 0) then				-- 1/6 chance of hitting
    act("$n's sends a volley of quills flying, piercing $N's flesh!",
        TRUE, me, NIL, ch, TO_NOTVICT)
    act("A volley of quills fly from $n, piercing your flesh!",
        TRUE, me, NIL, ch, TO_VICT)
    local damage = number(1, 10)
    ch.hp = ch.hp - damage
    save_char(ch)
  end
end
