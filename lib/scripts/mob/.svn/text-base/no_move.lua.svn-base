function no_move(dir)
-- Prevent a player from moving in a specific direction as determined by the
-- amount of gold the mobile possesses:
--   North is 0
--   East is 1
--   South is 2
--   West is 3

  local command = ""
  local block = FALSE

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

--   if ((ch.level >= LVL_IMMORT) or ishunt(ch)) then
--     return
--   end

  if ((command == "north") and (dir == NORTH)) then
    block = TRUE
  elseif ((command == "east") and (dir == EAST)) then
    block = TRUE
  elseif ((command == "south") and (dir == SOUTH)) then
    block = TRUE
  elseif ((command == "west") and (dir == WEST)) then
    block = TRUE
  end

  if (block == TRUE) then
    act("$n blocks $N's way.", TRUE, me, NIL, ch, TO_NOTVICT)
    act("$n blocks your way.", TRUE, me, NIL, ch, TO_VICT)
    say("I cannot let you pass.")
    return (TRUE)
  end
end
