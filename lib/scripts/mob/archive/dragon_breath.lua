function fight()
  local dragons = {
    [4209] = SPELL_FROST_BREATH,
    [4705] = SPELL_FROST_BREATH,
    [10200] = SPELL_FIRE_BREATH,
    [10300] = SPELL_FIRE_BREATH,
    [10301] = SPELL_ACID_BREATH,
    [10302] = SPELL_LIGHTNING_BREATH,
    [20027] = SPELL_GAS_BREATH
  }
  if (number(0, 14) == 0) then
    spell(ch, NIL, dragons[me.vnum], FALSE)
  end
end

function greet()
  if (me.vnum == 4209) then
    act("$n looks at you.", TRUE, me, NIL, NIL, TO_ROOM)
    act("$n growls, 'So, you have found my lair...'", TRUE ,me ,NIL, NIL,TO_ROOM)
    act("$n exclaims, 'For that you must die!'", TRUE, me, NIL, NIL, TO_ROOM)
    action(me, "kill "..ch.name)
  end
end
