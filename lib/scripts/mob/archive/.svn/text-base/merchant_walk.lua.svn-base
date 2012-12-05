function onpulse_all()
-- This mob will travel from the gates of Mist Keep to Xixieqi with a nominated
-- escort. Should the escort "abandon" the mob or the mob is killed, the quest
-- will fail. Attached to mob 6805.

  local dir = 0
  local buf = ""
  local position = mod(merchant_ptr, 3)

  if (merchant_conv_state ~= 0) then
    if (not escort()) then			-- Check for our escort
      extraction()
      clear_quest()
      return
    else					-- Is the escort following?
      if (merchant_yesno == FALSE) then
        if (not merchant_char.leader) then
          if ((merchant_char.pos == POS_SITTING) or (merchant_char.pos == POS_RESTING)) then
            say("This is no time to be resting.")
            act("$n pulls you to your feet.", TRUE, me, NIL, merchant_char, TO_VICT)
            act("$n pulls $N to $S feet.", TRUE, me, NIL, merchant_char, TO_NOTVICT)
            merchant_char.pos = POS_STANDING
            save_char(merchant_char)
          elseif (merchant_char.pos == POS_SLEEPING) then
            say("What does $e think $e's doing?")
            merchant_char.pos = POS_SITTING
            save_char(merchant_char)
            act("Somebody rouses you from your slumber.", FALSE, merchant_char, NIL, NIL, TO_CHAR)
            act("$n shakes $N on the shoulder to wake $M up.", TRUE, me, NIL, merchant_char, TO_NOTVICT)
            return
          end
          say(merchant_char.name..", if you follow me, we can begin our journey.")
          merchant_yesno = TRUE
          return
        end
      elseif (merchant_char.leader) then	-- Still not following, wait
        if (merchant_char.leader.name ~= me.name) then
          return
        elseif (merchant_conv_state < 2) then
          say("Excellent, then let us get underway. Try and keep up.")
          merchant_conv_state = 2
        end
      else
        return
      end
    end

    if (end_quest()) then			-- Is the quest at the end?
      extraction()
      clear_quest()
      return
    end

    attack_time()				-- Should we be attacked now?
    
    if (position == 0) then 
      dir = direction(room.vnum, 4860)
      if (dir == 0) then
        buf = "north"
      elseif (dir == 1) then
        buf = "east"
      elseif (dir == 2) then
        buf = "south"
      elseif (dir == 3) then
        buf = "west"
      elseif (dir == 4) then
        buf = "up"
      elseif (dir == 5) then
        buf = "down"
      else
        buf = NIL
      end

      if (buf) then
        action(me, buf)
      else
        say("I seem to be lost, you should inform a staff member about this!")
        act("$n turns around and disappears down the road.", TRUE, me, NIL, NIL, TO_ROOM)
        extraction()
        clear_quest()
        return
      end
    end

    merchant_ptr = merchant_ptr + 1
  end
end

function oncmd()
-- Prevent the player from sitting down, resting or sleeping - after all, they are
-- supposed to be the escort!

  local command = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if ((command == "rest") or (command == "sleep") or (command == "sit")) then
    say("We haven't got time for that, we must keep moving on!")
    return (TRUE)
  end
end

function fight()
  local attacker = isfighting(me)

  if (merchant_conv_state < 3) then
    if (attacker.name == merchant_char.name) then
      say("I have been deceived! May the gods have mercy on me!")
      action(merchant_char, "outlaw")
      merchant_conv_state = 3
    end
  end
end

function death()
  me = NIL
  clear_quest()
end

function init_quest()
-- Before the quest begins, initiate a "3 hour" delay before teleporting the 
-- quest mobiles to the start room.

  me = inworld("mob", 6805)			-- Make sure the global "me" is set
  create_event(me, NIL, NIL, NIL, "tele_mobs", 480, LT_MOB)
end

function tele_mobs()
-- Teleport the quest mob to the start point.

  tport(me, 5315)
  merchant_conv_state = 1
  act("$n arrives from the east.", TRUE, me, NIL, NIL, TO_ROOM)
end

function escort()				-- Is our escort still here?
  local found = FALSE

  for i = 1, getn(room.char) do
    if (room.char[i].name == merchant_char.name) then
      merchant_char = room.char[i]		-- Reset any player variables (ie. leader)
      found = TRUE
      break
    end
  end

  if (found == FALSE) then			-- No escort, failed quest
    say("Where has my escort gone!??! I must return to Mist Keep immediately.")
    act("$n turns around and disappears down the road.", TRUE, me, NIL, NIL, TO_ROOM)
    return (NIL)
  end

  return (TRUE)
end

function end_quest()				-- Have we reached the end of the quest?
  if (room.vnum == 4860) then
    say("Well done "..merchant_char.name..", we have made it!")
    say("As promised, here is your reward.")
    act("$n hands you 5000 gold coins and then departs into the city.",
      TRUE, me, NIL, merchant_char, TO_VICT)
    act("$n hands $N a collection of gold coins and then departs into the city.",
      TRUE, me, NIL, merchant_char, TO_NOTVICT)
    merchant_char.gold = merchant_char.gold + 5000
    save_char(merchant_char)
    return (TRUE)
  else
    return (NIL)
  end
end

function attack_time()				-- Should we initiate an attack now?
  local bandit1, bandit2, bandit3
  local arrive = number(6500, 6699)

  if (room.vnum == 7093) then
    if (bandit == FALSE) then			-- No bandits, let them loose
      bandit = TRUE
      bandit1 = mload(6506, arrive)
      bandit2 = mload(6506, arrive)
      bandit3 = mload(6506, arrive)

      set_hunt(bandit1, me)			-- Set them hunting
      set_hunt(bandit2, me)
      set_hunt(bandit3, merchant_char)
    end
  end
end
    
function clear_quest()				-- Reset all of the quest variables
  merchant_char = NIL
  bandit = FALSE
  merchant_conv_state = 0
  merchant_yesno = FALSE
  merchant_ptr = 1
end

function extraction()
-- Extract all of the non-fighting quest mobs and objs

  if (me and not isfighting(me)) then
    if (me.objs) then
      for i = 1, getn(me.objs) do
        extobj(me.objs[i])
      end
    end
    extchar(me)
  end
end

