function onpulse_all()
-- The mob will disappear if he is not fighting. Attached to mob 10205.

  if (me.wear) then
    for i = 1, getn(me.wear) do			-- Clear him of worn items
      extobj(me.wear[i])
    end
  end

  act("White-hot flames burst forth from the altar, and the fire god is gone!",
    FALSE, me, NIL, NIL, TO_ROOM)

  extchar(me)
end
