/* This contains a MySQL dump of the dp database schema. */

-- MySQL dump 10.9
--
-- Host: localhost    Database: dp
-- ------------------------------------------------------
-- Server version	4.1.20-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `accept_types`
--

DROP TABLE IF EXISTS `accept_types`;
CREATE TABLE `accept_types` (
  `id` int(11) NOT NULL auto_increment,
  `shop_id` int(11) NOT NULL default '0',
  `type_flag_id` int(11) NOT NULL default '0',
  `keywords` varchar(255) default NULL,
  PRIMARY KEY  (`id`),
  KEY `fk_at_shop` (`shop_id`),
  CONSTRAINT `fk_at_shop` FOREIGN KEY (`shop_id`) REFERENCES `shops` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `aff_flags`
--

DROP TABLE IF EXISTS `aff_flags`;
CREATE TABLE `aff_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `aff_flags_mobs`
--

DROP TABLE IF EXISTS `aff_flags_mobs`;
CREATE TABLE `aff_flags_mobs` (
  `mob_id` int(11) NOT NULL default '0',
  `aff_flag_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`mob_id`,`aff_flag_id`),
  KEY `fk_afm_aff_flag` (`aff_flag_id`),
  CONSTRAINT `fk_afm_aff_flag` FOREIGN KEY (`aff_flag_id`) REFERENCES `aff_flags` (`id`),
  CONSTRAINT `fk_afm_mob` FOREIGN KEY (`mob_id`) REFERENCES `mobs` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `affects`
--

DROP TABLE IF EXISTS `affects`;
CREATE TABLE `affects` (
  `id` int(11) NOT NULL auto_increment,
  `object_id` int(11) NOT NULL default '0',
  `apply_flag_id` int(11) NOT NULL default '0',
  `modifier` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fk_a_object` (`object_id`),
  KEY `fk_a_apply_flag` (`apply_flag_id`),
  CONSTRAINT `fk_a_object` FOREIGN KEY (`object_id`) REFERENCES `objects` (`id`),
  CONSTRAINT `fk_a_apply_flag` FOREIGN KEY (`apply_flag_id`) REFERENCES `apply_flags` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `apply_flags`
--

DROP TABLE IF EXISTS `apply_flags`;
CREATE TABLE `apply_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `especs`
--

DROP TABLE IF EXISTS `especs`;
CREATE TABLE `especs` (
  `id` int(11) NOT NULL auto_increment,
  `mob_id` int(11) NOT NULL default '0',
  `keyword` varchar(50) NOT NULL default '',
  `value` varchar(25) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `fk_e_mob` (`mob_id`),
  CONSTRAINT `fk_e_mob` FOREIGN KEY (`mob_id`) REFERENCES `mobs` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `exits`
--

DROP TABLE IF EXISTS `exits`;
CREATE TABLE `exits` (
  `id` int(11) NOT NULL auto_increment,
  `room_id` int(11) NOT NULL default '0',
  `direction` int(11) NOT NULL default '0',
  `general_description` varchar(255) NOT NULL default '',
  `keyword` varchar(255) NOT NULL default '',
  `door_info` int(11) NOT NULL default '0',
  `door_key` int(11) NOT NULL default '0',
  `to_room_vnum` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fk_e_room` (`room_id`),
  CONSTRAINT `fk_e_room` FOREIGN KEY (`room_id`) REFERENCES `rooms` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `extra_descriptions`
--

DROP TABLE IF EXISTS `extra_descriptions`;
CREATE TABLE `extra_descriptions` (
  `id` int(11) NOT NULL auto_increment,
  `room_id` int(11) default NULL,
  `object_id` int(11) default NULL,
  `keyword` varchar(255) NOT NULL default '',
  `description` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `fk_ed_room` (`room_id`),
  KEY `fk_ed_object` (`object_id`),
  CONSTRAINT `fk_ed_room` FOREIGN KEY (`room_id`) REFERENCES `rooms` (`id`),
  CONSTRAINT `fk_ed_object` FOREIGN KEY (`object_id`) REFERENCES `objects` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `extra_flags`
--

DROP TABLE IF EXISTS `extra_flags`;
CREATE TABLE `extra_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `extra_flags_objects`
--

DROP TABLE IF EXISTS `extra_flags_objects`;
CREATE TABLE `extra_flags_objects` (
  `extra_flag_id` int(11) NOT NULL default '0',
  `object_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`extra_flag_id`,`object_id`),
  KEY `fk_efo_object` (`object_id`),
  CONSTRAINT `fk_efo_extra_flag` FOREIGN KEY (`extra_flag_id`) REFERENCES `extra_flags` (`id`),
  CONSTRAINT `fk_efo_object` FOREIGN KEY (`object_id`) REFERENCES `objects` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mob_flags`
--

DROP TABLE IF EXISTS `mob_flags`;
CREATE TABLE `mob_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mob_flags_mobs`
--

DROP TABLE IF EXISTS `mob_flags_mobs`;
CREATE TABLE `mob_flags_mobs` (
  `mob_id` int(11) NOT NULL default '0',
  `mob_flag_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`mob_id`,`mob_flag_id`),
  KEY `fk_mfm_mob_flag` (`mob_flag_id`),
  CONSTRAINT `fk_mfm_mob_flag` FOREIGN KEY (`mob_flag_id`) REFERENCES `mob_flags` (`id`),
  CONSTRAINT `fk_mfm_mob` FOREIGN KEY (`mob_id`) REFERENCES `mobs` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `mobs`
--

DROP TABLE IF EXISTS `mobs`;
CREATE TABLE `mobs` (
  `id` int(11) NOT NULL auto_increment,
  `vnum` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `short_description` varchar(255) NOT NULL default '',
  `long_description` varchar(255) NOT NULL default '',
  `description` text NOT NULL,
  `alignment` int(11) NOT NULL default '0',
  `level` int(11) NOT NULL default '0',
  `hitroll` int(11) NOT NULL default '0',
  `damroll` int(11) NOT NULL default '0',
  `num_dam_dice` int(11) NOT NULL default '0',
  `size_dam_dice` int(11) NOT NULL default '0',
  `armor` int(11) NOT NULL default '0',
  `num_hp_dice` int(11) NOT NULL default '0',
  `size_hp_dice` int(11) NOT NULL default '0',
  `hp_bonus` int(11) NOT NULL default '0',
  `gold` int(11) NOT NULL default '0',
  `experience` int(11) NOT NULL default '0',
  `position` int(11) NOT NULL default '0',
  `default_position` int(11) NOT NULL default '0',
  `sex` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `vnum` (`vnum`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `notrade_flags`
--

DROP TABLE IF EXISTS `notrade_flags`;
CREATE TABLE `notrade_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `object_values`
--

DROP TABLE IF EXISTS `object_values`;
CREATE TABLE `object_values` (
  `id` int(11) NOT NULL auto_increment,
  `object_id` int(11) NOT NULL default '0',
  `value_zero` int(11) NOT NULL default '0',
  `value_one` int(11) NOT NULL default '0',
  `value_two` int(11) NOT NULL default '0',
  `value_three` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fk_ov_object` (`object_id`),
  CONSTRAINT `fk_ov_object` FOREIGN KEY (`object_id`) REFERENCES `objects` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `objects`
--

DROP TABLE IF EXISTS `objects`;
CREATE TABLE `objects` (
  `id` int(11) NOT NULL auto_increment,
  `type_flag_id` int(11) NOT NULL default '0',
  `vnum` int(11) NOT NULL default '0',
  `name` varchar(50) NOT NULL default '',
  `short_description` varchar(255) NOT NULL default '',
  `description` text NOT NULL,
  `action_description` varchar(255) NOT NULL default '',
  `weight` int(11) NOT NULL default '0',
  `cost` int(11) NOT NULL default '0',
  `percent_chance_to_load` float NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `vnum` (`vnum`),
  KEY `fk_o_type_flag` (`type_flag_id`),
  CONSTRAINT `fk_o_type_flag` FOREIGN KEY (`type_flag_id`) REFERENCES `type_flags` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `objects_wear_flags`
--

DROP TABLE IF EXISTS `objects_wear_flags`;
CREATE TABLE `objects_wear_flags` (
  `wear_flag_id` int(11) NOT NULL default '0',
  `object_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`wear_flag_id`,`object_id`),
  KEY `fk_wfo_object` (`object_id`),
  CONSTRAINT `fk_wfo_extra_flag` FOREIGN KEY (`wear_flag_id`) REFERENCES `wear_flags` (`id`),
  CONSTRAINT `fk_wfo_object` FOREIGN KEY (`object_id`) REFERENCES `objects` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `products`
--

DROP TABLE IF EXISTS `products`;
CREATE TABLE `products` (
  `id` int(11) NOT NULL auto_increment,
  `shop_id` int(11) NOT NULL default '0',
  `object_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fk_p_shop` (`shop_id`),
  CONSTRAINT `fk_p_shop` FOREIGN KEY (`shop_id`) REFERENCES `shops` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `reset_cmds`
--

DROP TABLE IF EXISTS `reset_cmds`;
CREATE TABLE `reset_cmds` (
  `id` int(11) NOT NULL auto_increment,
  `zone_id` int(11) NOT NULL default '0',
  `position` int(11) NOT NULL default '0',
  `command` char(1) NOT NULL default '',
  `if_flag` tinyint(1) NOT NULL default '0',
  `arg1` int(11) NOT NULL default '0',
  `arg2` int(11) NOT NULL default '0',
  `arg3` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fk_rc_zone` (`zone_id`),
  CONSTRAINT `fk_rc_zone` FOREIGN KEY (`zone_id`) REFERENCES `zones` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `room_flags`
--

DROP TABLE IF EXISTS `room_flags`;
CREATE TABLE `room_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `room_flags_rooms`
--

DROP TABLE IF EXISTS `room_flags_rooms`;
CREATE TABLE `room_flags_rooms` (
  `room_id` int(11) NOT NULL default '0',
  `room_flag_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`room_id`,`room_flag_id`),
  KEY `fk_rfr_room_flag` (`room_flag_id`),
  CONSTRAINT `fk_rfr_room` FOREIGN KEY (`room_id`) REFERENCES `rooms` (`id`),
  CONSTRAINT `fk_rfr_room_flag` FOREIGN KEY (`room_flag_id`) REFERENCES `room_flags` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `rooms`
--

DROP TABLE IF EXISTS `rooms`;
CREATE TABLE `rooms` (
  `id` int(11) NOT NULL auto_increment,
  `zone_id` int(11) NOT NULL default '0',
  `vnum` int(11) NOT NULL default '0',
  `sector_type` int(11) default NULL,
  `name` varchar(255) NOT NULL default '',
  `description` text,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `vnum` (`vnum`),
  KEY `fk_r_zone` (`zone_id`),
  CONSTRAINT `fk_r_zone` FOREIGN KEY (`zone_id`) REFERENCES `zones` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `rooms_shops`
--

DROP TABLE IF EXISTS `rooms_shops`;
CREATE TABLE `rooms_shops` (
  `room_id` int(11) NOT NULL default '0',
  `shop_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`room_id`,`shop_id`),
  KEY `fk_rs_shop` (`shop_id`),
  CONSTRAINT `fk_rs_room` FOREIGN KEY (`room_id`) REFERENCES `rooms` (`id`),
  CONSTRAINT `fk_rs_shop` FOREIGN KEY (`shop_id`) REFERENCES `shops` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `shop_flags`
--

DROP TABLE IF EXISTS `shop_flags`;
CREATE TABLE `shop_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `shop_flags_shops`
--

DROP TABLE IF EXISTS `shop_flags_shops`;
CREATE TABLE `shop_flags_shops` (
  `shop_id` int(11) NOT NULL default '0',
  `shop_flag_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`shop_id`,`shop_flag_id`),
  KEY `fk_sfs_shop_flag` (`shop_flag_id`),
  CONSTRAINT `fk_sfs_shop` FOREIGN KEY (`shop_id`) REFERENCES `shops` (`id`),
  CONSTRAINT `fk_sfs_shop_flag` FOREIGN KEY (`shop_flag_id`) REFERENCES `shop_flags` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `shop_messages`
--

DROP TABLE IF EXISTS `shop_messages`;
CREATE TABLE `shop_messages` (
  `id` int(11) NOT NULL auto_increment,
  `shop_id` int(11) NOT NULL default '0',
  `keeper_no_item` varchar(255) NOT NULL default '',
  `player_no_item` varchar(255) NOT NULL default '',
  `keeper_no_cash` varchar(255) NOT NULL default '',
  `player_no_cash` varchar(255) NOT NULL default '',
  `keeper_no_buy` varchar(255) NOT NULL default '',
  `buy_success` varchar(255) NOT NULL default '',
  `sell_success` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `fk_sm_shop` (`shop_id`),
  CONSTRAINT `fk_sm_shop` FOREIGN KEY (`shop_id`) REFERENCES `shops` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `shops`
--

DROP TABLE IF EXISTS `shops`;
CREATE TABLE `shops` (
  `id` int(11) NOT NULL auto_increment,
  `vnum` int(11) NOT NULL default '0',
  `buy_profit` float NOT NULL default '0',
  `sell_profit` float NOT NULL default '0',
  `mob_id` int(11) default NULL,
  `open_time_1` int(11) NOT NULL default '0',
  `close_time_1` int(11) NOT NULL default '0',
  `open_time_2` int(11) NOT NULL default '0',
  `close_time_2` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `vnum` (`vnum`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `shops_notrade_flags`
--

DROP TABLE IF EXISTS `shops_notrade_flags`;
CREATE TABLE `shops_notrade_flags` (
  `shop_id` int(11) NOT NULL default '0',
  `notrade_flag_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`shop_id`,`notrade_flag_id`),
  KEY `fk_snf_notrade_flag` (`notrade_flag_id`),
  CONSTRAINT `fk_snf_shop` FOREIGN KEY (`shop_id`) REFERENCES `shops` (`id`),
  CONSTRAINT `fk_snf_notrade_flag` FOREIGN KEY (`notrade_flag_id`) REFERENCES `notrade_flags` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `type_flags`
--

DROP TABLE IF EXISTS `type_flags`;
CREATE TABLE `type_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `wear_flags`
--

DROP TABLE IF EXISTS `wear_flags`;
CREATE TABLE `wear_flags` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(50) NOT NULL default '',
  `on_bit` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `zones`
--

DROP TABLE IF EXISTS `zones`;
CREATE TABLE `zones` (
  `id` int(11) NOT NULL auto_increment,
  `number` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `top` int(11) NOT NULL default '0',
  `lifespan` int(11) NOT NULL default '0',
  `reset_mode` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `number` (`number`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

