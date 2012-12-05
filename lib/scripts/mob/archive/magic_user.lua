function fight()
  local type = { SPELL_MAGIC_MISSILE, 0, 0, 0, 0, SPELL_BURNING_HANDS, 0,
                 0, 0, SPELL_DISPEL_EVIL, 0, 0, SPELL_DISPEL_GOOD, 0, 
                 SPELL_LIGHTNING_BOLT, 0, 0, SPELL_FIREBALL, 0, SPELL_HELLFIRE, 0, 
                 SPELL_COLOR_SPRAY, SPELL_DISRUPT, 0, SPELL_DISINTEGRATE, 0, SPELL_FLAMESTRIKE,
                 0, 0, SPELL_TELEPORT, 0, SPELL_ACID_BLAST, 0 }

  if (number(0, 6) == 0) then
    local choice = number(1, round(me.level / 3))
    if ((type[choice] == SPELL_DISPEL_EVIL) or (type[choice] == SPELL_DISPEL_GOOD)) then
      if ((me.evil == FALSE) and (ch.evil == TRUE)) then
        type[choice] = SPELL_DISPEL_EVIL
      elseif ((me.evil == TRUE) and (ch.evil == FALSE)) then
        type[choice] = SPELL_DISPEL_GOOD
      end
    end
    if (type[choice] ~= 0) then
      if (type[choice] == SPELL_TELEPORT) then
        if (number(0, 4) == 0) then			-- Let's give a 20% chance of teleporting
          spell(ch, NIL, type[choice], TRUE)
          ch.pos = POS_STANDING
          me.pos = POS_STANDING
        end
      else
        spell(ch, NIL, type[choice], TRUE)
      end
    end
  elseif (number(0, 6) == 0) then
    if (me.wear) then
      for i = 1, getn(me.wear) do
        if (me.wear[i].type == ITEM_STAFF) then
          action(me, "use staff "..ch.name)
        end
      end
    end
  end
end
