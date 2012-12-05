function fight()
-- Percentage chance (as shown) that the mob will attempt to drown the player

  if (number(0, round(3 + ch.level/33))) == 0 then
    act("Your lungs fill with water as $n pulls your head beneath the surface.",
        TRUE, me, NIL, ch, TO_VICT);
    local damage = number(0, 25)
    ch.hp = ch.hp - damage
    save_char(ch)
  end
end
