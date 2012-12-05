function fight()
-- When the mob's hit points drop below a threshold, teleport the mob to another
-- location. Attached to mob 14411.

  if (me.hp < (me.maxhp / 2)) then
    act("$n says, 'My work here is done.'", TRUE, me, NIL, NIL, TO_ROOM)
    spell(me, NIL, SPELL_TELEPORT, TRUE)
    me.pos = POS_STANDING
    ch.pos = POS_STANDING
  end
end
