# MySQL-Front Dump 2.4
#
# Host: localhost   Database: nel
#--------------------------------------------------------
# Server version 3.23.34

USE nel;


#
# Table structure for table 'permission'
#

DROP TABLE IF EXISTS permission;
CREATE TABLE `permission` (
  `UId` int(10) unsigned NOT NULL default '0',
  `ClientApplication` char(64) NOT NULL default 'snowballs'
) TYPE=MyISAM;



#
# Table structure for table 'shard'
#

DROP TABLE IF EXISTS shard;
CREATE TABLE `shard` (
  `ShardId` int(10) NOT NULL auto_increment,
  `WsAddr` varchar(64) default NULL,
  `NbPlayers` int(10) unsigned default '0',
  `Name` varchar(64) default NULL,
  `Online` tinyint(1) unsigned default '0',
  `ClientApplication` varchar(64) NOT NULL default '',
  `Version` varchar(64) default NULL,
  PRIMARY KEY  (`ShardId`)
) TYPE=MyISAM COMMENT='contains all shards informations for login system';



#
# Table structure for table 'user'
#

DROP TABLE IF EXISTS user;
CREATE TABLE `user` (
  `UId` int(10) NOT NULL auto_increment,
  `Login` varchar(64) NOT NULL default '',
  `Password` tinytext NOT NULL,
  `ShardId` int(10) NOT NULL default '-1',
  `State` enum('Offline','Online') NOT NULL default 'Offline',
  PRIMARY KEY  (`UId`)
) TYPE=MyISAM COMMENT='contains all users informations for login system';

