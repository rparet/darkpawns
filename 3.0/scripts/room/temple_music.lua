function enter()
-- A random midi file is played on entrance into a temple. It will remain playing until
-- the player exits the inn, quits the game or selects MSP to off.

  local files = { "temple1", "temple2", "temple3", "temple4", "temple5", "temple6" }
  local buf = files[number(1,getn(files))]

  msp("music", buf, 20, -1, 100, "Temple")
end
