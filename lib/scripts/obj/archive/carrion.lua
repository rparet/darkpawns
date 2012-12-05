function oncmd()
-- If the player looks at the obj, a carrion mob arrives and attacks. Attached to
-- obj 14313.

  local command = ""
  local subcmd = ""
  local carrion = NIL

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "look") then
    if (not obj_list(subcmd, "room") or (obj.vnum ~= 14313)) then
      return
    end

    carrion = mload(14308, room.vnum)
    carrion.level = me.level			-- Give the mob some new stats
    if (carrion.hp < (me.hp / 2)) then
      carrion.hp = me.hp / 2
    end
    save_char(carrion)
    act("Suddenly $n skitters from out of a corpse!", TRUE, carrion, NIL, NIL, TO_ROOM)
    action(carrion, "kill "..me.name)
  end
end
