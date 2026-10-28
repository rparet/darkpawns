function ondrop()
-- Any item dropped in these rooms is considered "lost" and cannot be retrieved. Attached to
-- rooms 19133, 19134 and 19135.

  if (room.objs) then
    for i = 1, getn(room.objs) do
      if (not iscorpse(room.objs[i])) then
        act("$p falls into the ocean and is lost forever.", TRUE, NIL, room.objs[i], NIL, TO_ROOM)
        extobj(room.objs[i])
      end
    end
  end
end

