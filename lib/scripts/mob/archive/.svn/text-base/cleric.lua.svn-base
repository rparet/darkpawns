function fight()
  local type = { SPELL_HEAL, SPELL_VITALITY, SPELL_CURE_LIGHT, SPELL_DISPEL_EVIL, 0, 0,
                 SPELL_DISPEL_GOOD, 0, SPELL_HARM, 0, 0, SPELL_BLINDNESS, 0, 0, SPELL_CURSE,
                 0, 0, SPELL_POISON, 0, 0, SPELL_EARTHQUAKE, SPELL_HARM, 0, 0, 0,
                 SPELL_BURNING_HANDS, 0, SPELL_EARTHQUAKE, 0, 0, 0, SPELL_DIVINE_INT, 0 }
  local healperc = 0

  if (number(0, 6) == 0) then
    local choice = number(4, (me.level / 3))

    if ((type[choice] == SPELL_DISPEL_EVIL) or (type[choice] == SPELL_DISPEL_GOOD)) then
      if ((me.evil == FALSE) and (ch.evil == TRUE)) then
        type[choice] = SPELL_DISPEL_EVIL
      elseif ((me.evil == TRUE) and (ch.evil == FALSE)) then
        type[choice] = SPELL_DISPEL_GOOD
      end
    end

    if ((me.hp < (me.maxhp / 4)) and (choice > 25)) then	-- Am I almost dead??
      if (number(0, 2) == 0) then
        act("$n mumbles a few words and vanishes in a puff of smoke!", 
            TRUE, me, NIL, NIL, TO_ROOM)
        spell(me, NIL, SPELL_TELEPORT, FALSE)
        me.pos = POS_STANDING
        ch.pos = POS_STANDING
        act("$n arrives in a puff of smoke!", TRUE, me, NIL, NIL, TO_ROOM)
        return
      else
        act("$n mumbles a few words and $N vanishes in a puff of smoke!", 
            TRUE, me, NIL, ch, TO_NOTVICT)
        act("$n mumbles a few words and the world goes dark...", TRUE, me, NIL, ch, TO_VICT)
        spell(ch, NIL, SPELL_TELEPORT, FALSE)
        me.pos = POS_STANDING
        ch.pos = POS_STANDING
        act("$n arrives in a puff of smoke!", TRUE, ch, NIL, NIL, TO_ROOM)
      end
    end

    if (me.hp < (me.maxhp / 2)) then			-- Should I hit a foe or heal myself?
      healperc = 7
    elseif (me.hp < (me.maxhp / 4)) then
      healperc = 5
    elseif (me.hp < (me.maxhp / 8)) then
      healperc = 3
    end

    if (number(1, healperc + 2) < 3) then
      if (type[choice] ~= 0) then			-- Hit a foe
        spell(ch, NIL, type[choice], TRUE)
        return
      end
    else
      spell(me, NIL, type[number(1, 3)], TRUE)	-- Heal myself
    end
  end
end

