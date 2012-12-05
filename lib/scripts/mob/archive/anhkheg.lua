function fight()
  if (number(0, 4) == 0) then				-- 20% chance of casting
    spell(ch, NIL, SPELL_ACID_BLAST, TRUE)
  end
end