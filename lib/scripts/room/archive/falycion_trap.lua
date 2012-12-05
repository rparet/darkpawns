function oncmd()
-- There will be a 30% chance that the player will walk into an unseen spider web and be
-- pulled into a start point within zone 18 (Falycion Spider Tunnels). They will be
-- paralyzed as part of the transfer. Attached to rooms 6806, 6821, 6835, 7031, 7038 and
-- 7064.

  local command = ""
  local buf = ""
  local skeleton = NIL
  local scroll = NIL
  local room_num = number(1801, 1899)
  local location = { 6806, 6821, 6835, 7038, 7064, 7031 }
  local new_location = { 1801, 1802, 1807, 1890, 1898, 1882 }

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  -- Only affect PCs who move in a cardinal direction
  if (isnpc(me)) then
    return
  end

  if ((command == "north") or (command == "south") or (command == "west") or (command == "east")) then
    if (number(0,3) == 0) then
      for i = 1, getn(location) do
        if (room.vnum == location[i]) then
          buf = "Before you realize, you're stuck in some sort of web! The last thing you feel are"
              .." the fangs sinking into your neck...\r\n"
          act(buf, FALSE, me, NIL, NIL, TO_CHAR)
          buf = "$n runs into an unseen web and is stuck! In the blink of an eye, a creature"
              .." emerges from the ground and drags $m beneath the surface."
          act(buf, TRUE, me, NIL, NIL, TO_ROOM)
          spell(me, NIL, SPELL_PARALYSE, FALSE)		-- Paralyse the player

          tport(me, new_location[i])
          act("$n appears out of nowhere and collapses on the ground in front of you!",
            TRUE, me, NIL, NIL, TO_ROOM)

          skeleton = oload(me, 1800, "room")	-- Load a skeleton
          objfrom(skeleton, "room")
          objto(skeleton, "room", room_num)	-- Move the skeleton to a random location
          scroll = oload(me, 8052, "char")	-- Load a scroll of recall (ensures player can get out)
          objfrom(scroll, "char")
          objto(scroll, "obj", skeleton)	-- Put the scroll in the skeleton
                    
          return (TRUE)
        end
      end
    end
  end      
end

