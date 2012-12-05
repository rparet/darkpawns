function oncmd()
-- To prevent the players from executing any command, we'll trap it here and do
-- nothing. We need to allow the "recall" command to work properly otherwise the
-- player will always get stuck here.

  local command = ""
  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
  else
    command = argument
  end

  if (command ~= "recall") then
    if (me.level < LVL_IMMORT) then
      return (TRUE)
    end
  end
end


function onpulse()
-- Display text to each character upon initial entry into the game. Following the
-- speech, teleport them to room XXXX where they will undergo the character
-- creation questions.

  local buf = ""

  if (me.level < LVL_IMMORT) then
    buf = "   Suddenly, the hairs on the back of your neck stand up as if lightning"
        .." had\r\nstruck nearby. A keen wailing fills the air and an ethereal"
        .." image appears\r\nbefore you.\r\n\n"
    buf = buf.."   '"..me.name..", now is not your time to die,' speaks the figure.\r\n"
        .."   'Prove your worth and I may well grant you eternal life.'\r\n   'Trust no"
        .." one, for all here are but dark pawns above which you must\r\n    struggle to"
        .." prove yourself. All here strive to be a king...at any cost.'\r\n\n   The"
        .." figure glows for a moment, then disappears but the voice remains.\r\n"
        .."   'Your life begins now...' it says, as the world around you fades...\r\n"

    act(buf, FALSE, me, NIL, NIL, TO_CHAR)
    action(me, "recall")			-- Teleport the player to their starting temple
  end
end
