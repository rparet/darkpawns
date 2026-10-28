function enter()
-- A random midi file is played on entrance into a inn. It will remain playing until
-- the player exits the inn, quits the game or selects MSP to off.

  local files = { "inn1", "inn2", "inn3", "inn4", "inn5", "inn6" }
  local buf = files[number(1,getn(files))]

  msp("music", buf, 20, -1, 100, "Inn")
end
