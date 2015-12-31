/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996, 97, 98 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

/* $Id: spec_assign.c 1487 2008-05-22 01:36:10Z jravn $ */

#include "config.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern struct room_data *world;
extern int top_of_world;
extern int mini_mud;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  int rnum;
  if ((rnum = real_mobile(mob)) >= 0)
    mob_index[rnum].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
        mob);
    log("%s", buf);
  }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
        obj);
    log("%s", buf);
  }
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
        room);
    log("%s", buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  SPECIAL(postmaster);
  SPECIAL(cityguard);
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(guild_guard);
  SPECIAL(guild);
  SPECIAL(puff);
  SPECIAL(fido);
  SPECIAL(janitor);
  SPECIAL(mayor);
  SPECIAL(snake);
  SPECIAL(thief);
  SPECIAL(magic_user);
  SPECIAL(dracula);
  SPECIAL(brass_dragon);
  SPECIAL(mickey);
  SPECIAL(mallory);
  SPECIAL(cleric);
  SPECIAL(dragon_breath);
  SPECIAL(citizen);
  SPECIAL(conductor);
  SPECIAL(black_undead_knight);
  SPECIAL(red_undead_knight);
  SPECIAL(tipster);
  SPECIAL(rescuer);
  SPECIAL(cuchi);
  SPECIAL(normal_checker);
  SPECIAL(ninelives);
  SPECIAL(whirlpool);
  SPECIAL(stableboy);
  SPECIAL(pissedalchemist);
  SPECIAL(remorter);
  SPECIAL(tattoo1);
  SPECIAL(tattoo2);
  SPECIAL(tattoo3);
  SPECIAL(tattoo4);
  SPECIAL(identifier);
  SPECIAL(eviltrade);
  SPECIAL(bono);
  SPECIAL(clerk);
  SPECIAL(little_boy);
  SPECIAL(ira);
  SPECIAL(jailguard);
  SPECIAL(outofjailguard);
  SPECIAL(take_to_jail);
  SPECIAL(medusa);
  SPECIAL(eq_thief);
  SPECIAL(breed_killer);
  SPECIAL(bat);
  SPECIAL(no_move_east);
  SPECIAL(no_move_west);
  SPECIAL(castle_guard_east);
  SPECIAL(mindflayer);
  SPECIAL(backstabber);
  SPECIAL(teleporter);
  SPECIAL(fighter);
  SPECIAL(no_move_north);
  SPECIAL(no_move_south);
  SPECIAL(never_die);
  SPECIAL(butler);
  SPECIAL(mortician);
  SPECIAL(brain_eater);
  SPECIAL(teleport_victim);
  SPECIAL(con_seller);
  SPECIAL(no_move_down);
  SPECIAL(castle_guard_down);
  SPECIAL(castle_guard_up);
  SPECIAL(castle_guard_north);
  SPECIAL(beholder);
  SPECIAL(no_get);
  SPECIAL(troll);
  SPECIAL(recharger);
  SPECIAL(zen_master);
  SPECIAL(quan_lo);
  SPECIAL(werewolf);
  SPECIAL(prostitute);
  SPECIAL(roach);
  SPECIAL(conjured);
  SPECIAL(paladin);
  SPECIAL(hisc);
  SPECIAL(recruiter);
  SPECIAL(chosen_guard);
  SPECIAL(wall_guard_ns);
  SPECIAL(key_seller);

  /* Matt's spec procs - mobs (spec_procs3.c) */
  SPECIAL(elements_minion);
  SPECIAL(elements_guardian);
  SPECIAL(forest_aurumvorax);
  SPECIAL(forest_mymic);
  SPECIAL(forest_gazer);
  SPECIAL(forest_drake);
  SPECIAL(forest_bear_cubs);
  SPECIAL(forest_anhkheg);

  ASSIGNMOB(1, puff);
  ASSIGNMOB(4, remorter);
  ASSIGNMOB(8, recharger);
  ASSIGNMOB(23, roach);
  ASSIGNMOB(81, conjured);
  ASSIGNMOB(82, conjured);
  ASSIGNMOB(83, conjured);
  ASSIGNMOB(84, conjured);
  ASSIGNMOB(85, conjured);
  ASSIGNMOB(86, conjured);


  /* Elemental Temple */
  ASSIGNMOB(1313, elements_minion);
  ASSIGNMOB(1314, elements_guardian);
  ASSIGNMOB(1305, cleric);
  ASSIGNMOB(1307, magic_user);
  ASSIGNMOB(1315, magic_user);

  /* Orc Burrows */
  ASSIGNMOB(2106, no_move_west);
  ASSIGNMOB(2108, magic_user);


  /* Amber/Arden */
  ASSIGNMOB(2716, magic_user);
  ASSIGNMOB(2720, magic_user);
  ASSIGNMOB(2732, magic_user);
  ASSIGNMOB(2733, magic_user);
  ASSIGNMOB(2734, magic_user);
  ASSIGNMOB(2747, cityguard);
  ASSIGNMOB(2766, tattoo4);
  ASSIGNMOB(3010, postmaster);

  /* Ironwood Forest  */

  ASSIGNMOB(3119, thief); /* cutthroat */
  ASSIGNMOB(3103, magic_user);
  ASSIGNMOB(3113, magic_user);
  ASSIGNMOB(3118, magic_user);

  /* The Lakeshore Tavern */
  ASSIGNMOB(4101, magic_user);
  ASSIGNMOB(4102, cleric);

  /* Bhyroga Valley */
  ASSIGNMOB(4202, magic_user);
  ASSIGNMOB(4209, dragon_breath);

  /* Sulfur Fur Mountains */
  ASSIGNMOB(4704, magic_user); /* ice mage */
  ASSIGNMOB(4705, dragon_breath); /* khal'mast */

  /* Xixieqi */
  ASSIGNMOB(4813,guild);
  ASSIGNMOB(4821,guild);
  ASSIGNMOB(4825,guild);

  ASSIGNMOB(4914, fighter);
  ASSIGNMOB(4916, magic_user);
  ASSIGNMOB(4919, magic_user);

  /* Xixieqi highlands */

  ASSIGNMOB(5200, fighter);

   /* Taltos forest */
  ASSIGNMOB(5503, magic_user);
  ASSIGNMOB(5507, magic_user);
  ASSIGNMOB(5510, werewolf);


  /* Corbiel */
  ASSIGNMOB(7023, magic_user);

  /*Random Load World*/
  ASSIGNMOB(71, paladin); /* death dealer */
  ASSIGNMOB(7900, breed_killer);
  ASSIGNMOB(7901, fighter);
  ASSIGNMOB(7902, fighter);
  ASSIGNMOB(7903, dracula);
  ASSIGNMOB(7909, rescuer);
  ASSIGNMOB(7910, breed_killer);
  ASSIGNMOB(7915, paladin);
  ASSIGNMOB(7979, eq_thief);
  ASSIGNMOB(7969, magic_user);
  ASSIGNMOB(7970, cleric);  /* unicorn */

  /* Kir Drax'in */
  ASSIGNMOB(8014, guild_guard);
  ASSIGNMOB(8017, guild_guard);
  ASSIGNMOB(8016, guild_guard);
  ASSIGNMOB(8018, guild_guard);
  ASSIGNMOB(8019, guild_guard);
  ASSIGNMOB(8025, guild_guard);

  ASSIGNMOB(8012, guild);
  ASSIGNMOB(8013, guild);
  ASSIGNMOB(8014, guild);
  ASSIGNMOB(8015, guild);
  ASSIGNMOB(8022, stableboy);
  ASSIGNMOB(8023, prostitute);

  ASSIGNMOB(8027, take_to_jail);
  ASSIGNMOB(8059, take_to_jail);
  ASSIGNMOB(8060, wall_guard_ns);

  ASSIGNMOB(8001, take_to_jail);
  ASSIGNMOB(8002, take_to_jail);
  ASSIGNMOB(8020, take_to_jail);
  ASSIGNMOB(8061, janitor);
  ASSIGNMOB(8062, citizen);
  ASSIGNMOB(8063, fido);
  ASSIGNMOB(8081, magic_user);
  ASSIGNMOB(8086, tattoo1);
  ASSIGNMOB(8087, identifier);
  ASSIGNMOB(8088, jailguard);
  ASSIGNMOB(8089, outofjailguard);
  ASSIGNMOB(8092, butler);
  ASSIGNMOB(8095, mortician);
  ASSIGNMOB(8096, chosen_guard); /* old guard */

  ASSIGNMOB(8024, guild);  /* muntara */
  ASSIGNMOB(8026, guild);  /* ninja guildmaster */
  ASSIGNMOB(8083, guild_guard); /* psionic guild guard */

  /* road/forest*/
  ASSIGNMOB(9151, backstabber);

  /* Cemetary */

  ASSIGNMOB(9903, magic_user);
  ASSIGNMOB(9905, magic_user); /* crowley */
  ASSIGNMOB(9911, magic_user);

  /* Stadium  */
  ASSIGNMOB(10029, troll);

   /* Desert */
  ASSIGNMOB(11023, cleric);
  ASSIGNMOB(11024, magic_user);
  ASSIGNMOB(11029, magic_user);
  ASSIGNMOB(11030, magic_user);
  ASSIGNMOB(11000, dragon_breath);
  ASSIGNMOB(11001, dragon_breath);
  ASSIGNMOB(11002, dragon_breath);
  ASSIGNMOB(11005, magic_user);
  ASSIGNMOB(11006, magic_user);
  ASSIGNMOB(11007, magic_user);
  ASSIGNMOB(11008, magic_user);
  ASSIGNMOB(11011, beholder);
  ASSIGNMOB(11016, magic_user);
  ASSIGNMOB(11023, cleric);
  ASSIGNMOB(11024, cleric);
  ASSIGNMOB(11029, magic_user);
  ASSIGNMOB(11030, magic_user);
  ASSIGNMOB(11038, cleric);
  ASSIGNMOB(11039, cleric);

  /* Swamp  */
  ASSIGNMOB(12200, whirlpool);

  /*crystal temple and mini areas*/
  ASSIGNMOB(11706, magic_user);

  /* Haven */
  ASSIGNMOB(12111, fighter); /* elven pirate */
  ASSIGNMOB(12115, fido); /*seagull*/
  ASSIGNMOB(12118, eq_thief); /* kender */
  ASSIGNMOB(12127, thief);

  /* Temple */
  ASSIGNMOB(14100, troll);
  ASSIGNMOB(14101, medusa);
  ASSIGNMOB(14102, medusa);
  ASSIGNMOB(14103, snake);  /* large snake */
  ASSIGNMOB(14127, snake);  /* snake */

  /* Alaozar */
  ASSIGNMOB(14202, cleric);
  ASSIGNMOB(14206, cleric);
  ASSIGNMOB(14220, cleric);
  ASSIGNMOB(14219, magic_user);
  ASSIGNMOB(14221, magic_user);
  ASSIGNMOB(14225, eq_thief); /*Oz*/

  /* Ogre stronghold */
  ASSIGNMOB(14311, troll);
  ASSIGNMOB(14312, troll);
  ASSIGNMOB(14314, magic_user);  /* ogre shaman */
  ASSIGNMOB(14306, magic_user);  /* guardian */
  ASSIGNMOB(14309, cleric);  /* lizard shaman */

  /* The Grey keep */
  ASSIGNMOB(14401, never_die);
  ASSIGNMOB(14405, teleport_victim);
  ASSIGNMOB(14406, magic_user);
  ASSIGNMOB(14407, fighter);
  ASSIGNMOB(14410, no_move_east); /* silk */
  ASSIGNMOB(14411, teleporter); /* master */
/*   ASSIGNMOB(14412, hisc); */
  ASSIGNMOB(14414, mindflayer);
  ASSIGNMOB(14415, snake);
  ASSIGNMOB(14416, no_get);
  ASSIGNMOB(14420, brain_eater);
  ASSIGNMOB(14421, no_move_west);
  ASSIGNMOB(14430, no_get);
  ASSIGNMOB(14432, brain_eater);
  ASSIGNMOB(14435, magic_user);


  /* the plains */
  ASSIGNMOB(15108, fido);


  /* Kir Drax'in Guard Training Centre */
  ASSIGNMOB(16300, recruiter);
  ASSIGNMOB(16308, no_move_south);

  /* Kir-Oshi */
  ASSIGNMOB(18202, citizen);
  ASSIGNMOB(18203, fido);
  ASSIGNMOB(18213, tattoo2);
  ASSIGNMOB(18218, thief);
  ASSIGNMOB(18219, zen_master);
  ASSIGNMOB(18228, clerk);
  ASSIGNMOB(18215, cityguard);

  /* The Checker Board, and O's Stuff */
  ASSIGNMOB(18301, normal_checker);
  ASSIGNMOB(18302, normal_checker);
  ASSIGNMOB(18303, normal_checker);
  ASSIGNMOB(18304, normal_checker);
  ASSIGNMOB(18306, cuchi);


  /* Lighthouse */
  ASSIGNMOB(15804, magic_user);
  ASSIGNMOB(15807, magic_user);
  ASSIGNMOB(15808, rescuer);
  ASSIGNMOB(15814, pissedalchemist);


  /*Mines*/
  ASSIGNMOB(12848, magic_user);/*demon knight*/
  ASSIGNMOB(12850, fighter);   /*indy*/
  ASSIGNMOB(12876, no_move_east); /* guardian */
  ASSIGNMOB(12877, cleric); /* soloman */

  /*temple*/
  ASSIGNMOB(14110, dracula); /*Lothar*/

  /*Abandoned city*/
  ASSIGNMOB(18601, magic_user);
  ASSIGNMOB(18603, magic_user);
  ASSIGNMOB(18604, magic_user);

  /* Ghost ship */
  ASSIGNMOB(19119, never_die);

  /* Fire Pagoda - Shaolin Temple */
  ASSIGNMOB(19412, magic_user); /* Fire Wizard */
  ASSIGNMOB(19405, quan_lo);

  /* Darius Elven Camp */
  ASSIGNMOB(19510, castle_guard_north);

  /* Player Castles */
  ASSIGNMOB(19601, castle_guard_north);
  ASSIGNMOB(19602, castle_guard_north);
  ASSIGNMOB(19610, key_seller);
  ASSIGNMOB(19626, castle_guard_down);
  ASSIGNMOB(19627, castle_guard_down);
  ASSIGNMOB(19640, castle_guard_down);
  ASSIGNMOB(19641, castle_guard_down);
  ASSIGNMOB(19650, castle_guard_up);
  ASSIGNMOB(19651, castle_guard_up);
  ASSIGNMOB(19690, castle_guard_north);
  ASSIGNMOB(19691, castle_guard_north);
  ASSIGNMOB(19675, castle_guard_north);
  ASSIGNMOB(19676, castle_guard_north);

  /* */
  ASSIGNMOB(19900, troll);
  ASSIGNMOB(19901, troll);

  /* DMT */
  ASSIGNMOB(20002, fighter);
  ASSIGNMOB(20003, magic_user);
  ASSIGNMOB(20004, thief);
  ASSIGNMOB(20005, cleric);
  ASSIGNMOB(20008, magic_user);
  ASSIGNMOB(20009, magic_user);
  ASSIGNMOB(20010, magic_user);
  ASSIGNMOB(20011, fighter);
  ASSIGNMOB(20014, magic_user);
  ASSIGNMOB(20018, cleric);
  ASSIGNMOB(20019, fighter);
  ASSIGNMOB(20020, fighter);
  ASSIGNMOB(20023, magic_user);
  ASSIGNMOB(20025, magic_user);
  ASSIGNMOB(20026, magic_user);
  ASSIGNMOB(20027, dragon_breath);
  ASSIGNMOB(20029, cleric);
  ASSIGNMOB(20030, magic_user);
  ASSIGNMOB(20035, cleric);
  ASSIGNMOB(20036, fighter);
  ASSIGNMOB(20041, cleric);
  ASSIGNMOB(20042, fighter);

  /* City of Alaozar */
  ASSIGNMOB(21200, cityguard);
  ASSIGNMOB(21201, cityguard);
  ASSIGNMOB(21202, cityguard);
  ASSIGNMOB(21203, cityguard);
  ASSIGNMOB(21214, guild);
  ASSIGNMOB(21215, guild);
  ASSIGNMOB(21216, guild);
  ASSIGNMOB(21217, guild);
  ASSIGNMOB(21221, cleric); /* high priest */
  ASSIGNMOB(21225, postmaster);
  ASSIGNMOB(21227, cityguard);
  ASSIGNMOB(21228, cityguard);
  ASSIGNMOB(21229, janitor);
  ASSIGNMOB(21242, thief);
  ASSIGNMOB(21244, tattoo3);
  ASSIGNMOB(21246, con_seller);


}



/* assign special procedures to objects */
void assign_objects(void)
{
  SPECIAL(bank);
  SPECIAL(couch);
  SPECIAL(gen_board);
  SPECIAL(moon_gate);
  SPECIAL(horn);
  SPECIAL(black_horn);
  SPECIAL(field_object);
  SPECIAL(turn_undead);
  SPECIAL(mirror);

  ASSIGNOBJ(50, field_object);
  ASSIGNOBJ(52, field_object);

  ASSIGNOBJ(4001, moon_gate);   /* blue portal  */
  ASSIGNOBJ(4002, moon_gate);   /* red portal   */


  ASSIGNOBJ(8034, bank);       /* KD atm  */
  ASSIGNOBJ(8064, gen_board);  /* customs house */
  ASSIGNOBJ(8065, gen_board);  /* chosen */
  ASSIGNOBJ(8096, gen_board);  /* social board   */
  ASSIGNOBJ(8097, gen_board);  /* freeze board   */
  ASSIGNOBJ(8098, gen_board);  /* immortal board */
  ASSIGNOBJ(8099, gen_board);  /* mortal board   */
  ASSIGNOBJ(19652, gen_board); /* clan board */
  ASSIGNOBJ(19601, gen_board); /* clan board */
  ASSIGNOBJ(19604, moon_gate);
  ASSIGNOBJ(19605, moon_gate);
  ASSIGNOBJ(19606, moon_gate);
  ASSIGNOBJ(19607, moon_gate);
  ASSIGNOBJ(19608, moon_gate);
  ASSIGNOBJ(19609, moon_gate);
  ASSIGNOBJ(19610, moon_gate);
  ASSIGNOBJ(19611, moon_gate);

  ASSIGNOBJ(19627, gen_board); /* clan board */
  ASSIGNOBJ(19640, gen_board);
  ASSIGNOBJ(19666, gen_board);
  ASSIGNOBJ(19677, gen_board);

  ASSIGNOBJ(14415, horn);


  ASSIGNOBJ(18224, bank);      /* kir-oshi atm */

}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  extern int dts_are_dumps;
  int i;

  SPECIAL(dump);
  SPECIAL(pet_shops);
  SPECIAL(pray_for_items);
  SPECIAL(elemental_room);
  SPECIAL(enter_circle);
  SPECIAL(elevator);
  SPECIAL(start_room);
  SPECIAL(newbie_zone_entrance);
  SPECIAL(oro_quarters_room);
  SPECIAL(oro_study_room);
  SPECIAL(suck_in);
  SPECIAL(assassin);
  SPECIAL(jail);
  SPECIAL(portal_room);
  SPECIAL(carrion);
  SPECIAL(bat_room);
  SPECIAL(alien_elevator);
  SPECIAL(portal_to_temple);
  SPECIAL(itoh);

  /* Matt's spec procs - rooms (spec_procs3.c) */
  SPECIAL(elements_master_column);
  SPECIAL(elements_platforms);
  SPECIAL(elements_load_cylinders);
  SPECIAL(elements_galeru_column);
  SPECIAL(elements_galeru_alive);
  SPECIAL(zigg_recesses);
  SPECIAL(zigg_darts);
  SPECIAL(zigg_dt);
  SPECIAL(zigg_portal);
  SPECIAL(fly_exit_up);

  ASSIGNROOM(8008, pray_for_items);
  ASSIGNROOM(8099, start_room);
  ASSIGNROOM(16300, newbie_zone_entrance);
  ASSIGNROOM(8114, assassin);
  ASSIGNROOM(8118, jail);
  ASSIGNROOM(8085, dump);

  ASSIGNROOM(14305, carrion);
  ASSIGNROOM(18399, oro_study_room);
  ASSIGNROOM(18397, oro_quarters_room);
  ASSIGNROOM(19658, portal_to_temple);
  ASSIGNROOM(20073, suck_in);
  ASSIGNROOM(21223, dump);
  ASSIGNROOM(21235, pet_shops);


  /* Elemental Temple */
  ASSIGNROOM(1315, elements_master_column);
  ASSIGNROOM(1326, elements_platforms);
  ASSIGNROOM(1337, elements_platforms);
  ASSIGNROOM(1348, elements_platforms);
  ASSIGNROOM(1359, elements_platforms);
  ASSIGNROOM(1360, elements_load_cylinders);
  ASSIGNROOM(1364, elements_load_cylinders);
  ASSIGNROOM(1380, elements_load_cylinders);
  ASSIGNROOM(1384, elements_load_cylinders);
  ASSIGNROOM(1372, elements_galeru_column);
  ASSIGNROOM(1394, elements_galeru_alive);

  /* Multi-zone Procs */
  ASSIGNROOM(1389, fly_exit_up);


  if (dts_are_dumps)
    for (i = 0; i < top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_DEATH))
    world[i].func = dump;
}
