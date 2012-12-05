function onpulse_all()
-- The bear cub will wander around until it meets it's mother, then will follow it

  if (room.char) then
    if (not me.leader) then			-- No master, continue
      for i = 1, getn(room.char) do
        if (room.char[i].vnum == 9111) then	-- Is mama bear here?
          follow(room.char[i], TRUE)		-- Follow mama bear, CHARM cub
          break
        end
      end
    end
  end
end
