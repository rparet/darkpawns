function onget()
-- If a player gets the Tear of Seas (19131) and provided they don't already have one,
-- Francesca (19126) will begin hunting them. Attached to room 19121.

  local tear = 0
  local francesca = NIL

  if (obj.vnum == 19131) then				-- Did they get the Tear of Seas?
    for i = 1, getn(me.objs) do			-- Do they already have one?
      if (me.objs[i].vnum == 19131) then
        tear = tear + 1
      end
    end

    if (me.wear) then					-- Include worn items
      for i = 1, getn(me.wear) do
        if (me.wear[i].vnum == 19131) then
          tear = tear + 1
        end
      end
    end

    if (tear == 1) then
      francesca = inworld("mob", 19126)			-- Locate Francesca in the world
      if (francesca) then
        set_hunt(francesca, me)
      end
    end
  end
end
