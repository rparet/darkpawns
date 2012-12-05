function onpulse()
-- The object will spawn a mature giant fire ant after a random period of time
-- (between 10 and 20 minutes). Attached to obj 1701.

  local buf = ""

  obj.timer = obj.timer - 1
  save_obj(obj)

  if (obj.timer <= 0) then
    buf = "The strange cocoon suddenly breaks open and a mature giant fire ant emerges.\r\n"
    echo(room, "room", buf)
    extobj(obj)
    mload(1700, room.vnum)
  end
end
