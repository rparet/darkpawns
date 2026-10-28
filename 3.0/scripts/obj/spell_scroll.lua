function oncmd()
-- When a player uses this scroll, their skill level for the appropriate spell will increase
-- to 100%. If they have not already learned the spell or the associated group, the parchment
-- (obj 1280) is useless to them.

  local command = ""
  local subcmd = ""
  local buf = ""
  local exp = 0

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "use") then
    if (not obj_list(subcmd, "char") or (obj.vnum ~= 1280)) then
      return
    end

    -- Do I already know the spell?
    if (get_spell(ch, obj.val[1])) then
      act("Your already have knowledge of this spell.", FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    ch.group_lvl = get_group_lvl(ch, obj.val[2])

    if (ch.group_lvl == 0) then		-- Doesn't know the group
      buf = "You are unfamiliar with the basics of this spell. The parchment is useless to you!"
      act(buf, FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    -- Let's see if they know the correct group level
    if (obj.val[3] > ch.group_lvl) then
      act("You lack the level of knowledge required to learn this spell.",
          FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    -- Is there enough unassigned exp to learn?
    exp = 10000 * ch.group_lvl
    if (ch.exp_q < exp) then
      act("You lack the experience necessary to learn this spell.",
          FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    act("$n reads from $p and it disappears in a flash of light.", TRUE, ch, obj, NIL, TO_ROOM)
    act("You read from $p and it disappears in a flash of light.", TRUE, ch, obj, NIL, TO_CHAR)
    buf = "You feel like you've learned something.\r\nYou also feel very tired."
    act(buf, FALSE, ch, NIL, NIL, TO_CHAR)

    get_spell(ch, obj.val[1], "1")
    ch.exp_q = ch.exp_q - exp
    ch.hp = 1
    ch.mana = 1
    ch.move = 1

    extobj(obj)
    return (TRUE)
  end
end
