-- MySQL dump 10.9
--
-- Host: localhost    Database: dpmain
-- ------------------------------------------------------
-- Server version	4.0.20-standard
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `accounts`
--

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `name` varchar(20) NOT NULL default '',
  `pwd` varchar(10) NOT NULL default '',
  `bad_pws` tinyint(1) unsigned NOT NULL default '0',
  `num_chars` tinyint(1) unsigned NOT NULL default '0',
  `char_one` smallint(4) unsigned NOT NULL default '0',
  `char_two` smallint(4) unsigned NOT NULL default '0',
  `char_three` smallint(4) unsigned NOT NULL default '0',
  PRIMARY KEY  (`name`)
) TYPE=MyISAM;

--
-- Table structure for table `aliases`
--

DROP TABLE IF EXISTS `aliases`;
CREATE TABLE `aliases` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `alias` varchar(120) NOT NULL default '',
  `replacement` varchar(120) NOT NULL default '',
  `type` smallint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`,`alias`)
) TYPE=MyISAM;

--
-- Table structure for table `ban`
--

DROP TABLE IF EXISTS `ban`;
CREATE TABLE `ban` (
  `site` varchar(30) NOT NULL default '',
  `type` tinyint(1) unsigned NOT NULL default '0',
  `date` int(10) unsigned NOT NULL default '0',
  `length` tinyint(1) unsigned NOT NULL default '0',
  `account` varchar(20) NOT NULL default '',
  `name` varchar(20) NOT NULL default '',
  `comment` varchar(30) NOT NULL default '',
  PRIMARY KEY  (`site`)
) TYPE=MyISAM;

--
-- Table structure for table `houseobjs`
--

DROP TABLE IF EXISTS `houseobjs`;
CREATE TABLE `houseobjs` (
  `room_vnum` smallint(5) unsigned NOT NULL default '0',
  `obj_id` char(36) NOT NULL default '',
  `vnum` smallint(5) NOT NULL default '0',
  `location` smallint(5) NOT NULL default '0',
  PRIMARY KEY  (`room_vnum`,`obj_id`)
) TYPE=MyISAM COMMENT='Persistant objects in rooms';

--
-- Table structure for table `mail`
--

DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `id_from` smallint(4) unsigned NOT NULL default '0',
  `id` smallint(4) unsigned NOT NULL default '0',
  `date` int(10) unsigned NOT NULL default '0',
  `mail` text NOT NULL,
  PRIMARY KEY  (`id_from`,`date`)
) TYPE=MyISAM;

--
-- Table structure for table `mob_saves`
--

DROP TABLE IF EXISTS `mob_saves`;
CREATE TABLE `mob_saves` (
  `uuid` varchar(36) NOT NULL default '',
  `alias` varchar(30) NOT NULL default '',
  `short_desc` varchar(50) NOT NULL default '',
  `long_desc` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`uuid`)
) TYPE=MyISAM;

--
-- Table structure for table `mobobjs`
--

DROP TABLE IF EXISTS `mobobjs`;
CREATE TABLE `mobobjs` (
  `uuid` char(36) NOT NULL default '',
  `obj_id` char(36) NOT NULL default '',
  `vnum` smallint(5) unsigned NOT NULL default '0',
  `location` smallint(5) NOT NULL default '0',
  PRIMARY KEY  (`uuid`,`obj_id`)
) TYPE=MyISAM COMMENT='Persistant objects on mobiles';

--
-- Table structure for table `mobs`
--

DROP TABLE IF EXISTS `mobs`;
CREATE TABLE `mobs` (
  `uuid` char(36) NOT NULL default '',
  `vnum` smallint(5) unsigned NOT NULL default '0',
  `room` smallint(5) unsigned NOT NULL default '0',
  `hitpoints` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uuid`),
  KEY `uuid` (`uuid`)
) TYPE=MyISAM COMMENT='Persistant mobiles';

--
-- Table structure for table `mud_date`
--

DROP TABLE IF EXISTS `mud_date`;
CREATE TABLE `mud_date` (
  `day` tinyint(3) unsigned NOT NULL default '0',
  `month` tinyint(3) unsigned NOT NULL default '0',
  `year` smallint(4) unsigned NOT NULL default '0',
  `game_restrict` tinyint(3) unsigned NOT NULL default '0',
  `shops_time` int(10) unsigned NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `notes`
--

DROP TABLE IF EXISTS `notes`;
CREATE TABLE `notes` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `note` varchar(255) NOT NULL default '',
  `date` int(10) unsigned NOT NULL default '0',
  `note_by` smallint(4) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`,`date`)
) TYPE=MyISAM;

--
-- Table structure for table `obj_action_desc`
--

DROP TABLE IF EXISTS `obj_action_desc`;
CREATE TABLE `obj_action_desc` (
  `uuid` varchar(36) NOT NULL default '',
  `description` text NOT NULL,
  PRIMARY KEY  (`uuid`)
) TYPE=MyISAM;

--
-- Table structure for table `obj_affects`
--

DROP TABLE IF EXISTS `obj_affects`;
CREATE TABLE `obj_affects` (
  `uuid` char(36) NOT NULL default '',
  `location` tinyint(2) unsigned NOT NULL default '0',
  `modifier` tinyint(2) NOT NULL default '0',
  PRIMARY KEY  (`uuid`,`location`)
) TYPE=MyISAM COMMENT='Object affections';

--
-- Table structure for table `obj_container`
--

DROP TABLE IF EXISTS `obj_container`;
CREATE TABLE `obj_container` (
  `uuid` char(36) NOT NULL default '',
  `cont_id` char(36) NOT NULL default '',
  PRIMARY KEY  (`uuid`)
) TYPE=MyISAM COMMENT='Object and their containers';

--
-- Table structure for table `obj_extra_desc`
--

DROP TABLE IF EXISTS `obj_extra_desc`;
CREATE TABLE `obj_extra_desc` (
  `uuid` varchar(36) NOT NULL default '',
  `keyword` varchar(128) NOT NULL default '',
  `description` text NOT NULL,
  PRIMARY KEY  (`uuid`,`keyword`)
) TYPE=MyISAM COMMENT='Object extra descriptions';

--
-- Table structure for table `obj_reqs`
--

DROP TABLE IF EXISTS `obj_reqs`;
CREATE TABLE `obj_reqs` (
  `uuid` char(36) NOT NULL default '',
  `location` tinyint(2) unsigned NOT NULL default '0',
  `modifier` tinyint(2) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uuid`,`location`)
) TYPE=MyISAM COMMENT='Object requirements';

--
-- Table structure for table `obj_value`
--

DROP TABLE IF EXISTS `obj_value`;
CREATE TABLE `obj_value` (
  `uuid` char(36) NOT NULL default '',
  `val_zero` int(10) NOT NULL default '0',
  `val_one` tinyint(2) NOT NULL default '0',
  `val_two` tinyint(2) NOT NULL default '0',
  `val_three` smallint(3) NOT NULL default '0',
  `val_four` tinyint(2) NOT NULL default '0',
  `cost` int(10) NOT NULL default '0',
  PRIMARY KEY  (`uuid`)
) TYPE=MyISAM COMMENT='Object values';

--
-- Table structure for table `objs`
--

DROP TABLE IF EXISTS `objs`;
CREATE TABLE `objs` (
  `uuid` varchar(36) NOT NULL default '',
  `weight` smallint(5) unsigned NOT NULL default '0',
  `timer` tinyint(3) unsigned NOT NULL default '0',
  `extra` varchar(10) default NULL,
  `wear` varchar(10) default NULL,
  `bitvector` varchar(10) default NULL,
  `name` varchar(128) default NULL,
  `descr` varchar(255) default NULL,
  `sdescr` varchar(128) default NULL,
  PRIMARY KEY  (`uuid`)
) TYPE=MyISAM COMMENT='Persistant objects';

--
-- Table structure for table `player_affected`
--

DROP TABLE IF EXISTS `player_affected`;
CREATE TABLE `player_affected` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `type` tinyint(3) unsigned NOT NULL default '0',
  `duration` tinyint(3) unsigned NOT NULL default '0',
  `modifier` smallint(4) unsigned NOT NULL default '0',
  `location` tinyint(2) unsigned NOT NULL default '0',
  `bitvector` varchar(10) default NULL,
  PRIMARY KEY  (`id`,`type`,`location`)
) TYPE=MyISAM;

--
-- Table structure for table `player_prefs`
--

DROP TABLE IF EXISTS `player_prefs`;
CREATE TABLE `player_prefs` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `infobar` tinyint(1) unsigned NOT NULL default '0',
  `size` tinyint(2) unsigned NOT NULL default '0',
  `religion` tinyint(1) unsigned NOT NULL default '0',
  `mount_vnum` smallint(5) unsigned NOT NULL default '0',
  `stable` smallint(5) unsigned NOT NULL default '0',
  `rank` tinyint(2) unsigned NOT NULL default '0',
  `patron` smallint(4) unsigned NOT NULL default '0',
  `bind_room` smallint(5) NOT NULL default '0',
  `wimp_level` smallint(4) unsigned NOT NULL default '0',
  `freeze_level` tinyint(3) unsigned NOT NULL default '0',
  `invis_level` tinyint(3) unsigned NOT NULL default '0',
  `hometown` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `player_race`
--

DROP TABLE IF EXISTS `player_race`;
CREATE TABLE `player_race` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `race` tinyint(1) unsigned NOT NULL default '0',
  `kills` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `player_rent`
--

DROP TABLE IF EXISTS `player_rent`;
CREATE TABLE `player_rent` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `rent_code` tinyint(1) unsigned NOT NULL default '0',
  `time` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `player_shops`
--

DROP TABLE IF EXISTS `player_shops`;
CREATE TABLE `player_shops` (
  `shop_nr` tinyint(3) unsigned NOT NULL default '0',
  `keeper` smallint(5) unsigned NOT NULL default '0',
  `room` smallint(5) unsigned NOT NULL default '0',
  `type` tinyint(1) unsigned NOT NULL default '0',
  `buy_profit` float(4,2) unsigned NOT NULL default '0.00',
  `sell_profit` float(4,2) unsigned NOT NULL default '0.00',
  `item_limit` smallint(3) unsigned NOT NULL default '0',
  `open` tinyint(2) unsigned NOT NULL default '0',
  `close` tinyint(2) unsigned NOT NULL default '0',
  `id` smallint(4) unsigned NOT NULL default '0',
  PRIMARY KEY  (`shop_nr`)
) TYPE=MyISAM;

--
-- Table structure for table `player_skills`
--

DROP TABLE IF EXISTS `player_skills`;
CREATE TABLE `player_skills` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `grp` tinyint(1) unsigned NOT NULL default '0',
  `level` tinyint(1) unsigned NOT NULL default '0',
  `skill` tinyint(3) unsigned NOT NULL default '0',
  `improve` tinyint(2) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`,`grp`)
) TYPE=MyISAM;

--
-- Table structure for table `player_spells`
--

DROP TABLE IF EXISTS `player_spells`;
CREATE TABLE `player_spells` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `spells` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `player_squelch`
--

DROP TABLE IF EXISTS `player_squelch`;
CREATE TABLE `player_squelch` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `squelch_one` smallint(4) unsigned NOT NULL default '0',
  `squelch_two` smallint(4) unsigned NOT NULL default '0',
  `squelch_three` smallint(4) unsigned NOT NULL default '0',
  `squelch_four` smallint(4) unsigned NOT NULL default '0',
  `squelch_five` smallint(4) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `player_stats`
--

DROP TABLE IF EXISTS `player_stats`;
CREATE TABLE `player_stats` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `mana` smallint(4) unsigned NOT NULL default '0',
  `max_mana` smallint(4) unsigned NOT NULL default '0',
  `hit` smallint(4) unsigned NOT NULL default '0',
  `max_hit` smallint(4) unsigned NOT NULL default '0',
  `move` smallint(4) unsigned NOT NULL default '0',
  `max_move` smallint(4) unsigned NOT NULL default '0',
  `real_str` tinyint(2) unsigned NOT NULL default '0',
  `real_intel` tinyint(2) unsigned NOT NULL default '0',
  `real_wis` tinyint(2) unsigned NOT NULL default '0',
  `real_dex` tinyint(2) unsigned NOT NULL default '0',
  `real_con` tinyint(2) unsigned NOT NULL default '0',
  `real_cha` tinyint(2) unsigned NOT NULL default '0',
  `pkcount` smallint(3) unsigned NOT NULL default '0',
  `killcount` smallint(5) unsigned NOT NULL default '0',
  `deathcount` smallint(3) unsigned NOT NULL default '0',
  `lastdeath` int(10) unsigned NOT NULL default '0',
  `vitae` tinyint(2) unsigned NOT NULL default '0',
  `vitae_exp` mediumint(6) unsigned NOT NULL default '0',
  `exp` int(8) unsigned NOT NULL default '0',
  `exp_queue` int(8) unsigned NOT NULL default '0',
  `weight` tinyint(3) unsigned NOT NULL default '0',
  `alignment` smallint(4) NOT NULL default '0',
  `sex` tinyint(1) unsigned NOT NULL default '0',
  `level` tinyint(3) unsigned NOT NULL default '0',
  `stat_points` tinyint(3) unsigned NOT NULL default '0',
  `group_points` tinyint(1) unsigned NOT NULL default '0',
  `tattoo` tinyint(1) unsigned NOT NULL default '0',
  `dyes` varchar(10) default NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `player_timers`
--

DROP TABLE IF EXISTS `player_timers`;
CREATE TABLE `player_timers` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `drunk` tinyint(3) NOT NULL default '0',
  `full` tinyint(3) NOT NULL default '0',
  `thirsty` tinyint(3) NOT NULL default '0',
  `tired` tinyint(3) NOT NULL default '0',
  `outlaw` tinyint(2) unsigned NOT NULL default '0',
  `jail` tinyint(3) unsigned NOT NULL default '0',
  `mount` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `players`
--

DROP TABLE IF EXISTS `players`;
CREATE TABLE `players` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `name` varchar(20) NOT NULL default '',
  `description` varchar(240) default NULL,
  `title` varchar(80) default NULL,
  `prompt` varchar(80) NOT NULL default '>',
  `birth` int(10) unsigned NOT NULL default '0',
  `played` mediumint(8) unsigned NOT NULL default '0',
  `host` varchar(50) default NULL,
  `last_logon` int(10) unsigned NOT NULL default '0',
  `act` varchar(10) default NULL,
  `affected_by` varchar(10) default NULL,
  `pref` varchar(20) default NULL,
  `load_room` smallint(5) NOT NULL default '0',
  `gold` mediumint(8) unsigned NOT NULL default '0',
  `bank_gold` int(10) unsigned NOT NULL default '0',
  `olc_zone` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `plrobjs`
--

DROP TABLE IF EXISTS `plrobjs`;
CREATE TABLE `plrobjs` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `obj_id` char(36) NOT NULL default '',
  `vnum` smallint(5) unsigned NOT NULL default '0',
  `location` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`,`obj_id`)
) TYPE=MyISAM;

--
-- Table structure for table `poofs`
--

DROP TABLE IF EXISTS `poofs`;
CREATE TABLE `poofs` (
  `id` smallint(4) unsigned NOT NULL default '0',
  `poofin` varchar(120) default NULL,
  `poofout` varchar(120) default NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `shops`
--

DROP TABLE IF EXISTS `shops`;
CREATE TABLE `shops` (
  `uuid` varchar(36) NOT NULL default '',
  `gold` mediumint(8) unsigned NOT NULL default '0',
  `warned` tinyint(1) unsigned NOT NULL default '0',
  `timer` mediumint(6) unsigned NOT NULL default '0',
  `owned` tinyint(1) unsigned NOT NULL default '0',
  `deed` varchar(36) default NULL,
  `bank` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`uuid`)
) TYPE=MyISAM;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

