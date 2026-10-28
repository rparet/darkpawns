function code()
  local teachers =
  { [8024]  = {"Rejuvenation", "Protection", "Utility", "Field Craft"},
    [21220] = {"Rejuvenation", "Detection", "Astral", "Utility"},
    [21214] = {"Protection", "Environmental", "Necromancy", "Psychic"},
    [5307]  = {"Field Craft", "Hand to Hand", "Weaponry"},
    [4825]  = {"Detection", "Utility", "Malediction"},
    [18219] = {"Stealth", "Field Craft"}
  }
  local advanced =
  { [7971] = {"Rejuvenation", "Protection", "Malediction", "Necromancy", "Astral", "Psychic"},
    [7972] = {"Detection", "Enviromental", "Field Craft", "Stealth", "Utility"},
    [7973] = {"Weaponry", "Hand to Hand"}
  }

  local buf = ""
  local group = 0
  local group_name = ""
  local gold = 0
  local experience = 0
  local ad_teacher = FALSE

  if (teachers[me.vnum] == NIL) then			-- Is this a teacher?
    if (advanced[me.vnum] == NIL) then			-- Perhaps an advanced teacher?
      act("$n is not qualified as a teacher, you should search elsewhere.",
          TRUE, me, NIL, ch, TO_VICT)
      return
    end
    ad_teacher = TRUE
  end

  if (argument == "") then				-- Display skills/spells to teach
    act("$n might be willing to teach you...", TRUE, me, NIL, ch, TO_VICT)
    if (ad_teacher == FALSE) then
      for index, value in teachers[me.vnum] do
        buf = mxp(value, "learn "..me.name.." "..value)
        act("  "..buf, FALSE, me, NIL, ch, TO_VICT)
      end
    else
      for index, value in advanced[me.vnum] do
        buf = mxp(value, "learn "..me.name.." "..value)
        act("  "..buf, FALSE, me, NIL, ch, TO_VICT)
      end
    end
    return
  else
    if (ad_teacher == FALSE) then
      for index, value in teachers[me.vnum] do		-- Is it a skill group?
        if (strfind(strlower(value), strlower(argument))) then
          group = skill_group(value)
          group_name = value
        end
      end
    else
      for index, value in advanced[me.vnum] do		-- Is it a skill group?
        if (strfind(strlower(value), strlower(argument))) then
          group = skill_group(value)
          group_name = value
        end
      end
    end

    if (group == 0) then					-- Skill group specified is not in the list
      act("$n exclaims, 'I am unfamiliar with that group!'", TRUE, me, NIL, ch, TO_VICT)
      return
    end

    ch.group_lvl = get_group_lvl(ch, group)
    ch.group_pts = get_group_pts(ch)

    if ((ch.group_lvl >= 3) and (ad_teacher == FALSE)) then
      act("$n states, 'I can teach you nothing further in this group. You must seek out someone "
        .."more skilled than myself.", TRUE, me, NIL, ch, TO_VICT)
      return
    end

    if (ch.group_lvl == 4) then				-- Already has knowledge of the group
      act("$n states, 'You already possess the knowledge of this group, I can teach no more.'",
          TRUE, me, NIL, ch, TO_VICT)
      return
    end

    if (ch.group_pts == 0) then
      act("$n states, 'You are not ready to advance your knowledge in this area.'",
          TRUE, me, NIL, ch, TO_VICT)
      return
    end

    gold = 1000 * (ch.group_lvl + 1)
    if (ch.gold < gold) then					-- Can we afford the knowledge?
      act("$n states, 'You lack the "..gold.." gold pieces I require.'", TRUE, me, NIL, ch, TO_VICT)
      return
    end

    experience = 1000 * exp(ch.group_lvl + 1)
    if (ch.exp_q < experience) then				-- Do we have enough experience?
      act("$n states, 'You lack the experience required to learn this group.'",
          TRUE, me, NIL, ch, TO_VICT)
      return
    end

    get_group_lvl(ch, group, ch.group_lvl + 1)
    get_group_pts(ch, ch.group_pts - 1)
    if (ch.group_lvl > 0) then
      set_skill(ch, group, (ch.group_lvl * 25) + 5)	-- Increment competency 5% past group level cap
    else
      set_skill(ch, group, 20)				-- Default to 20% for initial purchase
    end

    buf = "$n explains the skills involved in the '"..group_name.."' group."
    act(buf, TRUE, me, NIL, ch, TO_VICT)
    act("$n speaks with $N for some time.", TRUE, ch, NIL, me, TO_ROOM)
    buf = "You feel like you've learned something.\r\nYou also feel very tired."
    act(buf, TRUE, me, NIL, ch, TO_VICT)

    ch.exp_q = ch.exp_q - experience
    ch.gold = ch.gold - gold
    ch.hp = 1
    ch.mana = 1
    ch.move = 1
  end
end

function oncmd()
-- Players can decide to "unlearn" their groups and have their group points refunded. Unassigned
-- experience and gold will not be returned!

  local command = ""
  local subcmd = ""
  local buf = ""
  local group_name = ""
  local group = 0
  local groups =
    { "Rejuvenation", "Protection", "Detection", "Necromancy", "Environmental", "Utility", "Astral",
      "Malediction", "Psychic", "Stealth", "Field Craft", "Hand to Hand", "Weaponry" }
               

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "remove") then
    if (subcmd == "") then
      return
    end

    for i = 1, getn(groups) do			-- Find the specified group
      if (strfind(strlower(groups[i]), strlower(subcmd))) then
        group = skill_group(groups[i])
        group_name = groups[i]
        break
      end
    end

    if (group == 0) then				-- Incorrect group
      return (FALSE)
    end

    ch.group_lvl = get_group_lvl(ch, group)	-- Do we have knowledge of the group?
    if (ch.group_lvl == 0) then
      act("You can't remove a skill or spell group that you don't even know!", FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    ch.group_pts = get_group_pts(ch)		-- Reset things back the way they were
    get_group_lvl(ch, group, ch.group_lvl - 1)
    get_group_pts(ch, ch.group_pts + 1)

    act("$n places $s hand on your head and mumbles a few words.", TRUE, me, NIL, ch, TO_VICT)
    act("$n speaks with $N for some time.", TRUE, ch, NIL, me, TO_ROOM)
    buf = "Your knowledge of the '"..group_name.."' group has been diminished."
    act(buf, TRUE, ch, NIL, NIL, TO_CHAR)
    return (TRUE)
  end
end

