function code_one()
-- Player's entering the room will fall if they are unable to fly.

  act("You enjoy a brief moment of weightlessness and then fall to the floor below.\r\n",
      TRUE, me, NIL, NIL, TO_CHAR)
  act("$n has a surprised look on $s face as $e falls to the floor below.",
      TRUE, me, NIL, NIL, TO_ROOM)

  local room = tonumber(argument)
  tport(me, room)
  act("$n falls from above and lands in a heap in front on you.", TRUE, me, NIL, NIL, TO_ROOM)
  me.hp = me.hp - number(10,50)
end

function code_two()
-- Player's entering the room will fall if they are unable to fly. They will also drop their
-- heaviest piece of equipment and it will be "lost".

  act("You enjoy a brief moment of weightlessness and then fall to the floor below.\r\n",
      TRUE, me, NIL, NIL, TO_CHAR)
  act("$n has a surprised look on $s face as $e falls to the floor below.",
      TRUE, me, NIL, NIL, TO_ROOM)

  local room = tonumber(argument)
  tport(me, room)
  act("$n falls from above and lands in a heap in front on you.", TRUE, me, NIL, NIL, TO_ROOM)
  me.hp = me.hp - number(10,50)

  if (me.objs) then
    local heavy = 0
    for i = 1, getn(me.objs) do
      if ((me.objs[i].weight > heavy) and (me.objs[i].type ~= ITEM_KEY)) then
        heavy = me.objs[i].weight
        obj = me.objs[i]
      end
    end

    if (obj) then
      act("\nYou slowly get up and check your possessions. Something is missing!",
          TRUE, me, NIL, NIL, TO_CHAR)
      extobj(obj)
    end
  end
end
