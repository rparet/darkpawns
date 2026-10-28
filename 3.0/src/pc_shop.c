/* ************************************************************************
*  File: pc_shop.c                                      Part of CircleMUD *
*  Usage: various player-owned shop functions                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
  All parts of this code not covered by the copyright by the Trustees of
  the Johns Hopkins University are Copyright (C) 1996-1999 by the
  Dark Pawns Coding Team.

  This includes all original code done for Dark Pawns MUD by other authors.
  All code is the intellectual property of the author, and is used here
  by permission.

  No original code may be duplicated, reused, or executed without the
  written permission of the author. All rights reserved.

  See dp-team.txt or "help coding" online for members of the Dark Pawns
  Coding Team.
*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "interpreter.h"
#include "comm.h"
#include "shop.h"
#include "olc.h"
#include "screen.h"
#include "handler.h"
#include "constants.h"

/* External Variables */
extern int top_shop;
extern struct shop_data *shop_index;
extern char *hometowns[];
extern char *nrm, *grn, *cyn, *yel;
extern struct shop_cost_data shop_cost[];

/* External Functions */
extern int real_shop(int vnum);
void sedit_add_to_type_list(struct shop_buy_data **list, struct shop_buy_data *new);
SPECIAL(shop_keeper);
extern struct time_info_data time_info;
int real_zone(int vznum);
int is_open(struct char_data * keeper, int shop_nr, int msg);
void sql_write_notes(char *name, char *arg, struct char_data *vict);
void sql_save_shop(int shop_nr);

/* Display shop-cost data based on the city the player is in */
void shop_display_costs(struct char_data *ch, int homet)
{
  int i;

  sprintf(buf, "%s purchase and associated costs for a store include:\r\n", hometowns[homet]);
  sprintf(END_OF(buf), "  Purchase price:       %s%ld%s coins\r\n",
          CCGRN(ch, C_NRM), shop_cost[homet].cost, CCNRM(ch, C_NRM));
  sprintf(END_OF(buf), "  Annual tax rate:      %s%.0f%s percent\r\n",
          CCGRN(ch, C_NRM), shop_cost[homet].tax * 100, CCNRM(ch, C_NRM));
  sprintf(END_OF(buf), "  Staff wages per hour: %s%d%s coins\r\n",
          CCGRN(ch, C_NRM), shop_cost[homet].hire, CCNRM(ch, C_NRM));
  sprintf(END_OF(buf), "\r\nAvailable stores for purchase include:\r\n");
  for (i = 0; *shop_types[i] != '\n'; i++)
    sprintf(END_OF(buf), "  %d) %-20s\r\n", i+1, shop_types[i]);
  sprintf(END_OF(buf), "\r\nCurrently, there are %s%d%s stores available for purchase.\r\n",
          CCGRN(ch, C_NRM), shop_cost[homet].for_sale, CCNRM(ch, C_NRM));

  send_to_char(buf, ch);
}

/* Save the shop control variables to the SQL database */
void shop_control_save(void)
{
  int shop_nr;
  void sql_save_shop(int shop_nr);

  for (shop_nr = 0; shop_nr < top_shop; shop_nr++)
    sql_save_shop(shop_nr);
}

/* Cleaning the shop up of all keeper objs and the shop deed */
int pcshop_clean(struct char_data *keeper)
{
  struct obj_data *obj, *next_obj;
  int gold = 0;

  for (obj = keeper->carrying; obj; obj = next_obj) {
    next_obj = obj->next_content;
    gold += GET_OBJ_COST(obj);
    extract_obj(obj);
  }

  for (obj = world[IN_ROOM(keeper)].contents; obj; obj = obj->next_content)
    if (GET_OBJ_VNUM(obj) == 1223) {
      extract_obj(obj);
      break;
    }

  gold += GET_GOLD(keeper);
  return (gold);
}

/* Create a player-owned store */
void create_shop(struct shop_data *shop, long copy_room, int type)
{
  long in_room = real_room(*(shop->in_room));
  struct shop_buy_data new_entry;

  /* Copy the correct room over */
  if (!ZONE_FLAGGED(world[real_room(copy_room)].zone, ZONE_UNLOAD))
    reload_room_descs(world[real_room(copy_room)].zone);
  if (!ZONE_FLAGGED(world[in_room].zone, ZONE_UNLOAD))
    reload_room_descs(world[in_room].zone);

  /* Get rid of the old stuff */
  if (world[in_room].description)
    FREE(world[in_room].description);
  if (world[in_room].name)
    FREE(world[in_room].name);

  world[in_room].description = str_dup(world[real_room(copy_room)].description);
  world[in_room].name = str_dup(world[real_room(copy_room)].name);
  world[in_room].room_flags = world[real_room(copy_room)].room_flags;
  world[in_room].sector_type = world[real_room(copy_room)].sector_type;
  shop->shop_type = type;
  shop->copy_room = copy_room;

  /* Create new buying list for the keeper */
  switch(type) {
    case 0:   /* Armory */
      BUY_TYPE(new_entry) = ITEM_WORN;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_ARMOR;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    case 1: /* Weaponsmith */
      BUY_TYPE(new_entry) = ITEM_WEAPON;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    case 2: /* Jeweler */
      BUY_TYPE(new_entry) = ITEM_TREASURE;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    case 3: /* Provisioner */
      BUY_TYPE(new_entry) = ITEM_FOOD;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    case 4: /* General Store */
      BUY_TYPE(new_entry) = ITEM_OTHER;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_CONTAINER;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_DRINKCON;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_LIGHT;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    case 5: /* Apothecary */
      BUY_TYPE(new_entry) = ITEM_OTHER;
      BUY_WORD(new_entry) = str_dup("reagent");
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_POTION;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    case 6: /* Magical Store */
      BUY_TYPE(new_entry) = ITEM_SCROLL;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_STAFF;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      BUY_TYPE(new_entry) = ITEM_WAND;
      BUY_WORD(new_entry) = NULL;
      sedit_add_to_type_list(&shop->type, &new_entry);

      break;
    default:
      sprintf(buf, "SYSERR: Invalid shop type passed to player_buy_shop.");
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      break;
  }

  sql_save_shop(real_shop(world[in_room].number));
}

/* Delete a player-owned store */
void delete_shop(struct char_data *keeper, struct shop_data *shop)
{
  long in_room = real_room(*(shop->in_room));
  long copy_room = real_room(1230);
  int shop_nr = real_shop(world[in_room].number);
  extern void sql_remove_shop(struct char_data *keeper, int shop_nr);

  /* Copy the empty shop back over */
  if (!ZONE_FLAGGED(world[copy_room].zone, ZONE_UNLOAD))
    reload_room_descs(world[copy_room].zone);
  if (!ZONE_FLAGGED(world[in_room].zone, ZONE_UNLOAD))
    reload_room_descs(world[in_room].zone);

  /* Get rid of the old stuff */
  if (world[in_room].description)
    FREE(world[in_room].description);
  if (world[in_room].name)
    FREE(world[in_room].name);

  world[in_room].description = str_dup(world[copy_room].description);
  world[in_room].name = str_dup(world[copy_room].name);
  world[in_room].room_flags = world[copy_room].room_flags;
  world[in_room].sector_type = world[copy_room].sector_type;

  /* Remove the previously set shop variables */
  FREE(shop->owner);
  shop->owner = str_dup("None");
  SHOP_KEEPER(shop_nr) = NOBODY;
  SHOP_BUYPROFIT(shop_nr) = 1.0;
  SHOP_SELLPROFIT(shop_nr) = 1.0;
  SHOP_ITEMS(shop_nr) = 0;

  sql_remove_shop(keeper, shop_nr);
}

/* Player has decided to buy a shop, sort out the room and shop */
void player_buy_shop(struct char_data *ch, int homet, int type)
{
  struct char_data *keeper;
  struct shop_data *shop;
  int shop_num;
  long mobile;
  long copy_vnums[NUM_SHOP_TYPES] = {1220, 1221, 1222, 1223, 1224, 1225, 1226 };

  shop_cost[homet].for_sale--;                                /* Decrement the shops available */
  shop_num = real_shop(world[ch->in_room].number);            /* Find the corresponding shop */
  shop = shop_index + shop_num;

  /* Basic error checking */
  if (shop_num == NOWHERE) {    /* Shop does not exist! */
    sprintf(buf, "SYSERR: Missing shop in room number #%ld.", GET_ROOM_VNUM(IN_ROOM(ch)));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    send_to_char("There was a problem with the purchase of your store. Inform a staff member.\r\n", ch);
    return;
  }

  FREE(shop->owner);
  shop->owner = str_dup(GET_NAME(ch));

  /* Set up the proper shop keeper and give him a portion of gold */
  mobile = shop_keeps[homet] + shop_cost[homet].for_sale;
  if (real_mobile(mobile) != NOBODY) {
    mob_index[real_mobile(mobile)].func = shop_keeper;
    keeper = read_mobile(mobile, VIRTUAL, NULL);
    char_to_room(keeper, ch->in_room);
    GET_GOLD(keeper) = 50000;
    GET_GOLD(ch) -= 50000;
    SHOP_KEEPER(shop_num) = real_mobile(mobile);

    create_shop(shop, copy_vnums[type], type);
  } else {
    sprintf(buf, "SYSERR: Unable to locate a shop keeper for a new store.");
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    send_to_char("There was a problem with the purchase of your store. Inform a staff member.\r\n", ch);
  }
}


/* Player has decided to sell a shop - restore the room and shop to the original */
void player_sell_shop(struct char_data *ch)
{
  struct char_data *keeper;
  struct shop_data *shop;
  int shop_num, gold;

  shop_cost[ch->player.hometown].for_sale++;                  /* Decrement the shops available */
  shop_num = real_shop(world[ch->in_room].number);            /* Find the corresponding shop */
  shop = shop_index + shop_num;

  /* Basic error checking */
  if (shop_num == NOWHERE) {    /* Shop does not exist! */
    sprintf(buf, "SYSERR: Missing shop in room number #%ld.", GET_ROOM_VNUM(IN_ROOM(ch)));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    send_to_char("There was a problem with the sale of your store. Inform a staff member.\r\n", ch);
    return;
  }

  if (!(keeper = get_char_num(SHOP_KEEPER(shop_num)))) {
    sprintf(buf, "SYSERR: Unable to locate shop keeper #%ld in player_sell_shop.",
      SHOP_KEEPER(shop_num));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    send_to_char("There was a problem with the sale of your store. Inform a staff member.\r\n", ch);
  } else {
    delete_shop(keeper, shop);
    gold = pcshop_clean(keeper);
    extract_char(keeper);
    if (gold > 0)
      GET_GOLD(ch) += gold;
  }
}


/* Handles the settings assigned by the player */
SPECIAL(player_shop)
{
  int i, shop_num, gold;
  bool update = FALSE;
  struct shop_data *shop;
  struct char_data *keeper = get_char_room_vis(ch, "shop");
  char *options[] = {
    "open",     /* 0 */
    "close",
    "buy",
    "sell",
    "withdraw",
    "limit",
    "\n"
  };

  if (!(CMD_IS("shop")))
    return(FALSE);

  shop_num = real_shop(world[ch->in_room].number);
  shop = shop_index + shop_num;

  /* Is this my store? */
  if (strcmp(shop->owner, GET_NAME(ch)) && (GET_LEVEL(ch) < LVL_IMMORT))
    return (FALSE);

  skip_spaces(&argument);
  two_arguments(argument, buf1, buf2);

  if (!*buf1) {
    strcpy(buf, "Current store settings:\r\n");
    sprintf(END_OF(buf), "  Opens at: &g%2d&n\r\n  Closes at: &g%2d&n\r\n"
                         "  Buy rate: &g%5.2f&n\r\n  Sell rate: &g%1.2f&n\r\n"
                         "  Inventory limit:  &g%-3d&n\r\n"
                         "  Shop keeper gold: &c%d&n\r\n\r\n",
                         SHOP_OPEN1(shop_num), SHOP_CLOSE1(shop_num),
                         SHOP_BUYPROFIT(shop_num), SHOP_SELLPROFIT(shop_num),
                         SHOP_ITEMS(shop_num),
                         GET_GOLD(keeper));
    sprintf(END_OF(buf), "Shop options:\r\n");
    for (i = 0; *options[i] != '\n'; i++)
      sprintf(END_OF(buf), "  %-6s", options[i]);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return (TRUE);
  }

  /* Parse the argument */
  for (i = 0; *options[i] != '\n'; i++)
    if (!strncmp(buf1, options[i], strlen(argument)))
      break;

  switch(i) {
    case 0:
      if (!is_number(buf2) || (!*buf2))
        send_to_char("Usage: shop open <time>\r\n", ch);
      else {
        SHOP_OPEN1(shop_num) = LIMIT(atoi(buf2), 0, 24);
        update = TRUE;
      }
      break;
    case 1:
      if (!is_number(buf2) || (!*buf2))
        send_to_char("Usage: shop close <time>\r\n", ch);
      else {
        SHOP_CLOSE1(shop_num) = LIMIT(atoi(buf2), 0, 24);
        update = TRUE;
      }
      break;
    case 2:
      if (!*buf2)
        send_to_char("Usage: shop buy <rate>\r\n", ch);
      else {
        SHOP_BUYPROFIT(shop_num) = LIMIT(atoi(buf2), 1.0, 10.0);
        update = TRUE;
      }
      break;
    case 3:
      if (!*buf2)
        send_to_char("Usage: shop sell <rate>\r\n", ch);
      else {
        SHOP_SELLPROFIT(shop_num) = LIMIT(atoi(buf2), 1.0, 10.0);
        update = TRUE;
      }
      break;
    case 4:
      if (!is_number(buf2) || (!*buf2))
        send_to_char("Usage: shop withdraw <amount>\r\n", ch);
      else {
        gold = atoi(buf2);
        if (!gold)
          send_to_char("You want to withdraw 0 coins??\r\n", ch);
        else if ((GET_GOLD(keeper) - gold) < 20000)
          send_to_char("Your shop keeper must keep at least 20000 gold coins.\r\n", ch);
        else {
         GET_GOLD(keeper) -= gold;
         GET_GOLD(ch) += gold;
         sprintf(buf, "You withdraw %d coins from the shopkeeper.\r\n", gold);
         send_to_char(buf, ch);
         sql_save_shop(shop_num);
        }
      }
      break;
    case 5:
      if (!is_number(buf2) || (!*buf2))
        send_to_char("Usage: shop limit <amount>\r\n", ch);
      else {
        SHOP_ITEMS(shop_num) = LIMIT(atoi(buf2), 0, 300);
        update = TRUE;
      }
      break;
    default:
      sprintf(buf, "Shop options:\r\n");
      for (i = 0; *options[i] != '\n'; i++)
        sprintf(END_OF(buf), "  %-6s", options[i]);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
  }

  /* Update the relevant shop files */
  if (update) {
    send_to_char("Your store has been updated.\r\n", ch);
    sql_save_shop(shop_num);
  }

  return (TRUE);
}

/* Assigns an empty store to the player upon use */
SPECIAL(deed_unassigned)
{
  struct obj_data *deed = (struct obj_data *)me;
  struct obj_data *store_deed;
  struct shop_data *shop;
  struct extra_descr_data *new_descr;
  int shop_num, type, day, weekday;
  const char *suf;
  void system_notes(struct char_data *ch, char *text, int type);

  if (!(CMD_IS("use")))
    return (FALSE);

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return (TRUE);
  }

  /* Was the unused deed specified? */
  if ((store_deed = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    if (deed != store_deed)
      return (FALSE);
  } else
    return (FALSE);

  /* Was the "deed" used by the correct player? */
  if ((GET_OBJ_VAL(deed, 2) != GET_IDNUM(ch)) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("This scroll does not belong to you!\r\n", ch);
    return (TRUE);
  }

  /* Was the "deed" used in the player's hometown? */
  if (GET_OBJ_VAL(deed, 1) != ch->player.hometown) {
    sprintf(buf, "This scroll can only be used in %s.\r\n", hometowns[ch->player.hometown]);
    send_to_char(buf, ch);
    return (TRUE);
  }

  /* Was the "deed" used in an empty store allocated for a player shop? */
  if (GET_ROOM_SPEC(ch->in_room) != player_shop) {
    send_to_char("You can only use this in an empty store!\r\n", ch);
    return (TRUE);
  } else {
    shop_num = real_shop(world[ch->in_room].number);
    shop = shop_index + shop_num;
    if (strcmp(shop->owner, "None")) {
      sprintf(buf, "This store has already been purchased by %s.\r\n", shop->owner);
      send_to_char(buf, ch);
      return (TRUE);
    }
  }

  /* Need 50000 coins to kick start the keeper's finances */
  if (GET_GOLD(ch) < 50000) {
    send_to_char("You lack the 50000 gold coins to give to the shop keeper!\r\n", ch);
    return (TRUE);
  }

  /* Purchase the deed - it's now theirs! */
  type = GET_OBJ_VAL(deed, 0);
  send_to_char("You proudly frame the new deed and hang it on the wall.\r\n", ch);
  act("$n proudly frames $s new deed and hangs it on the wall.", TRUE, ch, 0, 0, TO_ROOM);

  /* Copy over appropriate variables to the "store deed" */
  store_deed = read_object(1223, VIRTUAL, NULL);
  GET_OBJ_VAL(store_deed, 0) = type;
  GET_OBJ_VAL(store_deed, 1) = GET_OBJ_VAL(deed, 1);
  GET_OBJ_VAL(store_deed, 2) = GET_OBJ_VAL(deed, 2);
  GET_OBJ_VAL(store_deed, 3) = GET_OBJ_VAL(deed, 3);
  extract_obj(deed);

  /* Work out the time/day for the deed */
  day = time_info.day + 1;	                      /* day in [1..35] */
  weekday = ((DAYS_PER_MONTH * time_info.month) + day) % DAYS_PER_WEEK;
  suf = "th";
  if (((day % 100) / 10) != 1) {                  /* suffix to the day */
    switch (day % 10) {
    case 1:
      suf = "st";
      break;
    case 2:
      suf = "nd";
      break;
    case 3:
      suf = "rd";
      break;
    }
  }

  /* Place the new deed on the shop wall */
  CREATE(new_descr, struct extra_descr_data, 1);
  sprintf(buf,
    " ______________________________________________________\r\n"
    "/\\                                                     \\ \r\n"
    "\\_|  Official deed between the city of: &g%-16s&n|\r\n"
    "  |  And local citizen: &g%-32s&n|\r\n"
    "  |  For the purchase of: &g%-30s&n|\r\n"
    "  |                                                     |\r\n"
    "  |  Dated %-45s|\r\n"
    "  |  The %d%s Day of the %-31s|\r\n"
    "  |  Year %-46d|\r\n"
    "  |   __________________________________________________|_\r\n"
    "   \\_/___________________________________________________/\r\n",
    hometowns[ch->player.hometown], GET_NAME(ch), shop_types[type],
    weekdays[weekday], day, suf, month_name[(int) time_info.month], time_info.year);

  new_descr->keyword = str_dup(obj_proto[GET_OBJ_RNUM(store_deed)].ex_description->keyword);
  new_descr->description = str_dup(buf);
  store_deed->ex_description = new_descr;

  player_buy_shop(ch, GET_OBJ_VAL(store_deed, 1), type);
  obj_to_room(store_deed, ch->in_room);

  /* Let's send a system note to all players and let them know */
  sprintf(buf, "%s has opened %s in the city of %s.", GET_NAME(ch), shop_types[type],
    hometowns[ch->player.hometown]);
  system_notes(ch, buf, 0);
  return (TRUE);
}

/* "Closes" the player's store and returns it to the empty state */
SPECIAL(deed_assigned)
{
  struct obj_data *store_deed = (struct obj_data *)me;
  struct obj_data *deed;
  void system_notes(struct char_data *ch, char *text, int type);

  if (!(CMD_IS("use")))
    return (FALSE);

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return (TRUE);
  }

  /* Was the used deed specified? */
  if ((deed = get_obj_in_list_vis(ch, arg, world[IN_ROOM(ch)].contents))) {
    if (deed != store_deed)
      return (FALSE);
  } else
    return (FALSE);

  /* Was the "deed" used by the correct player? */
  if ((GET_OBJ_VAL(store_deed, 2) != GET_IDNUM(ch)) && (GET_LEVEL(ch) < LVL_IMPL - 1)) {
    send_to_char("This deed does not belong to you!\r\n", ch);
    return (TRUE);
  }

  /* Remove the store deed and give to the player */
  deed = read_object(1222, VIRTUAL, NULL);
  GET_OBJ_VAL(deed, 0) = GET_OBJ_VAL(store_deed, 0);
  GET_OBJ_VAL(deed, 1) = GET_OBJ_VAL(store_deed, 1);
  GET_OBJ_VAL(deed, 2) = GET_OBJ_VAL(store_deed, 2);
  GET_OBJ_VAL(deed, 3) = GET_OBJ_VAL(store_deed, 3);
  if (store_deed->ex_description->description)
    FREE(store_deed->ex_description->description);
  if (store_deed->ex_description)
    FREE(store_deed->ex_description);
  extract_obj(store_deed);
  obj_to_char(deed, ch);

  /* Return the store to it's empty state */
  player_sell_shop(ch);

  send_to_char("Sadly, you remove the deed from the wall.\r\n", ch);
  act("With a tear in $s eye, $n removes the deed from the wall.", TRUE, ch, 0, 0, TO_ROOM);

  /* Let's send a system note to all players and let them know */
  sprintf(buf, "%s has closed %s in the city of %s.", GET_NAME(ch), shop_types[GET_OBJ_VAL(deed, 0)],
    hometowns[ch->player.hometown]);
  system_notes(ch, buf, 0);
  return (TRUE);
}

/* Based on the city tax rate, tax the gold earned by the player's shopkeep */
int taxed_amount(struct char_data *keeper, int amount)
{
  int tax, shop_nr, homet;

  for (shop_nr = 0, tax = 0; shop_nr < top_shop; shop_nr++)
    if (get_id_by_name(SHOP_OWNER(shop_nr)) > 0)
      if (GET_MOB_VNUM(keeper) == SHOP_KEEPER(shop_nr)) {
        switch (zone_table[real_zone(world[keeper->in_room].number)].number) {
          case 80:  homet = 0; break;
          case 182: homet = 1; break;
          case 212: homet = 2; break;
          case 48:  homet = 3; break;
          case 53:  homet = 4; break;
          default:  homet = 0; break;
        }
        tax = amount * shop_cost[homet].tax;
        break;
      }

  return (amount - tax);
}

/* For player-owned shops, update keeper (ie. total gold, wages, etc) */
void pcshop_update(void)
{
  int shop_nr, homet;
  struct char_data *keeper, *cbuf = NULL;
  void system_notes(struct char_data *ch, char *text, int type);

  for (shop_nr = 0; shop_nr < top_shop; shop_nr++)
    if (get_id_by_name(SHOP_OWNER(shop_nr)) > 0)
      if ((keeper = get_char_num(SHOP_KEEPER(shop_nr)))) {
        switch (zone_table[real_zone(world[keeper->in_room].number)].number) {
          case 80:  homet = 0; break;
          case 182: homet = 1; break;
          case 212: homet = 2; break;
          case 48:  homet = 3; break;
          case 53:  homet = 4; break;
          default:  homet = 0; break;
        }
        /* Deduct shopkeeper wages based on opening/closing times */
        GET_GOLD(keeper) -= ((SHOP_CLOSE1(shop_nr) - SHOP_OPEN1(shop_nr)) * shop_cost[homet].hire);
        GET_GOLD(keeper) = MAX(GET_GOLD(keeper), 0);

        if (!(cbuf = load_char(SHOP_OWNER(shop_nr))))
          continue;

        /* Running out of cash, inform the player via a SYSTEM note */
        if ((GET_GOLD(keeper) < 20000) && !MOB_FLAGGED(keeper, MOB_WARN1)) {
          sprintf(buf, "%s shop keeper in %s has less than 20000 gold coins.",
            world[IN_ROOM(keeper)].name, hometowns[homet]);
          sql_write_notes("SYSTEM", buf, cbuf);
          SET_BIT(MOB_FLAGS(keeper), MOB_WARN1);
          free_char(cbuf);
          continue;
        }

        /* Out of cash, inform the player and set the "closure" timer */
        if ((GET_GOLD(keeper) <= 0) && !MOB_FLAGGED(keeper, MOB_WARN2)) {
          sprintf(buf, "%s shop keeper in %s is out of money!", 
            world[IN_ROOM(keeper)].name, hometowns[homet]);
          sql_write_notes("SYSTEM", buf, cbuf);
          GET_MOB_TIMER(keeper) = (short int)(3 * SECS_PER_REAL_DAY);     /* 72 hours */
          SET_BIT(MOB_FLAGS(keeper), MOB_WARN2);
          free_char(cbuf);
          continue;
        }

        /* Out of cash, player warned and timed expired */
        if ((GET_GOLD(keeper) <= 0) && MOB_FLAGGED(keeper, MOB_WARN1) &&
             MOB_FLAGGED(keeper, MOB_WARN2) && !GET_MOB_TIMER(keeper)) {
          sprintf(buf, "%s in %s has been reclaimed by the town due to lack of money!",
            world[IN_ROOM(keeper)].name, hometowns[homet]);
          sql_write_notes("SYSTEM", buf, cbuf);
          sprintf(buf, "%s, owned by %s, has been reclaimed.",
            world[IN_ROOM(keeper)].name, SHOP_OWNER(shop_nr));
          mudlog(buf, NRM, LVL_IMMORT, TRUE);

          act("Members from the city clerk's office enter the store, seize all items and close"
              " it down due to lack of funding.", FALSE, keeper, 0, 0, TO_ROOM);

          CREATE(keeper->desc, struct descriptor_data, 1);
          player_sell_shop(keeper);
          free_char(cbuf);
          FREE(keeper->desc);
          continue;
        }

        /* Somewhere between "just ok" and "ok" - reset variables */
        if (MOB_FLAGGED(keeper, MOB_WARN1) && MOB_FLAGGED(keeper, MOB_WARN2)) {
          if ((GET_GOLD(keeper) > 0) && (GET_GOLD(keeper) < 20000)) {
            GET_MOB_TIMER(keeper) = 0;
            REMOVE_BIT(MOB_FLAGS(keeper), MOB_WARN2);
          } else if (GET_GOLD(keeper) > 20000) {
            GET_MOB_TIMER(keeper) = 0;
            REMOVE_BIT(MOB_FLAGS(keeper), MOB_WARN1);
            REMOVE_BIT(MOB_FLAGS(keeper), MOB_WARN2);
          }
        }
        free_char(cbuf);
      }
}
