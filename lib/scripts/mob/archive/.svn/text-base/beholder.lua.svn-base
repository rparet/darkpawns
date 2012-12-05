function oncmd()
-- Prevent spells from being cast in the same room as the mob. Attached to mob 12000.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (ch.level >= LVL_IMMORT) then			-- Don't affect Imms
    return
  end

  if ((command == "cast") or (command == "recite")) then
    if (subcmd == "") then
      return
    end

    act("A ray of light shoots out from one of $n's eyestalks, hitting $N!",
      TRUE, me, NIL, ch, TO_NOTVICT)
    act("A ray of light shoots out from one of $n's eyestalks, breaking your concentration!",
      TRUE, me, NIL, ch, TO_VICT)
    return (TRUE)
  end
end

function onpulse_pc()
-- Randomly cast spells on any players within the room

  local vict = NIL
  local found = FALSE
  local spells = { SPELL_SLEEP, SPELL_CHARM, SPELL_CURSE }

  if (room.char) then
    repeat                                              -- Find another victim
      vict = room.char[number(1, getn(room.char))]
      if (vict.level < LVL_IMMORT) then
        found = TRUE
      else
        return
      end
    until (found == TRUE)

    spell(vict, NIL, spells[number(1, 3)], TRUE)
  end
end

function fight()
-- Randomly cast spells on players during battle

  local spells = { SPELL_DISRUPT, SPELL_DISINTEGRATE }

  if (number(0, 10) == 0) then
    spell(ch, NIL, spells[number(1, 2)], TRUE)
  elseif (number(0, 5) == 0) then
    spell(ch, NIL, spells[number(1, 2)], TRUE)
  end
end

