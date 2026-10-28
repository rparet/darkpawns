function fight()
  if (number(0, 9) == 0) then			-- 10% chance of casting
    spell(ch, NIL, SPELL_FIREBALL, TRUE)
  end
end