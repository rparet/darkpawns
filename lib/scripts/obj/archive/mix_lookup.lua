function code()
-- Lookup table for the do_mix section of code.

local mix = { 0, 0, 0, 0 }
local total = 0
local solution =
  -- Potions
  { [1260] = { 1225, 1250, 1251, 1256 },	-- purple potion
    [1261] = { 1225, 1250, 1252, 1255 },	-- vial of amber liquid
    [1262] = { 1225, 1250, 1253, 1254 },	-- dark red elixir
    [1263] = { 1225, 1251, 1253, 1254 },	-- dark blue elixir
    [1264] = { 1225, 1251, 1253, 1255 },	-- glowing white potion
    [1279] = { 1225, 1253, 1254, 1255 },	-- bubbly amber elixir
    [1265] = { 1251, 1252, 1255, 1256 },	-- vial of clear liquid

    [1266] = { 0, 1225, 1250, 1255 },	-- vial of bubbling liquid
    [1267] = { 0, 1225, 1250, 1253 },	-- red potion
    [1268] = { 0, 1225, 1253, 1254 },	-- white potion
    [1269] = { 0, 1250, 1251, 1252 },	-- green potion
    [1270] = { 0, 1250, 1251, 1256 },	-- black potion
    [1271] = { 0, 1250, 1252, 1255 },	-- opaque elixir
    [1272] = { 0, 1251, 1255, 1256 },	-- vial of milky-white liquid
    [1273] = { 0, 1253, 1254, 1255 },	-- swirling red and yellow potion
    [1274] = { 0, 1254, 1255, 1256 },	-- dark yellow potion

    [1275] = { 0, 0, 1225, 1250 },        -- golden elixir
    [1276] = { 0, 0, 1250, 1255 },        -- blue potion
    [1277] = { 0, 0, 1253, 1254 },        -- a swirling green and yellow potion
    [1278] = { 0, 0, 1254, 1256 },        -- a yellow potion

  -- Dyes
    [1281] = { 0, 0, 20307, 20307 },      -- red dye
    [1283] = { 0, 0, 20308, 20308 },      -- black dye
    [1284] = { 0, 0, 5334, 5334 },        -- yellow dye
    [1286] = { 0, 0, 9112, 9112 },        -- blue dye
    [1287] = { 0, 0, 0, 9124},		  -- white dye
    [1282] = { 5334, 9112, 9112, 9124 },  -- green dye (blue and yellow)
    [1285] = { 5334, 5334, 9124, 20307 }, -- orange dye (yellow and red)
    [1288] = { 9112, 9112, 9124, 20307 }, -- purple dye (red and blue)

  -- Preservatives
    [1290] = { 0, 0, 8016, 8016 },        -- vial of crushed herbs
    [1291] = { 0, 0, 20309, 20309 } }     -- vial of cedar oil

  for i = 1, getn(mix) do			-- Find the individual components
    if (strfind(argument, "%d%s") ~= NIL) then
      mix[i] = strsub(argument, 1, strfind(argument, "%d%s"))
      argument = gsub(argument, mix[i].." ", "", 1)
    end
    mix[i] = tonumber(mix[i])
  end

  sort(mix)					-- Sort the reagent list
  for index, value in solution do
    if (index) then
      total = 0
      for i = 1, 4 do				-- Locate the correct object vnum to load
        if (mix[i] == solution[index][i]) then
          total = total + 1
        end
      end
      if (total == 4) then
        total = index
        break
      end
    end
  end

  return(total)
end


