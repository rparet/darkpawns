function bribe()
  emote("tips his hat and smiles a thank you.")
end

function ongive()
  if (obj.cost <= 10 or obj.type == ITEM_DRINKCON) then
    say("Thanks for helping to clean this place up...")
  else
    say("Wow, this is pretty neat, thanks!")
  end
end

function onpulse_all()
  if (room.objs) then
    for i = 1, getn(room.objs) do
      obj = room.objs[i]
      if (not iscorpse(obj) and canget(obj)) then
        act("$n picks up some trash.", FALSE, me, NIL, NIL, TO_ROOM)
        objfrom(obj, "room")
        objto(obj, "char", me)
      end
    end
  end
end

