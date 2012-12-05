function sound()
  if (number(0, 5) == 0) then
    say("My, this is tough work.")
    emote("wipes the sweat from $s brow.")
  elseif (number(0, 5) == 0) then
    say("Greetings friend, welcome to city of Mist Keep.")
  elseif (number(0, 5) == 0) then
    say("Hmm, looks like rain is coming. I best finish my work soon.")
  elseif (number(0, 5) == 0) then
    emote("puffs and pants as $s plants a new row of wheat.")
  elseif (number(0, 5) == 0) then
    say("I hear there is a flour mill on the plains of Dor-Sefrith.")
  elseif (number(0, 5) == 0) then
    say("Help yourself to any wheat I have. It may be worth some gold to you.")
  end
end

function onpulse_pc()
-- If there is no wheat (obj 5300) available, collect some and place it in the field
-- for the player to take if desired. Attached to mob 5305.

  local wheat = 0

  if (not room.objs) then
    if (number(0, 9) == 0) then
      act("$n rolls a freshly collected harvest of wheat.", FALSE, me, NIL, NIL, TO_ROOM)
      oload(me, 5300, "room")
    end
  else
    for i = 1, getn(room.objs) do
      if (room.objs[i].vnum == 5300) then
        wheat = wheat + 1
      end
    end

    if (wheat < 4) then
      if (number(0, 19 * wheat) == 0) then
        act("$n rolls a freshly collected harvest of wheat.", FALSE, me, NIL, NIL, TO_ROOM)
        oload(me, 5300, "room")
      end
    end
  end
end

