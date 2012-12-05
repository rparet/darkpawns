function enter()
-- If a player's equipment has a gross total weight above a set amount, they are given
-- a warning - a death trap will apply in the next room. Attached to rooms 2224 and 2324.

  local weight = 0

  if ((me.level >= LVL_IMMORT) or (aff_flagged(me, AFF_FLY)) or isnpc(me)) then
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
    act("\r\nYour weight causes one of the sandstone blocks to move markedly!",
      FALSE, me, NIL, NIL, TO_CHAR)
  end
end
