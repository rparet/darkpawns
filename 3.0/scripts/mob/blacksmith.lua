function sound()
  if (number(0, 1) == 0) then
    say("I seek the humming black armor; a complete set is my life's goal.")
  else
    say("There are twenty-two pieces and fear they shall never be found.")
  end
end

function ongive()
  local pieces = { 14201, 14202, 14203, 14204, 14205, 14206, 14207, 14208, 14209, 14210, 14211,
			 14212, 14213, 14214, 14215, 14216, 14217, 14218, 14219, 14220, 14221, 14222 }
  local one = NIL
  local all = 0
  local found = FALSE
  local alias = ""

  if (obj.vnum == 10306) then					-- Stone of Serapis
    if (ch.gold > 2000) then
      emote("studies the tooth carefully then begins to work...")
      extobj(obj)
      obj = oload(ch, 10307, "char")
      act("$n gives $N $p.", TRUE, me, obj, ch, TO_NOTVICT)
      act("$n gives you $p.", TRUE, me, obj, ch, TO_VICT)
      ch.gold = ch.gold - 2000
      save_char(ch)
    else
      say("You don't have the 2000 coins I need to work, come back later!")
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
      action(me, "give "..alias.." "..ch.name)
    end
    return
  else
    one = foreachi(pieces, one_piece)
    all = all_pieces()
  end

  for i = 1, getn(pieces) do
    if (obj.vnum == pieces[i]) then
      found = TRUE
      break
    end
  end

  if (found == FALSE) then
    say("I don't need this extra piece.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
  end

  if (one) then
    if (all == getn(pieces)) then
      obj = oload(ch, 14223, "char")
      act("\r\nTaking the pieces, $n forges them together.\r\nMagick rumbles through the smithy, and finally"
          .." the armor is done.\r\n", TRUE, me, NIL, NIL, TO_ROOM)
      act("The blacksmith hands you a nearly weightless suit of armor.\r\n", TRUE, me, NIL, ch, TO_VICT)
      act("$n hands $p to $N.", TRUE, me, obj, ch, TO_NOTVICT)
      extract_pieces()
    end
  end
end

function one_piece(index, value)
  local pieces = { 14201, 14202, 14203, 14204, 14205, 14206, 14207, 14208, 14209, 14210, 14211,
			 14212, 14213, 14214, 14215, 14216, 14217, 14218, 14219, 14220, 14221, 14222 }

  if (obj.vnum == pieces[index]) then			-- Do I haev at least one piece?
    return (TRUE)
  else
    return (NIL)
  end
end

function all_pieces()
  local pieces = { 14201, 14202, 14203, 14204, 14205, 14206, 14207, 14208, 14209, 14210, 14211,
			 14212, 14213, 14214, 14215, 14216, 14217, 14218, 14219, 14220, 14221, 14222 }
  local found = 0

  for i = 1, getn(me.objs) do					-- How many pieces do I have?
    for j = 1, getn(pieces) do
      if (me.objs[i].vnum == pieces[j]) then
        found = found + 1
        pieces[j] = 0
        break
      end
    end
  end
  return (found)
end

function extract_pieces()
  local pieces = { 14201, 14202, 14203, 14204, 14205, 14206, 14207, 14208, 14209, 14210, 14211,
			 14212, 14213, 14214, 14215, 14216, 14217, 14218, 14219, 14220, 14221, 14222 }

  for i = 1, getn(me.objs) do					-- Extract the forged pieces
    for j = 1, getn(pieces) do
      if (me.objs[i].vnum == pieces[j]) then
        extobj(me.objs[i])
        pieces[j] = 0
        break
      end
    end
  end
end
