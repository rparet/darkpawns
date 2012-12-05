function sound()
  local case = number(0, 4)
  if ((case == 0) and (number(0, 1) == 0)) then
    emote("kneels in worship of the holy powers.")
  elseif ((case == 1) and (number(0, 2) == 0)) then
    say("There is a secret I must tell someone now.")
  elseif ((case == 2) and (number(0, 2) == 0)) then
    say("Passed down from High Priest to High Priest is the secret of the holy weapon.")
  elseif ((case == 3) and (number(0, 3) == 0)) then
    say("There are special oils decanted from the blood of the slain paladins of the holiest order.")
  elseif ((case == 4) and (number(0, 3) == 0)) then
    say("Should a person find these oils, and bring them to me with the blessed weapon...")
    emote("trails off mumbling about winning a holy war.")
  end
end

function ongive()
  avenger = { { 7915, 7917 }, 7916, 0 }
  
  dofile("scripts/mob/assembler.lua")
  
  local assembled = assemble_list(avenger[1], avenger[2], avenger[3])
  if (ch.align > 0) then
    if (assembled == 1) then
      emote("begins to pray aloud as he invokes the gods.")
      emote("pours a vial of holy oils along the blade of a holy sword.")
      act("The sword vanishes as blinding white light bursts forth from within it.\r\n\r\n"
          .."As the light recedes, $n stands in awe holding $p.",
          TRUE, me, obj, NIL, TO_ROOM)
      act("$n hands $p to you.", TRUE, me, obj, ch, TO_VICT)
      act("$n hands $p to $N.", TRUE, me, obj, ch, TO_NOTVICT)
      say("I present to you the mightiest weapon of the holy armies.")
      tell(ch.name, "Go forth and crusade against the demonic.  I shall pray for our survival!")
      return
    elseif (assembled == 2) then
       if (obj.vnum == 7915) then
         act("$n holds $p up and gasps in surprise.", TRUE, me, obj, NIL, TO_ROOM)
         say("Surely, this is the holy weapon!")
         tell(ch.name, "Did you bring me the holy oils?")
       elseif (obj.vnum == 7917)  then
         act("$n removes the cap and smells $p.", TRUE, me, obj, NIL, TO_ROOM)
         tell(ch.name, "These are the oils, but where is the Holy weapon?")
       end
       return
    elseif (assembled == 3) then
      return
    end
    return_obj("I can do nothing with this.", FALSE)
    return
  end
  say("You have not yet found salvation.")
  return_obj("Come see me again when you have found peace with god.")
end 