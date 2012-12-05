function onpulse_all()
-- After a period of time, the mob will form a cocoon (obj 1701) before developing
-- into its mature stage. Attached to mob 1702.
  local cocoon = NIL

  if (not ch) then
    return
  end

  if (ch.level == 1) then
    ch.timer = number(20, 30)
    ch.level = 2
    save_char(ch)
    return
  end

  if (ch.timer == 0) then
    act("$n quickly forms a protective cocoon around itself.", TRUE, ch, NIL, NIL, TO_ROOM)
    cocoon = oload(ch, 1701, "room")
    cocoon.timer = number(150, 300)
    save_obj(cocoon)
    extchar(ch)
    ch = NIL
  end
end
