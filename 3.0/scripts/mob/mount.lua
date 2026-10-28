function onpulse_all()
-- When the mount no longer has a master (ie. player has abandoned them, quits
-- or dies), the mount will "return" to the stables. This will prevent numerous
-- mounts from wandering the world aimlessly. Mount will also run after a set
-- period of time, forcing the purchase of a new mount.

  if (not me.leader) then			-- No leader, time to go yet?
    if (me.timer == 0) then			-- Out of time, head home
      act("$n looks around briefly and not finding $s master, decides to leave.",
           TRUE, me, NIL, NIL, TO_ROOM)
      extchar(me)
      return (TRUE)
    end
  else						-- I have a leader, but is it time to run away?
    if (me.timer == 0) then			-- Out of time, disappear
      ch = me.leader
      if (aff_flagged(ch, AFF_MOUNT)) then
        act("$n suddenly rears up and throws you off $s back before running off.",
            TRUE, me, NIL, ch, TO_VICT)
        act("$n suddenly rears up and throws $N off $s back before running off.",
            TRUE, me, NIL, ch, TO_NOTVICT)
      else
        act("$n suddenly rears up and runs off.", TRUE, me, NIL, NIL, TO_ROOM)
      end
      aff_flags(ch, "remove", AFF_MOUNT)	-- Remove character's MOUNT flag
      aff_flags(me, "remove", AFF_CHARM)  -- Remove mount's CHARM flag (stop "guts" msg)
      extchar(me)
      me = NIL
      return (TRUE)
    end
  end
end

