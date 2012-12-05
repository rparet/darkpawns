function onpulse_all()
-- Mob will eat anything gold within the room or within a corpse, or will attack any player
-- carrying or wearing a gold item. Attached to mob 9147.

  if (room.obj) then
    if (obj_list("gold", "room")) then		-- Located a "gold" item in the room
      act("$n sniffs $p before ferociously devouring it.", TRUE, me, obj, NIL, TO_ROOM);
      extobj(obj)				-- Extract the item
    end

    if (obj_list("gold", "corpse")) then 	-- Located a "gold" item in a corpse
      act("$n sniffs $p from within the corpse and devours it.", TRUE, me, obj, NIL, TO_ROOM);
      extobj(obj);				-- Extract the item
    end
  end

  if (room.char) then
    if (obj_list("gold", "vict")) then		-- Located a "gold" item on somebody
      act("You see the gleam of gold in the eyes of $n as $e charges at you.",
        TRUE, me, NIL, ch, TO_VICT);
      act("You see a gleam in the eyes of $n as $e charges at $N.",
        TRUE, me, NIL, ch, TO_NOTVICT);
      action(me, "kill "..ch.name)
    end
  end
end

