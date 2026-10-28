function fight()
-- Perform cleric spells during battle.

  dofile("scripts/mob/cleric.lua")
  call(fight, ch, "x")
end

function oncmd()
-- Prevent the player from moving south.

  dofile("scripts/mob/no_move.lua")
  call(oncmd, ch, "x")
end
