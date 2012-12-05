function sound()
  if (number(0, 2) == 0) then
    if (number(0, 1) == 0) then
      say("I seek the humming black armor; a complete set is my life's goal.")
    else
      say("There are twenty-two pieces and I fear they shall never be found.")
    end
  end
end

function ongive()
  -- singles = vnum old, cost, descriptive, vnum new
  local singles = { 
    { 11052, 2000, "tooth", 11021 },  -- stone of seripis
    { 21401, 10000, "scale", 21400 }  -- shield of the sea dragon
  }
  
  local sba = {
    { 14201, 14202, 14203, 14204, 14205, 14206, 14207, 14208, 14209, 14210, 14211,
      14212, 14213, 14214, 14215, 14216, 14217, 14218, 14219, 14220, 14221, 14222 },
    14223, 0
  }
  
  dofile("scripts/mob/assembler.lua")
  
  if (assemble_one(singles, FALSE, FALSE) == TRUE) then
    return
  end
  
  local assembled = assemble_list(sba[1], sba[2], sba[3])
  
  if (assembled == 1) then
    act("\r\nTaking the pieces, $n forges them together.\r\nMagick rumbles through the smithy, and finally "
	.."the armor is done.\r\n", TRUE, me, NIL, NIL, TO_ROOM)
    act("The blacksmith hands you a nearly weightless suit of armor.\r\n", TRUE, me, NIL, ch, TO_VICT)
    act("$n hands $p to $N.", TRUE, me, obj, ch, TO_NOTVICT)
    return
  elseif ((assembled == 2) or (assembled == 3)) then
    return
  end
  
  return_obj("I don't need this.", FALSE)
end
