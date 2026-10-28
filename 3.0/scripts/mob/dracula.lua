function fight()
  dofile("scripts/mob/magic_user.lua")
  call(fight, room.char, "x")
end

function oncmd()
  local command = ""
  local subcmd = ""

  if (ch.level >= LVL_IMMORT) then
    return
  end

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "look") then
    if ((subcmd ~= "") and strfind(me.alias, subcmd)) then
      act("You feel mesmerized...your will weakens.", FALSE, ch, NIL, NIL, TO_CHAR)
      act("$n sinks $s fangs into your neck!", TRUE, me, NIL, ch, TO_VICT)
      act("$N looks at $n, who returns the gaze intently before sinking $s fangs into $N's neck!",
          TRUE, me, NIL, ch, TO_NOTVICT)
      act("You say, 'Now I know...the blood is the life!\r\n", TRUE, ch, NIL, NIL, TO_CHAR)
      act("$n says, 'Now I know...the blood is the life!", TRUE, ch, NIL, NIL, TO_ROOM)
      if (not plr_flagged(ch, PLR_VAMPIRE) and not plr_flagged(ch, PLR_WEREWOLF)) then
        act("Your blood boils with a stinging fire...\r\n", FALSE, ch, NIL, NIL, TO_CHAR)
        plr_flags(ch, "set", PLR_VAMPIRE)
      end
    end
  end
end
