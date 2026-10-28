function fight()
-- The mob will cast a spell (word of recall, teleport) on the player it is
-- fighting. Attached to mob 7919.

  local case = number(0, 20)

  if (case == 0) then
    act("$n touches $N on the arm.", TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n touches you lightly on the arm.", TRUE, me, NIL, ch, TO_VICT)
    create_event(me, ch, NIL, NIL, "word", 1, LT_MOB)
  elseif (case == 20) then
    say("You have violence, but not thought.")
    create_event(me, ch, NIL, NIL, "teleport", 1, LT_MOB)
  end
end

function teleport()
  spell(ch, NIL, SPELL_TELEPORT, TRUE)
  ch.pos = POS_STANDING			-- Required to end fight
  save_char(ch)
end

function word()
  spell(ch, NIL, SPELL_WORD_OF_RECALL, TRUE)
  ch.pos = POS_STANDING			-- Required to end fight
  save_char(ch)
end
