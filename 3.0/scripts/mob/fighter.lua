function fight()
-- Allows the mob to perform fighter skills during combat

  local case = number(0, 20)
  local fighting = isfighting(me)
  local skill = ""

  if ((case == 0) or (case > 6)) then
    return
  end

  if (case == 1) then
    skill = "headbutt"
  elseif (case == 2) then
    skill = "parry"
  elseif (case == 3) then
    skill = "bash"
  elseif (case == 4) then
    skill = "berserk"
  elseif (case == 5) then
    skill = "kick"
  elseif (case == 6) then
    skill = "trip"
  end

  if (me == ch) then			-- Stop us potentially using a skill on ourselves
    return
  end

  if (not fighting) then
    return
  end

  action(me, skill.." "..fighting.name)
  return (TRUE)
end
