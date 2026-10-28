function onpulse_pc()
-- Players in the room are "charmed" by song and will either strike themselves or others if there
-- are more than 1 in the room. Attached to mob 1314.

  local num_victims = 0
  local vict = 0
  local next_vict = 0
  local found = FALSE
  local loop = 0

  if (room.char) then
    if (number(0, 4) == 0) then
      for i = 1, getn(room.char) do
        if (not isnpc(room.char[i]) and (room.char[i].level < LVL_IMMORT)) then
          num_victims = num_victims + 1
        end
      end

      if (num_victims == 0) then	-- No PCs? Do nothing
        return
      end

      repeat					-- Who's the "victim" of the song
        vict = number(1, getn(room.char))
        loop = loop + 1
        if (not isnpc(room.char[vict]) and (room.char[vict].level < LVL_IMMORT)) then
          found = TRUE
        end
        if (loop == 1000000) then
          return
        end
      until (found == TRUE)
      ch = room.char[vict]

      found = FALSE
      if (num_victims > 1) then		-- More than 1 char in the room, locate one
        repeat				-- Find another victim
          next_vict = number(1, getn(room.char))
          if ((vict ~= next_vict) and not isnpc(room.char[next_vict]) and
             (room.char[next_vict].level < LVL_IMMORT)) then
            found = TRUE
          end
        until (found == TRUE)
        vict = room.char[next_vict]

        for i = 1, getn(room.char) do
          if ((room.char[i] ~= ch) and (room.char[i] ~= vict)) then
            buf = me.name.." mumbles softly and "..ch.name.." screams loudly, attacking "..vict.name.."!"
            act(buf, TRUE, room.char[i], NIL, NIL, TO_CHAR)
          end
        end

        buf = me.name.." mumbles softly and "..ch.name.." screams loudly, attacking you!"
        act(buf, TRUE, vict, NIL, NIL, TO_CHAR)
        buf = me.name.." mumbles softly and you scream loudly, attacking "..vict.name.."!"
        act(buf, TRUE, ch, NIL, NIL, TO_CHAR)
        action(ch, "kill "..vict.name)
      else
        local damage = number(10, 30)
        ch.hp = ch.hp - damage
        save_char(ch)
        act("$n mumbles softly and $N begins screaming loudly, hitting $Mself.",
            TRUE, me, NIL, ch, TO_NOTVICT)
        act("$n mumbles softly and you begin to scream, involuntarily hitting yourself.",
            TRUE, me, NIL, ch, TO_VICT)
      end
    end
  end
end
