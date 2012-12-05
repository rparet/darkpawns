function fight()
  if (number(0, 5) == 0) then				-- 1/6 chance of hitting
    act("$N screams in agony as a volley of thorns flies from $n, piercing $S armor!",
        TRUE, me, NIL, ch, TO_NOTVICT)
    act("Jagged barbs whistle from beneath the leaves of $n, rending your flesh!",
        TRUE, me, NIL, ch, TO_VICT)
    local damage = number(10, 20)
    ch.hp = ch.hp - damage
  end
end
