function oncmd()
-- Provided players have a NOTE to write upon, they can make a copy of the rune
-- within the room to be used as a tattoo design. Obj 1209 is the designated
-- rune copy.

  -- We don't need these as globals, they're only used here
  local TATTOO_NONE     = 0
  local TATTOO_TRIBAL   = 1
  local TATTOO_SCORPION = 2
  local TATTOO_MOON     = 3
  local TATTOO_SUN      = 4
  local TATTOO_DRAGON   = 5
  local TATTOO_SKULL    = 6
  local TATTOO_ROSE     = 7
  
  local command = ""
  local subcmd = ""
  local buf = ""
  local rune_copy = NIL
  local rune = {
    [1402] =  { TATTOO_SUN,      "Aethen, the Sun God",                40 },
    [2627] =  { TATTOO_TRIBAL,   "a tribal symbol",                    10 },
    [4692] =  { TATTOO_MOON,     "Jehduti, the Moon God",              30 },
    [5631] =  { TATTOO_SCORPION, "a scorpion poised to strike",        25 },
    [6536] =  { TATTOO_SKULL,    "a skull resting on a bed of thorns", 8  },
    [9132] =  { TATTOO_ROSE,     "a wild rose in full blossom",        5  },
    [10214] = { TATTOO_DRAGON,   "a dragon rising from the flames",    20 }
  }

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    subcmd = gsub(argument, command.." ", "")
  else
    command = argument
  end

  if (command == "use") then
    if (not obj_list(subcmd, "char") or (obj.type ~= ITEM_NOTE)) then
      return
    end
    
    act("You make a rubbing of the rune using $p.", TRUE, me, obj, NIL, TO_CHAR)
    act("$n makes a rubbing of the rune using $p.", TRUE, me, obj, NIL, TO_ROOM)

    rune_copy = oload(me, 1209, "char")		-- Load the rune copy proto
    rune_copy.val[1] = rune[room.vnum][1]	-- Assign the type of tattoo
    rune_copy.val[2] = rune[room.vnum][3] 	-- Assign the cost in gold to acquire it
    save_obj(rune_copy)

    buf = "This one depicts "..rune[room.vnum][2].."."
    extra(rune_copy, buf)

    extobj(obj)					-- Remove the original NOTE
    return (TRUE)
  end
end
