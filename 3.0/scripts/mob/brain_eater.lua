function onpulse_all()
-- The mob will behead a corpse and "eat" the brains to advance its level
-- or hitpoints. Attached to mobs 14420 and 14432.

  if (room.objs) then
    for i = 1, getn(room.objs) do
      if (iscorpse(room.objs[i]) and (strfind(room.objs[i].alias, "corpse"))) then
        if (strfind(room.objs[i].alias, "headless")) then
          return
        end

        action(me, "behead corpse")		-- Behead the corpse
        act("$n pulls the brain out of the head and eats it with a noisy slurp, blood "
            .."and drool flying everywhere.", TRUE, me, NIL, NIL, TO_ROOM)
        if (me.level < 100) then
          me.level = me.level + 1
        else
          me.hp = me.hp + 100			-- Used to be damroll, change if required
        end
        ch = me
      end
    end
  end
end
