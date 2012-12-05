function assemble_one(singles, giveback, trade)
  -- { original object, cost, taken object name, final object }
  -- checks for single forging items
  for i = 1, getn(singles) do
    if (obj.vnum == singles[i][1]) then
      if (ch.gold >= singles[i][2]) then
	if (trade == FALSE) then
	  act("$n studies $p.", TRUE, me, obj, NIL, TO_ROOM)
	  act("$n starts working with $p...", TRUE, me, obj, NIL, TO_ROOM)
	end
	extobj(obj)
	obj = oload(ch, singles[i][4], "char")
	act("$n gives $N $p.", TRUE, me, obj, ch, TO_NOTVICT)
	act("$n gives you $p.", TRUE, me, obj, ch, TO_VICT)
	ch.gold = ch.gold - singles[i][2]
	save_char(ch)
      else
	return_obj("That will cost "..format("%u", singles[i][2])
		 .." coins which you don't have, come back later!", FALSE)
      end
      return (TRUE)
    end
  end
  if (giveback == TRUE) then
    return_obj("I don't need this.", FALSE)
    return (TRUE)
  end
  return (FALSE)
end

function assemble_list(list, newobj, cost)
  -- multiple pieces for one object
  -- { {list of objects}, final object, cost }
  if (one_piece(list) == TRUE) then
    if (count_piece(obj.vnum) > 1) then
      return_obj("I don't need this extra piece.", TRUE)
      return (3)
    end
    if (all_pieces(list) == TRUE) then
      if (ch.gold >= cost) then
	obj = oload(ch, newobj, "char")
	extract_pieces(list)
	ch.gold = ch.gold - cost
	save_char(ch)
	return (1)
      else
	return_obj("That will cost "..format("%u", cost)
		 .." coins which you don't have, come back later!", FALSE)
	return (3)
      end
    end
    return (2)
  end
  return (NIL)
end

function return_obj(msg, usetell)
  local alias = ""
  
  if (usetell == TRUE) then
    tell(ch.name, msg)
  else
    say(msg)
  end
  
  found, t, alias = strfind(obj.alias, "^(%a+)")
  if not found then
    log("[LUA] Could not return object.")
    return FALSE
  end
  
  action(me, "give "..alias.." "..ch.name)
  
  return TRUE
end

-- is this a piece?
function one_piece(pieces)
  local found = FALSE
  for i = 1, getn(pieces) do
    if (obj.vnum == pieces[i]) then
      found = TRUE
      break
    end
  end
  
  return (found)
end

-- Do I have one of each?
function all_pieces(pieces)
  local all = 0
  
  for i = 1, getn(pieces) do
    if (has_piece(pieces[i]) == TRUE) then
      all = all + 1
    end
  end
  
  if (all == getn(pieces)) then
    return (TRUE)
  else
    return (FALSE)
  end
end

-- extract each piece
function extract_pieces(pieces)
  for i = 1, getn(pieces) do
    local j = find_piece(pieces[i])
    if (j ~= 0) then
      extobj(me.objs[j])
    end
  end
  
  return
end

-- how many do I have?
function count_piece(piece)
  local found = 0
	
  for i = 1, getn(me.objs) do
    if (me.objs[i].vnum == piece) then
      found = found + 1
    end
  end
  
  return (found)
end

-- if I have this piece, what is it's index?
function find_piece(piece)
  local found = 0
  
  for i = 1, getn(me.objs) do
    if (me.objs[i].vnum == piece) then
      found = i
      break
    end
  end
  
  return (found)
end

-- Do I have this piece already?
function has_piece(piece)
  local found = FALSE
  
  for i = 1, getn(me.objs) do
    if (me.objs[i].vnum == piece) then
      found = TRUE
      break
    end
  end
  
  return (found)
end