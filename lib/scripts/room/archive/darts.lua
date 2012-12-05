function onpulse()
-- Small darts "shoot" from the walls and damage the player. Attached to numerous rooms within
-- the ziggurat (zone 22).

  if (ch.level < LVL_IMMORT) then
    act("Tiny darts shoot from holes in the wall and strike you!", FALSE, ch, NIL, NIL, TO_CHAR)
    act("$n is hit with hundreds of tiny darts!", TRUE, ch, NIL, NIL, TO_ROOM)

    local damage = number(20, 40)
    ch.hp = ch.hp - damage
  end
end
