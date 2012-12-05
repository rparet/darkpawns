function onpulse()
-- When a player retrieves the scroll of recall (obj 8052) from the skeleton (obj 1800),
-- it will disintegrate into dust (obj 18).

  if (not obj.contents) then
    act("$p disintegrates into a pile of dust.", TRUE, NIL, obj, NIL, TO_ROOM)
    oload(me, 18, "room")
    extobj(obj)
  end
end
