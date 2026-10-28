function fight()
-- The mob has a 10% chance of causing a poisonous bite to their attacker. Attached
-- to mob 1700.

  if (number(0,9) == 0) then
    if (not isnpc(ch)) then
      act("$n gives you a painful bite on the arm.", TRUE, me, NIL, ch, TO_VICT)
      act("$n gives $N a painful bite on the arm.", TRUE, me, NIL, ch, TO_NOTVICT)
      spell(ch, NIL, SPELL_POISON, FALSE)
    end
  end
end
