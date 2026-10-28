function greet()
  if (ch.vnum == 8063) or (ch.vnum == 12115) or (ch.vnum == 18203) then
    emote("mutters something about filthy animals in $s shop.")
    action(me, "kill "..ch.name)
  end
end
