function greet()
  if (number(0, 10) == 0) then
    if (ch.align > 0) then
      emote("wags its tail happily.")
    else
      emote("growls.")
    end
  end
end


function bribe()
  emote("sniffs the coins and proceeds to eat them.")
end


function ongive()
  if (obj.type == ITEM_FOOD) then
    act("$n devours $p and wags $s tail happily.", TRUE, me, obj, NIL, TO_ROOM)
    extobj(obj)
  else
    act("$n sniffs around and plays with $p for a while.", TRUE, me, obj, NIL, TO_ROOM)
    objfrom(obj, "char")
    objto(obj, "room", room.vnum);
    act("$n quickly loses interest.", TRUE, me, NIL, NIL, TO_ROOM);
  end
end
