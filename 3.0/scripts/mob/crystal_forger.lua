function oncmd()
-- The crystal forger accepts crystalline chunks (11701) and converts them into a crystal
-- item depending upon the player's gold and number of chunks carried. Attached to mob
-- 7923.

  local command = ""
  local subcmd = ""
  local chunks = 0

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
      if (ch.objs[i].vnum == 11701) then
        chunks = chunks + 1
      end
    end
  end

  if (command == "list") then
    if (chunks == 0) then
      tell(ch.name, "Hmm..I don't see you carrying any crystalline chunks. Return when you have some.")
    else
      local buf = "one"
      local buf2 = "this crystal"
      local buf3 = "a nice pair of crystalline gloves"
      if (chunks > 1) then
        buf = "some"
        buf2 = "these crystals"
      end
      if (chunks >= 3) then
        buf3 = buf3..", a pair of crystalline leggings or sleeves"
      end
      if (chunks >= 5) then
        buf3 = buf3..", or an exquisite crystalline breastplate"
      end
      tell(ch.name, "Excellent, you have "..buf.."! With "..buf2..", I can forge for you "..buf3..".")
    end
    return (TRUE)
  elseif (command == "buy") then
    if (subcmd == "") then
      tell(ch.name, "What do you wish me to forge for you?")
    elseif (chunks == 0) then
      tell(ch.name, "Hmm..I don't see you carrying any crystalline chunks. Return when you have some.")
    else
      local gold = 0
      local item = 0
      local used = 0
      local buf = "one"
      local buf2 = "crystal"

      if (strlower(subcmd) == "gloves") then			-- What did the player specify?
        if (ch.gold < 200) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 200
          item = 11706
          used = 1
        end
      elseif (strlower(subcmd) == "leggings") then
        if (chunks < 3) then
          tell(ch.name, "You do not have enough crystalline chunks. Begone!")
          return(TRUE)
        end
        if (ch.gold < 500) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 500
          item = 11707
          used = 3
        end
      elseif (strlower(subcmd) == "sleeves") then
        if (chunks < 3) then
          tell(ch.name, "You do not have enough crystalline chunks. Begone!")
          return(TRUE)
        end
        if (ch.gold < 500) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 500
          item = 11708
          used = 3
        end
      elseif (strlower(subcmd) == "breastplate") then
        if (chunks < 5) then
          tell(ch.name, "You do not have enough crystalline chunks. Begone!")
          return(TRUE)
        end
        if (ch.gold < 1000) then
          tell(ch.name, "You do not have the gold I require. Begone!")
          return (TRUE)
        else
          gold = 1000
          item = 11709
          used = 5
        end
      else
        tell(ch.name, "I'm sorry, I can not forge that item.")
        return (TRUE)
      end
      ch.gold = ch.gold - gold
      obj = oload(ch, item, "char")
      save_char(ch)

      if (chunks > 1) then
        buf = "some"
        buf2 = "crystals"
      end
      act("$N hands $n "..buf.." "..buf2..".", TRUE, me, NIL, ch, TO_NOTVICT)
      act("You hand $n "..buf.." "..buf2..".", TRUE, me, NIL, ch, TO_VICT)
      act("Moments later, there is a brilliant flash and $n hands $p to $N.",
           TRUE, me, obj, ch, TO_NOTVICT)
      act("Moments later, there is a brilliant flash and $n hands $p to you.",
           TRUE, me, obj, ch, TO_VICT)

      for i = 1, used do				-- Extract the "used" chunks
        for j = 1, getn(ch.objs) do
          if (ch.objs[j].vnum == 11701) then
            extobj(ch.objs[j])
            tremove(ch.objs, j)
            break
          end
        end
      end
    end
    return (TRUE)
  end
end
