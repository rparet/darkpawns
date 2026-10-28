function onpulse_pc()
-- The mob will engage a player (over level 25) in conversation and request their
-- assistance to escort the mob from Mist Keep to Xixieqi. Attached to mob 5332.

  local found = FALSE
  local buf = ""
  local buf2 = ""

  if (inworld("mob", 6805)) then		-- The "travelling" merchant still exists
    return
  end

  if (not merchant_char) then			-- Currently, no assigned player
    if (room.char) then
      for i = 1, getn(room.char) do
        if ((not isnpc(room.char[i])) and (room.char[i].level >= 25)) then
          local gold = room.char[i].gold + room.char[i].bank
          if (gold < 50000) then
            ch = room.char[i]
            act("$n beckons $N over to $s table.", TRUE, me, NIL, ch, TO_NOTVICT)
            act("$n beckons you over to $s table.", TRUE, me, NIL, ch, TO_VICT)
            buf = mxp("interested?", "say Interested?")
            say("I have a proposition for you "..room.char[i].name..", if you're "..buf)
            merchant_char = ch
            break
          end
        end
      end
    end
  else
    if (room.char) then				-- Make sure escort is still here
      for i = 1, getn(room.char) do
        if (room.char[i].name == merchant_char.name) then
          found = TRUE
          break
        end
      end
    end

    if (found == FALSE) then			-- Escort has left!
      say("Hmm, obviously not interested in my proposal.")
      clear_quest()
    end
  end
end

function oncmd()
-- The mob is expecting 'yes' or 'no' answers to his questions. If he doesn't get them,
-- he will prompt the first time but expect something from then on.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (strfind(argument, "^'[%a%s]") ~= NIL) then
    command = "say"
    subcmd = skip_spaces(strsub(argument, 2))
  end

  if ((merchant_char) and (merchant_char.name == ch.name)) then
    -- Assigned player? Check commands
    if (command == "say") then
      if ((strlower(subcmd) ~= "yes") and (strlower(subcmd) ~= "no")) then
        if (merchant_yesno == FALSE) then
          create_event(me, ch, NIL, NIL, "yesno", 0, LT_MOB)
          return
        end
      elseif (strlower(subcmd) == "yes") then
        create_event(me, ch, NIL, NIL, "yes", 0, LT_MOB)
        return
      else
        create_event(me, NIL, NIL, NIL, "no", 0, LT_MOB)
      end
    end
  end
end

function death()				-- Reset global variables if he's killed
  clear_quest()
end

function yesno()				-- The player did not answer 'yes' or 'no'
  buf = mxp("yes", "say Yes")
  buf2 = mxp("no", "say No")
  say("A simple "..buf.." or "..buf2.." will suffice. What say you?")
  merchant_yesno = TRUE
end

function yes()					-- The player answered 'yes'
  if (merchant_conv_state == 0) then
    say("I seek an escort from here to the city of Xixieqi.")
    say("I offer 5000 gold coins as a reward for safe passage on the journey. Do you accept?")
    merchant_conv_state = 1
  elseif (merchant_conv_state == 1) then
    say("Excellent! Meet me at the city's entrance in 1 hour. You will recognise it by"
      .." the stone archway and sentry posts.")
    act("$n finishes $s drink, bows at you and leaves the inn.", TRUE, me, NIL, ch, TO_VICT)
    act("$n finishes $s drink, bows at $N and leaves the inn.", TRUE, me, NIL, ch, TO_NOTVICT)
    extchar(me)
    clear_quest()
    init_quest()
  end
end

function no()					-- The player answered 'no'
  say("That is a pity, I shall take my gold elsewhere.")
  act("$n finishes $s drink, stands up and leaves the inn.", TRUE, me, NIL, NIL, TO_ROOM)
  clear_quest()
  extchar(me)
end

function clear_quest()				-- Reset all of the quest variables
  merchant_char = NIL
  bandit = FALSE
  merchant_conv_state = 0
  merchant_yesno = FALSE
  merchant_ptr = 1
end

function init_quest()				-- Load second stage of escort quest
  local load_in = load_room(5476)
  local merchant = mload(6805, load_in.vnum)

  merchant_char = ch				-- Need to reset the global
  dofile("scripts/mob/merchant_walk.lua")
  call(init_quest, room, "x")
end

