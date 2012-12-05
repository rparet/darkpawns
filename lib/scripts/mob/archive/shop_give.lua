function ongive()
-- We need to check the items that a player is giving shop keepers, specifically
-- player-owned stores. That way, we can stop them giving items that the shop
-- keeper wasn't meant to sell.

  local alias = ""

  if (not item_check(obj)) then		-- Item is not a production one!
    say("Sorry "..ch.name..", but I don't deal in such items.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end
end
