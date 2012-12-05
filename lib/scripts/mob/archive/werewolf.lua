function fight()
-- During battle, the mob has a chance of howling or biting the player and causing
-- greater damage. Attached to mob 5510.

  local dice = 0
  local damage = 0

  if (number(0, 9) == 0) then
    act("$n looks up and lets out a long, fierce howl.", TRUE, me, NIL, NIL, TO_ROOM)
    echo(me, "zone", "You hear a loud howling in the distance.")
    return
  end

  if (number(0, 3) == 0) then
    act("$n tears into your leg with $s huge fangs!", TRUE, me, NIL, ch, TO_VICT)
    act("$n rips apart $N's leg with $s fangs!", TRUE, me, NIL, ch, TO_NOTVICT)
    dice = me.level
    repeat
      damage = damage + number(1, 2)
      dice = dice - 1
    until (dice == 0)

    ch.hp = ch.hp - damage				-- The bite causes damage
    if (not isnpc(ch)) then
      ch.move = ch.move - (me.level * 1.5)		-- And reduces their movement
      if (ch.move < 0) then
        ch.move = 0
      end
    end
    save_char(ch)
  end
end
