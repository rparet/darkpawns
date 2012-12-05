function sound()
  if (number(0, 5) == 0) then
    say("Now where did I put that wand...")
  elseif (number(0, 5) == 0) then
    say("Perhaps I can be of assistance to you?")
  elseif (number(0, 5) == 0) then
    emote("reads through a rather large spell book.")
  elseif (number(0, 5) == 0) then
    say("I hear the great Phoenix has been seen recently.")
  elseif (number(0, 5) == 0) then
    say("I can remove the curse on any item you have for a price.")
  elseif (number(0, 5) == 0) then
    say("Books can be a wonderful source of knowledge!")
  elseif (number(0, 5) == 0) then
    social(me, "cough")
  end
end

function greet()
-- The mob will notify a player that he can remove cursed items should the player
-- possess one. Attached to mob

  local count = 0

  if (isnpc(ch)) then
    return
  end

  if (ch.objs) then
    for i = 1, getn(ch.objs) do
      if (obj_flagged(ch.objs[i], ITEM_NODROP)) then
        count = count + 1
      end
    end
  end

  if (ch.wear) then
    for i = 1, getn(ch.wear) do
      if (obj_flagged(ch.wear[i], ITEM_NODROP)) then
        count = count + 1
      end
    end
  end

  if (count > 0) then
    create_event(me, ch, NIL, NIL, "curse", 4, LT_MOB)
  end
end

function curse()
-- We have a cursed item! Now to offer our assistance.

  say("I can be of assistance to you "..ch.name..".")
  say("For a measly "..(ch.level * 50).." gold coins per item, I can remove the curse placed on it.")
  say("Simply type 'remove', followed by the name of the item and it shall be done!")
end

function oncmd()
-- Now to see if the player has asked for the removal of a curse

  local command = ""
  local subcmd = ""

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "remove") then
    if (subcmd == "") then
      return (FALSE)
    end

    if (ch.wear) then
      for i = 1, getn(ch.wear) do			-- Search worn equipment first
        if (strfind(ch.wear[i].alias, subcmd)) then
          if (ch.gold < (ch.level * 50)) then
            say("I'm sorry, you don't have the "..(ch.level * 50).." gold I require.")
            return (TRUE)
          elseif (not obj_flagged(ch.wear[i], ITEM_NODROP)) then
            say("That item is not cursed...perhaps you meant another one?")
            return (TRUE)
          else
            act("$n mutters a few words and $p briefly glows blue.", TRUE, me, ch.wear[i], NIL, TO_ROOM)
            ch.gold = ch.gold - (ch.level * 50)
            obj_extra(ch.wear[i], "remove", ITEM_NODROP)
            return (TRUE)
          end
        end
      end
    end

    if (ch.objs) then
      for i = 1, getn(ch.objs) do			-- Now search the inventory
        if (strfind(ch.objs[i].alias, subcmd)) then
          if (ch.gold < (ch.level * 50)) then
            say("I'm sorry, you don't have the "..(ch.level * 50).." gold I require.")
            return (TRUE)
          elseif (not obj_flagged(ch.objs[i], ITEM_NODROP)) then
            say("That item is not cursed...perhaps you meant another one?")
            return (TRUE)
          else
            act("$n mutters a few words and $p briefly glows blue.", TRUE, me, ch.objs[i], NIL, TO_ROOM)
            ch.gold = ch.gold - (ch.level * 50)
            obj_extra(ch.objs[i], "remove", ITEM_NODROP)
            return (TRUE)
          end
        end
      end
    end

    say("I'm sorry, you don't seem to have one of those.")
    return (TRUE)
  end
end
