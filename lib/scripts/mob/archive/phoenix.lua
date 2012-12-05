function fight()
-- A rider (mob 1402) will assist the mob in battle but will be immune from damage
-- until the mob "crashes" to the ground in death. Attached to mob 1401.

  local trident = NIL
  local rider = NIL

  if (me.gold == 0) then
    me.gold = 1
    save_char(me)
    rider = mload(1402, room.vnum)
    act("$n, riding atop the phoenix, joins the fight and strikes at $N!",
      TRUE, rider, NIL, ch, TO_NOTVICT)
    act("$n, riding atop the phoenix, joins the fight and strikes at you!",
      TRUE, rider, NIL, ch, TO_VICT)

    trident = oload(rider, 1420, "char")
    local percent = number(0, 100)
    if (trident.perc_load < percent) then
      me.gold = 2
      save_char(me)
    end
    extobj(trident)
    extchar(rider)
  else
    rider = mload(1402, room.vnum)
    trident = oload(rider, 1420, "char")

    if (me.gold == TRUE) then
      equip_char(rider, trident)
    else
      extobj(trident)
    end

    action(rider, "kill "..ch.name)
    extchar(rider)		-- Remove the rider and weapon until the phoenix is dead
  end
end

function death()
  local rider = NIL
  local trident = NIL

  rider = mload(1402, room.vnum)
  action(rider, "kill "..ch.name)

  trident = oload(rider, 1420, "char")
  act("Before $n crashes to the ground, $N leaps off.", TRUE, me, NIL, rider, TO_ROOM)

  if (me.gold == TRUE) then
    equip_char(rider, trident)
  else
    extobj(trident)
  end
end
