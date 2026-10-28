function question(num, ques, answ)
-- The function displays the question and series of answers rather than repeat
-- it for each individual question.

  local buf = ""
  local buf2 = ""

  act(num..ques, FALSE, ch, NIL, NIL, TO_CHAR)
  for i = 1, 3 do
    if (i == 1) then
      buf = "   &ca.&n "
      buf2 = ","
    elseif (i == 2) then
      buf = "   &cb.&n "
      buf2 = ", or"
    else
      buf = "   &cc.&n "
      buf2 = "?"
    end

    act(buf..answ[i]..buf2, FALSE, ch, NIL, NIL, TO_CHAR)
  end
end

function question1()
-- The script is required at character creation and is called directly from
-- CON_CHARCREATE. The player will be asked 3 questions from 3 different
-- categories: attributes, background and lifestyle. The answers to these
-- questions will determine what attributes, alignment, gold and equipment
-- the player will start with.

  local att_ques =
    { "If you were given a choice of schooling, would you select:",
      "You have reached the age of the apprentice and must choose from:",
      "As a young adult, which profession would interest you the most:",
      "You have reached a crossroads in your life and must choose:",
      "You enter into a local competition to earn some gold, would you choose:",
      "If you were permitted to travel to a single city to observe its life, would you choose:",
      "Your village is celebrating the annual harvest but you are occupied, will you:" }
  local att_answ =
    { [1] = {"The priests of Kir Drax'in, strict disciplinarians and scholars in the art of war",
             "Elders in your local village, wise in the way of the ancients",
             "The monks of Mist Keep, naturalists and lovers of fine music" },
      [2] = {"Tholdur, a renowned knight",
             "Eldrich, the local sorcerer",
             "Le Tal, owner of the town's Alchemy shop" },
      [3] = {"A church guard, protector of the city streets",
             "A healer, dedicated to helping those in need",
             "A scribe, recording battles and traveller's stories for historical purposes" },
      [4] = {"The more difficult path, providing a higher risk yet significant gain",
             "The easier path, almost without risk yet yielding very little",
             "Allow life to continue as it may, waiting for the decision to become apparent" },
      [5] = {"The broadsword, using your strength to overpower your competition",
             "The magickal staff, relying on the ways of the ancients to help you",
             "The polearm, making use of its extended range to hit your foes" },
      [6] = {"Kir Drax’in, where the population is ruled with an iron fist under the church",
             "Mist Keep, where the monks live a secluded existence, surrounded in mystery",
             "Kir Morthis, where the ale flows as strongly as the nearby river" },
      [7] = {"Continue with your physical training, knowing it will provide for your future",
             "Find somewhere quiet to read the journal you acquired from a passing adventurer",
             "Join the local townsfolk in the celebration" }}
  local q_to_ask = number(1, getn(att_ques))	-- Which question to ask?

  question("1. ", att_ques[q_to_ask], att_answ[q_to_ask])
end


function question2()

  local back_ques =
    { "Whilst visiting the local tavern, you are accosted by a drunkard and challenged"
        .." to a fight. Will you:",
      "At the end of a heated battle, no one is sure who struck the fatal blow and should"
        .." claim the bounty. Will you:",
      "Your father taught you to treat all life as sacred, yet you stand before a man"
        .." dying of a poisonous bite. He pleads with you to end his life, do you:",
      "During a large battle, a fellow adventurer flees from the field and is set upon by"
        .." a large beast. Do you:",
      "You discover your mentor, one of the local merchants, tortures slaves for pleasure."
        .." Do you:",
      "Whilst travelling through the city on business, you are accosted by the local roughian."
        .." Will you:",
      "Conducting business with the local armorer, you discover she has given you too much gold"
        .." in return. Do you:",
      "During battle, you manage to disarm your foe who lies at your mercy. Will you:",
      "Your master grows greedy with power and orders you to attack the neighboring village."
        .." Knowing this is wrong, will you:",
      "You believe the weaponsmith has cheated you on your latest purchase. Will you:" }
  local back_answ =
    { [1] = {"Decline, knowing no good can come of it",
             "Accept, using your superior dexterity and strength to teach him a lesson",
             "Continue drinking your ale, hoping the poor fool will go away" },
      [2] = {"Allow another to claim the prize, although you are unsure of his story",
	       "Claim it was you, knowing in good conscience that it probably wasn’t",
             "Remain quiet, letting your companions decide the outcome" },
      [3] = {"Attempt to use your meagre healing skills and hope to save his life",
             "Do as he wishes, ending his life with a quick blow to the head",
             "Remain with him and comfort him, knowing he is beyond help" },
      [4] = {"Gather your comrades and run to his aid",
             "Allow him to fight alone and go to another’s aid, knowing he made the decision"
               .." to run",
             "Continue with your own fight, hoping he will survive" },
      [5] = {"Seek out the Captain of the Guards and report the incident",
             "Request to assist him, providing a solution to your morbid curiosity for pain",
             "Pretend you saw nothing, hoping the cries for help will be heard by others" },
      [6] = {"Reveal your hand-crafted broadsword, scaring him off",
             "Accept his challenge for a fight, knowing your skills will serve you well",
             "Humbly decline and offer to buy him an ale at the local tavern" },
      [7] = {"Return to the armory and hand over the excess gold",
             "Pocket the gold, it’s not like she needs it anyway",
             "Keep the gold for now, you can hand it back when you next meet" },
      [8] = {"Accept his plea for mercy and claim victory",
             "Make the fatal blow, ending his life",
             "Return his weapon to him, opting to let fate decide the outcome" },
      [9] = {"Draw your sword and kill him, knowing in your heart that it is required",
             "Do it anyway, having sworn to obey your lord at all times",
             "Refuse, leaving his service in disgrace but with honor" },
     [10] = {"Point the mistake out, relying on his good name to retrieve your gold",
             "Draw your weapon and demand your gold back in exchange for his life",
             "Leave the store, making a note never to return there" }}
  local q_to_ask = number(1, getn(back_ques))	-- Which question to ask?

  question("\n2. ", back_ques[q_to_ask], back_answ[q_to_ask])
end


function question3()

  local life_ques =
    { "How would you describe your upbringing:",
      "You take a moment to reflect on your youth. Which incident best describes it:",
      "With a dwindling supply of gold, you have the opportunity to enter a game of chance."
        .." Will you:",
      "Your parents were recently killed in a vicious attack on their farm. Will you:",
      "Your aging father can no longer care for you and you must decide where you shall"
        .." live and work. Will you:" }
  local life_answ =
    { [1] = {"In the dark alleys of Xixieqi, scrounging on whatever you could get your hands on",
             "Working mainly on your father’s farm, leading a comfortable life",
             "Sailing the seas with your uncle, a wealthy merchant who travelled often" },
      [2] = {"The death of your parents at an early age, having to take care of yourself and"
               .." younger sister",
             "The enjoyable time spent at the town armory, learning your current trade",
             "The gold statue donated by your family to honor your scholarly achievements" },
      [3] = {"Choose not to, keeping what gold you have but realising it won’t last very long,"
               .." leaving you a pauper",
             "Enter the game, risking only a portion of your gold and attempting to maintain"
               .." a modest lifestyle",
             "Enter the game risking everything you have, knowing the payout will leave wealthy"
               .." beyond your dreams" },
      [4] = {"Take care of yourself, earning what little you can by selling fruit at the local"
               .." markets",
             "Go and live with your aunt, a tailor in Kir Morthis",
             "Live with your uncle, the city’s wealthiest citizen" },
      [5] = {"Travel to Kir Oshi, where there is plenty of work but the pay is extremely poor and"
               .." prices high",
             "Travel to Mist Keep, where there is some work on the local farms and average"
               .." accomodation",
             "Travel to Kir Morthis, where there is almost no work to be found but where the pay is"
               .." extremely good and prices are cheap" }}
  local q_to_ask = number(1, getn(life_ques))	-- Which question to ask?

  question("\n3. ", life_ques[q_to_ask], life_answ[q_to_ask])
end
