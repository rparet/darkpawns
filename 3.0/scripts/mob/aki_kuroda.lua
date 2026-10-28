function sound()
  emote("randomly splashes paint on an empty canvas.")
end

function oncmd()
-- If a player attempts to open the "painting", they are attacked by the mob.
-- Attached to mob 12915.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "open") then			-- Did they "open" the "painting"?
    if (obj_list(subcmd, "room") and (obj.vnum == 12915)) then
      act("$n exclaims, 'Don't touch that!' before lunging towards $N.",
        TRUE, me, NIL, ch, TO_NOTVICT)
      act("$n exclaims, 'Don't touch that!' before lunging towards you.",
        TRUE, me, NIL, ch, TO_VICT)
      action(me, "kill "..ch.name)
    end
  end
end
