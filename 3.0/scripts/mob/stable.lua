function code()
  local command = ""
  local mounts = 
    {  [8022] = 8021, 
      [18210] = 8021,
       [4821] = 4823,
      [21217] = 8021 }
  local creature = NIL
    
  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (command == "list") then
    tell(ch.name, "You can buy a mount for 300 gold coins.")
    return
  elseif (command == "buy") then
    if (ch.followers >= (ch.cha / 2)) then		-- Follower limit
      tell(ch.name, "You can't have any more followers!")
      return
    elseif (ch.gold < 300) then				-- Not enough gold
      tell(ch.name, "You can't afford a mount!")
      return
    else
      ch.gold = ch.gold - 300
      tell(ch.name, "That'll be 300 coins, treat 'er well.")
      return (mounts[me.vnum])
    end
  elseif (command == "stable") then
    if (aff_flagged(ch, AFF_MOUNT)) then
      mount(ch, NIL, "unmount")
      if (room.char) then
        creature = find_mount()				-- Locate the mount in the room

        if (not creature) then
          tell(ch.name, "How do you expect to stable a mount that isn't here?")
          return
        end

        tell(ch.name, "I'll take good care of 'er for 5 coins a day.")
        aff_flags(creature, "remove", AFF_CHARM)
        return (creature.vnum)
      else
        tell(ch.name, "How do you expect to stable a mount that isn't here?")
        return
      end
    else
      if (not ch.followers) then			-- The player has no followers
        tell(ch.name, "How do you expect to stable a mount? You have no mount!")
        return
      else
        if (room.char) then
          creature = find_mount()			-- Locate the mount in the room

          if (not creature) then
            tell(ch.name, "How do you expect to stable a mount that isn't here?")
            return
          end

          tell(ch.name, "I'll take good care of 'er for 5 coins a day.")
          aff_flags(creature, "remove", AFF_CHARM)
          return (creature.vnum)
        else
          tell(ch.name, "How do you expect to stable a mount that isn't here?")
          return
        end
      end
    end
  elseif (command == "collect") then
  -- "argument" will be of the form "<COMMAND> mount <VNUM> time <TIME>". Need to extract
  -- individual elements for use.

    local vnum = 0
    local time = 0
    local rent = 0
    local days = 0
    local cost = 0

    say(argument)
    vnum = tonumber(strsub(argument, strfind(argument, "%d%d"), strfind(argument, "%d%s")))
    time = strsub(argument, strfind(argument, "[e]%s"), -1)
    time = strsub(time, strfind(time, "%d"), -1)

    if (not vnum) then
      tell(ch.name, "Hey now, you need to have stabled a mount to pick one up.")
      return (FALSE)
    end

    rent = tonumber(time)
    if (rent <= 0) then					-- Determine the stable cost
      days = 1
    else
      days = rent / 60 / 60 / 24
      if (days <= 0) then
        days = 1
      end
    end

    cost = round(days) * 5
    if (ch.gold < cost) then
      tell(ch.name, "Hey, you can't afford the "..cost.." gold you need to get your mount back.")
      return (FALSE)
    end

    tell(ch.name, "Here ya go, all patted down and ready to go...cost ya "..cost..
         " coins to keep 'em here.")
    ch.gold = ch.gold - cost
    return (vnum)
  end
end

function find_mount()
-- Locate any creature within the room that is MOUNTABLE and determine if the mount's
-- leader is the character collecting the mount.

  local creature = NIL

  for i = 1, getn(room.char) do
    if (mob_flagged(room.char[i], MOB_MOUNTABLE)) then
      if (aff_flagged(room.char[i], AFF_CHARM)) then
        if (room.char[i].leader.name == ch.name) then	-- Found our mount
          creature = room.char[i]
          break
        end
      end
    end
  end

  return (creature)
end

