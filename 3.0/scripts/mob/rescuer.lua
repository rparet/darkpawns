function onpulse_pc()
-- The mob will "rescue" the first fighting mob (fighting a player) it comes across.
-- Attached to mob 7909.

  local vict = NIL
  local alias = ""

  if (room.char) then
    for i = 1, getn(room.char) do
      vict = isfighting(room.char[i])
      if (vict) then
        if (isnpc(room.char[i]) and not isnpc(vict)) then
          if (strfind(room.char[i].alias, "%a%s")) then
            alias = strsub(room.char[i].alias, 1, strfind(room.char[i].alias, "%a%s"))
          else
            alias = room.char[i].alias
          end
          action(me, "rescue "..alias)
        end
      end
    end
  end
end
