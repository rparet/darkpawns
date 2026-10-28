function onpulse()
-- The object contains up to 3 giant fire ant larvae (mob 1702) which, after a certain
-- period of time, will "hatch" and move around the ant hill. Attached to obj 1700.

  local hatch = number(1, 3)
  local buf = ""
  local buf2 = ""
  local buf3 = ""

  if (obj.timer == 0) then
    obj.timer = number(200,400)
  end

  obj.timer = obj.timer - 1
  save_obj(obj)

  if (obj.timer <= 0) then
    if (hatch == 1) then
      buf = "larva"
      buf2 = "crawls"
    else
      buf = "larvae"
      buf2 = "crawl"
    end

    buf3 = "A giant fire ant egg slowly opens and "..hatch.." "..buf.." "..buf2.." out.\r\n"
    echo(room, "room", buf3)

    for i = 1, hatch do
      mload(1702, room.vnum)
    end
    extobj(obj)
  end
end
