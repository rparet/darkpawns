function oncmd()
-- Attack player who has the proper key and unlocks the door to the Captain's cabin.
-- Attached to mob 19114.

  local command = ""
  local subcmd = ""
  local keyfound = FALSE

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "unlock") then
    if (strfind(subcmd, "cabin") or strfind(subcmd, "door")) then  -- If player tries the door,
      if (ch.objs) then
        for i = 1, getn(ch.objs) do                              -- check his inventory
          if (ch.objs[i].vnum == 19118) then                     -- If he has the proper key,
            keyfound = TRUE
            break
          end
        end
      end

      if (keyfound == TRUE) then
        say("Arrr... Ya di'n't say mudder may I!")               -- guard curses at him
        action(me, "kill "..ch.name)                             -- and attacks.
      else                                                       -- If he doesn't have the key
        say("Da Cap'n's cabin is off limits to yas!")            -- guard tells him to get lost. 
      end
    end
  end
end
