function fight()
-- Allow the mob to cast sorcery spells during a fight.

  dofile("scripts/mob/sorcery.lua")
  call(fight, me, "x")
end

function death()
-- Reset any global variables that allow the "conversation" to begin again
-- after zone reset.

  keep_bane_dead = TRUE

  if ((keep_bane_dead == TRUE) and (keep_val_dead == TRUE)) then
    keep_conv_state = 0
    keep_conv_over = FALSE
    keep_bane_dead = FALSE
    keep_val_dead = FALSE
  end
end

function greet()
-- Let the players know they don't need to be in this location when the
-- mobs are already positioned there.

  if (keep_conv_over == TRUE) then
    act("\r\n$n says, 'I will stand in this position. Provided the other points "
      .."of the star have been occupied, you are welcome to watch from here.",
      TRUE, me, NIL, ch, TO_VICT)
  end
end

function onpulse_pc()
-- Bane (1408) and Valoran (1407) will have a "conversation" between themselves
-- and the player.

  local char = NIL

  if (keep_conv_over == TRUE) then		-- Conversation is finished
    return
  end

  if (keep_conv_state == 0) then
    keep_people = 0				-- Re-initialise globals
    keep_focus_owner = NIL
    obj = NIL
    argument = NIL

    for i = 1, getn(room.char) do		-- How many people are there in here?
      char = room.char[i]
      if (not isnpc(char)) then
        keep_people = keep_people + 1
        if (char.objs) then
          for j = 1, getn(char.objs) do	-- Is there a daemonic focus?
            if (char.objs[j].vnum == 1406) then
              keep_focus_owner = char
              break
            end
          end
        end
      end
    end

    say("Well met "..(keep_people > 1 and "friends" or "friend")..", we do not receive "
      .."many visitors these days.")
    keep_conv_state = 1
    create_event(me, ch, NIL, NIL, "bane_one", 10, LT_MOB)
  elseif (keep_conv_state == 4) then
    keep_conv_state = 5
    create_event(me, ch, NIL, NIL, "bane_two", 6, LT_MOB)
  end
end

function bane_one()
  if (keep_conv_state == 2) then
    say("Indeed, and it appears "..keep_focus_owner.name.." has the focus necessary "
      .."for the ceremony.")
    create_event(me, ch, NIL, NIL, "bane_two", 16, LT_MOB)
  else
    say("Hmmm...it would seem that "..(keep_people > 1 and "none of you" or "you do not").." "
      .."possess the focus necessary for the ceremony. Return to us when you feel you are ready.")
    act("\r\n$n mumbles a few words and your vision fades...\r\n",
      TRUE, me, NIL, NIL, TO_ROOM)
    keep_conv_state = 3

    for i = 1, getn(room.char) do		-- Get everybody out
      if (not isnpc(room.char[i])) then
        tport(room.char[i], 1412)
      end
    end
  end
end

function bane_two()
  say("Very well, listen young "..(keep_people > 1 and "acolytes" or "acolyte")..", for I "
    .."shall speak of this but once.")
  create_event(me, ch, NIL, NIL, "bane_three", 6, LT_MOB)
end

function bane_three()
  say("The ceremony of summoning is a dangerous one with unpredictable results. "
    .."Although you possess the daemonic focus which has within it the ability "
    .."to banish even the greatest of daemons, the spell is not always successful. "
    .."The circle wthin this room is capable of containing the foul beasts for "
    .."only a short period of time - should you be unsuccessful in banishing the "
    .."creature, it will not be long before it roams free and you must undertake "
    .."the task of killing it by mortal means.")
  create_event(me, ch, NIL, NIL, "bane_four", 10, LT_MOB)
end

function bane_four()
  say("The ceremony has not been performed for some time as our numbers grow small "
    .."and the danger is just too great. The rewards, however, are equally as great "
    .."- with each ceremony, this is a strong likelihood that the foul beast shall "
    .."bring forth enchanted items from its own world. These in turn will assist "
    .."the bearer in battle against other such beasts.")
  create_event(me, ch, NIL, NIL, "bane_five", 10, LT_MOB)
end

function bane_five()
  local char = NIL
  local buf = ""

  keep_people = 0					-- Re-initialise globals
  keep_focus_owner = NIL

  for i = 1, getn(room.char) do		-- How many people are there in here?
    char = room.char[i]
    if (not isnpc(char)) then
      keep_people = keep_people + 1
      if (char.objs) then
        for j = 1, getn(char.objs) do	-- Is there a daemonic focus?
          if (char.objs[j].vnum == 1406) then
            keep_focus_owner = char
            break
          end
        end
      end
    end
  end

  if (keep_people > 3) then
    act("$n says, 'It appears that we have a sufficient number of people to perform "
      .."the ceremony. You should all position yourselves at a point of the star "
      .."within this room and place a bloodwax candle at your feet. Failure to do "
      .."either will prevent the ceremony from continuing. When everything is in "
      .."place, $N must stand on the small dias to the southeast and use the focus "
      .."$E holds - this will begin the ceremony and, if the gods are with us, "
      .."banish the daemon when it is over.'", TRUE, me, NIL, keep_focus_owner, TO_ROOM)
  else
    if (keep_people > 2) then
      buf = "person"
    else
      buf = "people"
    end

    act("$n says, 'Unfortunately, we require "..(4 - keep_people).." more "..buf.." to "
      .."begin the ceremony. When you are able to gather the numbers, you should "
      .."position yourselves at a point of the star within this room and place a "
      .."bloodwax candle at your feet. Failure to do either will prevent the ceremony "
      .."from continuing. When everything is in place, $N must stand on the small "
      .."dias to the southeast and use the focus $E holds - this will begin the "
      .."ceremony and, if the gods are with us, banish the daemon when it is over.'",
      TRUE, me, NIL, keep_focus_owner, TO_ROOM)
  end
  create_event(me, ch, NIL, NIL, "bane_six", 10, LT_MOB)
end

function bane_six()
  say("Now, I too must prepare for the ceremony. Valoran and I will stand at a point "
    .."of the star each. The remainder of you should take up a vacant position and "
    .."wait. Good luck to us all.")
  act("\r\n$n leaves to the east.", TRUE, me, NIL, NIL, TO_ROOM)
  tport(me, 1439)
  act("$n arrives from the south.", TRUE, me, NIL, NIL, TO_ROOM)

  keep_conv_over = TRUE
end
