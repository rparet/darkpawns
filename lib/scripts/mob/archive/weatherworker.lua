function fight()
  if (number(0, 4) == 0) then			-- 20% chance of casting
    spell(ch, NIL, SPELL_DISRUPT, TRUE)
  end
end