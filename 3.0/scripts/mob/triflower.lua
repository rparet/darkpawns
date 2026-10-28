function onpulse_pc()
-- If a player is in the room, the plant will attemp to sleeper the player.
-- Attached to mob 20310.

  local vict = room.char[number(1, getn(room.char))]    -- Affects last player in room

  if (number(0, 3) == 0) then                           -- 25% chance of casting
    if (not isnpc(vict) and (vict.level < LVL_IMMORT)) then
      if (vict.pos ~= POS_SLEEPING) then
        act("Orange pollen drifts down upon you... suddenly you feel faint.", TRUE, me, NIL, vict, TO_VICT)
        act("Orange pollen falls upon $N and $E collapses to the ground.", TRUE, me, NIL, vict, TO_NOTVICT)
        spell(vict, NIL, SPELL_SLEEP, FALSE)
      end
    end
  end
end

function fight()
  local burn_ch = NIL                                   -- Affect random player in room
  local found = FALSE

  if (room.char) then
    repeat                                              -- Find another victim
      burn_ch = room.char[number(1, getn(room.char))]
      if (burn_ch.level < LVL_IMMORT) then
        found = TRUE
      else
        return
      end
    until (found == TRUE)
  end

  if (number(0, 2) == 0) then					  -- 30% chance of damage
    act("A yellow blossom bends down, showering you with sticky yellow enzyme.",
        TRUE, me, NIL, burn_ch, TO_VICT);
    act("A shower of yellow enzyme falls upon $N, burning $S exposed flesh.",
        TRUE, me, NIL, burn_ch, TO_NOTVICT);
    burn_ch.hp = burn_ch.hp - number(10,30)             -- Damage to random player
    save_char(burn_ch)
  end
end

