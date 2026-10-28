function fight()
-- Allow the mob to cast sorcery spells during a fight.

  dofile("scripts/mob/sorcery.lua")
  call(fight, me, "x")
end

function death()
-- Reset any global variables that allow the "conversation" to begin again
-- after zone reset.

  keep_val_dead = TRUE

  if ((keep_bane_dead == TRUE) and (keep_val_dead == TRUE)) then
    keep_conv_over = FALSE
    keep_conv_state = 0
    keep_bane_dead = FALSE
    keep_val_dead = FALSE
  end
end

function greet()
-- Let the players know they don't need to be in this location when the
-- mobs are already positioned there.

  if (keep_conv_over == TRUE) then
    act("\r\n$n says, 'I will stand in this position. Provided the other points "
      .."of the star have been occupied, you are welcome to watch from here.'",
      TRUE, me, NIL, ch, TO_VICT)
  end
end

function onpulse_pc()
-- Bane (1408) and Valoran (1407) will have a "conversation" between themselves
-- and the player.

  if (keep_conv_over == TRUE) then		-- Conversation is finished
    return
  end

  if (keep_conv_state == 1) then
    create_event(me, NIL, NIL, NIL, "val_one", 6, LT_MOB)
  elseif (keep_conv_state == 3) then
    create_event(me, NIL, NIL, NIL, "val_not_ready", 1, LT_MOB)
  end
end

function val_one()
  local buf = ""
  local buf2 = ""
  local char = NIL

  if (keep_focus_owner) then
    char = keep_focus_owner
    if (keep_people > 1) then
      buf, buf2 = "their", "they"
    end

    act("$n asks, 'Quite true, $S presence here is a good sign. Perhaps $E could be of "
      .."assistance to us?", TRUE, me, NIL, char, TO_ROOM)
    keep_conv_state = 2
    create_event(me, NIL, NIL, NIL, "val_two", 10, LT_MOB)
  else
    say("Quite true, however I sense that our "..(keep_people > 1 and "friends are" or "friend is").." "
      .."not quite ready.")
  end
end

function val_two()
  say("Excellent! Explain the ceremony to our "..(keep_people > 1 and "friends" or "friend").." "
    .."while I make the preperations.")
  act("\r\n$n leaves to the east.", TRUE, me, NIL, NIL, TO_ROOM)
  tport(me, 1443)
  act("$n arrives from the west.", TRUE, me, NIL, NIL, TO_ROOM)
end

function val_not_ready()
  local char = NIL

  for i = 1, getn(room.char) do
    char = room.char[i]
    if (not isnpc(char)) then
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

  if (keep_focus_owner) then
    create_event(me, NIL, NIL, NIL, "val_two", 1, LT_MOB)
    keep_conv_state = 4
  else
    say("You are still not ready...you must purchase a focus from the Enchanter who "
      .."lives in the catacombs beneath this keep. It then has to be bound with a "
      .."number of spells in an empowering circle. Return to us when this has been "
      .."done.")
    act("\r\n$n mumbles a few words and your vision fades...\r\n",
      TRUE, me, NIL, NIL, TO_ROOM)

    for i = 1, getn(room.char) do		-- Get everybody out
      if (not isnpc(room.char[i])) then
        tport(room.char[i], 1412)
      end
    end
  end
end
