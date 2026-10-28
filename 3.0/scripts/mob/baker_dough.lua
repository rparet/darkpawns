function ongive()
-- Should the player give the mob a roll of dough (obj 8015), the baker will produce
-- a loaf of bread free of charge and reward the player for their efforts.

  local alias = ""

  if (obj.vnum == 8015) then
    say("Excellent! I was wondering when my next shipment would arrive.")
    act("$n hands you 20 gold coins in appreciation.\r\n", TRUE, me, NIL, ch, TO_VICT)
    act("$n hands $N some gold coins.\r\n", TRUE, me, NIL, ch, TO_NOTVICT)
    ch.gold = ch.gold + 20

    say("Please, take this for your efforts.")
    act("$n pulls a freshly baked loaf of bread from the oven and hands it to you.",
         TRUE, me, NIL, ch, TO_VICT)
    act("$n pulls a freshly baked loaf of bread from the oven and hands it to $N.",
         TRUE, me, NIL, ch, TO_NOTVICT)

    oload(ch, 8010, "char")
    return (TRUE)
  else
    say("I'm sorry, I have no use for that...you better keep it.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end
end
