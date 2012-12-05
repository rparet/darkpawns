function oncmd()
-- If a player looks at the mob, they are turned to stone. Attached to mobs
-- 14101 and 14102.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if ((command == "look") or (command == "examine")) then
    if ((subcmd ~= "") and strfind(strlower(me.alias), strlower(subcmd))) then
      if (number(0, 100) > number(0, 100)) then
        act("With a sound like that of a crashing wave, $N slowly turns to stone!",
          TRUE, me, NIL, ch, TO_NOTVICT)
        act("With growing horror and increasing agony, your body slowly turns to stone!",
          TRUE, ch, NIL, NIL, TO_CHAR)
        raw_kill(ch, me, SPELL_PETRIFY)
        return (TRUE)
      end
    end
  end
end

function fight()
  dofile("scripts/mob/magic_user.lua")
  call(fight, me, "x")
end
