function oncmd()
-- If a player possess the head of another player, the mob will convert it into
-- a wearable necklace. Additional heads may be placed on the necklace at a later
-- time. Attached to mob 7920.

  local command = ""
  local buf = ""
  local necklace = NIL
  local heads = 0
  local cost = 0
  local space = 10

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if ((command == "list") or (command == "buy")) then
    if (ch.objs) then
      for i = 1, getn(ch.objs) do		-- How many PC heads does the player have?
        if (ch.objs[i].owner) then
          heads = heads + 1
          cost = cost + 200
        end
        if (ch.objs[i].vnum == 7925) then	-- Already possesses a necklace
          necklace = ch.objs[i]
        end
      end
    end
  end

  if (command == "list") then
    if (heads > 0) then
      buf = "I can make a necklace for you at a cost of "..cost.." gold coins."
    else
      buf = "Do not bother me unless you have the head of another player!"
    end

    tell(ch.name, buf)
    return (TRUE)
  elseif (command == "buy") then
    if (heads == 0) then
      tell(ch.name, "Do not bother me unless you have the head of another player!")
      return (TRUE)
    end

    if (ch.gold < cost) then			-- Not enough cash to pay for it
      tell(ch.name, "Do not try my patience! My work comes at a fee of 200 gold per "
        .."head and you don't have the gold.")
      return (TRUE)
    end

    if (necklace == NIL) then			-- No necklace, load one
      necklace = oload(ch, 7925, "char")
    end

    space = 15 - necklace.weight		-- How many spaces are left on the neckalce?
    if (space == 0) then
      tell(ch.name, "Well well my friend, you have been busy. Unfortunately you have no "
        .."more space on your necklace and I can do no more for you.")
      return (TRUE)
    elseif ((space - heads) < 0) then
      tell(ch.name, "You have been busy I see...your necklace can only hold 10 heads so "
        .."I shall make use of what space is left.")
    end

    heads = space				-- Only "space" number of spaces left
    buf = make_necklace(heads, necklace)	-- Make a string of the head names
    extra(necklace, buf)
    necklace.weight = necklace.weight + heads
    save_obj(necklace)				-- Save the object

    if (cost > (heads * 200)) then
      cost = (heads * 200)
    end

    act("$n hands some gold and a number of heads to $N.", TRUE, ch, NIL, me, TO_ROOM)
    act("You hand over some gold and your heads to $N.", TRUE, ch, NIL, me, TO_CHAR)
    ch.gold = ch.gold - cost
    tell(ch.name, "My fee comes to "..cost.." coins, enjoy your trophy.")
    act("After a few minutes work, $N hands a necklace to $n.", TRUE, ch, NIL, me, TO_ROOM)
    act("After a few minutes work, $N hands a necklace to you.", TRUE, ch, NIL, me, TO_CHAR)
    save_char(ch)

    return (TRUE)
  end
end

function make_necklace(heads, necklace)
  local buf = ""
  local length = 0
  local line = 1
  local count = (necklace.weight - 4)

  if (count > 5) then				-- Need to find how many heads are already there
    count = count - 5
  end

  if (ch.objs) then
    for i = 1, getn(ch.objs) do
      if (ch.objs[i].owner) then
        if (count == 5) then			-- 4 names per line, add new line
          buf = buf.."\r\n"
          count = 1
        end

        length = strlen(ch.objs[i].owner)	-- The length of the name
        while ((29 - length) ~= 0) do
          buf = buf.." "				-- Insert spaces - pad out name
          length = length + 1
        end

        buf = buf.." "..ch.objs[i].owner
        count = count + 1
        extobj(ch.objs[i])
      end
    end
  end

  return (buf)
end
