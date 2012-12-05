function fight()
  dofile("scripts/mob/fighter.lua")
  call(fight, ch, "x")
end

function onpulse_pc()

  local vict = NIL

  if (room.char) then
    for i = 1, getn(room.char) do
      vict = room.char[i]
      if (not isnpc(vict) and cansee(vict)) then
        if (aff_flagged(vict, AFF_VAMPIRE) or aff_flagged(vict, AFF_WEREWOLF)) then
          if (number(0, 5) == 0) then
            act("You hear a low growl in the back of $n's throat.", TRUE, me, NIL, NIL, TO_ROOM)
          else
            say("Die, nightbreed!!")
          end

-- Success depends on: Your level greater than your victim, or your victim level minus your 
-- level less than a random number from 0 to Immort, or if the victim is asleep.

          if (obj_list("stake", "char") or obj_list("spike", "char")) then
            if ((me.level > vict.level) or ((vict.level - me.level) < number(0, LVL_IMMORT)) or
              vict.pos == POS_SLEEPING) then
              act("$n drives $p into the chest of $N!", TRUE, me, obj, vict, TO_NOTVICT)
              act("$n drives $p into your chest with a solid blow!", TRUE, me, obj, vict, TO_VICT)
              if (plr_flagged(vict, PLR_VAMPIRE)) then
                plr_flags(vict, "remove", PLR_VAMPIRE)
              elseif (plr_flagged(vict, PLR_WEREWOLF)) then
                plr_flags(vict, "remove", PLR_WEREWOLF)
              end
              raw_kill(vict, me, TYPE_UNDEFINED)
              return
            else
              act("$N growls in anger as $n tries to drive a $p into $M!", TRUE, me, obj, vict, TO_NOTVICT)
              act("$n comes at you with a $p, but you dodge the attempt!", TRUE, me, obj, vict, TO_VICT)
              return
            end
          else
            action(me, "kill "..vict.name)
            return
          end
        end
      end
    end
  end
end
