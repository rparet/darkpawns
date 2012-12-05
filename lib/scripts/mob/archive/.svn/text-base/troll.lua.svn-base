function onpulse_all()
  if (me.hp < me.maxhp) then
    if (number(0, 20) == 0) then
      act("$n's wounds glow brightly for a moment, then disappear!", TRUE, me, NIL, NIL, TO_ROOM)
      me.hp = me.level * 2
      if (me.hp > me.maxhp) then
        me.hp = me.maxhp
      end
    end
  end
end

function fight()
  if (number(0, 10) == 0) then
    act("$n's wounds glow brightly for a moment, then disappear!", TRUE, me, NIL, NIL, TO_ROOM)
    me.hp = me.level * 2
    if (me.hp > me.maxhp) then
      me.hp = me.maxhp
    end
  end
end
