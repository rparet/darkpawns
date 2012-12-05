function fight()

  if (ch) then
    set_hunt(me, NIL)				-- Stop mob from hunting
    if (aff_flagged(ch, AFF_MOUNT)) then
      mount(ch, NIL, "unmount")
    end

    -- If they're nightbreeds, we'll need to fight them normally
    if (aff_flagged(ch, AFF_VAMPIRE) or aff_flagged(ch, AFF_WEREWOLF)) then
      return
    end

    obj = NIL
    create_event(me, ch, NIL, NIL, "jail", 1, LT_MOB)
  end
end

function onpulse_pc()
  dofile("scripts/mob/cityguard.lua")
  call(onpulse_pc, me, "x")
end

function jail()
  local guard = { [5302] = 5365, [8001] = 8062, [8002] = 8062, [8020] = 8062, [8027] = 8062,
                  [8059] = 8062, [18215] = 18290, [18223] = 18290, [18224] = 18290,
                  [21227] = 21226 }

  if ((ch.jail == 0) and (ch.pos > POS_INCAP)) then
    act("$n grabs $N by the collar, and quickly beats $M into submission.\r\nJerking $M to $S feet, $n carts $N off"
        .." to jail.", TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n grabs you by the collar and quickly beats you into submission.",
      TRUE, me, NIL, ch, TO_VICT)
    act("Jerking you to your feet, he carts you off to jail...\r\n", TRUE, me, NIL, ch, TO_VICT)

    tport(ch, guard[me.vnum])
    ch.hp = 1
    ch.jail = 1 + (ch.level / 20)
    ch.pos = POS_STANDING
    save_char(ch)
  end
end
