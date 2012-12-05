/* Populate database with DP2.2 imm zone */
/* zone */
insert into zones (number,name,top,lifespan,reset_mode)
 values	(12,'God Rooms',1299,50,0);

insert into reset_cmds (zone_id,position,command,if_flag,arg1,arg2,arg3)
 values	(1,1,'M',0,1200,2,1204),
	(1,2,'O',0,1207,2,1204);

/* rooms */
insert into rooms (zone_id,vnum,sector_type,name,description)
 values (1,1202,0,'The Ice Box',"   Here lie the frozen remains of mud players who just couldn't play with the other kids in the sandbox.\n"),
	(1,1204,0,'The Board Room Of The Immortal',"   This large room is well-decorated, and lit from above by a soft magical glow of no particular origin.  Scattered around the room are comforable chairs and piles of furs of various dimension, and in the very middle there is a huge oaken table that shimmers in an odd manner. A large picture window looks down over the whole world, showing the exploits of the many intrepid mortals.  A spiral staircase leads down to another part of this room.  There is a sign hovering in the awesomeness of this room.\n"),
	(1,1205,0,'The Lower Board Room of the Immortals',"   This room is a close twin to the main Immortal Board Room at the top of the spiral staircase.  Your mind and body cannot help but relax as you gaze around this comfortable room, and hope that you will be able to linger here a while.  Many green plants line a marble stairway that leads down to the Temple of Kir Drax'in.\n");

insert into exits (room_id,direction,general_description,keyword,door_info,door_key,to_room_vnum)
 values	(2,0,'You see the lower part of this room, and the normal board.\n','',0,-1,1205),
	(3,1,'You see the main Immortal Board Room at the top of the spiral staircase\n','',0,-1,1204);

insert into room_flags_rooms (room_id,room_flag_id)
 values	(1,3),
	(2,3),
	(2,10),
	(3,3),
	(3,10);

insert into extra_descriptions (room_id,keyword,description)
 values (2,'sign',"type help mail if you don't know how to use the dang mailman\n");

/* mobs */
insert into mobs (vnum,name,short_description,long_description,description,alignment,level,hitroll,damroll,num_dam_dice,size_dam_dice,armor,num_hp_dice,size_hp_dice,hp_bonus,gold,experience,position,default_position,sex)
 values (1200,'unfinished mob','an unfinished mob','An Unfinished mob stands here.','It looks, err, unfinished.\n',0,5,5,3,3,4,50,5,5,60,0,900,8,8,1);

insert into especs (mob_id,keyword,value)
 values (1,'Race','2');

insert into aff_flags_mobs (mob_id,aff_flag_id)
 values (1,8);

insert into mob_flags_mobs (mob_id,mob_flag_id)
 values (1,22);

/* objects */
insert into objects (type_flag_id,vnum,name,short_description,description,action_description,weight,cost,percent_chance_to_load)
 values (17,1207,'piece parchment','a piece of parchment','A piece of parchment has been rolled up and left here.','',1,5,100.0);

insert into object_values (object_id,value_zero,value_one,value_two,value_three)
 values (1,0,0,0,0);

insert into objects_wear_flags (object_id,wear_flag_id)
 values (1,2),
	(1,1);

insert into extra_flags_objects (object_id,extra_flag_id)
 values (1,2);

insert into affects (object_id,apply_flag_id,modifier)
 values (1,10,60);

/* shops */
insert into shops (vnum,buy_profit,sell_profit,mob_id,open_time_1,close_time_1,open_time_2,close_time_2)
 values	(1200,0.8,1.5,null,0,28,0,0);

insert into rooms_shops (shop_id,room_id)
 values	(1,2),
	(1,3);

insert into shop_flags_shops (shop_id,shop_flag_id)
 values	(1,1),
	(1,2);

insert into shops_notrade_flags (shop_id,notrade_flag_id)
 values	(1,3),
	(1,4);

insert into shop_messages (shop_id,keeper_no_item,player_no_item,keeper_no_cash,player_no_cash,keeper_no_buy,buy_success,sell_success)
 values (1,"%s Sorry, I don't stock that item.","%s You don't seem to have that.","%s You are too poor!","%s I can't afford that!","%s I don't trade in such items.","%s That'll be %d coins, thanks.","%s I'll give you %d coins for that.");

insert into accept_types (shop_id,type_flag_id,keywords)
 values (1,4,"wand"),
	(1,10,"plate");

insert into products (shop_id,object_id)
 values (1,1);
