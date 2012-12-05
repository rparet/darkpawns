function onpulse()
-- Any item dropped here will be sent to a corresponding area on the main deck

  if (room.objs) then
    for i = 1, getn(room.objs) do
      act("$p falls to the deck below.", TRUE, NIL, room.objs[i], NIL, TO_ROOM);
      objfrom(room.objs[i], "room")
      objto(room.objs[i], "room", 19127)
      act("$p falls from above and lands on the deck before you.",
        TRUE, NIL, room.objs[i], NIL, TO_ROOM)
    end
  end
end
