function oncmd()
-- Allow players to withdraw and deposit gold as well as to check their current balance.

  local command = ""
  local temp = NIL
  local subcmd = NIL

  if (strfind(argument, "%a%s") ~= NIL) then
    command = strsub(argument, 1, strfind(argument, "%a%s"))
    temp = gsub(argument, command.." ", "")
    if (strfind(temp, "%d%s") ~= NIL) then
      subcmd = strsub(temp, 1, strfind(temp, "%d%s"))
    else
      subcmd = temp
    end
  else
    command = argument
  end

  if (command == "balance") then
    if (ch.bank > 0) then
      act("Your current balance is "..ch.bank.." coins.", FALSE, ch, NIL, NIL, TO_CHAR)
    else
      act("You currently have no money deposited.", FALSE, ch, NIL, NIL, TO_CHAR)
    end
    return (TRUE)
  elseif (command == "deposit") then
    if ((subcmd == NIL) or (subcmd == "")) then
      act("How much do you wish to deposit?", FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end
      
    local amount = tonumber(subcmd)
    if (amount == NIL) then
      act("What do you want to deposit??", FALSE, ch, NIL, NIL, TO_CHAR)
    elseif (amount <= 0) then
      act("How much do you wish to deposit?", FALSE, ch, NIL, NIL, TO_CHAR)
    elseif (ch.gold < amount) then
      act("You don't have that many coins!", FALSE, ch, NIL, NIL, TO_CHAR)
    else
      ch.gold = ch.gold - amount
      ch.bank = ch.bank + amount
      act("You deposit "..amount.." coins.", FALSE, ch, NIL, NIL, TO_CHAR)
      act("$n makes a bank transaction.", TRUE, ch, NIL, NIL, TO_ROOM)
    end
    return (TRUE)
  elseif (command == "withdraw") then
    if (subcmd == NIL) then
      act("How much do you wish to withdraw?", FALSE, ch, NIL, NIL, TO_CHAR)
      return (TRUE)
    end

    local amount = tonumber(subcmd)
    if (amount == NIL) then
      act("What do you wish to withdraw??", FALSE, ch, NIL, NIL, TO_CHAR)
    elseif (amount <= 0) then
      act("How much do you wish to withdraw?", FALSE, ch, NIL, NIL, TO_CHAR)
    elseif (ch.bank < amount) then
      act("You don't have that many coins deposited!", FALSE, ch, NIL, NIL, TO_CHAR)
    else
      ch.gold = ch.gold + amount
      ch.bank = ch.bank - amount
      act("You withdraw "..amount.." coins.", FALSE, ch, NIL, NIL, TO_CHAR)
      act("$n makes a bank transaction.", TRUE, ch, NIL, NIL, TO_ROOM)
    end
    return (TRUE)
  end
end

