function sound()
  if (number(0, 5) == 0) then
    say("Hmm, I don't often have guests in my house. Welcome!")
  elseif (number(0, 5) == 0) then
    emote("yawns tiredly and stretches $s arms.")
  elseif (number(0, 5) == 0) then
    say("If you can bring me some wood, I'll pay you gold for it.")
  elseif (number(0, 5) == 0) then
    say("I wonder if it will rain tomorrow, the trees certainly need it.")
  elseif (number(0, 5) == 0) then
    say("Now where did I put my axe...")
  end
end

function ongive()
-- Should the player give the forester some wood, they shall be rewarded 50 gold coins
-- for their efforts. Attached to mob 9160.

  local alias = ""

  if (obj.vnum ~= 1221) then
    say("I'm sorry, I have no use for that...you better keep it.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  say("Thank you, you've saved me some work.")
  act("$n hands you 50 gold coins in appreciation.", TRUE, me, NIL, ch, TO_VICT)
  act("$n hands $N some gold coins.", TRUE, me, NIL, ch, TO_NOTVICT)
  ch.gold = ch.gold + 50

  extobj(obj)
  obj = NIL
end
