function onpulse_all()
-- The mob will retrieve crystalline chunks (obj 11701) from any wooden crate
-- (obj 11702) it comes across and when it passes the mine entrance (room 11708),
-- it will deposit the chunks into a "bucket" which will remove them from the game.
-- Attached to mob 11706.

  local alias = ""
  local buf = ""
  local buf2 = ""
  local crate = NIL
  local chunk = NIL
  local amount = 0

  if (room.vnum == 11708) then				-- At the entrance
    if (me.objs) then
      for i = 1, getn(me.objs) do
        if (me.objs[i].vnum == 11701) then
          amount = amount + 1
          extobj(me.objs[i])					-- Has a chunk, remove it
        end
      end

      if (amount > 1) then
        buf = "a few"
        buf2 = "chunks"
      else
        buf = "a"
        buf2 = "chunk"
      end

      act("$n puts "..buf.." crystalline "..buf2.." into a small wooden bucket.",
        TRUE, me, NIL, NIL, TO_ROOM)
      act("Shortly after, the small wooden bucket disappears into the darkness before "
        .."reappearing moments later, empty.", FALSE, me, NIL, NIL, TO_ROOM)
    end
  end

  if (room.objs) then
    for i = 1, getn(room.objs) do
      if (room.objs[i].vnum == 11702) then		-- Found a crate
        if (me.objs and (getn(me.objs) > 3)) then
          return
        end

        crate = room.objs[i]
        if (crate.contents) then				-- Does it have anything in it?
          for j = 1, getn(crate.contents) do
            if (crate.contents[j].vnum == 11701) then
              chunk = crate.contents[j]
              alias = strsub(chunk.alias, 1, strfind(chunk.alias, "%a%s"))
              action(me, "get all."..alias.." from crate")
              return
            end
          end
        end
      end
    end
  end

end
            
