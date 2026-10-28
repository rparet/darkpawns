function oncmd()
-- Let's out a simple zone message. Attached to obj 14415.

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "use") then
    if (not obj_list(subcmd, "char") or (obj.vnum ~= 14415)) then
      return
    end

    if (obj.val[1] == 0) then
      obj.val[1] = 4
      save_obj(obj)
    elseif (obj.val[1] == 1) then
      act("You inhale deeply then blow hard...but nothing happens!", FALSE, me, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    echo(me, "zone", "You hear the blaring of a loud horn.\r\n")
    act("You inhale deeply then blow hard!", FALSE, me, NIL, NIL, TO_CHAR)
    act("A blaring note resounds through the air.", FALSE, me, NIL, NIL, TO_CHAR)
    act("$n blows into $p.", TRUE, me, obj, NIL, TO_ROOM)
    act("$p lets out a blaring note...", TRUE, me, obj, NIL, TO_ROOM)

    if (round(room.vnum / 100) == 144) then		-- Release nightmare beast
      vrix_teleport = FALSE
      obj.val[1] = obj.val[1] - 1
      save_obj(obj)
    end
      
    return (TRUE)
  end
end
