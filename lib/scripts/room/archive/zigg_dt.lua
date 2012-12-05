function onpulse()
-- If a player's equipment has a gross total weight above a set amount, they fall through
-- the floor into a death trap. Attached to rooms 2228 and 2328.

  local weight = 0

  if ((me.level >= LVL_IMMORT) or (aff_flagged(me, AFF_FLY))) then
    return
  end

  if (me.wear) then
    for i = 1, getn(me.wear) do			-- add up worn weight
      weight = weight + me.wear[i].weight
    end
  end

  if (me.objs) then
    for i = 1, getn(me.objs) do			-- add up inventory weight
      weight = weight + me.objs[i].weight
    end
  end

  if (weight > 120) then
    act("The floor gives way beneath you with a giant crack and you fall to...",
      TRUE, me, NIL, NIL, TO_CHAR)
    tport(me, 2232)
  end
end
