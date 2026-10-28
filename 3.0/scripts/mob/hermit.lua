function greet()
-- The mob will greet any players that enter the room and do not have a master.

  if (not ch.leader) then
    say("Have a seat there! Stay a while, rest your bones and warm your feet by the fire!")
  end
end

