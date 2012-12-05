function onpulse_pc()
-- As soon as the conjured mob is no longer charmed, it will disappear from the
-- world.

  if (not aff_flagged(me, AFF_CHARM)) then
    if (me.vnum >= 81 and me.vnum <= 84) then
      if (me.leader) then
        act("You lose control and $N fizzles away!", TRUE, me.leader, NIL, me, TO_CHAR)
        act("$n returns to $s own plane of existance.", TRUE, me, NIL, NIL, TO_ROOM)
      end
    else
      say("My work here is done.")
      act("$n disappears in a flash of white light!", TRUE, me, NIL, NIL, TO_ROOM)
    end
    extchar(me)
  end
end
