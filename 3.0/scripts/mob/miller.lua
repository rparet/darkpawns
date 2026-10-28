function sound()
  if (number(0, 5) == 0) then
    say("Hail and well met traveller. Can I be of assistance?")
  elseif (number(0, 5) == 0) then
    say("A traveller on the plains of Dor-Sefrith, I don't see those often.")
  elseif (number(0, 5) == 0) then
    emote("gathers some wheat and places it on the grindstone.")
  elseif (number(0, 5) == 0) then
    emote("collects some flour from the grindstone and places it in a sack.")
  elseif (number(0, 5) == 0) then
    say("Help yourself to the sacks of flour. The baker in Kir Drax'in pays well for it.")
  elseif (number(0, 5) == 0) then
    say("Hmmm, my supply of wheat grows small. Perhaps you could get some from me?")
  elseif (number(0, 5) == 0) then
    say("The farmers in Mist Keep produce some fine quality wheat. I would pay well for any you obtain.")
  end
end

function ongive()
-- Should the player deliver a stack of wheat (obj 5300) then they will be rewarded with 30 gold
-- coins and a sack of flour will be produced. This, in turn, can be taken to a baker to produce
-- bread.

  local alias = ""

  if (obj.vnum ~= 5300) then
    say("I'm sorry, I have no use for that...you better keep it.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  say("Thank you, you've saved me a lengthy trip.")
  act("$n hands you 30 gold coins in appreciation.", TRUE, me, NIL, ch, TO_VICT)
  act("$n hands $N some gold coins.", TRUE, me, NIL, ch, TO_NOTVICT)
  ch.gold = ch.gold + 30
  extobj(obj)

  act("$n collects enough flour from the grindstone to produce a full sack.",
    TRUE, me, NIL, NIL, TO_ROOM)
  oload(me, 15100, "room")
end

