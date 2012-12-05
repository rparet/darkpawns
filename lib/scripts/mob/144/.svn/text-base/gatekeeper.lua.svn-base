function sound()
local case = number(0, 10)
  if (case == 1 or case == 3) then
    say("I seek the dull blackened stones in which the souls of mortals have been trapped!")
  elseif (case == 2 or case == 5) then
    say("I shall open a portal to the Grey Fortress in exchange for a soul stone.")
  end
  return (TRUE)
end

function ongive()
  if (obj.vnum ~= 9900) then
  act("$n peers at $p closely, then hands it back.",
      TRUE, me, obj, NIL, TO_ROOM)
  act("$n growls, 'Are you mocking me?'",
      TRUE, me, NIL, NIL, TO_ROOM)
    --return object
else
  act("$n peers at the soul, then licks $s lips.", TRUE, me, NIL, NIL, TO_ROOM)
  say("This will do nicely.. you may enter!")
  act("$n pops the soul into his mouth and swallows it, as a "
      .."hideous\r\n screaming rings in your ears...",
      TRUE, me, NIL, NIL, TO_ROOM)
  extobj(obj)
    obj = oload(me, 19611, "room")
    act("$n parts his gnarled hands and a shimmering black portal materializes before you!",
      TRUE, me, NIL, NIL, TO_ROOM)
    say("Enter the portal quickly! It will not last long!")
end
end

