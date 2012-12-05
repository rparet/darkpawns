function fight()
-- Allow the mob to cast "sorcery" spells

  local vict = NIL
  local found = FALSE
  local type = { SPELL_BURNING_HANDS, SPELL_FIREBALL, SPELL_MIND_BAR, SPELL_DISPEL_GOOD,
                 SPELL_DISPEL_EVIL, SPELL_HARM }

  if (room.char) then
    repeat                                              -- Find another victim
      vict = room.char[number(1, getn(room.char))]
      if (vict.level < LVL_IMMORT) then
        found = TRUE
      else
        return
      end
    until (found == TRUE)
  end

  if (number(0, 6) == 0) then
    local choice = number(1, 6)
    if ((type[choice] == SPELL_DISPEL_EVIL) or (type[choice] == SPELL_DISPEL_GOOD)) then
      if ((me.evil == FALSE) and (vict.evil == TRUE)) then
        type[choice] = SPELL_DISPEL_EVIL
      elseif ((me.evil == TRUE) and (vict.evil == FALSE)) then
        type[choice] = SPELL_DISPEL_GOOD
      end
    end

    spell(vict, NIL, type[choice] , TRUE)
  end
end