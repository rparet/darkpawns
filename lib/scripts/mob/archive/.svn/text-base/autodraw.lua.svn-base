function code()
-- Automated drawing mob

  local buf = ""
  local people = 0

  if (autodraw_char) then			-- Draw already underway
    act("A draw is already underway, please try again later.", FALSE, me, NIL, ch, TO_VICT)
    extchar(me)
    return
  end

  act("$n appears in a flash of light.", FALSE, me, NIL, NIL, TO_ROOM)
  say("I have come at the request of "..ch.name.." to assist in the draw for "..obj.name..".")

  for i = 1, getn(room.char) do		-- Let's make sure we have people here to do it
    if (not isnpc(room.char[i])) then
      people = people + 1
    end
  end

  if (people < 2) then
    say("However, it appears as though there is no group to draw. Farewell.")
    act("$n disappears in a flash of light.", FALSE, me, NIL, NIL, TO_ROOM)
    extchar(me)
    return
  end

  buf = mxp("begin", "say Begin.")
  social(ch, "point")
  say("When you are ready, let us "..buf..".")
  autodraw_char = ch

  -- Take the item ready to be drawn
  act("$n takes $p from $N.", TRUE, me, obj, ch, TO_NOTVICT)
  act("$n takes $p from you.", TRUE, me, obj, ch, TO_VICT)
  objfrom(obj, "char")
  objto(obj, "char", me)
end

function onpulse_all()
-- If the individual who commenced the draw leaves, we'll consider the draw over.

  local found = NIL
  local buf = ""

  if (autodraw_char) then
    for i = 1, getn(room.char) do
      if (room.char[i].name == autodraw_char.name) then
        found = TRUE
      end
    end

    if (not found) then				-- The draw initiator has left
      say("It would appear my services are no longer required. Farewell.")
      if (me.objs) then
        if (inworld("char", autodraw_char.name)) then
          objfrom(me.objs[1], "char")
          objto(me.objs[1], "char", autodraw_char)
          act("$p has been returned to you and the draw has been cancelled.", FALSE, me, me.objs[1], autodraw_char, TO_VICT)
        else
          action(me, "drop "..me.objs[1].alias)
        end
      end

      autodraw_char = NIL
      drawers = { }
      act("$n disappears in a flash of light.", FALSE, me, NIL, NIL, TO_ROOM)
      extchar(me)
    else						-- Are we still waiting for numbers?
      if (me.gold > 0) then
        winner()
        autodraw_char = NIL
        drawers = { }
        act("$n disappears in a flash of light.", FALSE, me, NIL, NIL, TO_ROOM)
        extchar(me)
        return
      end
    end
  else				-- No one "initiated" the draw, yet here I am???
    act("$n disappears in a flash of light.", FALSE, me, NIL, NIL, TO_ROOM)
    drawers = { }
    extchar(me)
  end
end        

function oncmd()
  local command = ""
  local subcmd = ""
  local mob = ""
  local choice = ""
  local j = 1

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

  if (command == "say") then
    if ((strlower(subcmd) == "begin") or (strlower(subcmd) == "begin.")) then
      if (ch.name ~= autodraw_char.name) then
        argument = "Only "..autodraw_char.name.." may begin the draw."
        create_event(me, ch, NIL, argument, "do_tell", 0, LT_MOB)
        return
      end

      if (me.gold > 0) then				-- We are already in a draw
        argument = "You have already begun the draw."
        create_event(me, ch, NIL, argument, "do_tell", 0, LT_MOB)
        return
      end

      for i = 1, getn(room.char) do			-- Gather the draw participants
        if (not isnpc(room.char[i])) then
          drawers[j] = room.char[i]
          j = j + 1
        end
      end

      if (j < 3) then					-- We're down to only one person again
        argument = "You seem to have lost the members for the draw. I'll wait until they return."
        create_event(me, ch, NIL, argument, "do_tell", 0, LT_MOB)
        return
      end

      me.gold = 1						-- Let's begin
      save_char(me)
      return
    end
    return
  end
end

function do_tell()
-- Allow the autodraw system to send tells, delayed for a certain period of time
          
  tell(ch.name, argument)
end

function winner()
-- Sort through the list of numbers to determine the draw winner

  local position = 0
  local buf = ""

  position = number(1, getn(drawers))

  buf = drawers[position].name.." has been chosen as the draw winner."
  say(buf)

  if (inworld("char", drawers[position].name)) then	-- Make sure the player is here!
    act("$N has received $p from the draw.", FALSE, me, me.objs[1], drawers[position], TO_NOTVICT)
    act("You have received $p from the draw.", FALSE, me, me.objs[1], drawers[position], TO_VICT)
    objfrom(me.objs[1], "char")
    objto(me.objs[1], "char", drawers[position])
  else
    say("Although it appears "..drawers[position].name.." is no longer with us.")
    if (inworld("char", autodraw_char.name)) then
      act("$n returns $p to $N.", FALSE, me, me.objs[1], autodraw_char, TO_NOTVICT)
      act("$n returns $p to you.", FALSE, me, me.objs[1], autodraw_char, TO_VICT)
      objfrom(me.objs[1], "char")
      objto(me.objs[1], "char", autodraw_char)
    else
      action(me, "drop "..me.objs[1].name)
    end
  end

  return (TRUE)
end

