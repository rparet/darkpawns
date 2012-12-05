function enter()
-- When a player moves into this room, they are teleported to a further room based on the
-- type of talisman(s) they possess. Attached to room 1315.

  local new_loc = { 1320, 1331, 1342, 1353, 1372 }
  local talisman = { 0, 0, 0, 0 }
  local obj_name = {"earth", "air", "fire", "water"}
  local found = 1
  local buf = "You feel a tingling sensation and your vision fades. When you wake...\r\n"

  if (me.objs) then
    for i = 1, getn(me.objs) do
      if (me.objs[i].vnum == 1300) then		-- Earth talisman
        talisman[1] = TRUE
      elseif (me.objs[i].vnum == 1301) then	-- Air talisman
        talisman[2] = TRUE
      elseif (me.objs[i].vnum == 1302) then	-- Fire talisman
        talisman[3] = TRUE
      elseif (me.objs[i].vnum == 1303) then	-- Water talisman
        talisman[4] = TRUE
      end
    end

    for i = 1, getn(talisman) do			-- Which talismans do we have?
      if (talisman[i] == FALSE) then
        if (i ~= 1) then
          buf = "The talisman of "..obj_name[i-1].." glows softly and your vision fades. When you wake...\r\n"
        end
        found = i
        break
      else
        found = found + 1				-- Found another talisman
      end
   end

   if (found == 5) then					-- Char has all of the talismans
      buf = "The four talismans glow softly and your vision fades. When you wake...\r\n"
   end
 end

 act(buf, TRUE, me, NIL, NIL, TO_CHAR)
 act("$n vanishes in a brilliant flash of light.", TRUE, me, 0, 0, TO_NOTVICT)
 tport(me, new_loc[found])
 act("$n appears in a brilliant flash of light.", TRUE, me, 0, 0, TO_NOTVICT)
end
