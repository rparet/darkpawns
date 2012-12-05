function sound()
  if (number(0, 2) == 0) then
    say("Long ago, a powerful artifact belonging to my God was violently unmade.")
    say("Its pieces were scattered into the wretched Swamps of Sadness...")
    say("I fear that it shall never be made whole again!")
    action(me, "sigh")
  elseif (number(0, 2) == 0) then
    act("$n looks at the sky plaintively.", TRUE, me, NIL, NIL, TO_ROOM)    
    action(me, "sigh")
  end
end

function ongive()
  local runestone = {
    { 4600, 4601, 4602, 4603, 4604, 4605, 4606, 4607, 4608, 
      4609, 4610, 4611, 4612, 4613, 4614, 4615, 4616, 4617 },
    18313, 0
  }
  
  dofile("scripts/mob/assembler.lua")				
  
  local assembled = assemble_list(runestone[1], runestone[2], runestone[3])
  if (assembled == 1) then
    act("Taking all of the stones, $n begins to weave a spell... \r\n"
        .."$n suddenly glows with a multicoloured light, which then fades to a dull "
        .."chromatic aura.\r\n", TRUE, me, NIL, NIL, TO_ROOM)
    say("It is done!  The rune stone is remade!")
    act("$n hands $p to you.", TRUE, me, obj, ch, TO_VICT)
    act("$n hands $p to $N.", TRUE, me, obj, ch, TO_NOTVICT)
    tell(ch.name, "Guard this carefully, for it contains some of the power of the ancient gods!")
    return
  elseif (assembled == 2) then
    return
  elseif (assembled == 3) then
    if (number(0, 3) == 0) then
      say("This is definately one of the stones!")
      say("Still missing some though.")
    else
      say("That's a piece of it. Not all, though.")
    end
  end
  
  return_obj("What do I need this rubbish for?", FALSE)
end

