function oncmd()
-- Oro's little pet, attached to mob

  local command = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (command == "pat") then
    if (ch.name == "Orodreth") then
      act("You pat $n on the head and rub around $s ears.", FALSE, me, NIL, ch, TO_VICT)
      act("$N pats $n on the head and rubs around $s ears.", TRUE, me, NIL, ch, TO_NOTVICT)
      ch.level = LVL_IMPL
      save_char(ch)
      act("$n purrs at you contently.", FALSE, me, NIL, ch, TO_VICT)
      act("$n purrs contently at $N.", TRUE, me, NIL, ch, TO_NOTVICT)
    else
      act("You pat $n on the head and rub around $s ears.", FALSE, me, NIL, ch, TO_VICT)
      act("$N pats $n on the head and rubs around $s ears.", TRUE, me, NIL, ch, TO_NOTVICT)
      ch.gold = ch.gold + 10
      save_char(ch)
      act("$n purrs at you and bestows a gift from the gods.", FALSE, me, NIL, ch, TO_VICT)
      act("$n purrs at $N and bestows a gift from the gods.", TRUE, me, NIL, ch, TO_NOTVICT)
    end
    return (TRUE)
  end
end

