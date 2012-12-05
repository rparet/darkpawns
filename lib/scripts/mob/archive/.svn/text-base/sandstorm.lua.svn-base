function fight()
-- During a fighting round, there is a 50% chance of attempting and then a 50% chance
-- for success of teleporting a PC to a random location within the zone. Attached to
-- mob 6102.

  if (number(0, 1) == 0) then
    if (number(0, 1) == 0) then
      create_event(me, NIL, NIL, NIL, "port", 1, LT_MOB)
    end
  end
end

function port()
  local vict = NIL
  local counter = 0

  if (room.char) then
    repeat					-- Locate a random PC
      vict = room.char[number(1, getn(room.char))]
      counter = counter + 1
      if (counter > 100) then
        return
      end
    until (not isnpc(vict))

    if (vict == NIL) then
      return
    end

    act("You gasp in horror as $n is drawn within the whirling funnel cloud!",
      TRUE, vict, NIL, NIL, TO_ROOM)
    act("Suddenly, you find yourself spinning violently within the whirling cloud!",
      FALSE, vict, NIL, NIL, TO_CHAR)
    act("You crash to the ground, badly battered and separated from your companions.\r\n",
      FALSE, vict, NIL, NIL, TO_CHAR)
    tport(vict, number(6101, 6299))		-- Send them to a random location in the zone
    act("From out of nowhere, $n crashes to the ground, narrowly missing you!",
      TRUE, vict, NIL, NIL, TO_ROOM)

    vict.pos = POS_STANDING			-- Required to end the fight
    vict.hp = vict.hp - 75
    save_char(vict)
  end
end

