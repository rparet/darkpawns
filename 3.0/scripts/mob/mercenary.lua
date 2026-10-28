function bribe()
-- When given a percentage of the mob's level in gold, this mob will follow the player and
-- assist as a charmed ally.

  local amount = tonumber(argument)

  if (me.leader) then
    act("$n has already sworn allegiance to another.", TRUE, me, NIL, NIL, TO_ROOM)
    ch.gold = ch.gold + amount
    return
  end

  if (amount >= (100 * ch.level)) then			-- Was enough gold given as a bribe?
    act("$n counts the coins then secrets them away.", TRUE, me, NIL, ch, TO_VICT)
    act("$n swears his allegiance to you.", TRUE, me, NIL, ch, TO_VICT)
    follow(ch, TRUE)						-- Mob follows and is charmed
    act("$n hires $N.", TRUE, ch, NIL, me, TO_ROOM)
  else
    act ("$n laughs somewhat rudely.", TRUE, me, NIL, NIL, TO_ROOM)
  end
end

function onpulse_all()
-- Need to "remove" mercenaries who have been abandoned all over the world when they
-- no longer have a master.

  if ((room.vnum < 8000) or (room.vnum > 8099)) then
    if (not me.leader) then
      act("Without a leader, $n decides to return to the city.", TRUE, me, NIL, NIL, TO_ROOM)
      extchar(me)
      me = NIL
      return (TRUE)
    end
  end
end
