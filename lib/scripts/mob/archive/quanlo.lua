function oncmd()
  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (me.pos > POS_SLEEPING) then
    if ((command == "flee") or (command == "retreat") or (command == "escape")) then
      gossip("What was that "..ch.name.."? This is not a shawade. Try it again. This time with feewing.")
    elseif (((command == "look") or (command == "examine")) and strfind(me.alias, subcmd)) then
      say("What is it you seek, "..ch.name.."? Tell me and begone.")
    end
  end
end
