function onpulse_pc()
-- The prisoner will slowly die of poisoning unless the player can retrieve the antidote
-- (18281) and give it to the prisoner. He will then reward them with a key (18280) and
-- diary (12900). Attached to mob 18245

  me.hp = me.hp - 5			-- Extra damage to prevent prisoner "healing"
  if (not aff_flagged(me, AFF_POISON)) then
    action(me, "eat bread")
  end
  ch = me
end

function sound()
  if (number(0, 5) == 0) then
    social(me, "shiver")
  elseif (number(0, 5) == 0) then
    emote("gasps, 'Please....help me!'")
  elseif (number(0, 5) == 0) then
    social(me, "cough")
  elseif (number(0, 5) == 0) then
    say("I'm dying...please...help me!")
  elseif (number(0, 5) == 0) then
    emote("gasps, 'The antidote...I need the antidote...'")
  end
end

function ongive()
  local alias = ""

  for i = 1, getn(me.objs) do
    if (me.objs[i].vnum == 18281) then
      if (aff_flagged(me, AFF_POISON)) then
        extobj(me.objs[i])
        act("Tears of joy run down $s cheek, 'Thank you so much!' $m says.",
          TRUE, me, NIL, NIL, TO_ROOM)
        act("$n says, 'Here, take this..find the key to release me and I will offer you "
          .."what reward I can.'", TRUE, me, NIL, ch, TO_VICT)
        act("$n hands something to $N.", TRUE, me, NIL, ch, TO_NOTVICT)
        act("$n hands something to you.", TRUE, me, NIL, ch, TO_VICT)
        oload(ch, 18280, "char")			-- Load the key and diary
        oload(ch, 12900, "char")
        spell(me, NILL, SPELL_REMOVE_POISON, FALSE)
        break
      else
        say("I give you thanks, but I have no need for this anymore.")
        if (strfind(me.objs[i].alias, "%a%s")) then
          alias = strsub(me.objs[i].alias, 1, strfind(me.objs[i].alias, "%a%s"))
        else
          alias = me.objs[i].alias
        end
        action(me, "give "..alias.." "..ch.name)
      end
    elseif (me.objs[i].vnum == 12901) then
      act("$n hurriedly unlocks $s shackles.", TRUE, me, NIL, NIL, TO_ROOM)
      say("You saved my life! This is all I have..please take it.")
      act("$n gives $N some gold before running off south.", TRUE, me, NIL, ch, TO_NOTVICT)
      act("$n hands you some gold before running off south.", TRUE, me, NIL, ch, TO_VICT)
      extobj(me.objs[i])				-- Remove the key
      extchar(me)						-- Remove the prisoner
      ch.gold = ch.gold + 20000
      save_char(ch)
      break
    else
      say("Thank you, but I have no use for this.")
      if (strfind(me.objs[i].alias, "%a%s")) then
        alias = strsub(me.objs[i].alias, 1, strfind(me.objs[i].alias, "%a%s"))
      else
        alias = me.objs[i].alias
      end
      action(me, "give "..alias.." "..ch.name)
    end
  end
end
