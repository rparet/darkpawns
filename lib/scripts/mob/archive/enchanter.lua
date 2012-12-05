function sound()
  if (number(0, 5) == 0) then
    say("Hmm, I don't often have guests down here.")
  elseif (number(0, 5) == 0) then
    say("Perhaps I can be of assistance to you?")
  elseif (number(0, 5) == 0) then
    say("If you have a pile of elemental dust, I can help you.")
  elseif (number(0, 5) == 0) then
    say("Do you seek an enchantment?")
  elseif (number(0, 5) == 0) then
    say("I can sell you some bare necessities to assist in your quest.")
  elseif (number(0, 5) == 0) then
    say("Seek out Bane and Valoran, they may be able to help you further.")
  elseif (number(0, 5) == 0) then
    say("Be sure to study the art of sorcery before climbing the tower.")
  end
end

function ongive()
-- Provided an enchantable item is given to the mob and the player possesses some
-- elemental dust (1314), the item is enchanted at "god" level and handed back
-- to the player. Attached to mob 1400.

  local dust = NIL
  local alias = ""

  if ((obj.type ~= ITEM_WORN) and (obj.type ~= ITEM_ARMOR) and (obj.type ~= ITEM_WEAPON)) then
    tell(ch.name, "I thank you for your gift, but unfortunately I cannot enchant this item.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  if (obj_flagged(obj, ITEM_MAGIC)) then	-- Item already MAGIC
    tell(ch.name, "This item is magical in nature already. I am unable to enchant it for you.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
    return
  end

  if (ch.objs) then
    for i = 1, getn(ch.objs) do			-- Do they possess elemental dust?
      if (ch.objs[i].vnum == 1314) then
        dust = ch.objs[i]
        break
      end
    end
  end

  if (dust) then
    act("$n takes $p from $N.", TRUE, me, dust, ch, TO_NOTVICT)
    act("$n takes $p from you.", TRUE, me, dust, ch, TO_VICT)

    if (obj.type == ITEM_WEAPON) then		-- Appropriate spell based on type
      spell(NIL, obj, SPELL_ENCHANT_WEAPON, TRUE)
    else
      spell(NIL, obj, SPELL_ENCHANT_ARMOR, TRUE)
    end

    tell(ch.name, "There you are my friend, may it serve you well.")
    extobj(dust)
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
  else
    tell(ch.name, "You do not possess the elemental dust I require. Fetch some if you want "
      .. "your items enchanted.")
    if (strfind(obj.alias, "%a%s")) then
      alias = strsub(obj.alias, 1, strfind(obj.alias, "%a%s"))
    else
      alias = obj.alias
    end
    action(me, "give "..alias.." "..ch.name)
  end
end
