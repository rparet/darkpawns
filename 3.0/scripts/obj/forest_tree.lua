function onpulse()
-- If the tree is empty, we want it to be removed from the zone and reloaded elsewhere.
-- Attached to obj 9116.

  if (obj.contents) then
    return
  else
    extobj(obj)
  end
end
