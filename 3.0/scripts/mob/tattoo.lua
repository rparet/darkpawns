function greet()
-- The mob will let players know how they may obtain tattoos through interactive
-- speech.

  local buf = ""

  if ((not ch.leader) and (not isnpc(ch))) then
    buf = mxp("tattoo", "say tattoo?")
    say("Can I interest you in a "..buf.."?")
  end
end

function oncmd()
-- This portion of the script will provide the interactivity to keep the conversation
-- going.

  local command = ""
  local subcmd = ""
  local designs = { }
  local dyes = { }
  local j = 1
  local k = 1
  local tattoo = {
    "of a Tribal Symbol",
    "of a Scorpion poised to strike",
    "of Jehduti, the Moon God",
    "of Aethen, the Sun God",
    "of a Dragon rising from the flames",
    "of a skull resting on a bed of thorns",
    "of a wild rose in full blossom"
  }

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (strfind(argument, "^'[%a%s]") ~= NIL) then
    command = "say"
    subcmd = skip_spaces(strsub(argument, 2))
  end

  if (command == "say") then
    if ((strlower(subcmd) == "tattoo") or (strlower(subcmd) == "tattoo?")) then
      tattoo_conv_state = 1
      create_event(me, ch, NIL, NIL, "reply", 0, LT_MOB)
    elseif ((strlower(subcmd) == "materials") or (strlower(subcmd) == "materials?")) then
      tattoo_conv_state = 2
      create_event(me, ch, NIL, NIL, "reply", 0, LT_MOB)
    elseif ((strlower(subcmd) == "design") or (strlower(subcmd) == "design?")) then
      tattoo_conv_state = 3
      create_event(me, ch, NIL, NIL, "reply", 0, LT_MOB)
    elseif ((strlower(subcmd) == "dyes") or (strlower(subcmd) == "dyes?")) then
      tattoo_conv_state = 4
      create_event(me, ch, NIL, NIL, "reply", 0, LT_MOB)
    elseif ((strlower(subcmd) == "ingredients") or (strlower(subcmd) == "ingredients?")) then
      tattoo_conv_state = 5
      create_event(me, ch, NIL, NIL, "reply", 0, LT_MOB)
    end
  elseif (command == "list") then
    if (ch.objs) then
      for i = 1, getn(ch.objs) do			-- Let's look for a tattoo design
        if (ch.objs[i].vnum == 1209) then		-- Got the design
          designs[j] = { tattoo[ch.objs[i].val[1]], ch.objs[i].val[2] }
          j = j + 1
        elseif (strfind(ch.objs[i].alias, "dye", 1, 1)) then
          dyes[k] = ch.objs[i].name			-- Got at least one dye
          k = k + 1
        end
      end
    end

    if (not designs[1]) then				-- Missing a component
      act("$n says, 'Sorry, you don't have any tattoo designs. I can do nothing for you.'",
        TRUE, me, NIL, ch, TO_VICT)
      return (TRUE)
    elseif (not dyes[1]) then
      act("$n says, 'Sorry, you don't have any dye I can use. I can do nothing for you.'",
        TRUE, me, NIL, ch, TO_VICT)
      return (TRUE)
    end
        
    act("To buy a tattoo: BUY <number of tattoo> <colors>.", FALSE, ch, NIL, NIL, TO_CHAR)
    act("Available tattoos are:", FALSE, ch, NIL, NIL, TO_CHAR)
    for i = 1, getn(designs) do
      buf = format('[%d] - (&g%5d&n) - %-20s', i, designs[i][2] * 1000, designs[i][1])
      act(buf, FALSE, ch, NIL, NIL, TO_CHAR)
    end

    act("\r\nAvailable dyes are:", FALSE, ch, NIL, NIL, TO_CHAR)
    for i = 1, getn(dyes) do
      buf = format('   %-20s', dyes[i])
      act(buf, FALSE, ch, NIL, NIL, TO_CHAR)
    end
    return (TRUE)
  end
end

function reply()
-- Answers to the various questions possibly asked by the player, and determine if
-- they have the appropriate materials with which to make the tattoo

  local buf = ""
  local buf2 = ""
  local design = NIL
  local dye = NIL

  if (tattoo_conv_state == 1) then
  buf = mxp("materials", "say materials?")
    say("Yes, that's right. Of course you need to provide me with the "..buf..".")
    tattoo_conv_state = 0
  elseif (tattoo_conv_state == 2) then
    buf = mxp("design", "say design?")
    buf2 = mxp("dyes", "say dyes?")
    say("I require the tattoo "..buf.." and the necessary "..buf2.." with which to make it.")

    if (ch.objs) then					-- Do we have what we need?
      for i = 1, getn(ch.objs) do
        if (ch.objs[i].vnum == 1209) then		-- Got the design
          design = TRUE
        elseif (strfind(ch.objs[i].alias, "dye", 1, 1)) then
          dye = TRUE					-- Got at least one dye
        end
      end
    end

    if (design) then
      if (dye) then
        tattoo_conv_state = 6
        create_event(me, ch, NIL, NIL, "reply", 1, LT_MOB)
      else
        tattoo_conv_state = 7
        create_event(me, ch, NIL, NIL, "reply", 1, LT_MOB)
      end
    else
      tattoo_conv_state = 8
      create_event(me, ch, NIL, NIL, "reply", 1, LT_MOB)
    end
  elseif (tattoo_conv_state == 3) then
    say("The best ones to find are the ancient runes, for they hold much power.")
    say("I'm not sure where they are myself, you'll have to explore a little to find them.")
    say("When you do, simply use a piece of parchment which you can obtain in most general stores.")
    tattoo_conv_state = 0
  elseif (tattoo_conv_state == 4) then
    buf = mxp("ingredients", "say ingredients?")
    say("There's a variety to choose from, you just need to find the "..buf..".")
    tattoo_conv_state = 0
  elseif (tattoo_conv_state == 5) then
    say("You'll need to search for those, and you'll need the use of the Alchemy skill.")
    tattoo_conv_state = 0
  elseif (tattoo_conv_state == 6) then
    say("And it looks like you've got them!")
    tattoo_conv_state = 0
  elseif (tattoo_conv_state == 7) then
    say("I can see you have a tattoo design, but you don't have any dyes.")
    tattoo_conv_state = 0
  elseif (tattoo_conv_state == 8) then
    if (dye) then
      say("I can see you have some dye, but you don't have a tattoo design.")
    end
    tattoo_conv_state = 0
  end
end

function code()
-- Front end scripting to tattoo artists. Format is [TYPE], cost, name, description

  local command = ""
  local subcmd = ""
  local buf = ""
  local designs = { }
  local dyes = { }
  local colors = { }
  local j = 1
  local k = 1
  local color_match = { "red", "black", "yellow", "blue", "white", "green", "orange", "purple" }

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    argument = gsub(argument, command.." ", "")
    if (strfind(argument, "%w%s") ~= NIL) then
      subcmd = strsub(argument, 1, strfind(argument, "%w%s"))
      argument = gsub(argument, subcmd.." ", "")
    else
      subcmd = argument
      argument = NIL
    end
  else
    command = argument
  end

  if (ch.tattoo > 0) then				-- Player already has a tattoo
    tell(ch.name, "You already possess a tattoo, I cannot give you another.")
    return
  end

  if (ch.objs) then
    for i = 1, getn(ch.objs) do				-- Let's look for a tattoo design
      if (ch.objs[i].vnum == 1209) then			-- Got the design
        designs[j] = { ch.objs[i].val[1], ch.objs[i].val[2], ch.objs[i] }
        j = j + 1
      elseif (strfind(ch.objs[i].alias, "dye", 1, 1)) then
        dyes[k] = ch.objs[i]				-- Got at least one dye
        k = k + 1
      end
    end
  end

  if (not designs[1]) then				-- Missing a component
    act("$n says, 'Sorry, you don't have any tattoo designs. I can do nothing for you.'",
      TRUE, me, NIL, ch, TO_VICT)
    return
  elseif (not dyes[1]) then
    act("$n says, 'Sorry, you don't have any dye I can use. I can do nothing for you.'",
      TRUE, me, NIL, ch, TO_VICT)
    return
  end

  if (subcmd == "") then
    tell(ch.name, "Buy what number?")
    return
  end

  local arg = tonumber(subcmd)				-- Did they select a proper number?
  if (not arg or (arg > getn(designs)) or (arg <= 0)) then
    tell(ch.name, "You want to buy what??")
    return
  end

  if (not argument) then				-- No color specified
    tell(ch.name, "Ok, I can do that...but what colors would you like?")
    return
  end

  j = 1
  while (argument) do					-- Let's find out what colors were specified
    if (strfind(argument, "%a%s")) then
      colors[j] = strlower(strsub(argument, 1, strfind(argument, "%a%s")))
    else
      colors[j] = strlower(argument)
      break
    end
    argument = gsub(argument, colors[j].." ", "")
    j = j + 1
  end

  if (getn(colors) > 3) then
    tell(ch.name, "Sorry, but I can only use a maximum of three colors...company policy.")
    return
  end

  k = getn(colors)
  for j = 1, getn(colors) do
    for i = 1, getn(dyes) do				-- Let's match the colors to dyes
      if (dyes[i]) then
        if (strfind(dyes[i].name, colors[j], 1, 1)) then
          colors[j] = dyes[i]
          dyes[i] = NIL					-- So we can't use the same dye again
          k = k - 1
          break
        end
      end
    end
  end

  if (k > 0) then					-- Didn't match them all
    tell(ch.name, "Sorry, but you don't have all of the dyes you specified.")
    return
  end

  if (ch.gold < designs[arg][2]) then			-- Do they have enough gold?
    tell(ch.name, "You lack the "..designs[arg][2].." gold pieces required for my services!")
    return
  end

  for i = 1, getn(colors) do				-- Find out which DYE bits to set
    for j = 1, getn(color_match) do
      if (strfind(colors[i].name, color_match[j], 1, 1)) then
        get_dye(ch, (j - 1))
        break
      end
    end
  end

  for i = 1, getn(colors) do				-- Clean up...remove dyes
    extobj(colors[i])
  end

  ch.gold = ch.gold - (designs[arg][2] * 1000)
  ch.tattoo = designs[arg][1]
  save_char(ch)
  extobj(designs[arg][3])

  act("$n starts to work on $N's tattoo...", TRUE, me, NIL, ch, TO_NOTVICT)
  act("A ghastly scream is ripped from $N's lips just before $E blacks out.",
    TRUE, me, NIL, ch, TO_NOTVICT)
  act("$n starts to work on your tattoo...", TRUE, me, NIL, ch, TO_VICT)
  act("The pain is incredible; it seems to eat into your soul.\r\n"
    .."A scream is ripped from your lips...", FALSE, ch, NIL, NIL, TO_CHAR)
  action(ch, "shout Arrrrrrrrrgggggggghhhh!")
  act("You black out.", FALSE, ch, NIL, NIL, TO_CHAR)
  return
end

