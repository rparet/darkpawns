function oncmd()
-- Prevent player commands from executing until conversation is over

  if (me.level < LVL_IMMORT) then
    if (keep_conv_over == FALSE) then
      act("You really should be paying attention to the conversation!", TRUE, me, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    if (keep_ceremony == TRUE) then
      act("A summoning ceremony is underway, you should be concentrating!", TRUE, me, NIL, NIL, TO_CHAR)
      return (TRUE)
    end
  end
end