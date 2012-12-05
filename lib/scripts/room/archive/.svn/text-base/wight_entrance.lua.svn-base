function enter()
-- When a player enters the room, if they are affected by DETECT MAGIC, they will be
-- able to see the entrance to the Wight Island zone. Attached to room 3703.

  if (aff_flagged(me, AFF_DETECT_MAGIC)) then		-- The player can "see" the entrance
    create_event(me, NIL, NIL, NIL, "entrance", 1, LT_ROOM)
  end
end

function entrance()
-- Display the entrance text as an addition to the room description.

  local buf = mxp("stone", "look stone")

  act("The top of the "..buf.." shimmers briefly and a passageway appears, leading into\r\n"
    .."the depths of the island.", FALSE, me, NIL, NIL, TO_CHAR)
end

function oncmd()
  local command = ""
  local subcmd = ""
  local buf = mxp("stone", "look stone")

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (aff_flagged(me, AFF_DETECT_MAGIC)) then
    if (command == "look") then
      subcmd = strlower(subcmd)
      if (subcmd == "") then			-- "look" only, no target
        create_event(me, NIL, NIL, NIL, "entrance", 1, LT_ROOM)
      elseif ((subcmd == "stone") or (subcmd == "passage") or (subcmd == "passageway")) then
        act("The "..buf.." is approximately 10 feet in diameter and has a highly polished\r\n"
          .."surface, although there is no indication of why it is here. Your ability\r\n"
          .."to detect magic has revealed a secret entrance into the depths below.", FALSE, me, NIL, NIL, TO_CHAR)
        return (TRUE)
      end
    elseif ((command == "down") or (command == "enter")) then
      act("$n steps towards the stone and vanishes before your eyes!", TRUE, me, NIL, NIL, TO_ROOM)
      act("You step towards the stone and find yourself...\r\n", FALSE, me, NIL, NIL, TO_CHAR)
      teleport(me, 3709)
      act("$n enters from above.", TRUE, me, NIL, NIL, TO_ROOM)
      return (TRUE)
    end
  end
end
