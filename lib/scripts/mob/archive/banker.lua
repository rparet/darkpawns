function ongive()
-- Attached to each city banker, allows the players to cash in their bond certificates
-- (obj 1248) to receive gold. The amount of gold is dependant on the type of lifestyle
-- decided durng character creation.

  local alias = ""

  if (obj.vnum ~= 1248) then
    say("I'm sorry, I have no use for that...you better keep it.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  if (ch.level < 5) then
    say("I'm sorry, you'll need to be level 5 before I can accept this.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  if (ch.id ~= obj.val[2]) then
    say("Hmmm...this certificate does not belong to you "..ch.name.."!")
    act("$n files $p into a desk drawer.", TRUE, me, obj, NIL, TO_ROOM)
    extobj(obj)
    obj = NIL
    return
  end

  say("Ah, I see you're progressing well "..ch.name)
  act("$n hands $N "..1500/obj.val[1].." gold coins.", TRUE, me, NIL, ch, TO_NOTVICT)
  act("$n hands you "..1500/obj.val[1].." gold coins.", TRUE, me, NIL, ch, TO_VICT)

  ch.gold = ch.gold + (1500 / obj.val[1])
  extobj(obj)
  obj = NIL
end
