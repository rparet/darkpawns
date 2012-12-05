function sound()
  if (number(0,25) == 0) then
    local obj = oload(me, 20, "room")
    obj.timer = 2
  emote("relieves itself, nearly hitting your foot.")
  end
  return (TRUE)
end
function bribe()
  local amount = tonumber(argument)
  emote("sniffs the coins and proceeds to eat them.")
  ch.gold = ch.gold - amount
  return (TRUE)
end
function greet()
  if (number(0, 5) == 0) then
    if (ch.align < 0) then
      action(me, "lick "..ch.name)
    else
      action(me, "growl "..ch.name)
    end
  end
  return (TRUE)
end
function ongive()
  if (obj.type == ITEM_FOOD) then
    act("$n devours $p and wags $s tail happily.", TRUE, me, obj, NIL, TO_ROOM)
    extobj(obj)
  else 
    act("$n sniffs around and plays with $p for a while.", TRUE, me, obj, NIL, TO_ROOM)
    emote("quickly loses interest.")
    objfrom(obj, "char")
    objto(obj, "room", room.vnum)
  end
  return (TRUE)
end

