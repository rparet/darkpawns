function enter()
-- When a player enters the room, if they do not possess a recall scroll then they are
-- automatically given one.

  local scroll = NIL
  local count = 1
  local container = { }

  if (isnpc(me)) then
    return
  end

  if (me.objs) then						-- Let's look for a scroll in their inventory
    if (obj_list("recall", "char")) then
      return
    end
  end

  if (me.wear) then						-- Worn items containing the scroll
    for i = 1, getn(me.wear) do
      if (me.wear[i].contents) then
        container[count] = me.wear[i]
        count = count + 1
       end
    end
  end

  if (me.objs) then						-- Inventory items containing the scroll
    for i = 1, getn(me.objs) do
      if (me.objs[i].contents) then
        container[count] = me.objs[i]
        count = count + 1
      end
    end
  end

  if (count > 1) then						-- We found at least one container
    for i = 1, getn(container) do
      for j = 1, getn(container[i].contents) do
        if (container[i].contents[j].vnum == 8052) then
          return
        end

        if (container[i].contents[j].contents) then		-- A container within a container
          for count = 1, getn(container[i].contents[j].contents) do
            if (container[i].contents[j].contents[count].vnum == 8052) then
              return
            end
          end
        end
      end
    end
  end

  if (get_spell(me, SPELL_WORD_OF_RECALL)) then			-- Char knows the recall spell
    return
  end

  scroll = oload(me, 8052, "char")
  obj_extra(scroll, "set", ITEM_NODROP)
  act("$p magically appears in your inventory.", TRUE, me, scroll, NIL, TO_CHAR)
end
