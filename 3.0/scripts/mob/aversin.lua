function sound()
  emote("looks at you.")
  say("Carry on, citizen")
end

function fight()
  dofile("scripts/mob/take_jail.lua")
  call(fight, me, "x")
end

function onpulse_pc()
  dofile("scripts/mob/guard_captain.lua")
  call(onpulse_pc, me, "x")
end
