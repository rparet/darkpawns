function ongive()
-- When the player gives the mob a shop deed (obj 1222), the player is refunded the cost
-- of the purchase. Attached to mobs 4812, 5315 and 18228.

  local lifestyle = 0
  local pack = NIL
  local item = NIL
  local alias = ""
  local equip_invent =
    { [1] = { 5303, 5305, 5307 },
      [2] = { 8040, 8021 },
      [3] = { 8019, 8023 }
    }
  local equip_pack =
    { [1] = { 5331, 5314 },
      [2] = { 8010, 5314 },
      [3] = { 19104, 8063 }
    }
    

  if ((obj.vnum == 1245) or (obj.vnum == 1246) or (obj.vnum == 1247)) then
    if (obj.val[1] ~= ch.id) then			-- This letter doesn't belong to this player!
      say("Hmmm...this letter does not belong to you "..ch.name.."!")
      act("$n files $p into a desk drawer.", TRUE, me, obj, NIL, TO_ROOM)
      extobj(obj)
      obj = NIL
      return
    end

    if (obj.vnum == 1245) then
      lifestyle = 1
    elseif (obj.vnum == 1246) then
      lifestyle = 2
    elseif (obj.vnum == 1247) then
      lifestyle = 3
    end
  end

  if ((obj.vnum ~= 1222) and (lifestyle == 0)) then
    say("I'm sorry, I have no use for that...you better keep it.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  if (lifestyle ~= 0) then
    say("Ah "..ch.name..", I've been expecting you. Welcome!")
    act("$n disappears into a side room and rummages around.", TRUE, me, NIL, NIL, TO_ROOM)

    extobj(obj)
    obj = NIL
    pack = oload(me, 8038, "char")			-- Load the pack and bond certificate
    item = oload(me, 8030, "char")
    objfrom(item, "char")
    objto(item, "obj", pack)
    item = oload(me, 1248, "char")
    objfrom(item, "char")
    objto(item, "obj", pack)
    item.val[1] = lifestyle
    item.val[2] = ch.id
    save_obj(item)

    for index, value in equip_invent[lifestyle] do	-- Inventory items
      item = oload(me, value, "char")
      obj_extra(item, "set", ITEM_NOSELL)
    end

    for index, value in equip_pack[lifestyle] do	-- Pack items
      item = oload(me, value, "char")
      objfrom(item, "char")
      objto(item, "obj", pack)
    end

    create_event(me, ch, NIL, NIL, "equipped", 1, LT_MOB)
    return
  end

  ch.gold = ch.gold + (0.9 *(obj.val[4] * 10000))	-- Refund 90% of the cost of the store
  extobj(obj)

  tell(ch.name, "I'm sorry the store didn't work out for you.")
  act("$n returns a large portion of your gold, keeping the rest as account fees.",
    TRUE, me, NIL, ch, TO_VICT)
  act("$n gives $N a large portion of gold.", TRUE, me, NIL, ch, TO_NOTVICT)
end

function equipped()
-- Give them all of the goodies they need

  action(me, "give all "..ch.name)
  create_event(me, NIL, NIL, NIL, "commence", 4, LT_MOB)
end

function commence()
-- Now that the player is equipped with some starting equipment, we can provide them a
-- bit of guidance.

  say("You should now work towards improving your experience in combat. You can fight creatures"..
    " beyond the city walls. When you have sufficient experience, you can seek out the city's teacher.")
  say("They can train you in a number of skills and spells that you will need to survive,"..
    " and you can usually find them in the city library.")
  say("I've given you a bond certificate that you should take to the bank after reaching"..
    " your 5th level. They will provide you with a sum of gold coins to assist you further.")
  emote("goes back to signing some paperwork.")
end
