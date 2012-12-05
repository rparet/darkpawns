function ongive()
  local alias = ""

  if (obj.vnum == 4618) then
    act("$n peers at the soul, then licks $s lips.", TRUE, me, NIL, ch, TO_ROOM)
    say("This will do nicely, you may pass...")
    act("$n pops the soul into $s mouth and swallows it, a hideous screaming ringing in your ears.",
        TRUE, me, NIL, ch, TO_ROOM)
    act("$n pushes $N through the gate.", TRUE, me, NIL, ch, TO_NOTVICT)
    tport(ch, 14405)
    act("$N stumbles through the gate, pushed from the other side.", TRUE, me, NIL, ch, TO_NOTVICT)
  else
    act("$n peers at $p closely before handing it back.", TRUE, me, obj, ch, TO_ROOM)
    emote("growls, 'Are you mocking me?'")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
  end
end
