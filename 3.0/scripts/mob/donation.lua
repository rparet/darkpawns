function onpulse_all()
-- The mob will sort objects in the room, disposing of them if required or storing
-- them in one of the room's containers...relies upon the containers being present!

  local alias = ""

  if (room.objs) then
    for i = 1, getn(room.objs) do
      if (canget(room.objs[i])) then
        action(me, "get all")
      end
    end
  end

  if (me.objs) then
    for i = 1, getn(me.objs) do
      if (strfind(me.objs[i].alias, "%a%s")) then
        alias = strsub(me.objs[i].alias, 1, strfind(me.objs[i].alias, "%a%s"))
      else
        alias = me.objs[i].alias
      end

      if ((me.objs[i].type == ITEM_TRASH) or (me.objs[i].type == ITEM_KEY) or
         (me.objs[i].type == ITEM_NOTE) or (me.objs[i].type == ITEM_PEN) or
         (me.objs[i].type == ITEM_FOOD)) then		-- Useless
        action(me, "junk "..alias)
        return
      end

      if (obj_flagged(me.objs[i], ITEM_BROKEN)) then	-- Broken
        action(me, "junk "..alias)
        return
      end

      if (me.objs[i].cost < 10) then			-- Worthless
        action(me, "junk "..alias)
        return
      end

      if ((me.objs[i].type == ITEM_ARMOR) or (me.objs[i].type == ITEM_WORN)) then
        action(me, "open case")
        action(me, "put "..alias.." in case")
      elseif (me.objs[i].type == ITEM_WEAPON) then
        action(me, "open cabinet")
        action(me, "put "..alias.." in cabinet")
      else
        action(me, "open chest")
        action(me, "put "..alias.." in chest")
      end

      action(me, "close case")
      action(me, "close cabinet")
      action(me, "close chest")
    end
  end
end
