function onpulse_pc()
-- The mob will steal items from a player's inventory without being noticed.

  local vict = NIL

  if ((room.char) or (me.pos ~= POS_STANDING)) then
    for i = 1, getn(room.char) do
      vict = room.char[i]
      if (not isnpc(vict) and (vict.level < LVL_IMMORT) and (number(0, 4) == 0)) then
        if ((vict.objs) and (not isfighting(vict))) then
          for j = 1, getn(vict.objs) do			-- What do they have?
            if (canget(vict.objs[j])) then
              local percent = number(1, 101)
              if (vict.pos < POS_SLEEPING) then
                percent = -1				-- Instant success
              end
              if (percent < number(30, 100)) then
                steal(vict, vict.objs[j])			-- Steal the item
              end
            end
          end
        end
      end
    end
  end
end
