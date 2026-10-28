function oncmd()
-- The dragon forger accept dragon scales and converts them into a dragonscale item dependant
-- upon the player's gold and number of scales carried. Attached to mob 7917.

  local command = ""
  local subcmd = ""
  local scales = 0
  local used = 0

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (not ch) then
    return
  end

  if (ch.objs) then
    for i = 1, getn(ch.objs) do
      if (ch.objs[i].vnum == 10204) then
        scales = scales + 1
      end
    end
  end

  if (command == "list") then
    if (scales == 0) then
      tell(ch.name, "Hmm..I don't see you carrying any dragon scales. Return when you have some.")
    else
      local buf = "one"
      local buf2 = "this scale"
      local buf3 = "a nice pair of dragon scale gloves"
      if (scales > 1) then
        buf = "some"
        buf2 = "these scales"
      end
      if (scales >= 3) then
        buf3 = buf3..", a pair of dragon scale leggings or sleeves"
      end
      if (scales >= 5) then
        buf3 = buf3..", or an exquisite dragon scale breastplate"
      end
      tell(ch.name, "Excellent, you have "..buf.."! With "..buf2..", I can forge for you "..buf3..".")
    end
    return (TRUE)
  elseif (command == "buy") then
    if (subcmd == "") then
      tell(ch.name, "What do you wish me to forge for you?")
    elseif (scales == 0) then
      tell(ch.name, "Hmm..I don't see you carrying any dragon scales. Return when you have some.")
    else
      local gold = 0
      local item = 0
      local buf = "one"
      local buf2 = "scale"

      if (strlower(subcmd) == "gloves") then		-- What did the player specify?
        if (ch.gold < 2000) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 5000
          item = 7906
          used = 1
        end
      elseif (strlower(subcmd) == "leggings") then
        if (scales < 3) then
          tell(ch.name, "You do not have enough dragon scales. Begone!")
          return(TRUE)
        end
        if (ch.gold < 5000) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 5000
          item = 7907
          used = 3
        end
      elseif (strlower(subcmd) == "sleeves") then
        if (scales < 3) then
          tell(ch.name, "You do not have enough dragon scales. Begone!")
          return(TRUE)
        end
        if (ch.gold < 5000) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 5000
          item = 7908
          used = 3
        end
      elseif (strlower(subcmd) == "breastplate") then
        if (scales < 5) then
          tell(ch.name, "You do not have enough dragon scales. Begone!")
          return(TRUE)
        end
        if (ch.gold < 9000) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 9000
          item = 7909
          used = 5
        end
      else
        tell(ch.name, "I'm sorry, I can not forge that item.")
        return (TRUE)
      end
      ch.gold = ch.gold - gold
      obj = oload(ch, item, "char")
      save_char(ch)

      if (scales > 1) then
        buf = "some"
        buf2 = "scales"
      end
      act("$N hands $n "..buf.." dragon "..buf2..".", TRUE, me, NIL, ch, TO_NOTVICT)
      act("You hand $n "..buf.." dragon "..buf2..".", TRUE, me, NIL, ch, TO_VICT)
      act("Moments later, there is a brilliant flash and $n hands $p to $N.",
           TRUE, me, obj, ch, TO_NOTVICT)
      act("Moments later, there is a brilliant flash and $n hands $p to you.",
           TRUE, me, obj, ch, TO_VICT)

      for i = 1, used do				-- Extract the "used" scales
        for j = 1, getn(ch.objs) do
          if (ch.objs[j].vnum == 10204) then
            extobj(ch.objs[j])
            tremove(ch.objs, j)
            break
          end
        end
      end
    end
    return (TRUE)
  elseif (command == "look") then
    if ((subcmd ~= "") and strfind(me.alias, subcmd)) then
      if (ch.objs) then
        for i = 1, getn(ch.objs) do
          if (ch.objs[i].vnum == 7903) then
            return
          end
        end
      end

      act("$n hands you a small card.\r\n", TRUE, me, NIL, ch, TO_VICT)
      act("$n hands $N a small card.", TRUE, me, NIL, ch, TO_NOTVICT)
      oload(ch, 7903, "char")
    end
  end
end
