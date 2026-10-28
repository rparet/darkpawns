function oncmd()
-- The daemonic focus will begin the "summoning ceremony" where Pyros (1410) is bought
-- into the world. Given a % chance, Pyros may be "banished" otherwise he will roam
-- and must be killed by normal means. Attached to obj 1406.

  local command = ""
  local subcmd = ""
  local buf = ""
  local tmp_room = { }
  local rooms = { 1439, 1441, 1443, 1444, 1446 }
  local candles = 0
  local people = 0

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "use") then
    if (not obj_list(subcmd, "char") or (obj.vnum ~= 1406)) then
      return
    end

    if (room.vnum ~= 1445) then			-- Not used in the correct location
      act("This is not the appropriate location to use your $p.", TRUE, me, obj, NIL, TO_CHAR)
      return (TRUE)
    end

    if (keep_ceremony == TRUE) then			-- Ceremony already in progress
      act("There is a summoning ceremony already underway.", FALSE, me, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    if (keep_pyros == TRUE) then
      act("A vile daemon has already been summoned to this world. It must be destroyed "
        .."before another can be bought forth.", FALSE, me, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    keep_ceremony = TRUE
    for i = 1, getn(rooms) do
      tmp_room[i] = load_room(rooms[i])		-- Load all of the other rooms
    end

    for i = 1, 5, 1 do					-- Count the candles
      if (tmp_room[i].objs) then
        for j = 1, getn(tmp_room[i].objs) do
	    if (tmp_room[i].objs[j].vnum == 1402) then
	      candles = candles + 1
		break
	    end
	  end
	end
    end

    if (candles ~= 5) then				-- Missing at least one candle, if not more
      act("$n's $p glows softly but nothing happens.", TRUE, me, obj, NIL, TO_ROOM)
      act("Your $p glows softly but nothing happens.", TRUE, me, obj, NIL, TO_CHAR)
      keep_ceremony = FALSE
      return (TRUE)
    end

    for i = 1, 5, 1 do
      if (tmp_room[i].char) then			-- At least one PC/NPC in the room
        people = people + 1
      end
    end

    if (people ~= 5) then				-- Missing at least one person, if not more
      if (obj.val[1] == 0) then			-- Candles aren't lit yet
        act("Your $p glows softly and the bloodwax candles ignite, but nothing else happens.",
          TRUE, me, obj, NIL, TO_CHAR)
        act("$n's $p glows softly and the bloodwax candles ignite, but nothing else happens.",
          TRUE, me, obj, NIL, TO_ROOM)
        buf = me.name.."'s "..obj.name.." glows softly and the bloodwax candles ignite, but "
          .."nothing else happens.\r\n"
        for i = 1, 5, 1 do
          echo(tmp_room[i], "room", buf)	-- send_to_room
        end
        obj.val[1] = 1
        save_obj(obj)
      else
        act("Nothing seems to happen.", FALSE, me, NIL, NIL, TO_CHAR)
      end

      keep_ceremony = FALSE
      return (TRUE)
    end

    if (obj.val[1] == 0) then				-- Candles aren't lit yet
      act("Your $p glows softly and the bloodwax candles ignite.", TRUE, me, obj, NIL, TO_CHAR)
      act("$n's $p glows softly and the bloodwax candles ignite.", TRUE, me, obj, NIL, TO_ROOM)
      buf = me.name.."'s "..obj.name.." glows softly and the bloodwax candles ignite.\r\n"
      obj.val[1] = 1
      save_obj(obj)
      for i = 1, 5, 1 do
        echo(tmp_room[i], "room", buf)		-- send_to_room
      end
    end

    create_event(me, NIL, obj, NIL, "phase_two", 3, LT_OBJ)
    return (TRUE)
  end
end

function phase_two()
-- Second phase of the summoning, endure heat for all PCs/NPCs

  local buf = ""
  local tmp_room = { }
  local rooms = { 1439, 1441, 1443, 1444, 1446 }

  for i = 1, getn(rooms) do
    tmp_room[i] = load_room(rooms[i])		-- Load all of the other rooms
  end

  act("You utter the words 'unsmecondus poih' and a beam of light from your $p surrounds you.",
    TRUE, me, obj, NIL, TO_CHAR)
  act("$n utters the words 'unsmecondus poih' and a beam of light shoots from $s $p, striking you!",
    TRUE, me, obj, NIL, TO_ROOM)
  buf = me.name.." utters the words 'unsmecondus poih' and a beam of light shoots from "..obj.name
    ..", striking you.\r\n"
  for i = 1, 5, 1 do
    echo(tmp_room[i], "room", buf)		-- send_to_room
  end

  create_event(me, NIL, obj, NIL, "phase_three", 3, LT_OBJ)
  return(TRUE)
end

function phase_three()
-- Third phase of the summoning, summon the daemon

  local buf = ""
  local tmp_room = { }
  local rooms = { 1439, 1441, 1443, 1444, 1446 }

  for i = 1, getn(rooms) do
    tmp_room[i] = load_room(rooms[i])		-- Load all of the other rooms
  end

  act("Raising your hands into the air, you utter the words 'gewbar miobar'. Flames engulf the "
    .."bloodwax candles and there is a loud crack. The center of the circle opens with a plume "
    .."of flame and a large Daemon emerges.", TRUE, me, NIL, NIL, TO_CHAR)
  act("Raising $s hands into the air, $n utters the words 'gewbar miobar.' Flames engulf the "
    .."bloodwax candles and there is a loud crack. The center of the circle opens with a plume "
    .."of flame and a large Daemon emerges.", TRUE, me, NIL, NIL, TO_ROOM)
  buf = me.name.." utters the words 'gewbar miobar'.  Flames engulf the bloodwax candles "
    .."and there is a loud crack. The center of the circle opens with a plume of flame and a "
    .."large Daemon emerges.\r\n"

  for i = 1, getn(tmp_room) do
    echo(tmp_room[i], "room", buf)		-- send_to_room
  end

  create_event(me, NIL, obj, NIL, "phase_four", 3, LT_OBJ)
  return (TRUE)
end

function phase_four()
-- Fourth phase of the summoning, banishment of the Daemon. Won't necessarily succeed.

  local buf = ""
  local tmp_room = { }
  local rooms = { 1439, 1441, 1443, 1444, 1446 }
  local pyros = NIL
  local pyros_item = NIL

  for i = 1, getn(rooms) do
    tmp_room[i] = load_room(rooms[i])		-- Load all of the other rooms
  end

-- Load Pyros and kill anyone who is standing in the center of the circle
  pyros = mload(1410, 1442)
  keep_pyros = TRUE
  for i = 1415, 1417, 1 do
    pyros_item = oload(pyros, i, "char")
    local percent = number(0, 100)
    if (pyros_item.perc_load < percent) then
      extobj(pyros_item)
    else
      equip_char(pyros, pyros_item)
    end
  end

  local kill_room = load_room(1442)
  for i = 1, getn(kill_room.char) do	-- The "killing" part
    if (kill_room.char[i].vnum ~= pyros.vnum) then
      act("You scream in pain as the flames engulf you! Your death is instant.",
        FALSE, kill_room.char[i], NIL, NIL, TO_CHAR)
      raw_kill(kill_room.char[i], pyros, SPELL_HELLFIRE)
    end
  end

  if (number(0, (34 - me.level)) == 0) then
    act("With a final clap of your hands, you utter the words 'vibugp miobar'. $N is "
      .."swallowed up by a tongue of flame, the center of the circle closing behind $M.",
      TRUE, me, NIL, pyros, TO_CHAR)
    act("With a final clap of $s hands, $n utters the words 'vibugp miobar'. $N is "
      .."swallowed up by a tongue of flame, the center of the circle closing behind $M.",
      TRUE, me, NIL, pyros, TO_ROOM)
    buf = me.name.." utters the words 'vibugp miobar'. "..pyros.name.." is swallowed up "
      .."by a tongue of flame, the center of the circle closing behind it.\r\n"

    for i = 1, 5, 1 do
      echo(tmp_room[i], "room", buf)		-- send_to_room
    end
    raw_kill(pyros, NIL, SPELL_DISINTEGRATE)
    keep_pyros = FALSE
  else
    act("With a final clap of your hands, you utter the words 'vibugp miobar'...but there "
      .."is no reaction. $N laughs evilly before coming for you!",
      TRUE, me, NIL, pyros, TO_CHAR)
    act("With a final clap of $s hands, $n utters the words 'vibugp miobar'...but there is "
      .."no reaction. $N laughs evilly at $n before moving towards $m!",
      TRUE, me, NIL, pyros, TO_ROOM)
    buf = me.name.." utters the words 'vibugp miobar'...but there is no reaction. "..pyros.name
      .." laughs evilly before striking!\r\n"

    for i = 1, 5, 1 do
      echo(tmp_room[i], "room", buf)		-- send_to_room
    end
    mob_flags(pyros, "remove", MOB_SENTINEL)
    mob_flags(pyros, "set", MOB_HUNTER)
    set_hunt(pyros, me)
  end

-- Ceremony cleanup - remove all of the candles, destroy focus if its been used.
  for i = 1, 5, 1 do
    for j = 1, getn(tmp_room[i].objs) do
      if (tmp_room[i].objs[j].vnum == 1402) then
        extobj(tmp_room[i].objs[j])
        break
      end
    end
  end
 
  act("\r\nIts magical energy depleted, the daemonic focus vanishes with a puff of smoke!",
    FALSE, me, NIL, NIL, TO_CHAR)
  extobj(obj)
  keep_ceremony = FALSE
  return (TRUE)
end
