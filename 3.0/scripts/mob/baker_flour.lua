function ongive()
-- Should the player give the mob a sack of flour (obj 15100), the baker will offer
-- some gold as payment and produce dough (obj 8015) for the player. If the player wishes,
-- the baker will in turn produce bread for the player at a reduced price, or, they can
-- take the dough to the baker in Kir-Morthis.

  local alias = ""

  if (obj.vnum == 15100) then
    say("My thanks to you "..ch.name..", you have saved me a lengthy trip.")
    act("$n hands you 50 gold coins in appreciation.", TRUE, me, NIL, ch, TO_VICT)
    act("$n hands $N some gold coins.", TRUE, me, NIL, ch, TO_NOTVICT)
    ch.gold = ch.gold + 50

    act("$n goes to work and produces a roll of dough.\r\n", FALSE, me, NIL, NIL, TO_ROOM)
    say("Here you go. You can take this to the baker in Kir Morthis who is in need "
        .."of it, or I can bake some bread for you at a reduced price if you like?")
    oload(me, 8015, "char")
    action(me, "give dough "..ch.name)
    return (TRUE)
  elseif (obj.vnum == 8015) then
    if (ch.gold > 1) then
      say("Very well, some warm bread coming right up!")
      act("$n pulls a freshly baked loaf of bread from the oven and hands it to you.",
           TRUE, me, NIL, ch, TO_VICT)
      act("$n pulls a freshly baked loaf of bread from the oven and hands it to $N.",
           TRUE, me, NIL, ch, TO_NOTVICT)
      oload(ch, 8010, "char")
      ch.gold = ch.gold - 1
      return (TRUE)
    else
      say("I'm sorry, I do require some gold for my bread. Perhaps another time?")
      action(me, "give dough" ..ch.name)
      return (TRUE)
    end
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

