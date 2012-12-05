function sound()
  if (number(0, 2) == 0) then
    emote("jiggles in your direction.")
  elseif (number(0, 5) == 0) then
    say("For a good time, give me 5 gold coins.")
    social(me, "wink")
  end
end

function bribe()
  local amount = tonumber(argument)

  if (amount > 10) then
    act("$n pulls $N into the shadows for a few minutes...you decide not to watch.",
      TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n pulls you into the shadows and gives you much more than you expected.",
      TRUE, me, NIL, ch, TO_VICT)
  else
    say("Thanks hon, but I ain't that cheap!")
  end
end

